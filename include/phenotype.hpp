#ifndef _PHENOTYPE_HPP
#define _PHENOTYPE_HPP

#include "tree.hpp"
#include "instr.hpp"
#include "runs.hpp"
#include <string>

class Phenotype {
public:
    Phenotype(Run *run);
    Phenotype(Phenotype *ptype);
    ~Phenotype();
    bool add_child(int parent_index, Instr *instr);
    string to_str();
    string to_code();
    int size();
    int height();
    float branching_factor();
    void reset();
    int get_num_children(int id);
    void set_instr(int id, Instr *instr);
    int get_num_unfilled_nodes();
    int get_num_filled_nodes();
    bool is_leaf(int id);
    int count_instr_type(int type);
    int count_var_occurs();
    int count_distinct_vars();
    
    Tree *tree;
    
private:
    Run *run;
};

#endif
