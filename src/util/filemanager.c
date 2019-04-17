/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/filemanager.c
 Description:  Provides a simple interface for managing files
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/01/2019
 ******************************************************************************
*/


// MSVC ONLY
#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
#else
    #include <stdlib.h>
#endif

#include <stdarg.h>

#include "filemanager.h"


typedef struct file_s {
    char *filename;    // Assumes this is a null terminated string
    FILE *file;
} file_handle_t;


// local (private) functions
int _fopen(FILE **f, const char *name, const char *mode);
int _get_temp_filename(char **tempname);


file_handle_t *create_file_manager() {

    file_handle_t *file_handle;
    file_handle = (file_handle_t *)calloc(1, sizeof(file_handle_t));

	file_handle->filename = NULL;
	file_handle->file = NULL;

    return file_handle;
}

void delete_file_manager(file_handle_t *file_handle) {

	if (file_handle->file != NULL)
		close_file(file_handle);

	free(file_handle->filename);
	free(file_handle);
}


int get_filename(file_handle_t *file_handle, char **filename)
//
// BE AWARE: The memory allocated here must be freed by the caller
//
{
    return cstr_duplicate(filename, file_handle->filename);
}


int open_file(file_handle_t *file_handle, const char *filename, const char *file_mode) {
    int error = 0;

    if (filename == NULL)
        _get_temp_filename(&(file_handle->filename));
    else
        cstr_duplicate(&(file_handle->filename), filename);

    if (file_mode == NULL)
        error = -1;
    else {
        error = _fopen(&(file_handle->file), file_handle->filename, file_mode);
    }

    return error;
}

int seek_file(file_handle_t *file_handle, F_OFF offset, int whence)
{
#ifdef _MSC_VER // Windows (32-bit and 64-bit)
#define FSEEK64 _fseeki64
#else         // Other platforms
#define FSEEK64 fseeko
#endif

    return FSEEK64(file_handle->file, offset, whence);
}

F_OFF tell_file(file_handle_t *file_handle)
{
#ifdef _MSC_VER // Windows (32-bit and 64-bit)
#define FTELL64 _ftelli64
#else         // Other platforms
#define FTELL64 ftello
#endif

    return FTELL64(file_handle->file);
}

// Read and write to a binary file
size_t read_file(void *ptr, size_t size, size_t nmemb, file_handle_t *file_handle)
{
    return fread(ptr, size, nmemb, file_handle->file);
}

size_t write_file(const void * ptr, size_t size, size_t count, file_handle_t *file_handle)
{
    return fwrite(ptr, size, count, file_handle->file);
}


// print and get from a text file
int printf_file(file_handle_t *file_handle, const char *format, ... )
{
    int error = 0;
    va_list args;

    va_start(args, format);
    error = vfprintf(file_handle->file, format, args);
    va_end(args);

    return error;
}

int gets_file(char *str, int num, file_handle_t *file_handle)
{
    fgets(str, num, file_handle->file);
    return 0;
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


bool is_valid(file_handle_t *file_handle)
{
	if ((file_handle->filename == NULL && file_handle->file == NULL) ||
		(cstr_isnullterm(file_handle->filename) && file_handle != NULL))
		return true;
	else
		return false;
}


int _fopen(FILE **f, const char *name, const char *mode)
//
//  Purpose: Substitute for fopen_s on platforms where it doesn't exist
//  Note: fopen_s is part of C++11 standard
//
{
    int ret = 0;

#ifdef _MSC_VER
    ret = (int)fopen_s(f, name, mode);
#else
    *f = fopen(name, mode);
    if (!*f)
        ret = -1;
#endif
    return ret;
}

int _get_temp_filename(char **tempname)
{
    int error = 0;

#ifdef _MSC_VER
    char *name = NULL;

    // --- use Windows _tempnam function to get a pointer to an
    //     unused file name that begins with "en"
    if ((name = _tempnam(name, "en")) == NULL) {
        error = -1;
        return error;
    }
    else
        cstr_duplicate(tempname, name);

    // --- free the pointer returned by _tempnam
    if (name)
        free(name);

    // --- for non-Windows systems:
#else
    // --- use system function mkstemp() to create a temporary file name
    cstr_duplicate(tempname, "enXXXXXX");
    error = mkstemp(*tempname);
#endif
    return error;
}
