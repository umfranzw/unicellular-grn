#include "node.hpp"
#include <sstream>
#include <iostream>

Node::Node(int id, Atom *atom, int parent) {
    this->id = id;
    this->atom = atom;
    this->parent = parent;
}

Node::Node(Node *node) {
    this->id = node->id;
    this->atom = node->atom;
    for (int child_id : node->children) {
        this->children.push_back(child_id);
    }
    this->parent = node->parent;
}

Node::~Node() {
    if (this->atom != nullptr) {
        delete this->atom;
    }
    //note: children will be deleted by Tree class
}

string Node::to_str() {
    stringstream info;

    info << "Node:" << endl;
    info << "  id: " << this->id << endl;
    info << "  atom: ";
    if (this->atom == nullptr) {
        info << "null";
    }
    else {
        info << this->atom->to_code();
    }
    info << endl;
    
    return info.str();
}
