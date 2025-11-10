#include "camera.h"
#include "input.h"
#include "mm.h"

mat4_t
camera_rotation(camera_t* camera) {
    return r3_rotate(r3_from_euler(camera->yaw, camera->pitch, 0));
}

mat4_t
camera_view(camera_t* camera) {
    vec3_t eye = camera->position;
    return m4_look_at(eye, camera->target, camera->up);
}

mat4_t
camera_projection(camera_t* camera, float aspect) {
    if (camera->kind == 0) {
        float size = camera->ortho_size > 0.0f ? camera->ortho_size : 1.0f;
        return m4_ortho(size, aspect, camera->near, camera->far);
    } else {
        return m4_persp(camera->fov, aspect, camera->near, camera->far);
    }
}

void
camera_update(camera_t* camera, float dt) {
    vec3_t velocity = vec3_zero;

    float z = input_get_key(KEY_CODE_W) ? 1 :  input_get_key(KEY_CODE_S) ? -1 : 0;
    float x = input_get_key(KEY_CODE_D) ? -1 :  input_get_key(KEY_CODE_A) ? 1 : 0;
    float y = input_get_key(KEY_CODE_SPACE) ? 1 :  input_get_key(KEY_CODE_C) ? -1 : 0;

    vec3_t direction = {{ x, y, z }};

    float sum = x + y + z;
    if (mm_abs(sum) > 0.0f) {
        direction = v3_normalized(direction);
    }

    velocity = v3_scl(camera->forward, direction.z, direction.z, direction.z);
    velocity = v3_add(velocity, v3_scl(camera->right, direction.x, direction.x, direction.x));
    velocity = v3_add(velocity, v3_scl(camera->up, direction.y, direction.y, direction.y));

    camera->position = v3_add(camera->position, v3_scl(velocity, 2 * dt, 2 * dt, 2 * dt));

    if (input_get_key(KEY_CODE_RIGHT_ARROW)) {
        camera->yaw -= 2 * dt;
    }

    if (input_get_key(KEY_CODE_LEFT_ARROW)) {
        camera->yaw += 2 * dt;
    }

    if (input_get_key(KEY_CODE_UP_ARROW)) {
        camera->pitch -= 2 * dt;
    }

    if (input_get_key(KEY_CODE_DOWN_ARROW)) {
        camera->pitch += 2 * dt;
    }

    camera->rotation = r3_from_euler(camera->yaw, 0, camera->pitch);
    r3_get_axis(camera->rotation, &camera->forward, &camera->right, &camera->up);
    camera->target = v3_add(camera->position, camera->forward);
}
