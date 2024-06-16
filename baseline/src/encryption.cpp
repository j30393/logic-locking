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
	area = 0;
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
	area = 0;
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
		if(checkerflag == "IN"){
			std::string name = "";
			name.assign(buffer, 6, buffer.size()-7);
				
			Type	t  = Type::PI;
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

	//caculate area
	this->caculateArea();
	
	surplus_area = (int)std::floor((double)area*0.1);
	constraint = (int)std::floor((double)surplus_area/(double)4.0);

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
	os<<"CCO -> "<<p->getCC0()<<std::endl;
	os<<"CC1 -> "<<p->getCC1()<<std::endl;
	os<<"CO -> "<<p->getCO()<<std::endl;
	os<<"FI node : ";
	for(auto q :p->getFI()){
		os<<q->getName()<<" ";
	}
	os<<"\nFO node : ";
	for(auto q :p->getFO()){
		os<<q->getName()<<" ";
	}
	os<<"\n-------------------------------------------------------\n";
	return os;
}

bool compareNode(NODE *_node1, NODE *_node2 ){
	return _node1->getEnd() > _node2->getEnd();
}

void encryption::caculateArea(){
	area = 0; //reset
	for(auto p : NODE_Ary){
		area += p->getCost();  
	}
}

void encryption::controllability(){
	for(auto p :NODE_Ary){
		int min = 2147483640;
		int max = 0;
		int a,b;
		if(p->getType() == Type::PI){
			p->setCC0(1);
			p->setCC1(1);
		}
		else{
			switch(p->getFtype())
			{
				case FType::AND   	:  
					min = 2147483640;
					max = 0;
					for(auto q: p->getFI()){
						max += q->getCC1();
						if(min > q->getCC0())
							min = q->getCC0();
					}
					p->setCC0(min+1);
					p->setCC1(max+1);
					break;
				case FType::OR 		:  
					min = 2147483640;
					max = 0;
					for(auto q: p->getFI()){
						max += q->getCC0();
						if(min > q->getCC1())
							min = q->getCC1();
					}
					p->setCC0(max+1);
					p->setCC1(min+1);
					break;
				case FType::NOR 	:  
					min = 2147483640;
					max = 0;
					for(auto q: p->getFI()){
						max += q->getCC0();
						if(min > q->getCC1())
							min = q->getCC1();
					}
					p->setCC0(min+1);
					p->setCC1(max+1);
					break;
				case FType::NAND 	:  
					min = 2147483640;
					max = 0;
					for(auto q: p->getFI()){
						max += q->getCC1();
						if(min > q->getCC0())
							min = q->getCC0();
					}
					p->setCC0(max+1);
					p->setCC1(min+1);
					break;
				case FType::NOT 	: 
					p->setCC0(p->getFI()[0]->getCC1()+1);
					p->setCC1(p->getFI()[0]->getCC0()+1);
					break;
				case FType::XNOR 	:
					if(p->getFI().size() == 0)
						break;
					else if(p->getFI().size() == 1){
						a = p->getFI()[0]->getCC0() + p->getFI()[0]->getCC1();
						p->setCC0(a+1);
						break;
					}

					a = p->getFI()[0]->getCC0() + p->getFI()[1]->getCC1();
					b = p->getFI()[1]->getCC0() + p->getFI()[0]->getCC1();
					p->setCC0(std::min(a, b)+1);
					a = p->getFI()[0]->getCC0() + p->getFI()[1]->getCC0();
					b = p->getFI()[1]->getCC1() + p->getFI()[0]->getCC1();
					p->setCC1(std::min(a, b)+1);
					break;
				case FType::XOR 	:  
					if(p->getFI().size() == 0)
						break;
					else if(p->getFI().size() == 1){
						a = p->getFI()[0]->getCC0() + p->getFI()[0]->getCC1();
						p->setCC0(a+1);
						break;
					}
					a = p->getFI()[0]->getCC0() + p->getFI()[1]->getCC1();
					b = p->getFI()[1]->getCC0() + p->getFI()[0]->getCC1();
					p->setCC0(std::min(a, b)+1);
					a = p->getFI()[0]->getCC0() + p->getFI()[1]->getCC0();
					b = p->getFI()[1]->getCC1() + p->getFI()[0]->getCC1();
					p->setCC1(std::min(a, b)+1);
					break;
				case FType::BUF 	:  
					if(p->getFI().size() == 0)
						break;
					p->setCC0(p->getFI()[0]->getCC0()+1);
					p->setCC1(p->getFI()[0]->getCC1()+1);
					break;
				default    			:  
					if(p->getFI().size() == 0)
						break;
					p->setCC0(p->getFI()[0]->getCC0()+1);
					p->setCC1(p->getFI()[0]->getCC1()+1);
			}
		}
	}
}


