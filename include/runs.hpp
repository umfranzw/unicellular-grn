#ifndef _RUNS_HPP
#define _RUNS_HPP

#include "rand.hpp"
#include "TOMLParser/toml.hpp"
#include <vector>
#include <string>

class Run {
public:
    Run(toml::Table& t, int file_index);
    Run(bool fix_rng_seed, int fixed_rng_seed);
    ~Run();
    
    Rand *rand;
    int pop_size;
    int ga_steps;
    int reg_steps;
    float mut_prob;
    float mut_prob_limit;
    float mut_step;
    float cross_frac;
    float cross_frac_limit;
    float cross_step;
    int num_genes;
    int gene_bits;
    float min_protein_conc; //proteins in which all conc values dip below this are deleted
    float max_protein_conc;
    float decay_rate;
    int initial_proteins;
    int max_proteins;
    int max_pgm_size;
    float max_mut_float;
    int max_mut_bits;
    int fitness_log_interval;
    int binding_seq_play;

    bool graph_results;
    bool log_grns;
    bool log_reg_steps;
    bool log_code_with_fitness;

    int growth_start;
    int growth_end;
    int growth_sample_interval;

    string growth_seq;
    float growth_threshold;

    float term_cutoff;

    int code_start;
    int code_end;
    int code_sample_interval;

    bool fix_rng_seed;
    int fixed_rng_seed;

    string log_dir;
    string log_mode;
    
    int file_index; //index of run in file (this is filled in by the Runs() constructor and is not part of toml file)
};

class Runs {
public:
    Runs(string run_file);
    ~Runs();
    Run* get_next();

    int size;

private:
    void parse_runs(string run_file);
    
    int index;
    vector<Run*> runs;
};

#endif
