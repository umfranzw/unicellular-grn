#ifndef _ATOMIC_EXP_HPP
#define _ATOMIC_EXP_HPP

#include <string>

using namespace std;

class Atom {
public:
    Atom();
    virtual ~Atom();
    virtual Atom *clone() = 0;
    virtual string to_code() = 0;    
};

#endif
