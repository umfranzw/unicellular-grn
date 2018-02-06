#include "gene.h"
#include <iostream>
#include <string>
#include "protein.h"

using namespace std;

Gene::Gene()
{
    //this->binding_seq = Protein();
    //this->protein_seq = Protein();
    this->threshold = 0.0;
    this->prod_rate = 0.0;
}

ostream& operator<<(ostream& strm, const Gene &g) {
    strm << "Gene(" << endl;
    //strm << "\tbinding_seq : " << g.binding_seq << endl;
    //strm << "\tprotein_seq : " << g.protein_seq << endl;
    strm << "\t  threshold : " << g.threshold << endl;
    strm << "\t  prod_rate : " << g.prod_rate << endl;
    strm << ")";
    
    return strm;
}

