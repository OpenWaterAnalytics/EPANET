/*
 *  errormanager.h
 *
 *  Created on: Aug 25, 2017
 *
 *      Author: Michael E. Tryby
 *              US EPA - ORD/NRMRL
 */

#ifndef ERRORMANAGER_H_
#define ERRORMANAGER_H_

#define ERR_MAXMSG 256

typedef struct error_s {
	int error_status;
	void (*p_msg_lookup)(int, char*, int);
} error_handle_t;

error_handle_t* new_errormanager(void (*p_error_message)(int, char*, int));
void dst_errormanager(error_handle_t* error_handle);

int set_error(error_handle_t* error_handle, int errorcode);
char* check_error(error_handle_t* error_handle);
void clear_error(error_handle_t* error_handle);

#endif /* ERRORMANAGER_H_ */
