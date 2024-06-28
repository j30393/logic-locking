#include "encryption.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>

encryption::encryption()
{
    twolevelfile = false;
    keyCount = 0;
}

encryption::~encryption()
{
    for (auto p : NODE_Ary) {
        delete p;
    }
    for (auto p : KEY_Ary) {
        delete p;
    }
    for (auto p : ENCY_Ary) {
        delete p;
    }
}

std::vector<NODE*> getRandomKNodes(const std::vector<NODE*>& nodes, size_t k)
{
    // Copy the nodes to a new vector to avoid modifying the original
    std::vector<NODE*> shuffled_nodes = nodes;

    std::vector<NODE*> random_nodes;
    random_nodes.reserve(k);

    std::random_shuffle(shuffled_nodes.begin(), shuffled_nodes.end());

    for (auto it : shuffled_nodes) {
        // Determine whether the node is not on a critical path
        // TODO: replace 1 with the delay of 2-input XOR or XNOR?
        if (it->RAT - it->AT >= 1) {
            random_nodes.push_back(it);
            if (!--k)
                break;
        }
    }

    return random_nodes;
}

// xor encryption
void encryption::xor_encryption()
{
    int total_enc_num = std::min(static_cast<int>(NODE_Ary.size()), 128);
    total_enc_num = ceil(this->key_ratio * total_enc_num);
    std::vector<NODE*> enc_nodes = getRandomKNodes(NODE_Ary, total_enc_num);
    total_enc_num = std::min(total_enc_num, static_cast<int>(enc_nodes.size()));
    assert(total_enc_num > 0);
    if (is_debug) {
        std::cout << "encryption a total of " << total_enc_num << " nodes" << std::endl;
    }
    std::vector<bool> key_arr(total_enc_num, false);
    for (auto&& key_element : key_arr) {
        key_element = rand() % 2;
        if (key_element == 0) {
            key += "0";
        } else {
            key += "1";
        }
    }

    if (is_debug) {
        std::cout << "encryption key_arr: ";
        for (auto key_element : key_arr) {
            std::cout << key_element << " ";
        }
    }

    for (int i = 0; i < total_enc_num; i++) {
        NODE* enc_node = enc_nodes[i];
        if (key_arr[i] == 0) {
            // XOR gate
            NODE* key_node = new NODE(Type::PRIMARY_INPUT, FType::BUF, "keyinput" + std::to_string(i));
            assert(name2node.find(key_node->name) == name2node.end());
            KEY_Ary.push_back(key_node);
            name2node[key_node->name] = key_node;
            NODE* xor_node = new NODE(Type::INTERNAL, FType::XOR, "xor" + std::to_string(i));
            ENCY_Ary.push_back(xor_node);
            xor_node->insertFI(enc_node);
            xor_node->insertFI(key_node);
            enc_node->enc = true;
            // if we are operating on the output node
            if (enc_node->getFO().empty()) {
                xor_node->name = enc_node->name;
                // ptw = plaintext wire
                enc_node->name = enc_node->name + "$ptw";
                assert(name2node.find(enc_node->name) == name2node.end());
                name2node[xor_node->name] = xor_node;
                name2node[enc_node->name] = enc_node;
                *std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = xor_node;
            } else { // none output node
                assert(name2node.find(xor_node->name) == name2node.end());
                name2node[xor_node->name] = xor_node;
                for (auto fan_out_node : enc_node->getFO()) {
                    fan_out_node->insertFI(xor_node);
                    fan_out_node->eraseFI(enc_node);
                    xor_node->insertFO(fan_out_node);
                }
                enc_node->clearFO();
            }
            enc_node->insertFO(xor_node);
            key_node->insertFO(xor_node);
        } else {
            // XNOR gate
            NODE* key_node = new NODE(Type::PRIMARY_INPUT, FType::BUF, "keyinput" + std::to_string(i));
            assert(name2node.find(key_node->name) == name2node.end());
            KEY_Ary.push_back(key_node);
            name2node[key_node->name] = key_node;
            NODE* xnor_node = new NODE(Type::INTERNAL, FType::XNOR, "xnor" + std::to_string(i));
            ENCY_Ary.push_back(xnor_node);
            xnor_node->insertFI(enc_node);
            xnor_node->insertFI(key_node);
            enc_node->enc = true;
            // if we are operating on the output node
            if (enc_node->getFO().empty()) {
                xnor_node->name = enc_node->name;
                // ptw = plaintext wire
                enc_node->name = enc_node->name + "$ptw";
                assert(name2node.find(enc_node->name) == name2node.end());
                name2node[xnor_node->name] = xnor_node;
                name2node[enc_node->name] = enc_node;
                *std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = xnor_node;
            } else { // none output node
                assert(name2node.find(xnor_node->name) == name2node.end());
                name2node[xnor_node->name] = xnor_node;
                for (auto fan_out_node : enc_node->getFO()) {
                    fan_out_node->insertFI(xnor_node);
                    fan_out_node->eraseFI(enc_node);
                    xnor_node->insertFO(fan_out_node);
                }
                enc_node->clearFO();
            }
            enc_node->insertFO(xnor_node);
            key_node->insertFO(xnor_node);
        }
    }
}

