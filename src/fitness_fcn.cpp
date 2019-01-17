#include "fitness_fcn.hpp"

float FitnessFcn::eval(Phenotype *ptype, vector<Instr*> *input_params) {
    float fitness = 100.0f;

    fitness += abs(ptype->size() - 3) * 10.0f;
    
    int empty_nodes = ptype->get_num_unfilled_nodes();
    if (empty_nodes == 0) {
        Program pgm = Program(ptype);
        float test_ratio = FitnessFcn::test_pgm(&pgm, input_params);
        //cout << test_ratio << endl;
        fitness -= test_ratio * 100.0f;
    }
    else {
        fitness += empty_nodes * 10.0f;
    }

    return fitness;
}

//returns success ratio
float FitnessFcn::test_pgm(Program *pgm, vector<Instr*> *input_params) {
    int passed = 0;
    int failed = 0;

    string output;
    vector<string> args;

    args.push_back("2");
    args.push_back("2");
    output = pgm->run(input_params, &args);
    if (output == "4") {
        passed++;
    }
    else {
        failed++;
        cout << (int) output[1] << endl;
    }
    args.clear();
    
    args.push_back("2");
    args.push_back("3");
    output = pgm->run(input_params, &args);
    if (output == "6") {
        passed++;
    }
    else {
        failed++;
    }
    args.clear();
    
    return passed / (float) (passed + failed);
}
