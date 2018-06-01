#ifndef _UTILS_HPP
#define _UTILS_HPP

#include "runs.hpp"
#include "bitvec.hpp"

class Utils {
public:
    static void fill_rand(BitVec *set, int len, Run *run);
    static void fill_rand(vector<float> *vec, int len, Run *run);
    static vector<float> zeros(int len);
    static bool contains_id(vector<int> *vec, int id);
    static int hamming_dist(BitVec *x, BitVec *y);

    //clamps to closed interval [low, high]
    template<typename T>
    static T clamp(T val, T low, T high) {
        return min(max(val, low), high);
    }
};

#endif
