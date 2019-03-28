

#include <stdio.h>
#include <string.h>

#include "filemanager.h"


// local (private) functions
int _get_temp_filename(char *tempname);


file_handle_t *create_file_manager(const char *filename, const char *file_mode) {

    file_handle_t *file_handle;
    file_handle = (file_handle_t*)calloc(1, sizeof(file_handle_t));

    if (filename == NULL)
        _get_temp_filename(file_handle->filename);
    else
        strncpy(file_handle->filename, filename, FILE_MAXNAME);

    file_handle->file = NULL;
    strncpy(file_handle->mode, file_mode, FILE_MAXMODE);

    return file_handle;
}

int delete_file_manager(file_handle_t *file_handle) {
    free(file_handle);
}


int open_file(file_handle_t *file_handle) {
    int error = 0;

    if ((file_handle->file = fopen(file_handle->filename, file_handle->mode)) == NULL)
        error = -1;

    return error;
}

FILE *get_file(file_handle_t *file_handle) {
    return file_handle->file;
}

int close_file(file_handle_t *file_handle) {
    int error = 0;

    if (file_handle->file != NULL) {
        error = fclose(file_handle->file);
        file_handle->file = NULL;
    }
    return error;
}

int remove_file(file_handle_t *file_handle) {
    return remove(file_handle->filename);
}


int _get_temp_filename(char *tempname) {
    int error = 0
#ifdef _WIN32
    char* name = NULL;

    // --- use Windows _tempnam function to get a pointer to an
    //     unused file name that begins with "en"
    if ((name = _tempnam(NULL, "en") == NULL) {
        error = -1;
        return error;
    }
    else if (strlen(name) < FILE_MAXNAME)
        strncpy(tempname, name, FILE_MAXNAME);
    else
        tempname = NULL;

    // --- free the pointer returned by _tempnam
    if (name)
        free(name);

    // --- for non-Windows systems:
#else
    // --- use system function mkstemp() to create a temporary file name
    strncpy(tempname, "enXXXXXX", 8);
    error = mkstemp(tempname);
#endif
    return error;
}
