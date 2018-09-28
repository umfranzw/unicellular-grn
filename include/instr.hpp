#ifndef _INSTR_HPP
#define _INSTR_HPP

#include <string>
#include <vector>
#include <cassert>

#define UNLIMITED_ARGS -1

using namespace std;

class Instr {
public:
    Instr(int min_args, int max_args, int type) {
        this->min_args = min_args;
        this->max_args = max_args;
        this->type = type;
    }

    virtual string to_code(vector<string> *args) = 0;
    
    //because copy constructors don't support polymorphism
    virtual Instr *clone() = 0;
    
    void check_args(vector<string> *args) {
        assert(
            (args == nullptr && this->min_args == 0 && (this->max_args >= 0 || this->max_args == UNLIMITED_ARGS)) ||
            (args != nullptr && (int) args->size() >= this->min_args && ((int) args->size() <= this->max_args || this->max_args == UNLIMITED_ARGS))
            );
    }

    virtual ~Instr() {
    }

    int min_args;
    int max_args;
    int type;
};

#endif
