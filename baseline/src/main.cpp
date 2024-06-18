#include "./inc/node.h"
#include "./inc/encryption.h"
int main(int argc, char* argv[]){
	std::srand((unsigned)9487);
	encryption* enc = new encryption();
	if(argc == 3)
		enc->setOutputname(argv[2]);
	enc->readfile(argv[1]);
	enc->setDebugMode(false);
	enc->set_test_num(5);
	enc->topological_sort();
	enc->fault_impact_cal();
	enc->key_ratio = 0.2;
	enc->xor_encryption();
	enc->outputfile();
	return 0;
}
