#include "mm.h"

#define LUT_SIZE 4096
#define LUT_MASK (LUT_SIZE - 1)
#define TAU_25 (TAU * 0.25f)
#define TAU_50 (TAU * 0.5f)

extern const float* mm_get_sin_lut();
extern const float* mm_get_cos_lut();
extern const float* mm_get_sqrt_lut();

static const float* get_sin_lut() { return mm_get_sin_lut(); }
static const float* get_cos_lut() { return mm_get_cos_lut(); }
static const float* get_sqrt_lut() { return mm_get_sqrt_lut(); }

static float fast_sqrt_approx(float x) {
    union { float f; unsigned int i; } u = {x};
    u.i = 0x5f3759df - (u.i >> 1);
    return x * u.f * (1.5f - 0.5f * x * u.f * u.f);
}

static int fast_mod(int x, int mask) {
    return x & mask;
}

static float normalize_angle(float x) {
    while (x < 0.0f) x += TAU;
    while (x >= TAU) x -= TAU;
    return x;
}

static const float pi = 3.14159265358979323846f;
static const float tau = pi * 2.0f;

float
mm_modf(float x, float y) {
    float v = x / y;
    float i = (float)(long)v;
    return v - i;
}

float
mm_rsqrt(float v) {
    long i;
    float x2, y;
    const float hl3 = 1.5f;

    x2 = v * 0.5f;
    y = v;
    i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(long*)&i;
    y = y * (hl3 - (x2 * y * y));
    /* y = y * (hl3 - (x2 * y * y)); */

    return y;
}

float
mm_sqrt(float v) {
    return fast_sqrt_approx(v);
}

float
mm_wrapf(float v) {
#if 0
    float i;
    float frac = v / tau;
    i = (float)(long)frac;
    frac = frac - i;
    if (frac < 0.0f) frac += 1.0f;
    return frac * tau;
#else
    v = mm_modf(v, tau);
    if (v > pi) v -= tau;
    if (v < -pi) v += tau;
    return v;
#endif
}

float mm_cos(float x) {
    int index = (int)(normalize_angle(x) * LUT_SIZE / TAU) & LUT_MASK;
    return get_cos_lut()[index];
}

float mm_sin(float x) {
    int index = (int)(normalize_angle(x) * LUT_SIZE / TAU) & LUT_MASK;
    return get_sin_lut()[index];
}

float mm_tan(float x) {
    float cos_x = mm_cos(x);
    if (cos_x == 0.0f) return 0.0f;
    return mm_sin(x) / cos_x;
}

float mm_atan2(float y, float x) {
    if (x == 0.0f) {
        return (y > 0.0f) ? TAU_25 : -TAU_25;
    }

    float abs_y = (y < 0.0f) ? -y : y;
    float abs_x = (x < 0.0f) ? -x : x;

    float ratio = abs_y / abs_x;
    float angle = 0.0f;

    if (ratio < 1.0f) {
        angle = ratio * (1.0f - ratio * ratio * 0.33f);
    } else {
        angle = TAU_25 - (1.0f / ratio) * (1.0f - (1.0f / ratio) * (1.0f / ratio) * 0.33f);
    }

    if (x < 0.0f) angle = TAU_50 - angle;
    if (y < 0.0f) angle = -angle;

    return angle;
}

float mm_hypot(float x, float y) {
    float abs_x = (x < 0.0f) ? -x : x;
    float abs_y = (y < 0.0f) ? -y : y;

    if (abs_x > abs_y) {
        return abs_x * mm_sqrt(1.0f + (abs_y / abs_x) * (abs_y / abs_x));
    } else {
        return abs_y * mm_sqrt(1.0f + (abs_x / abs_y) * (abs_x / abs_y));
    }
}

float mm_floor(float f) {
    if (f >= 0.0f) {
        return (float)((int)f);
    } else {
        int i = (int)f;
        return (float)(i - (f != (float)i));
    }
}

float mm_acos(float x) {
    float cmp = x;
    if (cmp < -1.0f) cmp = -1.0f;
    if (cmp > 1.0f) cmp = 1.0f;

    float abs_x = (cmp < 0.0f) ? -cmp : cmp;
    float result = TAU_25 - cmp * (1.0f + cmp * cmp * (0.16666667f + cmp * cmp * 0.075f));

    return (cmp < 0.0f) ? TAU_50 - result : result;
}

float mm_abs(float x) {
    union { float f; unsigned int i; } u = {x};
    u.i &= 0x7FFFFFFF;
    return u.f;
}

float mm_fmod(float x, float y) {
    if (y == 0.0f) return 0.0f;

    float q = x / y;
    int qi = (int)q;
    float qf = (float)qi;

    float r = x - qf * y;

    float ay = (y < 0.0f) ? -y : y;
    if (r >= ay) r -= ay;
    else if (r <= -ay) r += ay;

    if (r == 0.0f) r = (x < 0.0f) ? -0.0f : 0.0f;

    return r;
}

