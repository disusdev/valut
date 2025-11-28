#include "input.h"
#include "event.h"
#include <stdint.h>

#define BIT(x_) (1llu << (x_))

#define KEYS_COUNT ((KEY_CODE_COUNT + 63) >> 6)

static struct {
    uint64_t keys_down[KEYS_COUNT];
    uint64_t keys_up[KEYS_COUNT];
    uint64_t keys_hold[KEYS_COUNT];
    uint64_t keys_prev_hold[KEYS_COUNT];

    uint64_t buttons_down[KEYS_COUNT];
    uint64_t buttons_up[KEYS_COUNT];
    uint64_t buttons_hold[KEYS_COUNT];
    uint64_t buttons_prev_hold[KEYS_COUNT];

    uint8_t char_trigger;

    int32_t mouse_x;
    int32_t mouse_y;
} ctx = {0};

void input_register_char(event_t* e);
void input_register_mouse_pos(event_t* e);
void input_register_key_down(event_t* e);
void input_register_key_up(event_t* e);
void input_register_button_down(event_t* e);
void input_register_button_up(event_t* e);

void
input_init()
{
    b_event_register(INPUT_CHAR, input_register_char);
    b_event_register(INPUT_MOUSE_MOVE, input_register_mouse_pos);
    b_event_register(INPUT_KEY_DOWN, input_register_key_down);
    b_event_register(INPUT_KEY_UP, input_register_key_up);
    b_event_register(INPUT_BUTTON_DOWN, input_register_button_down);
    b_event_register(INPUT_BUTTON_UP, input_register_button_up);
}

void
input_pre_update()
{
    ctx.char_trigger = 0;
}

void
input_update()
{
    int count = KEYS_COUNT;
    int k;
    for (k = 0; k < count; k += 1)
    {
        ctx.keys_down[k] = (ctx.keys_hold[k] ^ ctx.keys_prev_hold[k]) & ctx.keys_hold[k];
        ctx.keys_up[k] = (ctx.keys_hold[k] ^ ctx.keys_prev_hold[k]) & ctx.keys_prev_hold[k];
        ctx.keys_prev_hold[k] = ctx.keys_hold[k];

        ctx.buttons_down[k] = (ctx.buttons_hold[k] ^ ctx.buttons_prev_hold[k]) & ctx.buttons_hold[k];
        ctx.buttons_up[k] = (ctx.buttons_hold[k] ^ ctx.buttons_prev_hold[k]) & ctx.buttons_prev_hold[k];
        ctx.buttons_prev_hold[k] = ctx.buttons_hold[k];
    }

    x_mouse_position_get(&ctx.mouse_x, &ctx.mouse_y);
}

uint64_t
input_keys_get(int idx) {
    return ctx.keys_hold[idx];
}

void
input_register_char(event_t* e)
{
    ctx.char_trigger = e->ctx.u8[0];
}

void
input_register_mouse_pos(event_t* e)
{
    ctx.mouse_x = e->ctx.i32[0];
    ctx.mouse_y = e->ctx.i32[1];
}

void
input_register_key_down(event_t* e)
{
    uint32_t key = e->ctx.u32[0];
    ctx.keys_hold[key >> 6] |= BIT(key & 63);
}

void
input_register_key_up(event_t* e)
{
    uint32_t key = e->ctx.u32[0];
    ctx.keys_hold[key >> 6] &= ~BIT(key & 63);
}

void
input_register_button_down(event_t* e)
{
    uint32_t button = e->ctx.u32[0];
    ctx.buttons_hold[button >> 6] |= BIT(button & 63);
}

void
input_register_button_up(event_t* e)
{
    uint32_t button = e->ctx.u32[0];
    ctx.buttons_hold[button >> 6] &= ~BIT(button & 63);
}

int
input_get_key_down(key_code_t key)
{
    uint64_t key_flag = BIT(key & 63);
    return (ctx.keys_down[key >> 6] & key_flag) == key_flag;
}

int
input_get_key_up(key_code_t key)
{
    uint64_t key_flag = BIT(key & 63);
    return (ctx.keys_up[key >> 6] & key_flag) == key_flag;
}

int
input_get_key(key_code_t key)
{
    uint64_t key_flag = BIT(key & 63);
    return (ctx.keys_hold[key >> 6] & key_flag) == key_flag;
}

int
input_get_button_down(button_code_t button)
{
    uint64_t button_flag = BIT(button & 63);
    return (ctx.buttons_down[button >> 6] & button_flag) == button_flag;
}

int
input_get_button_up(button_code_t button)
{
    uint64_t button_flag = BIT(button & 63);
    return (ctx.buttons_up[button >> 6] & button_flag) == button_flag;
}

int
input_get_button(button_code_t button)
{
    uint64_t button_flag = BIT(button & 63);
    return (ctx.buttons_hold[button >> 6] &button_flag) == button_flag;
}

void
input_get_mouse_pos(int32_t* out_x,
                    int32_t* out_y)
{
    *out_x = ctx.mouse_x;
    *out_y = ctx.mouse_y;
}

int
input_get_char(char* ch)
{
    *ch = ctx.char_trigger;
    return ctx.char_trigger != 0;
}
