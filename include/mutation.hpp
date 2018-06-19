#ifndef _MUTATION_HPP
#define _MUTATION_HPP

#include "grn.hpp"
#include "runs.hpp"
#include "protein.hpp"
#include "bitvec.hpp"
#include "genetic_op.hpp"
#include <vector>

class Mutation : public GeneticOp {
public:
    Mutation(Run *run);
    void run_op(vector<Grn*> *pop, vector<float> *fitnesses);
    void mutate_initial_proteins(vector<Protein*> *proteins);
    
private:
    void mutate_int(int *val, int lower, int upper);
    void mutate_float(float *val, float lower, float upper);
    void mutate_bitset(BitVec *bits);
};

#endif
