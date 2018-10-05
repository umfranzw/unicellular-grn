#ifndef _PROGRAM_HPP
#define _PROGRAM_HPP

#include <string>
#include "phenotype.hpp"
#include "node.hpp"

using namespace std;

class Program {
public:
    Program(Phenotype *ptype);
    ~Program();
    string run(vector<Instr*> *params, vector<string> *args);

    string code;
};

#endif