void encryption::DFS(int _id, int* _time){
//	std::cout<<*_time<<std::endl;

	color[_id] = 2; //gray
	NODE_Ary[_id]->setStart((*_time)++);	
	for(auto p : NODE_Ary[_id]->getFO()){
		if(color[p->getId()] == 0){
			DFS(p->getId(), _time);	
		}
	}
//	std::cout<<*_time<<std::endl;
	color[_id] = 1;//black
	NODE_Ary[_id]->setEnd((*_time)++);

}


void encryption::topological_sort(){

	color.resize(NODE_Ary.size());

	std::fill(color.begin(), color.end(), 0);


	int time = 0;
	
//	for(size_t i=0;i<NODE_Ary.size();i++)
//		std::cout<<color[i]<<std::endl;

	for(auto p : NODE_Ary){
		if(color[p->getId()]==0){
			DFS(p->getId() , &time);
		}
	}

	std::sort(NODE_Ary.begin(), NODE_Ary.end(), compareNode); //sory by finifsh time
	// make controllability
	controllability();


	int count = 0;
	for(auto p :NODE_Ary){
		//reset id
		p->setId(count++); //set ID
		if(p->getFtype() == FType::AND)
			AND_Ary.push_back(p);
		else if(p->getFtype() == FType::OR)
			OR_Ary.push_back(p);
		//count and num
		if(p->getFI().size()==0){
			p->setAndC(0);
			p->setOrC(0);
		}
		else{
			if(p->getFtype() == FType::AND){
				int max_and = 0;
				RecursiveFtype(NODE_Ary[p->getId()], max_and, FType::AND);
				//set ANDC
				p->setAndC(max_and+1);
			}
			else if(p->getFtype() == FType::OR){
				int max_or = 0;
				RecursiveFtype(NODE_Ary[p->getId()], max_or, FType::OR);
				//set ANDC
				p->setOrC(max_or+1);
			}
			else{
				p->setAndC(0);
				p->setOrC(0);
			}
			
		}
		//or count
		//std::cout<<p->getId()<<"->"<<p->getName()<<"("<<p->getStart()<<","<<p->getEnd()<<")"<<"\n";
	}
	
}

void encryption::RecursiveFtype(NODE* _node, int& max,FType _ft){
	for(auto p : _node->getFI()){
		if(p->getFtype() == _ft){
			switch(_ft)
			{
				case FType::OR :
					if(p->getOrC() > max)
						max = p->getOrC();
					break;
				case FType::AND :
					if(p->getAndC() > max)
						max = p->getAndC();
					break;
				default :
					break;
			}
		}
		else if(p->getFtype() == FType::BUF){
			this->RecursiveFtype(p, max, _ft);
		}
	}
	return;

}

void encryption::RecursiveLogicCone(CONE* _cone, NODE* _node, FType _ft){	
	if(_ft == FType::AND){ //AND
		for(auto p :_node->getFI()){
			if(p->getAndC() == 0){
				_cone->insertInput(NODE_Ary[p->getId()]);
				color[p->getId()] = 1;
			}
			else if(p->getAndC() >= 1){
			//	std::cout<<"recursive : "<<p->getName()<<std::endl;
				//std::cout<<( _cone->getInput().size() + p->getFI().size() )<<std::endl;
				if( ( _cone->getInput().size() + p->getFI().size() ) >= constraint){
					_cone->insertInput(NODE_Ary[p->getId()]);
					color[p->getId()] = 1;
					return ;	
				}
				else{
					color[p->getId()] = 1;
					RecursiveLogicCone(_cone, NODE_Ary[p->getId()], _ft);
				}
			}
		}
		return;
	}
	else{ //OR
		for(auto p :_node->getFI()){
			if(p->getOrC() == 0){
				_cone->insertInput(NODE_Ary[p->getId()]);
				hue[p->getId()] = 1;
			}
			else if(p->getOrC() >= 1){
				if( ( _cone->getInput().size() + p->getFI().size() ) >= constraint){
					_cone->insertInput(NODE_Ary[p->getId()]);
					color[p->getId()] = 1;
					return ;	
				}
				else{
					hue[p->getId()] = 1;
					RecursiveLogicCone(_cone, NODE_Ary[p->getId()], _ft);
				}
			}
		}
		return;
	}
}

