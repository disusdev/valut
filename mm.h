#ifndef __MM_H__
#define __MM_H__

#include <stdint.h>

#if defined(_MSC_VER)
#define INLINE __inline
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE
#endif

#define TAU 6.28318530717958647692f
#define DEG2RAD (TAU / 360.0f)
#define RAD2DEG (360.0f / TAU)

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

float mm_modf(float, float);
float mm_rsqrt(float);
float mm_sqrt(float);
float mm_wrapf(float);
float mm_sin(float);
float mm_cos(float);
float mm_acos(float);
float mm_tan(float);
float mm_floor(float);
float mm_abs(float);

static INLINE float lerp(float from, float to, float t) {
    return from + t * (to - from);
}

static INLINE float
triangle_area2(float x1, float y1, float x2, float y2, float x3, float y3) {
    return (x1 * y2 - x2 * y1) + (x2 * y3 - x3 * y2) + (x3 * y1 - x1 * y3);
}

typedef union {
    float data[2];

    struct {
        float x, y;
    };
} vec2_t;

typedef union {
    float data[3];

    struct {
        float x, y, z;
    };
} vec3_t;

typedef union {
    float data[4];

    struct {
        float x, y, z, w;
    };

    struct {
        float rows[4];
    };

    struct {
        float s, xy, yz, zx;
    };
} vec4_t;

typedef vec4_t rotor3_t;

enum {
    BASIS_RIGHT = 0,
    BASIS_UP,
    BASIS_FORWARD,
    BASIS_POSITION
};

typedef struct {
    vec4_t cols[4];
} mat4_t;

#define vec2_zero (vec2_t){{ 0, 0 }}
#define vec2_one (vec2_t){{ 1, 1 }}
#define vec2_left (vec2_t){{ -1, 0 }}
#define vec2_right (vec2_t){{ 1, 0 }}
#define vec2_up (vec2_t){{ 0, 1 }}
#define vec2_down (vec2_t){{ 0, -1 }}

#define vec3_zero (vec3_t){{ 0, 0, 0 }}
#define vec3_one (vec3_t){{ 1, 1, 1 }}
#define vec3_left (vec3_t){{ -1, 0, 0 }}
#define vec3_right (vec3_t){{ 1, 0, 0 }}
#define vec3_up (vec3_t){{ 0, 1, 0 }}
#define vec3_down (vec3_t){{ 0, -1, 0 }}
#define vec3_forward (vec3_t){{ 0, 0, -1 }}
#define vec3_back (vec3_t){{ 0, 0, 1 }}

#define mat4_identity (mat4_t){{\
    (vec4_t){{ 1, 0, 0, 0 }},\
    (vec4_t){{ 0, 1, 0, 0 }},\
    (vec4_t){{ 0, 0, 1, 0 }},\
    (vec4_t){{ 0, 0, 0, 1 }}\
}}

uint32_t mm_rng(int32_t pos, uint32_t seed);
int32_t mm_rng_range(int32_t min, int32_t max, int32_t pos, uint32_t seed);
uint32_t mm_rng_noise2d(int posX, int posY, uint32_t seed);
uint32_t mm_rng_noise3d(int posX, int posY, int posZ, uint32_t seed);
uint32_t mm_rng_rand(uint32_t seed);

float mm_rng_float01(int32_t pos, uint32_t seed);
float mm_rng_float11(int32_t pos, uint32_t seed);
float mm_rng_rangef(float min, float max, int32_t pos, uint32_t seed);
float mm_rng_noise2d_f(int posX, int posY, uint32_t seed);
float mm_rng_noise3d_f(int posX, int posY, int posZ, uint32_t seed);
float mm_noise2d(float x, float y, uint32_t seed);
float mm_noise3d(float x, float y, float z, uint32_t seed);

static INLINE vec2_t v2_add(vec2_t a, vec2_t b) {
    return (vec2_t) {{ a.x + b.x, a.y + b.y }};
}

static INLINE vec2_t v2_sub(vec2_t a, vec2_t b) {
    return (vec2_t) {{ a.x - b.x, a.y - b.y }};
}

