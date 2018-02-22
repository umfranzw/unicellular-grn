#include <iostream>
#include "gene.h"
#include "grn.h"
#include "protein.h"
#include "utils.h"

using namespace std;

int main(int argc, char *argv[])
{
    RandGen gen = make_gen();
    GRN g;
    g.init_rand((RandGen&) gen);
    cout << g.str() << endl;

    // Gene g1;
    // g1.init_rand((RandGen&) gen);
    // g2.init_rand((RandGen&) gen);
    // Gene g2;
                                
    // cout << g1.str() << endl;
    // cout << g2.str() << endl;

    return EXIT_SUCCESS;
}
