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
    const std::string& getKey() const { return key; }
    int getKeylen() const { return KEY_Ary.size(); }
    void insertNODE(NODE* _node) { NODE_Ary.push_back(_node); }
    void insertPI(NODE* _node) { PI_Ary.push_back(_node); }
    void erasePI(NODE* _node);
    void insertPO(NODE* _node) { PO_Ary.push_back(_node); }
    void erasePO(NODE* _node);
    void insertKey(NODE* _node) { KEY_Ary.push_back(_node); }
    void readfile(std::string); // read .bench file
    void topological_sort(); // pre-process for logic cone
    void outputfile();
    void setOutputname(std::string _name);
    void setDebugMode(bool _debug) { is_debug = _debug; }
    NODE* initialize();
    bool check_pairwise_secure(NODE*, NODE*, bool);
    float key_ratio;
    void sl_one_encryption();
    void sl_compare_encryption();
    void sl_brute_encryption();
    bool check_brute_secure(NODE*, NODE*);
    void set_unknown(NODE*);
    int Rand(int);

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
