#include "event.h"
#include "game.h"
#include "xeno.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <windows.h>
#include "event.h"
#include "input.h"


#include <mmsystem.h>
#include <winnt.h>
#include <winuser.h>
#pragma comment(lib, "winmm.lib")

// #define ENABLE_SREC

#define true 1
#define false 0

typedef struct system_state {
    HINSTANCE instance;
    HWND handle;
    BITMAPINFO bmi;
    Game* game;
    uint8_t is_running;

    double time_start;
    double time_passed;
    double time_scale;
    float time_frame;

    float window_x;
    float window_y;

    int window_dirty;

    HWAVEOUT hWaveOut;
    WAVEHDR hdr;
} system_state;

static system_state state;

static LARGE_INTEGER frequency;
static double clock_frequency;
static double awake_time;

LRESULT CALLBACK win32_process_msg(HWND window,
                                   uint32_t msg,
                                   WPARAM w_param,
                                   LPARAM l_param);

void platform_audio_init(int sampleRate, int channels) {
    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = channels;
    wfx.nSamplesPerSec = sampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * channels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    if (waveOutOpen(&state.hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        MessageBox(NULL, "Failed to open audio device", "Error", MB_OK);
    }
}

void platform_play_frame(short *samples, int sample_count, int channels) {
    int dataSize = sample_count * sizeof(short) * channels;

    state.hdr.lpData = (LPSTR)samples;
    state.hdr.dwBufferLength = dataSize;

    waveOutPrepareHeader(state.hWaveOut, &state.hdr, sizeof(WAVEHDR));
    waveOutWrite(state.hWaveOut, &state.hdr, sizeof(WAVEHDR));
}

void platform_init(const char *app_name,
                   int32_t x,
                   int32_t y,
                   int32_t width,
                   int32_t height) {
    state.instance = GetModuleHandleA(0);

    HICON icon = LoadIcon(state.instance, IDI_APPLICATION);
    WNDCLASS wc;
    platform_mem_set(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = win32_process_msg;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state.instance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "window_class";

    if (!RegisterClassA(&wc)) {
        printf("Window registration failed!\n");
        exit(1);
    }

    int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    uint32_t client_x = screenWidth / 2 + x - width / 2;
    uint32_t client_y = screenHeight / 2 + y - height / 2;
    uint32_t client_w = width;
    uint32_t client_h = height;

    uint32_t window_x = client_x;
    uint32_t window_y = client_y;
    uint32_t window_w = client_w;
    uint32_t window_h = client_h;

    uint32_t window_style = WS_VISIBLE | WS_POPUP;
    uint32_t window_ex_style = WS_EX_APPWINDOW;

    window_style |= WS_MAXIMIZEBOX;
    window_style |= WS_MINIMIZEBOX;
    // window_style |= WS_THICKFRAME;

    // uint32_t aero_borderless  = WS_POPUP            | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    RECT border_rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    window_x += border_rect.left;
    window_y += border_rect.top;

    window_w += border_rect.right - border_rect.left;
    window_h += border_rect.bottom - border_rect.top;

    state.window_x = window_x;
    state.window_y = window_y;
    state.window_dirty = 0;

    HWND handle = CreateWindowExA(window_ex_style, "window_class", app_name,
        window_style, window_x, window_y, window_w,
                                window_h, 0, 0, state.instance, 0);

    if (handle == NULL) {
        printf("Window creation failed!\n");
        exit(2);
    } else {
        state.handle = handle;
    }

    int32_t should_activate = 1;
    int32_t show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;

    ShowWindow(state.handle, show_window_command_flags);

    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (double)frequency.QuadPart;
    awake_time = platform_get_absolute_time();

    state.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    state.bmi.bmiHeader.biWidth = state.game->width;
    state.bmi.bmiHeader.biHeight = -state.game->height; // top-down
    state.bmi.bmiHeader.biPlanes = 1;
    state.bmi.bmiHeader.biBitCount = 32;
    state.bmi.bmiHeader.biCompression = BI_RGB;
}

int
platform_screenshot_save(const char* path) {
    BITMAPFILEHEADER fileHeader = {0};

    int dataSize = state.game->width * state.game->height * 4;
    fileHeader.bfType = 0x4D42; // "BM"
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize = fileHeader.bfOffBits + dataSize;

    FILE* fp = 0;
    fopen_s(&fp, path, "wb");
    if (!fp) return 0;

    fwrite(&fileHeader, sizeof(fileHeader), 1, fp);
    fwrite(&state.bmi, sizeof(state.bmi), 1, fp);
    fwrite(state.game->color, dataSize, 1, fp);

    fclose(fp);
    return 1;
}

void platform_term() {
    DestroyWindow(state.handle);
    state.handle = 0;
}

double platform_get_time() {
    return state.time_passed;
}

float
platform_time_frame_get() {
    return state.time_frame;
}

void platform_pump_msg() {
    input_pre_update();

    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    input_update();
}

void *platform_alloc(uint64_t size, uint8_t aligned) { return malloc(size); }

void platform_free(void *block, uint8_t aligned) { return free(block); }

void *platform_mem_zero(void *block, uint64_t size) {
    return memset(block, 0, size);
}

void *platform_mem_copy(void *dst, const void *src, uint64_t size) {
    return memcpy(dst, src, size);
}

void *platform_mem_set(void *dst, int32_t value, uint64_t size) {
    return memset(dst, value, size);
}

double platform_get_absolute_time() {
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return (double)time.QuadPart * clock_frequency;
}

uint64_t
platform_raw_time_get() {
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return time.QuadPart;
}

void
platform_mouse_position_get(int* x, int* y) {
    POINT pt;
    GetCursorPos(&pt);
    *x = pt.x;
    *y = pt.y;
}

void
platform_time_scale_set(float scale) {
    state.time_scale = scale;
}

const uint8_t colors[] = {FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
                          FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
                          FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
                          FOREGROUND_RED | FOREGROUND_INTENSITY};

void platform_console_write(const char *msg, uint32_t msg_length,
                            uint8_t color_index) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console_handle, colors[color_index]);
    OutputDebugStringA(msg);
    LPDWORD number_written = 0;
    WriteConsoleA(console_handle, msg, (DWORD)msg_length, number_written, 0);
    SetConsoleTextAttribute(console_handle, colors[0]);
}

