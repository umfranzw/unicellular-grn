#ifndef UTILS_H
#define UTILS_H
#include <random>

using namespace std;

typedef mt19937 RandGen;

unsigned int make_lower_bitmask(int bits);
RandGen make_gen();

#endif
