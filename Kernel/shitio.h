#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "port.h"


size_t strlen(const char *str);

int strcmp(const char *a, const char *b);

static uint16_t *const VGA_MEMORY = (uint16_t*)0xB8000;

void initalize();

void k_print(char str[256],...);

void t_print(char str[256],...);

void putchar(char c);

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

void clear_screen();

bool end_of_terminal();

extern "C" void gdt_flush(uint32_t);
