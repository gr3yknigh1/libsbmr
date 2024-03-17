#include "shader.h"

#include <stdlib.h>

#include <glad/glad.h>

bool
ShaderTypeIsValid(ShaderType value) {
    return value >= 0 && value < SHADER_TYPE_COUNT;
}

ShaderCompileStatus
ShaderCompile(const char *source, unsigned long sourceLength, ShaderType type, Shader *shader) {
    if (source == nullptr || sourceLength == 0 || shader == nullptr || !ShaderTypeIsValid(type)) {
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

    glShaderSource(id, 1, &source, (GLint *)&sourceLength);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) {
        return SHADER_COMPILE_ERR;
    }

    *shader = id;

    return SHADER_COMPILE_OK;
}

ShaderCompileStatus
ShaderLoadAndCompile(const char *sourceFilePath, ShaderType type, Shader *shader) {
    char *source = nullptr;
    unsigned long sourceLength = 0;

    LoadFileStatus rc0 = LoadFile(sourceFilePath, (char **const)&source, &sourceLength);

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

    ShaderCompileStatus rc1 = ShaderCompile(source, sourceLength, type, shader);
    free(source);
    return rc1;
}
