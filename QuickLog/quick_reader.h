//
//  quick_reader.h
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#ifndef quick_reader_h
#define quick_reader_h

#include <stdbool.h>

/// 解密日志
/// @param file_path 需要解密的文件路径
/// @param destination_path 解密后的文件路径
bool read_log_file(const char *file_path,
                   const char *destination_path,
                   const char *(*get_key_with_file_json)(char *json),
                   void (*take_log)(char *log, int format_type));

#endif /* quick_reader_h */
