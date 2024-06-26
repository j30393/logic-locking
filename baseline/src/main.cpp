#include "./inc/node.h"
#include "./inc/encryption.h"
int main(int argc, char* argv[]){
	std::srand(static_cast<unsigned>(5678));
	encryption* enc = new encryption();
	if(argc == 3)
		enc->setOutputname(argv[2]);
	enc->readfile(argv[1]);
	enc->setDebugMode(false);
	enc->set_test_num(100); // number of testcase for fault impact 
	enc->topological_sort();
	enc->fault_impact_cal();
	enc->key_ratio = 0.01; // ratio of number of key bits
	enc->xor_encryption();
	// enc->masking();
	enc->outputfile();
	delete enc;
	return 0;
}
