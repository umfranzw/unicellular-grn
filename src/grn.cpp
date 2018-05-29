#include "grn.hpp"
#include "protein.hpp"
#include "runs.hpp"
#include "protein_store.hpp"
#include <boost/dynamic_bitset.hpp>
#include "utils.hpp"
#include "kernels.hpp"
#include "gene.hpp"
#include <sstream>
#include <vector>

Grn::Grn(Run *run, vector<Gene> genes) {
    this->run = run;
    this->genes = genes;
}

//randomly initializes genes
Grn::Grn(Run *run) {
    this->run = run;

    //insert (random) genes
    for (int i = 0; i < run->num_genes; i++) {
        Gene gene(run, i);
        this->genes.push_back(gene);
    }
}

void Grn::push_initial_proteins() {
    //insert initial (random) proteins
    for (int i = 0; i < this->run->initial_proteins; i++) {
        int pos = run->rand.in_range(0, this->run->num_genes);
        this->proteins.add(new Protein(this->run, pos));
    }
}

void Grn::run_decay() {
    for (const int &id : this->proteins) {
        Protein *protein = this->proteins.get(id);
        for (int pos = 0; pos < this->run->num_genes; pos++) {
            protein->concs[pos] = max(0.0f, protein->concs[pos] - protein->concs[pos] * this->run->decay_rate);
        }
    }
}

void Grn::run_binding() {
    for (int pos = 0; pos < this->run->num_genes; pos++) {
        vector<pair<int, float>> weighted_probs;
        float pos_sum = 0.0f; //sum of all concentrations in current position
        vector<int> protein_ids = this->proteins.get_ids();
        for (const int& id: protein_ids) {
            Protein* protein = this->proteins.get(id);
            int hamming_dist = Utils::hamming_dist(&protein->seq, &this->genes[pos].binding_seq);
            float w = this->run->alpha * protein->concs[pos] + this->run->beta * hamming_dist;
            weighted_probs.push_back(pair<int, float>(id, w));
            pos_sum += w;
        }

        //if there are proteins above this position
        if (weighted_probs.size() > 0) {
            //build the probability distribution and sample it (roulette-wheel-selection-style)
            float r = this->run->rand.next_float();
            float running_sum = 0.0f;
            int i = 0;
            //second part of condition below is in case of floating point error
            while (r > running_sum && i < (int) weighted_probs.size()) {
                weighted_probs[i].second /= pos_sum;
                running_sum += weighted_probs[i].second;
                i += 1;
            }

            //deal with potential floating point error
            if (i == (int) weighted_probs.size()) {
                i = (int) weighted_probs.size() - 1;
            }

            this->genes[pos].update_binding(&weighted_probs[i], &this->proteins);
        }
        //no proteins above this position => unbind
        else {
            this->genes[pos].update_binding(nullptr, &this->proteins);
        }
    }
}

void Grn::run_diffusion() {
    for (int i = 0; i < this->run->num_genes; i++) {
        Gene *g = &this->genes[i];
        vector<float> new_concs(this->run->num_genes, 0.0f);
        vector<pair<int, int>> rm_pairs; //(index in gene.outputs, protein id)

        for (int j = 0; j < (int) g->outputs.size(); j++) {
            int p_id = g->outputs[j];
            Protein *p = this->proteins.get(p_id);
            const vector<float> *kernel = &KERNELS[p->kernel_index];
            int mid = (int) kernel->size() / 2;
            bool above_threshold = false; //will set to true if any conc in the protein is > run.min_protein_conc

            for (int k = 0; k < this->run->num_genes; k++) {
                for (int l = 0; l < (int) kernel->size(); l++) {
                    int col = k - mid + l;
                    if (col >= 0 && col < (int) this->run->num_genes) {
                        new_concs[k] = min(new_concs[k] + p->concs[col] * (*kernel)[l], this->run->max_protein_conc); //ensure we don't go over the max. Note: the min threshold is checked below.
                    }
                }
                above_threshold = above_threshold || (new_concs[k] > this->run->min_protein_conc);
            }

            //replace old concentrations with new ones,
            //but don't waste time copying if we're just going to throw the protein away because it's concentrations
            //don't meet the min threshold
            if (above_threshold) {
                p->concs = new_concs;
            }
            else {
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
            this->proteins.remove(id);

            //remove any bindings to the protein we just deleted
            for (int k = 0; k < this->run->num_genes; k++) {
                if (this->genes[k].bound_protein == id) {
                    this->genes[k].update_binding(nullptr, &this->proteins);
                }
            }
        }
    }
}

void Grn::update_output_proteins() {
    for (int i = 0; i < this->run->num_genes; i++) {
        this->genes[i].update_output_protein(&this->proteins);
    }
}

string Grn::to_str() {
    stringstream info;

    info << "****" << endl;
    info << "Grn:" << endl;
    info << "****" << endl;

    info << "------" << endl;
    info << "Genes:" << endl;
    info << "------" << endl;
    for (Gene& gene : this->genes) {
        info << gene.to_str();
    }

    return info.str();
}
