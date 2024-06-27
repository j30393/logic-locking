#include "encryption.h"
#include "node.h"
int main(int argc, char* argv[])
{
    std::srand(static_cast<unsigned>(time(NULL)));
    encryption* enc = new encryption();
    if (argc == 3)
        enc->setOutputname(argv[2]);
    enc->readfile(argv[1]);
    enc->setDebugMode(false);
    enc->topological_sort();
    enc->key_ratio = 0.0078125; // ratio of number of key bits
    enc->xor_encryption();
    enc->outputfile();
    delete enc;
    return 0;
}
