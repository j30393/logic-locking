#include "./inc/encryption.h"
//#define bug

encryption::encryption(){
	NODE_Ary.clear();
	PI_Ary.clear();
	PO_Ary.clear();
	KEY_Ary.clear();
	name2node.clear();
	filename = "";
	key="";
	twolevelfile = false;
	ENCY_Ary.clear();
	AND_Ary.clear();
	OR_Ary.clear();
	outputname ="";
	keyCount = 0;
	threshold = 0.4;
	constraint = 0;
}

encryption::~encryption(){
	for(auto p : NODE_Ary ){
		delete p;
	}
	NODE_Ary.clear();
	PI_Ary.clear();
	PO_Ary.clear();
	KEY_Ary.clear();
	name2node.clear();
	key="";
}

// 109062233 add graph traverse 
void encryption::graph_traverse(){
	for(auto p : NODE_Ary){
		std::cout << p <<std::endl;
	}
}

// add solver
std::vector<bool> encryption::solver(std::vector<bool> input){
	std::vector<bool> output;
	assert(input.size() == PI_Ary.size());
	for(auto n : NODE_Ary){
		n->setCurrentOutput(2);
	}
	for(int i = 0; i < PI_Ary.size(); i++){
		if(NODE_Ary[i]->is_StuckFaulting()){
			NODE_Ary[i]->setCurrentOutput(NODE_Ary[i]->getStuckFaultValue());
		}
		else{
			NODE_Ary[i]->setCurrentOutput(input[i]);
		}
		output.push_back(NODE_Ary[i]->getCurrentOutput());
	}
	// calculate the output 
	for(int i = PI_Ary.size(); i < NODE_Ary.size(); i++){
		// n is the current node
		NODE* n = NODE_Ary[i];
		int ans = -1;
		for(auto p : n->getFI()){
			if(p->getCurrentOutput() == 2){
				std::cout << "Current node: " << n->getName() << std::endl;
				std::cout << "fan in name: " << p->getName() << std::endl;
				std::cout << "error: topology sort failed" << std::endl;
			}
		}
		if(n->is_StuckFaulting()){
			ans = n->getStuckFaultValue();
		}
		else{
			switch(n->getFtype()){
				case(FType::AND):{
					ans = 1;
					for(auto p : n->getFI()){
						ans = ans & p->getCurrentOutput();
					}
					break;
				}
				case(FType::OR):{
					ans = 0;
					for(auto p : n->getFI()){
						ans = ans | p->getCurrentOutput();
					}
					break;
				}
				case(FType::NAND):{
					ans = 1;
					for(auto p : n->getFI()){
						ans = ans & p->getCurrentOutput();
					}
					ans = !ans;
					break;
				}
				case(FType::NOR):{
					ans = 0;
					for(auto p : n->getFI()){
						ans = ans | p->getCurrentOutput();
					}
					ans = !ans;
					break;
				}
				case(FType::XOR):{
					ans = 0;
					for(auto p : n->getFI()){
						ans = ans ^ p->getCurrentOutput();
					}
					break;
				}
				case(FType::XNOR):{
					ans = 0;
					for(auto p : n->getFI()){
						ans = ans ^ p->getCurrentOutput();
					}
					ans = !ans;
					break;
				}
				case(FType::NOT):{
					assert(n->getFIlen() == 1);
					ans = !n->getFI()[0]->getCurrentOutput();
					break;
				}
				case(FType::BUF):{
					assert(n->getFIlen() == 1);
					ans = n->getFI()[0]->getCurrentOutput();
					break;
				}
				default:{
					std::cout << "error: unexpected type" << std::endl;
				}
			}
		}
		n->setCurrentOutput(ans);
		output.push_back(ans);
	}
	assert(output.size() == NODE_Ary.size());
	return output;
}

std::pair<bool, int> compareAndHammingDistance(const std::vector<bool>& vec1, const std::vector<bool>& vec2) {
    // Check if the sizes are the same
    if (vec1.size() != vec2.size()) {
        throw std::invalid_argument("Vectors must be of the same length");
    }

    bool areEqual = true;
    int hammingDistance = 0;

    // Compare elements and calculate Hamming distance
    for (size_t i = 0; i < vec1.size(); ++i) {
        if (vec1[i] != vec2[i]) {
            areEqual = false;
            hammingDistance++;
        }
    }
	// std::cout << hammingDistance << std::endl;

    return std::make_pair(areEqual, hammingDistance);
}

// identify what we want to encrypt
std::vector<NODE*> getTopKNodes(const std::vector<NODE*>& nodes, size_t k) {
    // Copy the nodes to a new vector to avoid modifying the original
    std::vector<NODE*> sorted_nodes = nodes;

    // Sort the nodes based on fault_impact in descending order
    std::sort(sorted_nodes.begin(), sorted_nodes.end(), [](NODE* a, NODE* b) {
        return a->fault_impact > b->fault_impact;
    });

    // If k is greater than the number of nodes, adjust k
    if (k > sorted_nodes.size()) {
        k = sorted_nodes.size();
    }

    // Select the top k nodes
    std::vector<NODE*> top_k_nodes(sorted_nodes.begin(), sorted_nodes.begin() + k);
    return top_k_nodes;
}

