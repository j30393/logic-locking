#include "node.h"

std::ostream& operator<<(std::ostream& os, Type _t)
{
    switch (_t) {
    case Type::INTERNAL:
        os << "INTERNAL";
        break;
    case Type::PRIMARY_INPUT:
        os << "PRIMARY_INPUT";
        break;
    case Type::PRIMARY_OUTPUT:
        os << "PRIMARY_OUTPUT";
        break;
    default:
        os.setstate(std::ios_base::failbit);
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, FType _ft)
{
    switch (_ft) {
    case FType::AND:
        os << "AND";
        break;
    case FType::OR:
        os << "OR";
        break;
    case FType::NOR:
        os << "NOR";
        break;
    case FType::NAND:
        os << "NAND";
        break;
    case FType::NOT:
        os << "NOT";
        break;
    case FType::XNOR:
        os << "XNOR";
        break;
    case FType::XOR:
        os << "XOR";
        break;
    case FType::BUF:
        os << "BUF";
        break;
    default:
        os.setstate(std::ios_base::failbit);
    }
    return os;
}

const std::string NODE::stringFType()
{
    std::string r = "";
    switch (ft) {
    case FType::AND:
        r = "AND";
        break;
    case FType::OR:
        r = "OR";
        break;
    case FType::NOR:
        r = "NOR";
        break;
    case FType::NAND:
        r = "NAND";
        break;
    case FType::NOT:
        r = "NOT";
        break;
    case FType::XNOR:
        r = "XNOR";
        break;
    case FType::XOR:
        r = "XOR";
        break;
    case FType::BUF:
        r = "BUF";
        break;
    default:
        r = "BUF";
    }
    return r;
}

const int NODE::FIfind(NODE* _node)
{
    int return_index = 0;
    for (auto p : FI_Ary) {
        if (*p == _node) {
            return return_index;
        }
        ++return_index;
    }
    return return_index;
}

const int NODE::FOfind(NODE* _node)
{
    int return_index = 0;
    for (auto p : FO_Ary) {
        if (*p == _node) {
            return return_index;
        }
        ++return_index;
    }
    return return_index;
}

void NODE::eraseFI(NODE* _node)
{
    FI_Ary.erase(FI_Ary.begin() + FIfind(_node));
}

void NODE::eraseFO(NODE* _node)
{
    FO_Ary.erase(FO_Ary.begin() + FOfind(_node));
}
