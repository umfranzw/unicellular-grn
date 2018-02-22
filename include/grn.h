#ifndef GRN_H
#define GRN_H

#include "gene.h"
#include <string>
#include "constants.h"
#include "utils.h"

using namespace std;

class GRN
{
public:
    Gene genes[NUM_GENES];

    GRN();
    string str();
    void init_rand(RandGen& gen);
};

#endif
