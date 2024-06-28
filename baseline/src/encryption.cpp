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
    for (auto p : nodes) {
        delete p;
    }
    for (auto p : key_inputs) {
        delete p;
    }
    for (auto p : encrypted_nodes) {
        delete p;
    }
}

// add solver
std::vector<bool> encryption::solver(std::vector<bool> input)
{
    std::vector<bool> output;
    assert(input.size() == primary_inputs.size());
    for (auto n : nodes) {
        n->setCurrentOutput(2);
    }
    for (int i = 0; i < primary_inputs.size(); i++) {
        if (nodes[i]->is_stuck_faulting) {
            nodes[i]->setCurrentOutput(nodes[i]->stuck_fault_value);
        } else {
            nodes[i]->setCurrentOutput(input[i]);
        }
        output.push_back(nodes[i]->getCurrentOutput());
    }
    // calculate the output
    for (int i = primary_inputs.size(); i < nodes.size(); i++) {
        // n is the current node
        NODE* n = nodes[i];
        int ans = -1;
        for (auto p : n->get_fan_ins()) {
            if (p->getCurrentOutput() == 2) {
                std::cout << "Current node: " << n->name << std::endl;
                std::cout << "fan in name: " << p->name << std::endl;
                std::cout << "error: topology sort failed" << std::endl;
            }
        }
        if (n->is_stuck_faulting) {
            ans = n->stuck_fault_value;
        } else {
            switch (n->gate) {
            case (GateType::AND): {
                ans = 1;
                for (auto p : n->get_fan_ins()) {
                    ans = ans & p->getCurrentOutput();
                }
                break;
            }
            case (GateType::OR): {
                ans = 0;
                for (auto p : n->get_fan_ins()) {
                    ans = ans | p->getCurrentOutput();
                }
                break;
            }
            case (GateType::NAND): {
                ans = 1;
                for (auto p : n->get_fan_ins()) {
                    ans = ans & p->getCurrentOutput();
                }
                ans = !ans;
                break;
            }
            case (GateType::NOR): {
                ans = 0;
                for (auto p : n->get_fan_ins()) {
                    ans = ans | p->getCurrentOutput();
                }
                ans = !ans;
                break;
            }
            case (GateType::XOR): {
                ans = 0;
                for (auto p : n->get_fan_ins()) {
                    ans = ans ^ p->getCurrentOutput();
                }
                break;
            }
            case (GateType::XNOR): {
                ans = 0;
                for (auto p : n->get_fan_ins()) {
                    ans = ans ^ p->getCurrentOutput();
                }
                ans = !ans;
                break;
            }
            case (GateType::NOT): {
                assert(n->get_fan_in_count() == 1);
                ans = !n->get_fan_ins()[0]->getCurrentOutput();
                break;
            }
            case (GateType::BUF): {
                assert(n->get_fan_in_count() == 1);
                ans = n->get_fan_ins()[0]->getCurrentOutput();
                break;
            }
            default: {
                std::cout << "error: unexpected type" << std::endl;
            }
            }
        }
        n->setCurrentOutput(ans);
        output.push_back(ans);
    }
    assert(output.size() == nodes.size());
    return output;
}

std::pair<bool, int> compareAndHammingDistance(const std::vector<bool>& vec1, const std::vector<bool>& vec2)
{
    // Check if the sizes are the same
    if (vec1.size() != vec2.size()) {
        throw std::invalid_argument("Vectors must be of the same length");
    }

    bool areEqual = true;
    int hammingDistance = 0;

    // Compare elements and calculate Hamming distance
    for (size_t i = 0; i < vec1.size(); ++i) {
        if (vec1[i] != vec2[i]) {
            areEqual = false;
            hammingDistance++;
        }
    }

    return std::make_pair(areEqual, hammingDistance);
}

