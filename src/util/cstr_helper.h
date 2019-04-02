/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/cstr_helper.h
 Description:  Provides C string helper functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/02/2019
 ******************************************************************************
*/

#ifndef CSTR_HELPER_H_
#define CSTR_HELPER_H_


#include <stdbool.h>


int copy_cstr(const char *source, char **destination);
bool isnullterm_cstr(const char *source);


#endif /* CSTR_HELPER_H_ */
