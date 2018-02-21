#ifndef GRN_H
#define GRN_H

#include "gene.h"
#include <string>
#include "constants.h"

using namespace std;

class GRN
{
public:
    Gene genes[NUM_GENES];

    GRN();
    string str(bool compact=false);
    void init_rand(mt19937& gen);
};

#endif
