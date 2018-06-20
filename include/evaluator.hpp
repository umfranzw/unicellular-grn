#ifndef _EVALUATOR_HPP
#define _EVALUATOR_HPP

#include "runs.hpp"
#include "logger.hpp"
#include "grn.hpp"
#include <vector>

class Evaluator {
public:
    Evaluator(Run *run, Logger *logger);
    virtual ~Evaluator();
    void update_fitness(vector<Grn*> *pop, vector<float> *fitnesses, int ga_step);
    virtual float eval(Grn *grn) = 0;

protected:
    Run *run;
    Logger *logger;
};

#endif
