#include "shader.h"

#include <stdlib.h>

#include <glad/glad.h>

bool
shader_type_is_valid(shader_type_t value) {
    return value >= 0 && value < SHADER_TYPE_COUNT;
}

shader_compile_status_t
shader_compile(const char *source, u64 source_length, shader_type_t type,
               shader_t *shader) {
    if (source == nullptr || source_length == 0 || shader == nullptr ||
        !shader_type_is_valid(type)) {
        return SHADER_COMPILE_INVALID_VALUE;
    }

    GLuint id = 0;

    switch (type) {
    case SHADER_TYPE_VERT:
        id = glCreateShader(GL_VERTEX_SHADER);
        break;
    case SHADER_TYPE_FRAG:
        id = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    case SHADER_TYPE_COUNT:
        return SHADER_COMPILE_INVALID_VALUE;
    }

    glShaderSource(id, 1, &source, (GLint *)&source_length);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) {
        return SHADER_COMPILE_ERR;
    }

    *shader = id;

    return SHADER_COMPILE_OK;
}

shader_compile_status_t
shader_load_and_compile(const char *source_file_path, shader_type_t type,
                        shader_t *shader) {
    char *source = nullptr;
    u64 source_length = 0;

    load_file_status_t rc0 =
        load_file(source_file_path, (char **const)&source, &source_length);

    switch (rc0) {
    case LOAD_FILE_OK:
        break;
    case LOAD_FILE_NOT_EXISTS:
        return SHADER_COMPILE_FILE_NOT_FOUND;
    case LOAD_FILE_FAILED:
    case LOAD_FILE_NOT_OPENED:
    case LOAD_FILE_READ_ERR:
        return SHADER_COMPILE_FILE_LOAD_ERR;
    }

    shader_compile_status_t rc1 =
        shader_compile(source, source_length, type, shader);
    free(source);
    return rc1;
}
