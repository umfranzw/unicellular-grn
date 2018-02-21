#ifndef PROTEIN_H
#define PROTEIN_H

#include <bitset>
#include "constants.h"
#include <string>
#include <random>
#include "gene.h"

using namespace std;

class Gene;

class Protein
{
public:
    bitset<BIND_BITS> seq;
    float conc;
    Gene *src;
    
    Protein();
    void init_rand(mt19937 gen);
    string str(bool compact=false);
};

#endif
