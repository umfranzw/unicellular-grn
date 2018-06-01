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
    
    this->proteins.clear();
    this->next_id = 0;
}

int ProteinStore::add(Protein *protein) {
    int id = this->next_id;
    this->proteins.insert(make_pair(id, protein));
    this->next_id++;

    return id;
}

bool ProteinStore::remove(int id) {
    map<int, Protein*>::iterator item = this->proteins.find(id);
    int num_erased = 0;
    if (item != this->proteins.end()) {
        num_erased = this->proteins.erase(id);
        delete item->second;
    }
    
    return num_erased > 0;
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

vector<int> ProteinStore::get_ids() {
    vector<int> ids;
    for (pair<int, Protein*> entry : this->proteins) {
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
    return StoreIterator(this->proteins.begin());
}

ProteinStore::iterator ProteinStore::end() const {
    return StoreIterator(this->proteins.end());
}
