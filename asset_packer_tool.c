#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "darray.h"
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
} texture_data_t;

typedef struct {
    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t normal_count;
    uint32_t color_count;
    uint32_t uv_count;
    uint32_t uv_index_count;
} mesh_data_t;

typedef struct {
    char path[256];
    int width;
    int height;
    int channels;
    unsigned char* data;
} loaded_texture_t;

typedef struct {
    char path[256];
    mesh_t mesh;
} loaded_mesh_t;

static void
write_header(FILE* f, uint32_t chunk_count, uint32_t total_size) {
    asset_header_t header = {
        .magic = CHUNK_HEADER,
        .version = 1,
        .chunk_count = chunk_count,
        .total_size = total_size
    };
    fwrite(&header, sizeof(asset_header_t), 1, f);
}

static size_t
write_texture_chunk(FILE* f, loaded_texture_t* tex) {
    size_t pixel_data_size = tex->width * tex->height * 4;
    size_t chunk_size = sizeof(texture_data_t) + pixel_data_size;

    chunk_header_t chunk_hdr = {
        .chunk_id = CHUNK_TEXTURE,
        .chunk_size = chunk_size
    };
    fwrite(&chunk_hdr, sizeof(chunk_header_t), 1, f);

    texture_data_t tex_data = {
        .width = tex->width,
        .height = tex->height,
        .format = 0
    };
    fwrite(&tex_data, sizeof(texture_data_t), 1, f);

    fwrite(tex->data, 1, pixel_data_size, f);

    printf("  [TEXR] %s: %dx%d (%.2f KB)\n", 
           tex->path, tex->width, tex->height, 
           (sizeof(chunk_header_t) + chunk_size) / 1024.0f);

    return sizeof(chunk_header_t) + chunk_size;
}

static size_t
write_mesh_chunk(FILE* f, loaded_mesh_t* lmesh) {
    mesh_t* mesh = &lmesh->mesh;
    
    size_t vertex_size = da_size(mesh->vertices) * sizeof(float);
    size_t index_size = da_size(mesh->indices) * sizeof(uint32_t);
    size_t normal_size = da_size(mesh->normals) * sizeof(float);
    size_t color_size = da_size(mesh->colors) * sizeof(uint32_t);
    size_t uv_size = da_size(mesh->uvs) * sizeof(float);
    size_t uv_index_size = da_size(mesh->uv_indices) * sizeof(uint32_t);

    size_t chunk_size = sizeof(mesh_data_t) + 
                       vertex_size + 
                       index_size + 
                       normal_size + 
                       color_size + 
                       uv_size + 
                       uv_index_size;

    chunk_header_t chunk_hdr = {
        .chunk_id = CHUNK_MESH,
        .chunk_size = chunk_size
    };
    fwrite(&chunk_hdr, sizeof(chunk_header_t), 1, f);

    mesh_data_t mesh_data = {
        .vertex_count = da_size(mesh->vertices),
        .index_count = da_size(mesh->indices),
        .normal_count = da_size(mesh->normals),
        .color_count = da_size(mesh->colors),
        .uv_count = da_size(mesh->uvs),
        .uv_index_count = da_size(mesh->uv_indices)
    };
    fwrite(&mesh_data, sizeof(mesh_data_t), 1, f);

    if (vertex_size > 0) {
        fwrite(mesh->vertices->data, 1, vertex_size, f);
    }
    if (index_size > 0) {
        fwrite(mesh->indices->data, 1, index_size, f);
    }
    if (normal_size > 0) {
        fwrite(mesh->normals->data, 1, normal_size, f);
    }
    if (color_size > 0) {
        fwrite(mesh->colors->data, 1, color_size, f);
    }
    if (uv_size > 0) {
        fwrite(mesh->uvs->data, 1, uv_size, f);
    }
    if (uv_index_size > 0) {
        fwrite(mesh->uv_indices->data, 1, uv_index_size, f);
    }

    printf("  [MESH] %s: v=%u i=%u (%.2f KB)\n", 
           lmesh->path, mesh_data.vertex_count, mesh_data.index_count,
           (sizeof(chunk_header_t) + chunk_size) / 1024.0f);

    return sizeof(chunk_header_t) + chunk_size;
}

