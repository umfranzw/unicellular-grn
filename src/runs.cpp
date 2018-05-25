#include "runs.hpp"
#include "constants.hpp"
#include <fstream>
#include <iostream>

Run::Run(toml::Table& t) {
    this->rand = Rand();
    this->pop_size = toml::get<toml::Integer>(t.at("pop_size"));
    this->ga_steps = toml::get<toml::Integer>(t.at("ga_steps"));
    this->reg_steps = toml::get<toml::Integer>(t.at("reg_steps"));
    this->mut_prob = (float) toml::get<toml::Float>(t.at("mut_prob"));
    this->cross_frac = (float) toml::get<toml::Float>(t.at("cross_frac"));
    this->num_genes = toml::get<toml::Integer>(t.at("num_genes"));
    this->gene_bits = toml::get<toml::Integer>(t.at("gene_bits"));
    this->min_protein_conc = (float) toml::get<toml::Float>(t.at("min_protein_conc"));
    this->max_protein_conc = (float) toml::get<toml::Float>(t.at("max_protein_conc"));
    this->alpha = (float) toml::get<toml::Float>(t.at("alpha"));
    this->beta = (float) toml::get<toml::Float>(t.at("beta"));
    this->decay_rate = (float) toml::get<toml::Float>(t.at("decay_rate"));
    this->initial_proteins = toml::get<toml::Integer>(t.at("initial_proteins"));
    this->max_mut_float = (float) toml::get<toml::Float>(t.at("max_mut_float"));
    this->max_mut_bits = toml::get<toml::Integer>(t.at("max_mut_bits"));
    this->fitness_log_interval = toml::get<toml::Integer>(t.at("fitness_log_interval"));
}

Runs::Runs() {
    ifstream ifs(RUN_FILE);
    toml::Data data = toml::parse(ifs);

    vector<toml::Table> tables = toml::get<toml::Array<toml::Table>>(data.at("runs"));

    for (toml::Table & t : tables) {
        Run *cur = new Run(t);
        this->runs.push_back(cur);
    }
}

Runs::~Runs() {
    for (auto it = this->runs.begin(); it != this->runs.end(); it++) {
        delete *it;
    }
}

vector<Run*> Runs::get_runs() {
    return this->runs;
}
