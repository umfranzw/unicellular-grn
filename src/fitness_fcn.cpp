#include "fitness_fcn.hpp"

float FitnessFcn::eval(Phenotype *ptype, vector<Instr*> *input_params) {
    float fitness = 0.0f;

    fitness += abs(ptype->size() - 3) * 80.0f;
    
    int empty_nodes = ptype->get_num_unfilled_nodes();
    fitness += empty_nodes * 40.0f;

    // int var_occurs = ptype->count_var_occurs();
    // if (var_occurs > 0) {
    //     cout << ptype->to_code() << endl;
    //     fitness = max(fitness - 20.0f, 0.0f);
    // }

    if (empty_nodes == 0) {
        Program pgm = Program(ptype);
        float success_ratio = FitnessFcn::test_pgm(&pgm, input_params);
        fitness += (1 - success_ratio) * 10.0f;
    }

    return fitness;

    //return abs(ptype->size() - 3) * 10.0f;
}

float FitnessFcn::test_pgm2(Program *pgm, vector<Instr*> *input_params) {
    int passed = 0;
    int failed = 0;

    string output;
    vector<string> args;

    args.push_back("0");
    args.push_back("0");
    output = pgm->run(input_params, &args);
    if (output == "0") {
        passed++;
    }
    else {
        failed++;
    }
    args.clear();

    args.push_back("0");
    args.push_back("1");
    output = pgm->run(input_params, &args);
    if (output == "1") {
        passed++;
    }
    else {
        failed++;
    }

    return passed / (float) (passed + failed);
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
    //cout << "output: " << output << endl;
    if (output == "4") {
        //cout << "Passed" << endl;
        passed++;
    }
    else {
        //cout << "Failed" << endl;
        failed++;
    }
    args.clear();
    
    args.push_back("2");
    args.push_back("4");
    output = pgm->run(input_params, &args);
    //cout << "output: " << output << endl;
    if (output == "8") {
        //cout << "Passed" << endl;
        passed++;
    }
    else {
        //cout << "Failed" << endl;
        failed++;
    }
    args.clear();
    
    return passed / (float) (passed + failed);
}
