#ifndef _FITNESS_HPP
#define _FITNESS_HPP

#include <vector>
#include "runs.hpp"
#include "logger.hpp"
#include "protein.hpp"
#include "grn.hpp"

class Fitness {
public:
    static void update_fitness(Run *run, Logger *logger,  vector<Grn*> *pop, vector<float> *fitnesses, int ga_step);
    
private:
    static float calc_fitness(Run *run, Grn *grn);
    static float calc_protein_error(Run *run, Protein *protein);
};

#endif
