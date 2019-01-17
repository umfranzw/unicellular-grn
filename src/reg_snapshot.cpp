#include "reg_snapshot.hpp"
#include "node.hpp"

//Note: this object is only intended to live for a single ga_step
RegSnapshot::RegSnapshot(Grn *grn, int ga_step, int pop_index) {
    this->grn = grn;
    this->ga_step = ga_step;
    this->pop_index = pop_index;
}

RegSnapshot::~RegSnapshot() {
    for (auto it = this->grns.begin(); it != this->grns.end(); it++) {
        delete it->second;
        delete this->ptypes[it->first];
    }
}

void RegSnapshot::add_reg_step(int reg_step, Phenotype *ptype) {
    Grn *grn_copy = new Grn(this->grn, true, true);
    Phenotype *ptype_copy = new Phenotype(ptype);
    this->grns[reg_step] = grn_copy;
    this->ptypes[reg_step] = ptype_copy;
}
