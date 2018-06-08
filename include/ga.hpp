#ifndef _GA_HPP
#define _GA_HPP

#include "runs.hpp"
#include "grn.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include <vector>

class Ga {
public:
    Ga(Run *run);
    ~Ga();
    void run_alg();
    void print_pop();
    
private:
    vector<Grn*> pop;
    vector<float> fitnesses;
    Run *run;
    Logger *logger;

    int get_fittest();
    float sum_fitnesses();
    vector<pair<int, int>> select();
    int spin_wheel(vector<float> *wheel);
    void adjust_params(Run *run, int ga_step);

    void cross(vector<pair<int, int>> *parents);
    vector<Gene*> build_child_genes(vector<Gene*>::iterator left, int left_len, vector<Gene*>::iterator right, int right_len);
    vector<Protein *> build_child_init_proteins(vector<Protein*>::iterator left, int left_len, vector<Protein*>::iterator right, int right_len);
    Gene *cross_genes(Gene *left, Gene *right, int pos);
    Protein *cross_proteins(Protein *left, Protein *right);
    BitVec *cross_bitsets(BitVec *left, BitVec *right);

    void mutate();
    void mutate_int(int *val, int lower, int higher);
    void mutate_float(float *val, float lower, float upper);
    void mutate_bitset(BitVec *bits);
    void mutate_initial_proteins(vector<Protein*> *proteins);

    void update_fitness(int ga_step);
    float calc_fitness(Grn *grn);
    float calc_protein_error(Protein *protein);

    template<typename T>
    T cross_primitives(T left, T right, T lower, T upper) {
        return Utils::clamp<T>((left + right) / (T) 2, lower, upper);
    }
};

#endif
