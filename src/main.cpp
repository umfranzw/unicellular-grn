#include <iostream>
#include "runs.hpp"
#include "ga.hpp"
#include "cxxopts/include/cxxopts.hpp"
#include "test.hpp"

using namespace std;

cxxopts::Options create_options() {
    cxxopts::Options options("GRN", "Simulation");
    options.add_options()
        ("t,test", "Run tests")
        ;

    return options;
}

int main(int argc, char *argv[])
{
    cxxopts::Options cmd_opts = create_options();
    auto result = cmd_opts.parse(argc, argv);
    if (result.count("test") == 1) {
        Test test;
        test.run();
    }

    else {
        Runs parser;
        Run *run;
        for (int i = 0; i < parser.size; i++) {
            run = parser.get_next();
            cout << "Run " << i + 1 << " (of " << parser.size << ")" << endl;
            Ga ga(run);
            ga.run_alg();
        }
    }

    return EXIT_SUCCESS;
}
