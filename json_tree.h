#ifndef LLP3_JSON_TREE_H
#define LLP3_JSON_TREE_H

#include <json-c/json.h>
#include "parser/request_tree.h"

#define MAX_SIZE 1024

json_object *tree_to_json(request_tree *tree);

#endif //LLP3_JSON_TREE_H
