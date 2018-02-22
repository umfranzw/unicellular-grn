#include "gene.h"
#include "utils.h"
#include "constants.h"
#include <sstream>
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

void Gene::init_rand(RandGen &gen)
{
    unsigned int mask = make_lower_bitmask(BIND_BITS);
    this->binding_seq = gen() & mask;
    mask = make_lower_bitmask(OUTPUT_BITS);
    this->output_seq = gen() & mask;
    this->threshold = (float) gen() / gen.max();
    this->out_rate = (float) gen() / gen.max();
    this->diff_index = gen() % NUM_DIFF_KERNELS; //note: the mod operation biases the distribution
}

string Gene::str()
{
    stringstream s;
    s << "[" << this->binding_seq << " : " << this->output_seq << " | " << this->threshold << " " << this->out_rate << " " << this->diff_index << "]";

    return s.str();
}

