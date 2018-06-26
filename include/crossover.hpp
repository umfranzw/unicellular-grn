#ifndef _CROSSOVER_HPP
#define _CROSSOVER_HPP

#include "gene.hpp"
#include "runs.hpp"
#include "grn.hpp"
#include "protein.hpp"
#include "bitvec.hpp"
#include "utils.hpp"
#include "genetic_op.hpp"
#include <vector>

class Crossover : public GeneticOp {
public:
    Crossover(Run *run);
    ~Crossover();
    void run_op(vector<Grn*> *pop, vector<float> *fitnesses);

private:
    vector<pair<int, int>> select(vector<float> *fitnesses);
    int get_fittest(vector<float> *fitnesses);
    vector<Protein *> build_child_init_proteins(vector<Protein*>::iterator left, int left_len, vector<Protein*>::iterator right, int right_len);
    vector<Gene*> build_child_genes(vector<Gene*>::iterator left, int left_len, vector<Gene*>::iterator right, int right_len);
    Gene *cross_genes(Gene *left, Gene *right, int pos);
    Protein *cross_proteins(Protein *left, Protein *right);
    BitVec *cross_bitsets(BitVec *left, BitVec *right);
    
    template<typename T>
    T cross_primitives(T left, T right, T lower, T upper) {
        return Utils::clamp<T>((left + right) / (T) 2, lower, upper);
    }
};

#endif