static INLINE vec2_t v2_scl(vec2_t a, float sx, float sy) {
    return (vec2_t) {{ a.x *  sx, a.y * sy }};
}

static INLINE float v2_dot(vec2_t a, vec2_t b) {
    return a.x * b.x + a.y * b.y;
}

static INLINE float v2_cross(vec2_t a, vec2_t b) {
    return a.x * b.y - a.y * b.x;
}

static INLINE float v2_sqr_length(vec2_t a) {
    return v2_dot(a, a);
}

static INLINE float v2_length(vec2_t a) {
    return mm_sqrt(v2_sqr_length(a));
}

static INLINE vec2_t v2_normalized(vec2_t a) {
    const float len = v2_length(a);
    return (vec2_t) {{ a.x / len, a.y / len }};
}

static INLINE float v2_dst(vec2_t a, vec2_t b) {
    return v2_length(v2_sub(a, b));
}

static INLINE vec2_t v2_lerp(vec2_t a, vec2_t b, float t) {
    return (vec2_t) {{
        lerp(a.x, b.x, t),
        lerp(a.y, b.y, t)
    }};
}

static INLINE vec3_t v3_add(vec3_t a, vec3_t b) {
    return (vec3_t) {{ a.x + b.x, a.y + b.y, a.z + b.z }};
}

static INLINE vec3_t v3_sub(vec3_t a, vec3_t b) {
    return (vec3_t) {{ a.x - b.x, a.y - b.y, a.z - b.z }};
}

static INLINE vec3_t v3_scl(vec3_t a, float sx, float sy, float sz) {
    return (vec3_t) {{ a.x * sx, a.y * sy, a.z * sz }};
}

