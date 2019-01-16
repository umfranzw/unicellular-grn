#include <iostream>
#include "tclap/CmdLine.h"
#include "runs.hpp"
#include "ga.hpp"
#include "rand_search.hpp"
#include "test.hpp"

const string DEFAULT_RUN_FILE = "runs.toml";

using namespace std;

int main(int argc, char *argv[])
{
    TCLAP::CmdLine cmd("Unicellular GRN Simulation", ' ', "0.1");
    TCLAP::SwitchArg test_switch("t", "test", "Run tests", cmd, false);
    TCLAP::SwitchArg rand_switch("r", "rand", "Run Random Search Alg", cmd, false);
    TCLAP::UnlabeledValueArg<string> runfile_arg("runfile", "filename", false, DEFAULT_RUN_FILE, "path");
    cmd.add(runfile_arg);

    cmd.parse(argc, argv);
    string run_file = runfile_arg.getValue();
    bool testing = test_switch.getValue();
    bool rand_alg = rand_switch.getValue();
    
    if (testing) {
        Test test;
        test.run();
    }

    else {
        Runs parser(run_file);
        Run *run;
        for (int i = 0; i < parser.size; i++) {
            run = parser.get_next();
            cout << "Run " << i + 1 << " (of " << parser.size << ")" << endl;
            
            if (rand_alg) {
                RandSearch rs(run);
                rs.run_alg();
            }
            else {
                Ga ga(run);
                ga.run_alg();
            }
        }
    }

    return EXIT_SUCCESS;
}
