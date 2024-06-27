#include "encryption.h"
#include "node.h"
int main(int argc, char* argv[])
{
    std::srand((unsigned)time(NULL));
    encryption* enc = new encryption();
    if (argc == 3)
        enc->setOutputname(argv[2]);
    enc->readfile(argv[1]);
    enc->setDebugMode(false);
    enc->topological_sort();
    enc->key_ratio = 1; // ratio of number of key bits
    enc->setDebugMode(1);
    // enc->sl_one_encryption();
    // enc->sl_compare_encryption();
    enc->sl_brute_encryption();
    enc->outputfile();
    delete enc;
    return 0;
}