static void
write_done_chunk(FILE* f) {
    chunk_header_t chunk_hdr = {
        .chunk_id = CHUNK_DONE,
        .chunk_size = 0
    };
    fwrite(&chunk_hdr, sizeof(chunk_header_t), 1, f);
}

static void
generate_c_file(const char* binary_path, const char* output_path, const char* array_name) {
    FILE* bin_file = fopen(binary_path, "rb");
    if (!bin_file) {
        printf("Failed to open binary file: %s\n", binary_path);
        return;
    }

    fseek(bin_file, 0, SEEK_END);
    size_t file_size = ftell(bin_file);
    fseek(bin_file, 0, SEEK_SET);

    uint8_t* data = malloc(file_size);
    fread(data, 1, file_size, bin_file);
    fclose(bin_file);

    FILE* c_file = fopen(output_path, "w");
    if (!c_file) {
        printf("Failed to create C file: %s\n", output_path);
        free(data);
        return;
    }

    fprintf(c_file, "#include <stdint.h>\n\n");
    fprintf(c_file, "const uint32_t %s_size = %zu;\n\n", array_name, file_size);
    fprintf(c_file, "const uint8_t %s[] = {\n", array_name);

    for (size_t i = 0; i < file_size; i++) {
        if (i % 12 == 0) {
            fprintf(c_file, "    ");
        }
        fprintf(c_file, "0x%02X", data[i]);
        if (i < file_size - 1) {
            fprintf(c_file, ",");
            if ((i + 1) % 12 == 0) {
                fprintf(c_file, "\n");
            } else {
                fprintf(c_file, " ");
            }
        }
    }

    fprintf(c_file, "\n};\n");
    fclose(c_file);
    free(data);

    printf("Generated C file: %s (%.2f KB)\n", output_path, file_size / 1024.0f);
}

static int
has_extension(const char* filename, const char* ext) {
    size_t len = strlen(filename);
    size_t ext_len = strlen(ext);
    if (len < ext_len) return 0;
    return strcmp(filename + len - ext_len, ext) == 0;
}

