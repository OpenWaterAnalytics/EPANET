
#ifndef CSTR_HELPER_H_
#define CSTR_HELPER_H_


#include <stdbool.h>


int copy_cstr(const char *source, char **destination);
bool isnullterm_cstr(const char *source);


#endif /* CSTR_HELPER_H_ */
