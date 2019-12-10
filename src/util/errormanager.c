/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/errormanager.c
 Description:  Provides a simple interface for managing errors
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/02/2019
 ******************************************************************************
*/

//#ifdef _WIN32
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#else
#include <stdlib.h>
//#endif
#include <string.h>

#include "errormanager.h"


typedef struct error_s {
    int error_status;
	void (*p_msg_lookup)(int, char*, int);
} error_handle_t;


error_handle_t *create_error_manager(void (*p_error_message)(int, char*, int))
//
// Purpose: Constructs a new error handle.
//
{
	error_handle_t *error_handle;
	error_handle = (error_handle_t*)calloc(1, sizeof(error_handle_t));

	error_handle->p_msg_lookup = p_error_message;

	return error_handle;
}

void delete_error_manager(error_handle_t *error_handle)
//
// Purpose: Destroys the error handle.
//
{
	free(error_handle);
}

int set_error(error_handle_t *error_handle, int error_code)
//
// Purpose: Sets an error code in the handle.
//
{
	// If the error code is 0 no action is taken and 0 is returned.
	// This is a feature not a bug.
	if (error_code)
		error_handle->error_status = error_code;

	return error_code;
}

int check_error(error_handle_t *error_handle, char **error_message)
//
// Purpose: Returns the error message or NULL.
//
// Note: Caller must free memory allocated by check_error
//
{   int error_code = error_handle->error_status;
	char *temp = NULL;

	if (error_code != 0) {
		temp = (char*) calloc(ERR_MAXMSG + 1, sizeof(char));

		if (temp)
			error_handle->p_msg_lookup(error_code, temp, ERR_MAXMSG);
	}
    *error_message = temp;
	return error_code;
}

void clear_error(error_handle_t *error_handle)
//
// Purpose: Clears the error from the handle.
//
{
	error_handle->error_status = 0;
}
