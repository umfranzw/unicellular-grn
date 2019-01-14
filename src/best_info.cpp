#include "best_info.hpp"

BestInfo::BestInfo() {
    this->run_best_updated = false;
    this->run_best_index = -1;
    this->run_best = nullptr;
    
    this->gen_best_updated = false;
    this->gen_best_index = -1;
    this->gen_best = nullptr;
}

BestInfo::~BestInfo() {
    if (this->run_best != nullptr) {
        delete this->run_best;
    }
    
    if (this->gen_best != nullptr && this->gen_best != this->run_best) {
        delete this->gen_best;
    }
}

RegSnapshot *BestInfo::get_gen_best() {
    return this->gen_best;
}

void BestInfo::set_gen_best(RegSnapshot *gen_best, int index) {
    if (this->gen_best != nullptr && this->gen_best != this->run_best) {
        delete this->gen_best;
    }
    this->gen_best = gen_best;
    this->gen_best_index = index;
    this->gen_best_updated = true;
}

RegSnapshot *BestInfo::get_run_best() {
    return this->run_best;
}

void BestInfo::set_run_best(RegSnapshot *run_best, int index) {
    if (this->run_best != nullptr && this->run_best != this->gen_best) {
        delete this->run_best;
    }
    this->run_best = run_best;
    this->run_best_index = index;
    this->run_best_updated = true;
}

void BestInfo::reset_updated_flags() {
    this->gen_best_updated = false;
    this->run_best_updated = false;
}

bool BestInfo::is_gen_best_updated() {
    return this->gen_best_updated;
}

bool BestInfo::is_run_best_updated() {
    return this->run_best_updated;
}
