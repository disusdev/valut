#ifndef __TEXT_H__
#define __TEXT_H__

#include <stdint.h>

const char* format_text(const char* format, ...);
void n_glyph_draw(uint32_t* buffer, int x0, int y0, const uint8_t glyph[8], uint32_t color, int scale);
void n_text_draw(uint32_t* buffer, int x, int y, const char* text, uint32_t color, int scale, int spacing);
void n_text_format_draw(uint32_t* buffer, int x, int y, uint32_t color, int scale, int spacing, const char* format, ...);
void n_text_size(const char* text, int scale, int spacing, int* w, int* h);

#endif
