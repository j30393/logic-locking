#pragma once

#ifndef _NODE_H
#define _NODE_H

#include <ostream>
#include <string>
#include <vector>

#define debug

enum class NodeType {
    INTERNAL,
    PRIMARY_INPUT,
    PRIMARY_OUTPUT
};

std::ostream& operator<<(std::ostream& os, NodeType _type);

enum class GateType {
    AND,
    OR,
    NOR,
    NAND,
    NOT,
    BUF,
    XOR,
    XNOR
};

std::ostream& operator<<(std::ostream& os, GateType _gate);

class NODE {
public:
    NODE()
        : type(NodeType::INTERNAL)
        , gate(GateType::BUF)
        , name("")
        , id(0)
        , enc(false)
        , is_stuck_faulting(false)
        , depth(-0xfffffff)
        , NoP0(0)
        , NoO0(0)
        , NoP1(0)
        , NoO1(0)
        , fault_impact(0)
    {
    }

    NODE(NodeType _t, GateType _ft, std::string _name)
        : type(_t)
        , gate(_ft)
        , name(_name)
        , id(0)
        , enc(false)
        , is_stuck_faulting(false)
        , depth(-0xfffffff)
        , NoP0(0)
        , NoO0(0)
        , NoP1(0)
        , NoO1(0)
        , fault_impact(0)
    {
    }

    ~NODE()
    {
    }

    // operator overloading
    bool operator==(NODE* _A)
    {
        return name == _A->name && gate == _A->gate;
    }

    friend std::ostream& operator<<(std::ostream& os, NODE* p);

    NodeType type;
    GateType gate;
    std::string name;
    int id;
    bool enc;
    // 109062233 add for stuck at fault
    bool stuck_fault_value;
    bool is_stuck_faulting;
    int depth;
    int NoP0; // detect the stuck at 0 fault
    int NoO0; // affect bits for stuck at 0 fault
    int NoP1; // detect the stuck at 1 fault
    int NoO1; // affect bits for stuck at 1 fault
    unsigned long long int fault_impact;

    // operator
    int get_fan_in_count() const { return fan_ins.size(); }
    int get_fan_out_count() const { return fan_outs.size(); }
    void insert_fan_in(NODE* _node) { fan_ins.push_back(_node); }
    void erase_fan_in(NODE* _node);
    void insert_fan_out(NODE* _node) { fan_outs.push_back(_node); }
    void erase_fan_out(NODE* _node);
    int find_fan_in(NODE* _node) const; // return index
    int find_fan_out(NODE* _node) const; // return index
    const std::vector<NODE*>& get_fan_ins() const { return fan_ins; }
    const std::vector<NODE*>& get_fan_outs() const { return fan_outs; }
    void clear_fan_ins() { fan_ins.clear(); }
    void clear_fan_outs() { fan_outs.clear(); }
    std::string gate_as_string() const;
    // 109062233 add for stuck at fault
    int getCurrentOutput() { return current_output; }
    void setCurrentOutput(int _b) { current_output = _b; }

private:
    std::vector<NODE*> fan_ins;
    std::vector<NODE*> fan_outs;
    int current_output;
};

#endif