// fault impact calculation 
void encryption::fault_impact_cal(){
	int num_tested = get_test_num();
	std::vector<bool> test_pattern(PI_Ary.size(), false);
	std::vector<bool> golden_output;
	std::vector<bool> sa0_output;
	std::vector<bool> sa1_output;

	for(int iteration = 0 ; iteration < num_tested ; iteration++){
		for(int i = 0; i < PI_Ary.size(); i++){
			test_pattern[i] = rand() % 2;
		}
		/*for(auto num : test_pattern){
			std::cout << num << " ";
		}*/
		golden_output = solver(test_pattern);
		for(auto n : NODE_Ary){
			// stuck at 0
			n->setStuckFaulting(true);
			n->setStuckFaultValue(0);
			sa0_output = solver(test_pattern);
			std::pair<int,int> result = compareAndHammingDistance(golden_output, sa0_output);
			if(!result.first){ // difference
				n->NoO0 ++ ;
				n->NoP0 += result.second;
			}
			n->setStuckFaulting(false);

			// stuck at 1
			n->setStuckFaulting(true);
			n->setStuckFaultValue(1);
			sa1_output = solver(test_pattern);
			result = compareAndHammingDistance(golden_output, sa1_output);
			if(!result.first){ // difference
				n->NoO1++;
				n->NoP1 += result.second;
			}
			n->setStuckFaulting(false);
		}
	}
	//setDebugMode(true);
	for(auto node: NODE_Ary){
		node->fault_impact = (node->NoO0 * node->NoP0 + node->NoO1 * node->NoP1);
		if(is_debug){
			std::cout << "Node " << node->getName() << " fault impact: " << node->fault_impact << std::endl;
		}
	}
	if(is_debug){
		for(auto node: NODE_Ary){
			std::cout << "Node " << node->getName() << std::endl;
			std::cout << "NoO0: " << node->NoO0 << " NoP0: " << node->NoP0 << std::endl;
			std::cout << "NoO1: " << node->NoO1 << " NoP1: " << node->NoP1 << std::endl;
		}
	}
	//setDebugMode(false);
	

	//setDebugMode(true);
	if(is_debug){
		int pattern_size = PI_Ary.size();
		int output_size = NODE_Ary.size() - PI_Ary.size();
		for(int i = 0; i < NODE_Ary.size(); i++){
			if(i < PI_Ary.size()){
				std::cout << "Node " << i << " : " << PI_Ary[i]->getName() << std::endl;
				std::cout << "Test value : " << test_pattern[i] << std::endl;
			}
			else{
				std::cout << "Node " << i << " : " << NODE_Ary[i]->getName() << std::endl;
				std::cout << "Output value : " << golden_output[i- PI_Ary.size()] << std::endl;
			}
		}
	}
	//setDebugMode(false);
	return;
}

void encryption::sl_compare_encryption() {
	int total_enc_num = ceil(this->key_ratio * PI_Ary.size());
	assert(total_enc_num <= NODE_Ary.size());

	if(is_debug){
		std::cout << "encryption a total of " << total_enc_num << " nodes" << "\n";
	}

	std::vector<bool>key_arr (total_enc_num, false);
	for(auto &&key_element : key_arr){
		key_element = rand() % 2;
		if(key_element == 0){
			key += "0";
		}
		else{
			key += "1";
		}
	}

	if(is_debug){
		std::cout << "encryption key_arr: ";
		for(auto key_element: key_arr){
			std::cout << key_element << " ";
		}
		std::cout << "\n";
	}

	std::queue<std::pair<NODE *, bool>> to_be_checked, waiting_list;
	std::vector<NODE *> to_be_enc;
	while(to_be_enc.size() < total_enc_num) {
		if(is_debug) std::cout << "Current to-be encrypted number: " << to_be_enc.size() << "\n";
		if(to_be_checked.empty() && waiting_list.empty()) {
			NODE *temp = initialize();
			to_be_checked.emplace(temp, 0);
			to_be_checked.emplace(temp, 1);
			to_be_enc.emplace_back(temp);
			temp->setEncryption(1);
		}
		else if(to_be_checked.empty()) {
			NODE *temp = waiting_list.front().first;
			to_be_checked.emplace(waiting_list.front());
			waiting_list.pop();
			to_be_enc.emplace_back(temp);
			temp->setEncryption(1);
		}

		NODE *cur = to_be_checked.front().first;
		bool way = to_be_checked.front().second;
		auto arr = (way == 0) ? cur->getFI() : cur->getFO();
		to_be_checked.pop();

		// To bypass not gates and buffers
		while(arr.size() == 1 && arr[0]->getType() != Type::PI) {
			cur = arr[0];
			arr = (way == 0) ? cur->getFI() : cur->getFO();
		}
		if(is_debug) std::cout << "white people\n";

		for(auto it: arr) {
			if(it->isEncryption()) continue;
			if(check_pairwise_secure(cur, it, way)) {
				to_be_checked.emplace(it, way);
				to_be_enc.emplace_back(it);
				it->setEncryption(1);
			}
			else {
				waiting_list.emplace(it, way);
			}
		}
	}

	// add key gates
	std::queue<NODE *> checker;
	while(!checker.empty()) checker.pop();

	for(int i = 0; i < total_enc_num; i++) {
		assert(i < to_be_enc.size());
		NODE *enc_node = to_be_enc[i];
		NODE *key_node = new NODE(Type::PI, FType::BUF, "keyinput" + std::to_string(i));
		NODE *xor_node = (key_arr[i] == 0) ? 
							new NODE(Type::Intl, FType::XOR, "xor" + std::to_string(i)):
							new NODE(Type::Intl, FType::XNOR, "xnor" + std::to_string(i));

		KEY_Ary.push_back(key_node);
		ENCY_Ary.push_back(xor_node);

		// make sure its not output, because im too lazy to implement that
		if(enc_node->getType() == Type::PO) {
			//change encoded node
			enc_node->setType(Type::Intl);
			enc_node->insertFO(xor_node);
			xor_node->setName(enc_node->getName());
			enc_node->setName(enc_node->getName() + std::to_string(i));
			*std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = xor_node;

			// xor node & all other nodes
			xor_node->insertFI(enc_node);
			xor_node->insertFI(key_node);
			xor_node->setType(Type::PO);

			// key node
			key_node->insertFO(xor_node);
		}
		else {
			// original encoded node change
			for(auto fan_out_node : enc_node->getFO()){
				checker.emplace(fan_out_node);
			}
			enc_node->clearFO();
			enc_node->insertFO(xor_node);
			enc_node->setEncNode(xor_node);

			// key node
			key_node->insertFO(xor_node);

			// xor node & all other nodes
			xor_node->insertFI(enc_node);
			xor_node->insertFI(key_node);
			while(!checker.empty()) {
				NODE *temp = checker.front();
				checker.pop();

				temp->eraseFI(enc_node);
				temp->insertFI(xor_node);
				xor_node->insertFO(temp);
			}
		}
	}
}

