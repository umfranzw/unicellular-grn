#include "utils.hpp"

void Utils::fill_rand(boost::dynamic_bitset<> *set, int len, Run *run) {
    for (int i = 0; i < len; i++) {
        int bit = (int) run->rand.next_float() < 0.5;
        set->push_back(bit);
    }
}

void Utils::fill_rand(vector<float> *vec, int len, Run *run) {
    for (int i = 0; i < len; i++) {
        vec->push_back(run->rand.next_float());
    }
}

vector<float> Utils::zeros(int len) {
    vector<float> result;
    for (int i = 0; i < len; i++) {
        result.push_back(0.0f);
    }

    return result;
}

bool Utils::contains_id(vector<int> *vec, int id) {
    auto it = vec->begin();
    while (it != vec->end() && *it != id) {
        it++;
    }
    return it != vec->end();
}

int Utils::hamming_dist(boost::dynamic_bitset<> *x, boost::dynamic_bitset<> *y) {
    return (*x ^ *y).count();
}
