/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       demand.h
 Description:  manages list of demand categories
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/26/2019
 ******************************************************************************
*/

#ifndef DEMAND_H
#define DEMAND_H

typedef struct list_s list_t;

typedef struct demand_data_s
{
    double base_demand;
    int    pattern_index;
    char   *category_name;
} demand_data_t;


int    create_demand_list(list_t **dlist);

void   delete_demand_list(list_t **dlist);

int    add_demand(list_t *dlist, double baseDemand, int pattern, const char *name);

int    get_num_demands(list_t *dlist);

int    get_demand_index(list_t *dlist, const char *name, int key);

void   replace_demand(demand_data_t *dd, double baseDemand, int pattern, const char *name);

void   remove_nth_demand(list_t *dlist, int n);

demand_data_t *get_first_demand(list_t *dlist);

demand_data_t *get_next_demand(list_t *dlist);

demand_data_t *get_nth_demand(list_t *dlist, int n);

#endif /* DEMAND_H */
