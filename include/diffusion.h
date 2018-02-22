#ifndef DIFFUSION_H
#define DIFFUSION_H
#include "constants.h"

class DiffKernels
{
public:
    DiffKernels();
    ~DiffKernels();
    float *get_diff_kernel(int index);
    
private:
    float *k0();
    float *k1();
    float *k2();
    float *kernels[NUM_DIFF_KERNELS];
};

#endif
