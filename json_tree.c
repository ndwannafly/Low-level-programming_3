
#include "json_tree.h"

json_object *tree_to_json(request_tree *tree) {
    json_object *obj = json_object_new_object();
    switch (tree->type) {
        case REQUEST_CLOSE:
        case UNDEFINED:
            break;
        case REQUEST_ADD_EDGE: {
            add_edge_struct edge = tree->add_edge;
            json_object *oper = json_object_new_int(REQUEST_ADD_EDGE);
            json_object *jstring = json_object_new_string(edge.schema_name);
            json_object *jint1 = json_object_new_int64(edge.node1);
            json_object *jint2 = json_object_new_int64(edge.node2);
            json_object_object_add(obj, "oper", oper);
            json_object_object_add(obj, "name", jstring);
            json_object_object_add(obj, "id1", jint1);
            json_object_object_add(obj, "id2", jint2);
            break;
        }

        case REQUEST_DELETE_EDGE: {
            delete_edge_struct edge = tree->delete_edge;
            json_object *oper = json_object_new_int(REQUEST_DELETE_EDGE);
            json_object *jstring = json_object_new_string(edge.schema_name);
            json_object *jint1 = json_object_new_int64(edge.node1);
            json_object *jint2 = json_object_new_int64(edge.node2);
            json_object_object_add(obj, "oper", oper);
            json_object_object_add(obj, "name", jstring);
            json_object_object_add(obj, "id1", jint1);
            json_object_object_add(obj, "id2", jint2);
            break;
        }

        case REQUEST_ADD_NODE: {
            add_node_struct node = tree->add_node;
            json_object *oper = json_object_new_int(REQUEST_ADD_NODE);
            json_object *jstring = json_object_new_string(node.schema_name);
            json_object_object_add(obj, "oper", oper);
            json_object_object_add(obj, "name", jstring);
            json_object *array = json_object_new_array();
            for (int i = 0; i < arraylist_size(node.attribute_values); i++) {
                attr_value *cur_attr = arraylist_get(node.attribute_values, i);
                switch (cur_attr->type) {
                    case ATTR_TYPE_INTEGER: {
                        json_object *att = json_object_new_object();
                        json_object *key = json_object_new_string(cur_attr->attr_name);
                        json_object *type = json_object_new_int(ATTR_TYPE_INTEGER);
                        json_object *value = json_object_new_int(cur_attr->value.integer_value);
                        json_object_object_add(att, "name", key);
                        json_object_object_add(att, "type", type);
                        json_object_object_add(att, "value", value);
                        json_object_array_add(array, att);
                        break;
                    }
                    case ATTR_TYPE_BOOLEAN: {
                        json_object *att = json_object_new_object();
                        json_object *key = json_object_new_string(cur_attr->attr_name);
                        json_object *type = json_object_new_int(ATTR_TYPE_BOOLEAN);
                        json_object *value = json_object_new_boolean(cur_attr->value.bool_value);
                        json_object_object_add(att, "name", key);
                        json_object_object_add(att, "type", type);
                        json_object_object_add(att, "value", value);
                        json_object_array_add(array, att);
                        break;
                    }
                    case ATTR_TYPE_FLOAT: {
                        json_object *att = json_object_new_object();
                        json_object *key = json_object_new_string(cur_attr->attr_name);
                        json_object *type = json_object_new_int(ATTR_TYPE_FLOAT);
                        json_object *value = json_object_new_double(cur_attr->value.float_value);
                        json_object_object_add(att, "name", key);
                        json_object_object_add(att, "type", type);
                        json_object_object_add(att, "value", value);
                        json_object_array_add(array, att);
                        break;
                    }
                    case ATTR_TYPE_STRING: {
                        json_object *att = json_object_new_object();
                        json_object *key = json_object_new_string(cur_attr->attr_name);
                        json_object *type = json_object_new_int(ATTR_TYPE_STRING);
                        json_object *value = json_object_new_string(cur_attr->value.string_value);
                        json_object_object_add(att, "name", key);
                        json_object_object_add(att, "type", type);
                        json_object_object_add(att, "value", value);
                        json_object_array_add(array, att);
                        break;
                    }
                }
            }
            json_object_object_add(obj, "att", array);
            break;
        }

        case REQUEST_SELECT: {
            arraylist *st = tree->statements;
            json_object *oper;
            if (tree->schema_name) {
                oper = json_object_new_int(REQUEST_SELECT);
                json_object *jstring = json_object_new_string(tree->schema_name);
                json_object_object_add(obj, "name", jstring);
            } else {
                oper = json_object_new_int(REQUEST_SELECT);
            }
            json_object_object_add(obj, "oper", oper);
            json_object *array = json_object_new_array();
            if (st != NULL) {
                for (int i = 0; i < arraylist_size(st); i++) {
                    statement *cur_statement = arraylist_get(st, i);

                    switch (cur_statement->type) {
                        case SELECT_CONDITION: {
                            if (cur_statement->conditions == NULL)
                                break;
                            json_object *att = json_object_new_object();
                            json_object *type = json_object_new_int(cur_statement->type);
                            json_object *conditions = json_object_new_array();
                            for (int j = 0; j < arraylist_size(cur_statement->conditions); j++) {
                                select_condition *cur_condition = arraylist_get(cur_statement->conditions, j);
                                json_object *condition = json_object_new_object();
                                json_object *c_type = json_object_new_int(cur_condition->type);
                                json_object *name = json_object_new_string(cur_condition->attr_name);
                                json_object *option = json_object_new_int(cur_condition->option);
                                json_object_object_add(condition, "type", c_type);
                                json_object_object_add(condition, "name", name);
                                json_object_object_add(condition, "option", option);
                                switch (cur_condition->type) {
                                    case ATTR_TYPE_INTEGER: {
                                        json_object *value = json_object_new_int(cur_condition->value.integer_value);
                                        json_object_object_add(condition, "value", value);
                                        break;
                                    }
                                    case ATTR_TYPE_BOOLEAN: {
                                        json_object *value = json_object_new_boolean(cur_condition->value.bool_value);
                                        json_object_object_add(condition, "value", value);
                                        break;
                                    }
                                    case ATTR_TYPE_FLOAT: {
                                        json_object *value = json_object_new_double(cur_condition->value.float_value);
                                        json_object_object_add(condition, "value", value);
                                        break;
                                    }
                                    case ATTR_TYPE_STRING: {
                                        json_object *value = json_object_new_string(cur_condition->value.string_value);
                                        json_object_object_add(condition, "value", value);
                                        break;
                                    }
                                }
                                json_object_array_add(conditions, condition);
                            }
                            json_object_object_add(att, "type", type);
                            json_object_object_add(att, "conditions", conditions);
                            json_object_array_add(array, att);
                            break;
                        }
                        case OUT: {
                            json_object *att = json_object_new_object();
                            json_object *type = json_object_new_int(cur_statement->type);
                            json_object *value = json_object_new_string(cur_statement->attr_name);
                            json_object_object_add(att, "type", type);
                            json_object_object_add(att, "value", value);
                            json_object_array_add(array, att);
                            break;
                        }
                        case DELETE: {
                            json_object *type = json_object_new_int(cur_statement->type);
                            json_object *delete = json_object_new_object();
                            json_object_object_add(delete, "type", type);
                            json_object_array_add(array, delete);
                            break;
                        }
                    }
                }
            }
            json_object_object_add(obj, "statements", array);
            break;
        }
        case REQUEST_OPEN:
        case REQUEST_CREATE:
            break;
    }
    return obj;
}
