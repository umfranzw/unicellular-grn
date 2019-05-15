#include "grn.hpp"
#include "protein.hpp"
#include "runs.hpp"
#include "protein_store.hpp"
#include "utils.hpp"
#include "gene.hpp"
#include "diffusion.hpp"
#include <sstream>
#include <vector>

Grn::Grn(Run *run, vector<Gene*> genes, vector<Protein*> initial_proteins) {
    this->run = run;
    this->genes = genes;
    this->proteins = new ProteinStore();
    this->initial_proteins = initial_proteins;
    
    this->push_initial_proteins();
}

//randomly initializes genes
Grn::Grn(Run *run) {
    this->run = run;
    this->proteins = new ProteinStore();

    //insert (random) genes
    for (int i = 0; i < run->num_genes; i++) {
        this->genes.push_back(new Gene(run, i));
    }

    for (int i = 0; i < run->initial_proteins; i++) {
        this->initial_proteins.push_back(new Protein(run, run->rand->in_range(0, run->num_genes)));
    }

    this->push_initial_proteins();
}

//note: set copy_store = false when you want the regulatory simulation to begin again from the initial conditions
Grn::Grn(Grn *grn, bool copy_store, bool copy_gene_state) { //copy constructor
    this->run = grn->run;
    //copy genes (note: the Gene copy constructor will remove all outputs and reset active_output and bound_protein.)
    for (int i = 0; i < this->run->num_genes; i++) {
        this->genes.push_back(new Gene(grn->genes[i], copy_gene_state));
    }

    //copy initial proteins
    for (int i = 0; i < this->run->initial_proteins; i++) {
        this->initial_proteins.push_back(new Protein(grn->initial_proteins[i]));
    }

    if (copy_store) {
        this->proteins = new ProteinStore(grn->proteins);
        this->growth_seq = new BitVec(*(grn->growth_seq));
    }

    else {
        this->proteins = new ProteinStore();
        this->push_initial_proteins(); //note: this will copy growth_seq
    }
}

void Grn::reset() {
    for (int i = 0; i < this->run->num_genes; i++) {
        this->genes[i]->reset();
    }

    this->proteins->reset();
    delete this->growth_seq;
    this->push_initial_proteins(); //resets the growth protein
}

Grn::~Grn() {
    delete this->proteins;
    delete this->growth_seq;
    
    for (Gene *gene : this->genes) {
        delete gene;
    }

    //note: these will not have been in the store we deleted above because we always push *copies* of them
    for (Protein *protein : this->initial_proteins) {
        delete protein;
    }
}

//note: this assumes we always have at least 1 initial protein
void Grn::push_initial_proteins() {
    //insert initial (random) proteins
    //push a *copy* - that way originals never get deleted

    //first, insert the first protein and make it the growth protein
    Protein *growth_protein = new Protein(this->initial_proteins[0]);
    this->growth_seq = new BitVec(*(growth_protein->seq));
    this->proteins->add(growth_protein);
    //then, insert the rest
    for (int i = 1; i < this->run->initial_proteins; i++) {
        this->proteins->add(new Protein(this->initial_proteins[i])); 
    }
}

void Grn::run_decay() {
    for (const int &id : *this->proteins) {
        Protein *protein = this->proteins->get(id);
        for (int pos = 0; pos < this->run->num_genes; pos++) {
            protein->concs[pos] = max(0.0f, protein->concs[pos] - protein->concs[pos] * this->run->decay_rate);
        }
    }
}

void Grn::run_binding(int pop_index, int reg_step, int ga_step) {
    for (int pos = 0; pos < this->run->num_genes; pos++) {
        vector<pair<int, float>> binding_probs;
        float pos_sum = 0.0f;
        vector<int> protein_ids = this->proteins->get_ids();
        for (const int& id: protein_ids) {
            Protein *protein = this->proteins->get(id);
            if (protein->src_pos != pos) { //prevent genes from producing proteins that bind to themselves
                int hamming_dist = Utils::hamming_dist(protein->seq, this->genes[pos]->binding_seq);
                if (hamming_dist <= this->run->binding_seq_play) {
                    if (protein->concs[pos] >= this->genes[pos]->threshold) {
                        binding_probs.push_back(pair<int, float>(id, protein->concs[pos]));
                        pos_sum += protein->concs[pos];
                    }
                }
            }
        }

        //if there are proteins above this position
        if (binding_probs.size() > 0) {
            //build the probability distribution and sample it (roulette-wheel-selection-style)
            float r = this->run->rand->next_float();
            float running_sum = 0.0f;

            int i = -1;
            do {
                i += 1;
                float normal_prob = binding_probs[i].second / pos_sum; //normalize the probability value
                running_sum += normal_prob;
                
            } while (r > running_sum && i < (int) binding_probs.size());
            //note: r should always be < 1.0, so this loop should always stop based on the first part of the condition (r > running_sum),
            //but we include the second condition just in case floating point error bites us

            //deal with potential floating point error
            if (i == (int) binding_probs.size()) {
                i = (int) binding_probs.size() - 1;
            }

            int sel_pid = binding_probs[i].first;
            this->genes[pos]->update_binding(&sel_pid, this->proteins);
        }

        //no proteins above this position => unbind
        else {
            //can unbind by passing null
            this->genes[pos]->update_binding(nullptr, this->proteins);
        }
    }
}

void Grn::run_diffusion() {
    for (int i = 0; i < this->run->num_genes; i++) {
        Gene *g = this->genes[i];
        vector<pair<int, int>> rm_pairs; //(index in gene.outputs, protein id)
        for (int j = 0; j < (int) g->outputs.size(); j++) {
            int p_id = g->outputs[j];
            Protein *p = this->proteins->get(p_id);
            bool above_threshold = Diffusion::diffuse(p);
            if (!above_threshold) {
                rm_pairs.push_back(pair<int, int>(j, p_id));
            }
        }

        for (int j = 0; j < (int) rm_pairs.size(); j++) {
            int index = rm_pairs[j].first;
            int id = rm_pairs[j].second;

            g->outputs.erase(g->outputs.begin() + index - j); //subtract j to compensate for shift due to removals on previous iterations

            if (g->active_output >= 0) {
                g->active_output = -1;
            }
            this->proteins->remove(id);

            //remove any bindings to the protein we just deleted
            for (int k = 0; k < this->run->num_genes; k++) {
                if (this->genes[k]->bound_protein == id) {
                    this->genes[k]->update_binding(nullptr, this->proteins);
                }
            }
        }
    }
}

void Grn::update_output_proteins() {
    for (int i = 0; i < this->run->num_genes; i++) {
        this->genes[i]->update_output_protein(this->proteins);
    }
}

int Grn::get_num_proteins() {
    return this->proteins->size();
}

string Grn::to_str() {
    stringstream info;

    info << "****" << endl;
    info << "Grn:" << endl;
    info << "****" << endl;

    info << "------" << endl;
    info << "Genes:" << endl;
    info << "------" << endl;
    for (Gene *gene : this->genes) {
        info << gene->to_str();
    }

    //info << this->proteins->to_str();

    return info.str();
}
