#include "protein_store.hpp"

#include <sstream>

ProteinStore::ProteinStore() {
    this->next_id = 0;
}

ProteinStore::ProteinStore(ProteinStore *store) { //copy constructor
    this->next_id = 0;
    
    for (const int& id : *store) {
        Protein *protein = store->get(id);
        Protein *copy = new Protein(protein);
        this->add(copy); //copy protein. Note: we always place proteins on the heap so we can fling pointers around with wild abandon
    }
}

ProteinStore::~ProteinStore() {
    vector<int> ids = this->get_ids();
    for (int id : ids) {
        Protein *protein = this->get(id);
        delete protein;
    }
}

void ProteinStore::reset() {
    vector<int> ids = this->get_ids();
    for (int id : ids) {
        Protein *protein = this->get(id);
        delete protein;
    }
    
    this->id_map.clear();
    this->next_id = 0;
}

int ProteinStore::add(Protein *protein) {
    int id = this->next_id;
    this->id_map.insert(make_pair(id, protein));
    this->next_id++;

    return id;
}

bool ProteinStore::remove(int id) {
    map<int, Protein*>::iterator item = this->id_map.find(id);
    int num_erased = 0;
    if (item != this->id_map.end()) {
        num_erased = this->id_map.erase(id);
        delete item->second;
    }
    
    return num_erased > 0;
}

Protein *ProteinStore::get(int id) {
    Protein *protein = nullptr;
    map<int, Protein*>::iterator result = this->id_map.find(id);
    if (result != this->id_map.end()) {
        protein = result->second;
    }

    return protein;
}

vector<Protein *>ProteinStore::get_all(const BitVec *bv) {
    vector<Protein *> proteins;

    for (auto it = this->id_map.begin(); it != this->id_map.end(); it++) {
        if (*(it->second->seq) == *bv) {
            proteins.push_back(it->second);
        }
    }

    return proteins;
}

pair<int, Protein *> ProteinStore::get_by_seq(BitVec *seq) {
    pair<int, Protein*> result;
    result.first = -1;
    result.second = nullptr;
    for (auto it = this->id_map.begin(); result.first == -1 && it != this->id_map.end(); it++) {
        if (*(it->second->seq) == *seq) {
            result.first = it->first;
            result.second = it->second;
        }
    }

    return result;
}

size_t ProteinStore::size() {
    return this->id_map.size();
}

vector<int> ProteinStore::get_ids() {
    vector<int> ids;
    for (pair<int, Protein*> entry : this->id_map) {
        ids.push_back(entry.first);
    }
    
    return ids;
}

string ProteinStore::to_str() {
    stringstream info;

    info << "--------------" << endl;
    info << "Protein Store:" << endl;
    info << "--------------" << endl;
    for (int id : this->get_ids()) {
        info << "Protein " << id << endl;
    }
    
    return info.str();
}

ProteinStore::iterator ProteinStore::begin() const {
    return StoreIterator(this->id_map.begin());
}

ProteinStore::iterator ProteinStore::end() const {
    return StoreIterator(this->id_map.end());
}
