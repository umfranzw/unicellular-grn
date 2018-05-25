#include "protein_store.hpp"

ProteinStore::ProteinStore() {
    this->next_id = 0;
}

ProteinStore::~ProteinStore() {
    // for (pair<int, Protein*> info : this->proteins) {
    //     delete info.second;
    // }
}

int ProteinStore::add(Protein *protein) {
    int id = this->next_id;
    this->proteins.insert(make_pair(id, protein));
    this->next_id++;

    return id;
}

bool ProteinStore::remove(int id) {
    // map<int, Protein*>::iterator item = this->proteins.find(id);
    // if (item != this->proteins.end()) {
    //     delete item->second;
    // }
    
    return this->proteins.erase(id) > 0;
}

Protein *ProteinStore::get(int id) {
    Protein *protein = nullptr;
    map<int, Protein*>::iterator result = this->proteins.find(id);
    if (result != this->proteins.end()) {
        protein = result->second;
    }

    return protein;
}

size_t ProteinStore::size() {
    return this->proteins.size();
}

bool ProteinStore::contains(int id) {
    return this->proteins.find(id) == this->proteins.end();
}

ProteinStore::iterator ProteinStore::begin() const {
    return StoreIterator(this->proteins.begin());
}

ProteinStore::iterator ProteinStore::end() const {
    return StoreIterator(this->proteins.end());
}
