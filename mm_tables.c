#include "mm.h"

static const float sin_lut[4096] = {
    #include "sin_table.inc"
};

static const float cos_lut[4096] = {
    #include "cos_table.inc"
};

static const float sqrt_lut[4096] = {
    #include "sqrt_table.inc"
};

const float* mm_get_sin_lut() { return sin_lut; }
const float* mm_get_cos_lut() { return cos_lut; }
const float* mm_get_sqrt_lut() { return sqrt_lut; }
