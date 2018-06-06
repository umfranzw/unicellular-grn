#include "protein.hpp"
#include "kernels.hpp"
#include "utils.hpp"

#include <sstream>

Protein::Protein(Run *run, BitVec *seq, vector<float> concs, int kernel_index, int src_pos) {
    this->run = run;
    this->seq = new BitVec(*seq);
    this->concs = concs;
    this->kernel_index = kernel_index;
    this->src_pos = src_pos;
}

//randomly initializes all the components that are not passed in
Protein::Protein(Run *run, int src_pos) {
    this->run = run;
    this->src_pos = src_pos;
    this->seq = new BitVec(this->run->gene_bits);
    Utils::fill_rand(this->seq, run->gene_bits, run);
    Utils::fill_rand(&this->concs, run->num_genes, run);
    this->kernel_index = run->rand.in_range(0, KERNELS.size());
}

Protein::Protein(Protein *protein) { //copy constructor
    this->run = protein->run;
    this->seq = new BitVec(*protein->seq);
    this->concs = protein->concs;
    this->kernel_index = protein->kernel_index;
    this->src_pos = protein->src_pos;
}

Protein::~Protein() {
    delete this->seq;
}

string Protein::to_str() {
    stringstream info;

    info << "Protein:" << endl;
    info << "  seq: " << this->seq->to_str() << endl;
    info << "  kernel_index: " << this->kernel_index << endl;
    info << "  src_pos: " << this->src_pos << endl;
    info << "  concs: [";
    for (size_t i = 0; i < this->concs.size(); i++) {
        info << this->concs[i];
        if (i < this->concs.size() - 1) {
            info << ", ";
        }
    }
    info << "]" << endl;

    return info.str();
}
