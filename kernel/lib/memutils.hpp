#ifndef MEMUTILS_HPP_
#define MEMUTILS_HPP_

#include <types.hpp>
#include <debug.hpp>

inline ssize_t div_roundup(ssize_t a, ssize_t b) {
    return (a + (b - 1)) / (b);
}

inline ssize_t align_up(ssize_t a, ssize_t b) {
    return div_roundup(a, b) * b; 
}

inline size_t abs(size_t a, size_t b) { 
    return a > b ? a - b : b - a;
}

inline size_t log2(size_t a) {
    size_t cnt = 0; 
    while(a /= 2)
        cnt++;
    return cnt;
}

inline size_t pow2_roundup(size_t a) {
    a--;
    a |= a >> 1;
    a |= a >> 2;
    a |= a >> 4;
    a |= a >> 8;
    a |= a >> 16;
    a++;
    return a;
}

template <typename T>
inline void bm_set(T *bitmap, size_t location) {
    bitmap[location / (sizeof(T) * 8)] |= (1 << (location % (sizeof(T) * 8)));
}

template <typename T>
inline void bm_clear(T *bitmap, size_t location) {
    bitmap[location / (sizeof(T) * 8)] &= ~(1 << (location % (sizeof(T) * 8)));
}

template <typename T>
inline bool bm_test(T *bitmap, size_t location) {
    return bitmap[location / (sizeof(T) * 8)] >> (location % (sizeof(T) * 8)) & 0x1;
}

template <typename T>
inline ssize_t bm_first_free(T *bitmap, size_t size) {
    for(size_t i = 0; i < size; i++)
        if(!bm_test(bitmap, i))
            return i;
    return -1;
}

template <typename T>
void bm_alloc_region(T *bitmap, size_t start, size_t limit) {
    for(size_t i = start; i < start + limit; i++) {
        bm_set(bitmap, i);
    }
}

template <typename T>
void bm_free_region(T *bitmap, size_t start, size_t limit) {
    for(size_t i = start; i < start + limit; i++) {
        bm_clear(bitmap, i);
    }    
}

template <typename T, size_t size>
constexpr size_t lengthof(T(&)[size]) { 
    return size;
}

inline constexpr ssize_t pow(ssize_t base, ssize_t exp) {
    ssize_t result = 1;

    for(;;) { 
        if(exp & 1)
            result *= base;

        exp >>= 1;

        if(exp <= 0)
            break;

        base *= base;
    }

    return result;
}

template <typename T>
class unique_ptr {
public:
    unique_ptr(const unique_ptr &other) { _data = other._data; }
    unique_ptr(unique_ptr &&other) { swap(*this, other); } 
    unique_ptr(T *_data) : _data(_data) { }
    unique_ptr() = default;
    ~unique_ptr() { delete _data; }

    T &operator[] (size_t index) const { return _data[index]; }

    unique_ptr &operator= (unique_ptr other) { swap(*this, other); return *this; }

    bool operator== (unique_ptr other) const { return other._data == _data; }
    bool operator!= (unique_ptr other) const { return other._data != _data; }
    bool operator< (unique_ptr other) const { return other._data < _data; }
    bool operator> (unique_ptr other) const { return other._data > _data; }
    bool operator<= (unique_ptr other) const { return other._data <= _data; }
    bool operator>= (unique_ptr other) const { return other._data >= _data; } 

    T &operator*() const { return *_data; }
    T *operator->() const { return _data; }
private:
    T *_data;

    void swap(unique_ptr &a, unique_ptr &b) { std::swap(a._data, b._data); }
};

void memset8(uint8_t *src, uint8_t data, size_t count);
void memset16(uint16_t *src, uint16_t data, size_t count);
void memset32(uint32_t *src, uint32_t data, size_t count);
void memset64(uint64_t *src, uint64_t data, size_t count);

void memcpy8(uint8_t *dest, uint8_t *src, size_t count);
void memcpy16(uint16_t *dest, uint16_t *src, size_t count);
void memcpy32(uint32_t *dest, uint32_t *src, size_t count);
void memcpy64(uint64_t *dest, uint64_t *src, size_t count);

extern "C" void memcpy(void *dest, void *src, size_t count);

size_t strlen(const char *str);
ssize_t strcmp(const char *str0, const char *str1);
ssize_t strncmp(const char *str0, const char *str1, size_t n);

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

#endif
