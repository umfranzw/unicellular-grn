#ifndef _NODE_HPP
#define _NODE_HPP

#include "atom.hpp"
#include <vector>
#include <string>

class Node {
public:
    Node(int id, Atom *atom, int parent);
    Node(Node *node);
    ~Node();
    string to_str();

    vector<int> children; //stores ids
    int parent; //id (-1 means none)
    Atom *atom;
    int id;
};

#endif