void encryption::sl_brute_encryption() {
	int total_enc_num = ceil(this->key_ratio * PI_Ary.size());
	total_enc_num = 128;
	assert(total_enc_num <= NODE_Ary.size());

	if(is_debug){
		std::cout << "encryption a total of " << total_enc_num << " nodes" << "\n";
	}

	std::vector<bool>key_arr (total_enc_num, false);
	for(auto &&key_element : key_arr){
		key_element = rand() % 2;
		if(key_element == 0){
			key += "0";
		}
		else{
			key += "1";
		}
	}

	if(is_debug){
		std::cout << "encryption key_arr: ";
		for(auto key_element: key_arr){
			std::cout << key_element << " ";
		}
		std::cout << "\n";
	}

	//int stands for which way 0 is up, 1 is down, 2 is down then up. currently 0 and 2 dont have any difference
	std::queue<std::pair<NODE *, int>> to_be_checked;
	std::vector<NODE *> to_be_enc, clique;
	std::unordered_set<NODE *> checked;
	while(to_be_enc.size() < total_enc_num) {
		if(is_debug) std::cout << to_be_enc.size() << " keys\n";
		if(to_be_checked.empty()) {
			NODE *temp = initialize();
			to_be_checked.emplace(temp, 0);
			to_be_checked.emplace(temp, 1);
			clique.clear();
			checked.clear();
		}

		NODE *cur = to_be_checked.front().first, *drop_one = NULL;
		int way = to_be_checked.front().second;
		auto arr = (way == 1) ? cur->getFO() : cur->getFI();
		to_be_checked.pop();

		to_be_enc.emplace_back(cur);
		clique.emplace_back(cur);
		cur->setEncryption(1);

		if(to_be_checked.size() + to_be_enc.size() >= total_enc_num) continue;

		// To bypass not gates and buffers
		//TODO: check if checking type is nessacrery by if PI fi size
		while(arr.size() == 1 && arr[0]->getType() != Type::PI) {
			drop_one = arr[0];
			arr = (way == 1) ? drop_one->getFO() : drop_one->getFI();
		}

		if(drop_one != NULL) arr = {drop_one};

		for(auto it: arr) {
			if(is_debug) std::cout << it->getName() << "\n";
			if(it->isEncryption() || checked.find(it) != checked.end()) continue;
			checked.emplace(it);
			//TODO: check if this is correct usage of theorem 1
			bool secured = 1;
			// if(way == 0) { //using theorem 1
			// 	secured = 0;
			// 	for(int i = 0; i < clique.size() && secured == 0; i++) {
			// 		//TODO: make this function
			// 		secured = check_brute_secure(cur, it);
			// 	}
			// }
			for(int i = 0; i < clique.size() && secured == 1; i++) {
				secured = check_brute_secure(cur, it);
			}

			if(secured) {
				to_be_checked.emplace(it, way);
				if(way == 1) to_be_checked.emplace(it, 2);
			}
		}
	}

	// add key gates
	std::queue<NODE *> checker;
	while(!checker.empty()) checker.pop();

	for(int i = 0; i < total_enc_num; i++) {
		assert(i < to_be_enc.size());
		NODE *enc_node = to_be_enc[i];
		NODE *key_node = new NODE(Type::PI, FType::BUF, "keyinput" + std::to_string(i));
		NODE *xor_node = (key_arr[i] == 0) ? 
							new NODE(Type::Intl, FType::XOR, "xor" + std::to_string(i)):
							new NODE(Type::Intl, FType::XNOR, "xnor" + std::to_string(i));

		KEY_Ary.push_back(key_node);
		ENCY_Ary.push_back(xor_node);

		// make sure its not output, because im too lazy to implement that
		if(enc_node->getType() == Type::PO) {
			//change encoded node
			enc_node->setType(Type::Intl);
			enc_node->insertFO(xor_node);
			xor_node->setName(enc_node->getName());
			enc_node->setName(enc_node->getName() + std::to_string(i));
			*std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = xor_node;

			// xor node & all other nodes
			xor_node->insertFI(enc_node);
			xor_node->insertFI(key_node);
			xor_node->setType(Type::PO);

			// key node
			key_node->insertFO(xor_node);
		}
		else {
			// original encoded node change
			for(auto fan_out_node : enc_node->getFO()){
				checker.emplace(fan_out_node);
			}
			enc_node->clearFO();
			enc_node->insertFO(xor_node);
			enc_node->setEncNode(xor_node);

			// key node
			key_node->insertFO(xor_node);

			// xor node & all other nodes
			xor_node->insertFI(enc_node);
			xor_node->insertFI(key_node);
			while(!checker.empty()) {
				NODE *temp = checker.front();
				checker.pop();

				temp->eraseFI(enc_node);
				temp->insertFI(xor_node);
				xor_node->insertFO(temp);
			}
		}
	}
}

