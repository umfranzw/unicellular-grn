#include "grn.h"
#include "constants.h"
#include <sstream>

GRN::GRN()
{
}

void GRN::init_rand(mt19937 &gen)
{
    for (int i = 0; i < NUM_GENES; i++)
    {
        this->genes[i].init_rand(gen);
    }
}    

string GRN::str(bool compact)
{
    stringstream s;
    
    for (int i = 0; i < NUM_GENES; i++)
    {
        s << genes[i].str(compact);
    }

    return s.str();
}
