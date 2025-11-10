#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "mm.h"

typedef struct {
    rotor3_t rotation;
    vec3_t position;
    vec3_t target;

    float fov;
    float near;
    float far;
    float ortho_size;

    int kind;

    vec3_t forward;
    vec3_t right;
    vec3_t up;

    float pitch;
    float yaw;
} camera_t;

static const camera_t camera_perspective_default = {
    {{1, 0, 0, 0}},
    {{0, 0, 3}},
    {{0, 0, 0}},
    90.0f,
    0.1f,
    100.0f,
    2,
    1
};

mat4_t camera_rotation(camera_t* camera);
mat4_t camera_view(camera_t* camera);
mat4_t camera_projection(camera_t* camera, float aspect);
void camera_update(camera_t* camera, float dt);

#endif
