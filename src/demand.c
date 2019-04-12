/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       demand.c
 Description:  demand pattern list
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/12/2019
 ******************************************************************************
*/

#include <stdlib.h>
#include <string.h>

#include "demand.h"


typedef struct demand_data_s
{
  double base_demand;
  int    pattern_index;
  char   *category_name;
} demand_data_t;


demand_data_t *create_demand_data(double base_demand, int pat_index, char *cat_name)
{
    demand_data_t *demand_data = (demand_data_t *)malloc(sizeof(demand_data_t));

    demand_data->base_demand = base_demand;
    demand_data->pattern_index = pat_index;

    if (cat_name)
        demand_data->category_name = strdup(cat_name);
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

demand_data_t *get_demand_data(list_node_t *lnode)
{
    return *(demand_data_t **)get_data(lnode);
}


double get_base_demand(demand_data_t *data)
{
    return data->base_demand;
}

int get_pattern_index(demand_data_t *data)
{
    return data->pattern_index;
}

char *get_category_name(demand_data_t *data)
{
    return data->category_name;
}
