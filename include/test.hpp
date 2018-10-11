#ifndef _TEST_HPP
#define _TEST_HPP

#include "runs.hpp"
#include "bitvec.hpp"

using namespace std;

class Test {
public:
    Test();
    ~Test();
    void run();

private:
    Run *create_run();
    void bitvec(Run *run);
    void to_code(Run *run);
    void fitness(Run *run);

    void reset_bitvec(BitVec *vec, string val);
};

#endif
