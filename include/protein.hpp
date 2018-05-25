#ifndef _PROTEIN_HPP
#define _PROTEIN_HPP

#include <boost/dynamic_bitset.hpp>
#include <vector>
#include <string>
#include "runs.hpp"

class Protein {
public:
    Protein(Run *run, boost::dynamic_bitset<> seq, vector<float> concs, int kernel_index, int src_pos);
    Protein(Run *run, int src_pos); //randomly initialized
    string to_str();

    boost::dynamic_bitset<> seq;
    vector<float> concs;
    int kernel_index;
    int src_pos;
    Run *run;
};

#endif
