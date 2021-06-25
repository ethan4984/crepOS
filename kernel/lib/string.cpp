#include <string.hpp>
#include <memutils.hpp>
#include <mm/slab.hpp>

namespace lib {

string::string(const string &other) {
    _raw = new char[other._length + 1];
    memcpy8(reinterpret_cast<uint8_t*>(_raw), reinterpret_cast<uint8_t*>(other._raw), other._length);
    _length = other._length;
    _raw[_length] = '\0';
}

string::string(string &&other) {
    swap(*this, other);
}

string::string(const char *raw) : _length(0) {
    _length = strlen(raw);

    _raw = new char[_length + 1];

    for(size_t i = 0; i < _length; i++)
        _raw[i] = raw[i];

    _raw[_length] = '\0';
}

string::string(const char *raw, size_t length) : _length(length) {
    _raw = new char[_length];

    for(size_t i = 0; i < _length; i++)
        _raw[i] = raw[i];

    _raw[_length] = '\0';
}

string::~string() {
    delete _raw;
}

string &string::operator= (string other) { 
    swap(*this, other);
    return *this;
}

string &string::operator+= (const string &other) {
    *this = *this + other;
    return *this;
}

bool string::operator== (const string &a) {
    if(_length != a._length)
        return false;

    for(size_t i = 0; i < _length; i++) {
        if(_raw[i] != a._raw[i])
            return false;
    }

    return true;
}

bool string::operator!= (const string &a) {
    if(_length != a._length)
        return true;

    for(size_t i = 0; i < _length; i++) {
        if(_raw[i] != a._raw[i])
            return true;
    }

    return false; 
}

char &string::operator[] (size_t index) {
    return _raw[index];
}

char *string::operator++ () {
    _length--;
    return ++_raw;
}

char *string::operator++ (int) {
    _length--;
    return _raw++;
}

char *string::operator-- () {
    _length++;
    return --_raw;
}

char *string::operator-- (int) {
    _length++;
    return _raw--;
}

string string::operator+ (const string &other) {
    char *new_str = new char[other._length + _length];

    memcpy8(reinterpret_cast<uint8_t*>(new_str), reinterpret_cast<uint8_t*>(_raw), _length + 1);
    memcpy8(reinterpret_cast<uint8_t*>(new_str + _length), reinterpret_cast<uint8_t*>(other._raw), other._length);

    size_t new_length = _length + other._length;

    new_str[new_length] = '\0';

    return string(new_str, new_length);
}

string string::operator+ (ssize_t number) {
    static char digits[] = "0123456789";
    static char buffer[50];

    char *str = &buffer[49];
    *str = '\0';

    do {
        *--str = digits[number % 10];
        number /= 10;
    } while(number);

    return *this + string(str);
}

size_t string::find_first(char c, size_t start) { 
    for(size_t i = start; i < _length; i++) {
        if(_raw[i] == c) 
            return i;
    }
    return npos;
}

size_t string::find_last(char c) {
    for(size_t i = _length; i > 0; i--) {
        if(_raw[i] == c) 
            return i;
    }
    return npos;
}

string &string::erase(size_t pos, size_t size) {
    if(size == npos)
        size = _length - pos;

    char *tmp = new char[_length - size];

    size_t cnt = 0; 

    for(size_t i = 0; i < pos; i++) {
        tmp[cnt++] = _raw[i];
    }

    for(size_t i = pos + size; i < _length; i++) {
        tmp[cnt++] = _raw[i];
    }

    delete _raw;

    _raw = tmp;
    _length -= size;

    return *this;
}

string string::substr(size_t start, size_t size) {
    if(size == npos)
        size = _length - start;

    return string(_raw + start, size);
}

void string::swap(string &a, string &b) {
    std::swap(a._raw, b._raw);
    std::swap(a._length, b._length);
}

}
