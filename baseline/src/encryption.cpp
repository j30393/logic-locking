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

void encryption::sl_compare_encryption()
{
    int total_enc_num = ceil(this->key_ratio * PI_Ary.size());
    assert(total_enc_num <= NODE_Ary.size());

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
        auto arr = (way == 0) ? cur->getFI() : cur->getFO();
        to_be_checked.pop();

        // To bypass not gates and buffers
        while (arr.size() == 1 && arr[0]->t != Type::PRIMARY_INPUT) {
            cur = arr[0];
            arr = (way == 0) ? cur->getFI() : cur->getFO();
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
        NODE* key_node = new NODE(Type::PRIMARY_INPUT, FType::BUF, "keyinput" + std::to_string(i));
        NODE* xor_node = (key_arr[i] == 0) ? new NODE(Type::INTERNAL, FType::XOR, "xor" + std::to_string(i)) : new NODE(Type::INTERNAL, FType::XNOR, "xnor" + std::to_string(i));

        KEY_Ary.push_back(key_node);
        ENCY_Ary.push_back(xor_node);

        // make sure its not output, because im too lazy to implement that
        if (enc_node->t == Type::PRIMARY_OUTPUT) {
            // change encoded node
            enc_node->t = Type::INTERNAL;
            enc_node->insertFO(xor_node);
            xor_node->name = enc_node->name;
            // ptw = plaintext wire
            enc_node->name = enc_node->name + "$ptw";
            *std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = xor_node;

            // xor node & all other nodes
            assert(name2node.find(enc_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            name2node[enc_node->name] = enc_node;
            xor_node->insertFI(enc_node);
            xor_node->insertFI(key_node);
            xor_node->t = Type::PRIMARY_OUTPUT;

            // key node
            key_node->insertFO(xor_node);
        } else {
            // original encoded node change
            for (auto fan_out_node : enc_node->getFO()) {
                checker.emplace(fan_out_node);
            }
            enc_node->clearFO();
            enc_node->insertFO(xor_node);

            // key node
            key_node->insertFO(xor_node);

            // xor node & all other nodes
            assert(name2node.find(xor_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            xor_node->insertFI(enc_node);
            xor_node->insertFI(key_node);
            while (!checker.empty()) {
                NODE* temp = checker.front();
                checker.pop();

                temp->eraseFI(enc_node);
                temp->insertFI(xor_node);
                xor_node->insertFO(temp);
            }
        }
    }
}

void encryption::sl_brute_encryption()
{
    int total_enc_num = ceil(this->key_ratio * PI_Ary.size());
    total_enc_num = std::min(1, (int)NODE_Ary.size());
    assert(total_enc_num <= NODE_Ary.size());

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
        auto arr = (way == 1) ? cur->getFO() : cur->getFI();
        to_be_checked.pop();

        to_be_enc.emplace_back(cur);
        clique.emplace_back(cur);
        cur->enc = 1;

        if (to_be_checked.size() + to_be_enc.size() >= total_enc_num)
            continue;

        // To bypass not gates and buffers
        // TODO: check if checking type is necessary by if PI fi size
        while (arr.size() == 1 && arr[0]->t != Type::PRIMARY_INPUT) {
            drop_one = arr[0];
            arr = (way == 1) ? drop_one->getFO() : drop_one->getFI();
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
        NODE* key_node = new NODE(Type::PRIMARY_INPUT, FType::BUF, "keyinput" + std::to_string(i));
        NODE* xor_node = (key_arr[i] == 0) ? new NODE(Type::INTERNAL, FType::XOR, "xor" + std::to_string(i)) : new NODE(Type::INTERNAL, FType::XNOR, "xnor" + std::to_string(i));

        KEY_Ary.push_back(key_node);
        ENCY_Ary.push_back(xor_node);

        // make sure its not output, because im too lazy to implement that
        if (enc_node->t == Type::PRIMARY_OUTPUT) {
            // change encoded node
            enc_node->t = Type::INTERNAL;
            enc_node->insertFO(xor_node);
            xor_node->name = enc_node->name;
            // ptw = plaintext wire
            enc_node->name = enc_node->name + "$ptw";
            *std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = xor_node;

            // xor node & all other nodes
            assert(name2node.find(enc_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            name2node[enc_node->name] = enc_node;
            xor_node->insertFI(enc_node);
            xor_node->insertFI(key_node);
            xor_node->t = Type::PRIMARY_OUTPUT;

            // key node
            key_node->insertFO(xor_node);
        } else {
            // original encoded node change
            for (auto fan_out_node : enc_node->getFO()) {
                checker.emplace(fan_out_node);
            }
            enc_node->clearFO();
            enc_node->insertFO(xor_node);

            // key node
            key_node->insertFO(xor_node);

            // xor node & all other nodes
            assert(name2node.find(xor_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            xor_node->insertFI(enc_node);
            xor_node->insertFI(key_node);
            while (!checker.empty()) {
                NODE* temp = checker.front();
                checker.pop();

                temp->eraseFI(enc_node);
                temp->insertFI(xor_node);
                xor_node->insertFO(temp);
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

        for (auto it : temp->getFI()) {
            qu.emplace(it);
        }
        if (temp->t == Type::PRIMARY_INPUT && a_used_PI.find(temp) == a_used_PI.end())
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

        for (auto it : temp->getFI()) {
            qu.emplace(it);
        }
        if (temp->t == Type::PRIMARY_INPUT && b_used_PI.find(temp) == b_used_PI.end())
            b_used_PI.emplace(temp);
    }

    std::vector<int> which_input;
    for (int i = 0; i < PI_Ary.size(); i++) {
        if ((a_used_PI.find(PI_Ary[i]) == a_used_PI.end()) || (b_used_PI.find(PI_Ary[i]) == b_used_PI.end())) {
            which_input.emplace_back(i);
        }
    }

    // set to 2^20 times
    unsigned int up_time = 10;
    for (auto n : NODE_Ary) {
        n->setCurrentOutput(2);
    }
    for (int i = 0; i < PI_Ary.size(); i++) {
        PI_Ary[i]->setCurrentOutput(rand() % 2);
    }

    for (int i = 0; i < pow(2, std::min(unsigned(which_input.size()), up_time)); i++) {
        std::vector<bool> out, changed_out;
        bool unchanged_a = 0, unchanged_b = 0;

        if (which_input.size() > up_time) {
            std::random_shuffle(which_input.begin(), which_input.end());
        }
        for (int j = 0; j < std::min(unsigned(which_input.size()), up_time); j++) {
            set_unknown(PI_Ary[which_input[j]]);
            // brute force if less, rand if more
            int temp = (which_input.size() <= up_time) ? (i >> j) & 1 : (rand() % 2);
            PI_Ary[which_input[j]]->setCurrentOutput(temp);
        }

        for (auto it : PO_Ary) {
            out.push_back(it->getCurrentOutput());
        }

        set_unknown(a);
        a->stuck_fault_value = 1;
        for (auto it : PO_Ary) {
            changed_out.push_back(it->getCurrentOutput());
        }
        a->stuck_fault_value = 0;

        if (out == changed_out)
            unchanged_a = 1;

        changed_out.clear();

        set_unknown(b);
        set_unknown(a);
        b->stuck_fault_value = 1;
        for (auto it : PO_Ary) {
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
        for (auto it : temp->getFO()) {
            qu.emplace(it);
        }
    }
}

bool encryption::check_pairwise_secure(NODE* main, NODE* bef, bool way)
{
    if (way == 1)
        std::swap(main, bef);

    FType main_type = main->ft, nono_type[2];
    if (main_type == FType::BUF || main_type == FType::NOT)
        return 0;
    if (main_type == FType::XNOR || main_type == FType::XOR)
        return 1;
    switch (main_type) {
    case FType::AND:
        nono_type[0] = FType::AND, nono_type[1] = FType::NOR;
        break;
    case FType::OR:
        nono_type[0] = FType::OR, nono_type[1] = FType::NAND;
        break;
    case FType::NOR:
        nono_type[0] = FType::OR, nono_type[1] = FType::NAND;
        break;
    case FType::NAND:
        nono_type[0] = FType::AND, nono_type[1] = FType::NOR;
        break;
    default: {
        if (is_debug)
            std::cout << "something happened and you should fix it\n";
        assert(0);
    }
    }

    for (auto it : main->getFI()) {
        if (it == bef)
            continue;
        if (it->ft == nono_type[0] || it->ft == nono_type[1])
            return 0;
    }

    return 1;
}

NODE* encryption::initialize()
{
    int cur_max = 0;
    NODE* ret_node = NULL;

    for (auto it : NODE_Ary) {
        if (it->enc)
            continue;
        if (cur_max < it->getFOlen()) {
            cur_max = it->getFOlen();
            ret_node = it;
        }
    }
    assert(ret_node != NULL);
    return ret_node;
}

void encryption::sl_one_encryption()
{
    int total_enc_num = ceil(this->key_ratio * PI_Ary.size()), output_find = 0;
    assert(total_enc_num <= NODE_Ary.size());

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
    while (to_be_enc.size() < total_enc_num && output_find <= PO_Ary.size()) {
        if (to_be_checked.size() == 0) {
            output_find++;
            if (output_find > PO_Ary.size())
                break;
            to_be_checked.emplace(PO_Ary[output_find - 1]);
            PO_Ary[output_find - 1]->enc = 1;
            to_be_enc.emplace_back(PO_Ary[output_find - 1]);
        }

        NODE* temp = to_be_checked.front();
        to_be_checked.pop();

        for (auto it : temp->getFI()) {
            if (it->enc)
                continue;
            it->enc = 1;
            to_be_enc.emplace_back(it);
            to_be_checked.emplace(it);
        }
    }

    if (is_debug) {
        std::cout << "Size of nodes to be encrypted: " << to_be_enc.size() << "\n";
        std::cout << "Size of main outputs: " << PO_Ary.size() << "\n";
        std::cout << "the stuff " << (to_be_enc.size() < total_enc_num)
                  << (output_find <= PO_Ary.size()) << "\n";
    }

    // add key gates
    while (!to_be_checked.empty())
        to_be_checked.pop();
    for (int i = 0; i < total_enc_num; i++) {
        assert(i < to_be_enc.size());
        NODE* enc_node = to_be_enc[i];
        NODE* key_node = new NODE(Type::PRIMARY_INPUT, FType::BUF, "keyinput" + std::to_string(i));
        NODE* xor_node = (key_arr[i] == 0)
            ? new NODE(Type::INTERNAL, FType::XOR, "xor" + std::to_string(i))
            : new NODE(Type::INTERNAL, FType::XNOR, "xnor" + std::to_string(i));

        KEY_Ary.push_back(key_node);
        ENCY_Ary.push_back(xor_node);

        // make sure its not output, because im too lazy to implement that
        if (enc_node->t == Type::PRIMARY_OUTPUT) {
            // change encoded node
            enc_node->t = Type::INTERNAL;
            enc_node->insertFO(xor_node);
            xor_node->name = enc_node->name;
            // ptw = plaintext wire
            enc_node->name = enc_node->name + "$ptw";
            *std::find(PO_Ary.begin(), PO_Ary.end(), enc_node) = xor_node;

            // xor node & all other nodes
            assert(name2node.find(enc_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            name2node[enc_node->name] = enc_node;
            xor_node->insertFI(enc_node);
            xor_node->insertFI(key_node);
            xor_node->t = Type::PRIMARY_OUTPUT;

            // key node
            key_node->insertFO(xor_node);
        } else {
            // original encoded node change
            for (auto fan_out_node : enc_node->getFO()) {
                to_be_checked.emplace(fan_out_node);
            }
            enc_node->clearFO();
            enc_node->insertFO(xor_node);

            // key node
            key_node->insertFO(xor_node);

            // xor node & all other nodes
            assert(name2node.find(xor_node->name) == name2node.end());
            name2node[xor_node->name] = xor_node;
            xor_node->insertFI(enc_node);
            xor_node->insertFI(key_node);
            while (!to_be_checked.empty()) {
                NODE* temp = to_be_checked.front();
                to_be_checked.pop();

                temp->eraseFI(enc_node);
                temp->insertFI(xor_node);
                xor_node->insertFO(temp);
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
        }
    }

    std::vector<NODE*> result;

    while (!q.empty()) {
        NODE* node = q.front();
        q.pop();
        if (node->depth == -0xfffffff) {
            int depth = -1;
            for (auto children : node->getFI()) {
                depth = std::max(depth, children->depth);
            }
            node->depth = depth + 1;
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
