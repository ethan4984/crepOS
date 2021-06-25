#include <memutils.hpp>

void memset8(uint8_t *src, uint8_t data, size_t count) {
    for(size_t i = 0; i < count; i++)
        *src++ = data;
}

void memset16(uint16_t *src, uint16_t data, size_t count) {
    for(size_t i = 0; i < count; i++)
        *src++ = data;
}

void memset32(uint32_t *src, uint32_t data, size_t count) {
    for(size_t i = 0; i < count; i++)
        *src++ = data;
}

void memset64(uint64_t *src, uint64_t data, size_t count) {
    for(size_t i = 0; i < count; i++) {
        *src++ = data;
    }
}

void memcpy8(uint8_t *dest, uint8_t *src, size_t count) {
    for(size_t i = 0; i < count; i++) {
        *dest++ = *src++;
    }
}

void memcpy16(uint16_t *dest, uint16_t *src, size_t count) {
    for(size_t i = 0; i < count; i++) {
        *dest++ = *src++;
    }
}

void memcpy32(uint32_t *dest, uint32_t *src, size_t count) {
    for(size_t i = 0; i < count; i++) {
        *dest++ = *src++;
    }
}

void memcpy64(uint64_t *dest, uint64_t *src, size_t count) {
    for(size_t i = 0; i < count; i++) {
        *dest++ = *src++;
    }
}

extern "C" void memcpy(void *dest, void *src, size_t count) { 
    uint8_t *d = reinterpret_cast<uint8_t*>(dest);
    uint8_t *s = reinterpret_cast<uint8_t*>(src);

    for(size_t i = 0; i < count; i++) {
        *d++ = *s++;
    }
}

size_t strlen(const char *str) {
    if(!str)
        return 0;

    size_t len = 0;
    while(str[len])
        len++;
    return len;
}


ssize_t strcmp(const char *str0, const char *str1) {
    for(size_t i = 0;; i++) {
        if(str0[i] != str1[i]) 
            return str0[i] - str1[i];
        if(!str0[i])
            return 0;
    }
}

ssize_t strncmp(const char *str0, const char *str1, size_t n) {
    for(size_t i = 0; i < n; i++) {
        if(str0[i] != str1[i]) 
            return str0[i] - str1[i];
        if(!str0[i])
            return 0;
    }

    return 0;
}

char *strcpy(char *dest, const char *src) {
    size_t i;

    for(i = 0; src[i]; i++)
        dest[i] = src[i];

    dest[i] = 0;

    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;

    for(i = 0; i < n && src[i]; i++)
        dest[i] = src[i];
    for(; i < n; i++)
        dest[i] = 0;

    return dest;
}
