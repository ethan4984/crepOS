#ifndef MAP_HPP_
#define MAP_HPP_

#include <vector.hpp>

namespace lib {

template <typename F, typename T>
class map {
public:
    map(const map &other);
    map(map &&other);
    map() = default;

    void swap(map &a, map &b);

    map &operator= (map other);
    T &operator[] (const F &index);

    size_t size() const { return _tags.size(); }

    void remove(const F &index);
    F get_tag(size_t i) { return _tags[i]; }
private:
    vector<F> _tags;
    vector<T> _data;
};

template <typename F, typename T>
map<F, T>::map(const map &other) {
    _tags = other._tags; 
    _data = other._data;
}

template <typename F, typename T>
map<F, T>::map(map &&other) {
    swap(*this, other);
}

template <typename F, typename T>
map<F, T> &map<F, T>::operator= (map other) {
    swap(*this, other);
    return *this;
}

template <typename F, typename T>
void map<F, T>::swap(map &a, map &b) {
    std::swap(a._tags, b._tags);
    std::swap(a._data, b._data);
}

template <typename F, typename T>
T &map<F, T>::operator[] (const F &index) {
    for(size_t i = 0; i < _tags.size(); i++) {
        if(_tags[i] == index)
            return _data[i];
    }

    _tags.push(index);

    return _data.push(T());
}

template <typename F, typename T>
void map<F, T>::remove(const F &index) {
    for(size_t i = 0; i < _tags.size(); i++) {
        if(_tags[i] == index) {
            _tags.remove(i);
            _data.remove(i);
        }
    }
}
    
}

#endif
