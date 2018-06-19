#ifndef _GENETIC_OP_HPP
#define _GENETIC_OP_HPP

#include <vector>
#include "runs.hpp"
#include "grn.hpp"

class GeneticOp {
public:
    GeneticOp(Run *run);
    virtual void run_op(vector<Grn*> *pop, vector<float> *fitnesses);

protected:
    Run *run;
};

#endif
