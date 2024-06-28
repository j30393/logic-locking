#include "node.h"

std::ostream& operator<<(std::ostream& os, NodeType _type)
{
    switch (_type) {
    case NodeType::INTERNAL:
        os << "INTERNAL";
        break;
    case NodeType::PRIMARY_INPUT:
        os << "PRIMARY_INPUT";
        break;
    case NodeType::PRIMARY_OUTPUT:
        os << "PRIMARY_OUTPUT";
        break;
    default:
        os.setstate(std::ios_base::failbit);
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, GateType _gate)
{
    switch (_gate) {
    case GateType::AND:
        os << "AND";
        break;
    case GateType::OR:
        os << "OR";
        break;
    case GateType::NOR:
        os << "NOR";
        break;
    case GateType::NAND:
        os << "NAND";
        break;
    case GateType::NOT:
        os << "NOT";
        break;
    case GateType::XNOR:
        os << "XNOR";
        break;
    case GateType::XOR:
        os << "XOR";
        break;
    case GateType::BUF:
        os << "BUF";
        break;
    default:
        os.setstate(std::ios_base::failbit);
    }
    return os;
}

std::string NODE::gate_as_string() const
{
    std::string r = "";
    switch (gate) {
    case GateType::AND:
        r = "AND";
        break;
    case GateType::OR:
        r = "OR";
        break;
    case GateType::NOR:
        r = "NOR";
        break;
    case GateType::NAND:
        r = "NAND";
        break;
    case GateType::NOT:
        r = "NOT";
        break;
    case GateType::XNOR:
        r = "XNOR";
        break;
    case GateType::XOR:
        r = "XOR";
        break;
    case GateType::BUF:
        r = "BUF";
        break;
    default:
        r = "BUF";
    }
    return r;
}

int NODE::find_fan_in(NODE* _node) const
{
    int return_index = 0;
    for (auto p : fan_ins) {
        if (*p == _node) {
            return return_index;
        }
        ++return_index;
    }
    return return_index;
}

int NODE::find_fan_out(NODE* _node) const
{
    int return_index = 0;
    for (auto p : fan_outs) {
        if (*p == _node) {
            return return_index;
        }
        ++return_index;
    }
    return return_index;
}

void NODE::erase_fan_in(NODE* _node)
{
    fan_ins.erase(fan_ins.begin() + find_fan_in(_node));
}

void NODE::erase_fan_out(NODE* _node)
{
    fan_outs.erase(fan_outs.begin() + find_fan_out(_node));
}

int NODE::get_gate_delay() const
{
    if (type == NodeType::PRIMARY_INPUT)
        return 0;
    return 1;
}
