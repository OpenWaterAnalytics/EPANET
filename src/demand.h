/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       demand.h
 Description:  data for demand pattern list
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/12/2019
 ******************************************************************************
*/

#ifndef DEMAND_H
#define DEMAND_H


#include "util/list.h"


#ifdef __cplusplus
extern "C" {
#endif


// Forward declarations
typedef struct demand_data_s demand_data_t;

// demand list gets declared in types.h struct Snode


list_t *create_demand_list(double base_demand, int pattern_index, char *category_name);


demand_data_t *create_demand_data(double base_demand, int pat_index, char *cat_name);

void delete_demand_data(void *data);

size_t get_demand_data_size(void);


bool convert_units(list_node_t *lnode, double unit_conversion);


double get_base_demand(list_node_t *lnode);
void set_base_demand(list_node_t *lnode, double base_demand);

int get_pattern_index(list_node_t *lnode);
void set_pattern_index(list_node_t *lnode, int pattern_index);

char *get_category_name(list_node_t *lnode);
void set_category_name(list_node_t *lnode, char *category_name);

// Make this private?
demand_data_t *get_demand_data(list_node_t *lnode);


#ifdef __cplusplus
}
#endif


#endif /* DEMAND_H */
