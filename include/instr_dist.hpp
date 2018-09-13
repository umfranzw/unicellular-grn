#ifndef _INSTR_DIST_HPP
#define _INSTR_DIST_HPP

#include "prob_dist.hpp"
#include "runs.hpp"
#include <vector>

using namespace std;

class InstrDist {
public:
    InstrDist(Run *run);
    InstrDist(InstrDist *other);
    ~InstrDist();
    int sample();
    float get_weight(int index);
    void set_weight(int index, float val);
    void set_weights(vector<float> *weights);
    float get_prob(int index);
    void set_prob(int index, float val);
    void set_probs(vector<float> *probs);
    string to_str();

private:
    void update_prob_dist();
    
    Run *run;
    vector<float> *probs;
    vector<float> *weights;
    ProbDist *dist;
};

#endif