#define BIT_NOSIE1 0xB5297A4D;
#define BIT_NOSIE2 0x68E31DA4;
#define BIT_NOSIE3 0x1B56C4E9;

uint32_t
mm_rng(int32_t pos, uint32_t seed) {
    uint32_t mangled = pos;
    mangled *= BIT_NOSIE1;
    mangled += seed;
    mangled ^= (mangled >> 8);
    mangled += BIT_NOSIE2;
    mangled ^= (mangled << 8);
    mangled *= BIT_NOSIE3;
    mangled ^= (mangled >> 8);
    return mangled;
}

int32_t
mm_rng_range(int32_t min, int32_t max, int32_t pos, uint32_t seed) {
    int range = max - min + 1;
    return mm_rng(pos, seed) % range + min;
}

uint32_t
mm_rng_noise2d(int posX, int posY, uint32_t seed) {
    int PRIME_NUMBER = 198491317;
    return mm_rng(posX + (PRIME_NUMBER * posY), seed);
}

uint32_t
mm_rng_noise3d(int posX, int posY, int posZ, uint32_t seed)
{
    int PRIME_NUMBER1 = 198491317;
    int PRIME_NUMBER2 = 6542989;
    return mm_rng(posX + (PRIME_NUMBER1 * posY)
        + (PRIME_NUMBER2 * posZ), seed);
}

uint32_t
mm_rng_rand(uint32_t seed)
{
    int PRIME_NUMBER = 198491317;
    return mm_rng(PRIME_NUMBER, seed);
}

float mm_rng_float01(int32_t pos, uint32_t seed) {
    return (float)mm_rng(pos, seed) / 4294967295.0f;
}

float mm_rng_float11(int32_t pos, uint32_t seed) {
    return ((float)mm_rng(pos, seed) / 2147483647.5f) - 1.0f;
}

float mm_rng_rangef(float min, float max, int32_t pos, uint32_t seed) {
    return min + (max - min) * mm_rng_float01(pos, seed);
}

float mm_rng_noise2d_f(int posX, int posY, uint32_t seed) {
    return (float)mm_rng_noise2d(posX, posY, seed) / 4294967295.0f;
}

float mm_rng_noise3d_f(int posX, int posY, int posZ, uint32_t seed) {
    return (float)mm_rng_noise3d(posX, posY, posZ, seed) / 4294967295.0f;
}

static float smoothstep(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float mm_noise2d(float x, float y, uint32_t seed) {
    int x0 = (int)mm_floor(x);
    int y0 = (int)mm_floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float sx = smoothstep(x - (float)x0);
    float sy = smoothstep(y - (float)y0);

    float n0 = mm_rng_noise2d_f(x0, y0, seed);
    float n1 = mm_rng_noise2d_f(x1, y0, seed);
    float ix0 = lerp(n0, n1, sx);

    float n2 = mm_rng_noise2d_f(x0, y1, seed);
    float n3 = mm_rng_noise2d_f(x1, y1, seed);
    float ix1 = lerp(n2, n3, sx);

    return lerp(ix0, ix1, sy);
}

float mm_noise3d(float x, float y, float z, uint32_t seed) {
    int x0 = (int)mm_floor(x);
    int y0 = (int)mm_floor(y);
    int z0 = (int)mm_floor(z);

    int x1 = x0 + 1;
    int y1 = y0 + 1;
    int z1 = z0 + 1;

    float sx = smoothstep(x - (float)x0);
    float sy = smoothstep(y - (float)y0);
    float sz = smoothstep(z - (float)z0);

    float n000 = mm_rng_noise3d_f(x0, y0, z0, seed);
    float n100 = mm_rng_noise3d_f(x1, y0, z0, seed);
    float n010 = mm_rng_noise3d_f(x0, y1, z0, seed);
    float n110 = mm_rng_noise3d_f(x1, y1, z0, seed);
    float n001 = mm_rng_noise3d_f(x0, y0, z1, seed);
    float n101 = mm_rng_noise3d_f(x1, y0, z1, seed);
    float n011 = mm_rng_noise3d_f(x0, y1, z1, seed);
    float n111 = mm_rng_noise3d_f(x1, y1, z1, seed);

    float ix00 = lerp(n000, n100, sx);
    float ix10 = lerp(n010, n110, sx);
    float ix01 = lerp(n001, n101, sx);
    float ix11 = lerp(n011, n111, sx);

    float iy0 = lerp(ix00, ix10, sy);
    float iy1 = lerp(ix01, ix11, sy);

    return lerp(iy0, iy1, sz);
}
