
#ifndef LLP1_IDATA_H
#define LLP1_IDATA_H

#include <stdio.h>
#include "data.h"

FILE *open_database(char *name);

void add_node(FILE *f, node *node);

void update_node(FILE *f, node *node);

void delete_node(FILE *f, long int id);

node *get_node(FILE *f, int id);

void add_attribute(FILE *f, attribute *att, long int id);

void update_attribute(FILE *f, attribute *att, long int id);

void delete_attribute(FILE *f, char *name, long int id);

void add_edge(FILE *f, edge *edge, long int id);

void update_edge(FILE *f, edge *edge, long int id);

void delete_edge(FILE *f, long int edge_id, long int id);

graph *find_by_name(FILE *f, char *name);

graph *find_filter(FILE *f, struct filter *filters);

graph *find_by_name_filter(FILE *f, char *name, struct filter *filters);

graph *find_edge(FILE *f, char *name, long int id);

graph *find_filter_part(FILE *f, struct filter *filters, graph *g);

#endif //LLP1_IDATA_H
