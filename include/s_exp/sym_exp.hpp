#ifndef _SYM_EXP_HPP
#define _SYM_EXP_HPP

#include <vector>
#include <string>

using namespace std;

class SymExp {
public:
    SymExp();
    virtual ~SymExp();
    virtual string to_code() = 0;
    virtual SymExp* clone() = 0;
};

#endif
