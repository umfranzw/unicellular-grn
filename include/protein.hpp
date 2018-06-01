#ifndef _PROTEIN_HPP
#define _PROTEIN_HPP

#include <vector>
#include <string>
#include "runs.hpp"
#include "bitvec.hpp"

class Protein {
public:
    Protein(Run *run, BitVec *seq, vector<float> concs, int kernel_index, int src_pos);
    Protein(Run *run, int src_pos); //randomly initialized
    Protein(Protein *protein);
    ~Protein();
    string to_str();

    BitVec *seq;
    vector<float> concs;
    int kernel_index;
    int src_pos;
    Run *run;
};

#endif
