#include "arena.h"

arena_t
arena_init(uint8_t* from, size_t cap) {
    arena_t arena = {
        from,
        cap,
        0
    };
    return arena;
}

void*
arena_alloc(arena_t* arena, size_t size) {
    if (size + arena->off >= arena->cap) {
        return 0;
    }

    void* ptr = arena->base + arena->off;
    arena->off += size;
    return ptr;
}

void
arena_reset(arena_t* arena) {
    arena->off = 0;
}
