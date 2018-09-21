#ifndef _INSTR_FACTORY_HPP
#define _INSTR_FACTORY_HPP

#include "instr.hpp"
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

//note: should comment out the ones you're not currently using
//will fix this later...
typedef enum INSTR_TYPES {
    ADD = 0,
    SUB,
    MULT,
    DIV,
    EQ,
    IF,
    LIST,
    FLOAT_CONST,
    INT_CONST,
    //FRAC_CONST,
    VAR_CONST,
    NUM_INSTR_TYPES
} INSTR_TYPES;

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

private:
    InstrFactory(Run *run);
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
