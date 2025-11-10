#include "darray.h"
#include "mesh.h"
#include "mm.h"
#include <stdint.h>
#include <stdio.h>

#define BUFFER_LENGTH 1024
static char buffer[BUFFER_LENGTH];

static float float_cache[6];
static uint32_t indices_cache[10];

mesh_t
mesh_obj_load(const char* file_path) {
    mesh_t mesh = {
        da_create(sizeof(float)),
        da_create(sizeof(uint32_t)),
        da_create(sizeof(float)),
        da_create(sizeof(uint32_t)),
        da_create(sizeof(float)),
        da_create(sizeof(uint32_t)),
        mat4_identity,
        NULL
    };

    FILE* f = fopen(file_path, "r");

    while(fgets(buffer, BUFFER_LENGTH, f)) {
        if (buffer[0] == '#') continue;
        if (buffer[0] == 'v' && buffer[1] == ' ') {
            int matches = sscanf(buffer, "v %f %f %f %f %f %f\n", &float_cache[0], &float_cache[1], &float_cache[2], &float_cache[3], &float_cache[4], &float_cache[5]);
            if (matches == 3) {
                da_add(mesh.vertices, &float_cache[0]);
                da_add(mesh.vertices, &float_cache[1]);
                da_add(mesh.vertices, &float_cache[2]);
                uint32_t color = 0xffffffff;
                da_add(mesh.colors, &color);
            } else {
                da_add(mesh.vertices, &float_cache[0]);
                da_add(mesh.vertices, &float_cache[1]);
                da_add(mesh.vertices, &float_cache[2]);
                uint32_t color = 0xff000000 +
                                ((uint32_t)(255 * float_cache[3]) << 16) +
                                ((uint32_t)(255 * float_cache[4]) << 8) +
                                255 * float_cache[5];
                da_add(mesh.colors, &color);
            }
        } else if (buffer[0] == 'v' && buffer[1] == 't' && buffer[2] == ' ') {
            int matches = sscanf(buffer, "vt %f %f\n", &float_cache[0], &float_cache[1]);
            if (matches == 2) {
                da_add(mesh.uvs, &float_cache[0]);
                da_add(mesh.uvs, &float_cache[1]);
            }
        } else if (buffer[0] == 'f' && buffer[1] == ' ') {
            int matches = sscanf(buffer, "f %d %d %d\n", &indices_cache[0], &indices_cache[1], &indices_cache[2]);
            if (matches == 3) {
                da_add(mesh.indices, &indices_cache[0]);
                da_add(mesh.indices, &indices_cache[1]);
                da_add(mesh.indices, &indices_cache[2]);
            } else {
                matches = sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", 
                    &indices_cache[0], &indices_cache[1], &indices_cache[2],
                    &indices_cache[3], &indices_cache[4], &indices_cache[5],
                    &indices_cache[6], &indices_cache[7], &indices_cache[8]);
                if (matches == 9) {
                    da_add(mesh.indices, &indices_cache[0]);
                    da_add(mesh.indices, &indices_cache[3]);
                    da_add(mesh.indices, &indices_cache[6]);
                    da_add(mesh.uv_indices, &indices_cache[1]);
                    da_add(mesh.uv_indices, &indices_cache[4]);
                    da_add(mesh.uv_indices, &indices_cache[7]);
                } else {
                    matches = sscanf(buffer, "f %d/%d %d/%d %d/%d\n", 
                        &indices_cache[0], &indices_cache[1], &indices_cache[2],
                        &indices_cache[3], &indices_cache[4], &indices_cache[5]);
                    if (matches == 6) {
                        da_add(mesh.indices, &indices_cache[0]);
                        da_add(mesh.indices, &indices_cache[2]);
                        da_add(mesh.indices, &indices_cache[4]);
                        da_add(mesh.uv_indices, &indices_cache[1]);
                        da_add(mesh.uv_indices, &indices_cache[3]);
                        da_add(mesh.uv_indices, &indices_cache[5]);
                    }
                }
            }
        }
    }

    fclose(f);

    return mesh;
}