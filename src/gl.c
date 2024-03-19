#include "gl.h"

#include <stdlib.h>

#include "log.h"

// TODO(i.akkuzin): Replace with dynamic array?
static GLenum errors[GL_MAX_ERROR_COUNT] = {0};
static unsigned short errorsCount = 0;

void
HandleGlError(const char *file, unsigned int lineno, const char *expression) {
    GLenum err = GL_NO_ERROR;

    while ((err = glGetError()) != GL_NO_ERROR) {
        errors[errorsCount++] = err;
    }

    for (unsigned short i = 0; i < errorsCount; ++i) {
        LOG_ERROR("GL: %s:%hu: %s => %i\n", file, lineno, expression, errors[i]);
    }

    if (errorsCount > 0) {
        exit(EXIT_FAILURE);
    }
}
