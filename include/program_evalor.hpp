#ifndef _PROGRAM_EVALOR_HPP
#define _PROGRAM_EVALOR_HPP

#include "grn.hpp"
#include "runs.hpp"
#include "logger.hpp"
#include "phenotype.hpp"
#include "bitvec.hpp"
#include "instr_factory.hpp"
#include "program.hpp"
#include "best_info.hpp"

class ProgramEvalor {
public:
    ProgramEvalor(Run *run, Logger *logger);
    ~ProgramEvalor();
    void update_fitness(vector<Grn*> *pop, vector<float> *fitnesses, vector<Phenotype*> *phenotypes, int ga_step);
    void grow_step(Grn *grn, Phenotype *ptype, int grn_index, int reg_step, int ga_step);
    void code_step(Grn *grn, Phenotype *ptype, int grn_index, int reg_step, int ga_step);
    float eval(Grn *grn, Phenotype *ptype);
    float test_pgm(Program *pgm);

    BestInfo bests;
    
private:
    Run *run;
    Logger *logger;
    int num_grow_samples;
    int num_code_samples;
    InstrFactory *instr_factory;
};

#endif
