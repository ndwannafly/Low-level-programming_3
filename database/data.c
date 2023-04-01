#include <malloc.h>
#include <stdio.h>

#include "data.h"

node *create_node(char *name, attribute *att, edge *edge) {
    node *node = malloc(sizeof(*node));
    node->name = name;
    node->att = att;
    node->edges = edge;
}

attribute *create_attribute(char *name, enum type type, void *p, attribute *next) {
    attribute *attribute = malloc(sizeof(*attribute));
    attribute->next = next;
    attribute->name = name;
    attribute->type = type;
    switch (type) {
        case integer:
            attribute->ifield = *(int *) p;
            break;
        case float_point:
            attribute->ffield = *(float *) p;
            break;
        case string:
            attribute->sfield = (char *) p;
            break;
        case boolean:
            attribute->bfield = *(_Bool *) p;
            break;
        default:
            attribute->sfield = NULL;
    }
    return attribute;
}

edge *create_edge(char *name, int id, edge *next) {
    edge *edge = malloc(sizeof(*edge));
    edge->name = name;
    edge->id = id;
    edge->next = next;
    return edge;
}

struct filter *create_filter(enum selector sel, attribute *att, struct filter *next) {
    struct filter *filter = malloc(sizeof(*filter));
    filter->sel = sel;
    filter->att = att;
    filter->next = next;
    return filter;
}

void free_attribute(attribute *att) {
    if (att != NULL) {
        free_attribute(att->next);
        free(att);
    }
}

void free_edge(edge *edge) {
    if (edge != NULL) {
        free_edge(edge->next);
        free(edge);
    }
}

void free_node(node *node) {
    if (node != NULL) {
        free_attribute(node->att);
        free_edge(node->edges);
        free(node);
    }
}

void free_filter(struct filter *filters) {
    if (filters != NULL) {
        free_filter(filters->next);
        free(filters);
    }
}

void free_graph(graph *graph) {
    for (int i = 0; i < graph->n; i++) {
        free_attribute(graph->nodes[i].att);
        free_edge(graph->nodes[i].edges);
    }
    free(graph->nodes);
    free(graph);
}
