#ifndef _LOGGER_HPP
#define _LOGGER_HPP

#include <sqlite3.h>
#include <string>
#include "runs.hpp"
#include "grn.hpp"
#include "phenotype.hpp"
#include "reg_snapshot.hpp"
#include "best_info.hpp"

using namespace std;

class Logger {
public:
    Logger(Run *run);
    ~Logger();

    void log_run();
    void log_fitnesses(int ga_step, vector<Grn*> *pop, vector<Phenotype*> *phenotypes, vector<float> *fitnesses);
    void log_ga_step(int ga_step, vector<Grn*> *grns, BestInfo<RegSnapshot*> *bests);
    void log_split_pt(int ga_step, int pop_index, int split_pt);
    void log_reg_snapshot(RegSnapshot *snappy);
    void log_reg_step(int ga_step, int reg_step, Grn *grn, int pop_index, Phenotype *ptype);
    void write_db();

    float get_run_best_fitness();
    float get_gen_avg_fitness();
    void print_results(int total_iters);
    sqlite3 *conn;

private:
    bool should_sample(int ga_step);
    void log_ga_step(int ga_step, vector<Grn*> *grns, int pop_index_offset);
    Run *run;

    float gen_avg_fitness;
    BestInfo<Grn*> grn_bests;
    BestInfo<Phenotype*> ptype_bests;

    void create_tables();
    float get_fitness_val(int ga_step, string *sql_fcn);
    void insert_node(Tree *tree, int tree_id, Node *cur, int parent_id);
};

#endif
