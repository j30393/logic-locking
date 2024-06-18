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


void encryption::constructEncryKey(FType _ft, NODE* _combinekey, NODE* _node, double _rand){
	if(_ft == FType::AND){
		std::string name = "";
		name = "keyinput";
		name += std::to_string(keyCount++);
		NODE *ktem = new NODE(Type::PI, FType::BUF, name);
		KEY_Ary.push_back(ktem);
		//encry
		if(_rand > threshold){ //XOR
			name.clear();
			name = "ENCXOR";
			name += std::to_string(keyCount++);
			NODE *tem = new NODE(Type::Intl, FType::XOR, name);
			ENCY_Ary.push_back(tem);
			//keyi xor xi
			tem->insertFI(_node); 
			tem->insertFI(ktem);
			//and comnine
			tem->insertFO(_combinekey);
			_combinekey->insertFI(tem);
			
			// p =  _node set
			_node->setEncNode(tem);
			_node->setEncryption(true);
			_node->setEncType(FType::XOR);
			key += "0";
		}
		else{//XNOR
			name.clear();
			name = "ENCXNOR";
			name += std::to_string(keyCount++);
			NODE *tem = new NODE(Type::Intl, FType::XOR, name);
			ENCY_Ary.push_back(tem);
			
			//not flip
			name.clear();
			name = "ENCNOT";
			name += std::to_string(keyCount++);
			NODE *temNot = new NODE(Type::Intl, FType::NOT, name);
			ENCY_Ary.push_back(temNot);
					
			//keyi xor xi
			tem->insertFI(_node); 
			tem->insertFI(ktem);
			//and comnine
			tem->insertFO(temNot);
			temNot->insertFI(tem);
			temNot->insertFO(_combinekey);
			_combinekey->insertFI(temNot);
			
			// p =  _node set
			_node->setEncNode(temNot);
			_node->setEncryption(true);
			_node->setEncType(FType::XNOR);
			key += "1";
		}
	}
	else if(_ft == FType::OR){
		std::string name = "";
		name = "keyinput";
		name += std::to_string(keyCount++);
		NODE *ktem = new NODE(Type::PI, FType::BUF, name);
		KEY_Ary.push_back(ktem);
		//encry
		if(_rand > threshold){ //XOR
			name.clear();
			name = "ENCXOR";
			name += std::to_string(keyCount++);
			NODE *tem = new NODE(Type::Intl, FType::XOR, name);
			ENCY_Ary.push_back(tem);
			//keyi xor xi
			tem->insertFI(_node); 
			tem->insertFI(ktem);
			//or comnine
			tem->insertFO(_combinekey);
			_combinekey->insertFI(tem);
			
			// p =  _node set
			_node->setEncNode(tem);
			_node->setEncryption(true);
			_node->setEncType(FType::XOR);
			key += "0";
		}
		else{//XNOR
			name.clear();
			name = "ENCXNOR";
			name += std::to_string(keyCount++);
			NODE *tem = new NODE(Type::Intl, FType::XOR, name);
			ENCY_Ary.push_back(tem);
			
			//not flip
			name.clear();
			name = "ENCNOT";
			name += std::to_string(keyCount++);
			NODE *temNot = new NODE(Type::Intl, FType::NOT, name);
			ENCY_Ary.push_back(temNot);
					
			//keyi xor xi
			tem->insertFI(_node); 
			tem->insertFI(ktem);
			//or comnine
			tem->insertFO(temNot);
			temNot->insertFI(tem);
			temNot->insertFO(_combinekey);
			_combinekey->insertFI(temNot);
			
			// p =  _node set
			_node->setEncNode(temNot);
			_node->setEncryption(true);
			_node->setEncType(FType::XNOR);
			key += "1";
		}
	}
}

void encryption::AND_encryption(CONE* _cone){
	//f' = f∨(∧[n~1] (xi ^ki))
	
	std::string tems = "";
	std::string name = "";

	//make key encryption
	
	//construct AND key
	//this is and all of the keyi^xi
	name.clear();
	name = "ENCAND";
	name += std::to_string(keyCount++);
	NODE *andkey = new NODE(Type::Intl, FType::AND, name);
	ENCY_Ary.push_back(andkey);

	//this is construct keyi^xi
	for(auto p: _cone->getInput()){
		if(p->isEncryption()){
			andkey->insertFI(p->getEncNode());
			p->getEncNode()->insertFO(andkey);
		}
		else{
			//construct key
			double  randnum = (double)rand()/RAND_MAX ;
		//	std::cout<<randnum<<std::endl;
			constructEncryKey(FType::AND, andkey, p, randnum);	
		}
	}

	//combinational all or
	name.clear();
	name = "ENCOR";
	name += std::to_string(keyCount++);
	NODE *orkey = new NODE(Type::Intl, FType::OR, name);
	orkey->insertFI(andkey);
	orkey->insertFI(_cone->getOutput());
	ENCY_Ary.push_back(orkey);

	//erase and original FO's FI
	for(auto p: _cone->getOutput()->getFO()){
		p->eraseFI(_cone->getOutput());
		p->insertFI(orkey);
		orkey->insertFO(p);
	}
	// update original and
	_cone->getOutput()->clearFO();
	_cone->getOutput()->insertFO(orkey);
}


