#include "phenotype.hpp"
#include <sstream>

Phenotype::Phenotype(Run *run) {
    this->run = run;
    this->tree = new Tree();
}

Phenotype::Phenotype(Phenotype *ptype) {
    this->run = ptype->run;
    this->tree = new Tree(ptype->tree);

    // for (InstrDist *old : ptype->dists) {
    //     this->dists.push_back(new InstrDist(old));
    // }
}

Phenotype::~Phenotype() {
    delete this->tree;
    // for (InstrDist *dist : this->dists) {
    //     delete dist;
    // }
}

bool Phenotype::add_child(int parent_index, Instr *instr) {
    //this->dists.push_back(new InstrDist(this->run));

    return this->tree->add_child(parent_index, instr);
}

int Phenotype::get_num_unfilled_nodes() {
    return this->tree->size() - this->tree->get_num_filled_nodes();
}

int Phenotype::get_num_filled_nodes() {
    return this->tree->get_num_filled_nodes();
}

string Phenotype::to_str() {
    return this->tree->to_str();
}

string Phenotype::to_code() {
    return this->tree->to_code();
}

int Phenotype::size() {
    return this->tree->size();
}

int Phenotype::height() {
    return this->tree->height();
}

float Phenotype::branching_factor() {
    return this->tree->branching_factor();
}

int Phenotype::get_num_children(int id) {
    return this->tree->get_num_children(id);
}

void Phenotype::set_instr(int id, Instr *instr) {    
    this->tree->set_instr(id, instr);
}

void Phenotype::reset() {
    delete this->tree;
    this->tree = new Tree();
    // for (InstrDist *dist : this->dists) {
    //     delete dist;
    // }
    // this->dists.clear();
}

// InstrDist *Phenotype::get_dist(int index) {
//     return this->dists[index];
// }
