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


#if (_MSC_VER <= 1600)
  #define bool  int
  #define true  1
  #define false 0
#else
  #include <stdbool.h>
#endif


#if defined(__cplusplus)
extern "C" {
#endif


int cstr_duplicate(char **dest, const char *source);


bool cstr_isnullterm(const char *source);


#if defined(__cplusplus)
}
#endif


#endif /* CSTR_HELPER_H_ */
