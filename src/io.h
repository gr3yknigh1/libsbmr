#ifndef BREAKOUT_IO_H_
#define BREAKOUT_IO_H_

#include <stdio.h>

#include <nostdlib/types.h>

typedef char *const path_t;

typedef enum {
    LOAD_FILE_OK,
    LOAD_FILE_FAILED,
    LOAD_FILE_NOT_EXISTS,
    LOAD_FILE_NOT_OPENED,
    LOAD_FILE_READ_ERR,
} load_file_status_t;

load_file_status_t load_file(const char *path, char **const file_content,
                             u64 *file_size);

u64 file_size(FILE *f);

#endif // BREAKOUT_IO_H_
