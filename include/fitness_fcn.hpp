#ifndef _FITNESS_FCN_HPP
#define _FITNESS_FCN_HPP

#include "phenotype.hpp"
#include "program.hpp"

namespace FitnessFcn {
    float eval(Phenotype *ptype, vector<Instr*> *input_params);
    float test_pgm(Program *pgm, vector<Instr*> *input_params);
}

#endif
