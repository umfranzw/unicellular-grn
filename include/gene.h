#ifndef GENE_H
#define GENE_H

#include <iostream>
#include "protein.h"
#include "constants.h"
#include <bitset>
#include <string>
#include "utils.h"

using namespace std;

class Protein;

class Gene
{
public:
    bitset<BIND_BITS> binding_seq;
    bitset<OUTPUT_BITS> output_seq;
    Protein *bound;
    float threshold;
    float out_rate;
    int diff_index;
    
    Gene();
    string str();
    void init_rand(RandGen &gen);
};

#endif
