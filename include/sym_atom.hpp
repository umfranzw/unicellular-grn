#ifndef _SYM_ATOM_HPP
#define _SYM_ATOM_HPP

#include <string>
#include "typed_atom.hpp"

class SymAtom : public TypedAtom<string> {
public:
    SymAtom(string val) : TypedAtom<string>(val);
};

#endif
