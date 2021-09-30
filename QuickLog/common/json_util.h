//
//  json_util.h
//  QuickLog
//
//  Created by jimmy on 2021/9/5.
//

#ifndef json_util_h
#define json_util_h

#include "cJson/cJSON.h"

#define JSON_MAP_STRING 1
#define JSON_MAP_NUMBER 2
#define JSON_MAP_BOOL 3

typedef struct json_map {
    char *key;
    const char *valueStr;
    double valueNumber;
    int valueBool;
    int type;
    struct json_map *nextItem;
} Json_map;

Json_map *create_json_map(void);

int is_empty_json_map(Json_map *item);

void add_item_string(Json_map *map, const char *key, const char *value);

void add_item_number(Json_map *map, const char *key, double number);

void add_item_bool(Json_map *map, const char *key, int boolValue);

void delete_json_map(Json_map *map);

void inflate_json_by_map(cJSON *root, Json_map *map);

#endif /* json_util_h */
