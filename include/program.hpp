#ifndef _PROGRAM_HPP
#define _PROGRAM_HPP

#include <string>
#include "phenotype.hpp"
#include "node.hpp"

using namespace std;

class Program {
public:
    Program(Phenotype *ptype, vector<Instr*> *args);
    ~Program();
    string run(vector<Instr*> *args);

    string code;
    int empty_nodes;
};

#endif
