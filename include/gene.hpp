#ifndef _GENE_HPP
#define _GENE_HPP

#include <vector>
#include "runs.hpp"
#include "protein_store.hpp"
#include "bitvec.hpp"

using namespace std;

class Gene {
public:
    Gene(Run *run, BitVec *binding_seq, BitVec *output_seq, float threshold, float output_rate, int kernel_index, int pos);
    //randomly initializes components that are not passed
    Gene(Run *run, int pos);
    Gene(Gene *gene);
    ~Gene();

    void update_output_protein(ProteinStore *store);
    void update_binding(int *bind_pid, ProteinStore *store);
    string to_str();
    void reset();
    
    BitVec *binding_seq;
    BitVec *output_seq;
    float threshold;
    float output_rate;
    int kernel_index;
    vector<int> outputs;
    int active_output;
    int bound_protein;
    int pos;
    Run *run;
};

#endif
