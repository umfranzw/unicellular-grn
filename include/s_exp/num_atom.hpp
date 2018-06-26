#ifndef _NUM_ATOM_HPP
#define _NUM_ATOM_HPP

#include "s_exp/atomic_exp.hpp"
#include <iostream>

using namespace std;

template <class T>
class NumAtom : public AtomicExp<T> {
public:
    NumAtom(float val) : AtomicExp<float>(val) {
        stringstream code;
        code << showpoint << this->val;
        this->code = code.str();
    }
    
    NumAtom(int val) : AtomicExp<int>(val) {
        stringstream code;
        code << this->val;
        this->code = code.str();
    }

    //for fractions
    NumAtom(pair<int, int> val) : AtomicExp<pair<int, int>>(val) {
        stringstream code;
        code << val.first << "/" << val.second;
        this->code = code.str();
    }

    string to_code() {
        return this->code;
    }

    NumAtom *clone() {
        return new NumAtom(this->val);
    }

private:
    string code;
};

#endif
