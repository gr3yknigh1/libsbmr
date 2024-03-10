#ifndef BREAKOUT_SHADER_H_
#define BREAKOUT_SHADER_H_

#include <nostdlib/types.h>

#include "io.h"

typedef enum {
    SHADER_TYPE_VERT,
    SHADER_TYPE_FRAG,
    SHADER_TYPE_COUNT,
} shader_type_t;

bool shader_type_is_valid(shader_type_t value);

typedef u32 shader_t;

typedef enum {
    SHADER_COMPILE_OK,
    SHADER_COMPILE_ERR,
    SHADER_COMPILE_INVALID_VALUE,
    SHADER_COMPILE_FILE_NOT_FOUND,
    SHADER_COMPILE_FILE_LOAD_ERR,
} shader_compile_status_t;

shader_compile_status_t shader_compile(const char *source, u64 source_length,
                                     shader_type_t type, shader_t *shader);

shader_compile_status_t shader_load_and_compile(const char *source_file,
                                              shader_type_t type,
                                              shader_t *shader);

#endif // BREAKOUT_SHADER_H_
