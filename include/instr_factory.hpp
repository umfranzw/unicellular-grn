#ifndef _INSTR_FACTORY_HPP
#define _INSTR_FACTORY_HPP

#include "instr.hpp"
#include "instr_types.hpp"
#include "float_instr.hpp"
#include "int_instr.hpp"
#include "frac_instr.hpp"
#include "fn_call_instr.hpp"
#include "bitvec.hpp"
#include "runs.hpp"
#include "bitvec.hpp"
#include <map>
#include <vector>

using namespace std;

class InstrFactory {
public:
    static InstrFactory *create(Run *run);
    ~InstrFactory();
    Instr *create_instr(BitVec *seq);
    //vector<int> filter_by_args(int num);
    unsigned int seq_to_type(BitVec *seq);
    unsigned int seq_to_sel(BitVec *seq);
    pair<int, int> seq_to_arg_range(BitVec *seq);
    vector<Instr*> *get_vars();

    vector<int> active_instr_types;

private:
    InstrFactory(Run *run);
    void init_active_instr_types();
    void init_F();
    void init_vars();
    void init_T();
    Run *run;

    int type_bits;
    int const_sel_bits;
    
    static InstrFactory *instance;
    map<int, Instr*> F;
    vector<Instr*> vars;
    map<int, vector<Instr*>> T;
};

#endif
