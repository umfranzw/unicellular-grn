#include "protein.hpp"
#include "kernels.hpp"
#include "utils.hpp"

#include <sstream>

Protein::Protein(Run *run, boost::dynamic_bitset<> seq, vector<float> concs, int kernel_index, int src_pos) {
    this->run = run;
    this->seq = seq;
    this->concs = concs;
    this->kernel_index = kernel_index;
    this->src_pos = src_pos;
}

//randomly initializes all the components that are not passed in
Protein::Protein(Run *run, int src_pos) {
    this->run = run;
    this->src_pos = src_pos;
    Utils::fill_rand(&this->seq, run->gene_bits, run);
    Utils::fill_rand(&this->concs, run->num_genes, run);
    this->kernel_index = run->rand.in_range(0, KERNELS.size());
}

string Protein::to_str() {
    stringstream info;

    info << "Protein:" << endl;
    info << "  seq: " << this->seq << endl;
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
