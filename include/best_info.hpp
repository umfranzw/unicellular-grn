#ifndef _BEST_INFO_HPP
#define _BEST_INFO_HPP

#include "reg_snapshot.hpp"

typedef struct BestInfo {
    bool run_best_updated;
    int run_best_index;
    bool gen_best_updated;
    int gen_best_index;

    RegSnapshot *gen_best;
    RegSnapshot *run_best;
} BestInfo;

#endif
