#ifndef _FN_CALL_INSTR_HPP
#define _FN_CALL_INSTR_HPP

#include "sym_instr.hpp"
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class FnCallInstr : public SymInstr {
public:
    FnCallInstr(int min_args, int max_args, string val) : SymInstr(min_args, max_args, val) {
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
    
    FnCallInstr (FnCallInstr *other) : SymInstr(other->min_args, other->max_args, other->val) {
    }

    FnCallInstr *clone() {
        return new FnCallInstr(this);
    }

    ~FnCallInstr() {
    }
};

#endif
