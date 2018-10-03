#ifndef _TEST_HPP
#define TEST_HPP

#include "instr_factory.hpp"
#include "tree.hpp"
#include "bitvec.hpp"
#include "runs.hpp"

using namespace std;

class Test {
public:
    Test();
    ~Test();
    void run();

private:
    Run *create_run();
    void to_code(Run *run);
};

#endif
