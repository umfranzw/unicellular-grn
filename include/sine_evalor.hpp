#ifndef _SINE_EVALOR_HPP
#define _SINE_EVALOR_HPP

#include <vector>
#include "runs.hpp"
#include "logger.hpp"
#include "protein.hpp"
#include "grn.hpp"
#include "evaluator.hpp"

class SineEvalor : public Evaluator {
public:
    SineEvalor(Run *run, Logger *logger);
    float eval(Grn *grn);
    
private:
    float calc_protein_error(Protein *protein);
};

#endif
