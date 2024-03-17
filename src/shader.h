#ifndef BREAKOUT_SHADER_H_
#define BREAKOUT_SHADER_H_

#include "io.h"

typedef enum {
    SHADER_TYPE_VERT,
    SHADER_TYPE_FRAG,
    SHADER_TYPE_COUNT,
} ShaderType;

bool ShaderTypeIsValid(ShaderType value);

typedef unsigned int Shader;

typedef enum {
    SHADER_COMPILE_OK,
    SHADER_COMPILE_ERR,
    SHADER_COMPILE_INVALID_VALUE,
    SHADER_COMPILE_FILE_NOT_FOUND,
    SHADER_COMPILE_FILE_LOAD_ERR,
} ShaderCompileStatus;

ShaderCompileStatus ShaderCompile(const char *source, unsigned long sourceLength, ShaderType type, Shader *shader);

ShaderCompileStatus ShaderLoadAndCompile(const char *sourceFile, ShaderType type, Shader *shader);

#endif // BREAKOUT_SHADER_H_
