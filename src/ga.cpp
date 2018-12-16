#include "ga.hpp"
#include "kernels.hpp"
#include "utils.hpp"
#include "mutation.hpp"
#include "crossover.hpp"
#include "program_evalor.hpp"
#include "genetic_op.hpp"
#include "vis_adapter.hpp"
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <array>

Ga::Ga(Run *run) {
    this->run = run;
    
    this->logger = new Logger(run);
    this->evalor = new ProgramEvalor(run, logger);
    this->gen_ops.push_back(new Crossover(this->run));
    this->gen_ops.push_back(new Mutation(this->run));

    //initialize population (randomly)
    for (int i = 0; i < run->pop_size; i++) {
        this->pop.push_back(new Grn(run));
        this->fitnesses.push_back(0.0f);
        this->phenotypes.push_back(new Phenotype(run));
    }
}

Ga::~Ga() {
    delete this->logger;
    delete this->evalor;
    for (int i = 0; i < (int) this->gen_ops.size(); i++) {
        delete this->gen_ops[i];
    }

    for (int i = 0; i < this->run->pop_size; i++) {
        delete this->pop[i];
        delete this->phenotypes[i];
    }
}

void Ga::print_pop() {
    for (Grn *grn : this->pop) {
        cout << grn->to_str() << endl;
    }
}

void Ga::run_alg() {
    this->logger->log_run(); //log the parameters used in this run
    this->logger->log_ga_step(-1, &this->pop); //log initial grns using ga_step = -1
    
    this->evalor->update_fitness(&this->pop, &this->fitnesses, &this->phenotypes, -1); //this will log initial reg sim using ga_step = -1
    this->logger->log_fitnesses(-1, &this->pop, &this->phenotypes, &this->fitnesses); //and finally the fitnesses

    int i = 0;
    while (i < this->run->ga_steps && this->logger->get_run_best_fitness() > this->run->term_cutoff) {
        //run all of the genetic operators (in the order they were inserted into the vector)
        for (GeneticOp *op : this->gen_ops) {
            op->run_op(&this->pop, &this->fitnesses);
        }

        this->logger->log_ga_step(i, &this->pop);

        this->evalor->update_fitness(&this->pop, &this->fitnesses, &this->phenotypes, i); //this will log the reg sim

        this->logger->log_fitnesses(i, &this->pop, &this->phenotypes, &this->fitnesses);

        this->adjust_params(this->run, i);

        i++;
    }

    this->logger->print_results(i);

    this->logger->write_db();

    this->graph_results();
}

void Ga::graph_results() {
    if (this->run->graph_results) {
        cout << endl;
        
        #if HAVE_ZMQ
        cout << "Generating graphs..." << endl;
        cout.flush();
        VisAdapter adapter = VisAdapter(this->run, this->logger->conn);
        adapter.listen();
        cout << "done." << endl;
        
        #else
        cout << "Cannot generate graphs - no zmq library." << endl;
        cout.flush();
        #endif
    }
}

void Ga::adjust_params(Run *run, int ga_step) {
    static float prev_avg = numeric_limits<float>::max();
    float cur_avg = this->logger->get_avg_fitness(ga_step);

    if (cur_avg <= prev_avg) {
        prev_avg = cur_avg;
    }
    else {
        this->run->mut_prob = this->clamp_param(this->run->mut_prob, this->run->mut_step, this->run->mut_prob_limit);
        
        this->run->cross_frac = this->clamp_param(this->run->cross_frac, this->run->cross_step, this->run->cross_frac_limit);
    }
}

float Ga::clamp_param(float cur_val, float step, float limit) {
    if (step > 0) { //treat limit as an upper bound
        cur_val = min(cur_val + step, limit);
    }
    else if (step < 0) { //treat limit as a lower bound
        cur_val = max(cur_val + step, limit);
    }

    return cur_val;
}
