#include "instr_dist.hpp"
#include <sstream>

//don't store non-zero probs
InstrDist::InstrDist(Run *run, InstrFactory *factory, map<BitVec*, float, bool(*)(BitVec*, BitVec*)> *buckets) {
    this->run = run;
    this->buckets = new map<BitVec*, float, bool(*)(BitVec*, BitVec*)>(BitVec::compare);
    this->factory = factory;

    float sum = 0.0f;
    for (pair<BitVec*, float> item : *buckets) {
        sum += item.second;
        (*this->buckets)[item.first] = item.second; //!!! requires < operator
    }

    if (sum > 0.0f) {
        for (pair<BitVec*, float> item : *(this->buckets)) {
            item.second /= sum;
        }
    }
}

InstrDist::InstrDist(InstrDist *other) {
    this->run = other->run;
    this->buckets = new map<BitVec*, float, bool(*)(BitVec*, BitVec*)>(*other->buckets);
    this->factory = other->factory;
}

InstrDist::~InstrDist() {
    delete this->buckets;
}

Instr *InstrDist::sample() {
    float spin = this->run->rand.next_float();
    auto it = this->buckets->begin();
    float accum = 0.0f;
    while (it != this->buckets->end() && spin > accum) {
        accum += it->second;
        it++;
    }

    Instr *instr = nullptr;
    if (it != this->buckets->end()) {
        BitVec *result = it->first;
        instr = this->factory->create_instr(result);
    }
    
    return instr;
}

string InstrDist::to_str() {
    stringstream info;
    info << "Instruction Prob. Dist.:" << endl;
    for (auto it = this->buckets->begin(); it != this->buckets->end(); it++) {
        info << it->first->to_str() << " - " << it->second << endl;
    }

    return info.str();
}
