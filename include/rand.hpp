#ifndef _RAND_HPP
#define _RAND_HPP

#include <random>

using namespace std;

class Rand {
public:
    Rand(bool fix_rng_seed, int fixed_rng_seed);
    int in_range(int low, int high);
    float in_range(float low, float high);
    float next_float();

    unsigned int seed;
    
private:
    mt19937 gen;
};

#endif
