//
//  log.h
//  QuickLog
//
//  Created by jimmy on 2021/9/19.
//

#ifndef log_h
#define log_h

#include <stdlib.h>
#include <stdbool.h>
#include <zlib.h>
#include "../common/log_cipher.h"
#include "../quick_log.h"


typedef int file_handle;

typedef struct {
    char *app_id;
    char *dir;
    unsigned int cipher_version;
    char *log_to_file;
} quick_log_info;

typedef struct {
    unsigned int total_size;
    unsigned char *bytes;
    unsigned int offset;
} common_log_buffer;

typedef struct {
    common_log_buffer *buffer;
} memory_log_buffer;

typedef struct {
    char *file_path;
    file_handle handle;
    common_log_buffer *buffer;
} mmap_log_buffer;

typedef struct {
    quick_log_info *info;
    void *log_buffer;
    z_stream *deflater;
    aes_cfb *cfb;
    bool use_mmap;
    bool init_cipher_status;
    bool init_compress_status;
} log;

static log *log_single = NULL;

// log_util.c
bool has_suffix(const char *str, const char *suffix);

void add_int_value_to_bytes(unsigned char *bytes, unsigned int bytes_start, unsigned int value);

char *log_strdup(const char *s);

char **init_string_array(void);

char **insert_string(char **arr, const char *str);

void release_string_array(char **arr);

bool set_file_attr_protection_none(const char *path);

char *init_filepath(const char *dir, const char *filename);

double tv2ms(const struct timeval *tv);

void collect_error(quick_log_err err, const char *err_msg);

// log_file.c
void clear_invalid_log_files(const char*dir, float valid_time_sec, log *log);

char *init_log_file(const char *app_id, const char *dir, const unsigned int cipher_version);

void log_to_file(unsigned char *bytes, size_t size, log *log, const char *file_name);

void clear_default_invalid_log_files(log *log);

char **get_log_file_paths(const char *dir, int day_span, int *path_count);

char **get_all_log_files(const char *dir, int *path_count);

unsigned char *init_target_log_file_header(const char *app_id, const unsigned int cipher_version, unsigned int *header_size);

//log_deflater.c
z_stream *init_compression_object(void);

const size_t get_compression_object_chunk_size(void);
//Bytef  zlib库中的无符号的8位字节的别称
int compress_log_buf(Bytef *dest, uInt *dest_len, Bytef *source, uInt source_len, uInt avail_out, z_stream *stream);

void dealloc_compression_object(z_stream *deflater);

void reset_compression_object(log *log);

// log_buffer.c
void write_full_log_bytes(unsigned char *source, uint32_t source_len, unsigned int *offset, bool init_cipher, bool init_compress);

void append_log_buffer(unsigned char *bytes, unsigned int size, log *log);

void write_log_buffer_header(log *log);

void flush_log_buffer(log *log, bool check_valid);

// log_writer.c
void init_log_writer(const char *dir, log *log);

void deinit_log_writer(log *log);

//log_format.c
unsigned char *get_formated_log(const quick_log_help_info *log_help_info, const char *log_body, size_t *size, log *log);

#endif /* log_h */
