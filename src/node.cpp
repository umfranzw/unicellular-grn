#include "node.hpp"
#include <sstream>
#include <iostream>

Node::Node(int id, Instr *instr, int parent) {
    this->id = id;
    this->instr = instr;
    this->parent = parent;
}

Node::Node(Node *node) {
    this->id = node->id;
    if (node->instr != nullptr) {
        this->instr = node->instr->clone();
    }
    for (int child_id : node->children) {
        this->children.push_back(child_id);
    }
    this->parent = node->parent;
}

Node::~Node() {
    if (this->instr != nullptr) {
        delete this->instr;
    }
    //note: children will be deleted by Tree class
}

string Node::to_str() {
    stringstream info;

    info << "Node:" << endl;
    info << "  id: " << this->id << endl;
    info << "  instr: ";
    if (this->instr == nullptr) {
        info << "null";
    }
    else {
        info << this->instr->to_code(nullptr); //does not print children (i.e. args)
    }
    info << endl;
    
    return info.str();
}
