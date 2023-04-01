#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "database.h"

FILE *start_database(char *name) {
    FILE *file = fopen(name, "rb+");
    if (file == NULL) {
        file = fopen(name, "w");
        fclose(file);
        file = fopen(name, "rb+");
    }
    return file;
}

void create_new_page(FILE *f) {
    char page[PAGE_SIZE];
    memset(page, 0, PAGE_SIZE * sizeof(char));
    page[0] = empty_page;
    if (fseek(f, 0, SEEK_END)) {
        fprintf(stderr, "Error create page\n");
        return;
    }
    fwrite(page, sizeof(*page), PAGE_SIZE, f);
}

void set_node_page(FILE *f, node *n) {
    n->id = ftell(f) / PAGE_SIZE;
    char flag = start_node;
    fwrite(&flag, sizeof(flag), 1, f);
    fwrite(&(n->id), sizeof(n->id), 1, f);
    char len = (char) strlen(n->name);
    fwrite(&len, sizeof(len), 1, f);
    fwrite(n->name, sizeof(*n->name), len, f);
    attribute *att = n->att;
    while (att) {
        len = (char) strlen(att->name);
        fwrite(&len, sizeof(len), 1, f);
        fwrite(att->name, sizeof(*att->name), strlen(att->name), f);
        char type = att->type;
        fwrite(&type, sizeof(type), 1, f);
        int ifield;
        float ffield;
        _Bool bfield;
        switch (type) {
            case integer:
                ifield = att->ifield;
                fwrite(&ifield, sizeof(ifield), 1, f);
                break;
            case float_point:
                ffield = att->ffield;
                fwrite(&ffield, sizeof(ffield), 1, f);
                break;
            case string:
                len = (char) strlen(att->sfield);
                fwrite(&len, sizeof(len), 1, f);
                fwrite(att->sfield, sizeof(*att->sfield), strlen(att->sfield), f);
                break;
            case boolean:
                bfield = att->bfield;
                fwrite(&bfield, sizeof(bfield), 1, f);
                break;
        }
        att = att->next;
    }
    edge *edge = n->edges;
    fwrite("\0", sizeof(char), 1, f);
    while (edge) {
        len = (char) strlen(edge->name);
        fwrite(&len, sizeof(len), 1, f);
        fwrite(edge->name, sizeof(*edge->name), strlen(edge->name), f);
        long int id = edge->id;
        fwrite(&id, sizeof(id), 1, f);
        edge = edge->next;
    }
    fwrite("\0", sizeof(char), 1, f);
}

void create_node_page(FILE *f, node *n) {
    if (fseek(f, 0, SEEK_SET)) {
        fprintf(stderr, "Error create node page. Can't set point to start\n");
        return;
    }
    char page[PAGE_SIZE];
    while (fread(page, sizeof(*page), PAGE_SIZE, f)) {
        if (page[0] == empty_page) {
            fseek(f, -PAGE_SIZE, SEEK_CUR);
            set_node_page(f, n);
            return;
        }
    }
    create_new_page(f);
    if (fseek(f, -PAGE_SIZE, SEEK_END)) {
        fprintf(stderr, "Error find empty page\n");
        return;
    }
    set_node_page(f, n);
}

node *page_to_node(const char page[PAGE_SIZE]) {
    int c = 1;
    node *n = calloc(1, sizeof *n);
    n->id = *(long int *) (page + c);
    c += sizeof(long int);
    char len = page[c];
    c++;
    char *str = calloc(len + 1, sizeof(*str));
    for (int i = 0; i < len; i++) {
        str[i] = page[i + c];
    }
    c += len;
    n->name = str;
    while (page[c] != '\0') {
        attribute *att = calloc(1, sizeof(*att));
        len = page[c];
        c++;
        str = calloc(len + 1, sizeof(*str));
        for (int i = 0; i < len; i++) {
            str[i] = page[i + c];
        }
        c += len;
        att->name = str;
        char type = page[c];
        att->type = (enum type) type;
        c++;
        switch (type) {
            case integer:
                att->ifield = *(int *) (page + c);
                c += sizeof(int);
                break;
            case float_point:
                att->ffield = *(float *) (page + c);
                c += sizeof(float);
                break;
            case string:
                len = page[c];
                c++;
                str = calloc(len + 1, sizeof(*str));
                for (int i = 0; i < len; i++) {
                    str[i] = page[i + c];
                }
                c += len;
                att->sfield = str;
                break;
            case boolean:
                att->bfield = *(_Bool *) (page + c);
                c += sizeof(_Bool);
                break;
            default:
                att->sfield = NULL;
        }
        attribute *cur = n->att;
        n->att = att;
        att->next = cur;
    }
    c++;
    while (page[c] != '\0') {
        edge *edge = calloc(1, sizeof(*edge));
        len = page[c];
        c++;
        str = calloc(len + 1, sizeof(*str));
        for (int i = 0; i < len; i++) {
            str[i] = page[i + c];
        }
        c += len;
        edge->name = str;
        edge->id = *(long int *) (page + c);
        c += sizeof(long int);
        struct edge *cur = n->edges;
        n->edges = edge;
        edge->next = cur;
    }
    return n;
}