int
main(int argc, char** argv) {
    if (argc < 3) {
        printf("Asset Packer Tool v2.0\n");
        printf("Usage: %s <output_name> <file1> <file2> ...\n", argv[0]);
        printf("\nSupported formats:\n");
        printf("  Models:   .obj\n");
        printf("  Textures: .png .jpg .jpeg .bmp .tga .psd .gif .hdr .pic .pnm\n");
        printf("\nExample:\n");
        printf("  %s game cube.obj cube.png sphere.obj sphere.png player.obj player.png\n", argv[0]);
        printf("\nThis will generate:\n");
        printf("  - game.assets (binary blob)\n");
        printf("  - game_assets.c (C array)\n");
        printf("\nAll textures will be packed first, then all meshes.\n");
        return 1;
    }

    const char* output_name = argv[1];

    char binary_path[256];
    char c_file_path[256];
    char array_name[256];

    snprintf(binary_path, sizeof(binary_path), "%s.assets", output_name);
    snprintf(c_file_path, sizeof(c_file_path), "%s_assets.c", output_name);
    snprintf(array_name, sizeof(array_name), "%s_assets", output_name);

    printf("=== Asset Packer v2.0 ===\n");
    printf("Output name: %s\n\n", output_name);

    loaded_texture_t textures[64];
    loaded_mesh_t meshes[64];
    int texture_count = 0;
    int mesh_count = 0;

    printf("=== Phase 1: Loading Assets ===\n");
    
    for (int i = 2; i < argc; i++) {
        const char* filepath = argv[i];
        
        if (has_extension(filepath, ".obj")) {
            printf("Loading mesh: %s\n", filepath);
            if (mesh_count >= 64) {
                printf("  WARNING: Too many meshes (max 64), skipping\n");
                continue;
            }
            
            loaded_mesh_t* lmesh = &meshes[mesh_count];
            strncpy(lmesh->path, filepath, sizeof(lmesh->path) - 1);
            lmesh->mesh = mesh_obj_load(filepath);
            
            printf("  Vertices: %zu, Indices: %zu, UVs: %zu\n",
                   da_size(lmesh->mesh.vertices),
                   da_size(lmesh->mesh.indices),
                   da_size(lmesh->mesh.uvs));
            
            mesh_count++;
        }
        else if (has_extension(filepath, ".png") || 
                 has_extension(filepath, ".jpg") ||
                 has_extension(filepath, ".jpeg") ||
                 has_extension(filepath, ".bmp") ||
                 has_extension(filepath, ".tga") ||
                 has_extension(filepath, ".psd") ||
                 has_extension(filepath, ".gif") ||
                 has_extension(filepath, ".hdr") ||
                 has_extension(filepath, ".pic") ||
                 has_extension(filepath, ".pnm")) {
            printf("Loading texture: %s\n", filepath);
            if (texture_count >= 64) {
                printf("  WARNING: Too many textures (max 64), skipping\n");
                continue;
            }
            
            loaded_texture_t* tex = &textures[texture_count];
            strncpy(tex->path, filepath, sizeof(tex->path) - 1);
            tex->data = stbi_load(filepath, &tex->width, &tex->height, &tex->channels, 4);
            
            if (!tex->data) {
                printf("  ERROR: Failed to load texture\n");
                continue;
            }
            
            printf("  %dx%d, %d channels\n", tex->width, tex->height, tex->channels);
            texture_count++;
        }
        else {
            printf("Unknown file type: %s (skipping)\n", filepath);
        }
    }

    printf("\n=== Summary ===\n");
    printf("Textures loaded: %d\n", texture_count);
    printf("Meshes loaded: %d\n", mesh_count);

    if (texture_count == 0 && mesh_count == 0) {
        printf("\nNo assets loaded! Exiting.\n");
        return 1;
    }

    printf("\n=== Phase 2: Writing Packed File ===\n");
    
    FILE* temp_file = fopen("temp.assets", "wb");
    if (!temp_file) {
        printf("Failed to create temporary file\n");
        return 1;
    }

    asset_header_t dummy_header = {0};
    fwrite(&dummy_header, sizeof(asset_header_t), 1, temp_file);

    size_t total_size = sizeof(asset_header_t);
    uint32_t chunk_count = 0;

    if (texture_count > 0) {
        printf("\nTexture Chunks:\n");
        for (int i = 0; i < texture_count; i++) {
            size_t written = write_texture_chunk(temp_file, &textures[i]);
            total_size += written;
            chunk_count++;
        }
    }

    if (mesh_count > 0) {
        printf("\nMesh Chunks:\n");
        for (int i = 0; i < mesh_count; i++) {
            size_t written = write_mesh_chunk(temp_file, &meshes[i]);
            total_size += written;
            chunk_count++;
        }
    }

    write_done_chunk(temp_file);
    total_size += sizeof(chunk_header_t);
    chunk_count++;

    fseek(temp_file, 0, SEEK_SET);
    write_header(temp_file, chunk_count, total_size);
    fclose(temp_file);

    remove(binary_path);
    rename("temp.assets", binary_path);

    printf("\n=== Packing Complete ===\n");
    printf("Total chunks: %u (%d textures + %d meshes + 1 done)\n", 
           chunk_count, texture_count, mesh_count);
    printf("Total size: %.2f KB\n", total_size / 1024.0f);
    printf("Binary file: %s\n", binary_path);

    printf("\n=== Phase 3: Generating C File ===\n");
    generate_c_file(binary_path, c_file_path, array_name);

    for (int i = 0; i < texture_count; i++) {
        stbi_image_free(textures[i].data);
    }

    for (int i = 0; i < mesh_count; i++) {
        da_destroy(meshes[i].mesh.vertices);
        da_destroy(meshes[i].mesh.indices);
        da_destroy(meshes[i].mesh.normals);
        da_destroy(meshes[i].mesh.colors);
        da_destroy(meshes[i].mesh.uvs);
        da_destroy(meshes[i].mesh.uv_indices);
    }

    printf("\n=== Done! ===\n");

    return 0;
}