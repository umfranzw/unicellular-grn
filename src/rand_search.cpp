#include "rand_search.hpp"
#include "fitness_fcn.hpp"

RandSearch::RandSearch(Run *run) {
    this->run = run;
    this->instr_factory = InstrFactory::create(this->run);
}

RandSearch::~RandSearch() {
    for (int i = 0; i < this->pop.size(); i++) {
        delete this->pop[i];
    }
    
    delete this->instr_factory;
}

void RandSearch::run_alg() {
    for (int i = 0; i < this->run->pop_size; i++) {
        this->pop.push_back(new Phenotype(this->run));
        this->fitnesses.push_back(0.0f);
    }

    Phenotype *ptype;
    float gen_avg_fitness;
    
    for (int i = 0; i < this->run->ga_steps; i++) {
        cout << "************" << endl;
        cout << "Iteration " << i + 1 << endl;
        cout << "************" << endl;
        ptype = this->pop[i];
        ptype->reset();
        gen_avg_fitness = 0.0f;
        
        for (int j = 0; j < this->run->pop_size; j++) {
            this->randomize_ptype(ptype);
            this->fitnesses[j] = FitnessFcn::eval(ptype, this->instr_factory->get_vars());
            gen_avg_fitness += this->fitnesses[j];

            Phenotype *copy = new Phenotype(ptype);
            bool gen_best_updated = this->bests.update_gen_best(copy, j, this->fitnesses[j]);
            if (gen_best_updated) {
                this->bests.update_run_best(copy, j, this->fitnesses[j]);
            }
        }

        gen_avg_fitness /= this->run->pop_size;

        cout << "Gen. Avg. Fitness: " << gen_avg_fitness << endl;
        cout << "Gen. Best Fitness: " << this->bests.get_gen_best_fitness() << endl;
        cout << "Run Best Fitness: " << this->bests.get_run_best_fitness() << endl;
        cout << "Run Best Phenotype: " << endl;
        if (this->bests.get_run_best() != nullptr) {
            cout << this->bests.get_run_best()->to_code();
        }
        cout << endl;

        this->bests.gen_done();
    }
}

void RandSearch::randomize_ptype(Phenotype *ptype) {
    int size = this->run->rand->in_range(0, MAX_NODES + 1);
    for (int i = 0; i < size; i++) {
        ptype->add_child(this->run->rand->in_range(0, MAX_NODES), nullptr);
    }

    for (int i = 0; i < ptype->size(); i++) {
        int num_children = ptype->get_num_children(i);
        Instr *instr = this->instr_factory->get_rand_instr(num_children);
        if (instr != nullptr) {
            ptype->set_instr(i, instr);
        }
    }
}