LRESULT CALLBACK win32_process_msg(HWND window, uint32_t msg, WPARAM w_param,
                                   LPARAM l_param) {
    if (w_param == SC_KEYMENU && (l_param >> 16) <= 0)
        return false;

    switch (msg) {
    case WM_ERASEBKGND: {
        return true;
    } break;

    case WM_DESTROY: {
        PostQuitMessage(0);
        return false;
    } break;

    case WM_TIMER: {
        return true;
    } break;

    case WM_CLOSE: {
        b_event_dispatch(GAME_QUIT);
        return false;
    } break;

    case WM_KEYDOWN: {
        event_t e;
        e.ctx.u32[0] = w_param;
        b_event_dispatch_ext(INPUT_KEY_DOWN, &e);
    } break;

    case WM_KEYUP: {
        event_t e;
        e.ctx.u32[0] = w_param;
        b_event_dispatch_ext(INPUT_KEY_UP, &e);
    } break;

    case WM_LBUTTONDOWN:
    {
        event_t e;
        e.ctx.u32[0] = 0;
        b_event_dispatch_ext(INPUT_BUTTON_DOWN, &e);
    } break;
    
    case WM_LBUTTONUP:
    {
        event_t e;
        e.ctx.u32[0] = 0;
        b_event_dispatch_ext(INPUT_BUTTON_UP, &e);
    } break;
    
    case WM_RBUTTONDOWN:
    {
        event_t e;
        e.ctx.u32[0] = 1;
        b_event_dispatch_ext(INPUT_BUTTON_DOWN, &e);
    } break;
    
    case WM_RBUTTONUP:
    {
        event_t e;
        e.ctx.u32[0] = 1;
        b_event_dispatch_ext(INPUT_BUTTON_UP, &e);
    } break;
    }

    return DefWindowProcA(window, msg, w_param, l_param);
}

void
platform_window_position_get(int* x, int* y) {
    *x = state.window_x;
    *y = state.window_y;
}

void
platform_window_position_set(int x, int y) {
    state.window_x = x;
    state.window_y = y;
    state.window_dirty = 1;
}

void on_quit() { state.is_running = false; }

static char text_buffer[128] = {0};
static int frame = 0;

int main(int argc, char** argv) {
    b_event_register(GAME_QUIT, &on_quit);
    input_init();
    const char* path = 0;
    if (argc > 1) {
        path = argv[1];
    }
    state.game = g_init(path);
    platform_init("Xeno", 0, 0, state.game->width, state.game->height);
    state.is_running = true;
    double frame_time = 1.0 / state.game->target_fps;
    state.time_start = platform_get_absolute_time();
    state.time_passed = 0.0;
    state.time_scale = 1.0;
    state.time_frame = 0.0f;
    timeBeginPeriod(1);
    while (state.is_running) {
        double start_time = platform_get_absolute_time();
        LARGE_INTEGER start, now;
        QueryPerformanceCounter(&start);

        platform_pump_msg();
        g_update(state.time_frame);

        if (state.window_dirty) {
            state.window_dirty = 0;
            SetWindowPos(state.handle, HWND_TOP, state.window_x, state.window_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        
        // Set up a BITMAPINFO describing the framebuffer
        HDC hdc = GetDC(state.handle);
        SetDIBitsToDevice(hdc, 0, 0, state.game->width, state.game->height, 0, 0, 0,
            state.game->height, state.game->color, &state.bmi,
            DIB_RGB_COLORS);
        ReleaseDC(state.handle, hdc);
        
#ifdef ENABLE_SREC
        sprintf(text_buffer, ".scr/demo_%d.bmp", frame++);
        platform_screenshot_save(text_buffer);
#endif

        do {
            QueryPerformanceCounter(&now);
            double elapsed = (double)(now.QuadPart - start.QuadPart) / frequency.QuadPart;
            double remaining = frame_time - elapsed;

            if (remaining > 0.002) {
                Sleep(1); // coarse sleep
            } else if (remaining > 0) {
                // busy spin for sub-ms accuracy
            } else {
                break; // over budget, skip sleep
            }
        } while (1);

        double end_time = platform_get_absolute_time();
        state.time_frame = end_time - start_time;
        state.time_passed += state.time_frame * state.time_scale;
    }
    g_term();
    platform_term();
    return 0;
}