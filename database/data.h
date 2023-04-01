#ifndef LLP1_DATA_H
#define LLP1_DATA_H

#include "data_struct.h"

node *create_node(char *name, attribute *att, edge *edge);

attribute *create_attribute(char *name, enum type type, void *p, attribute *next);

edge *create_edge(char *name, int id, edge *next);

struct filter *create_filter(enum selector sel, attribute *att, struct filter *next);

void free_node(node *node);

void free_attribute(attribute *att);

void free_edge(edge *edge);

void free_filter(struct filter *filters);

void free_graph(graph *graph);

#endif //LLP1_DATA_H
