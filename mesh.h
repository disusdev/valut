#ifndef __MESH_H__
#define __MESH_H__

#include "mm.h"
#include <stdint.h>
#include "darray.h"

typedef struct {
    darray_t* vertices;
    darray_t* indices;
    darray_t* normals;
    darray_t* colors;
    darray_t* uvs;
    darray_t* uv_indices;
    mat4_t transform;
    const void* texture;
} mesh_t;

mesh_t mesh_create_box(float width, float height, float depth);

mesh_t mesh_obj_load(const char* file_path);

#endif