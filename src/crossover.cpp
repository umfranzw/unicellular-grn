#include "crossover.hpp"

int Crossover::get_fittest(Run *run, vector<float> *fitnesses) {
    int min_index = 0;
    float min_val = (*fitnesses)[min_index]; //safe to assume we have pop_size of at least 1

    for (int i = 1; i < run->pop_size; i++) {
        if ((*fitnesses)[i] < min_val) {
            min_index = i;
            min_val = (*fitnesses)[i];
        }
    }

    return min_index;
}

float Crossover::sum_fitnesses(Run *run, vector<float> *fitnesses) {
    float sum = 0.0f;

    for (int i = 0; i < run->pop_size; i++) {
        sum += (*fitnesses)[i];
    }

    return sum;
}

vector<pair<int, int>> Crossover::select(Run *run, vector<float> *fitnesses) {
    int num_cross = (int) (run->cross_frac * run->pop_size);
    num_cross = max(num_cross - (num_cross % 2), 0);  //ensure it's even

    //build the roulette wheel
    float fit_sum = sum_fitnesses(run, fitnesses);
    //note: last slot is not needed because we are recording boundaries (it will always be 1.0)
    vector<float> wheel;
    float acc = 0.0f;
    for (int i = 0; i < run->pop_size - 1; i++) {
        float slice = 1.0 - (*fitnesses)[i] / fit_sum; //note: we're minimizing
        float bound = acc + slice;
        wheel.push_back(bound);
        acc = bound;
    }

    //pick the individuals using the wheel (with replacement)
    vector<pair<int, int>> parents;
    for (int i = 0; i < num_cross / 2; i++) {
        pair<int, int> couple = pair<int, int>(Crossover::spin_wheel(run, &wheel), Crossover::spin_wheel(run, &wheel));
        parents.push_back(couple);
    }

    return parents;
}

int Crossover::spin_wheel(Run *run, vector<float> *wheel) {
    float spin = run->rand.next_float();
    int i = 0;
    while (i < (int) wheel->size() && spin > (*wheel)[i]) {
        i++;
    }

    return i;
}

void Crossover::crossover(Run *run, vector<Grn*> *pop, vector<float> *fitnesses) {
    vector<pair<int, int>> parents = Crossover::select(run, fitnesses);
    
    //note: can't do in-place swap of individuals in existing population because selection is done with replacement (same individual might be selected more than once)
    vector<Grn*> new_pop;
    for (pair<int, int> couple : parents) {
        Grn *parent0 = (*pop)[couple.first];
        Grn *parent1 = (*pop)[couple.second];

        int gene_split_pt = run->rand.in_range(1, (int) run->num_genes + 1); //in [1, num_genes]
        int protein_split_pt = run->rand.in_range(1, (int) run->initial_proteins + 1); //in [1, initial_proteins]

        //build the children

        //child 0
        vector<Gene*>::iterator left_genes = parent0->genes.begin(); //length is gene_split_pt
        vector<Gene*>::iterator right_genes = parent1->genes.begin() + gene_split_pt; //length is (num_genes - gene_split_pt)
        vector<Gene*> child0_genes = Crossover::build_child_genes(run, left_genes, gene_split_pt, right_genes, run->num_genes - gene_split_pt);
        
        vector<Protein*>::iterator left_proteins = parent0->initial_proteins.begin();
        vector<Protein*>::iterator right_proteins = parent0->initial_proteins.begin() + protein_split_pt;
        vector<Protein*> child0_init_proteins = Crossover::build_child_init_proteins(run, left_proteins, protein_split_pt, right_proteins, run->initial_proteins - protein_split_pt);
        
        Grn *child0 = new Grn(run, child0_genes, child0_init_proteins);

        //child 1
        left_genes = parent0->genes.begin() + gene_split_pt; //length is (num_genes - gene_split_pt)
        right_genes = parent1->genes.begin(); //length is gene_split_pt
        vector<Gene*> child1_genes = Crossover::build_child_genes(run, right_genes, gene_split_pt, left_genes, run->num_genes - gene_split_pt);

        left_proteins = parent0->initial_proteins.begin() + protein_split_pt;
        right_proteins = parent1->initial_proteins.begin();
        vector<Protein*> child1_init_proteins = Crossover::build_child_init_proteins(run, right_proteins, protein_split_pt, left_proteins, run->initial_proteins - protein_split_pt);
        
        Grn *child1 = new Grn(run, child1_genes, child1_init_proteins);

        //insert children into new population
        new_pop.push_back(child0);
        new_pop.push_back(child1);
    }

    //preserve the fittest individual ("elitism")
    int best_index = Crossover::get_fittest(run, fitnesses); //index in old pop
    Grn *copy = new Grn((*pop)[best_index]); //make sure we push a *copy* (so we use the copy constructor to copy the store and all the proteins it contains). Otherwise we'll get double free() errors...

    //if there's room, append
    if ((int) new_pop.size() < run->pop_size) {
        new_pop.push_back(copy); 
    }
    //otherwise, we have to sacrifice a child...
    else {
        Grn *child = new_pop.back();
        new_pop.pop_back();
        delete child;
        new_pop.push_back(copy);
    }
    //fill out the rest of the new_pop with new Grns, randomly generated
    while ((int) new_pop.size() < run->pop_size) {
        new_pop.push_back(new Grn(run));
    }

    //replace old pop
    for (Grn *old_grn : *pop) {
        delete old_grn;
    }
    //note: vector overloads the assignment (=) op, so doing this does not destroy the vector that pop points to (the old population vector), it just destroys its elements, then copies
    //the *contents* of new_pop into it.
    //note also that the vectors contain *pointers* to Grns, so the copying is relatively cheap.
    *pop = new_pop;
}

