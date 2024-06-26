#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
int main(int argc, char* argv[])
{
    if (argc <= 1) {
        std::cerr << "Usage: " << argv[0] << " input.bench\n";
        return 1;
    }

    std::string module_name = argv[1];
    while (module_name.back() != '.')
        module_name.pop_back();
    module_name.pop_back();

    std::string::size_type z = module_name.rfind('/');
    if (z != std::string::npos)
        module_name = module_name.substr(z + 1);

    std::ifstream bench(argv[1]);
    std::string buffer, name, verilog_line;
    std::vector<std::string> IO_ports, wires, verilog_lines;
    std::unordered_set<std::string> output_set;

    while (getline(bench, buffer)) {
        if (buffer.empty() || buffer[0] == '#')
            continue;
        if (buffer.substr(0, 5) == "INPUT") {
            name = buffer.substr(6, buffer.size() - 7);
            IO_ports.emplace_back(name);
        } else if (buffer.substr(0, 6) == "OUTPUT") {
            name = buffer.substr(7, buffer.size() - 8);
            IO_ports.emplace_back(name);
            output_set.emplace(name);
        } else {
            std::istringstream ss(buffer);
            std::string tem_buf, ft_name;
            ss >> name >> tem_buf >> std::ws;
            assert(tem_buf == "=");

            // Get gate type
            std::getline(ss, tem_buf, '(');
            std::transform(
                tem_buf.begin(), tem_buf.end(), tem_buf.begin(), tolower);
            // Assumption: no wire name starts with new_
            verilog_line = "    " + tem_buf + " new_" + name
                + "(" + name + ", ";
            std::getline(ss, tem_buf, ')');
            verilog_line += tem_buf + ");\n";

            verilog_lines.emplace_back(verilog_line);
            if (output_set.find(name) == output_set.cend())
                wires.emplace_back(name);
        }
    }
    bench.close();

    std::ofstream verilog(module_name + ".v");
    verilog << "module " << module_name << "(\n";
    verilog_line = "   ";
    for (const std::string& IO_port_name : IO_ports) {
        if (verilog_line.size() + IO_port_name.size() > 78) {
            verilog << verilog_line << '\n';
            verilog_line = "   ";
        }
        verilog_line += ' ' + IO_port_name + ',';
    }
    assert(verilog_line.back() == ',');
    verilog_line.back() = '\n';
    verilog << verilog_line << ");\n";

    verilog_line = "    input";
    for (const std::string& IO_port_name : IO_ports) {
        if (output_set.find(IO_port_name) == output_set.cend()) {
            if (verilog_line.size() + IO_port_name.size() > 78) {
                verilog << verilog_line << '\n';
                verilog_line = "         ";
            }
            verilog_line += ' ' + IO_port_name + ',';
        }
    }
    assert(verilog_line.back() == ',');
    verilog_line.back() = ';';
    verilog << verilog_line << '\n';

    verilog_line = "    output";
    for (const std::string& IO_port_name : IO_ports) {
        if (output_set.find(IO_port_name) != output_set.cend()) {
            if (verilog_line.size() + IO_port_name.size() > 78) {
                verilog << verilog_line << '\n';
                verilog_line = "          ";
            }
            verilog_line += ' ' + IO_port_name + ',';
        }
    }
    assert(verilog_line.back() == ',');
    verilog_line.back() = ';';
    verilog << verilog_line << '\n';

    verilog_line = "    wire";
    for (const std::string& wire_name : wires) {
        if (verilog_line.size() + wire_name.size() > 78) {
            verilog << verilog_line << '\n';
            verilog_line = "        ";
        }
        verilog_line += ' ' + wire_name + ',';
    }
    assert(verilog_line.back() == ',');
    verilog_line.back() = ';';
    verilog << verilog_line << "\n\n";

    for (const std::string& line : verilog_lines)
        verilog << line;
    verilog << "endmodule\n";

    return 0;
}
