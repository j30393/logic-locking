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
    void insertNODE(NODE* _node) { NODE_Ary.push_back(_node); }
    void insertPI(NODE* _node) { PI_Ary.push_back(_node); }
    void insertPO(NODE* _node) { PO_Ary.push_back(_node); }
    void insertKey(NODE* _node) { KEY_Ary.push_back(_node); }
    void readfile(std::string); // read .bench file
    void topological_sort(); // pre-process for logic cone
    void outputfile();
    void setOutputname(std::string _name);
    void setDebugMode(bool _debug) { is_debug = _debug; }
    float key_ratio;
    void xor_encryption();

private:
    std::vector<NODE*> NODE_Ary;
    std::vector<NODE*> PI_Ary;
    std::vector<NODE*> PO_Ary;
    std::vector<NODE*> KEY_Ary;
    std::vector<NODE*> ENCY_Ary;
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
