#ifndef _BEST_INFO_HPP
#define _BEST_INFO_HPP

#include "reg_snapshot.hpp"

class BestInfo {
public:
    BestInfo();
    ~BestInfo();
    RegSnapshot *get_gen_best();
    void set_gen_best(RegSnapshot *gen_best, int index);
    RegSnapshot *get_run_best();
    void set_run_best(RegSnapshot *run_best, int index);
    void reset_updated_flags();

    bool is_gen_best_updated();
    bool is_run_best_updated();

private:
    RegSnapshot *gen_best;
    RegSnapshot *run_best;
    
    bool run_best_updated;
    int run_best_index;
    bool gen_best_updated;
    int gen_best_index;
};

#endif
