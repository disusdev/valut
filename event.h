#ifndef __EVENT_H__
#define __EVENT_H__

#include <stdint.h>

typedef enum {
#define EVENT_KIND(name) name,
#include "event_kinds.inc"
#undef EVENT_KIND
  EVENT_KIND_COUNT
} event_kind_t;

typedef struct {
  event_kind_t kind;
  
  union
  {
    uint64_t u64[2];
    int64_t i64[2];
    double f64[2];
    uint32_t u32[4];
    int32_t i32[4];
    float f32[4];
    uint16_t u16[8];
    int16_t i16[8];
    uint8_t u8[16];
    int8_t i8[16];
    char c[16];
  } ctx;
} event_t;

typedef void (*event_func_t)(event_t* event);

typedef struct {
  event_kind_t kind;
  event_func_t func;
} event_data_t;

void b_event_clear();
int b_event_register(event_kind_t, event_func_t);
event_t* b_event_handle_get();
void b_event_dispatch(event_kind_t);
void b_event_dispatch_ext(event_kind_t, event_t*);

#endif