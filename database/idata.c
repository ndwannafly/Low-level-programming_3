
#include "idata.h"
#include "database.h"

FILE *open_database(char *name){
    return start_database(name);
}

void add_node(FILE *f, node *node){
    create_node_page(f, node);
}

void update_node(FILE *f, node *node){
    update_node_page(f, node);
}

void delete_node(FILE *f, long int id){
    delete_node_page(f, id);
}

node *get_node(FILE *f, int id){
    return get_node_page(f, id);
}

void add_attribute(FILE *f, attribute *att, long int id){
    add_attribute_page(f, att, id);
}

void update_attribute(FILE *f, attribute *att, long int id){
    update_attribute_page(f, att, id);
}

void delete_attribute(FILE *f, char *name, long int id){
    delete_attribute_page(f, name, id);
}

void add_edge(FILE *f, edge *edge, long int id){
    add_edge_page(f, edge, id);
}

void update_edge(FILE *f, edge *edge, long int id){
    update_edge_page(f, edge, id);
}

void delete_edge(FILE *f, long int edge_id, long int id){
    delete_edge_page(f, edge_id, id);
}

graph *find_by_name(FILE *f, char *name){
    return find_by_name_page(f, name);
}

graph *find_filter(FILE *f, struct filter *filters){
    return find_filter_page(f, filters);
}

graph *find_by_name_filter(FILE *f, char *name, struct filter *filters){
    return find_by_name_filter_page(f, name, filters);
}

graph *find_edge(FILE *f, char *name, long int id){
    return find_edge_page(f, name, id);
}

graph *find_filter_part(FILE *f, struct filter *filters, graph *g){
    return find_filter_part_page(f, filters, g);
}