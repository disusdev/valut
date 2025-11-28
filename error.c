#include "error.h"
#include <stdio.h>

static v_error_callback_t g_error_callback = NULL;

const char* valut_error_string(valut_result_t error) {
    switch (error) {
        case V_OK: return "Success";
        case V_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case V_ERROR_BUFFER_OVERFLOW: return "Buffer overflow";
        case V_ERROR_INVALID_POINTER: return "Invalid pointer";
        case V_ERROR_FILE_NOT_FOUND: return "File not found";
        case V_ERROR_FILE_READ_FAILED: return "File read failed";
        case V_ERROR_FILE_WRITE_FAILED: return "File write failed";
        case V_ERROR_FILE_INVALID_FORMAT: return "Invalid file format";
        case V_ERROR_INVALID_DIMENSIONS: return "Invalid dimensions";
        case V_ERROR_BUFFER_NOT_INITIALIZED: return "Buffer not initialized";
        case V_ERROR_RENDER_FAILED: return "Render failed";
        case V_ERROR_PLATFORM_INIT_FAILED: return "Platform initialization failed";
        case V_ERROR_WINDOW_CREATION_FAILED: return "Window creation failed";
        case V_ERROR_AUDIO_INIT_FAILED: return "Audio initialization failed";
        case V_ERROR_ASSET_INVALID: return "Invalid asset";
        case V_ERROR_ASSET_LOAD_FAILED: return "Asset load failed";
        case V_ERROR_MESH_INVALID: return "Invalid mesh";
        case V_ERROR_TEXTURE_INVALID: return "Invalid texture";
        case V_ERROR_DIVISION_BY_ZERO: return "Division by zero";
        case V_ERROR_INVALID_MATRIX: return "Invalid matrix";
        case V_ERROR_INVALID_PARAM: return "Invalid parameter";
        case V_ERROR_NOT_IMPLEMENTED: return "Not implemented";
        default: return "Unknown error";
    }
}

void v_set_error_callback(v_error_callback_t callback) {
    g_error_callback = callback;
}

void v_report_error(valut_result_t error, const char* message, const char* file, int line) {
    if (g_error_callback) {
        g_error_callback(error, message, file, line);
    } else {
        fprintf(stderr, "[VALUT ERROR] %s: %s (at %s:%d)\n",
                v_error_string(error), message, file, line);
    }
}
