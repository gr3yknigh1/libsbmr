#ifndef BREAKOUT_GL_H_
#define BREAKOUT_GL_H_

#include <glad/glad.h>

#define GL_MAX_ERROR_COUNT 256

extern void HandleGlError(const char *file, unsigned int lineno, const char *expression);

#define GL_CALL(__EXPR)                             \
    do {                                            \
        __EXPR;                                     \
        HandleGlError(__FILE__, __LINE__, #__EXPR); \
    } while (0)

#define GL_CALLP(__EXPR, __PTR)                     \
    do {                                            \
        *(__PTR) = __EXPR;                          \
        HandleGlError(__FILE__, __LINE__, #__EXPR); \
    } while (0)

#endif //  BREAKOUT_GL_H_
