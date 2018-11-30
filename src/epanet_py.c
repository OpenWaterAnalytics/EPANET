

#include <stdlib.h>

#include "epanet_py.h"
#include "util/errormanager.h"
#include "epanet2_2.h"
#include "types.h"


typedef struct {
    Project *project;
    error_handle_t *error;
}handle_t;


int DLLEXPORT create_project(Handle *ph)
{
    handle_t *handle = (handle_t *)calloc(1, sizeof(handle_t));

    if (handle != NULL)
    {
        EN_createproject(&handle->project);
        handle->error = error_new_manager(&EN_geterror);
        *ph = handle;
        return 0;
    }
    return -1;
}

int DLLEXPORT delete_project(Handle *ph)
{
    handle_t *handle = (handle_t *)*ph;

    if (handle == NULL)
        return -1;
    else
    {
        EN_deleteproject(&handle->project);
        error_dst_manager(handle->error);
    }
    return 0;
}

int DLLEXPORT run_project(Handle ph, const char *input_path,
    const char *report_path, const char *output_path)
{
    handle_t *pr = (handle_t *)ph;

    return error_set(pr->error,
        EN_runproject(pr->project, input_path, report_path, output_path, NULL));
}
