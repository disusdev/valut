#ifndef __GAME_H__
#define __GAME_H__

#include <stdint.h>

typedef struct {
    unsigned int target_fps;

    uint32_t* color;
    float* depth;
    int width;
    int height;
} Game;

Game* g_init(const char* path);
void g_term();
void g_update(float);

#endif
