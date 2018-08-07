#ifndef _EVALUATOR_HPP
#define _EVALUATOR_HPP

#include "runs.hpp"
#include "logger.hpp"
#include "grn.hpp"
#include "phenotype.hpp"
#include <vector>

class Evaluator {
public:
    Evaluator(Run *run, Logger *logger);
    virtual ~Evaluator();
    virtual float update_fitness(vector<Grn*> *pop, vector<float> *fitnesses, vector<Phenotype*> *phenotypes, int ga_step);
    virtual float eval(Grn *grn, Phenotype *ptype) = 0;

protected:
    Run *run;
    Logger *logger;
};

#endif
