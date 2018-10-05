#include "program.hpp"
#include "lisp_worker.hpp"
#include <sstream>

Program::Program(Phenotype *ptype) {
    this->code = ptype->to_code();
}

Program::~Program() {
}

string Program::run(vector<Instr*> *params, vector<string> *args) {
    //call main with the args
    stringstream buf;
    buf << "(LET (";
    for (int i = 0; i < (int) params->size(); i++) {
        buf << "(";
        buf << (*params)[i]->to_code(nullptr);
        buf << " ";
        buf << (*args)[i];
        buf << ")";
    }
    buf << ")";
    buf << this->code;
    buf << ")";
    
    LispWorker worker = LispWorker(buf.str());
    worker.run();

    return worker.get_output();
}