int encryption::Rand(int n) {
	return rand() % n;
}

bool encryption::check_brute_secure(NODE *a, NODE *b) {
	std::unordered_set<NODE *> a_used_PI, b_used_PI, checked;
	std::queue<NODE *> qu;
	//if(is_debug) std::cout << "We checking\n";

	qu.emplace(a);
	while(!qu.empty()) {
		NODE *temp = qu.front();
		qu.pop();
		
		if(checked.find(temp) != checked.end()) continue;
		checked.emplace(temp);

		for(auto it: temp->getFI()) {
			qu.emplace(it);
		}
		if(temp->getType() == Type::PI && a_used_PI.find(temp) == a_used_PI.end()) a_used_PI.emplace(temp);
	}

	qu.emplace(b);
	checked.clear();
	while(!qu.empty()) {
		NODE *temp = qu.front();
		qu.pop();
		
		if(checked.find(temp) != checked.end()) continue;
		checked.emplace(temp);

		for(auto it: temp->getFI()) {
			qu.emplace(it);
		}
		if(temp->getType() == Type::PI && b_used_PI.find(temp) == b_used_PI.end()) b_used_PI.emplace(temp);
	}

	std::vector<int> which_input;
	for(int i = 0; i < PI_Ary.size(); i++) {
		if((a_used_PI.find(PI_Ary[i]) == a_used_PI.end()) || (b_used_PI.find(PI_Ary[i]) == b_used_PI.end())) {
			which_input.emplace_back(i);
		}
	}
	
	//if(is_debug) std::cout << "Determine inputs complete\n";

	// set to 2^20 times
	unsigned int up_time = 10;
	for(auto n : NODE_Ary){
		n->setCurrentOutput(2);
	}
	for(int i = 0; i < PI_Ary.size(); i++) {
		PI_Ary[i]->setCurrentOutput(rand() % 2);
	}

	for(int i = 0; i < pow(2, std::min(unsigned(which_input.size()), up_time)); i++) {
		//if(is_debug) std::cout << i << " pair checking\n";
		std::vector<bool> out, changed_out;
		bool unchanged_a = 0, unchanged_b = 0;

		if(which_input.size() > up_time) {
			std::random_shuffle(which_input.begin(), which_input.end());
		}
		for(int j = 0; j < std::min(unsigned(which_input.size()),up_time); j++) {
			set_unknown(PI_Ary[which_input[j]]);
			// brute force if less, rand if more
			int temp = (which_input.size() <= up_time) ? (i >> j) & 1 : (rand() % 2);
			PI_Ary[which_input[j]]->setCurrentOutput(temp);
		}

		for(auto it: PO_Ary) {
			out.push_back(it->getCurrentOutput());
		}

		set_unknown(a);
		a->setStuckFaultValue(1);
		for(auto it: PO_Ary) {
			changed_out.push_back(it->getCurrentOutput());
		}
		a->setStuckFaultValue(0);

		if(out == changed_out) unchanged_a = 1;

		changed_out.clear();

		set_unknown(b);
		set_unknown(a);
		b->setStuckFaultValue(1);
		for(auto it: PO_Ary) {
			changed_out.push_back(it->getCurrentOutput());
		}
		b->setStuckFaultValue(0);

		if(out == changed_out) unchanged_b = 1;

		if(unchanged_a ^ unchanged_b) return 0;
	}

	return 1;
}

void encryption::set_unknown(NODE *a) {
	std::queue<NODE *> qu;
	std::unordered_set<NODE *> checked;
	qu.emplace(a);
	while(!qu.empty()) {
		NODE *temp = qu.front();
		qu.pop();
		if(checked.find(temp) != checked.end()) continue;
		checked.emplace(temp);
		temp->setCurrentOutput(2);
		for(auto it: temp->getFO()) {
			qu.emplace(it);
		}
	}
}


bool encryption::check_pairwise_secure(NODE *main, NODE *bef, bool way) {
	bool ret = 1;

	if(way == 1) std::swap(main, bef);

	FType main_type = main->getFtype(), nono_type[2];
	if(main_type == FType::BUF || main_type == FType::NOT) return 0;
	if(main_type == FType::XNOR || main_type == FType::XOR) return 1;
	switch(main_type) {
		case FType::AND   	: nono_type[0] = FType::AND, nono_type[1] = FType::NOR;		break;
		case FType::OR 		: nono_type[0] = FType::OR, nono_type[1] = FType::NAND;		break;
		case FType::NOR 	: nono_type[0] = FType::OR, nono_type[1] = FType::NAND;		break;
		case FType::NAND 	: nono_type[0] = FType::AND, nono_type[1] = FType::NOR;		break;
		default: {
			if(is_debug) std::cout << "something happened and you should fix it\n";
			assert(0);
		}
	}

	for(auto it: main->getFI()) {
		if(it == bef) continue;
		if(it->getFtype() == nono_type[0] || it->getFtype() == nono_type[1]) return 0;
	}

	return 1;
}

