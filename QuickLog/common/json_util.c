//
//  json_util.c
//  QuickLog
//
//  Created by jimmy on 2021/9/5.
//

#include "json_util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Json_map *create_json_map(void) {
    Json_map *item = malloc(sizeof(Json_map));
    if (NULL != item)
//复制0到item指向的内存区域 长度是sizeof(Json_map)
        memset(item, 0, sizeof(Json_map));
    return  item;
}

/// check Json map empty
/// @param item Json map
/// @return 1 is empty
int is_empty_json_map(Json_map *item) {
    Json_map temp;
    memset(&temp, 0, sizeof(Json_map));
//memcmp 比较 item 和 &temp 前sizeof(Json_map) 个字节
    if(memcmp(item, &temp, sizeof(Json_map)) == 0)
        return  1;
    return  0;
}

void add_item_string(Json_map *map, const char *key, const char *value) {
//strnlen 获取字符串中实际字符个数，不包括结尾的'\0'，如果实际个数 <= maxlen（128），则返回n，否则返回128
    if(NULL != map && NULL != key && NULL != value && strnlen(key, 128) > 0) {
        Json_map *item = map;
        Json_map *temp = item;
        if (!is_empty_json_map(item)) {
            while (NULL != item->nextItem) {
                item = item->nextItem;
            }
            temp = create_json_map();
            item->nextItem = temp;
        }
        if (NULL != temp) {
            temp->type = JSON_MAP_STRING;
            temp->key = (char *)key;
            temp->valueStr = value;
        }
    }
}

void add_item_number(Json_map *map, const char *key, double number) {
    if(NULL != map && NULL != key && strnlen(key, 128) > 0) {
        Json_map *item = map;
        Json_map *temp = item;
        if (!is_empty_json_map(item)) {
            while (NULL != item->nextItem) {
              item = item->nextItem;
            }
        }
        temp = create_json_map();
        item->nextItem = temp;
        if (NULL != temp) {
            temp->type = JSON_MAP_NUMBER;
            temp->key = (char *)key;
            temp->valueNumber = number;
        }
    }
}

void add_item_bool(Json_map *map, const char *key, int boolValue) {
    if(NULL != map && NULL != key && strnlen(key, 128) > 0) {
        Json_map *item = map;
        Json_map *temp = item;
        if (!is_empty_json_map(item)) {
            while (NULL != item->nextItem) {
              item = item->nextItem;
            }
        }
        temp = create_json_map();
        item->nextItem = temp;
        if (NULL != temp) {
            temp->type = JSON_MAP_BOOL;
            temp->key = (char *)key;
            temp->valueBool = boolValue;
        }
    }
}

void delete_json_map(Json_map *map) {
    if (NULL != map) {
        Json_map *item = map;
        Json_map *temp = NULL;
        do {
            temp = item->nextItem;
            free(item);
            item = temp;
        } while (NULL != item);
    }
}

//扩充
void inflate_json_by_map(cJSON *root, Json_map *map) {
    if (NULL != root && NULL != map) {
        Json_map *item = map;
        do {
            switch (item->type) {
                case JSON_MAP_STRING:
                    if (NULL != item->valueStr) {
                        cJSON_AddStringToObject(root, item->key, item->valueStr);
                    }
                    break;
                case JSON_MAP_NUMBER:
                    cJSON_AddNumberToObject(root, item->key, item->valueNumber);
                    break;
                case JSON_MAP_BOOL:
                    cJSON_AddBoolToObject(root, item->key, item->valueBool);
                default:
                    break;
            }
            item = item->nextItem;
        } while (NULL != item);
    }
}
