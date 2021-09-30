//
//  log_format.c
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../quick_log.h"
#include "../common/json_util.h"
#include "log.h"
#include "../log_constants.h"



static unsigned char *init_full_log_with_json(const quick_log_help_info *log_info, const char *log, size_t log_total_size, size_t *full_log_size) {
    
    unsigned char *json_string = NULL;
    cJSON *root = NULL;
    Json_map *map = NULL;
    root = cJSON_CreateObject();
    map = create_json_map();
    
    if (NULL != root) {
        if (NULL != map) {
            add_item_number(map, level_key, log_info->level);
            add_item_string(map, tag_key, log_info->tag);
            add_item_string(map, fileneme_key, log_info->filename);
            add_item_string(map, func_name_key, log_info->func_name);
            if (-1 != log_info->line) {
                add_item_number(map, line_key, log_info->line);
            }
            add_item_number(map, thread_id_key, log_info->tid);
            add_item_number(map, process_id_key, log_info->pid);
            add_item_bool(map, is_main_thread_key, log_info->tid == log_info->main_tid);
            add_item_number(map, time_key, tv2ms(&log_info->timeval));
            
            // get local gmt offset
            time_t tv_sec = log_info->timeval.tv_sec;
            struct tm tm = *localtime((const time_t*)&tv_sec);
            add_item_number(map, gmtoff_key, tm.tm_gmtoff);
            
            char * log_bytes = NULL;
            if (NULL != log) {
                // 1kb for other info len
                size_t body_len = log_total_size - 1024 > 5 ? log_total_size - 1029 : 0;
                body_len = body_len > 0xFFFFU ? 0xFFFFU : body_len;
                body_len = strnlen(log, body_len);
                body_len = body_len > 0xFFFFU ? 0xFFFFU : body_len;
                log_bytes = malloc(body_len + 1);
                memset(log_bytes, 0, body_len + 1);
                memcpy(log_bytes, log, body_len);
                add_item_string(map, log_key, log_bytes);
            }
            
            inflate_json_by_map(root, map);
            char *json_print = cJSON_PrintUnformatted(root);
            size_t json_str_length = strlen(json_print);
            if (json_str_length < log_total_size - 5) {
                size_t length = json_str_length + 1;
                json_string = (unsigned char *)malloc(length);
                memset(json_string, 0, length);
                memcpy(json_string, json_print, json_str_length);
                *full_log_size = length;
            } else {
                collect_error(LOG_BYTES_TOO_LONG, "log too long");
            }
            free(json_print);
            if (NULL != log_bytes) {
                free(log_bytes);
            }
        } else {
            collect_error(INIT_JSON_FAILED, "json map init failed");
        }
        cJSON_Delete(root);
    } else {
        collect_error(INIT_JSON_FAILED, "cJSON init failed");
    }
    
    if (NULL != map) {
        delete_json_map(map);
    }
    
    return json_string;
}

unsigned char *get_formated_log(const quick_log_help_info *log_info, const char *log_body, size_t *size, log *log) {
    if (NULL != log_info) {
        common_log_buffer *log_buffer;
        if (true == log->use_mmap) {
            log_buffer = ((mmap_log_buffer *)log->log_buffer)->buffer;
        } else {
            log_buffer = ((memory_log_buffer *)log->log_buffer)->buffer;
        }
        size_t left_size = log_buffer->total_size - log_buffer->offset;
        unsigned char * full_log = init_full_log_with_json(log_info, log_body, left_size, size);
        if (NULL == full_log) {
            *size = 0;
            return NULL;
        } else {
            return full_log;
        }
    } else {
        *size = 0;
        return NULL;
    }
}