void encryption::observability(){
	for(auto p :NODE_Ary){
		int a = 0;
		int b = 0;
		if(p->getType() == Type::PO){
			p->setCO(0);
		}
		switch(p->getFtype())
		{
			case FType::AND   	:  
				for(auto q: p->getFI()){
					a = 0;
					for(auto z: p->getFI()){
						if(q->getName() != z->getName()){
							a += z->getCC1();
						}
					}
					q->setCO( p->getCO() + a + 1);
				}
				break;
			case FType::OR 		:  
				for(auto q: p->getFI()){
					a = 0;
					for(auto z: p->getFI()){
						if(q->getName() != z->getName()){
							a += z->getCC0();
						}
					}
					q->setCO( p->getCO() + a + 1);
				}
				break;
			case FType::NOR 	:  
				for(auto q: p->getFI()){
					a = 0;
					for(auto z: p->getFI()){
						if(q->getName() != z->getName()){
							a += z->getCC0();
						}
					}
					q->setCO( p->getCO() + a + 1);
				}
				break;
			case FType::NAND 	:  
				for(auto q: p->getFI()){
					a = 0;
					for(auto z: p->getFI()){
						if(q->getName() != z->getName()){
							a += z->getCC1();
						}
					}
					q->setCO( p->getCO() + a + 1);
				}
				break;
			case FType::NOT 	: 
					if(p->getFI().size() == 0)
						break;
				p->getFI()[0]->setCO( p->getCO() +1);
				break;
			case FType::XNOR 	:
					if(p->getFI().size() == 0)
						break;
					else if(p->getFI().size() == 1){
						a = p->getFI()[0]->getCC0() + p->getFI()[0]->getCC1();
						p->setCC0(a+1);
						break;
					}
				a = p->getFI()[1]->getCC0();
				b = p->getFI()[1]->getCC1();
				p->getFI()[0]->setCO(std::min(a, b)+1+p->getCO());	
				a = p->getFI()[0]->getCC0();
				b = p->getFI()[0]->getCC1();
				p->getFI()[1]->setCO(std::min(a, b)+1+p->getCO());	
				break;
			case FType::XOR 	:  
					if(p->getFI().size() == 0)
						break;
					else if(p->getFI().size() == 1){
						a = p->getFI()[0]->getCC0() + p->getFI()[0]->getCC1();
						p->setCC0(a+1);
						break;
					}
				a = p->getFI()[1]->getCC0();
				b = p->getFI()[1]->getCC1();
				p->getFI()[0]->setCO(std::min(a, b)+1+p->getCO());	
				a = p->getFI()[0]->getCC0();
				b = p->getFI()[0]->getCC1();
				p->getFI()[1]->setCO(std::min(a, b)+1+p->getCO());	
				break;
			case FType::BUF 	: 
					if(p->getFI().size() == 0)
						break;
				if(p->getType() != Type::PI)
					p->getFI()[0]->setCO( p->getCO() +1);
				break;
			default    			:
				continue;
		}
	}
}