vector<Protein *> Crossover::build_child_init_proteins(Run *run, vector<Protein*>::iterator left, int left_len, vector<Protein*>::iterator right, int right_len) {
    vector<Protein*> proteins;
    int mix_len = min(left_len, right_len);
    for (int i = 0; i < mix_len; i++) {
        //here, left parent's first part is first in child
        Protein *protein = Crossover::cross_proteins(run, *(left + i), *(right + i));
        proteins.push_back(protein);
    }

    for (int i = 0; i < mix_len; i++) {
        Protein *protein = Crossover::cross_proteins(run, *(right + i), *(left + i));
        proteins.push_back(protein);
    }

    //fill in the rest with proteins from the longer of the two slices, starting after the mixpoint
    int j = 0;
    vector<Protein*>::iterator longer = left_len > right_len ? left : right;
    while ((int) proteins.size() < run->initial_proteins) {
        Protein *copy = new Protein(*(longer + mix_len + j));
        proteins.push_back(copy);
        j++;
    }

    return proteins;
}

vector<Gene*> Crossover::build_child_genes(Run *run, vector<Gene*>::iterator left, int left_len, vector<Gene*>::iterator right, int right_len) {
    vector<Gene*> genes;
    int mix_len = min(left_len, right_len);
    for (int i = 0; i < mix_len; i++) {
        //here, left parent's first part is first in child
        Gene *gene = Crossover::cross_genes(run, *(left + i), *(right + i), i);
        genes.push_back(gene);
    }

    //note: mix_len is at most num_genes / 2, so doing this a second time is safe
    //note the reverse order of the params, which causes right parent's first part to be first in child
    //this also causes the bitvecs to be mixed in the opposite way
    for (int i = 0; i < mix_len; i++) {
        Gene *gene = Crossover::cross_genes(run, *(right + i), *(left + i), mix_len + i);
        genes.push_back(gene);
    }

    //fill in the rest with genes from the longer of the two slices, starting after the mixpoint
    int j = 0;
    vector<Gene*>::iterator longer = left_len > right_len ? left : right;
    while ((int) genes.size() < run->num_genes) {
        Gene *copy = new Gene(*(longer + mix_len + j));
        copy->pos = mix_len * 2 + j; //the copy will have the wrong position. Update it.
        genes.push_back(copy);
        j++;
    }

    return genes;
}

Gene *Crossover::cross_genes(Run *run, Gene *left, Gene *right, int pos) {
    BitVec *binding_seq = Crossover::cross_bitsets(run, left->binding_seq, right->binding_seq);
    BitVec *output_seq = Crossover::cross_bitsets(run, left->output_seq, right->output_seq);
    
    return new Gene(
        run,
        binding_seq,
        output_seq,
        Crossover::cross_primitives<float>(left->threshold, right->threshold, 0.0f, run->max_protein_conc),
        Crossover::cross_primitives<float>(left->output_rate, right->output_rate, 0.0f, run->max_protein_conc),
        left->kernel_index, //stick with the existing kernel index for this gene (doesn't make sense to average them or anything like that...)
        pos
        );
}

Protein *Crossover::cross_proteins(Run *run, Protein *left, Protein *right) {
    BitVec *seq = Crossover::cross_bitsets(run, left->seq, right->seq);
    vector<float> concs;
    float val;
    for (int i = 0; i < run->num_genes; i++) {
        val = Crossover::cross_primitives<float>(left->concs[i], right->concs[i], 0.0f, run->max_protein_conc);
        concs.push_back(val);
    }

    int src_pos = Crossover::cross_primitives<int>(left->src_pos, right->src_pos, 0, run->num_genes);

    return new Protein(
        run,
        seq,
        concs,
        left->kernel_index, //just choose arbitrarily for this one (I'm assuming averaging won't work great)
        src_pos
        );
}

BitVec *Crossover::cross_bitsets(Run *run, BitVec *left, BitVec *right) {
    int left_bits = run->rand.in_range(1, run->gene_bits);
    int right_bits = run->gene_bits - left_bits;

    //take first part of left and second part of right (if you want first part of right and second of left, just switch the left and right args you pass in)
    //first, take left and zero out the rightmost bits
    BitVec left_copy = BitVec(*left);
    left_copy >>= run->gene_bits - left_bits;
    left_copy <<= run->gene_bits - left_bits;

    //second, take right and zero out the leftmost bits
    BitVec right_copy = BitVec(*right);
    right_copy >>= run->gene_bits - right_bits;
    right_copy <<= run->gene_bits - right_bits;

    //finally, put them together
    return new BitVec(left_copy | right_copy);
}
