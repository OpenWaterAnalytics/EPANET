//-----------------------------------------------------------------------------
//
//   errormanager.c
//
//   Purpose: Provides a simple interface for managing runtime error messages.
//
//   Date:       08/25/2017
//
//   Author:     Michael E. Tryby
//               US EPA - ORD/NRMRL
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include "errormanager.h"

error_handle_t* new_errormanager(void (*p_error_message)(int, char*, int))
//
// Purpose: Constructs a new error handle.
//
{
	error_handle_t* error_handle;
	error_handle = (error_handle_t*)calloc(1, sizeof(error_handle_t));

	error_handle->p_msg_lookup = p_error_message;

	return error_handle;
}

void dst_errormanager(error_handle_t* error_handle)
//
// Purpose: Destroys the error handle.
//
{
	free(error_handle);
}

int set_error(error_handle_t* error_handle, int errorcode)
//
// Purpose: Sets an error code in the handle.
//
{
	// If the error code is 0 no action is taken and 0 is returned.
	// This is a feature not a bug.
	if (errorcode)
		error_handle->error_status = errorcode;

	return errorcode;
}

char* check_error(error_handle_t* error_handle)
//
// Purpose: Returns the error message or NULL.
//
// Note: Caller must free memory allocated by check_error
//
{
	char* temp = NULL;

	if (error_handle->error_status != 0) {
		temp = (char*) calloc(ERR_MAXMSG, sizeof(char));

		if (temp)
			error_handle->p_msg_lookup(error_handle->error_status, temp, ERR_MAXMSG);
	}
	return temp;
}

void clear_error(error_handle_t* error_handle)
//
// Purpose: Clears the error from the handle.
//
{
	error_handle->error_status = 0;
}
