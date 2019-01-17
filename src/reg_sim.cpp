#include <reg_sim.hpp>
#include "instr_dist.hpp"
#include "fitness_fcn.hpp"

RegSim::RegSim(Run* run, Logger* logger) {
    this->run = run;
    this->logger = logger;
    
    this->num_grow_samples = (this->run->growth_end - this->run->growth_start + 1) / this->run->growth_sample_interval;
    this->num_code_samples = (this->run->code_end - this->run->code_start + 1) / this->run->code_sample_interval;
    this->instr_factory = InstrFactory::create(this->run);
}

RegSim::~RegSim() {
    delete this->instr_factory;
}

void RegSim::update_fitness(vector<Grn*> *pop, vector<float> *fitnesses, vector<Phenotype*> *phenotypes, int ga_step) {
    //do regulatory simulation
    bool run_best_updated = false;
    
    for (int i = 0; i < this->run->pop_size; i++) {
        (*phenotypes)[i]->reset();
        Grn *grn = (*pop)[i];
        Phenotype *ptype = (*phenotypes)[i];

        RegSnapshot *snappy = new RegSnapshot(grn, ga_step, i);
        snappy->add_reg_step(-1, ptype);
        
        //this->logger->log_reg_step(ga_step, -1, grn, i, ptype);

        for (int j = 0; j < this->run->reg_steps; j++) {
            grn->run_binding(i, j, ga_step);

            grn->update_output_proteins();

            grn->run_diffusion();

            this->grow_step(grn, ptype, i, j, ga_step);

            this->code_step(grn, ptype, i, j, ga_step);

            //if we're not in the coding phase, do the decay
            if (j < this->run->code_start || j > this->run->code_end) {
                grn->run_decay();
            }
            
            //this->logger->log_reg_step(ga_step, j, grn, i, ptype);
            snappy->add_reg_step(j, ptype);
        }
        //update fitness value
        float fitness = FitnessFcn::eval((*phenotypes)[i], this->instr_factory->get_vars());
        (*fitnesses)[i] = fitness;

        if (this->run->log_mode == "all") {
            this->logger->log_reg_snapshot(snappy);
        }

        bool gen_best_updated = this->bests.update_gen_best(snappy, i, fitness);
        if (gen_best_updated) {
            run_best_updated = run_best_updated || this->bests.update_run_best(snappy, i, fitness);
        }
        
        else {
            delete snappy;
        }
        
        this->bests.gen_done();
    }

    if (this->run->log_mode == "best" && run_best_updated) {
        this->logger->log_reg_snapshot(this->bests.get_run_best());
    }
}

void RegSim::grow_step(Grn *grn, Phenotype *ptype, int grn_index, int reg_step, int ga_step) {
    bool growing = reg_step >= this->run->growth_start && reg_step <= this->run->growth_end;
    bool sampling = (reg_step - this->run->growth_start) % this->run->growth_sample_interval == 0;
    
    if (growing && sampling) {
        const static BitVec growth_seq(this->run->growth_seq);
        vector<Protein*> proteins = grn->proteins->get_all((const BitVec*) &growth_seq);

        map<int, bool> grow_indices;
        for (Protein *p : proteins) {
            for (int i = 0; i < this->run->num_genes; i++) {
                if (p->concs[i] > this->run->growth_threshold) {
                    grow_indices.insert(pair<int, bool>(i, true));
                }
            }
        }
        
        //note: map is sorted by keys by default
        for (auto it = grow_indices.begin(); it != grow_indices.end(); it++) {
            int grow_index = it->first;
            ptype->add_child(grow_index, nullptr); //leave instruction null for now (will be filled in in code_step() later)
        }
    }
}

void RegSim::code_step(Grn *grn, Phenotype *ptype, int grn_index, int reg_step, int ga_step) {
    bool coding = reg_step >= this->run->code_start && reg_step <= this->run->code_end;
    bool sampling = (reg_step - this->run->code_start) % this->run->code_sample_interval == 0;

    if (coding && sampling) {
        int code_step = (reg_step - this->run->code_start) / this->run->code_sample_interval;
        int samples_left = this->num_code_samples - code_step;

        if (samples_left > 0 && ptype->get_num_unfilled_nodes() > 0) {
            int instr_per_sample = (ptype->get_num_unfilled_nodes() / samples_left) || ptype->get_num_unfilled_nodes();
            vector<int> pids = grn->proteins->get_ids();
            int start = ptype->get_num_filled_nodes();
            int end = start + instr_per_sample;
            for (int i = start; i < end; i++) { //for each phenotype node
                int gene_index = i % this->run->num_genes;
                map<BitVec*, float, bool(*)(BitVec*, BitVec*)> buckets(BitVec::compare); //holds all the concs over position gene_index whose protein sequence maps to an instr with the correct number of args
                //for each protein in the grn, grab the protein in the pos given by
                //the phenotype node index and insert it into the bucket
                int num_args = ptype->get_num_children(i);
                for (int j = 0; j < (int) pids.size(); j++) {
                    Protein *p = grn->proteins->get(pids[j]);
                    //filter down to only those instructions with a reasonable number of args
                    InstrInfo info = this->instr_factory->seq_to_instr_info(p->seq);
                    if (num_args >= info.min_args && (num_args <= info.max_args || info.max_args == UNLIMITED_ARGS)) {
                        //check if we already have a protein with that seq
                        if (buckets.find(p->seq) != buckets.end()) {
                            buckets[p->seq] = max(buckets[p->seq], p->concs[gene_index]); //take the one with the higher conc
                        }
                        else {
                            buckets[p->seq] = p->concs[gene_index];
                        }
                    }
                }
                //check if we have any instructions that accept the correct number of arguments (and that we have more than 0 proteins)
                if (buckets.size() > 0) {
                    InstrDist dist(this->run, this->instr_factory, &buckets);
                    Instr *instr = dist.sample(); //note: instr may be nullptr
                    if (instr != nullptr) {
                        ptype->set_instr(i, instr);
                    }
                }
                //else if no proteins or no instructions with correct number of args,
                //do nothing. We'll recalc the number of instructions per sample on the
                //next sample iteration to compensate for not setting this instruction.
            
                //But, if this is the last sample and we have no proteins or no instructions
                //with the correct number of args, then the phenotype is left incomplete. The fitness function will deal with this.
            }
        }
    }
}
