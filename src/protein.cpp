#include "protein.h"
#include "utils.h"
#include <sstream>
#include <bitset>

using namespace std;

Protein::Protein()
{
}

void Protein::init_rand(RandGen gen)
{
    unsigned int mask = make_lower_bitmask(BIND_BITS);
    this->seq = gen() & mask;
    for (int i = 0; i < NUM_GENES; i++)
    {
        this->conc[i] = (float) gen() / RAND_MAX;
    }
}

string Protein::str()
{
    stringstream s;
    s << "(" << this->seq << " {";
    for (int i = 0; i < NUM_GENES; i++)
    {
        s << this->conc[i];
        if (i < NUM_GENES - 1)
        {
            s << " ";
        }
    }
    s << ")" << endl;

    return s.str();
}

