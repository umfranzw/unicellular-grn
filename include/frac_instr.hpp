#ifndef _FRAC_INSTR_HPP
#define _FRAC_INSTR_HPP

#include "typed_instr.hpp"
#include "instr_types.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

//for LISP's fraction type
class FracInstr : public TypedInstr<pair<int, int>> {
public:
    FracInstr(pair<int, int> val) : TypedInstr<pair<int, int>>(0, 0, val, FRAC_CONST) {
    }

    string to_code(vector<string> *args) {
        this->check_args(args);
        
        stringstream code;
        code << this->val.first << "/" << this->val.second;
        return code.str();
    }

    string to_str() {
        stringstream ss;
        ss << this->val.first << "/" << this->val.second;
        
        return ss.str();
    }

    FracInstr (FracInstr *other) : TypedInstr<pair<int, int>>(0, 0, other->val, other->type) {
    }

    FracInstr *clone() {
        return new FracInstr(this);
    }

    ~FracInstr() {
    }
};

#endif
