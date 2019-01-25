#include "instr_dist.hpp"
#include <sstream>

//don't store non-zero probs
InstrDist::InstrDist(Run *run, InstrFactory *factory, map<Protein*, float, bool(*)(Protein*, Protein*)> *buckets) {
    this->run = run;
    this->buckets = new map<Protein*, float, bool(*)(Protein*, Protein*)>(Protein::compare);
    this->factory = factory;

    float sum = 0.0f;
    for (pair<Protein*, float> item : *buckets) {
        sum += item.second;
        (*this->buckets)[item.first] = item.second; //note: this requires < operator to be implemented on Protein
    }

    if (sum > 0.0f) {
        for (pair<Protein*, float> item : *(this->buckets)) {
            item.second /= sum;
        }
    }
}

InstrDist::InstrDist(InstrDist *other) {
    this->run = other->run;
    this->buckets = new map<Protein*, float, bool(*)(Protein*, Protein*)>(*other->buckets);
    this->factory = other->factory;
}

InstrDist::~InstrDist() {
    delete this->buckets;
}

pair<Protein*, Instr *> InstrDist::sample() {
    float spin = this->run->rand->next_float();
    auto it = this->buckets->begin();
    float accum = 0.0f;
    while (it != this->buckets->end() && spin > accum) {
        accum += it->second;
        it++;
    }

    pair<Protein*, Instr *> result = make_pair(nullptr, nullptr);
    if (it != this->buckets->end()) {
        result.first = it->first;
        result.second = this->factory->create_instr(it->first->seq);
    }
    
    return result;
}

string InstrDist::to_str() {
    stringstream info;
    info << "Instruction Prob. Dist.:" << endl;
    for (auto it = this->buckets->begin(); it != this->buckets->end(); it++) {
        info << it->first->to_str() << " - " << it->second << endl;
    }

    return info.str();
}
