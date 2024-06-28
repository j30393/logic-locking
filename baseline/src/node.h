#pragma once

#ifndef _NODE_H
#define _NODE_H

#include <ostream>
#include <string>
#include <vector>

#define debug

enum class Type {
    INTERNAL,
    PRIMARY_INPUT,
    PRIMARY_OUTPUT
};

std::ostream& operator<<(std::ostream& os, Type _t);

enum class FType {
    AND,
    OR,
    NOR,
    NAND,
    NOT,
    BUF,
    XOR,
    XNOR
};

std::ostream& operator<<(std::ostream& os, FType _ft);

class NODE {
public:
    NODE()
        : t(Type::INTERNAL)
        , ft(FType::BUF)
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

    NODE(Type _t, FType _ft, std::string _name)
        : t(_t)
        , ft(_ft)
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
        return name == _A->name && ft == _A->ft;
    }

    friend std::ostream& operator<<(std::ostream& os, NODE* p);

    Type t;
    FType ft;
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
    int getFIlen() const { return FI_Ary.size(); }
    int getFOlen() const { return FO_Ary.size(); }
    void insertFI(NODE* _node) { FI_Ary.push_back(_node); }
    void eraseFI(NODE* _node);
    void insertFO(NODE* _node) { FO_Ary.push_back(_node); }
    void eraseFO(NODE* _node);
    int FIfind(NODE* _node) const; // return index
    int FOfind(NODE* _node) const; // return index
    const std::vector<NODE*>& getFI() const { return FI_Ary; }
    const std::vector<NODE*>& getFO() const { return FO_Ary; }
    void clearFI() { FI_Ary.clear(); }
    void clearFO() { FO_Ary.clear(); }
    std::string stringFType() const;
    // 109062233 add for stuck at fault
    int getCurrentOutput() { return current_output; }
    void setCurrentOutput(int _b) { current_output = _b; }

private:
    std::vector<NODE*> FI_Ary;
    std::vector<NODE*> FO_Ary;
    int current_output;
};

#endif
