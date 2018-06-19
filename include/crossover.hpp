#ifndef _CROSSOVER_HPP
#define _CROSSOVER_HPP

#include "gene.hpp"
#include "runs.hpp"
#include "grn.hpp"
#include "protein.hpp"
#include "bitvec.hpp"
#include "utils.hpp"
#include <vector>

class Crossover {
public:
    static void crossover(Run *run, vector<Grn*> *pop, vector<float> *fitnesses);

private:
    static vector<pair<int, int>> select(Run *run, vector<float> *fitnesses);
    static int get_fittest(Run *run, vector<float> *fitnesses);
    static float sum_fitnesses(Run *run, vector<float> *fitnesses);
    static int spin_wheel(Run *run, vector<float> *wheel);
    static vector<Protein *> build_child_init_proteins(Run *run, vector<Protein*>::iterator left, int left_len, vector<Protein*>::iterator right, int right_len);
    static vector<Gene*> build_child_genes(Run *run, vector<Gene*>::iterator left, int left_len, vector<Gene*>::iterator right, int right_len);
    static Gene *cross_genes(Run *run, Gene *left, Gene *right, int pos);
    static Protein *cross_proteins(Run *run, Protein *left, Protein *right);
    static BitVec *cross_bitsets(Run *run, BitVec *left, BitVec *right);

    
    template<typename T>
    static T cross_primitives(T left, T right, T lower, T upper) {
        return Utils::clamp<T>((left + right) / (T) 2, lower, upper);
    }
};

#endif
