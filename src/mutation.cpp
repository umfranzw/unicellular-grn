#include "mutation.hpp"
#include "utils.hpp"
#include "kernels.hpp"

void Mutation::mutate(Run *run, vector<Grn*> *pop) {
    #pragma omp parallel for
    for (int i = 0; i < run->pop_size; i++) {
        Grn *grn = (*pop)[i];
        for (int j = 0; j < run->num_genes; j++) {
            Gene *gene = grn->genes[j];
            Mutation::mutate_bitset(run, gene->binding_seq);
            Mutation::mutate_bitset(run, gene->output_seq);
            Mutation::mutate_float(run, &gene->threshold, 0.0f, run->max_protein_conc);
            Mutation::mutate_float(run, &gene->output_rate, 0.0f, run->max_protein_conc);
            Mutation::mutate_int(run, &gene->kernel_index, 0, (int) KERNELS.size());
        }
        Mutation::mutate_initial_proteins(run, &grn->initial_proteins);
    }
}

void Mutation::mutate_initial_proteins(Run *run, vector<Protein*> *proteins) {
    for (Protein *p : *proteins) {
        Mutation::mutate_bitset(run, p->seq);
        for (int i = 0; i < run->num_genes; i++) {
            Mutation::mutate_float(run, &p->concs[i], 0.0f, run->max_protein_conc);
        }
        Mutation::mutate_int(run, &p->kernel_index, 0, (int) KERNELS.size());
        Mutation::mutate_int(run, &p->src_pos, 0, run->num_genes);
    }
}

void Mutation::mutate_int(Run *run, int *val, int lower, int upper) {
    if (run->rand.next_float() < run->mut_prob) {
        *val = run->rand.in_range(lower, upper);
    }
}

void Mutation::mutate_float(Run *run, float *val, float lower, float upper) {
    if (run->rand.next_float() < run->mut_prob) { 
        float eps = run->rand.in_range(-run->max_mut_float, run->max_mut_float);
        *val = Utils::clamp(*val + eps, lower, upper);
    }
}

void Mutation::mutate_bitset(Run *run, BitVec *bits) {
    int i = 0;
    int count = 0;
    while (i < run->gene_bits && count < run->max_mut_bits) {
        if (run->rand.next_float() < run->mut_prob) {
            (*bits)[i] = ~((*bits)[i]);
        }
        i++;
    }
}
