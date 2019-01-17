#ifndef _BEST_INFO_HPP
#define _BEST_INFO_HPP

template <class T>
class BestInfo {
private:
    T *gen_best;
    T *run_best;
    
    int run_best_index;
    float run_best_fitness;
    int gen_best_index;
    float gen_best_fitness;
    bool gen_over;
    
public:
    BestInfo() {
        this->run_best_index = -1;
        this->run_best_fitness = -1.0f;
        this->run_best = nullptr;
    
        this->gen_best_index = -1;
        this->gen_best_fitness = -1.0f;
        this->gen_best = nullptr;
        
        this->gen_over = false;
    }

    ~BestInfo() {
        if (this->run_best != nullptr) {
            delete this->run_best;
        }
    
        if (this->gen_best != nullptr && this->gen_best != this->run_best) {
            delete this->gen_best;
        }
    }
    
    T *get_gen_best() {
        return this->gen_best;
    }

    float get_gen_best_fitness() {
        return this->gen_best_fitness;
    }
    
    bool update_gen_best(T *gen_best, int index, float fitness) {
        bool updated = false;
        
        if (this->gen_best == nullptr || this->gen_over || fitness < this->gen_best_fitness) {
            if (this->gen_best != nullptr && this->gen_best != this->run_best) {
                delete this->gen_best;
            }
        
            this->gen_best = gen_best;
            this->gen_best_index = index;
            this->gen_best_fitness = fitness;
            updated = true;
            this->gen_over = false;
        }

        return updated;
    }
    
    T *get_run_best() {
        return this->run_best;
    }

    float get_run_best_fitness() {
        return this->run_best_fitness;
    }
    
    bool update_run_best(T *run_best, int index, float fitness) {
        bool updated = false;

        if (this->run_best == nullptr || fitness < this->run_best_fitness) {
            if (this->run_best != nullptr && this->run_best != this->gen_best) {
                delete this->run_best;
            }
            this->run_best = run_best;
            this->run_best_index = index;
            this->run_best_fitness = fitness;
            updated = true;
        }

        return updated;
    }

    void gen_done() {
        this->gen_over = true;
    }
};

#endif
