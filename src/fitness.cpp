#include "fitness.hpp"

void Fitness::update_fitness(Run *run, Logger *logger, vector<Grn*> *pop, vector<float> *fitnesses, int ga_step) {
    //do regulatory simulation
    #pragma omp parallel for
    for (int i = 0; i < run->pop_size; i++) {
        Grn *grn = (*pop)[i];

        logger->log_reg_step(ga_step, -1, grn, i);

        for (int j = 0; j < run->reg_steps; j++) {
            grn->run_binding();

            grn->update_output_proteins();

            grn->run_diffusion();

            grn->run_decay();

            logger->log_reg_step(ga_step, j, grn, i);
        }

        //update fitness value
        (*fitnesses)[i] = Fitness::calc_fitness(run, grn);
    }

    //TODO: translate final grn proteins to phenotype

    //TODO: run phenotype through fitness function
}

float Fitness::calc_fitness(Run *run, Grn *grn) {
    float max_err = run->max_protein_conc * run->num_genes; //concs are in [0.0, max_protein_conc] so max possible error at over gene is max_protein_conc
    float result = max_err;
    
    //use the oldest protein (the one with the lowest id)
    if ((int) grn->proteins->size() > 0) {
        //get lowest id (note: grn.proteins iterator uses map iterator, which is set up to return ids in ascending order);
        int lowest_id = *(grn->proteins->begin());
        Protein *p = grn->proteins->get(lowest_id);
        result = Fitness::calc_protein_error(run, p);
    }

    return result;
}

float Fitness::calc_protein_error(Run *run, Protein *protein) {
    //sine wave
    float err = 0.0f;

    float factor = 2.0f * M_PI / (float) (run->num_genes - 1);
    for (int i = 0; i < run->num_genes; i++) {
        err += abs(
            protein->concs[i] - (1.5f + sin(i * factor))
            );
    }
    
    return err;
}
