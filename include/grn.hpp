#ifndef _GRN_HPP
#define _GRN_HPP

#include "runs.hpp"
#include "gene.hpp"
#include "protein.hpp"
#include <vector>
#include <string>

class Grn {
public:
    Grn(Run *run, vector<Gene*> genes, vector<Protein*> initial_proteins);
    Grn(Run *run); //randomly initializes genes
    Grn(Grn *grn, bool copy_store=false);
    ~Grn();
    
    void push_initial_proteins();
    void run_decay();
    void run_binding(int pop_index, int reg_step, int ga_step);
    void run_diffusion();
    void run_diffusion_kernel();
    void run_diffusion_fcn();
    void update_output_proteins();
    string to_str();
    void reset();

    vector<Gene*> genes;
    ProteinStore *proteins;
    Run *run;
    vector<Protein*> initial_proteins;
};

#endif
