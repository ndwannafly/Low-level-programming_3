//
// Created by Ja_Vani on 05.03.2023.
//

#ifndef LLP2_REQUEST_TREE_H
#define LLP2_REQUEST_TREE_H

#include <stdbool.h>
#include <stdlib.h>
#include "array_list.h"

typedef enum request_type {
    UNDEFINED,
    REQUEST_OPEN,
    REQUEST_CREATE,
    REQUEST_CLOSE,
    REQUEST_ADD_NODE,
    REQUEST_SELECT,
    REQUEST_ADD_EDGE,
    REQUEST_DELETE_EDGE
} request_type;

typedef enum attr_type {
    ATTR_TYPE_INTEGER = 1,
    ATTR_TYPE_BOOLEAN = 4,
    ATTR_TYPE_FLOAT = 2,
    ATTR_TYPE_STRING = 3
} attr_type;

typedef struct file_work_struct {
    char *filename;
} file_work_struct;

typedef struct attribute_declaration {
    char *attr_name;
    attr_type type;
    char *schema_ref_name;
} attribute_declaration;

union value {
    int integer_value;
    bool bool_value;
    char* string_value;
    float float_value;
};

typedef struct attr_value {
    char *attr_name;
    attr_type type;
    union value value;
} attr_value;

typedef struct add_edge_struct {
    char* schema_name;
    int node1;
    int node2;
} add_edge_struct;

typedef struct delete_edge_struct {
    char* schema_name;
    int node1;
    int node2;
} delete_edge_struct;

typedef struct add_node_struct {
    char* schema_name;
    arraylist *attribute_values;
} add_node_struct;

typedef enum select_option {
    OPTION_EQUAL = 0,
    OPTION_GREATER = 1,
    OPTION_GREATER_EQUAL = 2,
    OPTION_LESS = 3,
    OPTION_LESS_EQUAL = 4,
    OPTION_NOT_EQUAL = 5,
    OPTION_LIKE = 7,
    OPTION_REFERENCE = 6
} select_option;

typedef struct select_condition {
    char *attr_name;
    select_option option;
    attr_type type;
    union value value;
} select_condition;

typedef enum statement_type {
    SELECT_CONDITION = 0,
    OUT = 1,
    DELETE = 2
} statement_type;

typedef struct statement {
    statement_type type;
    union {
        arraylist *conditions;
        char *attr_name;
    };
} statement;

typedef struct request_tree {
    request_type type;
    char* schema_name;
    union {
        file_work_struct file_work;
        add_edge_struct add_edge;
        delete_edge_struct delete_edge;
        add_node_struct add_node;
        arraylist *statements;
    };
} request_tree;

void print_request_tree(request_tree tree);

size_t get_tree_size(request_tree tree);

request_tree get_request_tree();

#endif //LLP2_REQUEST_TREE_H
