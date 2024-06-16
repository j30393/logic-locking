#include "./inc/node.h"
#include "./inc/encryption.h"
int main(int argc, char* argv[]){

	#ifndef debug
		std::cout<<"debug\n";	
		FType a = FType::XNOR;
		FType b = FType::XOR;
		FType c = FType::NOR;
		std::cout<<(a==b)<<std::endl;
		std::cout<<cost(c,100)<<std::endl;
		
		NODE *A =new NODE();
		NODE *B =new NODE();
		NODE *C =new NODE();
		std::vector<NODE*>ary;
		ary.push_back(A);
		A->setName("A");
		ary.push_back(B);
		B->setName("B");
		ary.push_back(C);
		C->setName("C");

		ary[1]->insertFO(C);
		ary[1]->insertFO(A);
		std::cout<<"Index="<<ary[1]->FOfind(A)<<std::endl;
		std::cout<<(A==ary[0])<<std::endl;
		std::set<NODE*>S;
		std::set<NODE*>SV;
		std::set<NODE*>U;
		std::set<NODE*>B;
		NODE* node1 =new NODE(Type::PI, FType::BUF, "dog1");
		NODE* node2 =new NODE(Type::PI, FType::BUF, "dog2");
		NODE* node3 =new NODE(Type::PI, FType::BUF, "dog3");
		NODE* node4 =new NODE(Type::PI, FType::BUF, "dog4");
		NODE* node5 =new NODE(Type::PI, FType::BUF, "dog5");

		S.insert(node1);
		S.insert(node2);
		S.insert(node3);
		S.insert(node4);
		S.insert(node1);

		B.insert(node1);
		B.insert(node2);
		B.insert(node3);
		B.insert(node4);

		SV.insert(node1);
		SV.insert(node2);

		std::cout<<(B==S)<<std::endl;
		std::set_intersection(S.begin(), S.end(), SV.begin(), SV.end(), std::inserter(U, U.begin()));
		for(auto p:SV){
			std::cout<<p->getName()<<std::endl;
		}
	#endif
		std::srand((unsigned)time( NULL ) );
		encryption* enc = new encryption();
		if(argc == 3)
			enc->setOutputname(argv[2]);
		enc->readfile(argv[1]);
		enc->topological_sort();
		enc->Flogic_cone();
		enc->tree_encryption();
		enc->outputfile();
	return 0;
}
