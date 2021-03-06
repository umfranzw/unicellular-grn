#ifndef _FN_CALL_INSTR_HPP
#define _FN_CALL_INSTR_HPP

#include "sym_instr.hpp"
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class FnCallInstr : public SymInstr {
public:
    FnCallInstr(int min_args, int max_args, string val, int type) : SymInstr(min_args, max_args, val, type) {
    }

    string to_code(vector<string> *args) {
        this->check_args(args);

        stringstream code;
        code << "(" << val;
        for (int i = 0; args != nullptr && i < (int) args->size(); i++) {
            code << " " << (*args)[i];
        }
        code << ")" << endl;

        return code.str();
    }

    string to_str() {
        stringstream ss;
        ss << val;

        return ss.str();
    }
    
    FnCallInstr (FnCallInstr *other) : SymInstr(other->min_args, other->max_args, other->val, other->type) {
    }

    FnCallInstr *clone() {
        return new FnCallInstr(this);
    }

    ~FnCallInstr() {
    }
};

#endif
