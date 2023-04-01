#ifndef LLP1_DATA_STRUCT_H
#define LLP1_DATA_STRUCT_H
#define MAX_EDGE 25
#define MAX_ATTRIBUTES 25

enum type {
    integer = (char) 1,
    boolean = (char) 4,
    string = (char) 3,
    float_point = (char) 2
};

typedef struct edge {
    char *name;
    long int id;
    struct edge *next;
} edge;

typedef struct node node;

typedef struct attribute {
    char *name;
    enum type type;
    union {
        char *sfield;
        int ifield;
        float ffield;
        _Bool bfield;
    };
    struct attribute *next;
} attribute;

typedef struct node {
    long int id;
    char *name;
    attribute *att;
    edge *edges;
} node;

typedef struct graph {
    int n;
    node *nodes;
} graph;

enum selector{
    equals = 0,
    bigger = 1,
    bigger_eq = 2,
    smaller = 3,
    smaller_eq = 4,
    not_equals = 5,
    reference = 6
};

struct filter{
    enum selector sel;
    attribute *att;
    struct filter *next;
};

#endif //LLP1_DATA_STRUCT_H
