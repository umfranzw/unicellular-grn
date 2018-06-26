#ifndef _IF_EXP_HPP
#define _IF_EXP_HPP

#include "s_exp/list_exp.hpp"
#include "s_exp/sym_atom.hpp"

using namespace std;

class IfExp : public ListExp {
public:
    IfExp(ListExp *test_form, ListExp *then_form, ListExp *else_form);
    IfExp();
    size_t num_slots();
    void set_slot(int index, ListExp *form);
};

#endif
