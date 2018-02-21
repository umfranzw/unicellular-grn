#include <iostream>
#include "gene.h"
#include "grn.h"
#include "protein.h"
#include <random>
#include <sys/random.h>

using namespace std;

int main(int argc, char *argv[])
{
    //initialize and seed random number generator (using Mersenne Twister)
    unsigned int seed;
    getrandom(&seed, sizeof(unsigned int), GRND_RANDOM); //grabs bytes from /dev/random
    mt19937 gen;
    gen.seed(seed);

    GRN g;
    g.init_rand((mt19937&) gen);
    cout << g.str() << endl;

    // Gene g1;
    // g1.init_rand((mt19937&) gen);
    // Gene g2;
    // g2.init_rand((mt19937&) gen);

    // cout << g1.str() << endl;
    // cout << g2.str() << endl;

    return EXIT_SUCCESS;
}
