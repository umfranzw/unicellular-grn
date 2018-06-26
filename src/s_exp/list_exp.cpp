#include "s_exp/list_exp.hpp"
#include "s_exp/sym_atom.hpp"
#include <sstream>

ListExp::ListExp(vector<SymExp*> elements) {
    this->elements = elements;
}

ListExp::ListExp(int size) {
    for (int i = 0; i < size; i++) {
        this->elements.push_back(new SymAtom("nil"));
    }
}

ListExp::ListExp() {
}

ListExp::~ListExp() {
    for (SymExp *exp : this->elements) {
        delete exp;
    }
}

size_t ListExp::size() {
    return this->elements.size();
}

ListExp* ListExp::clone() {
    vector<SymExp*> elements;
    for (SymExp *exp : this->elements) {
        elements.push_back(exp->clone());
    }

    return new ListExp(elements);
}

string ListExp::to_code() {
    stringstream code;
    code << "(";
    for (int i = 0; i < (int) this->elements.size(); i++) {
        code << this->elements[i]->to_code();
        if (i < (int) this->elements.size() - 1) {
            code << " ";
        }
    }
    code << ")";

    return code.str();
}

size_t ListExp::num_slots() {
    return this->elements.size();
}

void ListExp::set_slot(int index, SymExp *form) {
    delete this->elements[index];
    this->elements[index] = form;
}
