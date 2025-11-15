#include "nude.h"
#include "collision.h"
#include "mm.h"
#include "texture.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

static drawing_flags_t flags = DRAW_FLAG_SHADE | DRAW_FLAG_CULLING | DRAW_FLAG_BACKFACE;

static struct {
    unsigned int clear_color;
    int width;
    int height;
    int ox;
    int oy;
    float aspect;
    mat4_t view;
    mat4_t proj;
} ctx = {
    0x00000000,
    1000, 1000,
    500, 500,
    1
};

void n_ctx_view_set(mat4_t mat) {
    ctx.view = mat;
}

void n_ctx_proj_set(mat4_t mat) {
    ctx.proj = mat;
}

static float
edge_function_bar(float ax, float ay, float bx, float by, float cx, float cy) {
    return (cx - ax) * (by - ay) - (cy - ay) * (bx - ax);
}

static int
top_left(float dx,
         float dy) {
    return (dy > 0) || (dy == 0 && dx > 0);
}

static vec4_t
m4_mul_v3_proj(mat4_t m, vec3_t v) {
    vec4_t res = m4_mul_v4(m, (vec4_t) {{ v.x, v.y,v.z, 1 }});

    if (res.w != 0.0) {
        res.x /= res.w;
        res.y /= res.w;
        res.z /= res.w;
    }

    return res;
}

static vec4_t
m4_mul_v4_proj(mat4_t m, vec4_t v) {
    vec4_t res = m4_mul_v4(m, v);

    if (res.w != 0.0) {
        res.x /= res.w;
        res.y /= res.w;
        res.z /= res.w;
    }

    return res;
}

void
n_flag_toggle(drawing_flags_t flag) {
    flags ^= flag;
}

void
n_size_set(int width, int height) {
    ctx.width = width;
    ctx.height = height;
    ctx.ox = width / 2;
    ctx.oy = height / 2;
    ctx.aspect = (float) height / width;
}

void
n_clear_color_set(unsigned int color) {
    ctx.clear_color = color;
}

void
n_clear(unsigned int* buffer, float* depth) {
    unsigned int count = ctx.width * ctx.height;
    memset(buffer, ctx.clear_color, count * sizeof(unsigned int));
    if (depth) {
        int i;
        for (i = 0; i < count; i++) {
            depth[i] = -3.402823466e+38f;
        }
    }
}

int n_point_draw(unsigned int* buffer, unsigned int x, unsigned int y, unsigned int color) {
    if (y * ctx.width + x < 0 || y * ctx.width + x > (ctx.width * ctx.height)) return 0;
    buffer[y * ctx.width + x] = color;
    return 1;
}

int n_depth_set(float* buffer, unsigned int x, unsigned int y, float depth) {
    if (depth >= buffer[y * ctx.width + x]) {
        buffer[y * ctx.width + x] = depth;
        return 1;
    }
    return 0;
}


void
n_line2d_draw(unsigned int* buffer, int x1, int y1, int x2, int y2, unsigned int color)
{
    int dx = mm_abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -mm_abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int error = dx + dy;

    while (1)
    {
        if (!n_point_draw(buffer, x1, y1, color)) return;

        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * error;

        if (e2 >= dy)
        {
            if (x1 == x2) break;
            error = error + dy;
            x1 = x1 + sx;
        }

        if (e2 <= dx)
        {
            if (y1 == y2) break;
            error = error + dx;
            y1 = y1 + sy;
        }
    }
}

void n_texture_draw(uint32_t* buffer, int w, int h) {
    int x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            n_point_draw(buffer, x, y, texture_get_color((float)x / w, (float)y / h));
        }
    }
}

void
n_grid_line_draw(uint32_t* buffer, int w, int h, int size) {
    int x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (x % size == 0 || y % size == 0) {
                n_point_draw(buffer, x, y, 0xff333333);
            }
        }
    }
}

void
n_grid_dot_draw(uint32_t* buffer,
                int w, int h,
                int size, float time) {
    int x, y;
    for (y = size / 2; y < h; y+=size) {
        for (x = size / 2; x < w; x+=size) {
            vec2_t color_pos = {{ x * 0.02 + time, y * 0.02 + time }};
            float ratio = mm_noise2d(color_pos.x, color_pos.y, 0);
            union {
                uint8_t data[4];
                uint32_t hex;
            } color = {{
                ratio * 255.0f,
                ratio * 255.0f,
                ratio * 255.0f,
                0xff,
            }};
            n_point_draw(buffer, x, y, color.hex);
        }
    }
}

