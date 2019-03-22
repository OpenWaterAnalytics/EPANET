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


#if defined(__cplusplus)
extern "C" {
#endif


typedef struct error_s {
    int error_status;
	void (*p_msg_lookup)(int, char*, int);
} error_handle_t;

error_handle_t* create_error_manager(void (*p_error_message)(int, char*, int));
void delete_error_manager(error_handle_t* error_handle);

int set_error(error_handle_t* error_handle, int error_code);
int check_error(error_handle_t* error_handle, char **error_message);
void clear_error(error_handle_t* error_handle);


#if defined(__cplusplus)
}
#endif

#endif /* ERRORMANAGER_H_ */
