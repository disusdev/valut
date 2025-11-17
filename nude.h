#ifndef __NUDE_H__
#define __NUDE_H__

#include "mm.h"
#include "mesh.h"
#include <stdint.h>

typedef enum {
    DRAW_FLAG_SHADE = 1 << 0,
    DRAW_FLAG_WIREFRAME = 1 << 1,
    DRAW_FLAG_DOT = 1 << 2,
    DRAW_FLAG_BACKFACE = 1 << 3,
    DRAW_FLAG_CULLING = 1 << 4,
    DRAW_FLAG_NORMALS = 1 << 5,
    DRAW_FLAG_FULLRECT = 1 << 6
} drawing_flags_t;

typedef struct {
    mesh_t mesh;
    mat4_t view;
    mat4_t proj;
} mesh_queue_entry_t;

typedef struct {
    mesh_queue_entry_t* entries;
    int count;
    int capacity;
} mesh_queue_t;

typedef struct {
    uint32_t* data;
    uint32_t width;
    uint32_t height;
} texture2d_t;

typedef struct {
    uint32_t* color;
    uint32_t* depth;
    uint32_t width;
    uint32_t height;
} framebuffer_t;

typedef union {
    uint32_t hex;
    struct {
        uint8_t b, g, r ,a;
    };
} color_t;

void n_ctx_view_set(mat4_t mat);
void n_ctx_proj_set(mat4_t mat);

void n_flag_toggle(drawing_flags_t flag);
void n_size_set(int width, int height);
void n_clear_color_set(uint32_t color);
void n_clear(uint32_t* buffer, float* depth);
int n_point_draw(uint32_t* buffer, uint32_t x, uint32_t y, uint32_t color);
int n_depth_set(float* buffer, unsigned int x, unsigned int y, float depth);
void n_line2d_draw(uint32_t* buffer, int x1, int y1, int x2, int y2, uint32_t color);
void n_line2d_draw_gradient(uint32_t* buffer, int x1, int y1, int x2, int y2, uint32_t color1, uint32_t color2);
void n_texture_draw(uint32_t* buffer, int w, int h);

void n_grid_line_draw(uint32_t* buffer, int w, int h, int size);
void n_grid_dot_draw(uint32_t* buffer, int w, int h, int size, float time);
void n_rect_fill_draw(uint32_t* buffer, int sx, int sy, int width, int height, uint32_t color);
void n_triangle_dots_draw(uint32_t* buffer, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color);
void n_triangle_wire_draw(uint32_t* buffer, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color);
uint32_t n_color_percent(uint32_t color, float precent);
uint32_t n_color_mix3(uint32_t c1, uint32_t c2, uint32_t c3);

void n_draw_ray(uint32_t* buffer, vec3_t o, vec3_t d, uint32_t color);
void n_mesh_draw(uint32_t* buffer, float* depth, int w, int h, mesh_t mesh, mat4_t view, mat4_t proj);
void n_triangle_draw(uint32_t* buffer, float* depth, const void* texture, int x1, int y1, float z1, float w1, float u1, float v1, uint32_t c1, int x2, int y2, float z2, float w2, float u2, float v2, uint32_t c2, int x3, int y3, float z3, float w3, float u3, float v3, uint32_t c3);
void n_triangle_tex_draw(uint32_t* color, float* depth, const void* texture, int x1, int y1, float z1, float w1, float u1, float v1, uint32_t c1, int x2, int y2, float z2, float w2, float u2, float v2, uint32_t c2, int x3, int y3, float z3, float w3, float u3, float v3, uint32_t c3);
void n_triangle_fill_draw(uint32_t* color, float* depth, int x1, int y1, float z1, uint32_t c1, int x2, int y2, float z2, uint32_t c2, int x3, int y3, float z3, uint32_t c3);


void n_mesh_draw_wireframe(texture2d_t* texture, const mesh_t* mesh, const mat4_t model, const mat4_t view, const mat4_t proj, unsigned int color);

mesh_queue_t* nude_mesh_queue_create(void);

void nude_mesh_queue_destroy(mesh_queue_t* queue);

void nude_mesh_queue_clear(mesh_queue_t* queue);

void nude_mesh_queue_add(mesh_queue_t* queue, mesh_t mesh, mat4_t view, mat4_t proj);

void nude_render(mesh_queue_t* queue, uint32_t* color, float* depth, int w, int h);

#endif
