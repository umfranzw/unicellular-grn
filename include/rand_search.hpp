#ifndef _RAND_SEARCH_HPP
#define _RAND_SEARCH_HPP

#define MAX_NODES 10

#include "runs.hpp"
#include "instr_factory.hpp"
#include "phenotype.hpp"
#include "best_info.hpp"

using namespace std;

class RandSearch {
public:
    RandSearch(Run *run);
    ~RandSearch();
    void run_alg();

private:
    void randomize_ptype(Phenotype *ptype);
        
    Run *run;
    InstrFactory *instr_factory;
    vector<Phenotype*> pop;
    vector<float> fitnesses;
    BestInfo<Phenotype> bests;
};

#endif
