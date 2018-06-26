#include "s_exp/sym_atom.hpp"

SymAtom::SymAtom(string name) : AtomicExp<string>(name) {
}

string SymAtom::to_code() {
    stringstream code;
    code << this->val;
        
    return code.str();
}

SymAtom *SymAtom::clone() {
    return new SymAtom(this->val);
}
    
