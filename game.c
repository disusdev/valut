#include "game.h"
#include "darray.h"
#include "event.h"
#include "nude.h"
#include "text.h"
#include "xeno.h"
#include "mesh.h"
#include "camera.h"
#include <float.h>
#include <stdarg.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <vadefs.h>
#define _USE_MATH_DEFINES
#include "mm.h"
#include "input.h"
#include "collision.h"
#include "framegraph.h"
#include "texture.h"
#include <stdlib.h>
#include <stdio.h>
#include "asset_loader.h"

typedef struct {
    vec3_t vel;
    vec3_t pos;
    mesh_t mesh;
} ship_t;

struct {
    vec3_t ship_vel;
    vec3_t ship_pos;
    mesh_t ship_mesh;

    mesh_t planet_mesh;

    mesh_queue_t* mesh_queue;
} game_state = { 0 };

struct {
    vec3_t direction;
    float intensity;
} direct_light = {
    vec3_down,
    0.5
};

static camera_t camera = camera_perspective_default;
static camera_t td_camera = {
    {{1, 0, 0, 0}},
    {{0, 5, 0}},
    {{0, 0, 0}},
    90.0f,
    0.1f,
    100.0f,
    2,
    1,
    {0}, {0}, {0},
    90 * DEG2RAD
};

typedef struct {
    camera_t* camera;
    mat4_t* target;
} camera_follow_t;

static void camera_folow(camera_follow_t* follow) {

}

static camera_t* current_camera = &td_camera;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define MAX_MODELS 8

static Game game = {0};

static float ASPECT_RATIO = 1;
static vec2_t WINDOW_ORIGIN = {0};
static float TIME_ON_FRAME = 0;
static float TIME_SCALE = 1.0;

static int mx, my, wx, wy;
static double draw_time, update_time;

static asset_blob_t asset_blob = {0};
static uint8_t* asset_data = NULL;
static size_t asset_data_size = 0;

vec3_t global_mesh_pos = {0};

typedef struct {
    vec3_t o;
    vec3_t d;
} ray_t;

ray_t ray;

ray_t
ray_from_mouse(int x,
               int y,
               camera_t* camera) {
    ray_t result;

    float ndc_x = (2.0f * x) / game.width - 1.0f;
    float ndc_y = 1.0f - (2.0f * y) / game.height;

    mat4_t view = camera_view(camera);
    mat4_t proj = camera_projection(camera, ASPECT_RATIO);

    mat4_t inv_proj = m4_inv(proj);
    mat4_t inv_view = m4_inv(view);

    vec4_t ray_clip = {{ndc_x, ndc_y, -1.0f, 1.0f}};

    vec4_t ray_view = m4_mul_v4(inv_proj, ray_clip);
    ray_view.z = -1.0f;
    ray_view.w = 0.0f;

    vec4_t ray_world = m4_mul_v4(inv_view, ray_view);


    result.o = camera->position;
    result.d = (vec3_t){{ray_world.x, -ray_world.y, ray_world.z}};
    result.d = v3_normalized(result.d);

    return result;
}

vec3_t
ray_intersect_xz(vec3_t origin,
                 vec3_t direction) {
    float t = -origin.y / direction.y;
    vec3_t intersection = v3_add(origin, v3_scale(direction, t));
    return intersection;
}

void
assets_load(const char* path) {
    asset_data_size = x_file_size(path);

    asset_data = x_alloc(asset_data_size, 0);
    asset_data = x_file_read(path, asset_data, asset_data_size);

    if (!asset_blob_init(&asset_blob, asset_data, (uint32_t)asset_data_size)) {
        printf("Failed to initialize asset blob\n");
        return;
    }

    if (!asset_blob_load(&asset_blob)) {
        printf("Failed to load asset chunks\n");
        return;
    }

    printf("Successfully loaded %u textures and %u meshes\n",
            asset_blob.texture_count, asset_blob.mesh_count);

    asset_mesh_t* asset_mesh = asset_get_mesh(&asset_blob, 0);
    game_state.ship_mesh = asset_mesh_to_mesh(asset_mesh);
    game_state.ship_mesh.transform = mat4_identity;
    game_state.ship_mesh.texture = asset_get_texture(&asset_blob, 0);
}

/* ============= INIT =============== */
Game*
g_init(const char* path) {
    game.target_fps = 60;
    game.width = SCREEN_WIDTH;
    game.height = SCREEN_HEIGHT;

    ASPECT_RATIO = (float) game.height / game.width;
    WINDOW_ORIGIN = (vec2_t) {{ (float) game.width / 2, (float) game.height / 2 }};

    float aspect_x = (float)game.width / (float)game.height;
    float fov_x = atan(tan(current_camera->fov / 2) * aspect_x) * 2;

    init_planes(fov_x, current_camera->fov, current_camera->near, current_camera->far);

    framegraph_size_set(SCREEN_WIDTH, SCREEN_HEIGHT);

    assets_load(path);

    game_state.mesh_queue = nude_mesh_queue_create();

    return &game;
}

void
g_term() {
    da_destroy(game_state.ship_mesh.vertices);
    da_destroy(game_state.ship_mesh.indices);
    da_destroy(game_state.ship_mesh.normals);
    da_destroy(game_state.ship_mesh.colors);
    da_destroy(game_state.ship_mesh.uvs);
    da_destroy(game_state.ship_mesh.uv_indices);

    if (asset_data) {
        x_free(asset_data, 0);
        asset_data = NULL;
    }

    nude_mesh_queue_destroy(game_state.mesh_queue);
}

int activated = 0;