static INLINE float v3_dot(vec3_t a, vec3_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static INLINE vec3_t v3_cross(vec3_t a, vec3_t b) {
    return (vec3_t) {{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    }};
}

static INLINE float v3_sqr_length(vec3_t a) {
    return v3_dot(a, a);
}

static INLINE float v3_length(vec3_t a) {
    return mm_sqrt(v3_sqr_length(a));
}

static INLINE vec3_t v3_normalized(vec3_t a) {
    const float len = v3_length(a);
    return (vec3_t) {{
        a.x / len,
        a.y / len,
        a.z / len
    }};
}

static INLINE float v3_dst(vec3_t a, vec3_t b) {
    return v3_length(v3_sub(a, b));
}

static INLINE vec3_t v3_scale(vec3_t a, float s) {
    return (vec3_t) {{ a.x * s, a.y * s, a.z * s }};
}

static INLINE vec4_t v4_add(vec4_t a, vec4_t b) {
    return (vec4_t) {{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }};
}

static INLINE vec4_t v4_sub(vec4_t a, vec4_t b) {
    return (vec4_t) {{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }};
}

static INLINE vec4_t v4_scl(vec4_t a, float sx, float sy,
                               float sz, float sw) {
    return (vec4_t) {{ a.x * sx, a.y * sy, a.z * sz, a.w * sw }};
}

static INLINE float v4_dot(vec4_t a, vec4_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

static INLINE float v4_sqr_length(vec4_t a) {
    return v4_dot(a, a);
}

static INLINE float v4_length(vec4_t a) {
    return mm_sqrt(v4_sqr_length(a));
}

static INLINE vec4_t v4_normalized(vec4_t a) {
    const float len = v4_length(a);
    return (vec4_t) {{
        a.x / len,
        a.y / len,
        a.z / len,
        a.w / len
    }};
}

static INLINE float v4_dst(vec4_t a, vec4_t b) {
    return v4_length(v4_sub(a, b));
}

static INLINE float m4_get(mat4_t a, int row, int col) {
    return a.cols[col].rows[row];
}

static INLINE vec4_t m4_mul_v4(mat4_t m, vec4_t v) {
    vec4_t result;
    result.x = m.cols[0].x * v.x + m.cols[1].x * v.y + m.cols[2].x * v.z + m.cols[3].x * v.w;
    result.y = m.cols[0].y * v.x + m.cols[1].y * v.y + m.cols[2].y * v.z + m.cols[3].y * v.w;
    result.z = m.cols[0].z * v.x + m.cols[1].z * v.y + m.cols[2].z * v.z + m.cols[3].z * v.w;
    result.w = m.cols[0].w * v.x + m.cols[1].w * v.y + m.cols[2].w * v.z + m.cols[3].w * v.w;
    return result;
}


static INLINE mat4_t m4_mul(mat4_t a, mat4_t b) {
    return (mat4_t) {
        m4_mul_v4(a, b.cols[0]),
        m4_mul_v4(a, b.cols[1]),
        m4_mul_v4(a, b.cols[2]),
        m4_mul_v4(a, b.cols[3])
    };
}

static INLINE mat4_t m4_ortho_bounds(float left, float right,
                              float bottom, float top,
                              float near, float far) {
    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;
    return (mat4_t) {
        (vec4_t) {{ 2.0f / rl, 0, 0, 0 }},
        (vec4_t) {{ 0, 2.0f / tb, 0, 0 }},
        (vec4_t) {{ 0, 0, -2.0f / fn, 0 }},
        (vec4_t) {{ -(right + left) / rl,
                    -(top + bottom) / tb,
                    -(near + far) / fn,
                    1.0f
        }}
    };
}

static INLINE mat4_t m4_ortho(float size, float aspect, float near, float far) {
    float rl = size * aspect;
    float tb = size;
    float fn = far - near;
    return (mat4_t) {
        (vec4_t) {{ 2.0f / rl, 0, 0, 0 }},
        (vec4_t) {{ 0, 2.0f / tb, 0, 0 }},
        (vec4_t) {{ 0, 0, -2.0f / fn, 0 }},
        (vec4_t) {{ -(rl / 2.0f), -(tb / 2.0f), -(near + far) / fn, 1.0f }}
    };
}

static INLINE mat4_t m4_persp(float fovy, float aspect,
                              float near, float far) {
    float f = 1.0f / mm_tan(fovy * 0.5f);
    float nf = 1.0f / (near - far);
    return (mat4_t) {
        (vec4_t) {{ f / aspect, 0, 0, 0 }},
        (vec4_t) {{ 0, f, 0, 0 }},
        (vec4_t) {{ 0, 0, far * nf, 1 }},
        (vec4_t) {{ 0, 0, -(far * near) * nf, 0 }},
    };
}

static INLINE mat4_t m4_translate(float x, float y, float z) {
    mat4_t res = mat4_identity;
    res.cols[3].x = x;
    res.cols[3].y = y;
    res.cols[3].z = z;
    return res;
}

static INLINE mat4_t m4_scale(float x, float y, float z) {
    mat4_t res = mat4_identity;
    res.cols[0].x = x;
    res.cols[1].y = y;
    res.cols[2].z = z;
    return res;
}

static INLINE mat4_t m4_look_at(vec3_t eye, vec3_t target, vec3_t up) {
    vec3_t f = v3_normalized(v3_sub(target, eye));
    vec3_t r = v3_normalized(v3_cross(up, f));
    vec3_t u = v3_cross(f, r);

    mat4_t res = {{
        {{ r.x, u.x, f.x, 0 }},
        {{ r.y, u.y, f.y, 0 }},
        {{ r.z, u.z, f.z, 0 }},
        {{ -v3_dot(r, eye), -v3_dot(u, eye), -v3_dot(f, eye), 1.0f }}
    }};
    return res;
}

static INLINE rotor3_t r3_from_to(vec3_t from, vec3_t to) {
    from = v3_normalized(from);
    to = v3_normalized(to);

    vec3_t half = v3_add(from, to);
    half = v3_normalized(half);

    vec3_t wedge = {{
        half.x * from.y - half.y * from.x,
        half.y * from.z - half.z * from.y,
        half.z * from.x - half.x * from.z
    }};

    rotor3_t r = {{
        v3_dot(from, half),
        wedge.x,
        wedge.y,
        wedge.z
    }};
    return r;
}

static INLINE rotor3_t r3_combine(rotor3_t r1, rotor3_t r2) {
    rotor3_t r = {{
        r1.s * r2.s - r1.xy * r2.xy - r1.yz * r2.yz - r1.zx * r2.zx,
        r1.s * r2.xy - r1.xy * r2.s - r1.yz * r2.zx - r1.zx * r2.yz,
        r1.s * r2.yz - r1.xy * r2.zx - r1.yz * r2.s - r1.zx * r2.xy,
        r1.s * r2.zx - r1.xy * r2.yz - r1.yz * r2.xy - r1.zx * r2.s,
    }};
    return r;
}

static INLINE rotor3_t r3_reverse(rotor3_t r) {
    return (rotor3_t) {{
        r.s,
        -r.xy,
        -r.yz,
        -r.zx
    }};
}

static INLINE rotor3_t r3_from_euler(float yaw, float pitch, float roll) {
    float hy = yaw * 0.5f;
    float hp = pitch * 0.5f;
    float hr = roll * 0.5f;

    float cy = mm_cos(hy), sy = mm_sin(hy);
    float cp = mm_cos(hp), sp = mm_sin(hp);
    float cr = mm_cos(hr), sr = mm_sin(hr);

    rotor3_t r = {{
        (cr * cp) * cy + (sr * sp) * sy,
        (cr * sp) * cy + (sr * cp) * sy,
        (sr * cp) * cy - (cr * sp) * sy,
        (cr * cp) * sy - (sr * sp) * cy
    }};

    return r;
}

static INLINE vec3_t v3_transform_r(rotor3_t r, vec3_t v) {
    vec3_t res = v;
    float sx = r.s * v.x + r.xy * v.y - r.zx * v.z;
    float sy = r.s * v.y - r.xy * v.x + r.yz * v.z;
    float sz = r.s * v.z - r.yz * v.y + r.zx * v.x;
    float sxyz = r.xy * v.z + r.yz * v.x + r.zx * v.y;
    res.x = sx * r.s + sy * r.xy + sxyz * r.yz - sz * r.zx;
    res.y = sy * r.s - sx * r.xy + sz * r.yz + sxyz * r.zx;
    res.z = sz * r.s + sxyz * r.xy - sy * r.yz + sx * r.zx;
    return res;
}

static INLINE mat4_t r3_rotate(rotor3_t r) {
    vec3_t basis_x = {{1, 0, 0}};
    vec3_t basis_y = {{0, 1, 0}};
    vec3_t basis_z = {{0, 0, 1}};

    vec3_t nx = v3_transform_r(r, basis_x);
    vec3_t ny = v3_transform_r(r, basis_y);
    vec3_t nz = v3_transform_r(r, basis_z);
    mat4_t res = {{
        {{ nx.x, nx.y, nx.z, 0 }},
        {{ ny.x, ny.y, ny.z, 0 }},
        {{ nz.x, nz.y, nz.z, 0 }},
        {{ 0, 0, 0, 1 }}
    }};
    return res;
}

static INLINE rotor3_t r3_nlerp(rotor3_t from, rotor3_t to, float t) {
    float dot = from.s * to.s + from.xy * to.xy + from.yz * to.yz + from.zx * to.zx;
    if (dot < 0) {
        to.s = -to.s;
        to.xy = -to.xy;
        to.yz = -to.yz;
        to.zx = -to.zx;
    }

    rotor3_t res = {{
        lerp(from.s, to.s, t),
        lerp(from.xy, to.xy, t),
        lerp(from.yz, to.yz, t),
        lerp(from.zx, to.zx, t)
    }};

    float mag = mm_sqrt(res.s * res.s + res.xy * res.xy + res.yz * res.yz + res.zx * res.zx);
    res.s = res.s / mag;
    res.xy = res.xy / mag;
    res.yz = res.yz / mag;
    res.zx = res.zx / mag;
    return res;
}

static INLINE void r3_get_axis(rotor3_t r, vec3_t* forward, vec3_t* right, vec3_t* up) {
    *forward = v3_transform_r(r, vec3_forward);
    *right = v3_transform_r(r, vec3_right);
    *up = v3_transform_r(r, vec3_up);
}

#endif
