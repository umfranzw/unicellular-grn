#ifndef GRN_H
#define GRN_H

#include "gene.h"
#include <string>
#include "constants.h"
#include "utils.h"
#include <vector>
#include "diffusion.h"

using namespace std;

class GRN
{
public:
    Gene genes[NUM_GENES];
    vector<Protein *> proteins;
    DiffKernels kernels;

    GRN();
    string str();
    void run_diffusion();
    void init_rand(RandGen& gen);
};

#endif
