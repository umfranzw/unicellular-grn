#ifndef _SYM_ATOM_HPP
#define _SYM_ATOM_HPP

#include <string>
#include "atomic_exp.hpp"

class SymAtom : public AtomicExp<string> {
public:
    SymAtom(string name);
    string to_code();
    SymAtom *clone();
};

#endif
