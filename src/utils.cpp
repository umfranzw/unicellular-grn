#include "utils.h"

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
