#ifndef __ASSET_LOADER_H__
#define __ASSET_LOADER_H__

#include <stdint.h>
#include "mesh.h"

#define CHUNK_HEADER 0x52444548
#define CHUNK_TEXTURE 0x52584554
#define CHUNK_MESH 0x4853454D
#define CHUNK_AUDIO 0x49445541
#define CHUNK_DONE 0x454E4F44

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t chunk_count;
    uint32_t total_size;
} asset_header_t;

typedef struct {
    uint32_t chunk_id;
    uint32_t chunk_size;
} chunk_header_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t format;
    const uint8_t* pixels;
} asset_texture_t;

typedef struct {
    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t normal_count;
    uint32_t color_count;
    uint32_t uv_count;
    uint32_t uv_index_count;
    const float* vertices;
    const uint32_t* indices;
    const float* normals;
    const uint32_t* colors;
    const float* uvs;
    const uint32_t* uv_indices;
} asset_mesh_t;

typedef struct {
    const uint8_t* data;
    uint32_t size;
    uint32_t position;
    
    asset_header_t header;
    
    asset_texture_t textures[8];
    asset_mesh_t meshes[8];
    uint32_t texture_count;
    uint32_t mesh_count;
} asset_blob_t;

int asset_blob_init(asset_blob_t* blob, const uint8_t* data, uint32_t size);
int asset_blob_load(asset_blob_t* blob);
asset_texture_t* asset_get_texture(asset_blob_t* blob, uint32_t index);
asset_mesh_t* asset_get_mesh(asset_blob_t* blob, uint32_t index);
mesh_t asset_mesh_to_mesh(const asset_mesh_t* asset_mesh);
uint32_t asset_texture_get_color(const asset_texture_t* texture, float u, float v);

#endif