NODE *encryption::initialize() {
	int cur_max = 0;
	NODE *ret_node = NULL;

	for(auto it: NODE_Ary) {
		if(it->isEncryption()) continue;
		if(cur_max < it->getFOlen()) {
			cur_max = it->getFOlen();
			ret_node = it;
		}
	}
	assert(ret_node != NULL);
	return ret_node;
}

void encryption::sl_one_encryption() {
	int total_enc_num = ceil(this->key_ratio * PI_Ary.size()), output_find = 0;
	assert(total_enc_num <= NODE_Ary.size());

	if(is_debug){
		std::cout << "encryption a total of " << total_enc_num << " nodes" << "\n";
	}

	std::vector<bool>key_arr (total_enc_num, false);
	for(auto &&key_element : key_arr){
		key_element = rand() % 2;
		if(key_element == 0){
			key += "0";
		}
		else{
			key += "1";
		}
	}

	if(is_debug){
		std::cout << "encryption key_arr: ";
		for(auto key_element: key_arr){
			std::cout << key_element << " ";
		}
		std::cout << "\n";
	}

	// find the nodes to be encrypted
	std::queue<NODE *> to_be_checked;
	std::vector<NODE *> to_be_enc;
	while(to_be_enc.size() < total_enc_num && output_find <= PO_Ary.size()) {
		if(to_be_checked.size() == 0) {
			output_find++;
			if(output_find > PO_Ary.size()) break;
			to_be_checked.emplace(PO_Ary[output_find - 1]);
			PO_Ary[output_find - 1]->setEncryption(1);
			to_be_enc.emplace_back(PO_Ary[output_find - 1]);
		}

		NODE *temp = to_be_checked.front();
		to_be_checked.pop();

		for(auto it: temp->getFI()) {
			if(it->isEncryption()) continue;
			it->setEncryption(1);
			to_be_enc.emplace_back(it);
			to_be_checked.emplace(it);
		}
	}

	if(is_debug) {
		std::cout << "Size of nodes to be encrypted: " << to_be_enc.size() << "\n";
		std::cout << "Size of main outputs: " << PO_Ary.size() << "\n";
		std::cout << "the stuff " << (to_be_enc.size() < total_enc_num) << (output_find <= PO_Ary.size()) << "\n";
	}

	// add key gates
	while(!to_be_checked.empty()) to_be_checked.pop();
	for(int i = 0; i < total_enc_num; i++) {
		assert(i < to_be_enc.size());
		NODE *enc_node = to_be_enc[i];
		NODE *key_node = new NODE(Type::PI, FType::BUF, "keyinput" + std::to_string(i));
		NODE *xor_node = (key_arr[i] == 0) ? 
							new NODE(Type::Intl, FType::XOR, "xor" + std::to_string(i)):
							new NODE(Type::Intl, FType::XNOR, "xnor" + std::to_string(i));

		KEY_Ary.push_back(key_node);
		ENCY_Ary.push_back(xor_node);

		// make sure its not output, because im too lazy to implement that
		if(enc_node->getType() == Type::PO) {
			//change encoded node
			enc_node->setType(Type::Intl);
			enc_node->insertFO(xor_node);
			xor_node->setName(enc_node->getName());
			enc_node->setName(enc_node->getName() + std::to_string(i));
			*std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = xor_node;

			// xor node & all other nodes
			xor_node->insertFI(enc_node);
			xor_node->insertFI(key_node);
			xor_node->setType(Type::PO);

			// key node
			key_node->insertFO(xor_node);
		}
		else {
			// original encoded node change
			for(auto fan_out_node : enc_node->getFO()){
				to_be_checked.emplace(fan_out_node);
			}
			enc_node->clearFO();
			enc_node->insertFO(xor_node);
			enc_node->setEncNode(xor_node);

			// key node
			key_node->insertFO(xor_node);

			// xor node & all other nodes
			xor_node->insertFI(enc_node);
			xor_node->insertFI(key_node);
			while(!to_be_checked.empty()) {
				NODE *temp = to_be_checked.front();
				to_be_checked.pop();

				temp->eraseFI(enc_node);
				temp->insertFI(xor_node);
				xor_node->insertFO(temp);
			}
		}
	}
}

void encryption::erasePI(NODE *_node) {
	for(auto it = PI_Ary.begin(); it != PI_Ary.end(); it++) {
		if(*it == _node) {
			PI_Ary.erase(it);
			break;
		}
	}
}

void encryption::erasePO(NODE *_node) {
	for(auto it = PO_Ary.begin(); it != PO_Ary.end(); it++) {
		if(*it == _node) {
			PO_Ary.erase(it);
			break;
		}
	}
}

