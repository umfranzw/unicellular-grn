#include "sine_evalor.hpp"

SineEvalor::SineEvalor(Run *run, Logger *logger) : Evaluator(run, logger) {
}

SineEvalor::~SineEvalor() {
}

float SineEvalor::eval(Grn *grn) {
    float max_err = this->run->max_protein_conc * run->num_genes; //concs are in [0.0, max_protein_conc] so max possible error at over gene is max_protein_conc
    float result = max_err;
    
    //use the oldest protein (the one with the lowest id)
    if ((int) grn->proteins->size() > 0) {
        //get lowest id (note: grn.proteins iterator uses map iterator, which is set up to return ids in ascending order);
        int lowest_id = *(grn->proteins->begin());
        Protein *p = grn->proteins->get(lowest_id);
        result = this->calc_protein_error(p);
    }

    return result;
}

float SineEvalor::calc_protein_error(Protein *protein) {
    //sine wave
    float err = 0.0f;

    float factor = 2.0f * M_PI / (float) (this->run->num_genes - 1);
    for (int i = 0; i < this->run->num_genes; i++) {
        err += abs(
            protein->concs[i] - (1.5f + sin(i * factor))
            );
    }
    
    return err;
}
