



#ifndef EPANET_PY_H
#define EPANET_PY_H


#include "epanet2_export.h"
#include "epanet2_enums.h"


#if defined(__cplusplus)
extern "C" {
#endif

// Opaque pointer to project
typedef void *Handle;


int DLLEXPORT create_project(Handle *ph);

int DLLEXPORT delete_project(Handle *ph);

int DLLEXPORT run_project(Handle ph, const char *input_path,
    const char *report_path, const char *output_path);


#if defined(__cplusplus)
}
#endif

#endif //EPANET_PY_H