void encryption::Flogic_cone(){
	
	//reverse topological sort
	std::reverse(NODE_Ary.begin(), NODE_Ary.end());
	// make observility
	observability();

	/*for(auto p :NODE_Ary){
		std::cout<<p<<std::endl;
	}*/
	hue.resize(NODE_Ary.size());
	std::fill(color.begin(), color.end(), 0);
	std::fill(hue.begin(), hue.end(), 0);
	
	int counc =0;
	for(auto p : NODE_Ary){
		p->setId(counc++); //set ID
	}
	
	for(auto p: NODE_Ary){
		if( color[p->getId()] == 0&& p->getFtype()== FType::AND){
			color[p->getId()] = 1;
			CONE* c =new CONE(p->getFtype(), NODE_Ary[p->getId()]);
			RecursiveLogicCone(c, NODE_Ary[p->getId()], p->getFtype());
			LogicCone.push_back(c);
		}
		else if( hue[p->getId()] == 0&& p->getFtype()== FType::OR){
			hue[p->getId()] = 1;
			CONE* c =new CONE(p->getFtype(), NODE_Ary[p->getId()]);
			RecursiveLogicCone(c, NODE_Ary[p->getId()], p->getFtype());
			LogicCone.push_back(c);
		}
	}

	std::sort(LogicCone.begin(), LogicCone.end(), compareCone);


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
			area +=1;
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
			area +=1;
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
	area +=	4*_cone->getInput().size();
	
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
			if(p->getEncType() == FType::XOR)
				area -= 3;
			else
				area -= 2; 
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
	area +=	4*_cone->getInput().size();
	
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
			if(p->getEncType() == FType::XOR)
				area -= 3;
			else
				area -= 2; 
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

void encryption::tree_encryption(){
	//unuse = 0, used = 1;
	threshold = 0.0;
	std::vector<int>is_use;
	is_use.resize(LogicCone.size());
	std::fill(is_use.begin(), is_use.end(), 0); //reset


	for(size_t i=0; i<LogicCone.size();i++){
		if(is_use[i]==0){
			for(size_t j=0; j<LogicCone.size();j++){
				if(i!=j && is_use[i]==0 && is_use[j]==0){
					auto pos = LogicCone[i]->getInput().find(LogicCone[j]->getOutput());
					if(pos != LogicCone[i]->getInput().end()){
						is_use[j]=1;
					}
					else{
						auto pos2 = LogicCone[j]->getInput().find(LogicCone[i]->getOutput());
						if(pos2 != LogicCone[j]->getInput().end()){
							is_use[i]=1;
						}
					}
				}
			}
		}
	}

	//intersection count

	int orig = surplus_area;
	
	std::vector<CONE*>already_encry;
	already_encry.clear();
	while(surplus_area>8){
		int last_area = area;
		int count=0;
		bool isf = false;
		for(auto p :LogicCone){
			if(is_use[count]==0){
	//			std::cout<<"need encryption size : "<< p->getInput().size()*3<<std::endl;
				
				int expected = p->IntersectionCone(already_encry).size();

				if((p->getInput().size()-expected)*4 > surplus_area){
					is_use[count] = 1;
				}
				else {
					isf = true;
					is_use[count] = 1;
					already_encry.push_back(p);
					if(p->getFtype() == FType::AND){
						AND_encryption(p);
					}
					else{
						OR_encryption(p);
					}
					//std::cout<<"after encryption: "<<area<<std::endl;
					//update intersectionC

					break;
				}
			}
		
			count++;
		}
		
		//update
		surplus_area -= (area - last_area);

		threshold *= (double)surplus_area/(double)orig ;
		if(threshold < 0.1)
			threshold = 0;
		//std::cout<<"threshold: "<<threshold<<std::endl;
		//std::cout <<"remading_area :" <<surplus_area << std::endl;
		if(isf == false)
			break;
	}
			
//		std::cout <<"after logic enc ="<<area<<"remading_area :" <<surplus_area << std::endl;
	
		std::sort(NODE_Ary.begin(), NODE_Ary.end(),compareCO);
	//	std::sort(AND_Ary.begin(), AND_Ary.end(),compareCO);
	//	std::sort(OR_Ary.begin(), OR_Ary.end(),compareCO);
		int c=0;
		for(auto p:NODE_Ary){
			p->setId(c++);
		}
	
		for(auto p:NODE_Ary){
			if(p->getType() == Type::Intl && p->isEncryption()==false){
				double  randnum = (double)rand()/RAND_MAX ;
			//	std::cout<<randnum<<std::endl;
				if(surplus_area >=3){
					if(randnum >0.5){
						ezXorenc(p);	
					}
					else {
						ezXorenc(p);
					}
					area+=3;
					surplus_area -=3;
				}
				else if(surplus_area <=0){
					break;
				}
				else{
					if(randnum >0.5){
						ezAndenc(p);	
					}
					else {
						ezOrenc(p);
					}
					
					area+=1;
					surplus_area -=1;
				}
			}
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
