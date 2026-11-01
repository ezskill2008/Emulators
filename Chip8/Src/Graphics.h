#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

#define CHIP8_WIDTH  64
#define CHIP8_HEIGHT 32

typedef struct graphics graphics;

graphics* graphics_create(const char* title, int scale);
void graphics_draw(graphics* g, const uint32_t pixels[CHIP8_WIDTH * CHIP8_HEIGHT]);
void graphics_destroy(graphics* g);
int graphics_window_open(graphics* g);

#endif
