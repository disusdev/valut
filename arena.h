#ifndef __ARENA_H__
#define __ARENA_H__

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* base;
    size_t cap;
    size_t off;
} arena_t;

arena_t arena_init(uint8_t* from, size_t cap);
void* arena_alloc(arena_t* arena, size_t size);
void arena_reset(arena_t* arena);

#endif
