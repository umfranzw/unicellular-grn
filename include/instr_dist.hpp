#ifndef _INSTR_DIST_HPP
#define _INSTR_DIST_HPP

#include "runs.hpp"
#include "bitvec.hpp"
#include "protein.hpp"
#include "instr.hpp"
#include "instr_factory.hpp"
#include <map>

using namespace std;

class InstrDist {
public:
    InstrDist(Run *run, InstrFactory *factory, map<Protein*, float, bool(*)(Protein*, Protein*)> *buckets);
    InstrDist(InstrDist *other);
    ~InstrDist();
    pair<Protein*, Instr *> sample();
    string to_str();

private:
    void update_prob_dist();
    
    Run *run;
    map<Protein*, float, bool(*)(Protein*, Protein*)> *buckets;
    InstrFactory *factory;
};

#endif