void
g_input(float dt) {
    if (input_get_key(KEY_CODE_SHIFT)) {
        if (input_get_button_down(BUTTON_MOUSE_LEFT)) {
            activated = 1;
            input_get_mouse_pos(&mx, &my);
            x_window_position_get(&wx, &wy);
        }
        if (activated && input_get_button(BUTTON_MOUSE_LEFT)) {
            printf("SHIFT");
            int x, y, dx, dy;
            activated = 0;
            input_get_mouse_pos(&x, &y);
            dx = x - mx;
            dy = y - my;
            x_window_position_set(wx + dx, wy + dy);
        }
    } else { activated = 0; }

    if (input_get_key_down(KEY_CODE_O)) {
        TIME_SCALE *= 0.5;
        TIME_SCALE = MAX(0, TIME_SCALE);
        x_time_scale_set(TIME_SCALE);
    }

    if (input_get_key_down(KEY_CODE_P)) {
        TIME_SCALE *= 2;
        TIME_SCALE = MIN(TIME_SCALE, 100);
        x_time_scale_set(TIME_SCALE);
    }

    if(input_get_key(KEY_CODE_ESC)) {
        b_event_dispatch(APP_QUIT);
    }

#ifdef ENABLE_SREC
    if (TIME_ON_FRAME > 3.0) {
        b_event_dispatch(APP_QUIT);
    }
#endif

    if (input_get_key_down(KEY_CODE_1)) {
        n_flag_toggle(DRAW_FLAG_SHADE);
    }

    if (input_get_key_down(KEY_CODE_2)) {
        n_flag_toggle(DRAW_FLAG_WIREFRAME);
    }

    if (input_get_key_down(KEY_CODE_3)) {
        n_flag_toggle(DRAW_FLAG_DOT);
    }

    if (input_get_key_down(KEY_CODE_4)) {
        n_flag_toggle(DRAW_FLAG_CULLING);
    }

    if (input_get_key_down(KEY_CODE_5)) {
        n_flag_toggle(DRAW_FLAG_BACKFACE);
    }

    if (input_get_key_down(KEY_CODE_6)) {
        n_flag_toggle(DRAW_FLAG_NORMALS);
    }

    if (input_get_key_down(KEY_CODE_7)) {
        n_flag_toggle(DRAW_FLAG_FULLRECT);
    }

    if (input_get_button(BUTTON_MOUSE_RIGHT)) {
        int x, y;
        input_get_mouse_pos(&x, &y);
        ray = ray_from_mouse(x, y, current_camera);
        vec3_t intersection = ray_intersect_xz(ray.o, ray.d);
        global_mesh_pos = intersection;
    }
}

void
draw_buttons() {
    if (input_get_key(KEY_CODE_W)) {
        n_text_draw(game.color, 100, 100, "W", 0xffffffff, 2, 2);
    }

    if (input_get_key(KEY_CODE_S)) {
        n_text_draw(game.color, 100, 120, "S", 0xffffffff, 2, 2);
    }

    if (input_get_key(KEY_CODE_A)) {
        n_text_draw(game.color, 80, 120, "A", 0xffffffff, 2, 2);
    }

    if (input_get_key(KEY_CODE_D)) {
        n_text_draw(game.color, 120, 120, "D", 0xffffffff, 2, 2);
    }
}

void
g_update(float dt) {
    double start_frame = x_get_absolute_time();
    TIME_ON_FRAME = x_get_time();

    g_input(dt);

    camera_update(current_camera, dt);

    game_state.ship_mesh.transform = mat4_identity;

    rotor3_t rot = r3_from_euler(0, 0, 0);
    mat4_t rot_mat = r3_rotate(rot);
    game_state.ship_mesh.transform = m4_translate(global_mesh_pos.x, global_mesh_pos.y, global_mesh_pos.z);

    framegraph_store(0, draw_time, x_time_frame_get());
    float frame_ratio = get_frame_avg_ratio();
    int pos = last_frame_pos(frame_ratio);

    /* END OF THE UPDATE */
    update_time = x_get_absolute_time() - start_frame;

    n_clear_color_set(0xff000000);
    n_size_set(game.width, game.height);
    n_clear(game.color, game.depth);

    n_grid_dot_draw(game.color, game.width, game.height, 10, TIME_ON_FRAME);

    nude_mesh_queue_clear(game_state.mesh_queue);

    nude_mesh_queue_add(game_state.mesh_queue, game_state.ship_mesh, camera_view(current_camera), camera_projection(current_camera, ASPECT_RATIO));

    nude_render(game_state.mesh_queue, game.color, game.depth, game.width, game.height);

    n_ctx_view_set(camera_view(current_camera));
    n_ctx_proj_set(camera_projection(current_camera, ASPECT_RATIO));
    n_draw_ray(game.color, ray.o, ray.d, 0x00ff00ff);

    if (input_get_key(KEY_CODE_SHIFT)) {
        /* FRAME STATISTICS */
        const char* text = format_text("frame: %.1fms", x_time_frame_get() * 1000);
        int tx, ty;
        n_text_size(text, 2, 2, &tx, &ty);
        n_text_draw(game.color, WINDOW_ORIGIN.x - tx * 0.5, ty, text, 0xffffffff, 2, 2);

        framegraph_draw(game.color, 1, 0xff00ff00);

        text = format_text("draw: %.4fms", (x_time_frame_get() * frame_ratio) * 1000);
        n_text_size(text, 2, 2, &tx, &ty);
        n_text_draw(game.color, SCREEN_WIDTH - tx - 10, ty / 2 + pos, text, 0xffffffff, 2, 2);
    }

    draw_buttons();

    /* END OF DRAW */
    draw_time = x_get_absolute_time() - start_frame;
}