void
n_rect_fill_draw(uint32_t* buffer, int sx, int sy, int width, int height, uint32_t color) {
    int x, y;
    for (y = sy; y < sy + height; y++) {
        for (x = sx; x < sx + width; x++) {
            n_point_draw(buffer, x, y, color);
        }
    }
}

void
n_triangle_dots_draw(uint32_t* buffer,
                     float x1, float y1,
                     float x2, float y2,
                     float x3, float y3,
                     uint32_t color) {
    n_rect_fill_draw(buffer, x1, y1, 4, 4, color);
    n_rect_fill_draw(buffer, x2, y2, 4, 4, color);
    n_rect_fill_draw(buffer, x3, y3, 4, 4, color);
}

void
n_triangle_wire_draw(uint32_t* buffer,
                     float x1, float y1,
                     float x2, float y2,
                     float x3, float y3,
                     uint32_t color) {
    n_line2d_draw(buffer, x1, y1, x2, y2, color);
    n_line2d_draw(buffer, x2, y2, x3, y3, color);
    n_line2d_draw(buffer, x3, y3, x1, y1, color);
}

uint32_t
n_color_percent(uint32_t color,
                float precent) {
    color_t c = { color };
    c.r *= precent;
    c.g *= precent;
    c.b *= precent;
    return c.hex;
}

uint32_t
n_color_mix3(uint32_t c1, uint32_t c2, uint32_t c3) {
    const float p = 1.0f / 3.0f;
    color_t c11 = { c1 };
    color_t c22 = { c2 };
    color_t c33 = { c3 };
    c11.r *= p; c11.g *= p; c11.b *= p; c11.a = 0;
    c22.r *= p; c22.g *= p; c22.b *= p; c22.a = 0;
    c33.r *= p; c33.g *= p; c33.b *= p; c33.a = 0;
    return 0xff000000 | ( c11.hex + c22.hex + c33.hex );
}

void
n_triangle_tex_draw(uint32_t* color, float* depth, const void* texture,
                    int x1, int y1, float z1, float w1, float u1, float v1, uint32_t c1,
                    int x2, int y2, float z2, float w2, float u2, float v2, uint32_t c2,
                    int x3, int y3, float z3, float w3, float u3, float v3, uint32_t c3) {
    int x, y;
    float area = edge_function_bar(x1, y1, x2, y2, x3, y3);
    if (area == 0.0f) return;

    int xmin = MIN(MIN(x1, x2), x3);
    int xmax = MAX(MAX(x1, x2), x3);
    int ymin = MIN(MIN(y1, y2), y3);
    int ymax = MAX(MAX(y1, y2), y3);

    float e1x = x2 - x1, e1y = y2 - y1;
    float e2x = x3 - x2, e2y = y3 - y2;
    float e3x = x1 - x3, e3y = y1 - y3;

    for (y = ymin; y < ymax + 1; y++) {
        for (x = xmin; x < xmax + 1; x++) {
            float px = x + 0.5f;
            float py = y + 0.5f;
            float a, b, g;

            a = edge_function_bar(x2, y2, x3, y3, px, py);
            b = edge_function_bar(x3, y3, x1, y1, px, py);
            g = edge_function_bar(x1, y1, x2, y2, px, py);

            int overlaps = 1;
            overlaps &= (a > 0 || (a == 0 && top_left(e1x, e1y)));
            overlaps &= (b > 0 || (b == 0 && top_left(e2x, e2y)));
            overlaps &= (g > 0 || (g == 0 && top_left(e3x, e3y)));

            if (overlaps) {
                a /= area;
                b /= area;
                g /= area;

                float z = 0.5f + (z1 * a) + (z2 * b) + (z3 * g);

                if (n_depth_set(depth, x, y, z)) {
                    float rw = a / w1 + b / w2 + g / w3;
                    float u = (u1 / w1 * a + u2 / w2 * b + u3 / w3 * g) / rw;
                    float v = (v1 / w1 * a + v2 / w2 * b + v3 / w3 * g) / rw;
                    uint32_t final = texture_get_color_from_asset((const struct asset_texture_t*)texture, u, v);
                    n_point_draw(color, x, y, final);
                }
            } else if (flags & DRAW_FLAG_FULLRECT) {
                a /= area;
                b /= area;
                g /= area;

                float z = 0.5f + (z1 * a) + (z2 * b) + (z3 * g);

                if (n_depth_set(depth, x, y, z)) {
                    n_point_draw(color, x, y, 0xff00ff00);
                }
            }
        }
    }
}

