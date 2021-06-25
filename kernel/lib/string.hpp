#ifndef STRING_HPP_
#define STRING_HPP_

#include <types.hpp>
#include <utility>

namespace lib {

class string {
public:
    string(const string &other);
    string(string &&other);
    string(const char *raw);
    string(const char *raw, size_t length);
    string() : _length(0), _raw(NULL) { }
    ~string();

    string &operator= (string other);
    string &operator+= (const string &other);
    string operator+ (const string &other);
    string operator+ (ssize_t number);
    char &operator[] (size_t index);
    char *operator++ ();
    char *operator++ (int);
    char *operator-- ();
    char *operator-- (int);
    bool operator== (const string &other);
    bool operator!= (const string &other);

    void swap(string &a, string &b);

    string substr(size_t start, size_t size);
    string &erase(size_t pos, size_t len); 
    size_t find_first(char c, size_t start = 0);
    size_t find_last(char c);

    size_t length() const { return _length; }
    char *data() const { return _raw; }

    static constexpr size_t npos = ~(0ull);
private:
    size_t _length;
    char *_raw;
};

}

#endif
