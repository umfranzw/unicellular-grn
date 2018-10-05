#ifndef _TEST_HPP
#define TEST_HPP

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
    void fitness(Run *run);
};

#endif
