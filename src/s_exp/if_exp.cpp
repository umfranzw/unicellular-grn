#include "s_exp/if_exp.hpp"
#include <vector>

IfExp::IfExp(ListExp *test_form, ListExp *then_form, ListExp *else_form) : ListExp() {
    this->elements.push_back(new SymAtom("if"));
    this->elements.push_back(test_form);
    this->elements.push_back(then_form);
    this->elements.push_back(else_form);
}

IfExp::IfExp() : ListExp() {
    this->elements.push_back(new SymAtom("if"));
    for (int i = 0; i < 3; i++) {
        this->elements.push_back(new SymAtom("nil"));
    }
}

size_t IfExp::num_slots() {
    return 3;
}

void IfExp::set_slot(int index, ListExp *form) {
    delete this->elements[index + 1];
    this->elements[index + 1] = form;
}
