#include "gene.hpp"
#include <sstream>
#include "utils.hpp"
#include "kernels.hpp"

Gene::Gene(Run *run, BitVec *binding_seq, BitVec *output_seq, float threshold, float output_rate, int kernel_index, int pos) {
    this->run = run;
    this->binding_seq = new BitVec(*binding_seq);
    this->output_seq = new BitVec(*output_seq);
    this->threshold = threshold;
    this->output_rate = output_rate;
    this->kernel_index = kernel_index;
    this->pos = pos;
    this->active_output = -1;
    this->bound_protein = -1;
}

//randomly initializes components that are not passed
Gene::Gene(Run *run, int pos) {
    this->run = run;
    this->pos = pos;
    this->binding_seq = new BitVec(this->run->gene_bits);
    this->output_seq = new BitVec(this->run->gene_bits);
    Utils::fill_rand(this->binding_seq, run->gene_bits, run);
    Utils::fill_rand(this->output_seq, run->gene_bits, run);
    this->threshold = run->rand.next_float();
    this->output_rate = run->rand.next_float();
    this->kernel_index = run->rand.in_range(0, KERNELS.size());
    this->active_output = -1;
    this->bound_protein = -1;
}

//copy constructor
Gene::Gene(Gene *gene) {
    this->run = gene->run;
    this->binding_seq = new BitVec(*gene->binding_seq);
    this->output_seq = new BitVec(*gene->output_seq);
    this->threshold = gene->threshold;
    this->output_rate = gene->output_rate;
    this->kernel_index = gene->kernel_index;
    this->pos = gene->pos;
    
    //don't copy these, or outputs
    this->active_output = -1;
    this->bound_protein = -1;
}

Gene::~Gene() {
    delete this->binding_seq;
    delete this->output_seq;
}

//clears bindings and outputs
void Gene::reset() {
    this->active_output = -1;
    this->bound_protein = -1;
    this->outputs.clear();
}

void Gene::update_output_protein(ProteinStore *store) {
    if (this->active_output != -1) {
        Protein *protein = store->get(this->active_output);
        protein->concs[this->pos] = Utils::clamp<float>(protein->concs[this->pos] + this->output_rate, 0.0, this->run->max_protein_conc);
    }
}

void Gene::update_binding(pair<int, float> *protein_info, ProteinStore *store) {
    if (protein_info != nullptr) {
        int id = protein_info->first;
        float conc = protein_info->second;
        this->bound_protein = id;
        if (conc >= this->threshold) {
            if (Utils::contains_id(&this->outputs, id)) {
                this->active_output = id;
            }
            else {
                Protein *protein = new Protein(this->run, this->output_seq, Utils::zeros(this->run->num_genes), this->kernel_index, this->pos);
                int id = store->add(protein);
                this->active_output = id;
                this->outputs.push_back(id);
            }
        }
        else {
            this->active_output = -1;
        }
    }
    else {
        this->active_output = -1;
        this->bound_protein = -1;
    }
}

string Gene::to_str() {
    stringstream info;

    info << "Gene:" << endl;
    info << "  binding_seq: " << this->binding_seq->to_str() << endl;
    info << "  output_seq: " << this->output_seq->to_str() << endl;
    info << "  threshold: " << this->threshold << endl;
    info << "  output_rate: " << this->output_rate << endl;
    info << "  kernel_index: " << this->kernel_index << endl;
    info << "  pos: " << this->pos << endl;
    info << "  bound_protein: " << this->bound_protein << endl;
    info << "  active_output: " << this->active_output << endl;
    info << "  outputs: [";

    for (size_t i = 0; i < this->outputs.size(); i++) {
        info << this->outputs[i];
        if (i < this->outputs.size() - 1) {
            info << ", ";
        }
    }
    info << "]" << endl;

    return info.str();
}
