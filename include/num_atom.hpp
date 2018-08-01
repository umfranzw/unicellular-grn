#ifndef _FLOAT_ATOM_HPP
#define _FLOAT_ATOM_HPP

#include "typed_atom.hpp"
#include <iostream>

using namespace std;

class FloatAtom : public TypedAtom<float> {
    FloatAtom(float val) : TypedAtom<float>(val) {
    }

    string to_code() {
        stringstream code;
        code << showpoint << this->val;
        return code.str();
    }
};

class IntAtom : public TypedAtom<int> {
    IntAtom(int val) : TypedAtom<int>(val) {
    }

    string to_code() {
        stringstream code;
        code << this->val;
        return code.str();
    }
};

//for LISP's fraction type
class FracAtom : public TypedAtom<pair<int, int>> {
    FracAtom(float val) : TypedAtom<float>(val) {
    }

    string to_code() {
        stringstream code;
        code << this->val.first << "/" << this->val.second;
        return code.str();
    }
};

#endif
