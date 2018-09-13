#include "instr_factory.hpp"
#include "constant_instr.hpp"

InstrFactory *InstrFactory::instance = nullptr;

InstrFactory *InstrFactory::create(Run *run) {
    if (InstrFactory::instance == nullptr) {
        InstrFactory::instance = new InstrFactory(run);
    }

    return InstrFactory::instance;
}

InstrFactory::InstrFactory(Run *run) {
    this->run = run;
    this->init_F();
    this->init_vars();
    this->init_T();

    this->type_bits = (int) ceil(log2((float) NUM_INSTR_TYPES));
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
    
    //delete instance
    delete InstrFactory::instance;
}

Instr *InstrFactory::create_instr(int instr_type, int instr_sel) {
    Instr *instr;
    if (this->F.find(instr_type) != this->F.end()) {
        instr = this->F[instr_type]->clone();
    }
    else {
        instr = this->T[instr_type][instr_sel]->clone();
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
    unsigned int type_val = BitVec::to_uint(&type_bits);
    unsigned int type = type_val % NUM_INSTR_TYPES;

    return type;
}

unsigned int InstrFactory::seq_to_sel(BitVec *seq) {
    unsigned int type = this->seq_to_type(seq);
    BitVec sel_bits = (*seq) << this->type_bits;
    sel_bits >>= this->const_sel_bits;
    unsigned int sel_val = BitVec::to_uint(&sel_bits);
    unsigned int sel = sel_val % (int) this->T[type].size();

    return sel;
}

pair<int, int> InstrFactory::seq_to_arg_range(BitVec *seq) {
    unsigned int type = this->seq_to_type(seq);
    Instr *instr = nullptr;

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
        }
    }
    
    return pair<int, int>(instr->min_args, instr->max_args);
}

vector<Instr*> *InstrFactory::get_vars() {
    return &this->vars;
}

void InstrFactory::init_vars() {
    this->vars.push_back((Instr *) new SymInstr("x0"));
    this->vars.push_back((Instr *) new SymInstr("x1"));
}

void InstrFactory::init_F() {
    this->F[ADD] = (Instr *) new FnCallInstr(1, UNLIMITED_ARGS, "+");
    this->F[SUB] = (Instr *) new FnCallInstr(1, UNLIMITED_ARGS, "-");
    this->F[MULT] = (Instr *) new FnCallInstr(1, UNLIMITED_ARGS, "*");
    this->F[DIV] = (Instr *) new FnCallInstr(1, UNLIMITED_ARGS, "//");
    this->F[EQ] = (Instr *) new FnCallInstr(2, 2, "=");
    this->F[IF] = (Instr *) new FnCallInstr(3, 3, "if");
    this->F[LIST] = (Instr *) new FnCallInstr(0, UNLIMITED_ARGS, "list");
}

void InstrFactory::init_T() {
    vector<Instr*> floats;
    floats.push_back((Instr *) new FloatInstr(0.0f));

    vector<Instr*> ints;
    ints.push_back((Instr *) new IntInstr(0));

    this->T[FLOAT_CONST] = floats;
    this->T[INT_CONST] = ints;
    this->T[VAR_CONST] = this->vars;
}
