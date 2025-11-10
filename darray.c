#include "darray.h"
#include <stddef.h>
#include <stdlib.h>

darray_t* da_create(size_t stride) {
    darray_t* arr = (darray_t*)malloc(sizeof(darray_t));
    if (!arr) return NULL;
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
    arr->stride = stride;
    return arr;
}

void da_destroy(darray_t* arr) {
    if (!arr) return;
    if (arr->data) free(arr->data);
    free(arr);
}

int da_reserve(darray_t* da, size_t cap) {
    if (!da) return 0;
    if (cap <= da->capacity) return 1;
    void* new_data = realloc(da->data, cap * da->stride);
    if (!new_data) return 0;
    da->data = new_data;
    da->capacity = cap;
    return 1;
}

void
da_resize(darray_t* da, size_t count) {
    if (count >= da->capacity) {
        da_reserve(da, count);
    }
    da->count = count;
}

int da_add(darray_t* da, const void* elem) {
    if (!da || !elem) return 0;
    if (da->count == da->capacity) {
        size_t new_cap = da->capacity ? da->capacity * 2 : 4;
        if (!da_reserve(da, new_cap)) return 0;
    }
    char* dest = (char*)da->data + da->count * da->stride;
    const char* src = (const char*)elem;
    size_t i;
    for (i = 0; i < da->stride; ++i) {
        dest[i] = src[i];
    }
    da->count += 1;
    return 1;
}

void* da_get(darray_t* da, size_t idx) {
    if (!da || idx >= da->count) return NULL;
    return (char*)da->data + idx * da->stride;
}

int da_set(darray_t* da, size_t idx, const void* elem) {
    if (!da || !elem || idx >= da->count) return 0;
    char* dest = (char*)da->data + idx * da->stride;
    const char* src = (const char*)elem;
    size_t i;
    for (i = 0; i < da->stride; ++i) {
        dest[i] = src[i];
    }
    return 1;
}

size_t da_size(const darray_t* da) {
    if (!da) return 0;
    return da->count;
}

void da_clear(darray_t* da) {
    if (!da) return;
    da->count = 0;
}
