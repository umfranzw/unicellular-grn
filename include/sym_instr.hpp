#ifndef _SYM_INSTR_HPP
#define _SYM_INSTR_HPP

#include "typed_instr.hpp"
#include <string>
#include <sstream>
#include <vector>

class SymInstr : public TypedInstr<string> {
public:
    SymInstr(string val) : TypedInstr<string>(0, 0, val) {
    }
    
    SymInstr(int min_args, int max_args, string val) : TypedInstr<string>(min_args, max_args, val) {
    }

    string to_code(vector<string> *args) {
        stringstream code;
        code << this->val;
        return code.str();
    }

    SymInstr(SymInstr *other) : TypedInstr<string>(0, 0, other->val) {
    }

    SymInstr *clone() {
        return new SymInstr(this);
    }

    virtual ~SymInstr() {
    }
};

#endif
