//
//  log_constants.h
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#ifndef log_constants_h
#define log_constants_h

#include <stdint.h>

static const char *file_header_format = "lg";
static const uint32_t file_meta_version = 2;
static const unsigned int buffer_block_length = 150 * 1024;
static const unsigned int buffer_total_size = 150 * 1024;

static const uint8_t compress_start_cipher_start = 0x03;
static const uint8_t compress_start = 0x04;
static const uint8_t cipher_start = 0x05;
static const uint8_t no_cipher_no_compress = 0x06;

static const uint32_t file_name_max_length = 20;

// 日志的 body，输出的内容。 `String`
static const char *log_key = "c";
// 日志的级别，`String` 对应的值如下: "V","D","I","W", "E"
static const char *level_key = "l";
// 日志的 tag `String`
static const char *tag_key = "t";
// 日志对应的文件名 `String`
static const char *fileneme_key = "f";
// 日志对应的方法名 `String`
static const char *func_name_key = "u";
// 日志对应的行数 `Double`
static const char *line_key = "e";
// 日志对应的时间，毫秒 `Double`
static const char *time_key = "s";
// 日志对应的时区, gmt offset
static const char *gmtoff_key = "g";
// 日志对应的线程 id `Double`
static const char *thread_id_key = "i";
// 日志对应的进程 id `Double`
static const char *process_id_key = "p";
// 该条日志是否在主线程 `Bool`
static const char *is_main_thread_key = "m";


#endif /* log_constants_h */
