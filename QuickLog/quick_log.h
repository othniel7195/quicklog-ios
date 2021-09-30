//
//  quick_log.h
//  QuickLog
//
//  Created by jimmy on 2021/9/19.
//

#ifndef quick_log_h
#define quick_log_h

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

typedef enum {
    verbose = 0,
    debug,
    info,
    warning,
    error
} quick_log_level;

/**
 日志辅助信息
 */
typedef struct {
    quick_log_level level; ///日志级别不能为空。
    const char *tag; ///日志所在模块,或者自定义的额外信息。
    const char *filename; ///日志所处文件信息。
    const char *func_name; ///日志所处调用方法信息。
    int line; ///日志所处行数, 默认-1
    struct timeval timeval; /// 日志调佣时间,不能为空。
    intmax_t pid; ///日志所处进程id,不能为空
    intmax_t tid; ///日志所处线程id,不能为空
    intmax_t main_tid; ///日志所处主线程id,不能为空
} quick_log_help_info;

typedef enum {
    LOG_FORMAT_FAILED = 1001,
    INIT_JSON_FAILED = 1002,
    LOG_BYTES_TOO_LONG = 1003,
    FLUSH_LOG_WITH_META_ERROR = 1004,
    INIT_ZLIB_FAILED = 1005,
    COMPRESS_ERROR_WITH_ZLIB = 1006,
    LOG_FILE_NAME_TOO_LONG = 1007,
    OPEN_LOG_DIR_FAILED = 1008,
    USE_MEMORY_LOG = 1009
} quick_log_err;

/// 初始化日志服务
/// @param app_id 需要日志服务的应用id
/// @param dir 存日志的文件夹,不能为空
/// @param cipher_version 加密版本
/// @param cipher_key 加密key
/// @param collect_quick_log_err 回调函数, 输出异常信息
void start_quick_log(const char *app_id,
                     const char *dir,
                     unsigned int cipher_version,
                     const char *cipher_key,
                     void (*collect_quick_log_err)(quick_log_err err, const char *err_msg));


/// 追加日志
/// @param log_info log辅助信息 不能为空
/// @param log log的message
void append_log(const quick_log_help_info *log_info, const char *log);

/// 主动日志落盘
void flush_log(void);

/// 停止日志服务
void end_quick_log(void);

/// 拉取日志
/// 调用方负责释放返回值的内存空间
/// void release_string_array(char **arr)
/// {
///   char** cpy = arr;
///   while(*cpy) {
///     free(*cpy);
///     cpy++;
///   }
///   free(arr);
/// }
/// @param dir 日志所在文件夹
/// @param day_span 当前日期的间隔天数
/// @param path_count 获取日志的个数, 不因为空, 0时返回NULL
/// @return 日志文件路径字符串数组。当 path_count 为 0 时，为空 NULL。
char **fetch_log_files(const char *dir, int day_span, int *path_count);

/// 拉取当前目录下所有日志文件
/// 调用方负责释放返回值的内存空间
/// @param dir 日志所在文件夹
/// @param path_count 获取日志的个数, 不因为空, 0时返回NULL
/// @return 日志文件路径字符串数组。当 path_count 为 0 时，为空 NULL。
char **fetch_all_log_files(const char *dir, int *path_count);

/// 主动删除过期日志文件
/// @param dir 日志所在文件夹
/// @param valid_time_sec 日志有效期, 单位秒, 不能为空
void delete_timeout_log_files(const char *dir, float valid_time_sec);


#endif /* quick_log_h */