// xor encryption 
void encryption::xor_encryption(){
	int total_enc_num = ceil(this->key_ratio * PI_Ary.size());
	assert(total_enc_num <= NODE_Ary.size());
	if(is_debug){
		std::cout << "encryption a total of " << total_enc_num << " nodes" << std::endl;
	}
	std::vector<NODE*> enc_nodes = getTopKNodes(NODE_Ary, total_enc_num);
	std::vector<bool>key_arr (total_enc_num, false);
	for(auto &&key_element : key_arr){
		key_element = rand() % 2;
		if(key_element == 0){
			key += "0";
		}
		else{
			key += "1";
		}
	}

	if(is_debug){
		std::cout << "encryption key_arr: ";
		for(auto key_element: key_arr){
			std::cout << key_element << " ";
		}
	}

	// If the key-bit is ‘0’, then the key-gate
	// structure can be either ‘XOR- gate’ or ‘ XNOR- gate + inverter ’. 
	// Similarly, if the key-bit is ‘1’, then the key-gate
	// structure can be either ‘XNOR-gate’ or ‘ NOR- gate + inverter ’.

	for(int i = 0; i < total_enc_num; i++){
		NODE* enc_node = enc_nodes[i];
		if(key_arr[i] == 0){
			// XOR gate
			NODE* key_node = new NODE(Type::PI, FType::BUF, "keyinput" + std::to_string(i));
			KEY_Ary.push_back(key_node);
			int type = rand() % 2;
			if(type == 1){
				// xor gate
				NODE* xor_node = new NODE(Type::Intl, FType::XOR, "xor" + std::to_string(i));
				ENCY_Ary.push_back(xor_node);
				xor_node->insertFI(enc_node);
				xor_node->insertFI(key_node);
				enc_node->setEncNode(xor_node);
				enc_node->setEncryption(true);
				// if we are operating on the output node
				if(enc_node->getFO().size() == 0){
					PO_Ary.push_back(xor_node);
				}
				else{ // none output node
					for(auto fan_out_node : enc_node->getFO()){
						fan_out_node->insertFI(xor_node);
						fan_out_node->eraseFI(enc_node);
					}
				}
			}
			else{
				// xnor gate + not
				NODE* xnor_node = new NODE(Type::Intl, FType::XNOR, "xnor" + std::to_string(i));
				ENCY_Ary.push_back(xnor_node);
				xnor_node->insertFI(enc_node);
				xnor_node->insertFI(key_node);
				NODE* not_node = new NODE(Type::Intl, FType::NOT, "not" + std::to_string(i));
				ENCY_Ary.push_back(not_node);
				not_node->insertFI(xnor_node);
				enc_node->setEncNode(not_node);
				enc_node->setEncryption(true);
				// if we are operating on the output node
				if(enc_node->getFO().size() == 0){
					PO_Ary.push_back(not_node);
				}
				else{ // none output node
					for(auto fan_out_node : enc_node->getFO()){
						fan_out_node->insertFI(not_node);
						fan_out_node->eraseFI(enc_node);
					}
				}
			}
		}
		else{
			// XNOR gate
			NODE* key_node = new NODE(Type::PI, FType::BUF, "keyinput" + std::to_string(i));
			KEY_Ary.push_back(key_node);
			int type = rand() % 2;
			if(type == 1){
				// xnor gate
				NODE* xnor_node = new NODE(Type::Intl, FType::XNOR, "xor" + std::to_string(i));
				ENCY_Ary.push_back(xnor_node);
				xnor_node->insertFI(enc_node);
				xnor_node->insertFI(key_node);
				enc_node->setEncNode(xnor_node);
				enc_node->setEncryption(true);
				// if we are operating on the output node
				if(enc_node->getFO().size() == 0){
					PO_Ary.push_back(xnor_node);
				}
				else{ // none output node
					for(auto fan_out_node : enc_node->getFO()){
						fan_out_node->insertFI(xnor_node);
						fan_out_node->eraseFI(enc_node);
					}
				}
			}
			else{
				// xor gate + not
				NODE* xor_node = new NODE(Type::Intl, FType::XOR, "xnor" + std::to_string(i));
				ENCY_Ary.push_back(xor_node);
				xor_node->insertFI(enc_node);
				xor_node->insertFI(key_node);
				NODE* not_node = new NODE(Type::Intl, FType::NOT, "not" + std::to_string(i));
				ENCY_Ary.push_back(not_node);
				not_node->insertFI(xor_node);
				enc_node->setEncNode(not_node);
				enc_node->setEncryption(true);
				// if we are operating on the output node
				if(enc_node->getFO().size() == 0){
					PO_Ary.push_back(not_node);
				}
				else{ // none output node
					for(auto fan_out_node : enc_node->getFO()){
						fan_out_node->insertFI(not_node);
						fan_out_node->eraseFI(enc_node);
					}
				}
			}
		}
	}
}


void encryption::setOutputname(std::string _name){
	twolevelfile = true;
	outputname = _name;
}

