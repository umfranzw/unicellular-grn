#ifndef _STORE_ITERATOR_HPP
#define _STORE_ITERATOR_HPP

#include "protein.hpp"
#include <map>
#include <iterator>

using namespace std;

class StoreIterator {
public:
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = int*;
    using reference = const int&;
    using iterator_category = std::forward_iterator_tag;

    StoreIterator() = default;
    explicit StoreIterator(map<int, Protein*>::const_iterator);

    reference operator*() const;

    StoreIterator& operator++();
    StoreIterator operator++(int);

    bool operator==(const StoreIterator& rhs);
    bool operator!=(const StoreIterator& rhs);

private:
    map<int, Protein*>::const_iterator map_iter;
};

#endif
