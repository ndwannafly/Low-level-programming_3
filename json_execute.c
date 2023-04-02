
#include "json_execute.h"
#include "parser/request_tree.h"

struct json_object *graph_to_json(graph *pGraph);

json_object *json_execute(json_object *obj, FILE *f) {
    enum request_type request_type = json_object_get_int(json_object_object_get(obj, "oper"));
    json_object *response = json_object_new_object();
    switch (request_type) {
        case REQUEST_ADD_NODE: {
            node *n = calloc(1, sizeof(*n));
            char *name = json_object_get_string(json_object_object_get(obj, "name"));
            n->name = name;
            json_object *att = json_object_object_get(obj, "att");
            for (int i = 0; i < json_object_array_length(att); i++) {
                json_object *val = json_object_array_get_idx(att, i);
                attribute *a = calloc(1, sizeof(*a));
                char *att_name = json_object_get_string(json_object_object_get(val, "name"));
                enum type type = json_object_get_int(json_object_object_get(val, "type"));
                a->name = att_name;
                a->type = type;
                switch (type) {

                    case integer: {
                        int value = json_object_get_int(json_object_object_get(val, "value"));
                        a->ifield = value;
                        break;
                    }
                    case boolean: {
                        _Bool value = json_object_get_boolean(json_object_object_get(val, "value"));
                        a->bfield = value;
                        break;
                    }
                    case string: {
                        char *value = (char *) json_object_get_string(json_object_object_get(val, "value"));
                        a->sfield = value;
                        break;
                    }
                    case float_point: {
                        float value = (float) json_object_get_double(json_object_object_get(val, "value"));
                        a->ffield = value;
                        break;
                    }
                }
                a->next = n->att;
                n->att = a;
            }
            add_node(f, n);
            free_node(n);
            json_object_object_add(response, "Response", json_object_new_string("Add node"));
            break;
        }
        case REQUEST_SELECT: {
            char *name = (char *) json_object_get_string(json_object_object_get(obj, "name"));
            json_object *statements = json_object_object_get(obj, "statements");
            graph *g = find_by_name(f, name);
            for (int k = 0; k < json_object_array_length(statements); k++) {
                json_object *value = json_object_array_get_idx(statements, k);
                statement_type type = json_object_get_int(json_object_object_get(value, "type"));
                switch (type) {
                    case SELECT_CONDITION: {
                        json_object *conditions = json_object_object_get(value, "conditions");
                        struct filter *filters = NULL;
                        for (int t = 0; t < json_object_array_length(conditions); t++) {
                            json_object *con = json_object_array_get_idx(conditions, t);
                            enum selector sel = json_object_get_int(json_object_object_get(con, "option"));
                            attribute att;
                            att.next = NULL;
                            att.name = (char *)json_object_get_string(json_object_object_get(con, "name"));
                            att.type = json_object_get_int(json_object_object_get(con, "type"));
                            switch (att.type) {

                                case integer:
                                    att.ifield = json_object_get_int(json_object_object_get(con, "value"));
                                    break;
                                case boolean:
                                    att.bfield = json_object_get_boolean(json_object_object_get(con, "value"));
                                    break;
                                case string:
                                    att.sfield = json_object_get_string(json_object_object_get(con, "value"));
                                    break;
                                case float_point:
                                    att.ffield = json_object_get_double(json_object_object_get(con, "value"));
                                    break;
                            }
                            struct filter *fil = create_filter(sel, &att, NULL);
                            fil->next = filters;
                            filters = fil;
                        }
                        g = find_filter_part(f, filters, g);
                        free_filter(filters);
                        break;
                    }
                    case OUT: {
                        graph **buff = calloc(g->n, sizeof(**buff));
                        int sum = 0, size = g->n;
                        for (int i = 0; i < size; i++) {
                            buff[i] = find_edge(f, (char *) json_object_get_string(
                                    json_object_object_get(value, "value")), g->nodes[i].id);
                            sum += buff[i]->n;
                        }
                        free_graph(g);
                        g = malloc(sizeof(*g));
                        g->n = sum;
                        g->nodes = malloc(sum * sizeof(node));
                        int counter = 0;
                        for (int i = 0; i < size; i++) {
                            for (int t = 0; t < buff[i]->n; t++, counter++) {
                                g->nodes[counter] = buff[i]->nodes[t];
                            }
                            free(buff[i]->nodes);
                            free(buff[i]);
                        }
                        free(buff);
                        break;
                    }
                    case DELETE:
                        for (int i = 0; i < g->n; i++) {
                            delete_node(f, g->nodes[i].id);
                        }
                        json_object_object_add(response, "Response", json_object_new_string("Delete nodes"));
                        break;
                }
            }
            if (g != NULL) {
                json_object_object_add(response, "Response", graph_to_json(g));
                free_graph(g);
            }
            break;
        }
        case REQUEST_ADD_EDGE: {
            edge *e = calloc(1, sizeof(*e));
            char *name = (char *) json_object_get_string(json_object_object_get(obj, "name"));
            e->name = name;
            int id1 = json_object_get_int(json_object_object_get(obj, "id1"));
            int id2 = json_object_get_int(json_object_object_get(obj, "id2"));
            e->id = id2;
            add_edge(f, e, id1);
            json_object_object_add(response, "Response", json_object_new_string("Add edge"));
            break;
        }
        case REQUEST_DELETE_EDGE: {
            int id1 = json_object_get_int(json_object_object_get(obj, "id1"));
            int id2 = json_object_get_int(json_object_object_get(obj, "id2"));
            delete_edge(f, id2, id1);
            json_object_object_add(response, "Response", json_object_new_string("Delete edge"));
            break;
        }
        case UNDEFINED:
        case REQUEST_OPEN:
        case REQUEST_CREATE:
        case REQUEST_CLOSE:
            break;
    }
    return response;
}

struct json_object *graph_to_json(graph *g) {
    json_object *array = json_object_new_array();
    for (int i = 0; i < g->n; i++) {
        json_object *node = json_object_new_object();
        json_object *name = json_object_new_string(g->nodes[i].name);
        attribute *att = g->nodes[i].att;
        json_object *attributes = json_object_new_array();
        while (att) {
            json_object *a = json_object_new_object();
            json_object_object_add(a, "name", json_object_new_string(att->name));
            switch (att->type) {

                case integer:
                    json_object_object_add(a, "value", json_object_new_int(att->ifield));
                    break;
                case boolean:
                    json_object_object_add(a, "value", json_object_new_boolean(att->bfield));
                    break;
                case string:
                    json_object_object_add(a, "value", json_object_new_string(att->sfield));
                    break;
                case float_point:
                    json_object_object_add(a, "value", json_object_new_double(att->ffield));
                    break;
            }
            json_object_array_add(attributes, a);
            att = att->next;
        }
        edge *e = g->nodes[i].edges;
        json_object *edges = json_object_new_array();
        while (e) {
            json_object *a = json_object_new_object();
            json_object_object_add(a, "name", json_object_new_string(e->name));
            json_object_object_add(a, "id", json_object_new_int((int) e->id));
            json_object_array_add(edges, a);
            e = e->next;
        }
        json_object_object_add(node, "name", name);
        json_object_object_add(node, "id", json_object_new_int((int) g->nodes[i].id));
        json_object_object_add(node, "attributes", attributes);
        json_object_object_add(node, "edges", edges);
        json_object_array_add(array, node);
    }
    return array;
}
