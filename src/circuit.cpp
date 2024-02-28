#include "circuit.h"

#include <iostream>
#include <algorithm>

Node Circuit::node_constant_0 = Node(NODE_CONSTANT, false);
Node Circuit::node_constant_1 = Node(NODE_CONSTANT, true);

Node::Node(NodeType _type, Function _function) : name(), type(_type),
    function(_function), lazy(false), lazy_value(false), 
    input(), output(), input_names(), output_name() {}

Node::Node(NodeType _type, bool _value) : name(), type(_type),
    value(_value), lazy(false), lazy_value(false), 
    input(), output(), input_names(), output_name() {}

bool Node::eval() const {
    if (lazy) {
        return lazy_value;
    }
    bool result = false;
    if (type == NODE_DEFAULT) {
        std::size_t i;
        switch (function) {
        case FUNCTION_AND:
            result = true;
            for (i = 0; i < input.size(); ++i) {
                result = result && input[i]->eval();
            }
            break;
        case FUNCTION_NAND:
            result = true;
            for (i = 0; i < input.size(); ++i) {
                result = result && input[i]->eval();
            }
            result = !result;
            break;
        case FUNCTION_OR:
            result = false;
            for (i = 0; i < input.size(); ++i) {
                result = result || input[i]->eval();
            }
            break;
        case FUNCTION_NOR:
            result = false;
            for (i = 0; i < input.size(); ++i) {
                result = result || input[i]->eval();
            }
            result = !result;
            break;
        case FUNCTION_XOR:
            result = false;
            for (i = 0; i < input.size(); ++i) {
                result = result ^ input[i]->eval();
            }
            break;
        case FUNCTION_XNOR:
            result = true;
            for (i = 0; i < input.size(); ++i) {
                result = result ^ input[i]->eval();
            }
            break;
        case FUNCTION_BUF:
        case FUNCTION_CUT:
            if (input.size()) {
                result = input[0]->eval();
            } else {
                result = false;
            }
        break;
            case FUNCTION_NOT:
            if (input.size()) {
                result = !input[0]->eval();
            } else {
                result = true;
            }
            break;
        default:
            break;
        }
    } else if (type == NODE_INPUT) {
        result = value;
    } else if (type == NODE_CONSTANT) {
        result = value;
    }
    lazy = true;
    return lazy_value = result;
}

Net::Net() : name(), type(NET_DEFAULT), input() {}

Net::Net(const std::string &_name, NetType _type) : name(_name),
    type(_type), input(nullptr) {}

void Net::setInput(Node *node) {
    input = node;
}

Circuit::Circuit() : name("top"), nodes(), service_nodes(), nets(), inputs(),
    outputs(), renames() {}

Circuit::~Circuit() {
    for (std::size_t i = 0; i < service_nodes.size(); ++i) {
        delete service_nodes[i];
    }
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        delete nodes[i];
    }
}

void Circuit::setName(const std::string &new_name) {
    name = new_name;
}

Node *Circuit::addNode(NodeType type, Function function) {
    Node *node = new Node(type, function);
    if (type == NODE_DEFAULT) {
        nodes.push_back(node);
    } else {
        service_nodes.push_back(node);
    }
    return node;
}

Node *Circuit::addNode(NodeType type, bool value) {
    Node *node = new Node(type, value);
    if (type == NODE_DEFAULT) {
        nodes.push_back(node);
    } else {
        service_nodes.push_back(node);
    }
    return node;
}

Node *Circuit::addNode(Function function) {
    return addNode(NODE_DEFAULT, function);
}

void Circuit::addNet(const std::string &name, NetType type) {
    nets[name] = Net(name, type);
    if (type == NET_INPUT) {
        inputs.push_back(name);
    } else if (type == NET_OUTPUT) {
        outputs.push_back(name);
    }
}

void Circuit::setNetInput(const std::string &name, Node *node) {
    nets[name].input = node;
}

Node *Circuit::getNetInput(const std::string &name) const {
    if (name == CONSTANT_0) {
        return &node_constant_0;
    }
    if (name == CONSTANT_1) {
        return &node_constant_1;
    }
    if (!nets.count(name)) {
        return nullptr;
    }
    return nets.at(name).input;
}

const std::vector<std::string> &Circuit::getInputs() const {
    return inputs;
}

const std::vector<std::string> &Circuit::getOutputs() const {
    return outputs;
}

void Circuit::setInputValue(const std::string &name, bool value) {
    Node *node = getNetInput(name);
    if (node) {
        node->value = value;
    }
}

bool Circuit::getInputValue(const std::string &name) const {
    Node *node = getNetInput(name);
    if (node) {
        return node->value;
    }
    return false;
}

NetType Circuit::getNetType(const std::string &name) const {
    if (name == CONSTANT_0) {
        return NET_CONSTANT;
    } else if (name == CONSTANT_1) {
        return NET_CONSTANT;
    } else {
        return nets.at(name).type;
    }
}

const std::vector<Node *> &Circuit::getNodes() const {
    return nodes;
}

const std::map<std::string, Net> &Circuit::getNets() const {
    return nets;
}

