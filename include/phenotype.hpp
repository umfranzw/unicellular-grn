#ifndef _PHEOTYPE_HPP
#define _PHEOTYPE_HPP

#include "tree.hpp"
#include "instr.hpp"
#include "instr_dist.hpp"
#include "runs.hpp"
#include <string>

class Phenotype {
public:
    Phenotype(Run *run);
    Phenotype(Phenotype *ptype);
    ~Phenotype();
    bool add_child(int parent_index, Instr *instr);
    string to_str();
    int size();
    int height();
    float branching_factor();
    void reset();
    InstrDist *get_dist(int index);
    int get_num_children(int id);
    void set_instr(int id, Instr *instr);
    int get_num_unfilled_nodes();
    
    Tree *tree;
    
private:
    Run *run;
    vector<InstrDist *> dists;
};

#endif
