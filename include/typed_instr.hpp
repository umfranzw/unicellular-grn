#ifndef _TYPED_INSTR_HPP
#define _TYPED_INSTR_HPP

#include "constant_instr.hpp"
#include <string>

using namespace std;

template <class T>
class TypedInstr : public ConstantInstr {
public:
    TypedInstr(int min_args, int max_args, T val) : ConstantInstr(min_args, max_args) {
        this->val = val;
    }

    virtual ~TypedInstr() {
    }

    void set_val(void *val) {
        T *typed_val = (T*) val;
        this->val = *typed_val;
    }

protected:
    T val;
};

#endif
