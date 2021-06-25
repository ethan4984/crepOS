#ifndef __DRIVERS_TTY_HPP_
#define __DRIVERS_TTY_HPP_

#include <stivale.hpp>
#include <fs/vfs.hpp>
#include <map.hpp>

namespace tty {

constexpr size_t default_tab_size = 4;
constexpr size_t default_text_fg = 0xffffff;
constexpr size_t default_text_bg = 0;
constexpr size_t default_cursor_fg = default_text_fg;

class tty;

class tty_ioctl : public vfs::default_ioctl {
public:
    tty_ioctl(tty &tty_cur) : tty_cur(tty_cur) { }

    int call(regs *regs_cur);
private:
    static constexpr size_t tiocginsz = 0x5413;

    struct winsize {
        uint16_t ws_row;
        uint16_t ws_col;
        uint16_t ws_xpixel;
        uint16_t ws_ypixel;
    };

    tty &tty_cur;
};

class screen {
public:
    screen(stivale *stivale_struct);
    screen() = default;

    void set_pixel(ssize_t x, ssize_t y, uint32_t colour);
    uint32_t get_pixel(ssize_t x, ssize_t y);
    void flush(uint32_t colour);

    friend tty;
    friend tty_ioctl;
private:
    volatile uint32_t *framebuffer;
    volatile uint32_t *double_buffer;

    uint32_t pitch;
    uint32_t width; 
    uint32_t height;
    uint32_t bpp;
    uint32_t size;
};

class tty {
public:
    tty(screen &sc, uint8_t *font, size_t font_height, size_t font_width);
    tty() = default;

    void render_char(ssize_t x, ssize_t y, uint32_t fg, uint32_t bg, char c);
    void plot_char(ssize_t x, ssize_t y, uint32_t fg, uint32_t bg, char c);
    void update_cursor(ssize_t x, ssize_t y);
    void putchar(char c);
    void scroll();

    friend tty_ioctl;
private:
    void draw_cursor();
    void clear_cursor();

    uint32_t cursor_foreground;
    uint32_t text_foreground;
    uint32_t text_background; 

    ssize_t cursor_x;
    ssize_t cursor_y;
    ssize_t tab_size;

    uint8_t *font;
    size_t font_height;
    size_t font_width;

    ssize_t rows;
    ssize_t cols;

    screen sc;

    char *char_grid;
};

inline lib::map<ssize_t, tty> tty_list;
inline ssize_t current_tty;

}

#endif
