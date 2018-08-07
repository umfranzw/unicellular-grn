#include "evaluator.hpp"

Evaluator::Evaluator(Run *run, Logger *logger) {
    this->run = run;
    this->logger = logger;
}

Evaluator::~Evaluator() {
}

float Evaluator::update_fitness(vector<Grn*> *pop, vector<float> *fitnesses, vector<Phenotype*> *phenotypes, int ga_step) {
    //do regulatory simulation
    //#pragma omp parallel for
    float best_fitness = -1.0f;
    for (int i = 0; i < this->run->pop_size; i++) {
        (*phenotypes)[i]->reset();
        Grn *grn = (*pop)[i];

        this->logger->log_reg_step(ga_step, -1, grn, i);

        for (int j = 0; j < this->run->reg_steps; j++) {
            grn->run_binding();

            grn->update_output_proteins();

            grn->run_diffusion();

            grn->run_decay();

            this->logger->log_reg_step(ga_step, j, grn, i);
        }

        //update fitness value
        float fitness = this->eval(grn, (*phenotypes)[i]);
        (*fitnesses)[i] = fitness;
        if (i == 0 || fitness < best_fitness) {
            best_fitness = fitness;
        }
    }

    return best_fitness;
}
