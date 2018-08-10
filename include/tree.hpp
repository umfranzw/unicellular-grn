#ifndef _TREE_HPP
#define _TREE_HPP

#include "node.hpp"
#include "instr.hpp"
#include <map>
#include <sstream>

using namespace std;

class Tree {
public:
    Tree();
    Tree(Tree *tree);
    ~Tree();
    int size();
    int height();
    bool add_child(int parent_id, Instr *instr);
    bool remove_child(int child_id);
    float branching_factor();
    string to_str();
    string to_code();
    
private:
    Node *root;
    map<int, Node*> id_to_node;
    int next_id;
    int max_depth;

    string to_code(Node *cur);
    void to_str(Node *cur, stringstream *info, string prefix, int depth);
    int calc_max_depth(Node *cur, int depth, int max_depth);
    pair<int, int> get_bf_info(Node *cur, int num_br, int num_splits);
};

#endif