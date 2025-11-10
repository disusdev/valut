#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "mm.h"

typedef enum {
    FRUSTUM_LEFT,
    FRUSTUM_RIGHT,
    FRUSTUM_TOP,
    FRUSTUM_BOTTOM,
    FRUSTUM_NEAR,
    FRUSTUM_FAR,
    FRUSTUM_COUNT
} frustum_plane_kind_t;

#define MAX_VERTICES_COUNT 10

typedef struct {
    vec3_t point;
    vec3_t norm;
} plane_t;

typedef struct {
    vec3_t point;
    float t;
} intersect_t;

typedef struct {
    vec3_t vertices[MAX_VERTICES_COUNT];
    vec2_t uvs[MAX_VERTICES_COUNT];
    int vertices_count;
} polygon_t;

void init_planes(float fov_x, float fov_y, float znear, float zfar);
polygon_t polygon_create(vec3_t v1, vec3_t v2, vec3_t v3, vec2_t uv1, vec2_t uv2, vec2_t uv3);
void polygon_clip(polygon_t* polygon);
void
polygon_clip_plane(polygon_t* polygon,
                   int plane_kind);

intersect_t line_plane_intersect(plane_t plane, vec3_t p1, vec3_t p2);

#endif