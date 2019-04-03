/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/filemanager.h
 Description:  Provides a simple interface for managing files
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/01/2019
 ******************************************************************************
*/

#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_


#include <stdio.h>
#include <stdarg.h>

#include "cstr_helper.h"


// F_OFF Must be a 8 byte / 64 bit integer for large file support
#ifdef _MSC_VER // Windows (32-bit and 64-bit)
#define F_OFF __int64
#else         // Other platforms
#define F_OFF off_t
#endif

#define FILE_MAXNAME 259


#if defined(__cplusplus)
extern "C" {
#endif

// Forward declariation of file_handle_t
typedef struct file_s file_handle_t;


file_handle_t *create_file_manager();

void delete_file_manager(file_handle_t *file_handle);


int get_filename(file_handle_t *file_handle, char **filename);


int open_file(file_handle_t *file_handle, const char *filename, const char *file_mode);

int seek_file(file_handle_t *file_handle, F_OFF offset, int whence);

F_OFF tell_file(file_handle_t *file_handle);


// Functions for working with binary files
size_t read_file(void *ptr, size_t size, size_t nmemb, file_handle_t *file_handle);

size_t write_file(const void *ptr, size_t size, size_t count, file_handle_t *file_handle);


// Functions for working with text files
int printf_file(file_handle_t *file_handle, const char *format, ... );

int gets_file(char *str, int num, file_handle_t *file_handle);


int close_file(file_handle_t *file_handle);

int remove_file(file_handle_t *file_handle);


bool is_valid(file_handle_t *file_handle);


#if defined(__cplusplus)
}
#endif

#endif /* FILEMANAGER_H_ */