node *get_node_page(FILE *f, long int id) {
    if (fseek(f, 0, SEEK_SET)) {
        fprintf(stderr, "Error get node page. Can't set point to start\n");
        return NULL;
    }
    char page[PAGE_SIZE];
    while (fread(page, sizeof(*page), PAGE_SIZE, f)) {
        if (page[0] == start_node && page[1] == id) {
            node *n = page_to_node(page);
            return n;
        }
    }
    fprintf(stderr, "Haven't page with it's id\n");
    return NULL;
}

void update_node_page(FILE *f, node *n) {
    node *page = get_node_page(f, n->id);
    if (page == NULL) {
        fprintf(stderr, "Update error in get page\n");
        return;
    }
    n->id = page->id;
    free_node(page);
    fseek(f, n->id * PAGE_SIZE, SEEK_SET);
    set_node_page(f, n);
}

void delete_node_page(FILE *f, long int id) {
    node *page = get_node_page(f, id);
    if (page == NULL) {
        fprintf(stderr, "Delete error in get page\n");
        return;
    }
    char empty[PAGE_SIZE];
    memset(empty, 0, PAGE_SIZE * sizeof(char));
    empty[0] = empty_page;
    fseek(f, page->id * PAGE_SIZE, SEEK_SET);
    fwrite(empty, sizeof(char), PAGE_SIZE, f);
    free_node(page);
}

void add_attribute_page(FILE *f, attribute *att, long int id) {
    node *page = get_node_page(f, id);
    if (page == NULL) {
        fprintf(stderr, "Add attribute error in get page\n");
        return;
    }
    attribute *cur = page->att;
    if (cur == NULL) {
        page->att = att;
    } else {
        attribute *cur2 = NULL;
        while (cur != NULL) {
            cur2 = cur;
            cur = cur->next;
        }
        cur2->next = att;
    }
    fseek(f, page->id * PAGE_SIZE, SEEK_SET);
    set_node_page(f, page);
    free_node(page);
}

void update_attribute_page(FILE *f, attribute *att, long int id) {
    node *page = get_node_page(f, id);
    if (page == NULL) {
        fprintf(stderr, "Update attribute error in get page\n");
        return;
    }
    attribute *cur = page->att;
    while (cur != NULL) {
        if (!strcmp(cur->name, att->name)) {
            cur->type = att->type;
            switch (att->type) {
                case integer:
                    cur->ifield = att->ifield;
                    break;
                case float_point:
                    cur->ffield = att->ffield;
                    break;
                case string:
                    cur->sfield = att->sfield;
                    break;
                case boolean:
                    cur->bfield = att->bfield;
                    break;
                default:
                    cur->sfield = NULL;
            }
        }
        cur = cur->next;
    }
    fseek(f, page->id * PAGE_SIZE, SEEK_SET);
    set_node_page(f, page);
    free_node(page);
}

void delete_attribute_page(FILE *f, char *name, long int id) {
    node *page = get_node_page(f, id);
    if (page == NULL) {
        fprintf(stderr, "Delete attribute error in get page\n");
        return;
    }
    attribute *cur = page->att;
    if (cur != NULL) {
        while (!strcmp(cur->name, name)) {
            page->att = cur->next;
            cur->next = NULL;
            free(cur);
            cur = page->att;
        }
    }
    attribute *cur2 = NULL;
    while (cur != NULL) {
        if (!strcmp(cur->name, name)) {
            cur2->next = cur->next;
            cur->next = NULL;
            free(cur);
        } else {
            cur2 = cur;
        }
        cur = cur2->next;
    }
    fseek(f, page->id * PAGE_SIZE, SEEK_SET);
    set_node_page(f, page);
    free_node(page);
}

