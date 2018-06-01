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
    Grn(Grn *grn);
    ~Grn();
    
    void push_initial_proteins();
    void run_decay();
    void run_binding();
    void run_diffusion();
    void update_output_proteins();
    string to_str();
    void reset();

    vector<Gene*> genes;
    ProteinStore *proteins;
    Run *run;
    vector<Protein*> initial_proteins;
};

#endif
