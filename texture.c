#include "texture.h"
#include "asset_loader.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "darray.h"

#define MAX_TEXTURES 8
darray_t* textures_raw[MAX_TEXTURES];
int textures_count = 0;

uint32_t
texture_get_color(float u, float v) {
    if (textures_count == 0 || textures_raw[0] == NULL) {
        return 0xFFFFFFFF; // White fallback
    }
    darray_t* texture = textures_raw[0];
    const uint32_t* raw = da_get(texture, 0);
    uint32_t w = *raw++;
    uint32_t h = *raw++;
    const uint32_t* tex = raw;

    int x = (int)(u * (w  - 1) + 0.5f);
    int y = (int)(v * (h - 1) + 0.5f);
    int index = (y * w + x);
    return tex[index];
}

uint32_t
texture_get_color_from_asset(const struct asset_texture_t* texture, float u, float v) {
    /* Use asset loader texture if provided */
    if (texture) {
        return asset_texture_get_color((const asset_texture_t*)texture, u, v);
    }
    /* Fallback to legacy texture system */
    return texture_get_color(u, v);
}

void
textures_load(const char* path) {
    FILE* input = fopen(path, "rb");
    uint32_t w, h;
    darray_t* raw;
    uint32_t* raw_me;
    while (fread(&w, sizeof(uint32_t), 1, input) > 0) {
        fread(&h, sizeof(uint32_t), 1, input);
        raw = da_create(sizeof(uint32_t));
        da_resize(raw, w * h + 2);
        raw_me = da_get(raw, 0);
        raw_me[0] = w;
        raw_me[1] = h;
        int read = fread(&raw_me[2], sizeof(uint32_t), w * h, input);
        printf("read: %d\n", read);
        textures_raw[textures_count++] = raw;
    }
    fclose(input);
}
