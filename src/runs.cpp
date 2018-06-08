#include "runs.hpp"
#include "constants.hpp"
#include <fstream>
#include <iostream>

Run::Run(toml::Table& t, int file_index) {
    this->rand = Rand();
    this->pop_size = toml::get<toml::Integer>(t.at("pop_size"));
    this->ga_steps = toml::get<toml::Integer>(t.at("ga_steps"));
    this->reg_steps = toml::get<toml::Integer>(t.at("reg_steps"));
    this->mut_prob = (float) toml::get<toml::Float>(t.at("mut_prob"));
    this->min_mut_prob = (float) toml::get<toml::Float>(t.at("min_mut_prob"));
    this->mut_step = (float) toml::get<toml::Float>(t.at("mut_step"));
    this->cross_frac = (float) toml::get<toml::Float>(t.at("cross_frac"));
    this->min_cross_frac = (float) toml::get<toml::Float>(t.at("min_cross_frac"));
    this->cross_step = (float) toml::get<toml::Float>(t.at("cross_step"));
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
    string bmeth = toml::get<toml::String>(t.at("binding_method"));
    if (bmeth == "scaled") {
        this->binding_method = BINDING_SCALED;
    }
    else if (bmeth == "thresholded") {
        this->binding_method = BINDING_THRESHOLDED;
    }
    else {
        cerr << "Unrecognized binding method: '" << bmeth << "'" << endl;
        cerr << "Defaulting to 'thresholded'." << endl;
        this->binding_method = BINDING_THRESHOLDED;
    }

    this->log_ga_steps = toml::get<toml::Boolean>(t.at("log_ga_steps"));
    this->log_reg_steps = toml::get<toml::Boolean>(t.at("log_reg_steps"));
    
    //this is not in the TOML file - it's used to name the output directories ("run0", "run1", etc.)
    this->file_index = file_index;
}

Runs::Runs() {
}

void Runs::get_runs(vector<Run> *runs) {
    ifstream ifs(RUN_FILE);
    toml::Data data = toml::parse(ifs);
    vector<toml::Table> tables = toml::get<toml::Array<toml::Table>>(data.at("runs"));

    for (int i = 0; i < (int) tables.size(); i++) {
        runs->push_back(Run(tables[i], i));
    }
}
