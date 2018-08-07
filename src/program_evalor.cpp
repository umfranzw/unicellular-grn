#include "program_evalor.hpp"

ProgramEvalor::ProgramEvalor(Run* run, Logger* logger) : Evaluator(run, logger) {
}

ProgramEvalor::~ProgramEvalor() {
}

void ProgramEvalor::update_fitness(vector<Grn*> *pop, vector<float> *fitnesses, vector<Phenotype*> *phenotypes, int ga_step) {
    //do regulatory simulation
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

            this->grow_step(grn, (*phenotypes)[i], j);
        }

        //update fitness value
        (*fitnesses)[i] = this->eval(grn, (*phenotypes)[i]);
    }
}

void ProgramEvalor::grow_step(Grn *grn, Phenotype *ptype, int reg_step) {
    bool growing = reg_step >= this->run->growth_start && reg_step <= this->run->growth_end;
    bool sampling = (reg_step - this->run->growth_start) % this->run->growth_sample_interval == 0;
    
    if (growing && sampling) {
        const static BitVec growth_seq(this->run->growth_seq);
        vector<Protein*> proteins = grn->proteins->get_all((const BitVec*) &growth_seq);

        map<int, bool> grow_indices;
        for (Protein *p : proteins) {
            for (int i = 0; i < this->run->num_genes; i++) {
                if (p->concs[i] > this->run->growth_threshold) {
                    grow_indices.insert(pair<int, bool>(i, true));
                }
            }
        }

        //note: map is sorted by keys by default
        for (auto it = grow_indices.begin(); it != grow_indices.end(); it++) {
            int grow_index = it->first;
            ptype->add_child(grow_index);
        }
    }
}

float ProgramEvalor::eval(Grn* grn, Phenotype *ptype) {
    const float target_bf = 2.0f;
    const int target_size = 7;
    const int target_height = 3; //root is at height 1
    const float scale_factor = 10.0f;
    
    float fitness = 0.0f;
    float bf = ptype->branching_factor();
    if (bf < 0) { //no nodes
        //penalize slightly more than the max bf penalty in the else clause below
        fitness += (target_bf * scale_factor) + scale_factor;
    }
    else {
        fitness += abs(target_bf - ptype->branching_factor()) * scale_factor;
    }
    fitness += abs(target_height - ptype->height()) * scale_factor;
    fitness += abs(target_size - ptype->size()) * scale_factor;

    return fitness;
}
