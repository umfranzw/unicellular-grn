#include "instr_factory.hpp"

InstrFactory *InstrFactory::instance = nullptr;

InstrFactory *InstrFactory::create(Run *run) {
    if (InstrFactory::instance == nullptr) {
        InstrFactory::instance = new InstrFactory(run);
    }

    return InstrFactory::instance;
}

InstrFactory::InstrFactory(Run *run) {
    this->run = run;
    this->init_active_instr_types();
    this->init_F();
    this->init_vars();
    this->init_T();

    this->type_bits = (int) ceil(log2((float) this->active_instr_types.size()));
    
    int max_vec_size = 0;
    for (pair<int, vector<Instr*>> item : this->T) {
        max_vec_size = max(max_vec_size, (int) item.second.size());
    }
    this->const_sel_bits = (int) ceil(log2(max_vec_size));
    
    if (this->type_bits + this->const_sel_bits > this->run->gene_bits) {
        cerr << "Error: num_bits is too small to support the number of instructions given." << endl;
        exit(1);
    }
}

InstrFactory::~InstrFactory() {
    //delete maps
    for (pair<int, Instr *> item : this->F) {
        delete item.second;
    }

    for (pair<int, vector<Instr *>> item : this->T) {
        for (Instr *instr : item.second) {
            delete instr;
        }
    }    
}

Instr *InstrFactory::create_instr(BitVec *seq) {
    int type = (int) this->seq_to_type(seq);
    Instr *instr;
    if (this->F.find(type) != this->F.end()) {
        instr = this->F[type]->clone();
    }
    else {
        int sel = (int) this->seq_to_sel(seq);
        instr = this->T[type][sel]->clone();
    }

    return instr;
}

// vector<int> InstrFactory::filter_by_args(int num) {
//     vector<int> result;
//     for (pair<int, Instr *> item : this->F) {
//         if (num >= item.second->min_args && (num <= item.second->max_args || num == UNLIMITED_ARGS)) {
//             result.push_back(item.first);
//         }
//     }

//     if (num == 0) {
//         for (pair<int, Instr *> item : this->T) {
//             result.push_back(item.first);
//         }
//     }
    
//     return result;
// }

unsigned int InstrFactory::seq_to_type(BitVec *seq) {
    BitVec type_bits = (*seq) >> (this->run->gene_bits - this->type_bits);
    unsigned int type_index = BitVec::to_uint(&type_bits) % (unsigned int) this->active_instr_types.size();
    unsigned int type = (unsigned int) this->active_instr_types[type_index];

    return type;
}

unsigned int InstrFactory::seq_to_sel(BitVec *seq) {
    unsigned int type = this->seq_to_type(seq);
    BitVec sel_bits = (*seq) << this->type_bits;
    sel_bits >>= (this->run->gene_bits - this->type_bits);
    unsigned int sel_val = BitVec::to_uint(&sel_bits);
    unsigned int sel = sel_val % (unsigned int) this->T[type].size();

    return sel;
}

InstrInfo InstrFactory::seq_to_instr_info(BitVec *seq) {
    unsigned int type = this->seq_to_type(seq);
    Instr *instr = nullptr;
    bool is_term = false;

    auto fcn_it = this->F.find(type);
    if (fcn_it != this->F.end()) {
        instr = fcn_it->second;
    }

    else {
        auto term_it = this->T.find(type);
        if (term_it != this->T.end()) {
            //we need to select an constant/variable from the vector
            int sel = this->seq_to_sel(seq);
            instr = this->T[type][sel];
            is_term = true;
        }
    }

    return InstrInfo{instr->min_args, instr->max_args, is_term};
}

Instr *InstrFactory::get_rand_instr(int num_args) {
    Instr *result = nullptr;
    
    vector<Instr*> filtered;
    for (auto it = this->active_instr_types.begin(); it != this->active_instr_types.end(); it++) {
        int type = *it;
        auto instr_it = this->F.find(type);
        if (instr_it != this->F.end()) {
            Instr *instr = instr_it->second;
            if (num_args >= instr->min_args &&
                (num_args <= instr->max_args || instr->max_args == UNLIMITED_ARGS)) {
                filtered.push_back(instr);
            }
        }
        else if (num_args == 0) { //all terminals take zero args
            vector<Instr*> instrs = this->T[type];
            for (auto term_it = instrs.begin(); term_it != instrs.end(); term_it++) {
                filtered.push_back(*term_it);
            }
        }
    }

    if ((int) filtered.size() > 0) {
        int index = this->run->rand->in_range(0, filtered.size());
        result = filtered[index];
    }
    
    return result;
}

vector<Instr*> *InstrFactory::get_vars() {
    return &this->vars;
}

void InstrFactory::init_active_instr_types() {
    this->active_instr_types.push_back(ADD);
    this->active_instr_types.push_back(SUB);
    this->active_instr_types.push_back(MULT);
    this->active_instr_types.push_back(DIV);
    //this->active_instr_types.push_back(EQ);
    //this->active_instr_types.push_back(IF);
    //this->active_instr_types.push_back(LIST);
    //this->active_instr_types.push_back(FLOAT_CONST);
    //this->active_instr_types.push_back(INT_CONST);
    this->active_instr_types.push_back(VAR_CONST);
}

void InstrFactory::init_vars() {
    this->vars.push_back((Instr *) new SymInstr("x0", VAR_CONST));
    this->vars.push_back((Instr *) new SymInstr("x1", VAR_CONST));
}

void InstrFactory::init_F() {
    this->F[ADD] = (Instr *) new FnCallInstr(1, UNLIMITED_ARGS, "+", ADD);
    this->F[SUB] = (Instr *) new FnCallInstr(1, UNLIMITED_ARGS, "-", SUB);
    this->F[MULT] = (Instr *) new FnCallInstr(1, UNLIMITED_ARGS, "*", MULT);
    this->F[DIV] = (Instr *) new FnCallInstr(1, UNLIMITED_ARGS, "//", DIV);
    this->F[EQ] = (Instr *) new FnCallInstr(2, 2, "=", EQ);
    this->F[IF] = (Instr *) new FnCallInstr(3, 3, "if", IF);
    this->F[LIST] = (Instr *) new FnCallInstr(0, UNLIMITED_ARGS, "list", LIST);
}

void InstrFactory::init_T() {
    vector<Instr*> floats;
    floats.push_back((Instr *) new FloatInstr(0.0f));
    this->T[FLOAT_CONST] = floats;

    vector<Instr*> ints;
    ints.push_back((Instr *) new IntInstr(0));
    ints.push_back((Instr *) new IntInstr(1));
    ints.push_back((Instr *) new IntInstr(2));
    this->T[INT_CONST] = ints;

    this->T[VAR_CONST] = this->vars;
}
