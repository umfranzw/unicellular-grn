#include <string>

using namespace std;

Protein::Protein()
{
    this.seq = "";
}

ostream& operator<<(ostream& strm, const Protein &p) {
    strm << "Protein(" << endl;
    strm << "\tseq : " << p.seq << endl;
    strm << ")";
    
    return strm;
}