void encryption::readfile(std::string _filename){
	filename = _filename;

	std::fstream input;
	input.open(_filename,std::ios::in);
	
	std::string buffer;

	int coun = 0;
	while(std::getline(input, buffer)){
		std::string checkerflag = "";
		checkerflag.assign(buffer, 0, 2);
		//annotation
		if(buffer[0]=='#')
			continue;
		if(buffer.empty()){
		//	std::cout<<"empty\n";
			continue;
		}
		if(checkerflag == "IN"){ // input 
			std::string name = "";
			name.assign(buffer, 6, buffer.size()-7);
				
			Type	t  = Type::PI;
			FType	ft = FType::BUF;
			
			NODE *n;
			auto it = name2node.find(name);
			if(it == name2node.end() ){
				n =new NODE(t, ft, name);
				n->setId(coun++); //set ID	
				n->setDepth(0); //set depth
				NODE_Ary.push_back(n);
				name2node[name] = n; 
			}
			else{
				n = it->second;
				n->setFtype(ft);
				n->setType(t);
			}
			//Input push in
			PI_Ary.push_back(n);


		}
		else if(checkerflag == "OU"){
			std::string name = "";
			name.assign(buffer, 7, buffer.size()-8);
			

			Type	t  = Type::PO;
			FType	ft = FType::BUF;
			
			NODE *n;
			auto it = name2node.find(name);
			if(it == name2node.end() ){
				n =new NODE(t, ft, name);
				n->setId(coun++); //set ID	
				NODE_Ary.push_back(n);
				name2node[name] = n; 
			}
			else{
					n = it->second;
					//n->setFtype(ft);
				if(it->second->getType() != Type::PI)
					n->setType(t);

			}
			//push in
			PO_Ary.push_back(n);	
		}
		else{
			Type t =Type::Intl; //set type 
			FType ft;
			
			std::string tem_buf = "";
			std::stringstream ss;
			ss << buffer;
			
			//name
			std::string name = "";
			ss >> name;
			// = kill
			ss >> tem_buf;
			tem_buf.clear();
			

			//get Ftype
			ss >> tem_buf;
			std::string ft_name = "";
			ft_name.assign(tem_buf, 0, 3);

			std::transform(ft_name.begin(), ft_name.end(), ft_name.begin(), tolower);
			if(ft_name == "not"){
				ft = FType::NOT;
				tem_buf.assign(tem_buf, 4, tem_buf.size() - 4 );
			}
			else if(ft_name == "buf"){
				ft = FType::BUF;
				tem_buf.assign(tem_buf, 4, tem_buf.size() - 4 );
			}
			else if(ft_name == "and"){
				ft = FType::AND;
				tem_buf.assign(tem_buf, 4, tem_buf.size() - 4 );
			}
			else if(ft_name == "xor"){
				ft = FType::XOR;
				tem_buf.assign(tem_buf, 4, tem_buf.size() - 4 );
			}
			else if(ft_name == "xno"){
				ft = FType::XNOR;
				tem_buf.assign(tem_buf, 5, tem_buf.size() - 5 );
			}
			else if(ft_name == "nan"){
				ft = FType::NAND;
				tem_buf.assign(tem_buf, 5, tem_buf.size() - 5 );
			}
			else if(ft_name == "nor"){
				ft = FType::NOR;
				tem_buf.assign(tem_buf, 4, tem_buf.size() - 4 );
			}
			else if(ft_name == "or("){
				ft = FType::OR;
				tem_buf.assign(tem_buf, 3, tem_buf.size() - 3 );
			}
			else{
				continue;
			}

			//creat node & push
			auto it = name2node.find(name);
			NODE *n;
			if(it == name2node.end() ){
				n =new NODE(t, ft, name);
				n->setId(coun++); //set ID	
				NODE_Ary.push_back(n);
				name2node[name] = n; 
			}
			else{
				n = it->second;
			//	std::cout<<n->getName()<<std::endl;
				n->setFtype(ft);
			}

			//erase unuse char
			if(tem_buf[0] == '(')
				tem_buf.erase(tem_buf.begin());
			if(tem_buf[0] == ',')
				tem_buf.erase(tem_buf.begin());
			if(tem_buf[tem_buf.size()-1] == ',')
				tem_buf.pop_back();
			if(tem_buf[tem_buf.size()-1] == ')')
				tem_buf.pop_back();
		
			//std::cout<<"insert node ( "<<name<<" ) = ";
			//std::cout<<tem_buf<<" ";
			

			it = name2node.find(tem_buf);
			if(it == name2node.end() ){
				NODE *tem_n;
				tem_n =new NODE(t, FType::BUF, tem_buf);
				tem_n->setId(coun++); //set ID	
				NODE_Ary.push_back(tem_n);
				name2node[tem_buf] = tem_n; 
				//fan out
				tem_n->insertFO(n);
				//fan in
				n->insertFI(tem_n);
			}
			else{
				NODE *tem_n;
				tem_n = it->second;
				//fan out
				tem_n->insertFO(n);
				//fan in
				n->insertFI(tem_n);
			}

			
			//delete [] tem_n;

			//clear
			tem_buf.clear();
			tem_buf="";

			while(ss >> tem_buf){
				if(tem_buf[0] == '(')
					tem_buf.erase(tem_buf.begin());
				if(tem_buf[0] == ',')
					tem_buf.erase(tem_buf.begin());
				if(tem_buf[tem_buf.size()-1] == ',')
					tem_buf.pop_back();
				if(tem_buf[tem_buf.size()-1] == ')')
					tem_buf.pop_back();
			
			//	std::cout<<tem_buf<<" ";
				NODE *tem_n;	
				it = name2node.find(tem_buf);
				if(it == name2node.end() ){
					tem_n =new NODE(t, FType::BUF, tem_buf);
					tem_n->setId(coun++); //set ID	
					NODE_Ary.push_back(tem_n);
					name2node[tem_buf] = tem_n; 
				}
				else{
					tem_n = it->second;
				}
				//fan ou
				tem_n->insertFO(n);
				//fan in
				n->insertFI(tem_n);
			//	delete [] tem_n;
			}

			//std::cout<<std::endl;
		}
		buffer.clear();
	}
	input.close();
	//std::cout<<constraint<<std::endl;
}

