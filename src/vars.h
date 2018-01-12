/*
************************************************************************
            Global Variables for EPANET Program                            
************************************************************************
*/
#ifndef VARS_H
#define VARS_H

#include <stdio.h>
#include "hash.h"

// this single global variable is used only when the library is called in "legacy mode"
// with the 2.1-style API. 
EXTERN void* _defaultModel;

#endif
