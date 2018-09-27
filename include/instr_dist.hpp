#ifndef _INSTR_DIST_HPP
#define _INSTR_DIST_HPP

#include "runs.hpp"
#include "bitvec.hpp"
#include "instr.hpp"
#include "instr_factory.hpp"
#include <map>

using namespace std;

class InstrDist {
public:
    InstrDist(Run *run, InstrFactory *factory, map<BitVec*, float, bool(*)(BitVec*, BitVec*)> *buckets);
    InstrDist(InstrDist *other);
    ~InstrDist();
    Instr *sample();
    string to_str();

private:
    void update_prob_dist();
    
    Run *run;
    map<BitVec*, float, bool(*)(BitVec*, BitVec*)> *buckets;
    InstrFactory *factory;
};

#endif
