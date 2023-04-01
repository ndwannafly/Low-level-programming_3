#ifndef LLP1_DATABASE_H
#define LLP1_DATABASE_H

#include "data.h"

#define PAGE_SIZE 2048

enum page_type {
    start_node = (char) 1,
    empty_page = (char) 0
};

FILE *start_database(char *name);

void create_node_page(FILE *f, node *n);

void update_node_page(FILE *f, node *n);

void delete_node_page(FILE *f, long int id);

node *get_node_page(FILE *f, long int id);

void add_attribute_page(FILE *f, attribute *att, long int id);

void update_attribute_page(FILE *f, attribute *att, long int id);

void delete_attribute_page(FILE *f, char *name, long int id);

void add_edge_page(FILE *f, edge *edge, long int id);

void update_edge_page(FILE *f, edge *edge, long int id);

void delete_edge_page(FILE *f, long int edge_id, long int id);

graph *find_by_name_page(FILE *f, char *name);

graph *find_filter_page(FILE *f, struct filter *filters);

graph *find_by_name_filter_page(FILE *f, char *name, struct filter *filters);

graph *find_edge_page(FILE *f, char *name, long int id);

graph *find_filter_part_page(FILE *f, struct filter *filters, graph *g);

#endif //LLP1_DATABASE_H
