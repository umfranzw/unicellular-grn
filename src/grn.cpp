#include "grn.h"
#include "constants.h"
#include <sstream>

GRN::GRN()
{
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
