/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       demand.c
 Description:  data for demand pattern list
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/12/2019
 ******************************************************************************
*/

#ifdef _DEBUG
  #define _CRTDBG_MAP_ALLOC
  #include <stdlib.h>
  #include <crtdbg.h>
#else
  #include <stdlib.h>
#endif

#include <string.h>

#include "util/list.h"
#include "demand.h"

// --- Local functions --- //

demand_data_t *create_demand_data(double base_demand, int pattern_index, const char *category_name)
{
    demand_data_t *demand_data = (demand_data_t *)malloc(sizeof(demand_data_t));

    demand_data->base_demand = base_demand;
    demand_data->pattern_index = pattern_index;

    if (category_name)
        demand_data->category_name = strdup(category_name);
    else
        demand_data->category_name = NULL;

    return demand_data;
}

void delete_demand_data(void *data)
{
    demand_data_t *demand_data = *(demand_data_t **)data;

    if (demand_data->category_name)
        free(demand_data->category_name);

    free(demand_data);
}

// ---  Exported functions  --- //

int create_demand_list(list_t **dlist)
{
    list_t *demand_list = create_list(sizeof(demand_data_t *), delete_demand_data);
	if (!demand_list) return 0;
	*dlist = demand_list;
    return 1;
}

void delete_demand_list(list_t **dlist)
{
    if (*dlist) delete_list(*dlist);
    *dlist = NULL;
}

int add_demand(list_t *dlist, double baseDemand, int pattern, const char *name)
{
    demand_data_t *demand_data;
    demand_data = create_demand_data(baseDemand, pattern, name);
    return append_list(dlist, &demand_data);
}

void remove_nth_demand(list_t *dlist, int n)
{
    remove_nth_node(dlist, n);
}

int get_num_demands(list_t *dlist)
{
    return size_list(dlist);
}

int get_demand_index(list_t *dlist, const char *name, int key)
{
    demand_data_t *dd;
    int index = 1;

    // Search for key
    if (name == NULL || strlen(name) == 0)
    {
        return get_index(dlist, key);
    }

    // Search for category name
    dd = get_first_demand(dlist);
    while (dd)
    {
        if (strcmp(dd->category_name, name) == 0) return index;
        index++;
        dd = get_next_demand(dlist);
    }
    return 0;
}

demand_data_t *get_first_demand(list_t *dlist)
{
    if (size_list(dlist) == 0) return NULL;
    return *(demand_data_t **)get_first_data(dlist);
}

demand_data_t *get_next_demand(list_t *dlist)
{
    void *dd = NULL;
    dd = get_next_data(dlist);
    if (dd) return *(demand_data_t **)dd;
    return NULL;
}

demand_data_t *get_nth_demand(list_t *dlist, int n)
{
    void *dd = NULL;
    demand_data_t *ddata;
    dd = get_nth_data(dlist, n);
    ddata = *(demand_data_t **)dd;
    if (dd) return *(demand_data_t **)dd;
    return NULL;
}

void replace_demand(demand_data_t *dd, double baseDemand, int pattern, const char *name)
{
    if (dd)
    {
        dd->base_demand = baseDemand;
        dd->pattern_index = pattern;
        if (dd->category_name) free(dd->category_name);
        if (name == NULL || strlen(name) == 0) dd->category_name = NULL;
        else dd->category_name = strdup(name);
    }
}
