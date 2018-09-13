#ifndef _BITVEC_HPP
#define _BITVEC_HPP

#include <vector>
#include <string>

using namespace std;

class BitRef;

class BitVec {
public:
    BitVec(int size);
    BitVec(vector<unsigned char> *blocks, int len);
    BitVec(string val);
    BitVec(const BitVec& bv);
    
    int size();
    int count();
    string to_str();
    static unsigned int to_uint(BitVec *bv);
    static BitVec *from_uint(unsigned int val, int size);

    BitVec operator&=(const BitVec& rhs);
    BitVec operator|=(const BitVec& rhs);
    BitVec operator^=(const BitVec& rhs);
    BitVec operator&(const BitVec& rhs);
    BitVec operator|(const BitVec& rhs);
    BitVec operator^(const BitVec& rhs);
    BitVec operator~();
    BitVec operator>>=(const int dist);
    BitVec operator<<=(const int dist);
    BitVec operator>>(const int dist);
    BitVec operator<<(const int dist);

    bool operator[](size_t pos) const;
    //constexpr const bool operator[](size_t pos) const;
    BitRef operator[](size_t pos);

    BitVec& operator=(const BitVec& other);

    bool operator==(const BitVec& rhs);
    bool operator!=(const BitVec& rhs);
    
private:
    vector<unsigned char> blocks;
    int len;

    unsigned char make_block_mask(int start, int end); //sets bits in positions [start, end) to val
    void zero_extra_bits();

    friend class BitRef;
};

class BitRef {
    public:
        BitRef(BitVec *bv, size_t pos);
        BitRef& operator=(bool val);
        BitRef& operator=(const BitRef& other);

        operator bool() const;
        bool operator~() const;
        
    private:
        BitVec *bv;
        size_t pos;
    };

#endif
