#include "phenotype.hpp"
#include <sstream>
#include <queue>

Phenotype::Phenotype() {
    this->tree = new Tree();
}

Phenotype::Phenotype(Phenotype *ptype) {
    this->tree = new Tree(ptype->tree);
}

Phenotype::~Phenotype() {
    delete this->tree;
}

bool Phenotype::add_child(int parent_index, Instr *instr) {
    return this->tree->add_child(parent_index, instr);
}

string Phenotype::to_str() {
    stringstream info;
    info << "Phenotype:" << endl;
    info << this->tree->to_str();

    return info.str();
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

void Phenotype::reset() {
    delete this->tree;
    this->tree = new Tree();
}