// identify what we want to encrypt
std::vector<NODE*> getTopKNodes(const std::vector<NODE*>& nodes, size_t k)
{
    // Copy the nodes to a new vector to avoid modifying the original
    std::vector<NODE*> sorted_nodes = nodes;

    // Sort the nodes based on fault_impact in descending order
    std::sort(sorted_nodes.begin(), sorted_nodes.end(), [](NODE* a, NODE* b) {
        return a->fault_impact > b->fault_impact;
    });

    // If k is greater than the number of nodes, adjust k
    if (k > sorted_nodes.size()) {
        k = sorted_nodes.size();
    }

    // Select the top k nodes
    std::vector<NODE*> top_k_nodes(sorted_nodes.begin(), sorted_nodes.begin() + k);
    return top_k_nodes;
}

// fault impact calculation
void encryption::fault_impact_cal()
{
    int num_tested = test_num;
    std::vector<bool> test_pattern(primary_inputs.size(), false);
    std::vector<bool> golden_output;
    std::vector<bool> sa0_output;
    std::vector<bool> sa1_output;

    for (int iteration = 0; iteration < num_tested; iteration++) {
        for (int i = 0; i < primary_inputs.size(); i++) {
            test_pattern[i] = rand() % 2;
        }
        golden_output = solver(test_pattern);
        for (auto n : nodes) {
            // stuck at 0
            n->is_stuck_faulting = true;
            n->stuck_fault_value = 0;
            sa0_output = solver(test_pattern);
            std::pair<bool, int> result = compareAndHammingDistance(golden_output, sa0_output);
            if (!result.first) { // difference
                n->NoO0++;
                n->NoP0 += result.second;
            }
            n->is_stuck_faulting = false;

            // stuck at 1
            n->is_stuck_faulting = true;
            n->stuck_fault_value = 1;
            sa1_output = solver(test_pattern);
            result = compareAndHammingDistance(golden_output, sa1_output);
            if (!result.first) { // difference
                n->NoO1++;
                n->NoP1 += result.second;
            }
            n->is_stuck_faulting = false;
        }
    }
    // setDebugMode(true);
    for (auto node : nodes) {
        node->fault_impact = (node->NoO0 * node->NoP0 + node->NoO1 * node->NoP1);
        if (is_debug) {
            std::cout << "Node " << node->name << " fault impact: " << node->fault_impact << std::endl;
        }
    }
    if (is_debug) {
        for (auto node : nodes) {
            std::cout << "Node " << node->name << std::endl;
            std::cout << "NoO0: " << node->NoO0 << " NoP0: " << node->NoP0 << std::endl;
            std::cout << "NoO1: " << node->NoO1 << " NoP1: " << node->NoP1 << std::endl;
        }
    }
    // setDebugMode(false);

    // setDebugMode(true);
    if (is_debug) {
        for (int i = 0; i < nodes.size(); i++) {
            if (i < primary_inputs.size()) {
                std::cout << "Node " << i << " : " << primary_inputs[i]->name << std::endl;
                std::cout << "Test value : " << test_pattern[i] << std::endl;
            } else {
                std::cout << "Node " << i << " : " << nodes[i]->name << std::endl;
                std::cout << "Output value : " << golden_output[i - primary_inputs.size()] << std::endl;
            }
        }
    }
    // setDebugMode(false);
    return;
}

