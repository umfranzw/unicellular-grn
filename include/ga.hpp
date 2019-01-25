#ifndef _GA_HPP
#define _GA_HPP

#include "runs.hpp"
#include "grn.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "genetic_op.hpp"
#include "phenotype.hpp"
#include <vector>
#include "reg_sim.hpp"

#define SHELL_BUFF_SIZE 128

class Ga {
public:
    Ga(Run *run);
    ~Ga();
    void run_alg();
    void print_pop();
    void graph_results();
    
private:
    vector<Grn*> pop;
    vector<float> fitnesses;
    vector<Phenotype*> phenotypes;
    
    Run *run;
    Logger *logger;
    vector<GeneticOp*> gen_ops;
    RegSim *evalor;

    void adjust_params(Run *run);
    float clamp_param(float cur_val, float step, float limit);

    void update_fitness(int ga_step);
    float calc_fitness(Grn *grn);
    float calc_protein_error(Protein *protein);

    void reset_proteins(vector<Grn*> *pop);
};

#endif
