#ifndef _PHEOTYPE_HPP
#define _PHEOTYPE_HPP

#include "tree.hpp"
#include "instr.hpp"
#include <string>

class Phenotype {
public:
    Phenotype();
    Phenotype(Phenotype *ptype);
    ~Phenotype();
    bool add_child(int parent_index, Instr *instr);
    string to_str();
    int size();
    int height();
    float branching_factor();
    void reset();

    Tree *tree;
};

#endif