// xor encryption
void encryption::xor_encryption()
{
    int total_enc_num = ceil(this->key_ratio * primary_inputs.size());
    total_enc_num = std::min(total_enc_num, static_cast<int>(nodes.size()));
    assert(total_enc_num <= nodes.size());
    std::vector<NODE*> enc_nodes = getTopKNodes(nodes, total_enc_num);
    total_enc_num = std::min(total_enc_num, static_cast<int>(enc_nodes.size()));
    assert(total_enc_num > 0);
    this->total_key_num = total_enc_num;
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
        std::cout << "encryption key_arr:";
        for (auto key_element : key_arr) {
            std::cout << " " << key_element;
        }
        std::cout << "\n";
    }

    // If the key-bit is '0', then the key-gate
    // structure can be either 'XOR- gate' or ' XNOR- gate + inverter '.
    // Similarly, if the key-bit is '1', then the key-gate
    // structure can be either 'XNOR-gate' or ' XOR- gate + inverter '.

    for (int i = 0; i < total_enc_num; i++) {
        NODE* enc_node = enc_nodes[i];
        if (key_arr[i] == 0) {
            // XOR gate
            NODE* key_node = new NODE(NodeType::PRIMARY_INPUT, GateType::BUF, "keyinput" + std::to_string(i));
            assert(name2node.find(key_node->name) == name2node.end());
            key_inputs.push_back(key_node);
            name2node[key_node->name] = key_node;
            int type = rand() % 2;
            if (type == 1) {
                // xor gate
                NODE* xor_node = new NODE(NodeType::INTERNAL, GateType::XOR, "xor" + std::to_string(i));
                encrypted_nodes.push_back(xor_node);
                xor_node->insert_fan_in(enc_node);
                xor_node->insert_fan_in(key_node);
                enc_node->enc = true;
                // if we are operating on the output node
                if (enc_node->type == NodeType::PRIMARY_OUTPUT) {
                    // change encoded node
                    enc_node->type = NodeType::INTERNAL;
                    xor_node->type = NodeType::PRIMARY_OUTPUT;
                    xor_node->name = enc_node->name;
                    // ptw = plaintext wire
                    enc_node->name = enc_node->name + "$ptw";
                    assert(name2node.find(enc_node->name) == name2node.end());
                    name2node[xor_node->name] = xor_node;
                    name2node[enc_node->name] = enc_node;
                    *std::find(primary_outputs.begin(), primary_outputs.end(), enc_node) = xor_node;
                } else { // none output node
                    assert(name2node.find(xor_node->name) == name2node.end());
                    name2node[xor_node->name] = xor_node;
                    for (auto fan_out_node : enc_node->get_fan_outs()) {
                        fan_out_node->insert_fan_in(xor_node);
                        fan_out_node->erase_fan_in(enc_node);
                        xor_node->insert_fan_out(fan_out_node);
                    }
                    enc_node->clear_fan_outs();
                }
                enc_node->insert_fan_out(xor_node);
                key_node->insert_fan_out(xor_node);
            } else {
                // xnor gate + not
                NODE* xnor_node = new NODE(NodeType::INTERNAL, GateType::XNOR, "xnor" + std::to_string(i));
                assert(name2node.find(xnor_node->name) == name2node.end());
                name2node[xnor_node->name] = xnor_node;
                encrypted_nodes.push_back(xnor_node);
                xnor_node->insert_fan_in(enc_node);
                xnor_node->insert_fan_in(key_node);
                NODE* not_node = new NODE(NodeType::INTERNAL, GateType::NOT, "not" + std::to_string(i));
                encrypted_nodes.push_back(not_node);
                not_node->insert_fan_in(xnor_node);
                xnor_node->insert_fan_out(not_node);
                enc_node->enc = true;
                // if we are operating on the output node
                if (enc_node->type == NodeType::PRIMARY_OUTPUT) {
                    // change encoded node
                    enc_node->type = NodeType::INTERNAL;
                    not_node->type = NodeType::PRIMARY_OUTPUT;
                    not_node->name = enc_node->name;
                    // ptw = plaintext wire
                    enc_node->name = enc_node->name + "$ptw";
                    assert(name2node.find(enc_node->name) == name2node.end());
                    name2node[not_node->name] = not_node;
                    name2node[enc_node->name] = enc_node;
                    *std::find(primary_outputs.begin(), primary_outputs.end(), enc_node) = not_node;
                } else { // none output node
                    assert(name2node.find(not_node->name) == name2node.end());
                    name2node[not_node->name] = not_node;
                    for (auto fan_out_node : enc_node->get_fan_outs()) {
                        fan_out_node->insert_fan_in(not_node);
                        fan_out_node->erase_fan_in(enc_node);
                        not_node->insert_fan_out(fan_out_node);
                    }
                    enc_node->clear_fan_outs();
                }
                enc_node->insert_fan_out(xnor_node);
                key_node->insert_fan_out(xnor_node);
            }
        } else {
            // XNOR gate
            NODE* key_node = new NODE(NodeType::PRIMARY_INPUT, GateType::BUF, "keyinput" + std::to_string(i));
            assert(name2node.find(key_node->name) == name2node.end());
            key_inputs.push_back(key_node);
            name2node[key_node->name] = key_node;
            int type = rand() % 2;
            if (type == 1) {
                // xnor gate
                NODE* xnor_node = new NODE(NodeType::INTERNAL, GateType::XNOR, "xnor" + std::to_string(i));
                encrypted_nodes.push_back(xnor_node);
                xnor_node->insert_fan_in(enc_node);
                xnor_node->insert_fan_in(key_node);
                enc_node->enc = true;
                // if we are operating on the output node
                if (enc_node->type == NodeType::PRIMARY_OUTPUT) {
                    // change encoded node
                    enc_node->type = NodeType::INTERNAL;
                    xnor_node->type = NodeType::PRIMARY_OUTPUT;
                    xnor_node->name = enc_node->name;
                    // ptw = plaintext wire
                    enc_node->name = enc_node->name + "$ptw";
                    assert(name2node.find(enc_node->name) == name2node.end());
                    name2node[xnor_node->name] = xnor_node;
                    name2node[enc_node->name] = enc_node;
                    *std::find(primary_outputs.begin(), primary_outputs.end(), enc_node) = xnor_node;
                } else { // none output node
                    assert(name2node.find(xnor_node->name) == name2node.end());
                    name2node[xnor_node->name] = xnor_node;
                    for (auto fan_out_node : enc_node->get_fan_outs()) {
                        fan_out_node->insert_fan_in(xnor_node);
                        fan_out_node->erase_fan_in(enc_node);
                        xnor_node->insert_fan_out(fan_out_node);
                    }
                    enc_node->clear_fan_outs();
                }
                enc_node->insert_fan_out(xnor_node);
                key_node->insert_fan_out(xnor_node);
            } else {
                // xor gate + not
                NODE* xor_node = new NODE(NodeType::INTERNAL, GateType::XOR, "xor" + std::to_string(i));
                assert(name2node.find(xor_node->name) == name2node.end());
                name2node[xor_node->name] = xor_node;
                encrypted_nodes.push_back(xor_node);
                xor_node->insert_fan_in(enc_node);
                xor_node->insert_fan_in(key_node);
                NODE* not_node = new NODE(NodeType::INTERNAL, GateType::NOT, "not" + std::to_string(i));
                encrypted_nodes.push_back(not_node);
                not_node->insert_fan_in(xor_node);
                xor_node->insert_fan_out(not_node);
                enc_node->enc = true;
                // if we are operating on the output node
                if (enc_node->type == NodeType::PRIMARY_OUTPUT) {
                    // change encoded node
                    enc_node->type = NodeType::INTERNAL;
                    not_node->type = NodeType::PRIMARY_OUTPUT;
                    not_node->name = enc_node->name;
                    // ptw = plaintext wire
                    enc_node->name = enc_node->name + "$ptw";
                    assert(name2node.find(enc_node->name) == name2node.end());
                    name2node[not_node->name] = not_node;
                    name2node[enc_node->name] = enc_node;
                    *std::find(primary_outputs.begin(), primary_outputs.end(), enc_node) = not_node;
                } else { // none output node
                    assert(name2node.find(not_node->name) == name2node.end());
                    name2node[not_node->name] = not_node;
                    for (auto fan_out_node : enc_node->get_fan_outs()) {
                        fan_out_node->insert_fan_in(not_node);
                        fan_out_node->erase_fan_in(enc_node);
                        not_node->insert_fan_out(fan_out_node);
                    }
                    enc_node->clear_fan_outs();
                }
                enc_node->insert_fan_out(xor_node);
                key_node->insert_fan_out(xor_node);
            }
        }
    }
}

