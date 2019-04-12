/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       demand.h
 Description:  demand pattern list
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/12/2019
 ******************************************************************************
*/

#ifndef DEMAND_H
#define DEMAND_H


#include "util/list.h"


// Forward declarations
typedef struct demand_data_s demand_data_t;

// demand list gets declared in types.h struct Snode


demand_data_t *create_demand_data(double base_demand, int pat_index, char *cat_name);

void delete_demand_data(void *data);

demand_data_t *get_demand_data(list_node_t *lnode);


double get_base_demand(demand_data_t *data);

int get_pattern_index(demand_data_t *data);

char *get_category_name(demand_data_t *data);


#endif /* DEMAND_H */