void add_edge_page(FILE *f, edge *edge, long int id) {
    node *page = get_node_page(f, id);
    if (page == NULL) {
        fprintf(stderr, "Add edge error in get page\n");
        return;
    }
    struct edge *cur = page->edges;
    if (cur == NULL) {
        page->edges = edge;
    } else {
        struct edge *cur2 = NULL;
        while (cur != NULL) {
            cur2 = cur;
            cur = cur->next;
        }
        cur2->next = edge;
    }
    fseek(f, page->id * PAGE_SIZE, SEEK_SET);
    set_node_page(f, page);
    free_node(page);
}

void update_edge_page(FILE *f, edge *edge, long int id) {
    node *page = get_node_page(f, id);
    if (page == NULL) {
        fprintf(stderr, "Update edge error in get page\n");
        return;
    }
    struct edge *cur = page->edges;
    while (cur != NULL) {
        if (cur->id == edge->id) {
            cur->name = edge->name;
        }
        cur = cur->next;
    }
    fseek(f, page->id * PAGE_SIZE, SEEK_SET);
    set_node_page(f, page);
    free_node(page);
}

void delete_edge_page(FILE *f, long int edge_id, long int id) {
    node *page = get_node_page(f, id);
    if (page == NULL) {
        fprintf(stderr, "Delete edge error in get page\n");
        return;
    }
    edge *cur = page->edges;
    if (cur != NULL) {
        if (cur->id == edge_id) {
            page->edges = cur->next;
            cur->next = NULL;
            free(cur);
            cur = page->edges;
        }
    }
    edge *cur2 = NULL;
    while (cur != NULL) {
        if (cur->id == edge_id) {
            cur2->next = cur->next;
            cur->next = NULL;
            free(cur);
        } else {
            cur2 = cur;
        }
        cur = cur2->next;
    }
    fseek(f, page->id * PAGE_SIZE, SEEK_SET);
    set_node_page(f, page);
    free_node(page);
}

graph *find_by_name_page(FILE *f, char *name) {
    if (fseek(f, 0, SEEK_SET)) {
        fprintf(stderr, "Error find name page. Can't set point to start\n");
        return NULL;
    }
    node *nodes = calloc(1, sizeof(node));
    int count = 0;
    char page[PAGE_SIZE];
    while (fread(page, sizeof(char), PAGE_SIZE, f)) {
        if (page[0] == start_node) {
            node *n = page_to_node(page);
            if (name == NULL || !strcmp(n->name, name)) {
                nodes = realloc(nodes, (count + 2) * sizeof(node));
                nodes[count] = *n;
                count++;
            }
            free(n);
        }
    }
    graph *graph = malloc(sizeof(*graph));
    graph->nodes = nodes;
    graph->n = count;
    return graph;
}

graph *ref(node *n, struct filter *f, FILE *file, _Bool *ref_used) {
    node *nodes = calloc(1, sizeof(node));
    int count = 0;
    attribute *att = n->att;
    while (att != NULL) {
        if (!strcmp(att->name, f->att->name)) {
            break;
        }
        att = att->next;
    }
    if (att != NULL) {
        _Bool c = false;
        char page[PAGE_SIZE];
        while (fread(page, sizeof(char), PAGE_SIZE, file)) {
            node *node = page_to_node(page);
            _Bool b = false;
            attribute *att2 = node->att;
            while (att2) {
                if (!strcmp(att2->name, f->att->sfield)) {
                    b = true;
                    break;
                }
                att2 = att2->next;
            }
            if (b && att2->type == att->type) {
                b = false;
                switch (att->type) {

                    case integer:
                        if (att->ifield == att2->ifield) b = true;
                        break;
                    case boolean:
                        if (att->bfield == att2->bfield) b = true;
                        break;
                    case string:
                        if (!strcmp(att->sfield, att2->sfield)) b = true;
                        break;
                    case float_point:
                        if (att->ffield == att2->ffield) b = true;
                        break;
                }
                if (b) {
                    nodes = realloc(nodes, (count + 2) * sizeof(*nodes));
                    nodes[count] = *node;
                    count++;
                    ref_used[node->id] = true;
                    c = true;
                }
            }
            free(node);
        }
        if (c) {
            nodes[count] = *n;
            count++;
        }
    }
    graph *graph = malloc(sizeof(*graph));
    graph->nodes = nodes;
    graph->n = count;
    return graph;
}