std::ostream& operator<<(std::ostream& os, NODE* p){
		//<<"#node: "<<NODE_Ary.size()<<std::endl;
	os<<"-------------------------------------------------------\n";
	os<<"name: "<<p->getName()<<std::endl;
	os<<"Gate type: "<<p->getFtype()<<std::endl;
	os<<"Type: "<<p->getType()<<std::endl;
	os<<"ID: "<<p->getId()<<std::endl;
	os<<"And Count: "<<p->getAndC()<<std::endl;
	os<<"Or Count: "<<p->getOrC()<<std::endl;
	os<<"FI node : ";
	for(auto q :p->getFI()){
		os<<q->getName()<<" ";
	}
	os<<"\nFO node : ";
	for(auto q :p->getFO()){
		os<<q->getName()<<" ";
	}
	os << "\ndepth "  << p->getDepth()<<std::endl ; 

	os<<"\n-------------------------------------------------------\n";
	return os;
}


/**
 * Topological sort for the graph using kahn's algorithm
 * https://www.geeksforgeeks.org/topological-sorting-indegree-based-solution/
 * 
 * @param none
 * @return none but finish the depth information for that graph (depth...etc)
 */
void encryption::topological_sort(){

	visited.resize(NODE_Ary.size());
	in_degree.resize(NODE_Ary.size());
	std::fill(visited.begin(), visited.end(), 0);
	std::fill(in_degree.begin(), in_degree.end(), 0);
	
	for(auto p : NODE_Ary){
		for(auto q : p->getFO()){
			in_degree[q->getId()]++;
		}
	}

	std::queue<NODE*> q;

	for(auto p : NODE_Ary){
		if(in_degree[p->getId()] == 0){
			q.push(p);
			p->setDepth(0);
		}
	}

	std::vector<NODE*> result;

    while (!q.empty()) {
        NODE* node = q.front();
        q.pop();
		if(node->getDepth() == -0xfffffff){
			int depth = -1;
			for(auto children : node->getFI()){
				depth = std::max(depth, children->getDepth());
			}
			node->setDepth(depth+1);
		}
        result.push_back(node);
       
        // Decrease indegree of adjacent vertices as the
        // current node is in topological order
        for (auto it : node->getFO()) {
            in_degree[it->getId()]--;
           
            // If indegree becomes 0, push it to the queue
            if (in_degree[it->getId()] == 0)
                q.push(it);
        }
    }
 
    // Check for cycle
    assert(result.size() == NODE_Ary.size());
	NODE_Ary.clear();
	for(auto it : result){
		NODE_Ary.push_back(it);
	}
	result.clear();
	result.shrink_to_fit(); // release memory 
 
	/*for(auto it: NODE_Ary){
		std::cout << it << std::endl;
	}*/
	if(this->is_debug){
		for(auto it: NODE_Ary){
			std::cout << "name : " << it->getName() << " depth : " << it->getDepth() << std::endl;
			for(auto child: it->getFI()){
				std::cout << "child : " << child->getName() << " depth : " << child->getDepth() << std::endl;
			}
		}
		std::cout << "\n\n\n\n";
	}
}

void encryption::outputfile(){
	std::fstream out;
	std::string fname = "";
	
	if(twolevelfile)
		fname=outputname;
	else{
		fname = filename;
		fname.erase(fname.end()-6, fname.end());
		fname += "ENC.bench";
	}
	out.open(fname, std::ios::out);
	
/*	if(out.is_open()){
		std::cout<<"out open\n";
	}*/

	out<<"# key="<<key<<std::endl;
	//INPUT
	for(auto p: PI_Ary){
		std::string tem = "INPUT(";
		tem += p->getName();
		tem+=")";
		out<<tem<<std::endl;
	}

	//OUTPUT
	for(auto p: PO_Ary){
		std::string tem = "OUTPUT(";
		tem += p->getName();
		tem+=")";
		out<<tem<<std::endl;
	}

	//OUTkey
	for(auto p: KEY_Ary){
		std::string tem = "INPUT(";
		tem += p->getName();
		tem+=")";
		out<<tem<<std::endl;
	}

	// original circuit
	for(auto p: NODE_Ary){
//		std::cout<<p->getName()<<std::endl;
		std::string tem = "";
		if( p->getType() == Type::Intl || p->getType() == Type::PO ){
			tem = p->getName();
			tem += " = ";
			tem += p->stringFType();
			tem += "(";
			
			for(size_t i=0; i< p->getFI().size(); i++){
				tem += p->getFI()[i]->getName();
				if(i+1 == p->getFI().size()){
					tem += ")";
				}
				else
					tem += ",";
			}
			out<<tem<<std::endl;
		}
	}
	// encry circuit
	for(auto p: ENCY_Ary){
		std::string tem = "";
		tem = p->getName();
		tem += " = ";
		tem += p->stringFType();
		tem += "(";
		
		for(size_t i=0; i< p->getFI().size(); i++){
			tem += p->getFI()[i]->getName();
			if(i+1 == p->getFI().size()){
				tem += ")";
			}
			else
				tem += ",";
		}
		out<<tem<<std::endl;
	}
	std::cout<<key<<std::endl;	
	out.close();
}
