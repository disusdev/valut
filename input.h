#ifndef __INPUT_H__
#define __INPUT_H__

#include <stdint.h>

typedef enum {
#include "input_keys.inc"
    KEY_CODE_COUNT = 73
} key_code_t;

typedef enum {
    BUTTON_MOUSE_LEFT = 0,
    BUTTON_MOUSE_RIGHT = 1,
} button_code_t;

void input_init();
void input_pre_update();
void input_update();

int input_get_key_down(key_code_t key);
int input_get_key_up(key_code_t key);
int input_get_key(key_code_t key);
int input_get_button_down(button_code_t button);
int input_get_button_up(button_code_t button);
int input_get_button(button_code_t button);
void input_get_mouse_pos(int32_t* out_x, int32_t* out_y);
int input_get_char(char* ch);
void platform_mouse_position_get(int32_t* out_x, int32_t* out_y);
uint64_t input_keys_get(int idx);

#endif
