#include"./inc/node.h"

std::ostream& operator<<(std::ostream& os, Type _t)
{
    switch(_t)
    {
		case Type::Intl		: os << "Intl";   break;
		case Type::PI 		: os << "PI"; 	  break;
		case Type::PO 		: os << "PO";  	  break;
        default    			: os.setstate(std::ios_base::failbit);
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, FType _ft)
{
    switch(_ft)
    {
		case FType::AND   	: os << "AND";  	  break;
		case FType::OR 		: os << "OR"; 		  break;
		case FType::NOR 	: os << "NOR"; 	 	  break;
		case FType::NAND 	: os << "NAND";  	  break;
		case FType::NOT 	: os << "NOT"; 	 	  break;
		case FType::XNOR 	: os << "XNOR"; 	  break;
		case FType::XOR 	: os << "XOR";  	  break;
		case FType::BUF 	: os << "BUF";  	  break;
        default    			: os.setstate(std::ios_base::failbit);
    }
    return os;
}

const std::string NODE::stringFType(){
	std::string r ="";
	switch(ft)
    {
		case FType::AND   	:  r = "AND";  		  break;
		case FType::OR 		:  r = "OR"; 		  break;
		case FType::NOR 	:  r = "NOR"; 	 	  break;
		case FType::NAND 	:  r = "NAND";  	  break;
		case FType::NOT 	:  r = "NOT"; 	 	  break;
		case FType::XNOR 	:  r = "XNOR"; 		  break;
		case FType::XOR 	:  r = "XOR";  		  break;
		case FType::BUF 	:  r = "BUF";  		  break;
		default    			:  r = "BUF";
    }
	return r;
}


const int NODE::FIfind(NODE* _node){
	int return_index = -1;
	for(auto p : FI_Ary){
		if(*p == _node){
			return ++return_index;
		}
		++return_index;
	}
	return return_index;
}

const int NODE::FOfind(NODE* _node){
	int return_index = -1;
	for(auto p : FO_Ary){
		if(*p == _node){
			return ++return_index;
		}
		++return_index;
	}
	return return_index;
}

void NODE::eraseFI(NODE* _node){
	FI_Ary.erase( FI_Ary.begin() + FIfind(_node) );	
}

void NODE::eraseFO(NODE* _node){
	FO_Ary.erase( FO_Ary.begin() + FOfind(_node) );	
}

bool compareCO(NODE* _node1, NODE *_node2){
	return _node1->getCO() > _node2->getCO();
}


bool NODE::path3length(NODE * _node, int _count){
	if(_count >=2)
		return false;
	if( FO_Ary.size() <= 0)
		return false;
	else{
		bool r=false;
		for(auto p : FO_Ary){
			if(p == _node){
				r = true;
				break;
			}
			else{
				r =p->path3length(_node, _count+1);
				if(r == true){
				 break;
				}
			}
		}
		return r;
	}
}


bool NODE::calculateValue() {
	if(current_output == 0 || current_output == 1) return current_output;
	if(t == Type::PI) {
		current_output = rand() % 2;
		return current_output;
	}
	bool ans;
	FType cur = ft;
	if(stuck_fault_value) {
		switch(ft) {
			case(FType::AND):{
				cur = FType::NAND;
				break;
			}
			case(FType::OR):{
				cur = FType::NOR;
				break;
			}
			case(FType::NAND):{
				cur = FType::AND;
				break;
			}
			case(FType::NOR):{
				cur = FType::OR;
				break;
			}
			case(FType::XOR):{
				cur = FType::XNOR;
				break;
			}
			case(FType::XNOR):{
				cur = FType::XOR;
				break;
			}
			case(FType::NOT):{
				cur = FType::BUF;
				break;
			}
			case(FType::BUF):{
				cur = FType::NOT;
				break;
			}
			default:{
				std::cout << "error: unexpected type switch in node.cpp\n";
			}
		}
	}

	switch(cur){
		case(FType::AND):{
			ans = 1;
			for(auto p : FI_Ary){
				ans = ans & p->getCurrentOutput();
			}
			break;
		}
		case(FType::OR):{
			ans = 0;
			for(auto p : FI_Ary){
				ans = ans | p->getCurrentOutput();
			}
			break;
		}
		case(FType::NAND):{
			ans = 1;
			for(auto p : FI_Ary){
				ans = ans & p->getCurrentOutput();
			}
			ans = !ans;
			break;
		}
		case(FType::NOR):{
			ans = 0;
			for(auto p : FI_Ary){
				ans = ans | p->getCurrentOutput();
			}
			ans = !ans;
			break;
		}
		case(FType::XOR):{
			ans = 0;
			for(auto p : FI_Ary){
				ans = ans ^ p->getCurrentOutput();
			}
			break;
		}
		case(FType::XNOR):{
			ans = 0;
			for(auto p : FI_Ary){
				ans = ans ^ p->getCurrentOutput();
			}
			ans = !ans;
			break;
		}
		case(FType::NOT):{
			ans = !FI_Ary[0]->getCurrentOutput();
			break;
		}
		case(FType::BUF):{
			ans = FI_Ary[0]->getCurrentOutput();
			break;
		}
		default:{
			std::cout << "error: unexpected type in node.cpp\n";
		}
	}

	return current_output = ans;
}