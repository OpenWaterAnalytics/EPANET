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

#include "demand.h"


typedef struct demand_data_s
{
  double base_demand;
  int    pattern_index;
  char   *category_name;
} demand_data_t;



list_t *create_demand_list(double base_demand, int pattern_index, const char *category_name)
{
	list_t *demand_list;
	demand_data_t *demand_data;

	demand_list = create_list(get_demand_data_size(), delete_demand_data);
	if (!demand_list) return NULL;

	demand_data = create_demand_data(base_demand, pattern_index, category_name);
	if (!demand_data) return NULL;

	append_list(demand_list, &demand_data);

	return demand_list;
}


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

size_t get_demand_data_size(void)
{
	return sizeof(demand_data_t *);
}

demand_data_t *get_demand_data(list_node_t *lnode)
{
    return *(demand_data_t **)get_data(lnode);
}


bool convert_units(list_node_t *lnode, double unit_conversion)
{
	double base_demand = get_base_demand(lnode);

	set_base_demand(lnode, base_demand/unit_conversion);
	return true;
}


double get_base_demand(list_node_t *lnode)
{
	return get_demand_data(lnode)->base_demand;
}

void set_base_demand(list_node_t *lnode, double base_demand)
{
	get_demand_data(lnode)->base_demand = base_demand;
}

int get_pattern_index(list_node_t *lnode)
{
    return get_demand_data(lnode)->pattern_index;
}

void set_pattern_index(list_node_t *lnode, int pattern_index)
{
	get_demand_data(lnode)->pattern_index = pattern_index;
}

char *get_category_name(list_node_t *lnode)
// Be advised: caller must free memory returned
{
    char *temp = get_demand_data(lnode)->category_name;

    if (temp)
        return strdup(temp);
    else
        return NULL;
}

void set_category_name(list_node_t *lnode, const char *category_name)
{
	free(get_demand_data(lnode)->category_name);
	get_demand_data(lnode)->category_name = strdup(category_name);
}
