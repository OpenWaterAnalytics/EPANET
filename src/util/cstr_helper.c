/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/cstr_helper.c
 Description:  Provides C string helper functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/02/2019
 ******************************************************************************
*/

#include <stdlib.h>
#include <string.h>

#include "cstr_helper.h"


int cstr_duplicate(char **dest, const char *source)
// Duplicates source string
{
    size_t size = 1 + strlen(source);
    *dest = (char *) calloc(size, sizeof(char));

    if (*dest == NULL)
        return -1;
    else {
#ifdef _MSC_VER
        strncpy_s(*dest, size, source, size);
#else
        strncpy(*dest, source, size);
#endif
    }
    return 0;
}


bool cstr_isvalid(const char *element_id)
// Determines if invalid characters are present in an element id string
{
    const char *invalid_chars = " \";";

    // if invalid char is present a pointer to it is returned else NULL
    if (strpbrk(element_id, invalid_chars))
        return false;
    else
        return true;
}


bool cstr_isnullterm(const char *source)
// Determines if the string passed is null terminated or not
{
    if (strchr(source, '\0'))
        return true;
    else
        return false;
}
