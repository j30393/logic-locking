#pragma once

#ifndef _ENCRYPTION_H
#define _ENCRYPTION_H

#include "node.h"
#include <map>

class encryption {
public:
    encryption();
    ~encryption();

    // operator
    void insert_node(NODE* _node) { nodes.push_back(_node); }
    void insert_primary_input(NODE* _node) { primary_inputs.push_back(_node); }
    void insert_primary_output(NODE* _node) { primary_outputs.push_back(_node); }
    void insert_key_input(NODE* _node) { key_inputs.push_back(_node); }
    void readfile(std::string); // read .bench file
    void topological_sort(); // pre-process for logic cone
    void outputfile();
    void setOutputname(std::string _name);
    void setDebugMode(bool _debug) { is_debug = _debug; }
    void fault_impact_cal();
    float key_ratio;
    void xor_encryption();
    // fault base example num
    std::vector<bool> solver(std::vector<bool>); // generate the output from the given input
    void masking();
    // fault base example num
    int test_num;
    int total_key_num;

private:
    std::vector<NODE*> nodes;
    std::vector<NODE*> primary_inputs;
    std::vector<NODE*> primary_outputs;
    std::vector<NODE*> key_inputs;
    std::vector<NODE*> encrypted_nodes;
    std::map<std::string, NODE*> name2node;
    std::string key;
    std::string filename;
    std::string outputname;
    bool twolevelfile;
    bool is_debug;
    int keyCount;
    std::vector<int> visited;
    std::vector<int> in_degree;
};

#endif
