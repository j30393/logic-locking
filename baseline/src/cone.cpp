#include"./inc/cone.h"


void CONE::eraseInput(NODE* _node){
	std::set<NODE*>::iterator it;
	it = input.find(_node);
	input.erase(it);
}

int CONE::IntersectionSize(CONE* _cone){

	std::set<NODE*>U;
	U.clear();
	std::set_intersection(input.begin(), input.end(), _cone->getInput().begin(), _cone->getInput().end(), std::inserter(U, U.begin()));
	return U.size();
}
std::set<NODE*>	CONE::IntersectionCone(std::vector<CONE*>_vec){
	std::set<NODE*>R;
	R.clear();
	for(auto p: _vec){
		std::set<NODE*>U;
		U.clear();
		std::set_intersection(input.begin(), input.end(), p->getInput().begin(), p->getInput().end(), std::inserter(U, U.begin()));
		std::set_union(R.begin(), R.end(), U.begin(), U.end(), std::inserter(R, R.begin()));
		
	}
	return R;

}


bool CONE::operator ==(CONE* _C){
	if( (_C->getInput() == input) && (_C->getOutput() == output) )
		return true;
	else
		return false;
}

std::ostream& operator<<(std::ostream& os, CONE* _cone){
	os<<"["<<_cone->getFtype()<<"]"<<_cone->output->getName()<<" -> ";
	for(auto p: _cone->getInput()){
		os<<p->getName()<<" ";
	}
	os<<"\n";

	return os;

}


bool compareCone(CONE *_cone1, CONE *_cone2 ){
	
	if(_cone1->getInput().size() == _cone2->getInput().size()){
		return _cone1->getOutput()->getCO() > _cone2->getOutput()->getCO();
	}
	else{
		return _cone1->getInput().size() > _cone2->getInput().size();
	}

}
