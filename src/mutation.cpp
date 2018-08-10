#include "mutation.hpp"
#include "utils.hpp"
#include "kernels.hpp"

Mutation::Mutation(Run *run) : GeneticOp(run) {
}

Mutation::~Mutation() {
}

void Mutation::run_op(vector<Grn*> *pop, vector<float> *fitnesses) {
//    #pragma omp parallel for
    for (int i = 0; i < this->run->pop_size; i++) {
        Grn *grn = (*pop)[i];
        for (int j = 0; j < this->run->num_genes; j++) {
            Gene *gene = grn->genes[j];
            this->mutate_bitset(gene->binding_seq);
            this->mutate_bitset(gene->output_seq);
            this->mutate_float(&gene->threshold, 0.0f, this->run->max_protein_conc);
            this->mutate_float(&gene->output_rate, 0.0f, this->run->max_protein_conc);
            this->mutate_int(&gene->kernel_index, 0, (int) KERNELS.size());
        }
        this->mutate_initial_proteins(&grn->initial_proteins);
    }
}

void Mutation::mutate_initial_proteins(vector<Protein*> *proteins) {
    for (Protein *p : *proteins) {
        this->mutate_bitset(p->seq);
        for (int i = 0; i < this->run->num_genes; i++) {
            this->mutate_float(&p->concs[i], 0.0f, this->run->max_protein_conc);
        }
        this->mutate_int(&p->kernel_index, 0, (int) KERNELS.size());
        this->mutate_int(&p->src_pos, 0, this->run->num_genes);
    }
}

void Mutation::mutate_int(int *val, int lower, int upper) {
    if (this->run->rand.next_float() < this->run->mut_prob) {
        *val = this->run->rand.in_range(lower, upper);
    }
}

void Mutation::mutate_float(float *val, float lower, float upper) {
    if (this->run->rand.next_float() < this->run->mut_prob) { 
        float eps = this->run->rand.in_range(-this->run->max_mut_float, this->run->max_mut_float);
        *val = Utils::clamp(*val + eps, lower, upper);
    }
}

void Mutation::mutate_bitset(BitVec *bits) {
    int i = 0;
    int count = 0;
    while (i < this->run->gene_bits && count < this->run->max_mut_bits) {
        if (this->run->rand.next_float() < this->run->mut_prob) {
            (*bits)[i] = ~((*bits)[i]);
        }
        i++;
    }
}
