#include "asset_loader.h"
#include "darray.h"
#include <stdio.h>
#include <string.h>

static uint32_t
read_uint32(const uint8_t* data, uint32_t* pos) {
    uint32_t value;
    memcpy(&value, data + *pos, sizeof(uint32_t));
    *pos += sizeof(uint32_t);
    return value;
}

static void
read_data(const uint8_t* src, uint32_t* pos, void* dst, size_t size) {
    memcpy(dst, src + *pos, size);
    *pos += size;
}

int
asset_blob_init(asset_blob_t* blob, const uint8_t* data, uint32_t size) {
    if (!blob || !data || size < sizeof(asset_header_t)) {
        return 0;
    }

    blob->data = data;
    blob->size = size;
    blob->position = 0;
    blob->texture_count = 0;
    blob->mesh_count = 0;

    memcpy(&blob->header, data, sizeof(asset_header_t));
    blob->position = sizeof(asset_header_t);

    if (blob->header.magic != CHUNK_HEADER) {
        printf("Invalid asset header magic: 0x%08X (expected 0x%08X)\n", 
               blob->header.magic, CHUNK_HEADER);
        return 0;
    }

    printf("Asset blob initialized:\n");
    printf("  Version: %u\n", blob->header.version);
    printf("  Chunks: %u\n", blob->header.chunk_count);
    printf("  Total size: %u bytes (%.2f KB)\n", 
           blob->header.total_size, 
           blob->header.total_size / 1024.0f);

    return 1;
}

int
asset_blob_load(asset_blob_t* blob) {
    if (!blob || !blob->data) {
        return 0;
    }

    printf("\n=== Loading Asset Chunks ===\n");

    while (blob->position < blob->size) {
        if (blob->position + sizeof(chunk_header_t) > blob->size) {
            printf("Not enough data for chunk header\n");
            break;
        }

        chunk_header_t chunk_hdr;
        memcpy(&chunk_hdr, blob->data + blob->position, sizeof(chunk_header_t));
        blob->position += sizeof(chunk_header_t);

        printf("\nChunk ID: 0x%08X, Size: %u bytes\n", 
               chunk_hdr.chunk_id, chunk_hdr.chunk_size);

        switch (chunk_hdr.chunk_id) {
            case CHUNK_TEXTURE: {
                if (blob->texture_count >= 8) {
                    printf("Too many textures, skipping\n");
                    blob->position += chunk_hdr.chunk_size;
                    break;
                }

                asset_texture_t* tex = &blob->textures[blob->texture_count];
                
                tex->width = read_uint32(blob->data, &blob->position);
                tex->height = read_uint32(blob->data, &blob->position);
                tex->format = read_uint32(blob->data, &blob->position);
                
                tex->pixels = blob->data + blob->position;
                
                uint32_t pixel_data_size = tex->width * tex->height * 4;
                blob->position += pixel_data_size;

                printf("  Texture [%u]: %ux%u, format=%u (%.2f KB)\n", 
                       blob->texture_count, tex->width, tex->height, tex->format,
                       pixel_data_size / 1024.0f);

                blob->texture_count++;
                break;
            }

            case CHUNK_MESH: {
                if (blob->mesh_count >= 8) {
                    printf("Too many meshes, skipping\n");
                    blob->position += chunk_hdr.chunk_size;
                    break;
                }

                asset_mesh_t* mesh = &blob->meshes[blob->mesh_count];
                
                mesh->vertex_count = read_uint32(blob->data, &blob->position);
                mesh->index_count = read_uint32(blob->data, &blob->position);
                mesh->normal_count = read_uint32(blob->data, &blob->position);
                mesh->color_count = read_uint32(blob->data, &blob->position);
                mesh->uv_count = read_uint32(blob->data, &blob->position);
                mesh->uv_index_count = read_uint32(blob->data, &blob->position);

                mesh->vertices = (mesh->vertex_count > 0) ? 
                    (const float*)(blob->data + blob->position) : NULL;
                blob->position += mesh->vertex_count * sizeof(float);

                mesh->indices = (mesh->index_count > 0) ? 
                    (const uint32_t*)(blob->data + blob->position) : NULL;
                blob->position += mesh->index_count * sizeof(uint32_t);

                mesh->normals = (mesh->normal_count > 0) ? 
                    (const float*)(blob->data + blob->position) : NULL;
                blob->position += mesh->normal_count * sizeof(float);

                mesh->colors = (mesh->color_count > 0) ? 
                    (const uint32_t*)(blob->data + blob->position) : NULL;
                blob->position += mesh->color_count * sizeof(uint32_t);

                mesh->uvs = (mesh->uv_count > 0) ? 
                    (const float*)(blob->data + blob->position) : NULL;
                blob->position += mesh->uv_count * sizeof(float);

                mesh->uv_indices = (mesh->uv_index_count > 0) ? 
                    (const uint32_t*)(blob->data + blob->position) : NULL;
                blob->position += mesh->uv_index_count * sizeof(uint32_t);

                printf("  Mesh [%u]:\n", blob->mesh_count);
                printf("    Vertices: %u\n", mesh->vertex_count);
                printf("    Indices: %u\n", mesh->index_count);
                printf("    Normals: %u\n", mesh->normal_count);
                printf("    Colors: %u\n", mesh->color_count);
                printf("    UVs: %u\n", mesh->uv_count);
                printf("    UV Indices: %u\n", mesh->uv_index_count);

                blob->mesh_count++;
                break;
            }

            case CHUNK_AUDIO: {
                printf("  Audio chunk (not implemented, skipping)\n");
                blob->position += chunk_hdr.chunk_size;
                break;
            }

            case CHUNK_DONE: {
                printf("  Done chunk reached\n");
                return 1;
            }

            default: {
                printf("  Unknown chunk type: 0x%08X, skipping\n", chunk_hdr.chunk_id);
                blob->position += chunk_hdr.chunk_size;
                break;
            }
        }
    }

    printf("\n=== Load Complete ===\n");
    printf("Loaded %u textures, %u meshes\n", blob->texture_count, blob->mesh_count);

    return 1;
}

