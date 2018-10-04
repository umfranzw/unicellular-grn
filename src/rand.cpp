#include "rand.hpp"
#include "constants.hpp"
#include <iostream>

using namespace std;

Rand::Rand() {
    if (FIX_RNG) {
        this->seed = FIXED_RNG_SEED;
    }
    else {
        random_device dev;
        this->seed = dev();
    }
    this->gen = mt19937(this->seed);
}

//result is in [low, high)
int Rand::in_range(int low, int high) {
    uniform_int_distribution<> dist(low, high - 1); //generates in [low, high]
    return dist(this->gen);
}

//result is in [low, high)
float Rand::in_range(float low=0.0, float high=1.0) {
    uniform_real_distribution<> dist(low, high);
    return dist(this->gen);
}

//result is in [0, 1)
float Rand::next_float() {
    return this->in_range(0.0f, 1.0f);
}
