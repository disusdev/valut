#include "collision.h"
#include "mm.h"
#include <assert.h>

static plane_t planes[FRUSTUM_COUNT];

void
init_planes(float fov_x, float fov_y, float znear, float zfar) {
	float cos_half_fov_x = mm_cos(fov_x / 2);
	float sin_half_fov_x = mm_sin(fov_x / 2);
	float cos_half_fov_y = mm_cos(fov_y / 2);
	float sin_half_fov_y = mm_sin(fov_y / 2);

	planes[FRUSTUM_LEFT].point = vec3_zero;
	planes[FRUSTUM_LEFT].norm.x = cos_half_fov_x;
	planes[FRUSTUM_LEFT].norm.y = 0;
	planes[FRUSTUM_LEFT].norm.z = sin_half_fov_x;

	planes[FRUSTUM_RIGHT].point = vec3_zero;
	planes[FRUSTUM_RIGHT].norm.x = -cos_half_fov_x;
	planes[FRUSTUM_RIGHT].norm.y = 0;
	planes[FRUSTUM_RIGHT].norm.z = sin_half_fov_x;

	planes[FRUSTUM_TOP].point = vec3_zero;
	planes[FRUSTUM_TOP].norm.x = 0;
	planes[FRUSTUM_TOP].norm.y = -cos_half_fov_y;
	planes[FRUSTUM_TOP].norm.z = sin_half_fov_y;

	planes[FRUSTUM_BOTTOM].point = vec3_zero;
	planes[FRUSTUM_BOTTOM].norm.x = 0;
	planes[FRUSTUM_BOTTOM].norm.y = cos_half_fov_y;
	planes[FRUSTUM_BOTTOM].norm.z = sin_half_fov_y;

	planes[FRUSTUM_NEAR].point = (vec3_t) {{ 0, 0, znear }};
	planes[FRUSTUM_NEAR].norm.x = 0;
	planes[FRUSTUM_NEAR].norm.y = 0;
	planes[FRUSTUM_NEAR].norm.z = 1;

	planes[FRUSTUM_FAR].point = (vec3_t) {{ 0, 0, zfar }};
	planes[FRUSTUM_FAR].norm.x = 0;
	planes[FRUSTUM_FAR].norm.y = 0;
	planes[FRUSTUM_FAR].norm.z = -1;
}

polygon_t polygon_create(vec3_t v1, vec3_t v2, vec3_t v3, vec2_t uv1, vec2_t uv2, vec2_t uv3){
    polygon_t poly = {0};
    poly.vertices_count = 3;
    poly.vertices[0] = v1;
    poly.vertices[1] = v2;
    poly.vertices[2] = v3;
    poly.uvs[0] = uv1;
    poly.uvs[1] = uv2;
    poly.uvs[2] = uv3;
    return poly;
}

void
polygon_clip_plane(polygon_t* polygon,
                   int plane_kind) {
    vec3_t plane_point = planes[plane_kind].point;
    vec3_t plane_norm = planes[plane_kind].norm;

    vec3_t inside[MAX_VERTICES_COUNT];
    vec2_t inside_uvs[MAX_VERTICES_COUNT];
    int inside_count = 0;

    vec3_t* curr = &polygon->vertices[0];
    vec3_t* prev = &polygon->vertices[polygon->vertices_count - 1];

    vec2_t* curr_uv = &polygon->uvs[0];
    vec2_t* prev_uv = &polygon->uvs[polygon->vertices_count - 1];

    float dot_curr = 0;
    float dot_prev = v3_dot(plane_norm, v3_sub(*prev, plane_point));

    while (curr != &polygon->vertices[polygon->vertices_count]) {
        dot_curr = v3_dot(plane_norm, v3_sub(*curr, plane_point));

        if (dot_curr * dot_prev < 0) {
            intersect_t intersect = line_plane_intersect(planes[plane_kind], *curr, *prev);
            inside[inside_count] = intersect.point;
            inside_uvs[inside_count] = v2_lerp(*curr_uv, *prev_uv, intersect.t);
            inside_count++;
        }

        if (dot_curr > 0) {
            inside[inside_count] = *curr;
            inside_uvs[inside_count] = *curr_uv;
            inside_count++;
        }
        dot_prev = dot_curr;
        prev = curr;
        curr++;
        prev_uv = curr_uv;
        curr_uv++;
    }

    polygon->vertices_count = inside_count;
    int i;
    for (i = 0; i < inside_count; i++) {
        polygon->vertices[i] = inside[i];
        polygon->uvs[i] = inside_uvs[i];
    }
}

void polygon_clip(polygon_t* polygon) {
    polygon_clip_plane(polygon, FRUSTUM_LEFT);
    polygon_clip_plane(polygon, FRUSTUM_RIGHT);
    polygon_clip_plane(polygon, FRUSTUM_TOP);
    polygon_clip_plane(polygon, FRUSTUM_BOTTOM);
    polygon_clip_plane(polygon, FRUSTUM_NEAR);
    polygon_clip_plane(polygon, FRUSTUM_FAR);
}

intersect_t
line_plane_intersect(plane_t plane, vec3_t p1, vec3_t p2) {
    float dp1 = v3_dot(plane.norm, v3_sub(plane.point, p1));
    float dp2 = v3_dot(plane.norm, v3_sub(plane.point, p2));

    assert(mm_abs(dp1 - dp2) > 0.000001);

    float t = dp1 / (dp1 - dp2);

    vec3_t intersect = {{
        lerp(p1.x, p2.x, t),
        lerp(p1.y, p2.y, t),
        lerp(p1.z, p2.z, t)
    }};

    return (intersect_t) {
        intersect, t
    };
}
