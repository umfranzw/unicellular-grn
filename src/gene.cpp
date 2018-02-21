#include "gene.h"
#include "utils.h"
#include "constants.h"
#include <sstream>
#include <iomanip>
#include "protein.h"

using namespace std;

Gene::Gene()
{
    this->binding_seq = 0;
    this->bound = NULL;
    this->output_seq = 0;
    this->threshold = 0.0;
    this->out_rate = 0.0;
    this->diff_index = 0;
}

void Gene::init_rand(mt19937 &gen)
{
    unsigned int mask = make_lower_bitmask(BIND_BITS);
    this->binding_seq = gen() & mask;
    mask = make_lower_bitmask(OUTPUT_BITS);
    this->output_seq = gen() & mask;
    this->threshold = (float) gen() / gen.max();
    this->out_rate = (float) gen() / gen.max();
    this->diff_index = gen() % NUM_DIFF_FCNS; //note: the mod operation biases the distribution
}

string Gene::str(bool compact)
{
    stringstream s;
    if (compact)
    {
        s << "[" << this->binding_seq << " : " << this->output_seq << " | " << this->threshold << " " << this->out_rate << " " << this->diff_index << "]";
    }

    else
    {
        s << "Gene(" << endl;
        s << setw(20) << "binding_seq : " << this->binding_seq << endl;
        s << setw(20) << "output_seq : " << this->output_seq << endl;
        s << setw(20) << "threshold : " << this->threshold << endl;
        s << setw(20) << "out_rate : " << this->out_rate << endl;
        s << setw(20) << "diff_index : " << this->diff_index << endl;
        s << ")";
    }

    return s.str();
}

