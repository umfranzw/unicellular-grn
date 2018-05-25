#include "store_iterator.hpp"

StoreIterator::StoreIterator(map<int, Protein*>::const_iterator map_iter) {
    this->map_iter = map_iter;
}

StoreIterator::reference StoreIterator::operator*() const {
    return (const int&) (this->map_iter->first);
}

StoreIterator& StoreIterator::operator++() {
    ++this->map_iter;
    return *this;
}

StoreIterator StoreIterator::operator++(int) {
    StoreIterator tmp = *this;
    ++this->map_iter;
    return tmp;
}

bool StoreIterator::operator==(const StoreIterator& rhs) {
    return this->map_iter == rhs.map_iter;
}

bool StoreIterator::operator!=(const StoreIterator& rhs) {
    return this->map_iter != rhs.map_iter;
}