_Bool node_filters(node *n, struct filter *f, FILE *file) {
    _Bool b = true;
    while (f != NULL && b) {
        attribute *att = n->att;
        b = false;
        while (att != NULL) {
            if (!strcmp(att->name, f->att->name)) {
                b = true;
                break;
            }
            att = att->next;
        }
        if (!b) break;
        if (att->type != f->att->type && f->sel != reference) {
            b = false;
            break;
        }
        switch (f->sel) {
            case equals:
                switch (att->type) {

                    case integer:
                        if (att->ifield != f->att->ifield) {
                            b = false;
                        }
                        break;
                    case boolean:
                        if (att->bfield != f->att->bfield) {
                            b = false;
                        }
                        break;
                    case string:
                        if (strcmp(att->sfield, f->att->sfield) != 0) {
                            b = false;
                        }
                        break;
                    case float_point:
                        if (att->ffield != f->att->ffield) {
                            b = false;
                        }
                        break;
                }

                break;
            case bigger:
                switch (att->type) {

                    case integer:
                        if (att->ifield <= f->att->ifield) {
                            b = false;
                        }
                        break;
                    case boolean:
                        if (att->bfield <= f->att->bfield) {
                            b = false;
                        }
                        break;
                    case string:
                        if (strcmp(att->sfield, f->att->sfield) >= 0) {
                            b = false;
                        }
                        break;
                    case float_point:
                        if (att->ffield <= f->att->ffield) {
                            b = false;
                        }
                        break;
                }
                break;
            case bigger_eq:
                switch (att->type) {

                    case integer:
                        if (att->ifield < f->att->ifield) {
                            b = false;
                        }
                        break;
                    case boolean:
                        if (att->bfield < f->att->bfield) {
                            b = false;
                        }
                        break;
                    case string:
                        if (strcmp(att->sfield, f->att->sfield) > 0) {
                            b = false;
                        }
                        break;
                    case float_point:
                        if (att->ffield < f->att->ffield) {
                            b = false;
                        }
                        break;
                }
                break;
            case smaller:
                switch (att->type) {

                    case integer:
                        if (att->ifield >= f->att->ifield) {
                            b = false;
                        }
                        break;
                    case boolean:
                        if (att->bfield >= f->att->bfield) {
                            b = false;
                        }
                        break;
                    case string:
                        if (strcmp(att->sfield, f->att->sfield) <= 0) {
                            b = false;
                        }
                        break;
                    case float_point:
                        if (att->ffield >= f->att->ffield) {
                            b = false;
                        }
                        break;
                }
                break;
            case smaller_eq:
                switch (att->type) {

                    case integer:
                        if (att->ifield > f->att->ifield) {
                            b = false;
                        }
                        break;
                    case boolean:
                        if (att->bfield > f->att->bfield) {
                            b = false;
                        }
                        break;
                    case string:
                        if (strcmp(att->sfield, f->att->sfield) < 0) {
                            b = false;
                        }
                        break;
                    case float_point:
                        if (att->ffield > f->att->ffield) {
                            b = false;
                        }
                        break;
                }
                break;
            case not_equals:
                switch (att->type) {

                    case integer:
                        if (att->ifield == f->att->ifield) {
                            b = false;
                        }
                        break;
                    case boolean:
                        if (att->bfield == f->att->bfield) {
                            b = false;
                        }
                        break;
                    case string:
                        if (strcmp(att->sfield, f->att->sfield) == 0) {
                            b = false;
                        }
                        break;
                    case float_point:
                        if (att->ffield == f->att->ffield) {
                            b = false;
                        }
                        break;
                }
                break;
            case reference: {

                switch (att->type) {

                    case integer:
                        if (att->ifield == f->att->ifield) {
                            b = false;
                        }
                        break;
                    case boolean:
                        if (att->bfield == f->att->bfield) {
                            b = false;
                        }
                        break;
                    case string:
                        if (strcmp(att->sfield, f->att->sfield) == 0) {
                            b = false;
                        }
                        break;
                    case float_point:
                        if (att->ffield == f->att->ffield) {
                            b = false;
                        }
                        break;
                }
                break;
            }
        }
        f = f->next;
    }
    return b;
}

