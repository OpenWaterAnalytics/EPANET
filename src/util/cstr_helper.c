
#include <stdlib.h>
#include <string.h>

#include "cstr_helper.h"


int copy_cstr(const char *source, char **dest)
// Determines length, allocates memory, and returns a null terminated copy
// Be Aware: caller is responsible for freeing memory
{
    size_t len;

    len = strlen(source);
    *dest = (char *) calloc((len + 1), sizeof(char));

    if (*dest == NULL)
        return -1;
    else {
        strncpy(*dest, source, (len + 1));
		strncat(*dest, "\0", 1);
    }
    return 0;
}


bool isnullterm_cstr(const char *source)
{
	if (strchr(source, '\0'))
		return true;
	else
		return false;

}
