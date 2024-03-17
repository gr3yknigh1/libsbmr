#include "io.h"

#include <stdlib.h>

#if defined(__linux__)
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
#define F_OK 0
#define access _access
#endif

static bool
IsPathExists(const char *s) {
    return access(s, F_OK) == 0;
}

unsigned long
GetFileSize(FILE *f) {
    unsigned long size = 0;

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    return size;
}

LoadFileStatus
LoadFile(const char *filePath, char **const content, size_t *loadedBytes) {
    if (!IsPathExists(filePath)) {
        return LOAD_FILE_NOT_EXISTS;
    }

    if (loadedBytes == nullptr) {
        return LOAD_FILE_FAILED;
    }

    FILE *f = fopen(filePath, "r");

    if (f == nullptr) {
        return LOAD_FILE_NOT_OPENED;
    }

    *loadedBytes = GetFileSize(f);

    if (loadedBytes == 0) {
        return LOAD_FILE_OK;
    }

    *content = malloc(*loadedBytes);
    fread(*content, *loadedBytes, 1, f);
    fclose(f);

    return LOAD_FILE_OK;
}
