
#include <stdlib.h>
#include <string.h>


int copy_cstr(const char *source, char **destination, size_t *size)
// Determines length, allocates memory, and returns a null terminated copy
// Be Aware: caller is responsible for freeing memory
{
    size_t len;

    len = strlen(source);
    *destination = (char *) calloc(len + 1, sizeof(char));

    if (*destination == NULL)
        return -1;
    else {
        strncpy(*destination, source, len);
        *size = len + 1;
    }
    return 0;
}