void encryption::setOutputname(std::string _name)
{
    twolevelfile = true;
    outputname = _name;
}

void encryption::readfile(std::string _filename)
{
    filename = _filename;

    std::fstream input;
    input.open(_filename, std::ios::in);

    std::string buffer;

    int count = 0;
    while (std::getline(input, buffer)) {
        std::string checkerflag = "";
        checkerflag.assign(buffer, 0, 2);
        if (buffer.empty()) {
            continue;
        }
        // annotation
        if (buffer[0] == '#') {
            continue;
        }
        if (checkerflag == "IN") { // input
            std::string name = "";
            name.assign(buffer, 6, buffer.size() - 7);

            Type t = Type::PRIMARY_INPUT;
            FType ft = FType::BUF;

            NODE* n;
            auto it = name2node.find(name);
            if (it == name2node.end()) {
                n = new NODE(t, ft, name);
                n->id = count++; // set ID
                n->depth = 0; // set depth
                NODE_Ary.push_back(n);
                name2node[name] = n;
            } else {
                n = it->second;
                n->ft = ft;
                n->t = t;
            }
            // Input push in
            PI_Ary.push_back(n);
        } else if (checkerflag == "OU") {
            std::string name = "";
            name.assign(buffer, 7, buffer.size() - 8);

            Type t = Type::PRIMARY_OUTPUT;
            FType ft = FType::BUF;

            NODE* n;
            auto it = name2node.find(name);
            if (it == name2node.end()) {
                n = new NODE(t, ft, name);
                n->id = count++; // set ID
                NODE_Ary.push_back(n);
                name2node[name] = n;
            } else {
                n = it->second;
                if (it->second->t != Type::PRIMARY_INPUT)
                    n->t = t;
            }
            // push in
            PO_Ary.push_back(n);
        } else {
            Type t = Type::INTERNAL; // set type
            FType ft;

            std::string tem_buf = "";
            std::stringstream ss;
            ss << buffer;

            // name
            std::string name = "";
            ss >> name;
            // = kill
            ss >> tem_buf >> std::ws;

            // get Ftype
            std::string ft_name = "";
            std::getline(ss, ft_name, '(');
            ss >> tem_buf;

            std::transform(ft_name.begin(), ft_name.end(), ft_name.begin(), tolower);
            if (ft_name == "not") {
                ft = FType::NOT;
            } else if (ft_name == "buf") {
                ft = FType::BUF;
            } else if (ft_name == "and") {
                ft = FType::AND;
            } else if (ft_name == "xor") {
                ft = FType::XOR;
            } else if (ft_name == "xnor") {
                ft = FType::XNOR;
            } else if (ft_name == "nand") {
                ft = FType::NAND;
            } else if (ft_name == "nor") {
                ft = FType::NOR;
            } else if (ft_name == "or") {
                ft = FType::OR;
            } else {
                continue;
            }

            // create node & push
            auto it = name2node.find(name);
            NODE* n;
            if (it == name2node.end()) {
                n = new NODE(t, ft, name);
                n->id = count++; // set ID
                NODE_Ary.push_back(n);
                name2node[name] = n;
            } else {
                n = it->second;
                assert(n->ft == FType::BUF);
                n->ft = ft;
            }

            do {
                // erase unused char
                if (tem_buf[0] == '(')
                    tem_buf.erase(tem_buf.begin());
                if (tem_buf[0] == ',')
                    tem_buf.erase(tem_buf.begin());
                if (tem_buf[tem_buf.size() - 1] == ')')
                    tem_buf.pop_back();
                if (tem_buf[tem_buf.size() - 1] == ',')
                    tem_buf.pop_back();

                NODE* tem_n;
                it = name2node.find(tem_buf);
                if (it == name2node.end()) {
                    tem_n = new NODE(t, FType::BUF, tem_buf);
                    tem_n->id = count++; // set ID
                    NODE_Ary.push_back(tem_n);
                    name2node[tem_buf] = tem_n;
                } else {
                    tem_n = it->second;
                }
                // fan out
                tem_n->insertFO(n);
                // fan in
                n->insertFI(tem_n);
            } while (ss >> tem_buf);
        }
    }
    input.close();
}

