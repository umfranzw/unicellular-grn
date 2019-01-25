#include "gene.hpp"
#include <sstream>
#include "utils.hpp"

Gene::Gene(Run *run, BitVec *binding_seq, BitVec *output_seq, float threshold, float output_rate, int pos) {
    this->run = run;
    this->binding_seq = new BitVec(*binding_seq);
    this->output_seq = new BitVec(*output_seq);
    this->threshold = threshold;
    this->output_rate = output_rate;
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
    this->threshold = run->rand->next_float();
    this->output_rate = run->rand->next_float();
    this->active_output = -1;
    this->bound_protein = -1;
}

//copy constructor
Gene::Gene(Gene *gene, bool copy_state) {
    this->run = gene->run;
    this->binding_seq = new BitVec(*gene->binding_seq);
    this->output_seq = new BitVec(*gene->output_seq);
    this->threshold = gene->threshold;
    this->output_rate = gene->output_rate;
    this->pos = gene->pos;

    if (copy_state) {
        this->active_output = gene->active_output;
        this->bound_protein = gene->bound_protein;
        this->outputs = vector<int>(gene->outputs);
    }
    else {
        //don't copy these, or outputs
        this->active_output = -1;
        this->bound_protein = -1;
    }
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

void Gene::update_binding(int *bind_pid, ProteinStore *store) {
    if (bind_pid != nullptr) {
        this->bound_protein = *bind_pid;
        //check if this gene has output its output protein in the past, and that protein is still around...
        pair<int, Protein*> result = store->get_by_seq(this->output_seq);
        //if so, reactivate the existing protein
        if (result.first != -1 && result.second->src_pos == this->pos) {
            this->active_output = result.first;
        }

        else {
            //otherwise, create a new protein and activate it
            Protein *protein = new Protein(this->run, this->output_seq, Utils::zeros(this->run->num_genes), this->pos);
            int pid = store->add(protein);
            this->active_output = pid;
            this->outputs.push_back(pid);

            //update the binding protein's interation count
            Protein *bind_protein = store->get(*bind_pid);
            bind_protein->interactions++;
        }
    }
    //if we got passed a null protein pointer, unbind and stop output protein production
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
