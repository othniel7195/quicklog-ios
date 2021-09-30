//
//  quick_log.c
//  QuickLog
//
//  Created by jimmy on 2021/9/19.
//

#include <string.h>
#include "../quick_log.h"
#include "log.h"

static void default_collect_quick_log_err(quick_log_err err, const char *err_msg) {
    printf("err: %d, err_msg: %s\n", err, err_msg);
}

typedef struct quick_log_hooks {
    void (*collect_quick_log_err)(quick_log_err err, const char *err_msg);
} quickLog_Hooks;

static quickLog_Hooks global_hooks = { default_collect_quick_log_err };

void collect_error(quick_log_err err, const char *err_msg) {
    global_hooks.collect_quick_log_err(err, err_msg);
}

void start_quick_log(const char *app_id,
                     const char *dir,
                     unsigned int cipher_version,
                     const char *cipher_key,
                     void (*collect_quick_log_err)(quick_log_err err, const char *err_msg)) {
    if (NULL != collect_quick_log_err) {
        global_hooks.collect_quick_log_err = collect_quick_log_err;
    }
    
    log_single = malloc(sizeof(log));
    memset(log_single, 0, sizeof(log));
    
    quick_log_info *info = malloc(sizeof(quick_log_help_info));
    info->app_id = log_strdup(app_id);
    info->dir = log_strdup(dir);
    info->cipher_version = cipher_version;
    info->log_to_file = init_log_file(app_id, dir, cipher_version);
    
    log_single->info = info;
    
    size_t key_len = strlen(cipher_key) + 1;
    unsigned char *key = malloc(key_len);
    memset(key, 0, key_len);
    memcpy(key, cipher_key, key_len -1);
    log_single->cfb = init_aes_cfb_cipher_context(key, key_len);
    free(key);
    
    log_single->init_cipher_status = true;
    log_single->deflater = init_compression_object();
    log_single->init_compress_status = true;
    
    init_log_writer(dir, log_single);
    clear_default_invalid_log_files(log_single);
}

void append_log(const quick_log_help_info *log_info, const char *log) {
    size_t size;
    unsigned char *log_bytes = get_formated_log(log_info, log, &size, log_single);
    if (NULL != log_bytes) {
        append_log_buffer(log_bytes, (unsigned int)size, log_single);
        free(log_bytes);
    } else {
        collect_error(LOG_FORMAT_FAILED, "format log with error");
    }
}

void flush_log(void) {
    reset_compression_object(log_single);
    flush_log_buffer(log_single, false);
    write_log_buffer_header(log_single);
}

void end_quick_log(void) {
    dealloc_compression_object(log_single->deflater);
    deinit_log_writer(log_single);
    reset_aes_cfb_cipher_context(log_single->cfb);
    
    free(log_single->info->app_id);
    free(log_single->info->dir);
    free(log_single->info->log_to_file);
    free(log_single->info);
    free(log_single->cfb);
    free(log_single);
}

char **fetch_log_files(const char *dir, int day_span, int *path_count) {
    return get_log_file_paths(dir, day_span, path_count);
}

char **fetch_all_log_files(const char *dir, int *path_count) {
    return get_all_log_files(dir, path_count);
}

void delete_timeout_log_files(const char *dir, float valid_time_sec) {
    clear_invalid_log_files(dir, valid_time_sec, log_single);
}
