#ifndef __ERROR_H__
#define __ERROR_H__

typedef enum {
    V_OK = 0,

    V_ERROR_OUT_OF_MEMORY = 1,
    V_ERROR_BUFFER_OVERFLOW = 2,
    V_ERROR_INVALID_POINTER = 3,

    V_ERROR_FILE_NOT_FOUND = 100,
    V_ERROR_FILE_READ_FAILED = 101,
    V_ERROR_FILE_WRITE_FAILED = 102,
    V_ERROR_FILE_INVALID_FORMAT = 103,

    V_ERROR_INVALID_DIMENSIONS = 200,
    V_ERROR_BUFFER_NOT_INITIALIZED = 201,
    V_ERROR_RENDER_FAILED = 202,

    V_ERROR_PLATFORM_INIT_FAILED = 300,
    V_ERROR_WINDOW_CREATION_FAILED = 301,
    V_ERROR_AUDIO_INIT_FAILED = 302,

    V_ERROR_ASSET_INVALID = 400,
    V_ERROR_ASSET_LOAD_FAILED = 401,
    V_ERROR_MESH_INVALID = 402,
    V_ERROR_TEXTURE_INVALID = 403,

    V_ERROR_DIVISION_BY_ZERO = 500,
    V_ERROR_INVALID_MATRIX = 501,

    V_ERROR_INVALID_PARAM = 900,
    V_ERROR_NOT_IMPLEMENTED = 901,
    V_ERROR_UNKNOWN = 999
} valut_result_t;

#ifdef NDEBUG
    #define V_ASSERT(expr) ((void)0)
#else
    #include <assert.h>
    #define V_ASSERT(expr) assert(expr)
#endif

#ifdef NDEBUG
    #define V_CHECK_PARAM(expr) ((void)0)
#else
    #define V_CHECK_PARAM(expr) \
        if (!(expr)) { \
            fprintf(stderr, "Parameter check failed: %s at %s:%d\n", \
                    #expr, __FILE__, __LINE__); \
            assert(0); \
        }
#endif

const char* v_error_string(valut_result_t error);

typedef void (*v_error_callback_t)(valut_result_t error, const char* message, const char* file, int line);

void v_set_error_callback(v_error_callback_t callback);
void v_report_error(valut_result_t error, const char* message, const char* file, int line);

#define V_REPORT_ERROR(err, msg) v_report_error(err, msg, __FILE__, __LINE__)

#endif
