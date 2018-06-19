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

    void adjust_params(Run *run, int ga_step);
    float clamp_param(float cur_val, float step, float limit);

    void update_fitness(int ga_step);
    float calc_fitness(Grn *grn);
    float calc_protein_error(Protein *protein);
};

#endif