bool Circuit::evalOutput(const std::string &name) const {
    if (!nets.count(name)) {
        return false;
    }
    const Net &net = nets.at(name);
    if (net.type != NET_OUTPUT) {
        return false;
    }
    if (!net.input) {
        return false;
    }
    for (auto node : nodes) {
        node->lazy = false;
    }
    for (auto node : service_nodes) {
        node->lazy = false;
    }
    return net.input->eval();
}

void Circuit::construct() {
    for (auto &i : service_nodes) {
        delete i;
    }
    service_nodes.clear();
    for (std::size_t i = 0; i < inputs.size(); ++i) {
        Node *input_node;
        setNetInput(inputs[i], input_node = addNode(NODE_INPUT, false));
        input_node->output_name = inputs[i];
    }
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i]->type == NODE_DEFAULT) {
            setNetInput(nodes[i]->output_name, nodes[i]);
        }
    }    
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        nodes[i]->input.clear();
        for (std::size_t j = 0; j < nodes[i]->input_names.size(); ++j) {
            nodes[i]->input.push_back(getNetInput(nodes[i]->input_names[j]));
        }
    }
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        nodes[i]->output.clear();
    }
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        for (std::size_t j = 0; j < nodes[i]->input.size(); ++j) {
            if (nodes[i]->input[j]->type == NODE_DEFAULT) {
                nodes[i]->input[j]->output.push_back(nodes[i]);
            }
        }
    }
}

void Circuit::topsort(Node *node, std::set<Node *> &used,
    std::vector<Node *> &result) const {
    used.insert(node);
    for (auto &i : node->input) {
        if (i->type == NODE_DEFAULT && !used.count(i)) {
            topsort(i, used, result);
        }
    }
    result.push_back(node);
}

void Circuit::sortNodes() {
    std::set<Node *> used;
    std::vector<Node *> result;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (!used.count(nodes[i])) {
            topsort(nodes[i], used, result);
        }
    }
    nodes = result;
}

void Circuit::print(bool abc_valid) const {
    std::cout << "module " << name << " (";
    bool first = true;
    bool wires = false;
    for (auto &i : nets) {
        if ((i.second.type == NET_INPUT) || (i.second.type == NET_OUTPUT)) {
            if (!first) {
                std::cout << ",";
            } else {
                first = false;
            }
            std::cout << " " << wire_name(i.first) << " ";
        }
        if (i.second.type == NET_DEFAULT) {
            wires = true;
        }
    }
    std::cout << ");" << std::endl;    
    
    std::cout << "input";
    first = true;
    for (auto &i : nets) {
        if (i.second.type == NET_INPUT) {
            if (!first) {
                std::cout << ",";
            } else {
                first = false;
            }            
            std::cout << " " << wire_name(i.first) << " ";
        }
    }
    std::cout << ";" << std::endl;
     
    std::cout << "output";
    first = true;
    for (auto &i : nets) {
        if (i.second.type == NET_OUTPUT) {
            if (!first) {
                std::cout << ",";
            } else {
                first = false;
            }            
            std::cout << " " << wire_name(i.first) << " ";
        }
    }
    std::cout << ";" << std::endl;
   
    if (wires) {
        std::cout << "wire";
        first = true;
        for (auto &i : nets) {
            if (i.second.type == NET_DEFAULT) {
                if (!first) {
                    std::cout << ",";
                } else {
                    first = false;
                }            
                std::cout << " " << wire_name(i.first) << " ";
            }
        }
        std::cout << ";" << std::endl;
    }

    for (auto &node : nodes) {
        if (node->type != NODE_DEFAULT) {
            continue;
        }
        switch (node->function) {
        case FUNCTION_AND:
            std::cout << "and";
            break;
        case FUNCTION_NAND:
            std::cout << "nand";
            break;
        case FUNCTION_OR:
            std::cout << "or";
            break;
        case FUNCTION_NOR:
            std::cout << "nor";
            break;
        case FUNCTION_XOR:
            std::cout << "xor";
            break;
        case FUNCTION_XNOR:
            std::cout << "xnor";
            break;
        case FUNCTION_BUF:
            std::cout << "buf";
            break;
        case FUNCTION_NOT:
            std::cout << "not";
            break;
        case FUNCTION_CUT:
            if (abc_valid) {
                std::cout << "buf";
            } else {
                std::cout << "_cut " << node->name;
            }
            break;
        default:
            std::cout << "undefined";
            break;
        }
        std::cout << " ( " << wire_name(node->output_name) << " ";
        for (auto &name : node->input_names) {
            if (name == CONSTANT_0) {
                std::cout << ", " << "1'b0" << " ";
            } else if (name == CONSTANT_1) {
                std::cout << ", " <<  "1'b1" << " ";
            } else {
                std::cout << ", " << wire_name(name) << " ";
            }
        }
        std::cout << ");" << std::endl;
    }

    std::cout << "endmodule" << std::endl;
}

const std::string &Circuit::wire_name(const std::string &name) const {
    if (renames.count(name)) {
        return renames.at(name);
    }
    return name;
}

void Circuit::renameNet(const std::string &old_name, const std::string &new_name) {
    renames[old_name] = new_name;
}

void Circuit::clearRenames() {
    renames.clear();
}

