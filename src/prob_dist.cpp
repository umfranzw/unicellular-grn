#include "prob_dist.hpp"

#include <sstream>

ProbDist::ProbDist(Run *run, vector<float> buckets, bool normalize, bool invert) {
    if (normalize) {
        float sum = 0;
        for (auto it = buckets.begin(); it != buckets.end(); it++) {
            if (invert) {
                *it = 1.0 - *it;
            }
            sum += *it;
        }
        float accum = 0.0f;
        for (auto it = buckets.begin(); it != buckets.end(); it++) {
            *it /= sum;
            accum += *it;
            this->wheel.push_back(accum);
        }
    }
    
    else {
        float accum = 0.0f;
        for (auto it = buckets.begin(); it != buckets.end(); it++) {
            if (invert) {
                *it = 1.0 - *it;
            }
            accum += *it;
            this->wheel.push_back(accum);
        }
    }
    
    this->buckets = buckets;
    this->dirty_index = -1;
    this->run = run;
}

int ProbDist::sample() {
    if (this->dirty_index > -1) {
        this->update_wheel();
    }
    
    float spin = this->run->rand.next_float();
    int i = 0;
    while (i < (int) this->wheel.size() && spin > this->wheel[i]) {
        i++;
    }

    return i;
}

string ProbDist::to_str() {
    if (this->dirty_index > -1) {
        this->update_wheel();
    }
    
    stringstream info;
    info << "Probability Distribution:" << endl;
    info << "    Buckets:  Wheel:" << endl;
    for (int i = 0; i < (int) this->buckets.size(); i++) {
        info << "i = " << i << ": " << this->buckets[i] << ", " << this->wheel[i] << endl;
    }
    
    return info.str();
}

BucketRef ProbDist::operator[](size_t index) {
    if (this->dirty_index > -1) {
        this->update_wheel();
    }
    return BucketRef(this, index);
}

ProbDist& ProbDist::operator=(const ProbDist& other) {
    if (this != &other) {
        if (this->dirty_index > -1) {
            this->update_wheel();
        }
        this->wheel = other.wheel;
        this->dirty_index = other.dirty_index;
        this->buckets = other.buckets;
    }

    return *this;
}

void ProbDist::update_wheel() {
    float accum = this->dirty_index > 0 ? this->wheel[this->dirty_index - 1] : 0.0f;
    for (int i = this->dirty_index; i < (int) this->wheel.size(); i++) {
        accum += this->buckets[i];
        this->wheel[i] = accum;
    }
    
    this->dirty_index = -1;
}

//-------------------------
BucketRef::BucketRef(ProbDist *dist, int index) {
    this->dist = dist;
    this->index = index;
}

BucketRef& BucketRef::operator=(float val) {
    this->dist->buckets[this->index] = val;
    this->dist->dirty_index = this->index;

    return *this;
}

BucketRef& BucketRef::operator=(const BucketRef& other) {
    this->dist->buckets[this->index] = other.dist->buckets[other.index];
    this->dist->dirty_index = this->index;

    return *this;
}

BucketRef::operator float() const {
    return this->dist->buckets[this->index];
}
