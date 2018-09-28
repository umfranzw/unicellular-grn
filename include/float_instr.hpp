#ifndef _FLOAT_INSTR_HPP
#define _FLOAT_INSTR_HPP

#include "typed_instr.hpp"
#include "instr_types.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class FloatInstr : public TypedInstr<float> {
public:
    FloatInstr(float val) : TypedInstr<float>(0, 0, val, FLOAT_CONST) {
    }

    string to_code(vector<string> *args) {
        this->check_args(args);
        
        stringstream code;
        code << showpoint << this->val;
        return code.str();
    }

    FloatInstr(FloatInstr *other) : TypedInstr<float>(0, 0, other->val, other->type) {
    }

    FloatInstr *clone() {
        return new FloatInstr(this);
    }

    ~FloatInstr() {
    }
};

#endif