void
n_triangle_fill_draw(uint32_t* color, float* depth,
                     int x1, int y1, float z1, uint32_t c1,
                     int x2, int y2, float z2, uint32_t c2,
                     int x3, int y3, float z3, uint32_t c3) {
    float area = edge_function_bar(x1, y1, x2, y2, x3, y3);
    if (area == 0.0f) return;

    int xmin = MIN(MIN(x1, x2), x3);
    int xmax = MAX(MAX(x1, x2), x3);
    int ymin = MIN(MIN(y1, y2), y3);
    int ymax = MAX(MAX(y1, y2), y3);

    float e1x = x2 - x1, e1y = y2 - y1;
    float e2x = x3 - x2, e2y = y3 - y2;
    float e3x = x1 - x3, e3y = y1 - y3;

    int x, y;
    for (y = ymin; y < ymax + 1; y++) {
        for (x = xmin; x < xmax + 1; x++) {
            float px = x + 0.5f;
            float py = y + 0.5f;
            float a, b, g;

            a = edge_function_bar(x2, y2, x3, y3, px, py);
            b = edge_function_bar(x3, y3, x1, y1, px, py);
            g = edge_function_bar(x1, y1, x2, y2, px, py);

            int overlaps = 1;
            overlaps &= (a > 0 || (a == 0 && top_left(e1x, e1y)));
            overlaps &= (b > 0 || (b == 0 && top_left(e2x, e2y)));
            overlaps &= (g > 0 || (g == 0 && top_left(e3x, e3y)));

            if (overlaps) {
                a /= area;
                b /= area;
                g /= area;

                float z = 0.5f + (z1 * a) + (z2 * b) + (z3 * g);

                if (n_depth_set(depth, x, y, z)) {
                    color_t final = { n_color_mix3(c1, c2, c3) };
                    n_point_draw(color, x, y, final.hex);
                }
            } else if (flags & DRAW_FLAG_FULLRECT) {
                a /= area;
                b /= area;
                g /= area;

                float z = 0.5f + (z1 * a) + (z2 * b) + (z3 * g);

                if (n_depth_set(depth, x, y, z)) {
                    n_point_draw(color, x, y, 0xff00ff00);
                }
            }
        }
    }
}

void int_swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void float_swap(float* a, float* b) {
    float tmp = *a;
    *a = *b;
    *b = tmp;
}

void
n_triangle_draw(uint32_t* color, float* depth, const void* texture,
                int x1, int y1, float z1, float w1, float u1, float v1, uint32_t c1,
                int x2, int y2, float z2, float w2, float u2, float v2, uint32_t c2,
                int x3, int y3, float z3, float w3, float u3, float v3, uint32_t c3) {
    v1 = 1.0 - v1;
    v2 = 1.0 - v2;
    v3 = 1.0 - v3;

    if (flags & DRAW_FLAG_SHADE && flags & DRAW_FLAG_WIREFRAME && flags & DRAW_FLAG_DOT) {
        n_triangle_fill_draw(color, depth,
                             x1, y1, z1, c1,
                             x2, y2, z2, c2,
                             x3, y3, z3, c3);
        n_triangle_wire_draw(color, x1, y1, x2, y2, x3, y3, 0xffffffff - c1);
        n_triangle_dots_draw(color, x1, y1, x2, y2, x3, y3, 0xff66ff66 - c1);
    } else if (flags & DRAW_FLAG_SHADE && flags & DRAW_FLAG_WIREFRAME) {
        n_triangle_fill_draw(color, depth,
                             x1, y1, z1, c1,
                             x2, y2, z2, c2,
                             x3, y3, z3, c3);
        n_triangle_wire_draw(color, x1, y1, x2, y2, x3, y3, 0xffffffff - c1);
    } else if (flags & DRAW_FLAG_SHADE && flags & DRAW_FLAG_DOT) {
        n_triangle_fill_draw(color, depth,
                             x1, y1, z1, c1,
                             x2, y2, z2, c2,
                             x3, y3, z3, c3);
        n_triangle_dots_draw(color, x1, y1, x2, y2, x3, y3, 0xffffffff - c1);
    } else if (flags & DRAW_FLAG_WIREFRAME && flags & DRAW_FLAG_DOT) {
        n_triangle_wire_draw(color, x1, y1, x2, y2, x3, y3, c1);
        n_triangle_dots_draw(color, x1, y1, x2, y2, x3, y3, 0xffffffff - c1);
    } else if (flags & DRAW_FLAG_SHADE) {
        n_triangle_tex_draw(color, depth, texture,
                            x1, y1, z1, w1, u1, v1, c1,
                            x2, y2, z2, w2, u2, v2, c2,
                            x3, y3, z3, w3, u3, v3, c3);
    } else if (flags & DRAW_FLAG_WIREFRAME) {
        n_triangle_wire_draw(color, x1, y1, x2, y2, x3, y3, c1);
    } else if (flags & DRAW_FLAG_DOT) {
        n_triangle_dots_draw(color, x1, y1, x2, y2, x3, y3, c1);
    }
}

