#ifndef _PROTEIN_STORE_HPP
#define _PROTEIN_STORE_HPP

#include "protein.hpp"
#include "store_iterator.hpp"
#include <map>
#include <iterator>

using namespace std;

class ProteinStore {
public:
    using iterator = StoreIterator;
    iterator begin() const;
    iterator end() const;
    
    ProteinStore();
    ~ProteinStore();
    int add(Protein* protein);
    bool remove(int id);
    Protein *get(int id);
    size_t size();
    bool contains(int id);
    vector<int> get_ids();

private:
    map<int, Protein*> proteins;

    int next_id;
};

#endif
