#include "grn.h"
#include "constants.h"
#include <sstream>

GRN::GRN()
{
}

void GRN::run_diffusion()
{
    for (vector<Protein *>::iterator it = proteins.begin(); it != proteins.end(); it++)
    {
        //get kernel
        int kernel_index = (*it)->src->diff_index;
        int kernel_size;
        float *kernel = kernels.get_diff_kernel(kernel_index, &kernel_size);
        
        // slide kernel across protein concentration array.
        float *concs = (*it)->conc;
        float updated_concs[NUM_GENES];
        int mid = kernel_size / 2;
        bool min_flag = false;
        for (int i = 0; i < NUM_GENES; i++)
        {
            float new_conc = 0;
            for (int j = 0; j < kernel_size; j++)
            {
                int col = i - mid + j;
                if (col > 0 && col < NUM_GENES)
                {
                    new_conc += concs[col] * kernel[j];
                }
                //else, the contribution of the kernel column is 0
            }
            updated_concs[i] = new_conc;
            min_flag = min_flag && (new_conc < MIN_PROTEIN_CONC);
        }
        if (min_flag)
        {
            //remove protein - all it's concentrations are below the minimum threshold.
            proteins.erase(it);
        }

        else
        {
            //overwrite old concentrations with new ones
            for (int i = 0; i < NUM_GENES; i++)
            {
                concs[i] = updated_concs[i];
            }
        }
    }

    //TODO: What happens when a protein has concentration 0 all throughout the array? Should be removed... but there will be floating point error. Maybe remove it if all elements are below some small error threshold?
}

void GRN::init_rand(RandGen &gen)
{
    for (int i = 0; i < NUM_GENES; i++)
    {
        this->genes[i].init_rand(gen);
    }
}    

string GRN::str()
{
    stringstream s;
    
    for (int i = 0; i < NUM_GENES; i++)
    {
        s << genes[i].str();
    }

    return s.str();
}
