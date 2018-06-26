#ifndef _ATOMIC_EXP_HPP
#define _ATOMIC_EXP_HPP

#include "s_exp/sym_exp.hpp"
#include <string>
#include <sstream>

using namespace std;

template <class T>
class AtomicExp : public SymExp {
public:
    AtomicExp(T val) {
        this->val = val;
    }
    virtual string to_code() = 0;
    
protected:
    T val;
};

#endif
