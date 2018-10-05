#include "test.hpp"
#include "phenotype.hpp"
#include "instr_factory.hpp"
#include "tree.hpp"
#include "bitvec.hpp"
#include "program.hpp"

Test::Test() {
}

Test::~Test() {
}

void Test::run() {
    Run *run = this->create_run();
    //to_code(run);
    fitness(run);
    delete run;
}

Run *Test::create_run() {
    Run *run = new Run();
    run->pop_size = 50;
    run->ga_steps = 1;
    run->reg_steps = 50;
    run->mut_prob = 0.1;
    run->mut_prob_limit = 0.2;
    run->mut_step = 0.0;
    run->cross_frac = 0.4;
    run->cross_frac_limit = 0.4;
    run->cross_step = 0.0;
    run->num_genes = 8;
    run->gene_bits = 8;
    run->min_protein_conc = 0.05;
    run->max_protein_conc = 1.0;
    run->alpha = 0.4;
    run->beta = 0.6;
    run->decay_rate = 0.05;
    run->initial_proteins = 5;
    run->max_proteins = 20;
    run->max_mut_float = 0.5;
    run->max_mut_bits = 8;
    run->fitness_log_interval = 10;
    run->binding_method = BINDING_SCALED;
    run->graph_results = false;
    run->log_grns = false;
    run->log_reg_steps = false;
    run->log_code_with_fitness = false;
    run->growth_start = 10;
    run->growth_end = 29;
    run->growth_sample_interval = 2;
    run->growth_seq = "11111111";
    run->growth_threshold = 0.5;
    run->term_cutoff = 0.0;
    run->code_start = 30;
    run->code_end = 49;
    run->code_sample_interval = 2;
    run->file_index = 0;

    return run;
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
    
    delete factory;
}

void Test::fitness(Run *run) {
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
