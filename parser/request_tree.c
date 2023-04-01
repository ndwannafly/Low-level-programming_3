//
// Created by Ja_Vani on 05.03.2023.
//

#include "request_tree.h"
#include <stdio.h>
#include <string.h>


static char *const select_option_strings[] = {
        [OPTION_EQUAL] = "=",
        [OPTION_GREATER] = ">",
        [OPTION_GREATER_EQUAL] = ">=",
        [OPTION_LESS] = "<",
        [OPTION_LESS_EQUAL] = "<=",
        [OPTION_NOT_EQUAL] = "!=",
        [OPTION_LIKE] = "like",
        [OPTION_REFERENCE] = "ref"
};

static void print_condition(select_condition condition) {
    switch (condition.type) {
        case ATTR_TYPE_INTEGER: {
            printf("\t\"%s\" %s %d\n", condition.attr_name, select_option_strings[condition.option],
                   condition.value.integer_value);
            break;
        }
        case ATTR_TYPE_BOOLEAN: {
            printf("\t\"%s\" %s %s\n", condition.attr_name, select_option_strings[condition.option],
                   condition.value.bool_value ? "true" : "false");
            break;
        }
        case ATTR_TYPE_FLOAT: {
            printf("\t\"%s\" %s %.4f\n", condition.attr_name, select_option_strings[condition.option],
                   condition.value.float_value);
            break;
        }
        case ATTR_TYPE_STRING: {
            printf("\t\"%s\" %s %s\n", condition.attr_name, select_option_strings[condition.option],
                   condition.value.string_value);
            break;
        }
    }
}

static void print_statement(statement stmt) {
    switch (stmt.type) {
        case SELECT_CONDITION: {
            printf("* Condition of selection:\n");
            for (int i = 0; i < arraylist_size(stmt.conditions); i++) {
                select_condition *condition = arraylist_get(stmt.conditions, i);
                print_condition(*condition);
            }
            break;
        }
        case OUT: {
            printf("* Out nodes by \"%s\"\n", stmt.attr_name);
            break;
        }
        case DELETE: {
            printf("* Delete nodes\n");
            break;
        }
    }
}

static void print_statements(arraylist *statements) {
    if (statements == NULL) return;
    for (int i = 0; i < arraylist_size(statements); i++) {
        statement *cur_stmt = arraylist_get(statements, i);
        print_statement(*cur_stmt);
    }
}

static void print_node(add_node_struct node) {
    printf("Add node of type: %s\n", node.schema_name);
    for (int i = 0; i < arraylist_size(node.attribute_values); ++i) {
        attr_value *cur_attr = arraylist_get(node.attribute_values, i);
        switch (cur_attr->type) {
            case ATTR_TYPE_INTEGER: {
                printf("\"%s\": %d\n", cur_attr->attr_name, cur_attr->value.integer_value);
                break;
            }
            case ATTR_TYPE_BOOLEAN: {
                printf("\"%s\": %s\n", cur_attr->attr_name, cur_attr->value.bool_value ? "true" : "false");
                break;
            }
            case ATTR_TYPE_FLOAT: {
                printf("\"%s\": %f\n", cur_attr->attr_name, cur_attr->value.float_value);
                break;
            }
            case ATTR_TYPE_STRING: {
                printf("\"%s\": %s\n", cur_attr->attr_name, cur_attr->value.string_value);
                break;
            }
        }
    }
}

static void print_edge(add_edge_struct node) {
    printf("Add edge %s of nodes: %d %d\n", node.schema_name, node.node1, node.node2);
}

static void print_delete_edge(delete_edge_struct node) {
    printf("Delete edge %s of nodes: %d %d\n", node.schema_name, node.node1, node.node2);
}

void print_request_tree(request_tree tree) {
    switch (tree.type) {
        case REQUEST_OPEN: {
            printf("Open file: \"%s\"\n", tree.file_work.filename);
            break;
        }
        case REQUEST_CREATE: {
            printf("Create file: \"%s\"\n", tree.file_work.filename);
            break;
        }
        case REQUEST_CLOSE: {
            printf("Close file\n");
            break;
        }
        case REQUEST_ADD_EDGE: {
            print_edge(tree.add_edge);
            break;
        }
        case REQUEST_DELETE_EDGE: {
            print_delete_edge(tree.delete_edge);
            break;
        }
        case REQUEST_ADD_NODE: {
            print_node(tree.add_node);
            break;
        }
        case REQUEST_SELECT: {
            if (tree.schema_name) {
                printf("Select nodes: \"%s\"\n", tree.schema_name);
            } else {
                printf("Select all nodes \n");
            }
            print_statements(tree.statements);
            break;
        }
        case UNDEFINED:
            printf("Empty tree\n");
            break;
    }
}

static size_t string_size(char *string) {
    return sizeof(char) * strlen(string);
}

size_t get_tree_size(request_tree tree) {
    size_t tree_size = sizeof(request_tree);

    if (tree.schema_name != NULL) {
        string_size(tree.schema_name);
    }

    switch (tree.type) {
        case REQUEST_CLOSE:
        case UNDEFINED:
            break;

        case REQUEST_OPEN:
        case REQUEST_CREATE: {
            tree_size += string_size(tree.file_work.filename);
            break;
        }
        case REQUEST_ADD_EDGE: {
            tree_size += string_size(tree.add_edge.schema_name);
            tree_size += 2 * sizeof(int);
            break;
        }

        case REQUEST_DELETE_EDGE: {
            tree_size += string_size(tree.delete_edge.schema_name);
            tree_size += 2 * sizeof(int);
            break;
        }

        case REQUEST_ADD_NODE: {
            tree_size += string_size(tree.add_node.schema_name);
            tree_size += sizeof(arraylist) + tree.add_node.attribute_values->capacity * sizeof(add_node_struct);
            for (int i = 0; i < arraylist_size(tree.add_node.attribute_values); i++) {
                attr_value *cur_attr = arraylist_get(tree.add_node.attribute_values, i);
                tree_size += string_size(cur_attr->attr_name);
            }
            break;
        }

        case REQUEST_SELECT: {
            tree_size += sizeof(arraylist);
            if (tree.statements == NULL)
                break;

            tree_size += tree.statements->capacity * sizeof(statement);
            for (int i = 0; i < arraylist_size(tree.statements); i++) {
                statement *cur_statement = arraylist_get(tree.statements, i);

                switch (cur_statement->type) {
                    case SELECT_CONDITION: {
                        if (cur_statement->conditions == NULL)
                            break;
                        tree_size += sizeof(arraylist) + cur_statement->conditions->capacity * sizeof(select_condition);
                        for (int j = 0; j < arraylist_size(cur_statement->conditions); j++) {
                            select_condition *cur_condition = arraylist_get(cur_statement->conditions, j);
                            tree_size += string_size(cur_condition->attr_name);
                        }
                        break;
                    }
                    case OUT: {
                        tree_size += string_size(cur_statement->attr_name);
                        break;
                    }
                    case DELETE: {
                        break;
                    }
                }
            }
            break;
        }
    }
    return tree_size;
}
