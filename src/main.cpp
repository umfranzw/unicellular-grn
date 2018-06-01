#include <iostream>
#include "runs.hpp"
#include "ga.hpp"
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
    Runs parser;
    vector<Run> runs;
    parser.get_runs(&runs);
    for (int i = 0; i < (int) runs.size(); i++) {
        cout << "Run " << i + 1 << " (of " << runs.size() << ")" << endl;
        Ga ga(&runs[i]);
        ga.run_alg();
    }

    return EXIT_SUCCESS;
}
