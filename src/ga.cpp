#include "ga.hpp"
#include "kernels.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include <cmath>

Ga::Ga(Run *run) {
    this->run = run;
    
    this->logger = new Logger(run);

    //initialize population (randomly)
    for (int i = 0; i < run->pop_size; i++) {
        this->pop.push_back(new Grn(run));
        this->fitnesses.push_back(0.0f);
    }
}

Ga::~Ga() {
    delete this->logger;

    for (Grn *grn : this->pop) {
        delete grn;
    }
}

void Ga::print_pop() {
    for (Grn *grn : this->pop) {
        cout << grn->to_str() << endl;
    }
}

void Ga::run_alg() {
    this->logger->log_run(); //log the parameters used in this run
    this->logger->log_ga_step(-1, &this->pop); //log initial grns using ga_step = -1
    
    this->update_fitness(-1); //this will log initial reg sim using ga_step = -1
    this->logger->log_fitnesses(-1, &this->fitnesses); //and finally the fitnesses

    for (int i = 0; i < this->run->ga_steps; i++) {
        vector<pair<int, int>> parents = this->select();
        this->cross(&parents);
        this->mutate();

        this->logger->log_ga_step(i, &this->pop);

        this->update_fitness(i); //this will log the reg sim

        this->logger->log_fitnesses(i, &this->fitnesses);
    }

    this->logger->write_db();
}

int Ga::get_fittest() {
    int min_index = 0;
    float min_val = this->fitnesses[min_index]; //safe to assume we have pop_size of at least 1

    for (int i = 1; i < this->run->pop_size; i++) {
        if (this->fitnesses[i] < min_val) {
            min_index = i;
            min_val = this->fitnesses[i];
        }
    }

    return min_index;
}

float Ga::sum_fitnesses() {
    float sum = 0.0f;

    for (int i = 0; i < this->run->pop_size; i++) {
        sum += this->fitnesses[i];
    }

    return sum;
}

vector<pair<int, int>> Ga::select() {
    int num_cross = (int) (this->run->cross_frac * this->run->pop_size);
    num_cross = max(num_cross - (num_cross % 2), 0);  //ensure it's even

    //build the roulette wheel
    float fit_sum = this->sum_fitnesses();
    //note: last slot is not needed because we are recording boundaries (it will always be 1.0)
    vector<float> wheel;
    float acc = 0.0f;
    for (int i = 0; i < this->run->pop_size - 1; i++) {
        float slice = 1.0 - this->fitnesses[i] / fit_sum; //note: we're minimizing
        float bound = acc + slice;
        wheel.push_back(bound);
        acc = bound;
    }

    //pick the individuals using the wheel (with replacement)
    vector<pair<int, int>> parents;
    for (int i = 0; i < num_cross / 2; i++) {
        pair<int, int> couple = pair<int, int>(this->spin_wheel(&wheel), this->spin_wheel(&wheel));
        parents.push_back(couple);
    }

    return parents;
}

int Ga::spin_wheel(vector<float> *wheel) {
    float spin = this->run->rand.next_float();
    int i = 0;
    while (i < (int) wheel->size() && spin > (*wheel)[i]) {
        i++;
    }

    return i;
}

