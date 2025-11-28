#include "darray.h"
#include "mesh.h"
#include "mm.h"
#include "xeno.h"
#include <stdint.h>

float BOX_VERTICES[8 * 3] = {
    -1, -1, -1,
    -1,  1, -1,
     1,  1, -1,
     1, -1, -1,
     1,  1,  1,
     1, -1,  1,
    -1,  1,  1,
    -1, -1,  1
};

uint32_t BOX_INDICES[12 * 3] = {
    1, 2, 3,
    1, 3, 4,
    4, 3, 5,
    4, 5, 6,
    6, 5, 7,
    6, 7, 8,
    8, 7, 2,
    8, 2, 1,
    2, 7, 5,
    2, 5, 3,
    8, 1, 4,
    8, 4, 6
};

mesh_t
mesh_create_box(float width,
                float height,
                float depth) {
    mesh_t m = {
        da_create(sizeof(float)),
        da_create(sizeof(uint32_t)),
        da_create(sizeof(float)),
        da_create(sizeof(uint32_t)),
        da_create(sizeof(float)),
        da_create(sizeof(uint32_t)),
        mat4_identity,
        NULL
    };
    da_resize(m.vertices, sizeof BOX_VERTICES / sizeof BOX_VERTICES[0]);
    da_resize(m.indices, sizeof BOX_INDICES / sizeof BOX_INDICES[0]);
    x_mem_copy(m.vertices->data, BOX_VERTICES, m.vertices->count * m.vertices->stride);
    x_mem_copy(m.indices->data, BOX_INDICES, m.indices->count * m.indices->stride);
    return m;
}
