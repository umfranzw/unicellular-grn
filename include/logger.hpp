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
    void log_ga_step(int ga_step, vector<Grn*> *grns, BestInfo<RegSnapshot> *bests);
    void log_reg_snapshot(RegSnapshot *snappy);
    void log_reg_step(int ga_step, int reg_step, Grn *grn, int pop_index, Phenotype *ptype);
    void write_db();

    float get_best_fitness(int ga_step);
    float get_avg_fitness(int ga_step);
    float get_run_best_fitness(); //so far

    void print_results(int total_iters);
    sqlite3 *conn;

private:
    bool should_sample(int ga_step);
    void log_ga_step(int ga_step, vector<Grn*> *grns, int pop_index_offset);
    Run *run;
    Grn *run_best_grn;
    Phenotype *run_best_ptype;
    float run_best_fitness;
    int run_best_pop_index;

    void create_tables();
    float get_fitness_val(int ga_step, string *sql_fcn);
    void insert_node(Tree *tree, int tree_id, Node *cur, int parent_id);
};

#endif
