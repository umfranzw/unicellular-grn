#include "runs.hpp"
#include "constants.hpp"
#include <fstream>
#include <iostream>

Run::Run(toml::Table& t, int file_index) {
    this->rand = new Rand();
    this->pop_size = toml::get<toml::Integer>(t.at("pop_size"));
    this->ga_steps = toml::get<toml::Integer>(t.at("ga_steps"));
    this->reg_steps = toml::get<toml::Integer>(t.at("reg_steps"));
    this->mut_prob = (float) toml::get<toml::Float>(t.at("mut_prob"));
    this->mut_prob_limit = (float) toml::get<toml::Float>(t.at("mut_prob_limit"));
    this->mut_step = (float) toml::get<toml::Float>(t.at("mut_step"));
    this->cross_frac = (float) toml::get<toml::Float>(t.at("cross_frac"));
    this->cross_frac_limit = (float) toml::get<toml::Float>(t.at("cross_frac_limit"));
    this->cross_step = (float) toml::get<toml::Float>(t.at("cross_step"));
    this->num_genes = toml::get<toml::Integer>(t.at("num_genes"));
    this->gene_bits = toml::get<toml::Integer>(t.at("gene_bits"));
    this->min_protein_conc = (float) toml::get<toml::Float>(t.at("min_protein_conc"));
    this->max_protein_conc = (float) toml::get<toml::Float>(t.at("max_protein_conc"));
    this->alpha = (float) toml::get<toml::Float>(t.at("alpha"));
    this->beta = (float) toml::get<toml::Float>(t.at("beta"));
    this->decay_rate = (float) toml::get<toml::Float>(t.at("decay_rate"));
    this->initial_proteins = toml::get<toml::Integer>(t.at("initial_proteins"));
    this->max_proteins = toml::get<toml::Integer>(t.at("max_proteins"));
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

    if (this->initial_proteins > this->max_proteins) {
        cerr << "Error: initial_proteins > max_proteins." << endl;
        cerr << "Setting initial_proteins = max_proteins" << endl;
        this->initial_proteins = this->max_proteins;
    }

    this->graph_results = toml::get<toml::Boolean>(t.at("graph_results"));
    this->log_grns = toml::get<toml::Boolean>(t.at("log_grns"));
    this->log_reg_steps = toml::get<toml::Boolean>(t.at("log_reg_steps"));
    this->log_code_with_fitness = toml::get<toml::Boolean>(t.at("log_code_with_fitness"));

    this->growth_start = toml::get<toml::Integer>(t.at("growth_start"));
    this->growth_end = toml::get<toml::Integer>(t.at("growth_end"));
    this->growth_sample_interval = toml::get<toml::Integer>(t.at("growth_sample_interval"));

    this->growth_seq = toml::get<toml::String>(t.at("growth_seq"));
    this->growth_threshold = toml::get<toml::Float>(t.at("growth_threshold"));

    this->term_cutoff = toml::get<toml::Float>(t.at("term_cutoff"));

    this->code_start = toml::get<toml::Integer>(t.at("code_start"));
    this->code_end = toml::get<toml::Integer>(t.at("code_end"));
    this->code_sample_interval = toml::get<toml::Integer>(t.at("code_sample_interval"));
    
    //this is not in the TOML file - it's used to name the output directories ("run0", "run1", etc.)
    this->file_index = file_index;
}

Run::Run() {
    this->rand = new Rand();
}

Run::~Run() {
    delete this->rand;
}

Runs::Runs() {
    this->index = 0;
    this->size = 0;
    this->parse_runs();
}

Runs::~Runs() {
    for (auto item : this->runs) {
        delete item;
    }
}

void Runs::parse_runs() {
    ifstream ifs(RUN_FILE);
    toml::Data data = toml::parse(ifs);
    vector<toml::Table> tables = toml::get<toml::Array<toml::Table>>(data.at("runs"));

    for (int i = 0; i < (int) tables.size(); i++) {
        this->runs.push_back(new Run(tables[i], i));
    }

    this->size = (int) tables.size();
}

Run* Runs::get_next() {
    Run *run = nullptr;

    if (this->index < (int) this->runs.size()) {
        run = this->runs[this->index];
        this->index++;
    }

    return run;
}
