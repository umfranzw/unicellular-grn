#include "ga.hpp"
#include "kernels.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include "mutation.hpp"
#include "crossover.hpp"
#include "sine_evalor.hpp"
#include "genetic_op.hpp"
#include <cmath>
#include <omp.h>

Ga::Ga(Run *run) {
    this->run = run;
    
    this->logger = new Logger(run);

    //initialize population (randomly)
    for (int i = 0; i < run->pop_size; i++) {
        this->pop.push_back(new Grn(run));
        this->fitnesses.push_back(0.0f);
    }
}

Ga::~Ga() {
    delete this->logger;

    for (Grn *grn : this->pop) {
        delete grn;
    }
}

void Ga::print_pop() {
    for (Grn *grn : this->pop) {
        cout << grn->to_str() << endl;
    }
}

void Ga::run_alg() {
    vector<GeneticOp> gen_ops;
    gen_ops.push_back(Crossover(this->run));
    gen_ops.push_back(Mutation(this->run));
    SineEvalor evalor = SineEvalor(this->run, this->logger);
    
    this->logger->log_run(); //log the parameters used in this run
    this->logger->log_ga_step(-1, &this->pop); //log initial grns using ga_step = -1
    
    evalor.update_fitness(&this->pop, &this->fitnesses, -1); //this will log initial reg sim using ga_step = -1
    this->logger->log_fitnesses(-1, &this->pop, &this->fitnesses); //and finally the fitnesses
    Crossover cross(this->run);
    Mutation mutate(this->run);

    for (int i = 0; i < this->run->ga_steps; i++) {
        //run all of the genetic operators (in the order they were inserted into the vector)
        for (GeneticOp& op : gen_ops) {
            op.run_op(&this->pop, &this->fitnesses);
        }

        this->logger->log_ga_step(i, &this->pop);

        evalor.update_fitness(&this->pop, &this->fitnesses, i); //this will log the reg sim

        this->logger->log_fitnesses(i, &this->pop, &this->fitnesses);

        this->adjust_params(this->run, i);
    }

    this->logger->print_run_best_grn();

    this->logger->write_db();
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
    else { //treat limit as a lower bound
        cur_val = max(cur_val + step, limit);
    }

    return cur_val;
}
