#include "diffusion.h"

DiffKernels::DiffKernels()
{
    kernels[0] = k0();
    kernels[1] = k1();
    kernels[2] = k2();
}

float *DiffKernels::get_diff_kernel(int index)
{
    return kernels[index];
}

float *DiffKernels::k0()
{
    float *k = new float[3];
    k[0] = 1.0 / 3;
    k[1] = 1.0 / 3;
    k[2] = 1.0 / 3;
    
    return k;
}

float *DiffKernels::k1()
{
    float *k = new float[3];
    k[0] = 1.0 / 3;
    k[1] = 1.0 / 8;
    k[2] = 1.0 / 3;
    
    return k;
}

float *DiffKernels::k2()
{
    float *k = new float[5];
    k[0] = 1.0 / 3;
    k[1] = 1.0 / 8;
    k[2] = 1.0 / 16;
    k[3] = 1.0 / 8;
    k[4] = 1.0 / 3;
    
    return k;
}

DiffKernels::~DiffKernels()
{
    for (int i = 0; i < NUM_DIFF_KERNELS; i++)
    {
        delete kernels[i];
    }
}