void encryption::OR_encryption(CONE* _cone){
	//f' = f∧(v[n~1] (xi ^ki))
	
	std::string tems = "";
	std::string name = "ENCAND";

	//make key encryption
	
	//construct OR key
	name.clear();
	name = "ENCOR";
	name += std::to_string(keyCount++);
	NODE *orkey = new NODE(Type::Intl, FType::OR, name);
	ENCY_Ary.push_back(orkey);
	
//	std::cout<<"size:"<<_cone->getInput().size()<<std::endl;
	
	for(auto p: _cone->getInput()){
		if(p->isEncryption()){ 
			orkey->insertFI(p->getEncNode());
			p->getEncNode()->insertFO(orkey);
		}
		else{
			//construct key
			double  randnum = (double)rand()/RAND_MAX ;
		//	std::cout<<randnum<<std::endl;
			constructEncryKey(FType::OR, orkey, p, randnum);	
		}
	}

	//combinational all and
	name.clear();
	name = "ENCAND";
	name += std::to_string(keyCount++);
	NODE *andkey = new NODE(Type::Intl, FType::AND, name);
	andkey->insertFI(orkey);
	andkey->insertFI(_cone->getOutput());
	ENCY_Ary.push_back(andkey);
	
	//erase and original FO's FI
	for(auto p: _cone->getOutput()->getFO()){
		p->eraseFI(_cone->getOutput());
		p->insertFI(andkey);
		andkey->insertFO(p);
	}
	// update original and
	_cone->getOutput()->clearFO();
	_cone->getOutput()->insertFO(andkey);
}

void encryption::ezXorenc(NODE* _node){
	std::string name = "";
	name = "keyinput";
	name += std::to_string(keyCount++);
	NODE *ktem = new NODE(Type::PI, FType::BUF, name);
	KEY_Ary.push_back(ktem);
	key += "0";
	
	name = "ENCXOR";
	name += std::to_string(keyCount++);
	NODE *encNode = new NODE(Type::Intl, FType::XOR, name);
	ENCY_Ary.push_back(encNode);
	for(auto p:_node->getFO()){
		p->eraseFI(_node);
		p->insertFI(encNode);
		encNode->insertFO(p);
	}
	_node->clearFO();
	_node->insertFO(encNode);
	encNode->insertFI(ktem);
	encNode->insertFI(_node);
	ktem->insertFO(encNode);
	_node->setEncNode(encNode);
	_node->setEncryption(true);
}

void encryption::ezXnorenc(NODE* _node){
	std::string name = "";
	name = "keyinput";
	name += std::to_string(keyCount++);
	NODE *ktem = new NODE(Type::PI, FType::BUF, name);
	KEY_Ary.push_back(ktem);
	key += "1";
	
	name = "ENCXNOR";
	name += std::to_string(keyCount++);
	NODE *encNode = new NODE(Type::Intl, FType::XNOR, name);
	ENCY_Ary.push_back(encNode);

	for(auto p:_node->getFO()){
		p->eraseFI(_node);
		p->insertFI(encNode);
		encNode->insertFO(p);
	}
	_node->clearFO();
	_node->insertFO(encNode);
	encNode->insertFI(ktem);
	encNode->insertFI(_node);
	ktem->insertFO(encNode);
	_node->setEncNode(encNode);
	_node->setEncryption(true);
}

void encryption::ezOrenc(NODE* _node){
	std::string name = "";
	name = "keyinput";
	name += std::to_string(keyCount++);
	NODE *ktem = new NODE(Type::PI, FType::BUF, name);
	KEY_Ary.push_back(ktem);
	key += "0";
	
	name = "ENCOR";
	name += std::to_string(keyCount++);
	NODE *encNode = new NODE(Type::Intl, FType::OR, name);
	ENCY_Ary.push_back(encNode);

	for(auto p:_node->getFO()){
		p->eraseFI(_node);
		p->insertFI(encNode);
		encNode->insertFO(p);
	}
	_node->clearFO();
	_node->insertFO(encNode);
	encNode->insertFI(ktem);
	encNode->insertFI(_node);
	ktem->insertFO(encNode);
	_node->setEncNode(encNode);
	_node->setEncryption(true);
}

void encryption::ezAndenc(NODE* _node){
	std::string name = "";
	name = "keyinput";
	name += std::to_string(keyCount++);
	NODE *ktem = new NODE(Type::PI, FType::BUF, name);
	KEY_Ary.push_back(ktem);
	key += "1";
	
	name = "ENCAND";
	name += std::to_string(keyCount++);
	NODE *encNode = new NODE(Type::Intl, FType::AND, name);
	ENCY_Ary.push_back(encNode);

	for(auto p:_node->getFO()){
		p->eraseFI(_node);
		p->insertFI(encNode);
		encNode->insertFO(p);
	}
	_node->clearFO();
	_node->insertFO(encNode);
	encNode->insertFI(ktem);
	encNode->insertFI(_node);
	ktem->insertFO(encNode);
	_node->setEncNode(encNode);
	_node->setEncryption(true);
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
