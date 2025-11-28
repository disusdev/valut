#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <stdint.h>

struct asset_texture_t;

extern int texture_width;
extern int texture_height;
extern const uint32_t texture_raw[];

uint32_t texture_get_color(float u, float v);
uint32_t texture_get_color_from_asset(const struct asset_texture_t* texture, float u, float v);
void textures_load(const char* path);

#endif
