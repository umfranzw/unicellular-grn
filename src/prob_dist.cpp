#include "prob_dist.hpp"

#include <sstream>

ProbDist::ProbDist(Run *run, vector<float> *buckets, bool normalize, bool invert) {
    this->run = run;
    this->normalize = normalize;
    this->invert = invert;
    this->wheel = new vector<float>();
    this->buckets = new vector<float>();

    //this must be called last!
    this->replace_buckets(buckets); //note: this sets this->dirty = false
}

void ProbDist::replace_buckets(vector<float> *buckets) {
    this->buckets->clear();

    float val;
    for (auto it = buckets->begin(); it != buckets->end(); it++) {
        val = this->invert ? 1.0 - *it : *it;
        this->buckets->push_back(val);
    }
    
    this->update_wheel(); //note: this sets this->dirty = false
}

ProbDist::ProbDist(ProbDist *other) {
    this->buckets = new vector<float>(*other->buckets);
    this->wheel = new vector<float>(*other->wheel);
    this->dirty = other->dirty;
    this->run = other->run;
}

ProbDist::~ProbDist() {
    delete this->buckets;
    delete this->wheel;
}

int ProbDist::sample() {
    if (this->dirty) {
        this->update_wheel();
    }
    
    float spin = this->run->rand.next_float();
    int i = 0;
    while (i < (int) this->wheel->size() && spin > (*this->wheel)[i]) {
        i++;
    }

    return i;
}

string ProbDist::to_str() {
    if (this->dirty) {
        this->update_wheel();
    }
    
    stringstream info;
    info << "Probability Distribution:" << endl;
    info << "    Buckets:  Wheel:" << endl;
    for (int i = 0; i < (int) this->buckets->size(); i++) {
        info << "i = " << i << ": " << (*this->buckets)[i] << ", " << (*this->wheel)[i] << endl;
    }
    
    return info.str();
}

BucketRef ProbDist::operator[](size_t index) {
    if (this->dirty) {
        this->update_wheel();
    }
    return BucketRef(this, index);
}

ProbDist& ProbDist::operator=(const ProbDist& other) {
    if (this != &other) {
        if (this->dirty) {
            this->update_wheel();
        }
        this->wheel = other.wheel;
        this->dirty = other.dirty;
        this->buckets = new vector<float>(*other.buckets);
    }

    return *this;
}

void ProbDist::update_wheel() {
    this->wheel->clear();
    
    if (this->normalize) {
        float sum = 0;
        for (auto it = this->buckets->begin(); it != this->buckets->end(); it++) {
            sum += *it;
        }
        float accum = 0.0f;
        for (auto it = this->buckets->begin(); it != this->buckets->end(); it++) {
            *it /= sum;
            accum += *it;
            this->wheel->push_back(accum);
        }
    }
    
    else {
        float accum = 0.0f;
        for (auto it = this->buckets->begin(); it != this->buckets->end(); it++) {
            accum += *it;
            this->wheel->push_back(accum);
        }
    }
    
    this->dirty = false;
}

//-------------------------
BucketRef::BucketRef(ProbDist *dist, int index) {
    this->dist = dist;
    this->index = index;
}

BucketRef& BucketRef::operator=(float val) {
    if (this->dist->invert) {
        val = 1.0f - val;
    }
    (*this->dist->buckets)[this->index] = val;
    this->dist->dirty = true;

    return *this;
}

BucketRef& BucketRef::operator=(const BucketRef& other) {
    float val = (*other.dist->buckets)[other.index];
    if (this->dist->invert && !other.dist->invert) {
        val = 1.0f - val;
    }
    (*this->dist->buckets)[this->index] = val;
    this->dist->dirty = true;

    return *this;
}

BucketRef::operator float() const {
    return (*this->dist->buckets)[this->index];
}
