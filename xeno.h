#ifndef __XENO_H__
#define __XENO_H__

#include <stdint.h>

void x_init(const char* app_name, int32_t x, int32_t y, int32_t width, int32_t height);
void x_audio_init(int sampleRate, int channels);
int x_screenshot_save(const char* path);
void x_term();
void x_pump_msg();
double x_get_time();
void* x_alloc(uint64_t size, uint8_t aligned);
void x_free(void* block, uint8_t aligned);
void* x_realloc(void* block, uint64_t size, uint8_t aligned);
void* x_mem_zero(void* block, uint64_t size);
void* x_mem_copy(void* dst, const void* src, uint64_t size);
void* x_mem_set(void* dst, int32_t value, uint64_t size);
double x_get_absolute_time();
uint64_t x_raw_time_get();
void x_window_position_get(int* x, int* y);
void x_window_position_set(int x, int y);
void x_time_scale_set(float scale);
float x_time_frame_get();
void x_console_write(const char* msg, uint32_t msg_length, uint8_t color_index);
void x_mouse_position_get(int32_t* out_x, int32_t* out_y);
uint64_t x_file_size(const char* file_path);
uint8_t* x_file_read(const char* file_path, uint8_t* buffer, uint64_t size);

#endif