typedef struct {
    vec4_t v1;
    vec4_t v2;
    vec4_t v3;
    vec2_t uv1;
    vec2_t uv2;
    vec2_t uv3;
} face_t;

face_t faces[10];
int faces_count = 0;

void
n_mesh_draw(uint32_t* color, float* depth,
            int w, int h,
            mesh_t mesh, mat4_t view, mat4_t proj) {
    int i, j;
    int culling = flags & DRAW_FLAG_CULLING;

    if (!mesh.indices || da_size(mesh.indices) == 0) {
        return;
    }
    if (!mesh.vertices || da_size(mesh.vertices) == 0) {
        return;
    }

    int has_uvs = (mesh.uvs && mesh.uv_indices &&
                   da_size(mesh.uvs) > 0 && da_size(mesh.uv_indices) > 0);

    for (i = 0; i < mesh.indices->count; i += 3) {
        uint32_t* indices = da_get(mesh.indices, i);
        uint32_t* uv_indices = da_get(mesh.uv_indices, i);
        vec3_t v1 = *(vec3_t*) da_get(mesh.vertices, (indices[0] - 1) * 3);
        vec3_t v2 = *(vec3_t*) da_get(mesh.vertices, (indices[1] - 1) * 3);
        vec3_t v3 = *(vec3_t*) da_get(mesh.vertices, (indices[2] - 1) * 3);
        uint32_t c1 = *(uint32_t*) da_get(mesh.colors, (indices[0] - 1));
        uint32_t c2 = *(uint32_t*) da_get(mesh.colors, (indices[1] - 1));
        uint32_t c3 = *(uint32_t*) da_get(mesh.colors, (indices[2] - 1));
        vec2_t tc1 = *(vec2_t*) da_get(mesh.uvs, (uv_indices[0] - 1) * 2);
        vec2_t tc2 = *(vec2_t*) da_get(mesh.uvs, (uv_indices[1] - 1) * 2);
        vec2_t tc3 = *(vec2_t*) da_get(mesh.uvs, (uv_indices[2] - 1) * 2);

        vec4_t projection_points[3] = {
            {{ v1.x, v1.y, v1.z, 1 }},
            {{ v2.x, v2.y, v2.z, 1 }},
            {{ v3.x, v3.y, v3.z, 1 }}
        };
        mat4_t model = mesh.transform;

        {
            for (j = 0; j < 3; j++) {
                projection_points[j] = m4_mul_v4(model, projection_points[j]);
                projection_points[j] = m4_mul_v4(view, projection_points[j]);
            }
        }

        vec3_t v1_trans = *(vec3_t*)&projection_points[0];
        vec3_t v2_trans = *(vec3_t*)&projection_points[1];
        vec3_t v3_trans = *(vec3_t*)&projection_points[2];

        polygon_t polygon = polygon_create(v1_trans, v2_trans, v3_trans, tc1, tc2, tc3);
        polygon_clip(&polygon);

        if (polygon.vertices_count < 3) {
            continue;
        }

        faces_count = 0;

        for (j = 0; j < polygon.vertices_count - 2; j++) {
            faces[faces_count].v1 = (vec4_t) {{ polygon.vertices[0].x, polygon.vertices[0].y, polygon.vertices[0].z, 1 }};
            faces[faces_count].v2 = (vec4_t) {{ polygon.vertices[j + 1].x, polygon.vertices[j + 1].y, polygon.vertices[j + 1].z, 1 }};
            faces[faces_count].v3 = (vec4_t) {{ polygon.vertices[j + 2].x, polygon.vertices[j + 2].y, polygon.vertices[j + 2].z, 1 }};

            faces[faces_count].uv1 = polygon.uvs[0];
            faces[faces_count].uv2 = polygon.uvs[j + 1];
            faces[faces_count].uv3 = polygon.uvs[j + 2];
            faces_count++;
        }

        vec3_t e1 = v3_sub(v2, v1);
        vec3_t e2 = v3_sub(v3, v1);
        e1 = v3_normalized(e1);
        e2 = v3_normalized(e2);
        vec3_t n = v3_cross(e1, e2);
        n = v3_normalized(n);

        for (j = 0; j < faces_count; j++) {
            { /* PROJECTION */
                faces[j].v1 = m4_mul_v4_proj(proj, faces[j].v1);
                faces[j].v1.y *= -1;
                faces[j].v1.x *= ctx.ox;
                faces[j].v1.x += ctx.ox;
                faces[j].v1.y *= ctx.oy;
                faces[j].v1.y += ctx.oy;

                faces[j].v2 = m4_mul_v4_proj(proj, faces[j].v2);
                faces[j].v2.y *= -1;
                faces[j].v2.x *= ctx.ox;
                faces[j].v2.x += ctx.ox;
                faces[j].v2.y *= ctx.oy;
                faces[j].v2.y += ctx.oy;

                faces[j].v3 = m4_mul_v4_proj(proj, faces[j].v3);
                faces[j].v3.y *= -1;
                faces[j].v3.x *= ctx.ox;
                faces[j].v3.x += ctx.ox;
                faces[j].v3.y *= ctx.oy;
                faces[j].v3.y += ctx.oy;
            }

            { /* DRAW */
                float area2 = triangle_area2(faces[j].v1.x, faces[j].v1.y,
                                             faces[j].v2.x, faces[j].v2.y,
                                             faces[j].v3.x, faces[j].v3.y);
                if (culling && area2 < 0) continue;

                { /* CALCULATE LIGHT INTENCITY PER FACE */
                    float factor = -v3_dot(n, vec3_down);
                    factor = MIN(MAX(0.3f, factor), 1.0f);
                    c1 = n_color_percent(c1, factor);
                    c2 = n_color_percent(c2, factor);
                    c3 = n_color_percent(c3, factor);
                }

                n_triangle_draw(color, depth, mesh.texture,
                    faces[j].v1.x, faces[j].v1.y, faces[j].v1.z, faces[j].v1.w, faces[j].uv1.x, faces[j].uv1.y, c1,
                    faces[j].v3.x, faces[j].v3.y, faces[j].v3.z, faces[j].v3.w, faces[j].uv3.x, faces[j].uv3.y, c3,
                    faces[j].v2.x, faces[j].v2.y, faces[j].v2.z, faces[j].v2.w, faces[j].uv2.x, faces[j].uv2.y, c2);
            }
        }
    }
}

void
n_draw_ray(uint32_t* buffer, vec3_t o, vec3_t d, uint32_t color) {
    vec3_t end = v3_add(o, v3_scl(d, 100, 100, 100));

    vec4_t v1 = m4_mul_v3_proj(ctx.view, o);
    vec4_t v2 = m4_mul_v3_proj(ctx.view, end);

    v1 = m4_mul_v4_proj(ctx.proj, v1);
    v1.y *= -1;
    v1.x *= ctx.ox;
    v1.x += ctx.ox;
    v1.y *= ctx.oy;
    v1.y += ctx.oy;

    v2 = m4_mul_v4_proj(ctx.proj, v2);
    v2.y *= -1;
    v2.x *= ctx.ox;
    v2.x += ctx.ox;
    v2.y *= ctx.oy;
    v2.y += ctx.oy;

    n_line2d_draw(buffer, (int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y, color);
}