std::ostream& operator<<(std::ostream& os, NODE* p)
{
    os << "-------------------------------------------------------\n";
    os << "name: " << p->name << std::endl;
    os << "Gate type: " << p->ft << std::endl;
    os << "Type: " << p->t << std::endl;
    os << "ID: " << p->id << std::endl;
    os << "FI node :";
    for (auto q : p->getFI()) {
        os << " " << q->name;
    }
    os << "\nFO node :";
    for (auto q : p->getFO()) {
        os << " " << q->name;
    }
    os << "\ndepth " << p->depth << std::endl;

    os << "\n-------------------------------------------------------\n";
    return os;
}

/**
 * Topological sort for the graph using kahn's algorithm
 * https://www.geeksforgeeks.org/topological-sorting-indegree-based-solution/
 *
 * @param none
 * @return none but finish the depth information for that graph (depth...etc)
 */
void encryption::topological_sort()
{

    visited.resize(NODE_Ary.size());
    in_degree.resize(NODE_Ary.size());
    std::fill(visited.begin(), visited.end(), 0);
    std::fill(in_degree.begin(), in_degree.end(), 0);

    for (auto p : NODE_Ary) {
        for (auto q : p->getFO()) {
            in_degree[q->id]++;
        }
    }

    std::queue<NODE*> q;

    for (auto p : NODE_Ary) {
        if (in_degree[p->id] == 0) {
            q.push(p);
            p->depth = 0;
            p->AT = 0;
        }
    }

    std::vector<NODE*> result;

    while (!q.empty()) {
        NODE* node = q.front();
        q.pop();
        if (node->depth == -0xfffffff) {
            int depth = -1, AT = -1;
            for (auto children : node->getFI()) {
                depth = std::max(depth, children->depth);
                AT = std::max(AT, children->AT);
            }
            node->depth = depth + 1;
            node->AT = AT + node->getGateDelay();
        }
        result.push_back(node);

        // Decrease indegree of adjacent vertices as the
        // current node is in topological order
        for (auto it : node->getFO()) {
            in_degree[it->id]--;

            // If indegree becomes 0, push it to the queue
            if (in_degree[it->id] == 0)
                q.push(it);
        }
    }

    // Check for cycle
    assert(result.size() == NODE_Ary.size());
    NODE_Ary = std::move(result);

    if (this->is_debug) {
        for (auto it : NODE_Ary) {
            std::cout << "name : " << it->name << " depth : " << it->depth << std::endl;
            for (auto child : it->getFI()) {
                std::cout << "child : " << child->name << " depth : " << child->depth << std::endl;
            }
        }
        std::cout << "\n\n\n\n";
    }

    // Compute the RAT in reverse topological order
    int critical_path = 0;
    for (auto it : PO_Ary)
        critical_path = std::max(critical_path, it->AT);
    for (auto p = NODE_Ary.rbegin(); p != NODE_Ary.rend(); ++p) {
        int RAT = critical_path;
        for (auto it : (*p)->getFO())
            RAT = std::min(RAT, it->RAT - it->getGateDelay());
        (*p)->RAT = RAT;
    }
}

void encryption::outputfile()
{
    std::fstream out;
    std::string fname = "";

    if (twolevelfile)
        fname = outputname;
    else {
        fname = filename;
        fname.erase(fname.end() - 6, fname.end());
        fname += "ENC.bench";
    }
    out.open(fname, std::ios::out);

    out << "# key=" << key << std::endl;
    // INPUT
    for (auto p : PI_Ary) {
        std::string tem = "INPUT(";
        tem += p->name;
        tem += ")";
        out << tem << std::endl;
    }

    // OUTPUT
    for (auto p : PO_Ary) {
        std::string tem = "OUTPUT(";
        tem += p->name;
        tem += ")";
        out << tem << std::endl;
    }

    // OUTkey
    for (auto p : KEY_Ary) {
        std::string tem = "INPUT(";
        tem += p->name;
        tem += ")";
        out << tem << std::endl;
    }

    // original circuit
    for (auto p : NODE_Ary) {
        std::string tem = "";
        if (p->t == Type::INTERNAL || p->t == Type::PRIMARY_OUTPUT) {
            tem = p->name;
            tem += " = ";
            tem += p->stringFType();
            tem += "(";

            for (size_t i = 0; i < p->getFI().size(); i++) {
                tem += p->getFI()[i]->name;
                if (i + 1 == p->getFI().size()) {
                    tem += ")";
                } else
                    tem += ", ";
            }
            out << tem << std::endl;
        }
    }
    // encry circuit
    for (auto p : ENCY_Ary) {
        std::string tem = "";
        tem = p->name;
        tem += " = ";
        tem += p->stringFType();
        tem += "(";

        for (size_t i = 0; i < p->getFI().size(); i++) {
            tem += p->getFI()[i]->name;
            if (i + 1 == p->getFI().size()) {
                tem += ")";
            } else
                tem += ", ";
        }
        out << tem << std::endl;
    }
    std::cout << key << std::endl;
    out.close();
}
