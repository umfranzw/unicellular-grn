#include "diffusion.h"

DiffKernels::DiffKernels()
{
    kernels[0] = k0();
    kernels[1] = k1();
    kernels[2] = k2();
}

float *DiffKernels::get_diff_kernel(int index, int *size)
{
    *size = this->kernel_sizes[index];
    return this->kernels[index];
}

float *DiffKernels::k0()
{
    kernel_sizes[0] = 3;
    float *k = new float[kernel_sizes[0]];
    k[0] = 1.0 / 3;
    k[1] = 1.0 / 3;
    k[2] = 1.0 / 3;
    
    return k;
}

float *DiffKernels::k1()
{
    kernel_sizes[1] = 3;
    float *k = new float[kernel_sizes[1]];
    k[0] = 1.0 / 3;
    k[1] = 1.0 / 8;
    k[2] = 1.0 / 3;
    
    return k;
}

float *DiffKernels::k2()
{
    kernel_sizes[2] = 5;
    float *k = new float[kernel_sizes[2]];
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
