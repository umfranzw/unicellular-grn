#ifndef _LIST_EXP_HPP
#define _LIST_EXP_HPP

#include "s_exp/sym_exp.hpp"
#include <vector>
#include <string>

using namespace std;

class ListExp : public SymExp {
public:
    ListExp(vector<SymExp*> elements);
    ListExp(int size);
    ListExp();
    virtual ~ListExp();
    size_t size();
    virtual string to_code();
    virtual ListExp* clone();
    virtual size_t num_slots();
    virtual void set_slot(int index, SymExp *form);
    
protected:
    vector<SymExp*> elements;
};

#endif
