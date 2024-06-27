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
        , depth(-0xfffffff)
    {
    }

    NODE(Type _t, FType _ft, std::string _name)
        : t(_t)
        , ft(_ft)
        , name(_name)
        , id(0)
        , enc(false)
        , depth(-0xfffffff)
    {
    }

    ~NODE()
    {
    }

    NODE* operator=(NODE* _n)
    {
        NODE* tem_n;
        tem_n = _n;
        return tem_n;
    }
    // operator overloading
    bool operator==(NODE* _A)
    {
        return (name == _A->name && ft == _A->ft);
    }
    bool operator==(std::string _name)
    {
        return name == _name;
    }

    bool operator>(NODE* _A)
    {
        return name > _A->name;
    }
    bool operator<(NODE* _A)
    {
        return name < _A->name;
    }
    friend std::ostream& operator<<(std::ostream& os, NODE* p);

    Type t;
    FType ft;
    std::string name;
    int id;
    bool enc;
    bool stuck_fault_value; // using this for flipping gate value
    int depth;

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
    int getCurrentOutput() { return calculateValue(); }
    void setCurrentOutput(int _b) { current_output = _b; }
    bool calculateValue();

private:
    std::vector<NODE*> FI_Ary;
    std::vector<NODE*> FO_Ary;
    int current_output;
};

#endif
