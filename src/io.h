#ifndef BREAKOUT_IO_H_
#define BREAKOUT_IO_H_

#include <stdio.h>

typedef enum {
    LOAD_FILE_OK,
    LOAD_FILE_FAILED,
    LOAD_FILE_NOT_EXISTS,
    LOAD_FILE_NOT_OPENED,
    LOAD_FILE_READ_ERR,
} LoadFileStatus;

LoadFileStatus LoadFile(const char *path, char **const fileContent, size_t *loadedBytes);

unsigned long GetFileSize(FILE *f);

#endif // BREAKOUT_IO_H_
