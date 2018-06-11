#ifndef _LOGGER_HPP
#define _LOGGER_HPP

#include <sqlite3.h>
#include <string>
#include "runs.hpp"
#include "grn.hpp"

using namespace std;

class Logger {
public:
    Logger(Run *run);
    ~Logger();

    void log_run();
    void log_fitnesses(int ga_step, vector<Grn*> *pop, vector<float> *fitnesses);
    void log_ga_step(int ga_step, vector<Grn*> *grns);
    void log_reg_step(int ga_step, int reg_step, Grn *grn, int pop_index);
    void write_db();

    float get_best_fitness(int ga_step);
    float get_avg_fitness(int ga_step);

private:
    Run *run;
    sqlite3 *conn;

    void create_tables();
    float get_fitness_val(int ga_step, string *sql_fcn);
};

#endif
