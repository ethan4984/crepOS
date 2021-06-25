#include <drivers/tty.hpp>
#include <mm/vmm.hpp>
#include <fs/vfs.hpp>

namespace tty {

static size_t tty_cnt = 0;

screen::screen(stivale *stivale_struct) {
    framebuffer = reinterpret_cast<volatile uint32_t*>(stivale_struct->fb_addr + vmm::high_vma);

    height = stivale_struct->fb_height;
    width = stivale_struct->fb_width;
    pitch = stivale_struct->fb_pitch;
    bpp = stivale_struct->fb_bpp;
    size = height * pitch;

    double_buffer = reinterpret_cast<volatile uint32_t*>(pmm::calloc(div_roundup(size, vmm::page_size)) + vmm::high_vma);

    size_t fb_page_off = stivale_struct->fb_addr / vmm::page_size;
    for(ssize_t i = 0; i < div_roundup(size, 0x200000); i++) { // map fb and double fb as wc
        vmm::kernel_mapping->map_page_raw(fb_page_off + vmm::high_vma + i * 0x200000, fb_page_off + i * 0x200000, 0x3, 0x3 | (1 << 7) | (1 << 8), vmm::pa_wc);
        vmm::kernel_mapping->map_page_raw((size_t)double_buffer + i * 0x200000, (size_t)double_buffer - vmm::high_vma + i * 0x200000, 0x3, 0x3 | (1 << 7) | (1 << 8), vmm::pa_wc);
    }
}

inline void screen::set_pixel(ssize_t x, ssize_t y, uint32_t colour) {
    size_t index = x + pitch / (bpp / 8) * y;

    framebuffer[index] = colour;
    double_buffer[index] = colour;
}

inline uint32_t screen::get_pixel(ssize_t x, ssize_t y) {
    return double_buffer[x + pitch / (bpp / 8) * y];
}

void screen::flush(uint32_t colour) {
    for(size_t i = 0; i < height; i++) {
        for(size_t j = 0; j < width; j++) {
            set_pixel(j, i, colour);
        }
    }
}

void tty::render_char(ssize_t x, ssize_t y, uint32_t fg, uint32_t bg, char c) {
    uint16_t offset = ((uint8_t)c - 0x20) * font_height;
    for(uint8_t i = 0, i_cnt = 8; i < font_width && i_cnt > 0; i++, i_cnt--) {
        for(uint8_t j = 0; j < font_height; j++) {
            if((font[offset + j] >> i) & 1)
                sc.set_pixel(x + i_cnt, y + j, fg);
            else
                sc.set_pixel(x + i_cnt, y + j, bg);
        }
    }
}

void tty::plot_char(ssize_t x, ssize_t y, uint32_t fg, uint32_t bg, char c) {
    render_char(x * font_width, y * font_height, fg, bg, c);    
    char_grid[x + y * cols] = c;
}

void tty::update_cursor(ssize_t x, ssize_t y) {
    clear_cursor();

    cursor_x = x;
    cursor_y = y;
    
    draw_cursor();
}

void tty::clear_cursor() {
    for(size_t i = 0; i < font_height; i++) {
        for(size_t j = 0; j < font_width; j++) {
            sc.set_pixel(j + cursor_x * font_width, i + cursor_y * font_height, text_background);
        }
    }
}

void tty::draw_cursor() {
    for(size_t i = 0; i < font_height; i++) {
        for(size_t j = 0; j < font_width; j++) {
            sc.set_pixel(j + cursor_x * font_width, i + cursor_y * font_height, cursor_foreground);
        }
    }
}

void tty::scroll() {
    clear_cursor();

    for(ssize_t i = cols; i < rows * cols; i++) {
        plot_char((i - cols) % cols, (i - cols) / cols, text_foreground, text_background, char_grid[i]);
    }

    for(ssize_t i = rows * cols - cols; i < rows * cols; i++) {
        plot_char(i % cols, i / cols, text_foreground, text_background, ' ');
    }

    draw_cursor();
}

void tty::putchar(char c) {
    switch(c) {
        case '\t':
            for(ssize_t i = 0; i < tab_size; i++) {
                putchar(' ');
            }
            break;
        case '\n':
            if(cursor_y == rows - 1) {
                update_cursor(0, cursor_y);
                scroll();
            } else { 
                update_cursor(0, cursor_y + 1);
            }
            break;
        case '\b':
            if(cursor_x || cursor_y) {
                clear_cursor();

                if(cursor_x) {
                    cursor_x--;
                } else {
                    cursor_y--;
                    cursor_x = cols - 1;
                }
                
                draw_cursor();
            }
            break;
        default:
            clear_cursor(); 

            plot_char(cursor_x++, cursor_y, text_foreground, text_background, c);

            if(cursor_x == cols) {
                cursor_x = 0;
                cursor_y++;
            }

            if(cursor_y == rows) {
                cursor_y--;
                scroll();
            }

            draw_cursor();
    }
}

int tty_ioctl::call(regs *regs_cur) {
    switch(regs_cur->rsi) {
        case tiocginsz: {
            winsize *win = reinterpret_cast<winsize*>(regs_cur->rdx);
            win->ws_row = tty_cur.rows;
            win->ws_col = tty_cur.cols;
            win->ws_xpixel = tty_cur.sc.width; 
            win->ws_ypixel = tty_cur.sc.height;
            return 0;
        }
    }

    return -1;
}

tty::tty(screen &sc, uint8_t *font, size_t font_height, size_t font_width) :
    cursor_foreground(default_cursor_fg),
    text_foreground(default_text_fg),
    text_background(default_text_bg),
    cursor_x(0),
    cursor_y(0),
    tab_size(default_tab_size),
    font(font),
    font_height(font_height),
    font_width(font_width),
    sc(sc) {
    sc.flush(text_background);
    rows = sc.height / font_height;
    cols = sc.width / font_width;
    char_grid = (char*)kmm::calloc(rows * cols);
    draw_cursor();
    current_tty = tty_cnt;

    tty_ioctl *ioctl = new tty_ioctl(*this);
    vfs::node new_node(lib::string("/dev/tty") + tty_cnt++, ioctl);
}

}
