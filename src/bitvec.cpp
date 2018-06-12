#include "bitvec.hpp"

#include <sstream>
#include <iostream>

BitVec::BitVec(int len) {
    this->len = len;

    int num_blocks = len / 8 + (len % 8 ? 1 : 0);
    for (int i = 0; i < num_blocks; i++) {
        this->blocks.push_back((unsigned char) 0);
    }
}

BitVec::BitVec(string val) {
    unsigned char cur = (unsigned char) 0;
    int i = 0;
    while (i < (int) val.size()) {
        if (val[i] == '1') {
            cur |= (0x01 << (8 - (i % 8) - 1));
        }
        i++;
        
        if (i % 8 == 0) {
            this->blocks.push_back(cur);
            cur = (unsigned char) 0;
        }
    }

    //if we didn't end on an iteration that pushed cur into blocks, do a final push
    if ((val.size() % 8) > 0) {
        this->blocks.push_back(cur);
    }

    this->len = val.size();
}

BitVec::BitVec(vector<unsigned char> *blocks, int len) {
    this->len = len;
    
    int num_blocks = len / 8 + (len % 8 ? 1 : 0);
    for (int i = 0; i < num_blocks; i++) {
        this->blocks.push_back((*blocks)[i]);
    }

    this->zero_extra_bits();
}

BitVec::BitVec(const BitVec& bv) {
    this->len = bv.len;
    this->blocks = bv.blocks;
}

unsigned char BitVec::make_block_mask(int start, int end) {
    unsigned char mask = (unsigned char) 0;

    for (int i = start; i < end; i++) {
        mask |= (0x01 << (8 - i - 1));
    }

    return mask;
}

int BitVec::size() {
    return this->len;
}

string BitVec::to_str() {
    stringstream info;

    int block;
    int bit;
    unsigned char val;
    for (int i = 0; i < this->len; i++) {
        block = i / 8;
        bit = i % 8;
        val = (this->blocks[block] >> (8 - bit - 1)) & 0x01;

        //add spaces every 4 bits for readability
        if (i > 0 && !(i % 4)) {
            info << " ";
        }

        info << (char) (val + '0');
    }
    
    return info.str();
}

//counts number of bits set to 1
int BitVec::count() {
    int count = 0;
    for (int i = 0; i < this->len; i++) {
        count += (int) (*this)[i];
    }

    return count;
}

//zeros out any unneeded bits in the last block
void BitVec::zero_extra_bits() {
    int overflow_bits = this->len % 8;
    if (overflow_bits) { //if true then the last block is a partial
        unsigned char mask = ~this->make_block_mask(overflow_bits, 8);
        this->blocks.back() &= mask;
    }
}

BitVec BitVec::operator&=(const BitVec& rhs) {
    if (this->len != rhs.len) {
        cerr << "Attempt to &= two bitvecs of different lengths." << endl;
    }

    vector<unsigned char> new_blocks;
    for (int i = 0; i < (int) this->blocks.size(); i++) {
        this->blocks[i] &= rhs.blocks[i];
    }

    return *this;
}

BitVec BitVec::operator|=(const BitVec& rhs) {
    if (this->len != rhs.len) {
        cerr << "Attempt to |= two bitvecs of different lengths." << endl;
    }

    vector<unsigned char> new_blocks;
    for (int i = 0; i < (int) this->blocks.size(); i++) {
        this->blocks[i] |= rhs.blocks[i];
    }

    return *this;
}

BitVec BitVec::operator^=(const BitVec& rhs) {
    if (this->len != rhs.len) {
        cerr << "Attempt to ^= two bitvecs of different lengths." << endl;
    }

    vector<unsigned char> new_blocks;
    for (int i = 0; i < (int) this->blocks.size(); i++) {
        this->blocks[i] ^= rhs.blocks[i];
    }

    return *this;
}

BitVec BitVec::operator&(const BitVec& rhs) {
    BitVec result = BitVec(*this);
    result &= rhs;

    return result;
}

BitVec BitVec::operator|(const BitVec& rhs) {
    BitVec result = BitVec(*this);
    result |= rhs;

    return result;
}

BitVec BitVec::operator^(const BitVec& rhs) {
    BitVec result = BitVec(*this);
    result ^= rhs;

    return result;
}

