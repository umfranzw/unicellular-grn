#include <iostream>
#include "runs.hpp"
#include "ga.hpp"
#include "cxxopts/include/cxxopts.hpp"
#include "test.hpp"

using namespace std;

cxxopts::Options create_options() {
    cxxopts::Options options("GRN", "Simulation");
    options.add_options()
        ("test", "Run tests")
        ;

    return options;
}

int main(int argc, char *argv[])
{
    cxxopts::Options cmd_opts("GRN", "Simulation");
    cmd_opts.add_options()
        ("t,test", "Run tests")
        ;
    auto result = cmd_opts.parse(argc, argv);
    if (result.count("test") == 1) {
        Test test;
        test.run();
    }

    else {
        Runs parser;
        vector<Run> runs;
        parser.get_runs(&runs);
        for (int i = 0; i < (int) runs.size(); i++) {
            cout << "Run " << i + 1 << " (of " << runs.size() << ")" << endl;
            Ga ga(&runs[i]);
            ga.run_alg();
        }
    }

    return EXIT_SUCCESS;
}
