



#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_


#include <stdio.h>

#define FILE_MAXNAME 259
#define FILE_MAXMODE 3


#if defined(__cplusplus)
extern "C" {
#endif


typedef struct file_s {
    char filename[FILE_MAXNAME + 1],
    FILE *file;
    char mode[FILE_MAXMODE + 1];
} file_handle_t;


file_handle_t *create_file_manager(const char *filename, const char *file_mode);

int delete_file_manager(file_handle_t *file_handle);


int open_file(file_handle_t *file_handle);

FILE *get_file(file_handle_t *file_handle);

int close_file(file_handle_t *file_handle);

int remove_file(file_handle_t *file_handle);


#if defined(__cplusplus)
}
#endif

#endif /* FILEMANAGER_H_ */
