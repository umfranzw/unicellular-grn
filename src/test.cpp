#include "test.hpp"
#include "phenotype.hpp"
#include "instr_factory.hpp"
#include "tree.hpp"
#include "bitvec.hpp"
#include "program.hpp"
#include "utils.hpp"
#include "fitness_fcn.hpp"
#include "grn.hpp"
#include "reg_sim.hpp"
#include "logger.hpp"
#include <iostream>

Test::Test() {
}

Test::~Test() {
}

void Test::run() {
    Run *run = this->create_run();
    // bitvec(run);
    // to_code(run);
    // program(run);
    //fitness(run);
    variability(run);
    delete run;
}

Run *Test::create_run() {
    Run *run = new Run(false, -1);
    run->pop_size = 1;
    run->ga_steps = 1;
    run->mut_prob = 0.05;
    run->mut_prob_limit = 0.2;
    run->mut_step = 0.0;
    run->cross_frac = 0.4;
    run->cross_frac_limit = 0.0;
    run->cross_step = 0.0;
    run->num_genes = 8;
    run->gene_bits = 8;
    run->term_cutoff = 0.0;

    run->reg_steps = 50;
    run->min_protein_conc = 0.04;
    run->max_protein_conc = 1.0;

    run->decay_rate = 0.05;
    run->initial_proteins = 5;
    run->max_proteins = 20;
    run->max_mut_float = 0.5;
    run->max_mut_bits = 8;

    run->binding_seq_play = 2;

    run->growth_start = 10;
    run->growth_end = 19;
    run->growth_sample_interval = 2;
    run->growth_seq = "11111111";
    run->growth_threshold = 0.25;

    run->code_start = 20;
    run->code_end = 49;
    run->code_sample_interval = 2;

    run->log_mode = "all";

    run->fitness_log_interval = 1;

    run->graph_results = false;

    run->log_grns = false;

    run->log_reg_steps = false;

    run->log_code_with_fitness = false;

    run->fix_rng_seed = false;
    run->fixed_rng_seed = 1207586232;

    run->log_dir = "data/dbs";

    return run;
}

void Test::bitvec(Run *run) {
    BitVec a(1);
    assert(a.size() == 1);
    assert(a[0] == 0);

    BitVec b(5);
    b[1] = 1;
    b[1] = 0;
    b[2] = 1;
    assert(b.size() == 5);
    for (int i = 0; i < b.size(); i++) {
        assert(b[i] == (i == 2));
    }

    assert(b == BitVec("00100"));
    assert(b != BitVec("01000"));
    assert(b << 2 == BitVec("10000"));
    assert(b >> 2 == BitVec("00001"));
    b >>= 3;
    assert(b == BitVec("00000"));
    b[0] = 1;
    b <<= 1;
    assert(b == BitVec("00000"));
    assert(~b == BitVec("11111"));

    string test_val = "01010";
    reset_bitvec(&b, test_val);

    BitVec mask = BitVec("10011");
    assert((b | mask) == BitVec("11011"));
    assert((b & mask) == BitVec("00010"));
    assert((b ^ mask) == BitVec("11001"));

    b |= mask;
    assert(b == BitVec("11011"));
    reset_bitvec(&b, test_val);

    b &= mask;
    assert(b == BitVec("00010"));
    reset_bitvec(&b, test_val);

    b ^= mask;
    assert(b == BitVec("11001"));
    reset_bitvec(&b, test_val);

    assert(b < BitVec("01011"));
    assert(!(BitVec("01011") < b));

    b[4] = 1;
    assert(b.count() == 3);
        
    unsigned int x = 5;
    BitVec *c = BitVec::from_uint(x, 7);
    assert(c->size() == 7);
    assert(*c == BitVec("0000101"));
    assert(BitVec::to_uint(c) == x);
    delete c;

}

void Test::reset_bitvec(BitVec *vec, string val) {
    for (int i = 0; i < (int) val.size(); i++) {
        (*vec)[i] = (val[i] - '0');
    }
}

