#ifndef _REG_SNAPSHOT_HPP
#define _REG_SNAPSHOT_HPP

#include "grn.hpp"
#include "phenotype.hpp"
#include "protein_store.hpp"
#include <map>

using namespace std;

class RegSnapshot {
public:
    RegSnapshot(Grn *grn, int ga_step, int pop_index);
    ~RegSnapshot();
    void add_reg_step(int reg_step, Phenotype *ptype);

    int ga_step;
    int pop_index;
    
    map<int, Grn*> grns;
    map<int, Phenotype*> ptypes;

private:
    Grn *grn;
};

#endif
