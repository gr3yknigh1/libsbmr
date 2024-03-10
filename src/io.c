#include "io.h"

#include <stdlib.h>

#include <nostdlib/fs.h>

u64
file_size(FILE *f) {
    u64 size = 0;

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    return size;
}

load_file_status_t
load_file(const char *file_path, char **const content, u64 *loaded_bytes) {
    if (!noc_fs_is_exists(file_path)) {
        return LOAD_FILE_NOT_EXISTS;
    }

    if (loaded_bytes == nullptr) {
        return LOAD_FILE_FAILED;
    }

    FILE *f = fopen(file_path, "r");

    if (f == nullptr) {
        return LOAD_FILE_NOT_OPENED;
    }

    *loaded_bytes = file_size(f);

    if (loaded_bytes == 0) {
        return LOAD_FILE_OK;
    }

    *content = malloc(*loaded_bytes);
    fread(*content, *loaded_bytes, 1, f);
    fclose(f);

    return LOAD_FILE_OK;
}