asset_texture_t*
asset_get_texture(asset_blob_t* blob, uint32_t index) {
    if (!blob || index >= blob->texture_count) {
        return NULL;
    }
    return &blob->textures[index];
}

asset_mesh_t*
asset_get_mesh(asset_blob_t* blob, uint32_t index) {
    if (!blob || index >= blob->mesh_count) {
        return NULL;
    }
    return &blob->meshes[index];
}

mesh_t
asset_mesh_to_mesh(const asset_mesh_t* asset_mesh) {
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

    if (asset_mesh->vertex_count > 0 && asset_mesh->vertices) {
        da_resize(mesh.vertices, asset_mesh->vertex_count);
        memcpy(mesh.vertices->data, asset_mesh->vertices, 
               asset_mesh->vertex_count * sizeof(float));
    }

    if (asset_mesh->index_count > 0 && asset_mesh->indices) {
        da_resize(mesh.indices, asset_mesh->index_count);
        memcpy(mesh.indices->data, asset_mesh->indices, 
               asset_mesh->index_count * sizeof(uint32_t));
    }

    if (asset_mesh->normal_count > 0 && asset_mesh->normals) {
        da_resize(mesh.normals, asset_mesh->normal_count);
        memcpy(mesh.normals->data, asset_mesh->normals, 
               asset_mesh->normal_count * sizeof(float));
    }

    if (asset_mesh->color_count > 0 && asset_mesh->colors) {
        da_resize(mesh.colors, asset_mesh->color_count);
        memcpy(mesh.colors->data, asset_mesh->colors, 
               asset_mesh->color_count * sizeof(uint32_t));
    }

    if (asset_mesh->uv_count > 0 && asset_mesh->uvs) {
        da_resize(mesh.uvs, asset_mesh->uv_count);
        memcpy(mesh.uvs->data, asset_mesh->uvs, 
               asset_mesh->uv_count * sizeof(float));
    }

    if (asset_mesh->uv_index_count > 0 && asset_mesh->uv_indices) {
        da_resize(mesh.uv_indices, asset_mesh->uv_index_count);
        memcpy(mesh.uv_indices->data, asset_mesh->uv_indices, 
               asset_mesh->uv_index_count * sizeof(uint32_t));
    }

    return mesh;
}

uint32_t
asset_texture_get_color(const asset_texture_t* texture, float u, float v) {
    if (!texture || !texture->pixels) {
        return 0xFFFFFFFF;
    }

    if (u < 0.0f) u = 0.0f;
    if (u > 1.0f) u = 1.0f;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

    int x = (int)(u * (texture->width - 1) + 0.5f);
    int y = (int)(v * (texture->height - 1) + 0.5f);

    if (x < 0) x = 0;
    if (x >= (int)texture->width) x = texture->width - 1;
    if (y < 0) y = 0;
    if (y >= (int)texture->height) y = texture->height - 1;

    int index = y * texture->width + x;
    const uint32_t* pixels = (const uint32_t*)texture->pixels;

    return pixels[index];
}