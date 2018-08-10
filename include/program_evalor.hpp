#ifndef _PROGRAM_EVALOR_HPP
#define _PROGRAM_EVALOR_HPP

#include "grn.hpp"
#include "runs.hpp"
#include "logger.hpp"
#include "evaluator.hpp"
#include "phenotype.hpp"
#include "bitvec.hpp"

class ProgramEvalor : public Evaluator {
public:
    ProgramEvalor(Run *run, Logger *logger);
    ~ProgramEvalor();
    void update_fitness(vector<Grn*> *pop, vector<float> *fitnesses, vector<Phenotype*> *phenotypes, int ga_step);
    void grow_step(Grn *grn, Phenotype *ptype, int reg_step);
    void code_step(Grn *grn, Phenotype *ptype, int reg_step);
    float eval(Grn *grn, Phenotype *ptype);
};

#endif
