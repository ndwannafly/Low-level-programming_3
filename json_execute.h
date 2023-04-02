#ifndef LLP3_JSON_EXECUTE_H
#define LLP3_JSON_EXECUTE_H

#include <json-c/json.h>
#include "database/idata.h"

#define MAX_SIZE 1024

json_object *json_execute(json_object *obj, FILE *f);

#endif //LLP3_JSON_EXECUTE_H
