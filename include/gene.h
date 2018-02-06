#ifndef GENE_H
#define GENE_H

#include <string>
#include <iostream>
#include "protein.h"

using namespace std;

class Gene
{
public:
    Protein binding_seq;
    Protein protein_seq;
    float threshold;
    float prod_rate;
    
    Gene();
    friend ostream& operator<<(ostream& strm, const Gene &g);
};

#endif
