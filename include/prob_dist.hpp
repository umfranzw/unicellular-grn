#ifndef _PROB_DIST_HPP
#define _PROB_DIST_HPP

#include "runs.hpp"
#include <vector>
#include <string>

using namespace std;

class BucketRef;

class ProbDist {
public:
    ProbDist(Run *run, vector<float> buckets, bool normalize, bool invert);
    int sample();
    
    BucketRef operator[](size_t index);
    ProbDist& operator=(const ProbDist& other);
    string to_str();
    
private:
    void update_wheel();
        
    vector<float> buckets;
    vector<float> wheel;
    int dirty_index;
    Run *run;

    friend class BucketRef;
};

class BucketRef {
public:
    BucketRef(ProbDist *dist, int index);
    BucketRef& operator=(float val);
    BucketRef& operator=(const BucketRef& other);
    operator float() const;
    
private:
    ProbDist *dist;
    int index;
};

#endif
