#include "program.hpp"
#include "lisp_worker.hpp"
#include <sstream>

Program::Program(Phenotype *ptype, vector<Instr*> *params) {
    this->empty_nodes = ptype->tree->get_num_unfilled_nodes();
    if (empty_nodes == 0) {
        stringstream buf;

        buf << "(DEFUN main (";
        for (int i = 0; i < (int) params->size(); i++) {
            buf << (*params)[i]->to_code(nullptr);
            if (i < (int) params->size() - 1) {
                buf << " ";
            }
        }
        buf << ")";
        buf << ptype->tree->to_code();
        buf << ")" << endl;
        
        this->code = buf.str();
    }
    else {
        this->code = "";
    }
}

Program::~Program() {
}

string Program::run(vector<Instr*> *args) {
    LispWorker worker = LispWorker(this->code);
    worker.run();

    return worker.get_output();
}
