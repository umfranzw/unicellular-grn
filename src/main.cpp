#include <iostream>
#include "runs.hpp"
#include "ga.hpp"
#include "cxxopts/include/cxxopts.hpp"
#include "test.hpp"

const string DEFAULT_RUN_FILE = "runs.toml";

using namespace std;

cxxopts::Options create_options() {
    cxxopts::Options options("GRN", "Simulation");
    options.add_options()
        ("t,test", "Run tests")
        ("f,file", "Run file (TOML)", cxxopts::value<string>())
        ;
    options.parse_positional("file");

    return options;
}

int main(int argc, char *argv[])
{
    cxxopts::Options cmd_opts = create_options();
    auto result = cmd_opts.parse(argc, argv);
    string run_file = DEFAULT_RUN_FILE;

    if (result.count("file") > 0) {
        run_file = result["file"].as<string>();
    }
    
    if (result.count("test") > 0) {
        Test test;
        test.run();
    }

    else {
        Runs parser(run_file);
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
