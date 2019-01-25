#include "crossover.hpp"
#include "prob_dist.hpp"

Crossover::Crossover(Run *run)
    : GeneticOp(run) {
}

Crossover::~Crossover() {
}

void Crossover::run_op(vector<Grn*> *pop, vector<float> *fitnesses) {
    //note: because we are crossing the initial proteins, and the growth protein is set to the first initial protein,
    //the growth proteins are automatically crossed and we don't have to do it explicitly
    
    vector<pair<int, int>> parents = this->select(fitnesses);
    
    //note: can't do in-place swap of individuals in existing population because selection is done with replacement (same individual might be selected more than once)
    vector<Grn*> new_pop;
    for (pair<int, int> couple : parents) {
        Grn *parent0 = (*pop)[couple.first];
        Grn *parent1 = (*pop)[couple.second];

        int gene_split_pt = this->run->rand->in_range(1, (int) this->run->num_genes + 1); //in [1, num_genes]
        int protein_split_pt = this->run->rand->in_range(1, (int) this->run->initial_proteins + 1); //in [1, initial_proteins]

        //build the children

        //child 0
        vector<Gene*>::iterator left_genes = parent0->genes.begin(); //length is gene_split_pt
        vector<Gene*>::iterator right_genes = parent1->genes.begin() + gene_split_pt; //length is (num_genes - gene_split_pt)
        vector<Gene*> child0_genes = this->build_child_genes(left_genes, gene_split_pt, right_genes, this->run->num_genes - gene_split_pt);
        
        vector<Protein*>::iterator left_proteins = parent0->initial_proteins.begin();
        vector<Protein*>::iterator right_proteins = parent0->initial_proteins.begin() + protein_split_pt;
        vector<Protein*> child0_init_proteins = this->build_child_init_proteins(left_proteins, protein_split_pt, right_proteins, this->run->initial_proteins - protein_split_pt);
        
        Grn *child0 = new Grn(this->run, child0_genes, child0_init_proteins);

        //child 1
        left_genes = parent0->genes.begin() + gene_split_pt; //length is (num_genes - gene_split_pt)
        right_genes = parent1->genes.begin(); //length is gene_split_pt
        vector<Gene*> child1_genes = this->build_child_genes(right_genes, gene_split_pt, left_genes, this->run->num_genes - gene_split_pt);

        left_proteins = parent0->initial_proteins.begin() + protein_split_pt;
        right_proteins = parent1->initial_proteins.begin();
        vector<Protein*> child1_init_proteins = this->build_child_init_proteins(right_proteins, protein_split_pt, left_proteins, this->run->initial_proteins - protein_split_pt);
        
        Grn *child1 = new Grn(this->run, child1_genes, child1_init_proteins);

        //insert children into new population
        new_pop.push_back(child0);
        new_pop.push_back(child1);
    }

    //preserve the fittest individual ("elitism")
    int best_index = this->get_fittest(fitnesses); //index in old pop
    Grn *copy = new Grn((*pop)[best_index]); //make sure we push a *copy* (so we use the copy constructor to copy the store and all the proteins it contains). Otherwise we'll get double free() errors...

    //if there's room, append
    if ((int) new_pop.size() < this->run->pop_size) {
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
    while ((int) new_pop.size() < this->run->pop_size) {
        new_pop.push_back(new Grn(this->run));
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

int Crossover::get_fittest(vector<float> *fitnesses) {
    int min_index = 0;
    float min_val = (*fitnesses)[min_index]; //safe to assume we have pop_size of at least 1

    for (int i = 1; i < this->run->pop_size; i++) {
        if ((*fitnesses)[i] < min_val) {
            min_index = i;
            min_val = (*fitnesses)[i];
        }
    }

    return min_index;
}

// float Crossover::sum_fitnesses(vector<float> *fitnesses) {
//     float sum = 0.0f;

//     for (int i = 0; i < this->run->pop_size; i++) {
//         sum += (*fitnesses)[i];
//     }

//     return sum;
// }

vector<pair<int, int>> Crossover::select(vector<float> *fitnesses) {
    int num_cross = (int) (this->run->cross_frac * this->run->pop_size);
    num_cross = max(num_cross - (num_cross % 2), 0);  //ensure it's even

    ProbDist dist = ProbDist(this->run, fitnesses, true, true);

    vector<pair<int, int>> parents;
    for (int i = 0; i < num_cross / 2; i++) {
        pair<int, int> couple = pair<int, int>(dist.sample(), dist.sample());
        parents.push_back(couple);
    }

    return parents;
}

vector<Protein *> Crossover::build_child_init_proteins(vector<Protein*>::iterator left, int left_len, vector<Protein*>::iterator right, int right_len) {
    vector<Protein*> proteins;
    int mix_len = min(left_len, right_len);
    for (int i = 0; i < mix_len; i++) {
        //here, left parent's first part is first in child
        Protein *protein = this->cross_proteins(*(left + i), *(right + i));
        proteins.push_back(protein);
    }

    for (int i = 0; i < mix_len; i++) {
        Protein *protein = this->cross_proteins(*(right + i), *(left + i));
        proteins.push_back(protein);
    }

    //fill in the rest with proteins from the longer of the two slices, starting after the mixpoint
    int j = 0;
    vector<Protein*>::iterator longer = left_len > right_len ? left : right;
    while ((int) proteins.size() < this->run->initial_proteins) {
        Protein *copy = new Protein(*(longer + mix_len + j));
        proteins.push_back(copy);
        j++;
    }

    return proteins;
}

vector<Gene*> Crossover::build_child_genes(vector<Gene*>::iterator left, int left_len, vector<Gene*>::iterator right, int right_len) {
    vector<Gene*> genes;
    int mix_len = min(left_len, right_len);
    for (int i = 0; i < mix_len; i++) {
        //here, left parent's first part is first in child
        Gene *gene = this->cross_genes(*(left + i), *(right + i), i);
        genes.push_back(gene);
    }

    //note: mix_len is at most num_genes / 2, so doing this a second time is safe
    //note the reverse order of the params, which causes right parent's first part to be first in child
    //this also causes the bitvecs to be mixed in the opposite way
    for (int i = 0; i < mix_len; i++) {
        Gene *gene = this->cross_genes(*(right + i), *(left + i), mix_len + i);
        genes.push_back(gene);
    }

    //fill in the rest with genes from the longer of the two slices, starting after the mixpoint
    int j = 0;
    vector<Gene*>::iterator longer = left_len > right_len ? left : right;
    while ((int) genes.size() < this->run->num_genes) {
        Gene *copy = new Gene(*(longer + mix_len + j));
        copy->pos = mix_len * 2 + j; //the copy will have the wrong position. Update it.
        genes.push_back(copy);
        j++;
    }

    return genes;
}

Gene *Crossover::cross_genes(Gene *left, Gene *right, int pos) {
    BitVec *binding_seq = this->cross_bitsets(left->binding_seq, right->binding_seq);
    BitVec *output_seq = this->cross_bitsets(left->output_seq, right->output_seq);
    
    return new Gene(
        this->run,
        binding_seq,
        output_seq,
        this->cross_primitives<float>(left->threshold, right->threshold, 0.0f, this->run->max_protein_conc),
        this->cross_primitives<float>(left->output_rate, right->output_rate, 0.0f, this->run->max_protein_conc),
        pos
        );
}

Protein *Crossover::cross_proteins(Protein *left, Protein *right) {
    BitVec *seq = this->cross_bitsets(left->seq, right->seq);
    vector<float> concs;
    float val;
    for (int i = 0; i < this->run->num_genes; i++) {
        val = this->cross_primitives<float>(left->concs[i], right->concs[i], 0.0f, this->run->max_protein_conc);
        concs.push_back(val);
    }

    int src_pos = this->cross_primitives<int>(left->src_pos, right->src_pos, 0, this->run->num_genes);

    return new Protein(
        this->run,
        seq,
        concs,
        src_pos
        );
}

BitVec *Crossover::cross_bitsets(BitVec *left, BitVec *right) {
    int left_bits = this->run->rand->in_range(1, this->run->gene_bits);
    int right_bits = this->run->gene_bits - left_bits;

    //take first part of left and second part of right (if you want first part of right and second of left, just switch the left and right args you pass in)
    //first, take left and zero out the rightmost bits
    BitVec left_copy = BitVec(*left);
    left_copy >>= this->run->gene_bits - left_bits;
    left_copy <<= this->run->gene_bits - left_bits;

    //second, take right and zero out the leftmost bits
    BitVec right_copy = BitVec(*right);
    right_copy >>= this->run->gene_bits - right_bits;
    right_copy <<= this->run->gene_bits - right_bits;

    //finally, put them together
    return new BitVec(left_copy | right_copy);
}
