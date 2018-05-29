#ifndef _RUNS_HPP
#define _RUNS_HPP

#include "rand.hpp"
#include "TOMLParser/toml.hpp"
#include <vector>

class Run {
public:
    Run(toml::Table& t, int run_index);
    
    Rand rand;
    int pop_size;
    int ga_steps;
    int reg_steps;
    float mut_prob;
    float cross_frac;
    int num_genes;
    int gene_bits;
    float min_protein_conc; //proteins in which all conc values dip below this are deleted
    float max_protein_conc;
    float alpha; //concentration weight (for binding)
    float beta; //sequence weight (for binding)
    float decay_rate;
    int initial_proteins;
    float max_mut_float;
    float max_mut_bits;
    int fitness_log_interval;

    int run_index; //index of run in file (this is filled in by the Runs() constructor and is not part of toml file)
};

class Runs {
public:
    Runs();
    vector<Run> get_runs();
};

#endif
