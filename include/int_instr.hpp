#ifndef _INT_INSTR_HPP
#define _INT_INSTR_HPP

#include "typed_instr.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class IntInstr : public TypedInstr<int> {
public:
    IntInstr(int val) : TypedInstr<int>(0, 0, val) {
    }

    string to_code(vector<string> *args) {
        this->check_args(args);
        
        stringstream code;
        code << this->val;
        return code.str();
    }

    IntInstr(IntInstr *other) : TypedInstr<int>(0, 0, other->val) {
    }

    IntInstr *clone() {
        return new IntInstr(this);
    }

    ~IntInstr() {
    }
};

#endif
