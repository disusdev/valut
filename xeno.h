#ifndef __XENO_H__
#define __XENO_H__

#include <stdint.h>

void
platform_init(const char* app_name,
              int32_t x,
              int32_t y,
              int32_t width,
              int32_t height);

void
platform_audio_init(int sampleRate,
                    int channels);

int
platform_screenshot_save(const char* path);

void
platform_term();

void
platform_pump_msg();

double
platform_get_time();

void*
platform_alloc(uint64_t size,
               uint8_t aligned);

void
platform_free(void* block,
              uint8_t aligned);

void*
platform_mem_zero(void* block,
                  uint64_t size);

void*
platform_mem_copy(void* dst,
                  const void* src,
                  uint64_t size);

void*
platform_mem_set(void* dst,
                 int32_t value,
                 uint64_t size);

double
platform_get_absolute_time();

uint64_t
platform_raw_time_get();

void
platform_window_position_get(int* x, int* y);

void
platform_window_position_set(int x, int y);

void
platform_time_scale_set(float scale);

float
platform_time_frame_get();

void
platform_console_write(const char* msg,
                       uint32_t msg_length,
                       uint8_t color_index);

#endif // __XENO_H__
