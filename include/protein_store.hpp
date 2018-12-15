#ifndef _PROTEIN_STORE_HPP
#define _PROTEIN_STORE_HPP

#include "protein.hpp"
#include "store_iterator.hpp"
#include "bitvec.hpp"
#include <map>
#include <iterator>
#include <vector>

using namespace std;

class ProteinStore {
public:
    using iterator = StoreIterator;
    iterator begin() const;
    iterator end() const;
    
    ProteinStore();
    ProteinStore(ProteinStore *protein);
    ~ProteinStore();
    int add(Protein* protein);
    bool remove(int id);
    Protein *get(int id); //constant time operation
    //retreives all proteins with a given seq
    vector<Protein *> get_all(const BitVec *bv); //linear time operation
    pair<int, Protein*> get_by_seq(BitVec *seq);
    size_t size();
    vector<int> get_ids();
    string to_str();
    void reset();

private:
    map<int, Protein*> id_map;
    int next_id;
};

#endif
