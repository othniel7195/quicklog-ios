//
//  log_buffer.c
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>

#include "log.h"
#include "../log_constants.h"


/// Log buffer header 结构 v1: `lg` ascii 码 2 bytes + 4 bytes meta version + 4 bytes log length + 4 bytes cipher version + 20 bytes file name
static const uint32_t log_header_len_v1 = 34;

static const uint32_t log_body_data_padding = 5; //v1: 5

static uint32_t get_log_len_v1(unsigned char *log_buffer) {
    uint32_t len = 0;
    memcpy(&len, log_buffer + 6, sizeof(len));
    return len;
}

// 日志 meta 是否一致
static bool check_log_bytes_format_valid(unsigned char *log_buffer) {
    char * file_type = malloc(3);
    memset(file_type, 0, 3);
    memcpy(file_type, log_buffer, 2);
    unsigned int meta_version = 0;
    memcpy(&meta_version, log_buffer + 2, 4);
    if (strcmp(file_type, file_header_format) == 0 && meta_version == file_meta_version) {
        free(file_type);
        return true;
    } else {
        free(file_type);
        return false;
    }
}

static common_log_buffer * get_log_buffer(log *log) {
    common_log_buffer *log_buffer;
    if (true == log->use_mmap) {
        log_buffer = ((mmap_log_buffer *)log->log_buffer)->buffer;
    } else {
        log_buffer = ((memory_log_buffer *)log->log_buffer)->buffer;
    }
    return log_buffer;
}

static void write_log_length_buffer(log *log, uint32_t len) {
    // log_buffer
    common_log_buffer *log_buffer = get_log_buffer(log);
    add_int_value_to_bytes(log_buffer->bytes, 6, len); // `lg` ascii 码 2 bytes + 4 bytes meta version
}

static char * get_log_file_name(unsigned char *log_buffer) {
    char *file_name = malloc(file_name_max_length);
    memset(file_name, 0, file_name_max_length);
    memcpy(file_name, log_buffer+14, file_name_max_length);
    return file_name;
}

void flush_log_buffer(log *log, bool check_valid) {
    common_log_buffer *log_buffer = get_log_buffer(log);
    if (true == check_valid) {
        bool valid = check_log_bytes_format_valid(log_buffer->bytes);
        // 如果日志 meta 不匹配，直接忽略。
        if (!valid) {
            collect_error(FLUSH_LOG_WITH_META_ERROR, "meta version is not on current version");
            return;
        }
    }
    uint32_t body_len = get_log_len_v1(log_buffer->bytes);
    char *file_name = get_log_file_name(log_buffer->bytes);
    log_to_file(log_buffer->bytes + log_header_len_v1, body_len, log, file_name);
    free(file_name);
    memset(log_buffer->bytes, 0, body_len + log_header_len_v1);
    log_buffer->offset = 0;
}

void write_log_buffer_header(log* log) {
    common_log_buffer *log_buffer = get_log_buffer(log);
    
    memcpy(log_buffer->bytes, file_header_format, 2);
    log_buffer->offset += 2;
    
    add_int_value_to_bytes(log_buffer->bytes, 2, file_meta_version);
    log_buffer->offset += 4;
    
    write_log_length_buffer(log, 0);
    log_buffer->offset += 4;
    
    add_int_value_to_bytes(log_buffer->bytes, 10, log->info->cipher_version);
    log_buffer->offset += 4;
    
    char * file_name = basename(log->info->log_to_file);
    memcpy(log_buffer->bytes + 14, file_name, strlen(file_name));
    log_buffer->offset += file_name_max_length;
}

void append_log_buffer(unsigned char *bytes, unsigned int size, log* log) {
    
    // log_buffer
    common_log_buffer *log_buffer = get_log_buffer(log);
    
    size_t left_size = log_buffer->total_size - log_buffer->offset;
    
    // defalter
    uInt chunk_size;
    compress_log_buf(log_buffer->bytes + log_buffer->offset + log_body_data_padding, &chunk_size, bytes, size, (uInt)left_size, log->deflater);
    
    // cipher
    log_encrypt_aes_cfb(log_buffer->bytes + log_buffer->offset + log_body_data_padding, log_buffer->bytes + log_buffer->offset + log_body_data_padding, chunk_size, log->cfb);
    
    // full log bytes writer
    write_full_log_bytes(log_buffer->bytes, chunk_size, &log_buffer->offset, log->init_cipher_status, log->init_compress_status);
    log->init_compress_status = false;
    log->init_cipher_status = false;
    write_log_length_buffer(log, log_buffer->offset - log_header_len_v1);
    
    // check flush
    if (log_buffer->offset > log_buffer->total_size*1/3) {
        reset_compression_object(log);
        flush_log_buffer(log, false);
        write_log_buffer_header(log);
    }
    
}
