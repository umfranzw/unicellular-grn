#ifndef _GRN_HPP
#define _GRN_HPP

#include "runs.hpp"
#include "gene.hpp"
#include <vector>
#include <string>

class Grn {
public:
    Grn(Run *run, vector<Gene> genes);
    Grn(Run *run); //randomly initializes genes
    void push_initial_proteins();
    void run_decay();
    void run_binding();
    void run_diffusion();
    void update_output_proteins();
    string to_str();

    vector<Gene> genes;
    ProteinStore proteins;
    Run *run;
};

#endif