// add the final masking step
void encryption::masking()
{
    // with type nand, nor, not , and, or in testbenchs
    int current_key_counter = this->total_key_num - 1;
    assert(current_key_counter >= 0); // should at least insert a key before
    for (auto node : primary_outputs) { // masking the output node
        if (!node->enc) {
            node->enc = true;
            current_key_counter++;
            NODE* key_node = new NODE(NodeType::PRIMARY_INPUT, GateType::BUF, "keyinput" + std::to_string(current_key_counter));
            assert(name2node.find(key_node->name) == name2node.end());
            key_inputs.push_back(key_node);
            name2node[key_node->name] = key_node;
            switch (node->gate) {
            case (GateType::AND): {
                key += "1";
                node->insert_fan_in(key_node);
                key_node->insert_fan_out(node);
                break;
            }
            case (GateType::OR): {
                key += "0";
                node->insert_fan_in(key_node);
                key_node->insert_fan_out(node);
                break;
            }
            case (GateType::NAND): {
                key += "1";
                node->insert_fan_in(key_node);
                key_node->insert_fan_out(node);
                break;
            }
            case (GateType::NOR): {
                key += "0";
                node->insert_fan_in(key_node);
                key_node->insert_fan_out(node);
                break;
            }
            case (GateType::NOT): {
                // determine which output should we use
                // not -> nxor (0, original input) or xor(1, original input)
                node->insert_fan_in(key_node);
                key_node->insert_fan_out(node);
                if (static_cast<int>(node->NoO0 * node->NoP0) > static_cast<int>(node->NoO1 * node->NoP1)) {
                    key += "0";
                    node->gate = GateType::XNOR;
                } else {
                    key += "1";
                    node->gate = GateType::XOR;
                }
                break;
            }
            default: {
                std::cout << "error: unexpected type" << std::endl;
            }
            }
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

            NodeType type = NodeType::PRIMARY_INPUT;
            GateType gate = GateType::BUF;

            NODE* n;
            auto it = name2node.find(name);
            if (it == name2node.end()) {
                n = new NODE(type, gate, name);
                n->id = count++; // set ID
                n->depth = 0; // set depth
                nodes.push_back(n);
                name2node[name] = n;
            } else {
                n = it->second;
                n->gate = gate;
                n->type = type;
            }
            // Input push in
            primary_inputs.push_back(n);
        } else if (checkerflag == "OU") {
            std::string name = "";
            name.assign(buffer, 7, buffer.size() - 8);

            NodeType type = NodeType::PRIMARY_OUTPUT;
            GateType gate = GateType::BUF;

            NODE* n;
            auto it = name2node.find(name);
            if (it == name2node.end()) {
                n = new NODE(type, gate, name);
                n->id = count++; // set ID
                nodes.push_back(n);
                name2node[name] = n;
            } else {
                n = it->second;
                if (it->second->type != NodeType::PRIMARY_INPUT)
                    n->type = type;
            }
            // push in
            primary_outputs.push_back(n);
        } else {
            NodeType type = NodeType::INTERNAL; // set type
            GateType gate;

            std::string tem_buf = "";
            std::stringstream ss;
            ss << buffer;

            // name
            std::string name = "";
            ss >> name;
            // = kill
            ss >> tem_buf >> std::ws;

            // get gate type
            std::string ft_name = "";
            std::getline(ss, ft_name, '(');
            ss >> tem_buf;

            std::transform(ft_name.begin(), ft_name.end(), ft_name.begin(), tolower);
            if (ft_name == "not") {
                gate = GateType::NOT;
            } else if (ft_name == "buf") {
                gate = GateType::BUF;
            } else if (ft_name == "and") {
                gate = GateType::AND;
            } else if (ft_name == "xor") {
                gate = GateType::XOR;
            } else if (ft_name == "xnor") {
                gate = GateType::XNOR;
            } else if (ft_name == "nand") {
                gate = GateType::NAND;
            } else if (ft_name == "nor") {
                gate = GateType::NOR;
            } else if (ft_name == "or") {
                gate = GateType::OR;
            } else {
                continue;
            }

            // create node & push
            auto it = name2node.find(name);
            NODE* n;
            if (it == name2node.end()) {
                n = new NODE(type, gate, name);
                n->id = count++; // set ID
                nodes.push_back(n);
                name2node[name] = n;
            } else {
                n = it->second;
                assert(n->gate == GateType::BUF);
                n->gate = gate;
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
                    tem_n = new NODE(type, GateType::BUF, tem_buf);
                    tem_n->id = count++; // set ID
                    nodes.push_back(tem_n);
                    name2node[tem_buf] = tem_n;
                } else {
                    tem_n = it->second;
                }
                tem_n->insert_fan_out(n);
                n->insert_fan_in(tem_n);
            } while (ss >> tem_buf);
        }
    }
    input.close();
}

