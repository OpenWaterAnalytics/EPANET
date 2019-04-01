



#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_


#include <stdio.h>


// F_OFF Must be a 8 byte / 64 bit integer for large file support
#ifdef _WIN32 // Windows (32-bit and 64-bit)
#define F_OFF __int64
#else         // Other platforms
#define F_OFF off_t
#endif

#define FILE_MAXNAME 259
#define FILE_MAXMODE 3


#if defined(__cplusplus)
extern "C" {
#endif


typedef struct file_s file_handle_t;


file_handle_t *create_file_manager();

void delete_file_manager(file_handle_t *file_handle);


int open_file(file_handle_t *file_handle, const char *filename, const char *file_mode);

int seek_file(file_handle_t *file_handle, F_OFF offset, int whence);

F_OFF tell_file(file_handle_t *file_handle);

size_t read_file(void *ptr, size_t size, size_t nmemb, file_handle_t *file_handle);


int close_file(file_handle_t *file_handle);

int remove_file(file_handle_t *file_handle);


#if defined(__cplusplus)
}
#endif

#endif /* FILEMANAGER_H_ */
