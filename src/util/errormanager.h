/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/errormanager.h
 Description:  Provides a simple interface for managing errors
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/02/2019
 ******************************************************************************
*/

#ifndef ERRORMANAGER_H_
#define ERRORMANAGER_H_

#define ERR_MAXMSG 256


#if defined(__cplusplus)
extern "C" {
#endif

// Forward declaration
typedef struct error_s error_handle_t;

error_handle_t* create_error_manager(void (*p_error_message)(int, char*, int));
void delete_error_manager(error_handle_t* error_handle);

int set_error(error_handle_t* error_handle, int error_code);
int check_error(error_handle_t* error_handle, char **error_message);
void clear_error(error_handle_t* error_handle);


#if defined(__cplusplus)
}
#endif

#endif /* ERRORMANAGER_H_ */
