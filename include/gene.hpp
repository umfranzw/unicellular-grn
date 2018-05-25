#ifndef _GENE_HPP
#define _GENE_HPP

#include <boost/dynamic_bitset.hpp>
#include <vector>
#include "runs.hpp"
#include "protein_store.hpp"

using namespace std;

class Gene {
public:
    Gene(Run *run, boost::dynamic_bitset<> binding_seq, boost::dynamic_bitset<> output_seq, float threshold, float output_rate, int kernel_index, int pos);

    //randomly initializes components that are not passed
    Gene(Run *run, int pos);

    Gene(Gene *gene);

    void update_output_protein(ProteinStore *store);
    void update_binding(pair<int, float> *protein_info, ProteinStore *store);
    string to_str();
    
    boost::dynamic_bitset<> binding_seq;
    boost::dynamic_bitset<> output_seq;
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
