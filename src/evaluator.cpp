#include "evaluator.hpp"

Evaluator::Evaluator(Run *run, Logger *logger) {
    this->run = run;
    this->logger = logger;
}

void Evaluator::update_fitness(vector<Grn*> *pop, vector<float> *fitnesses, int ga_step) {
    //do regulatory simulation
    #pragma omp parallel for
    for (int i = 0; i < this->run->pop_size; i++) {
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
        (*fitnesses)[i] = this->eval(grn);
    }

    //TODO: translate final grn proteins to phenotype

    //TODO: run phenotype through fitness function
}

//subclasses should override this
float Evaluator::eval(Grn *grn) {
    return 0.0f;
}
