#include <fstream>
#include <iostream>
#include <string>
int main(int argc, char* argv[])
{
    unsigned keys, gates;
    int cube_count;
    double cpu_time, denominator;
    std::ios_base::sync_with_stdio(false);
    if (argc <= 1) {
        std::cerr << "Usage: " << argv[0] << " sld.log\n";
        return 1;
    }

    std::ifstream sld(argv[1]);
    std::string s;
    while (sld >> s) {
        if (s.substr(0, 5) == "keys=")
            keys = stoul(s.substr(5));
        else if (s.substr(0, 6) == "gates=")
            gates = stoul(s.substr(6));
        else if (s.back() == ';') {
            s.pop_back();
            if (s.substr(0, 11) == "cube_count=")
                cube_count = stoi(s.substr(11));
            else if (s.substr(0, 9) == "cpu_time=")
                cpu_time = stod(s.substr(9));
        }
    }

    denominator = static_cast<double>(keys) * gates;
    std::cout << "Alpha part = " << cube_count / denominator << '\n';
    std::cout << "Beta part = " << cpu_time / denominator << std::endl;
    return 0;
}
