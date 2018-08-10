#include "tree.hpp"
#include <iostream>

Tree::Tree() {
    this->root = nullptr;
    this->next_id = 0;
}

Tree::Tree(Tree *tree) {
    this->root = nullptr;
    this->next_id = 0;

    for (auto it = tree->id_to_node.begin(); it != tree->id_to_node.end(); it++) {
        int node_id = it->first;
        Node *node = it->second;
        this->id_to_node[node_id] = new Node(node); //copy constructor should copy ids
    }
    if (this->id_to_node.size() > (int) 0) {
        this->root = this->id_to_node[0];
    }
    this->next_id = tree->next_id;
}

Tree::~Tree() {
    for (auto it = this->id_to_node.begin(); it != this->id_to_node.end(); it++) {
        delete it->second;
    }
}

int Tree::size() {
    return this->id_to_node.size();
}

bool Tree::add_child(int parent_id, Instr *instr) {
    bool success = false;
    if (this->root == nullptr && parent_id == 0) {
        this->root = new Node(0, instr, -1);
        this->id_to_node[0] = this->root;
        if (this->next_id == 0) {
            this->next_id++;
        }
        success = true;
    }
    else {
        map<int, Node*>::iterator it = this->id_to_node.find(parent_id);
        if (it != this->id_to_node.end()) {
            Node *parent = it->second;
            Node *child = new Node(this->next_id, instr, parent->id);
            parent->children.push_back(this->next_id);
            this->id_to_node[this->next_id] = child;
            this->next_id++;
            success = true;
        }
    }

    return success;
}

bool Tree::remove_child(int child_id) {
    bool success = false;
    if (this->root != nullptr && child_id == 0) {
        int num_removed = this->id_to_node.erase(this->root->id);
        if (num_removed > 0) {
            delete this->root;
            this->root = nullptr;
            success = true;
        }
    }
    else {
        map<int, Node*>::iterator it = this->id_to_node.find(child_id);
        if (it != this->id_to_node.end()) {
            Node *child = it->second;
            //remove all child's children
            for (int grandchild_id : child->children) {
                this->remove_child(grandchild_id);
            }

            //remove parent's record of child
            Node *parent = this->id_to_node[child->parent];
            vector<int>::iterator parent_it = parent->children.begin();
            while (*parent_it != child_id && parent_it != parent->children.end()) {
                parent_it++;
            }
            if (parent_it != parent->children.end()) {
                parent->children.erase(parent_it);
            }

            //remove child
            int num_removed = this->id_to_node.erase(child_id);
            if (num_removed > 0) {
                success = true;
                delete child;
            }
        }
    }

    return success;
}

string Tree::to_code() {
    return this->to_code(this->root);
}

string Tree::to_code(Node *cur) {
    string code = "";
    if (cur != nullptr && cur->instr != nullptr) {
        if ((int) cur->children.size() == 0) { //leaf
            code = cur->instr->to_code(nullptr);
        }
    
        else { //internal
            vector<string> args;
            for (int i = 0; i < (int) cur->children.size(); i++) {
                int child_id = cur->children[i];
                Node *child = this->id_to_node[child_id];
                args.push_back(this->to_code(child));
            }
            code = cur->instr->to_code(&args);
        }
    }

    return code;
}

string Tree::to_str() {
    stringstream info;
    string prefix = "";
    info << "height: " << this->height() << endl;
    info << "branching factor: " << this->branching_factor() << endl;
    if (this->root != nullptr) {
        this->to_str(this->root, &info, prefix, 0);
    }
    else {
        info << "<empty tree>" << endl;
    }

    //make it pretty with unicode
    //note: don't try to do this directly in to_str() b/c unicode chars being variable length == madness
    //not a big deal that this isn't efficient since it'll only be used for debugging
    string result = info.str();
    string pretty = "";
    for (int i = 0; i < (int) result.length(); i++) {
        switch (result[i]) {
        case '}':
            pretty += "├";
            break;
            
        case '|':
            pretty += "│";
            break;
            
        case '-':
            pretty += "─";
            break;

        case 'L':
            pretty += "└";
            break;
            
        default:
            pretty += string(1, result[i]);
            break;
        }
    }

    return pretty;
}

void Tree::to_str(Node *cur, stringstream *info, string prefix, int depth) {
    *info << prefix << "(" << cur->id << ")" << endl;

    for (int i = 0; i < (int) cur->children.size(); i++) {
        string child_prefix = "";
        for (int j = 0; j < (int) prefix.length(); j++) {
            if (prefix[j] == '}' || prefix[j] == '|') {
                child_prefix += "|";
            }
            else {
                child_prefix += " ";
            }
        }
        for (int j = (int) prefix.length(); j < depth * 4; j++) {
            child_prefix += " ";
        }
        
        child_prefix += " "; //indent
        
        if (i < (int) cur->children.size() - 1) { //not last child
            child_prefix += "}--";
        }
        else { //last child
            child_prefix += "L--";
        }
        Node *child = this->id_to_node[cur->children[i]];
        this->to_str(child, info, child_prefix, depth + 1);
    }
}

int Tree::height() {
    int max_depth = -1;
    if (this->root != nullptr) {
        max_depth = this->calc_max_depth(this->root, 0, max_depth);
    }

    return max_depth + 1;
}

int Tree::calc_max_depth(Node *cur, int depth, int max_depth) {
    if (depth > max_depth) {
        max_depth = depth;
    }
    
    for (int child_id : cur->children) {
        Node *child = this->id_to_node[child_id];
        max_depth = this->calc_max_depth(child, depth + 1, max_depth);
    }

    return max_depth;
}

float Tree::branching_factor() {
    float bf = -1.0f;

    if (this->root != nullptr) {
        pair<int, int> info = this->get_bf_info(root, 0, 0); //returns #branches, # split points (i.e. # internal nodes)
        int num_br = info.first;
        int num_splits = info.second;
        if (num_splits > 0) {
            bf = (float) num_br / (float) num_splits;
        }
        else {
            bf = 0.0f;
        }
    }

    return bf;
}

pair<int, int> Tree::get_bf_info(Node *cur, int num_br, int num_splits) {
    if (cur->children.size() > (int) 0) {
        num_br += (int) cur->children.size();
        num_splits++;

        for (int child_id : cur->children) {
            Node *child = this->id_to_node[child_id];
            pair<int, int> info = this->get_bf_info(child, num_br, num_splits);
            num_br = info.first;
            num_splits = info.second;
        }
    }

    return pair<int, int>(num_br, num_splits);
}
