#ifndef _UTILS_HPP
#define _UTILS_HPP

#include <boost/dynamic_bitset.hpp>
#include "runs.hpp"

class Utils {
public:
    static void fill_rand(boost::dynamic_bitset<> *set, int len, Run *run);
    static void fill_rand(vector<float> *vec, int len, Run *run);
    static vector<float> zeros(int len);
    static bool contains_id(vector<int> *vec, int id);
    static int hamming_dist(boost::dynamic_bitset<> *x, boost::dynamic_bitset<> *y);
};

#endif
