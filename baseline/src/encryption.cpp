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
			std::pair<bool,int> result = compareAndHammingDistance(golden_output, sa0_output);
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


// xor encryption 
void encryption::xor_encryption(){
	int total_enc_num = ceil(this->key_ratio * PI_Ary.size());
	assert(total_enc_num <= NODE_Ary.size());
	std::vector<NODE*> enc_nodes = getTopKNodes(NODE_Ary, total_enc_num);
	total_enc_num = std::min(total_enc_num, static_cast<int>(enc_nodes.size()));
	assert(total_enc_num > 0);
	if(is_debug){
		std::cout << "encryption a total of " << total_enc_num << " nodes" << std::endl;
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
	}

	// If the key-bit is ‘0’, then the key-gate
	// structure can be either ‘XOR- gate’ or ‘ XNOR- gate + inverter ’. 
	// Similarly, if the key-bit is ‘1’, then the key-gate
	// structure can be either ‘XNOR-gate’ or ‘ XOR- gate + inverter ’.

	for(int i = 0; i < total_enc_num; i++){
		NODE* enc_node = enc_nodes[i];
		if(key_arr[i] == 0){
			// XOR gate
			NODE* key_node = new NODE(Type::PI, FType::BUF, "keyinput" + std::to_string(i));
			KEY_Ary.push_back(key_node);
			int type = rand() % 2;
			// TODO: Do we need to update fanout and/or name2node?
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
					xor_node->setName(enc_node->getName());
					enc_node->setName(enc_node->getName() + std::to_string(i));
					*std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = xor_node;
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
					not_node->setName(enc_node->getName());
					enc_node->setName(enc_node->getName() + std::to_string(i));
					*std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = not_node;
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
					xnor_node->setName(enc_node->getName());
					enc_node->setName(enc_node->getName() + std::to_string(i));
					*std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = xnor_node;
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
					not_node->setName(enc_node->getName());
					enc_node->setName(enc_node->getName() + std::to_string(i));
					*std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = not_node;
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
