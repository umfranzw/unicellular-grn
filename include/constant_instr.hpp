#ifndef _CONSTANT_INSTR_HPP
#define _CONSTANT_INSTR_HPP

#include "instr.hpp"

using namespace std;

class ConstantInstr : public Instr {
public:
    ConstantInstr(int min_args, int max_args, int type) : Instr(min_args, max_args, type) {
    }

    virtual ~ConstantInstr() {
    }

    virtual void set_val(void *val) = 0;
};

#endif
