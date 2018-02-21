#include "protein.h"
#include "utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <bitset>

using namespace std;

Protein::Protein()
{
    this->seq = 0;
    this->conc = 0;
}

void Protein::init_rand(mt19937 gen)
{
    unsigned int mask = make_lower_bitmask(BIND_BITS);
    this->seq = gen() & mask;
    this->conc = (float) gen() / RAND_MAX;
}

string Protein::str(bool compact)
{
    stringstream s;
    if (compact)
    {
        s << "(" << this->seq << " " << this->conc << ")" << endl;
    }

    else
    {
        s << "Protein(" << endl;
        s << setw(20) << "seq : " << this->seq << endl;
        s << ")" << endl;
    }

    return s.str();
}

