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
    void log_fitnesses(int ga_step, vector<float> *fitnesses);
    void log_ga_step(int ga_step, vector<Grn> *grns);
    void log_reg_step(int ga_step, int reg_step, Grn *grn, int pop_index);
    void write_db();

private:
    sqlite3 *db;
    Run *run;

    void create_tables();
};

#endif