graph *find_filter_page(FILE *f, struct filter *filters) {
    fseek(f, 0, SEEK_END);
    long int max_id = ftello(f) / PAGE_SIZE;
    _Bool *ref_used = calloc(max_id + 1, sizeof(_Bool));
    if (fseek(f, 0, SEEK_SET)) {
        fprintf(stderr, "Error find filter page. Can't set point to start\n");
        return NULL;
    }
    node *nodes = calloc(1, sizeof(node));
    int count = 0;
    char page[PAGE_SIZE];
    while (fread(page, sizeof(char), PAGE_SIZE, f)) {
        if (page[0] == start_node) {
            long int addr = ftell(f);
            node *n = page_to_node(page);
            _Bool b;
            if (filters->sel == reference && !ref_used[n->id]) {
                b = false;
                ref_used[n->id] = true;
                graph *refg = ref(n, filters, f, ref_used);
                nodes = realloc(nodes, (count + 1 + refg->n) * sizeof(node));
                for (int i = 0; i < refg->n; i++) {
                    nodes[count + i] = refg->nodes[i];
                }
                count += refg->n;
                free(refg->nodes);
                free(refg);
            } else if(filters->sel != reference) {
                b = node_filters(n, filters, f);
            }
            if (b) {
                nodes = realloc(nodes, (count + 2) * sizeof(node));
                nodes[count] = *n;
                count++;
            }
            free(n);
            fseek(f, addr, SEEK_SET);
        }
    }
    graph *graph = malloc(sizeof(*graph));
    graph->nodes = nodes;
    graph->n = count;
    return graph;
}

graph *find_by_name_filter_page(FILE *f, char *name, struct filter *filters) {
    if (fseek(f, 0, SEEK_SET)) {
        fprintf(stderr, "Error find name filter page. Can't set point to start\n");
        return NULL;
    }
    node *nodes = calloc(1, sizeof(node));
    int count = 0;
    char page[PAGE_SIZE];
    while (fread(page, sizeof(char), PAGE_SIZE, f)) {
        if (page[0] == start_node) {
            long int addr = ftell(f);
            node *n = page_to_node(page);
            _Bool b;
            if (!strcmp(n->name, name)) {
                b = node_filters(n, filters, f);
            } else {
                b = false;
            }
            if (b) {
                nodes = realloc(nodes, (count + 2) * sizeof(node));
                nodes[count] = *n;
                count++;
            }
            free(n);
            fseek(f, addr, SEEK_SET);
        }
    }
    graph *graph = malloc(sizeof(*graph));
    graph->nodes = nodes;
    graph->n = count;
    return graph;
}

graph *find_edge_page(FILE *f, char *name, long int id) {
    node *n = get_node_page(f, id);
    node *nodes = calloc(1, sizeof(node));
    int count = 0;
    char page[PAGE_SIZE];
    edge *e = n->edges;
    while (e != NULL) {
        if (!strcmp(e->name, name)) {
            fseek(f, (e->id) * PAGE_SIZE, SEEK_SET);
            fread(page, sizeof(char), PAGE_SIZE, f);
            node *pn = page_to_node(page);
            nodes = realloc(nodes, (count + 2) * sizeof(node));
            nodes[count] = *pn;
            count++;
        }
        e = e->next;
    }
    free_node(n);
    graph *graph = malloc(sizeof(*graph));
    graph->nodes = nodes;
    graph->n = count;
    return graph;
}

graph *find_filter_part_page(FILE *f, struct filter *filters, graph *g){
    node *nodes = calloc(1, sizeof(node));
    long int max_id = ftello(f) / PAGE_SIZE;
    int count = 0;
    char page[PAGE_SIZE];
    _Bool *ref_used = calloc(max_id + 1, sizeof(_Bool));
    for(int i=0; i<g->n; i++){
        fseek(f, (g->nodes[i].id) * PAGE_SIZE, SEEK_SET);
        fread(page, sizeof(char), PAGE_SIZE, f);
        node *n = page_to_node(page);
        _Bool b;
        if (filters->sel == reference && !ref_used[n->id]) {
            b = false;
            ref_used[n->id] = true;
            graph *refg = ref(n, filters, f, ref_used);
            nodes = realloc(nodes, (count + 1 + refg->n) * sizeof(node));
            for (int i = 0; i < refg->n; i++) {
                nodes[count + i] = refg->nodes[i];
            }
            count += refg->n;
            free(refg->nodes);
            free(refg);
        } else if(filters->sel != reference) {
            b = node_filters(n, filters, f);
        }
        if (b) {
            nodes = realloc(nodes, (count + 2) * sizeof(node));
            nodes[count] = *n;
            count++;
        }
        free(n);
    }
    free_graph(g);
    graph *graph = malloc(sizeof(*graph));
    graph->nodes = nodes;
    graph->n = count;
    return graph;
}