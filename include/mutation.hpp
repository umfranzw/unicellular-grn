#ifndef _MUTATION_HPP
#define _MUTATION_HPP

#include "grn.hpp"
#include "runs.hpp"
#include "protein.hpp"
#include "bitvec.hpp"
#include <vector>

class Mutation {
public:
    static void mutate(Run *run, vector<Grn*> *pop);
    static void mutate_initial_proteins(Run *run, vector<Protein*> *proteins);
    
private:
    static void mutate_int(Run *run, int *val, int lower, int upper);
    static void mutate_float(Run *run, float *val, float lower, float upper);
    static void mutate_bitset(Run *run, BitVec *bits);
};

#endif
