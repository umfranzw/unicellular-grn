#include "utils.hpp"

void Utils::fill_rand(BitVec *set, int len, Run *run) {
    for (int i = 0; i < len; i++) {
        bool bit = (bool) (run->rand->next_float() < 0.5);
        (*set)[i] = bit;
    }
}

void Utils::fill_rand(vector<float> *vec, int len, Run *run) {
    for (int i = 0; i < len; i++) {
        vec->push_back(run->rand->next_float());
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

int Utils::hamming_dist(BitVec *x, BitVec *y) {
    return (*x ^ *y).count();
}