void Ga::cross(vector<pair<int, int>> *parents) {
    //note: can't do in-place swap of individuals in existing population because selection is done with replacement (same individual might be selected more than once)
    vector<Grn*> new_pop;
    for (pair<int, int> couple : *parents) {
        Grn *parent0 = this->pop[couple.first];
        Grn *parent1 = this->pop[couple.second];

        int gene_split_pt = this->run->rand.in_range(1, (int) this->run->num_genes + 1); //in [1, num_genes]
        int protein_split_pt = this->run->rand.in_range(1, (int) this->run->initial_proteins + 1); //in [1, initial_proteins]

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
    int best_index = this->get_fittest(); //index in old pop
    Grn *copy = new Grn(this->pop[best_index]); //make sure we push a *copy* (so we use the copy constructor to copy the store and all the proteins it contains). Otherwise we'll get double free() errors...

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
    for (Grn *old_grn : this->pop) {
        delete old_grn;
    }
    this->pop = new_pop;
}

vector<Protein *> Ga::build_child_init_proteins(vector<Protein*>::iterator left, int left_len, vector<Protein*>::iterator right, int right_len) {
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

vector<Gene*> Ga::build_child_genes(vector<Gene*>::iterator left, int left_len, vector<Gene*>::iterator right, int right_len) {
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

Gene *Ga::cross_genes(Gene *left, Gene *right, int pos) {
    BitVec *binding_seq = this->cross_bitsets(left->binding_seq, right->binding_seq);
    BitVec *output_seq = this->cross_bitsets(left->output_seq, right->output_seq);
    
    return new Gene(
        this->run,
        binding_seq,
        output_seq,
        this->cross_primitives<float>(left->threshold, right->threshold, 0.0f, this->run->max_protein_conc),
        this->cross_primitives<float>(left->output_rate, right->output_rate, 0.0f, this->run->max_protein_conc),
        left->kernel_index, //stick with the existing kernel index for this gene (doesn't make sense to average them or anything like that...)
        pos
        );
}

Protein *Ga::cross_proteins(Protein *left, Protein *right) {
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
        left->kernel_index, //just choose arbitrarily for this one (I'm assuming averaging won't work great)
        src_pos
        );
}

BitVec *Ga::cross_bitsets(BitVec *left, BitVec *right) {
    int left_bits = this->run->rand.in_range(1, this->run->gene_bits);
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

void Ga::mutate() {
    #pragma omp parallel for
    for (int i = 0; i < this->run->pop_size; i++) {
        Grn *grn = this->pop[i];
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

void Ga::mutate_initial_proteins(vector<Protein*> *proteins) {
    for (Protein *p : *proteins) {
        this->mutate_bitset(p->seq);
        for (int i = 0; i < this->run->num_genes; i++) {
            this->mutate_float(&p->concs[i], 0.0f, this->run->max_protein_conc);
        }
        this->mutate_int(&p->kernel_index, 0, (int) KERNELS.size());
        this->mutate_int(&p->src_pos, 0, this->run->num_genes);
    }
}

void Ga::mutate_int(int *val, int lower, int upper) {
    if (this->run->rand.next_float() < this->run->mut_prob) {
        *val = this->run->rand.in_range(lower, upper);
    }
}

void Ga::mutate_float(float *val, float lower, float upper) {
    if (this->run->rand.next_float() < this->run->mut_prob) { 
        float eps = this->run->rand.in_range(-this->run->max_mut_float, this->run->max_mut_float);
        *val = Utils::clamp(*val + eps, lower, upper);
    }
}

void Ga::mutate_bitset(BitVec *bits) {
    int i = 0;
    int count = 0;
    while (i < this->run->gene_bits && count < this->run->max_mut_bits) {
        if (this->run->rand.next_float() < this->run->mut_prob) {
            (*bits)[i] = ~((*bits)[i]);
        }
        i++;
    }
}

void Ga::update_fitness(int ga_step) {
    //do regulatory simulation
    //note: can't use pragma below when logging! (TODO: fix this in the future by enabling multithreaded access to db)
    //#pragma omp parallel for
    for (int i = 0; i < this->run->pop_size; i++) {
        Grn *grn = this->pop[i];

        this->logger->log_reg_step(ga_step, -1, grn, i);

        for (int j = 0; j < this->run->reg_steps; j++) {
            grn->run_binding();

            grn->update_output_proteins();

            grn->run_diffusion();

            grn->run_decay();

            this->logger->log_reg_step(ga_step, j, grn, i);
        }

        //update fitness value
        this->fitnesses[i] = this->calc_fitness(grn);
    }

    //TODO: translate final grn proteins to phenotype

    //TODO: run phenotype through fitness function
}

float Ga::calc_fitness(Grn *grn) {
    float max_err = this->run->max_protein_conc * this->run->num_genes; //concs are in [0.0, max_protein_conc] so max possible error at over gene is max_protein_conc
    float result = max_err;
    
    //use the oldest protein (the one with the lowest id)
    if ((int) grn->proteins->size() > 0) {
        //get lowest id (note: grn.proteins iterator uses map iterator, which is set up to return ids in ascending order);
        int lowest_id = *(grn->proteins->begin());
        Protein *p = grn->proteins->get(lowest_id);
        result = this->calc_protein_error(p);
    }

    return result;
}

float Ga::calc_protein_error(Protein *protein) {
    //sine wave
    float err = 0.0f;

    float factor = 2.0f * M_PI / (float) (this->run->num_genes - 1);
    for (int i = 0; i < this->run->num_genes; i++) {
        err += abs(
            protein->concs[i] - (1.5f + sin(i * factor))
            );
    }
    
    return err;
}
