#ifndef _GA_HPP
#define _GA_HPP

#include "runs.hpp"
#include "grn.hpp"
#include <vector>

class Ga {
public:
    Ga(Run *run);
    void run_alg();
    
private:
    vector<Grn> pop;
    vector<float> fitnesses;
    Run *run;

    int get_fittest();
    float get_avg_fitness();
    float sum_fitnesses();
    vector<pair<int, int>> select();
    int spin_wheel(vector<float> *wheel);

    void cross(vector<pair<int, int>> *parents);
    Grn build_child(vector<Gene>::iterator left, int left_len, vector<Gene>::iterator right, int right_len);
    Gene cross_genes(Gene *left, Gene *right, int pos);
    float cross_floats(float left, float right, float lower, float upper);
    boost::dynamic_bitset<> cross_bitsets(boost::dynamic_bitset<> *left, boost::dynamic_bitset<> *right);

    void mutate();
    void mutate_kernel_index(int *index);
    void mutate_float(float *val, float lower, float upper);
    void mutate_bitset(boost::dynamic_bitset<> *bits);

    void update_fitness();
    float calc_fitness(Grn *grn);
    float calc_protein_error(Protein *protein);
};

#endif
