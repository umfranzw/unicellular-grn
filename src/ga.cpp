#include "ga.hpp"
#include <boost/algorithm/clamp.hpp>
#include "kernels.hpp"
#include "constants.hpp"
#include <cmath>

Ga::Ga(Run *run) {
    this->run = run;
    this->logger = new Logger(run);

    //initialize population (randomly)
    for (int i = 0; i < run->pop_size; i++) {
        this->pop.push_back(Grn(run));
        this->fitnesses.push_back(0.0f);
    }
}

Ga::~Ga() {
    delete this->logger;
}

void Ga::run_alg() {
    for (int i = 0; i < this->run->pop_size; i++) {
        this->pop[i].push_initial_proteins();
    }
    
    this->logger->log_ga_step(-1, &this->pop); //log initial grns using ga_step = -1
    this->update_fitness(-1); //this will log initial reg sim using ga_step = -1
    this->logger->log_fitnesses(-1, &this->fitnesses); //and finally the fitnesses

    for (int i = 0; i < this->run->ga_steps; i++) {
        cout << "i: " << i + 1 << endl;
        //cout.flush();
        
        vector<pair<int, int>> parents = this->select();
        this->cross(&parents);
        this->mutate();

        this->logger->log_ga_step(i, &this->pop);

        this->update_fitness(i); //this will log the reg sim

        this->logger->log_fitnesses(-1, &this->fitnesses);
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

float Ga::get_avg_fitness() {
    return this->sum_fitnesses() / this->run->pop_size;
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
    vector<Grn> new_pop;
    for (pair<int, int> couple : *parents) {
        Grn *parent0 = &this->pop[couple.first];
        Grn *parent1 = &this->pop[couple.second];

        int split_pt = this->run->rand.in_range(1, (int) this->run->num_genes + 1); //in [1, num_genes]

        //build the children

        //child 0
        vector<Gene>::iterator left = parent0->genes.begin(); //length is split_pt
        vector<Gene>::iterator right = parent1->genes.begin() + split_pt; //length is (num_genes - split_pt)
        Grn child0 = this->build_child(left, split_pt, right, this->run->num_genes - split_pt);

        //child 1
        left = parent0->genes.begin() + split_pt; //length is (num_genes - split_pt)
        right = parent1->genes.begin(); //length is split_pt
        Grn child1 = this->build_child(right, split_pt, left, this->run->num_genes - split_pt);

        new_pop.push_back(child0);
        new_pop.push_back(child1);
    }

    //preserve the fittest individual ("elitism")
    int best_index = this->get_fittest();
    //if there's room, append
    if ((int) new_pop.size() < this->run->pop_size) {
        new_pop.push_back(this->pop[best_index]);
    }
    //otherwise, we have to sacrifice a child...
    else {
        *(new_pop.end() - 1) = this->pop[best_index];
    }

    //fill out the rest of the new_pop with new Grns, randomly generated
    while ((int) new_pop.size() < this->run->pop_size) {
        new_pop.push_back(Grn(this->run));
    }

    //replace old pop
    this->pop = new_pop;
}

Grn Ga::build_child(vector<Gene>::iterator left, int left_len, vector<Gene>::iterator right, int right_len) {
    vector<Gene> genes;
    int mix_len = min(left_len, right_len);
    for (int i = 0; i < mix_len; i++) {
        //here, left parent's first part is first in child
        Gene gene = this->cross_genes(&*(left + i), &*(right + i), i);
        genes.push_back(gene);
    }

    //note: mix_len is at most num_genes / 2, so doing this a second time is safe
    //note the reverse order of the params, which causes right parent's first part to be first in child
    //this also causes the bitvecs to be mixed in the opposite way
    for (int i = 0; i < mix_len; i++) {
        Gene gene = this->cross_genes(&*(right + i), &*(left + i), mix_len + i);
        genes.push_back(gene);
    }

    //fill in the rest with genes from the longer of the two slice, starting after the mixpoint
    int j = 0;
    vector<Gene>::iterator longer = left_len > right_len ? left : right;
    while ((int) genes.size() < this->run->num_genes) {
        Gene copy = Gene(&*(longer + mix_len + j));
        genes.push_back(copy);
        j++;
    }

    return Grn(this->run, genes);
}

Gene Ga::cross_genes(Gene *left, Gene *right, int pos) {
    return Gene(
        this->run,
        this->cross_bitsets(&left->binding_seq, &right->binding_seq),
        this->cross_bitsets(&left->output_seq, &right->output_seq),
        this->cross_floats(left->threshold, right->threshold, 0.0f, this->run->max_protein_conc),
        this->cross_floats(left->output_rate, right->output_rate, 0.0f, this->run->max_protein_conc),
        left->kernel_index, //stick with the existing kernel index for this gene (doesn't make sense to average them or anything like that...)
        pos
        );
}

float Ga::cross_floats(float left, float right, float lower, float upper) {
    return boost::algorithm::clamp((left + right) / 2.0f, lower, upper);
}

boost::dynamic_bitset<> Ga::cross_bitsets(boost::dynamic_bitset<> *left, boost::dynamic_bitset<> *right) {
    int left_bits = this->run->rand.in_range(1, this->run->gene_bits);
    int right_bits = this->run->gene_bits - left_bits;

    //take first part of left and second part of right (if you want first part of right and second of left, just switch the left and right args you pass in)
    //first, take left and zero out the rightmost bits
    boost::dynamic_bitset<> left_copy = boost::dynamic_bitset<>(*left);
    left_copy >>= this->run->gene_bits - left_bits;
    left_copy <<= this->run->gene_bits - left_bits;

    //second, take right and zero out the leftmost bits
    boost::dynamic_bitset<> right_copy = boost::dynamic_bitset<>(*right);
    right_copy >>= this->run->gene_bits - right_bits;
    right_copy <<= this->run->gene_bits - right_bits;

    //finally, put them together
    return left_copy | right_copy;
}

void Ga::mutate() {
    for (int i = 0; i < this->run->pop_size; i++) {
        for (int j = 0; j < this->run->num_genes; j++) {
            Gene *g = &this->pop[i].genes[j];
            this->mutate_bitset(&g->binding_seq);
            this->mutate_bitset(&g->output_seq);
            this->mutate_float(&g->threshold, 0.0f, this->run->max_protein_conc);
            this->mutate_float(&g->output_rate, 0.0f, this->run->max_protein_conc);
            this->mutate_kernel_index(&g->kernel_index);
        }
    }
}

void Ga::mutate_kernel_index(int *index) {
    if (this->run->rand.next_float() < this->run->mut_prob) {
        *index = this->run->rand.in_range(0, (int) KERNELS.size());
    }
}

void Ga::mutate_float(float *val, float lower, float upper) {
    if (this->run->rand.next_float() < this->run->mut_prob) {
        float eps = this->run->rand.in_range(-this->run->max_mut_float, this->run->max_mut_float);
        *val = boost::algorithm::clamp(*val + eps, lower, upper);
    }
}

void Ga::mutate_bitset(boost::dynamic_bitset<> *bits) {
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
    for (int i = 0; i < this->run->pop_size; i++) {
        // cout << "i: " << i << endl;
        // cout.flush();
        
        Grn *grn = &this->pop[i];
        this->logger->log_reg_step(ga_step, -1, grn, i);
        
        for (int j = 0; j < this->run->reg_steps; j++) {
            // cout << "j: " << j << endl;
            // cout.flush();

            // cout << "Running binding" << endl;
            // cout.flush();
            grn->run_binding();

            // cout << "Updating output proteins" << endl;
            // cout.flush();
            grn->update_output_proteins();

            // cout << "Running diffusion" << endl;
            // cout.flush();
            grn->run_diffusion();

            // cout << "Running decay" << endl;
            // cout.flush();
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
    float max_err = 3.0 * this->run->num_genes; //concs are in [0.0, 3.0] so max possible error at over gene is 3.0
    float result = max_err;
    
    //use the oldest protein (the one with the lowest id)
    if ((int) grn->proteins.size() > 0) {
        //get lowest id (note: grn.proteins iterator does not necessarily return ids in ascending order)
        int lowest_id = -1;
        for (const int& id : grn->proteins) {
            if (lowest_id == -1 || id < lowest_id) {
                lowest_id = id;
            }
        }

        Protein *p = grn->proteins.get(lowest_id);
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
