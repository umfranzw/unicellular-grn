#include "utils.h"
#include <sys/random.h>
#include <random>

//creates a mask with the lower 'bits' bits set to 1
unsigned int make_lower_bitmask(int bits)
{
    unsigned int mask = 0;
    for (int i = 0; i < bits; i++)
    {
        mask <<= 1;
        mask |= 1;
    }

    return mask;
}

RandGen make_gen()
{
    //initialize and seed random number generator (using Mersenne Twister)
    unsigned int seed;
    getrandom(&seed, sizeof(unsigned int), GRND_RANDOM); //grabs bytes from /dev/random
    RandGen gen;
    gen.seed(seed);

    return gen;
}
