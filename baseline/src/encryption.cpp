#include "encryption.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <unordered_set>

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

void encryption::sl_compare_encryption()
{
    int total_enc_num = ceil(this->key_ratio * primary_inputs.size());
    assert(total_enc_num <= nodes.size());

    if (is_debug) {
        std::cout << "encryption a total of " << total_enc_num << " nodes" << "\n";
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

    std::queue<std::pair<NODE*, bool>> to_be_checked, waiting_list;
    std::vector<NODE*> to_be_enc;
    while (to_be_enc.size() < total_enc_num) {
        if (is_debug)
            std::cout << "Current to-be encrypted number: " << to_be_enc.size() << "\n";
        if (to_be_checked.empty() && waiting_list.empty()) {
            NODE* temp = initialize();
            to_be_checked.emplace(temp, 0);
            to_be_checked.emplace(temp, 1);
            to_be_enc.emplace_back(temp);
            temp->enc = 1;
        } else if (to_be_checked.empty()) {
            NODE* temp = waiting_list.front().first;
            to_be_checked.emplace(waiting_list.front());
            waiting_list.pop();
            to_be_enc.emplace_back(temp);
            temp->enc = 1;
        }

        NODE* cur = to_be_checked.front().first;
        bool way = to_be_checked.front().second;
        auto arr = (way == 0) ? cur->get_fan_ins() : cur->get_fan_outs();
        to_be_checked.pop();

        // To bypass not gates and buffers
        while (arr.size() == 1 && arr[0]->type != NodeType::PRIMARY_INPUT) {
            cur = arr[0];
            arr = (way == 0) ? cur->get_fan_ins() : cur->get_fan_outs();
        }
        if (is_debug)
            std::cout << "white people\n";

        for (auto it : arr) {
            if (it->enc)
                continue;
            if (check_pairwise_secure(cur, it, way)) {
                to_be_checked.emplace(it, way);
                to_be_enc.emplace_back(it);
                it->enc = 1;
            } else {
                waiting_list.emplace(it, way);
            }
        }
    }

    // add key gates
    std::queue<NODE*> checker;
    while (!checker.empty())
        checker.pop();

    for (int i = 0; i < total_enc_num; i++) {
        assert(i < to_be_enc.size());
        NODE* enc_node = to_be_enc[i];
        NODE* key_node = new NODE(NodeType::PRIMARY_INPUT, GateType::BUF, "keyinput" + std::to_string(i));
        NODE* xor_node = (key_arr[i] == 0) ? new NODE(NodeType::INTERNAL, GateType::XOR, "xor" + std::to_string(i)) : new NODE(NodeType::INTERNAL, GateType::XNOR, "xnor" + std::to_string(i));

        key_inputs.push_back(key_node);
        encrypted_nodes.push_back(xor_node);

        // make sure its not output, because im too lazy to implement that
        if (enc_node->type == NodeType::PRIMARY_OUTPUT) {
            // change encoded node
            enc_node->type = NodeType::INTERNAL;
            enc_node->insert_fan_out(xor_node);
            xor_node->name = enc_node->name;
            // ptw = plaintext wire
            enc_node->name = enc_node->name + "$ptw";
            *std::find(primary_outputs.begin(), primary_outputs.end(), enc_node) = xor_node;

            // xor node & all other nodes
            assert(name2node.find(enc_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            name2node[enc_node->name] = enc_node;
            xor_node->insert_fan_in(enc_node);
            xor_node->insert_fan_in(key_node);
            xor_node->type = NodeType::PRIMARY_OUTPUT;

            // key node
            key_node->insert_fan_out(xor_node);
        } else {
            // original encoded node change
            for (auto fan_out_node : enc_node->get_fan_outs()) {
                checker.emplace(fan_out_node);
            }
            enc_node->clear_fan_outs();
            enc_node->insert_fan_out(xor_node);

            // key node
            key_node->insert_fan_out(xor_node);

            // xor node & all other nodes
            assert(name2node.find(xor_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            xor_node->insert_fan_in(enc_node);
            xor_node->insert_fan_in(key_node);
            while (!checker.empty()) {
                NODE* temp = checker.front();
                checker.pop();

                temp->erase_fan_in(enc_node);
                temp->insert_fan_in(xor_node);
                xor_node->insert_fan_out(temp);
            }
        }
    }
}

void encryption::sl_brute_encryption()
{
    int total_enc_num = ceil(this->key_ratio * primary_inputs.size());
    total_enc_num = std::min(1, (int)nodes.size());
    assert(total_enc_num <= nodes.size());

    if (is_debug) {
        std::cout << "encryption a total of " << total_enc_num << " nodes" << "\n";
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

    // int stands for which way 0 is up, 1 is down, 2 is down then up. currently 0 and 2 dont have any difference
    std::queue<std::pair<NODE*, int>> to_be_checked;
    std::vector<NODE*> to_be_enc, clique;
    std::unordered_set<NODE*> checked;
    while (to_be_enc.size() < total_enc_num) {
        if (is_debug)
            std::cout << to_be_enc.size() << " keys\n";
        if (to_be_checked.empty()) {
            NODE* temp = initialize();
            to_be_checked.emplace(temp, 0);
            to_be_checked.emplace(temp, 1);
            clique.clear();
            checked.clear();
        }

        NODE *cur = to_be_checked.front().first, *drop_one = NULL;
        int way = to_be_checked.front().second;
        auto arr = (way == 1) ? cur->get_fan_outs() : cur->get_fan_ins();
        to_be_checked.pop();

        to_be_enc.emplace_back(cur);
        clique.emplace_back(cur);
        cur->enc = 1;

        if (to_be_checked.size() + to_be_enc.size() >= total_enc_num)
            continue;

        // To bypass not gates and buffers
        // TODO: check if checking type is necessary by if PI fi size
        while (arr.size() == 1 && arr[0]->type != NodeType::PRIMARY_INPUT) {
            drop_one = arr[0];
            arr = (way == 1) ? drop_one->get_fan_outs() : drop_one->get_fan_ins();
        }

        if (drop_one != NULL)
            arr = { drop_one };

        for (auto it : arr) {
            if (is_debug)
                std::cout << it->name << "\n";
            if (it->enc || checked.find(it) != checked.end())
                continue;
            checked.emplace(it);
            // TODO: check if this is correct usage of theorem 1
            bool secured = 1;
            // if(way == 0) { //using theorem 1
            // 	secured = 0;
            // 	for(int i = 0; i < clique.size() && secured == 0; i++) {
            // 		//TODO: make this function
            // 		secured = check_brute_secure(cur, it);
            // 	}
            // }
            for (int i = 0; i < clique.size() && secured == 1; i++) {
                secured = check_brute_secure(cur, it);
            }

            if (secured) {
                to_be_checked.emplace(it, way);
                if (way == 1)
                    to_be_checked.emplace(it, 2);
            }
        }
    }

    // add key gates
    std::queue<NODE*> checker;
    while (!checker.empty())
        checker.pop();

    for (int i = 0; i < total_enc_num; i++) {
        assert(i < to_be_enc.size());
        NODE* enc_node = to_be_enc[i];
        NODE* key_node = new NODE(NodeType::PRIMARY_INPUT, GateType::BUF, "keyinput" + std::to_string(i));
        NODE* xor_node = (key_arr[i] == 0) ? new NODE(NodeType::INTERNAL, GateType::XOR, "xor" + std::to_string(i)) : new NODE(NodeType::INTERNAL, GateType::XNOR, "xnor" + std::to_string(i));

        key_inputs.push_back(key_node);
        encrypted_nodes.push_back(xor_node);

        // make sure its not output, because im too lazy to implement that
        if (enc_node->type == NodeType::PRIMARY_OUTPUT) {
            // change encoded node
            enc_node->type = NodeType::INTERNAL;
            enc_node->insert_fan_out(xor_node);
            xor_node->name = enc_node->name;
            // ptw = plaintext wire
            enc_node->name = enc_node->name + "$ptw";
            *std::find(primary_outputs.begin(), primary_outputs.end(), enc_node) = xor_node;

            // xor node & all other nodes
            assert(name2node.find(enc_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            name2node[enc_node->name] = enc_node;
            xor_node->insert_fan_in(enc_node);
            xor_node->insert_fan_in(key_node);
            xor_node->type = NodeType::PRIMARY_OUTPUT;

            // key node
            key_node->insert_fan_out(xor_node);
        } else {
            // original encoded node change
            for (auto fan_out_node : enc_node->get_fan_outs()) {
                checker.emplace(fan_out_node);
            }
            enc_node->clear_fan_outs();
            enc_node->insert_fan_out(xor_node);

            // key node
            key_node->insert_fan_out(xor_node);

            // xor node & all other nodes
            assert(name2node.find(xor_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            xor_node->insert_fan_in(enc_node);
            xor_node->insert_fan_in(key_node);
            while (!checker.empty()) {
                NODE* temp = checker.front();
                checker.pop();

                temp->erase_fan_in(enc_node);
                temp->insert_fan_in(xor_node);
                xor_node->insert_fan_out(temp);
            }
        }
    }
}

int encryption::Rand(int n)
{
    return rand() % n;
}

bool encryption::check_brute_secure(NODE* a, NODE* b)
{
    std::unordered_set<NODE*> a_used_PI, b_used_PI, checked;
    std::queue<NODE*> qu;

    qu.emplace(a);
    while (!qu.empty()) {
        NODE* temp = qu.front();
        qu.pop();

        if (checked.find(temp) != checked.end())
            continue;
        checked.emplace(temp);

        for (auto it : temp->get_fan_ins()) {
            qu.emplace(it);
        }
        if (temp->type == NodeType::PRIMARY_INPUT && a_used_PI.find(temp) == a_used_PI.end())
            a_used_PI.emplace(temp);
    }

    qu.emplace(b);
    checked.clear();
    while (!qu.empty()) {
        NODE* temp = qu.front();
        qu.pop();

        if (checked.find(temp) != checked.end())
            continue;
        checked.emplace(temp);

        for (auto it : temp->get_fan_ins()) {
            qu.emplace(it);
        }
        if (temp->type == NodeType::PRIMARY_INPUT && b_used_PI.find(temp) == b_used_PI.end())
            b_used_PI.emplace(temp);
    }

    std::vector<int> which_input;
    for (int i = 0; i < primary_inputs.size(); i++) {
        if ((a_used_PI.find(primary_inputs[i]) == a_used_PI.end()) || (b_used_PI.find(primary_inputs[i]) == b_used_PI.end())) {
            which_input.emplace_back(i);
        }
    }

    // set to 2^20 times
    unsigned int up_time = 10;
    for (auto n : nodes) {
        n->setCurrentOutput(2);
    }
    for (int i = 0; i < primary_inputs.size(); i++) {
        primary_inputs[i]->setCurrentOutput(rand() % 2);
    }

    for (int i = 0; i < pow(2, std::min(unsigned(which_input.size()), up_time)); i++) {
        std::vector<bool> out, changed_out;
        bool unchanged_a = 0, unchanged_b = 0;

        if (which_input.size() > up_time) {
            std::random_shuffle(which_input.begin(), which_input.end());
        }
        for (int j = 0; j < std::min(unsigned(which_input.size()), up_time); j++) {
            set_unknown(primary_inputs[which_input[j]]);
            // brute force if less, rand if more
            int temp = (which_input.size() <= up_time) ? (i >> j) & 1 : (rand() % 2);
            primary_inputs[which_input[j]]->setCurrentOutput(temp);
        }

        for (auto it : primary_outputs) {
            out.push_back(it->getCurrentOutput());
        }

        set_unknown(a);
        a->stuck_fault_value = 1;
        for (auto it : primary_outputs) {
            changed_out.push_back(it->getCurrentOutput());
        }
        a->stuck_fault_value = 0;

        if (out == changed_out)
            unchanged_a = 1;

        changed_out.clear();

        set_unknown(b);
        set_unknown(a);
        b->stuck_fault_value = 1;
        for (auto it : primary_outputs) {
            changed_out.push_back(it->getCurrentOutput());
        }
        b->stuck_fault_value = 0;

        if (out == changed_out)
            unchanged_b = 1;

        if (unchanged_a ^ unchanged_b)
            return 0;
    }

    return 1;
}

void encryption::set_unknown(NODE* a)
{
    std::queue<NODE*> qu;
    std::unordered_set<NODE*> checked;
    qu.emplace(a);
    while (!qu.empty()) {
        NODE* temp = qu.front();
        qu.pop();
        if (checked.find(temp) != checked.end())
            continue;
        checked.emplace(temp);
        temp->setCurrentOutput(2);
        for (auto it : temp->get_fan_outs()) {
            qu.emplace(it);
        }
    }
}

bool encryption::check_pairwise_secure(NODE* main, NODE* bef, bool way)
{
    if (way == 1)
        std::swap(main, bef);

    GateType main_type = main->gate, nono_type[2];
    if (main_type == GateType::BUF || main_type == GateType::NOT)
        return 0;
    if (main_type == GateType::XNOR || main_type == GateType::XOR)
        return 1;
    switch (main_type) {
    case GateType::AND:
        nono_type[0] = GateType::AND, nono_type[1] = GateType::NOR;
        break;
    case GateType::OR:
        nono_type[0] = GateType::OR, nono_type[1] = GateType::NAND;
        break;
    case GateType::NOR:
        nono_type[0] = GateType::OR, nono_type[1] = GateType::NAND;
        break;
    case GateType::NAND:
        nono_type[0] = GateType::AND, nono_type[1] = GateType::NOR;
        break;
    default: {
        if (is_debug)
            std::cout << "something happened and you should fix it\n";
        assert(0);
    }
    }

    for (auto it : main->get_fan_ins()) {
        if (it == bef)
            continue;
        if (it->gate == nono_type[0] || it->gate == nono_type[1])
            return 0;
    }

    return 1;
}

NODE* encryption::initialize()
{
    int cur_max = 0;
    NODE* ret_node = NULL;

    for (auto it : nodes) {
        if (it->enc)
            continue;
        if (cur_max < it->get_fan_out_count()) {
            cur_max = it->get_fan_out_count();
            ret_node = it;
        }
    }
    assert(ret_node != NULL);
    return ret_node;
}

void encryption::sl_one_encryption()
{
    int total_enc_num = ceil(this->key_ratio * primary_inputs.size()), output_find = 0;
    assert(total_enc_num <= nodes.size());

    if (is_debug) {
        std::cout << "encryption a total of " << total_enc_num << " nodes" << "\n";
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

    // find the nodes to be encrypted
    std::queue<NODE*> to_be_checked;
    std::vector<NODE*> to_be_enc;
    while (to_be_enc.size() < total_enc_num && output_find <= primary_outputs.size()) {
        if (to_be_checked.size() == 0) {
            output_find++;
            if (output_find > primary_outputs.size())
                break;
            to_be_checked.emplace(primary_outputs[output_find - 1]);
            primary_outputs[output_find - 1]->enc = 1;
            to_be_enc.emplace_back(primary_outputs[output_find - 1]);
        }

        NODE* temp = to_be_checked.front();
        to_be_checked.pop();

        for (auto it : temp->get_fan_ins()) {
            if (it->enc)
                continue;
            it->enc = 1;
            to_be_enc.emplace_back(it);
            to_be_checked.emplace(it);
        }
    }

    if (is_debug) {
        std::cout << "Size of nodes to be encrypted: " << to_be_enc.size() << "\n";
        std::cout << "Size of main outputs: " << primary_outputs.size() << "\n";
        std::cout << "the stuff " << (to_be_enc.size() < total_enc_num)
                  << (output_find <= primary_outputs.size()) << "\n";
    }

    // add key gates
    while (!to_be_checked.empty())
        to_be_checked.pop();
    for (int i = 0; i < total_enc_num; i++) {
        assert(i < to_be_enc.size());
        NODE* enc_node = to_be_enc[i];
        NODE* key_node = new NODE(NodeType::PRIMARY_INPUT, GateType::BUF, "keyinput" + std::to_string(i));
        NODE* xor_node = (key_arr[i] == 0)
            ? new NODE(NodeType::INTERNAL, GateType::XOR, "xor" + std::to_string(i))
            : new NODE(NodeType::INTERNAL, GateType::XNOR, "xnor" + std::to_string(i));

        key_inputs.push_back(key_node);
        encrypted_nodes.push_back(xor_node);

        // make sure its not output, because im too lazy to implement that
        if (enc_node->type == NodeType::PRIMARY_OUTPUT) {
            // change encoded node
            enc_node->type = NodeType::INTERNAL;
            enc_node->insert_fan_out(xor_node);
            xor_node->name = enc_node->name;
            // ptw = plaintext wire
            enc_node->name = enc_node->name + "$ptw";
            *std::find(primary_outputs.begin(), primary_outputs.end(), enc_node) = xor_node;

            // xor node & all other nodes
            assert(name2node.find(enc_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            name2node[enc_node->name] = enc_node;
            xor_node->insert_fan_in(enc_node);
            xor_node->insert_fan_in(key_node);
            xor_node->type = NodeType::PRIMARY_OUTPUT;

            // key node
            key_node->insert_fan_out(xor_node);
        } else {
            // original encoded node change
            for (auto fan_out_node : enc_node->get_fan_outs()) {
                to_be_checked.emplace(fan_out_node);
            }
            enc_node->clear_fan_outs();
            enc_node->insert_fan_out(xor_node);

            // key node
            key_node->insert_fan_out(xor_node);

            // xor node & all other nodes
            assert(name2node.find(xor_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            xor_node->insert_fan_in(enc_node);
            xor_node->insert_fan_in(key_node);
            while (!to_be_checked.empty()) {
                NODE* temp = to_be_checked.front();
                to_be_checked.pop();

                temp->erase_fan_in(enc_node);
                temp->insert_fan_in(xor_node);
                xor_node->insert_fan_out(temp);
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
