#ifndef DEBUG_HPP_
#define DEBUG_HPP_

#include <cpu.hpp>
#include <string.hpp>

namespace lib {

template <typename T>
struct formatter {
    template <typename out>
    static void format(out &output_format, T arg, [[maybe_unused]] int base);
};

template <>
struct formatter<const char *> {
    template <typename out>
    static void format(out &output_format, const char *arg, [[maybe_unused]] int base) {
        while(*arg)
            output_format.write(*arg++);
    }
};

template <>
struct formatter<char *> {
    template <typename out>
    static void format(out &output_format, char *arg, [[maybe_unused]] int base) {
        while(*arg)
            output_format.write(*arg++);
    }
};

template <>
struct formatter<char> {
    template <typename out>
    static void format(out &output_format, char arg, [[maybe_unused]] int base) {
        output_format.write(arg);
    }
};

template <>
struct formatter<lib::string> {
    template <typename out>
    static void format(out &output_format, lib::string arg, [[maybe_unused]] int base) {
        for(size_t i = 0; i < arg.length(); i++)
            output_format.write(arg[i]);
    }
};

template <typename out, typename T>
void print_integer(out &output_format, T arg, int base) {
    static char digits[] = "0123456789ABCDEF";
    static char buffer[50];
    char *str;

    str = &buffer[49];
    *str = '\0';

    do {
        *--str = digits[arg % base];
        arg /= base;
    } while(arg);

    while(*str)
        output_format.write(*str++);
}

#define INTEGER_IMPL(type) \
    template<> \
    struct formatter<type> { \
        template <typename out> \
        static void format(out &output_format, type arg, [[maybe_unused]] int base) { \
            print_integer(output_format, arg, base); \
        } \
    };

INTEGER_IMPL(int)
INTEGER_IMPL(unsigned int)

INTEGER_IMPL(short int)
INTEGER_IMPL(unsigned short int)

INTEGER_IMPL(long int)
INTEGER_IMPL(unsigned long int)

INTEGER_IMPL(long long int)
INTEGER_IMPL(unsigned long long int)

INTEGER_IMPL(unsigned char)

template <typename out, typename T>
void format(out &output_format, T arg, int base) {
    formatter<T>::format(output_format, arg, base);
}

template <typename out, typename ...Args>
void print_format(out &output_format, const char *str, Args && ...args) {
    ([&](auto &arg) {
        while(*str) {
            if(*str == '{') {
                if(*(str + 1) == '}') {
                    format(output_format, arg, 10);
                    str += 2;
                    return;
                } else if(*(str + 2) == '}') {
                    switch(*(str + 1)) {
                        case 'b':
                            format(output_format, arg, 2);
                            break;
                        case 'd':
                            format(output_format, arg, 10);
                            break;
                        case 'x':
                            format(output_format, arg, 16);
                            break;
                        default:
                            format(output_format, arg, 16);
                    }
                    str += 3;
                    return;
                }
            }

            output_format.write(*str++);
        }
    } (args), ...);

    while(*str)
        output_format.write(*str++);
}

struct serial_formatter {
    void write(char c) {
        while((inb(com1 + 5) & 0x20) == 0);
        outb(com1, c);
    }
};

inline serial_formatter global_serial_format;

}

template <typename ...Args>
void print(const char *str, Args && ...args) {
    lib::print_format<lib::serial_formatter>(lib::global_serial_format, str, args...);
}

#endif

