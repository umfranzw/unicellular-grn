#ifndef _GENETIC_OP_HPP
#define _GENETIC_OP_HPP

#include <vector>
#include "runs.hpp"
#include "grn.hpp"
#include "logger.hpp"

class GeneticOp {
public:
    GeneticOp(Run *run);
    virtual ~GeneticOp();
    virtual void run_op(vector<Grn*> *pop, vector<float> *fitnesses, int ga_step, Logger *logger) = 0;

protected:
    Run *run;
};

#endif