std::ostream& operator<<(std::ostream& os, NODE* p)
{
    os << "-------------------------------------------------------\n";
    os << "name: " << p->name << std::endl;
    os << "Gate type: " << p->gate << std::endl;
    os << "Type: " << p->type << std::endl;
    os << "ID: " << p->id << std::endl;
    os << "Fan-in node :";
    for (auto q : p->get_fan_ins()) {
        os << " " << q->name;
    }
    os << "\nFan-out node :";
    for (auto q : p->get_fan_outs()) {
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

    visited.resize(nodes.size());
    in_degree.resize(nodes.size());
    std::fill(visited.begin(), visited.end(), 0);
    std::fill(in_degree.begin(), in_degree.end(), 0);

    for (auto p : nodes) {
        for (auto q : p->get_fan_outs()) {
            in_degree[q->id]++;
        }
    }

    std::queue<NODE*> q;

    for (auto p : nodes) {
        if (in_degree[p->id] == 0) {
            q.push(p);
            p->depth = 0;
        }
    }

    std::vector<NODE*> result;

    while (!q.empty()) {
        NODE* node = q.front();
        q.pop();
        if (node->depth == -0xfffffff) {
            int depth = -1;
            for (auto children : node->get_fan_ins()) {
                depth = std::max(depth, children->depth);
            }
            node->depth = depth + 1;
        }
        result.push_back(node);

        // Decrease indegree of adjacent vertices as the
        // current node is in topological order
        for (auto it : node->get_fan_outs()) {
            in_degree[it->id]--;

            // If indegree becomes 0, push it to the queue
            if (in_degree[it->id] == 0)
                q.push(it);
        }
    }

    // Check for cycle
    assert(result.size() == nodes.size());
    nodes = std::move(result);

    if (this->is_debug) {
        for (auto it : nodes) {
            std::cout << "name : " << it->name << " depth : " << it->depth << std::endl;
            for (auto child : it->get_fan_ins()) {
                std::cout << "child : " << child->name << " depth : " << child->depth << std::endl;
            }
        }
        std::cout << "\n\n\n\n";
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
    for (auto p : primary_inputs) {
        std::string tem = "INPUT(";
        tem += p->name;
        tem += ")";
        out << tem << std::endl;
    }

    // OUTPUT
    for (auto p : primary_outputs) {
        std::string tem = "OUTPUT(";
        tem += p->name;
        tem += ")";
        out << tem << std::endl;
    }

    // OUTkey
    for (auto p : key_inputs) {
        std::string tem = "INPUT(";
        tem += p->name;
        tem += ")";
        out << tem << std::endl;
    }

    // original circuit
    for (auto p : nodes) {
        std::string tem = "";
        if (p->type == NodeType::INTERNAL || p->type == NodeType::PRIMARY_OUTPUT) {
            tem = p->name;
            tem += " = ";
            tem += p->gate_as_string();
            tem += "(";

            for (size_t i = 0; i < p->get_fan_ins().size(); i++) {
                tem += p->get_fan_ins()[i]->name;
                if (i + 1 == p->get_fan_ins().size()) {
                    tem += ")";
                } else
                    tem += ", ";
            }
            out << tem << std::endl;
        }
    }
    // encry circuit
    for (auto p : encrypted_nodes) {
        std::string tem = "";
        tem = p->name;
        tem += " = ";
        tem += p->gate_as_string();
        tem += "(";

        for (size_t i = 0; i < p->get_fan_ins().size(); i++) {
            tem += p->get_fan_ins()[i]->name;
            if (i + 1 == p->get_fan_ins().size()) {
                tem += ")";
            } else
                tem += ", ";
        }
        out << tem << std::endl;
    }
    std::cout << key << std::endl;
    out.close();
}