BitVec BitVec::operator~() {
    vector<unsigned char> new_blocks;
    for (int i = 0; i < (int) this->blocks.size(); i++) {
        new_blocks.push_back(~this->blocks[i]);
    }

    BitVec result(&new_blocks, this->len);
    result.zero_extra_bits();

    return result;
}

BitVec BitVec::operator>>=(const int dist) {
    int shift_blocks = dist / 8;
    int shift_offset = dist % 8;

    vector<unsigned char> new_blocks;
    for (int i = 0; i < shift_blocks; i++) {
        new_blocks.push_back((unsigned char) 0);
    }

    unsigned char carry = (unsigned char) 0;
    unsigned char new_block;
    for (int i = shift_blocks; i < (int) this->blocks.size(); i++) {
        new_block = this->blocks[i] >> shift_offset;
        new_block |= carry;
        new_blocks.push_back(new_block);
        
        carry = this->blocks[i] << (8 - shift_offset);
    }

    this->blocks = new_blocks;
    this->zero_extra_bits();

    return *this;
}

BitVec BitVec::operator<<=(const int dist) {
    int shift_blocks = dist / 8;
    int shift_offset = dist % 8;

    vector<unsigned char> new_blocks;
    for (int i = 0; i < shift_blocks; i++) {
        new_blocks.push_back((unsigned char) 0);
    }

    unsigned char carry = (unsigned char) 0;
    unsigned char new_block;
    for (int i = (int) this->blocks.size() - 1; i >= shift_blocks; i--) {
        new_block = this->blocks[i] << shift_offset;
        new_block |= carry;
        new_blocks.insert(new_blocks.begin(), new_block);

        carry = this->blocks[i] >> (8 - shift_offset);
    }

    this->blocks = new_blocks;
    //note: no need to zero extra bits here
    
    return *this;
}

BitVec BitVec::operator>>(const int dist) {
    BitVec result = BitVec(*this);
    result >>= dist;

    return result;
}

BitVec BitVec::operator<<(const int dist) {
    BitVec result = BitVec(*this);
    result <<= dist;

    return result;
}

bool BitVec::operator[](size_t pos) const {
    int block_index = pos / 8;
    int bit_index = pos % 8;
    unsigned char block = this->blocks[block_index];
    block >>= (8 - bit_index - 1);
    block &= 0x01;

    return block == (unsigned char) 1;
}

// constexpr const bool BitVec::operator[] (size_t pos) const {
//     return (*this)[pos];
// }

BitVec& BitVec::operator=(const BitVec& other) {
    if (this != &other) {
        this->blocks = other.blocks;
        this->len = other.len;
    }

    return *this;
}

BitRef BitVec::operator[](size_t pos) {
    return BitRef(this, pos);
}

bool BitVec::operator==(const BitVec& rhs) {
    //note: if all is well we should have zeroed out any extra bits, so we don't need to worry about them here...
    return this->blocks == rhs.blocks;
}

bool BitVec::operator!=(const BitVec& rhs) {
    return !(*this == rhs);
}

//------------------
BitRef::BitRef(BitVec *bv, size_t pos) {
    this->bv = bv;
    this->pos = pos;
}

BitRef& BitRef::operator=(bool val) {
    size_t block_index = pos / 8;
    size_t bit_index = pos % 8;
    unsigned char mask = bv->make_block_mask(bit_index, bit_index + 1);
    if (val) {
        this->bv->blocks[block_index] |= mask;
    }
    else {
        this->bv->blocks[block_index] &= ~mask;
    }

    return *this;
}

BitRef::operator bool() const {
    int block_index = this->pos / 8;
    int bit_index = this->pos % 8;
    unsigned char block = this->bv->blocks[block_index];
    block >>= (8 - bit_index - 1);
    block &= 0x01;

    return block == (unsigned char) 1;
}

BitRef& BitRef::operator=(const BitRef& other) {
    bool val = static_cast<bool>(other); //use operator bool, defined above
    return this->operator=(val); //use the other operator=(bool val), also defined above
}

bool BitRef::operator~() const {
    bool val = this->bv->operator[](this->pos);
    return !val;
}
