#ifndef __DARRAY_H__
#define __DARRAY_H__

typedef struct {
    void* data;
    size_t count;
    size_t capacity;
    size_t stride;
} darray_t;

darray_t* da_create(size_t stride);
void da_destroy(darray_t* arr);
int da_reserve(darray_t* da, size_t cap);
void da_resize(darray_t* da, size_t count);
int da_add(darray_t* da, const void* elem);
void* da_get(darray_t* da, size_t idx);
int da_set(darray_t* da, size_t idx, const void* elem);
size_t da_size(const darray_t* da);
void da_clear(darray_t* da);

#endif