#ifndef _TYPED_ATOM_HPP
#define _TYPED_ATOM_HPP

#include "atom.hpp"
#include <string>
#include <sstream>

template <class T>
class TypedAtom : public Atom {
public:
    TypedAtom(T val) : Atom() {
        this->val = val;
    }

    string to_code() {
        stringstream code;
        code << this->val;
        return code.str();
    }

    TypedAtom *clone() {
        return new TypedAtom<T>(this->val);
    }

protected:
    T val;
};

#endif
