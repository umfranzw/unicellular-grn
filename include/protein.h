#ifndef PROTEIN_H
#define PROTEIN_H

#include <string>

using namespace std;

class Protein
{
public:
    string seq;
    
    Protein();
    friend ostream& operator<<(ostream& strm, const Protein &p);
};

#endif