void Test::to_code(Run *run) {
    InstrFactory *factory = InstrFactory::create(run);
    Tree tree;
    tree.add_child(-1, nullptr);
    tree.add_child(0, nullptr);
    tree.add_child(0, nullptr);
    tree.add_child(0, nullptr);

    BitVec *if_bits = new BitVec("01010000");
    BitVec *zero_bits = new BitVec("10000000");
    BitVec *one_bits = new BitVec("10000001");
    BitVec *two_bits = new BitVec("10000010");
    
    Instr *if_instr = factory->create_instr(if_bits);
    Instr *zero_instr = factory->create_instr(zero_bits);
    Instr *one_instr = factory->create_instr(one_bits);
    Instr *two_instr = factory->create_instr(two_bits);

    tree.set_instr(0, if_instr);
    tree.set_instr(1, one_instr);
    tree.set_instr(2, zero_instr);
    tree.set_instr(3, two_instr);

    assert(tree.height() == 2);
    assert(tree.to_str() != "\n");
    assert(tree.to_code() == "(if 1 0 2)\n");

    delete if_bits;
    delete zero_bits;
    delete one_bits;
    delete two_bits;
    
    //delete factory;
}

void Test::program(Run *run) {
    Phenotype *ptype = new Phenotype(run);
    InstrFactory *factory = InstrFactory::create(run);

    BitVec *x0_bits = new BitVec("10010000");
    Instr *x0_instr = factory->create_instr(x0_bits);
    BitVec *x1_bits = new BitVec("10010001");
    Instr *x1_instr = factory->create_instr(x1_bits);
    BitVec *mult_bits = new BitVec("00100000");
    Instr *mult_instr = factory->create_instr(mult_bits);

    ptype->add_child(-1, mult_instr);
    ptype->add_child(0, x0_instr);
    ptype->add_child(0, x1_instr);

    vector<Instr*> params;
    params.push_back(x0_instr);
    params.push_back(x1_instr);
    Program pgm(ptype);

    vector<string> args;
    args.push_back("2");
    args.push_back("2");
    string output = pgm.run(&params, &args);
    assert(output == "4\n");
    args.clear();
    
    args.push_back("2");
    args.push_back("3");
    output = pgm.run(&params, &args);
    assert(output == "6\n");
    args.clear();
    
    delete ptype;
    delete x0_bits;
    delete x1_bits;
    delete mult_bits;
    delete factory;
}

void Test::fitness(Run *run) {
    InstrFactory *factory = InstrFactory::create(run);
    Phenotype ptype(run);
    Instr *mult = factory->get_F_instr(MULT);
    Instr *x0 = factory->get_T_instr(VAR_CONST, 0);
    Instr *x1 = factory->get_T_instr(VAR_CONST, 1);
    ptype.add_child(0, mult);
    ptype.add_child(0, x0);
    ptype.add_child(0, x1);

    vector<Instr*> args;
    args.push_back(x0);
    args.push_back(x1);

    float fitness = FitnessFcn::eval(&ptype, &args);
    assert(fitness == 0.0f);
}

void Test::variability(Run *run) {
    const int num_trials = 100;
    
    vector<Grn*> pop;
    vector<Phenotype*> phenotypes;
    vector<float> fitnesses;
    
    Grn *grn = new Grn(run);
    pop.push_back(grn);
    Phenotype *ptype = new Phenotype(run);
    phenotypes.push_back(ptype);
    float fitness = 0.0f;
    fitnesses.push_back(fitness);

    //Logger logger(run);
    RegSim rs(run, nullptr);
    for (int i = 0; i < num_trials; i++) {
        rs.update_fitness(&pop, &fitnesses, &phenotypes, i);
        cout << "Proteins: " << grn->get_num_proteins() << endl;
        cout << "Fitness: " << fitnesses[0] << endl;
        cout << "Phenotype: " << endl;

        cout << ptype->to_str();
        
        cout << endl;
        
        grn->reset();
    }
    
    delete grn;
    delete ptype;
}
