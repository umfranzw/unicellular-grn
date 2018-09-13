#include "instr_dist.hpp"
#include "instr_factory.hpp"
#include <vector>
#include <cstring>
#include <sstream>
#include "constants.hpp"

InstrDist::InstrDist(Run *run) {
    this->run = run;
    this->probs = new vector<float>();
    this->weights = new vector<float>();
    
    float prob = 1.0f / NUM_INSTR_TYPES;
    float weight = 1.0f;
    vector<float> buckets;
    for (int i = 0; i < NUM_INSTR_TYPES; i++) {
        this->probs->push_back(prob);
        this->weights->push_back(weight);
        buckets.push_back(prob * weight);
    }

    this->dist = new ProbDist(this->run, &buckets, true, false);
}

InstrDist::InstrDist(InstrDist *other) {
    this->run = other->run;
    this->probs = new vector<float>(*other->probs);
    this->weights = new vector<float>(*other->weights);
    this->dist = new ProbDist(other->dist);
}

InstrDist::~InstrDist() {
    delete this->dist;
    delete this->probs;
    delete this->weights;
}

int InstrDist::sample() {
    return this->dist->sample();
}

float InstrDist::get_weight(int index) {
    return (*this->weights)[index];
}

void InstrDist::set_weight(int index, float val) {
    (*this->weights)[index] = val;
    (*(this->dist))[index] = (*this->probs)[index] * (*this->weights)[index];
}

void InstrDist::set_weights(vector<float> *weights) {
    delete this->weights;
    this->weights = new vector<float>(*weights);
    this->update_prob_dist();
}

float InstrDist::get_prob(int index) {
    return (*this->probs)[index];
}

void InstrDist::set_prob(int index, float val) {
    (*this->probs)[index] = val;
    (*(this->dist))[index] = (*this->probs)[index] * (*this->weights)[index];
}

void InstrDist::set_probs(vector<float> *probs) {
    delete this->probs;
    this->probs = new vector<float>(*probs);
    this->update_prob_dist();
}

void InstrDist::update_prob_dist() {
    vector<float> buckets;
    for (int i = 0; i < NUM_INSTR_TYPES; i++) {
        buckets.push_back((*this->probs)[i] * (*this->weights)[i]);
    }

    this->dist->replace_buckets(&buckets);
}

string InstrDist::to_str() {
    stringstream info;
    info << "Instruction Prob. Dist.:" << endl;
    info << "     Prob.   Weight" << endl;
    for (int i = 0; i < (int) this->probs->size(); i++) {
        info << "i = " << i << ": " << (*this->probs)[i] << ", " << (*this->weights)[i] << endl;
    }
    info << this->dist->to_str();

    return info.str();
}
