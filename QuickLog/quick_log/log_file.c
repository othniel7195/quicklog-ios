//
//  log_file.c
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "log.h"
#include "../log_constants.h"

static bool has_suffix_lg(char *str) {
    return has_suffix(str, ".lg");
}

static char *init_file_name_suffix(time_t *time) {
    struct tm *pTm = localtime(time);
    char *suffix = malloc(64);
    snprintf(suffix, 64, "_%d%02d%02d.lg", 1900 + pTm->tm_year, 1 + pTm->tm_mon, pTm->tm_mday);
    return suffix;
}

static char *init_file_meta_json_string(const char *app_id, const int cipher_version) {
    char *json = malloc(100);
    snprintf(json, 100, "{\"appId\": \"%s\", \"cipherVersion\": %d}", app_id, cipher_version);
    return json;
}

static int generate_log_file_header_size(const char *json) {
    unsigned int format_size = 2;
    unsigned int meta_version_size = 4;
    unsigned int meta_length_size = 4;
    unsigned int json_size = (unsigned int)strlen(json);
    return format_size + meta_version_size + meta_length_size + json_size;
}

static uint8_t *init_log_file_header(const char *format, unsigned int meta_version, const char *json, unsigned int header_size) {
    unsigned char *file_header = malloc(header_size);
    memcpy(file_header, format, 2);
    
    int index = 2;
    add_int_value_to_bytes(file_header, index, meta_version);
    index += 4;
    
    int json_size = (int)strlen(json);
    add_int_value_to_bytes(file_header, index, json_size);
    index += 4;
    
    memcpy(file_header + index, json, json_size);
    
    return file_header;
}

static int get_log_dir_filename_last_index(const char *dir, const char *file_name_suffix) {
    struct dirent *file;
    DIR *p_dir = opendir(dir);
    if (NULL == p_dir) {
        collect_error(OPEN_LOG_DIR_FAILED, dir);
        return 0;
    }
    unsigned int index = 0;
    while(NULL != (file = readdir(p_dir))) {
        if (has_suffix(file->d_name, file_name_suffix) == false)
            continue;
        char *file_name_prefix = malloc(10);
        memset(file_name_prefix, 0, 10);
        strncpy(file_name_prefix, file->d_name, strlen(file->d_name) - strlen(file_name_suffix));
        //atoi把参数所指向的字符串转换为一个整数
        int prefix_number = atoi(file_name_prefix);
        if (prefix_number > index) {
            index = prefix_number;
        }
        free(file_name_prefix);
    }
    closedir(p_dir);
    return index;
}

static void write_log_to_file(const unsigned char *bytes, size_t size, const char *log_path) {
    FILE *fp;
    fp = fopen(log_path, "ab+");
    if (NULL != fp) {
        fwrite(bytes, size, 1, fp);
        fclose(fp);
    }
}

static char *init_full_filename(const int index, const char *suffix) {
    char *prefix = malloc(10);
    snprintf(prefix, 10, "%d", index);
    size_t length = strlen(prefix) + strlen(suffix);
    char *full_file_name = malloc(length + 1);
    snprintf(full_file_name, length + 1, "%s%s", prefix, suffix);
    free(prefix);
    if (strlen(full_file_name) > file_name_max_length) {
        collect_error(LOG_FILE_NAME_TOO_LONG, full_file_name);
    }
    return full_file_name;
}

static const float log_file_valid_time_sec = 60 * 60 * 24 * 7;

static const off_t max_log_file_size = 1024 * 1024 * 10;

static void create_new_log_file(const char *app_id, const char *filepath, const unsigned int cipher_version) {
    FILE *fp;
    fp = fopen(filepath, "ab+");
    if (NULL != fp) {
        set_file_attr_protection_none(filepath);
        char * json = init_file_meta_json_string(app_id, cipher_version);
        int header_size = generate_log_file_header_size(json);
        unsigned char *header = init_log_file_header(file_header_format, file_meta_version, json, header_size);
        fwrite(header, header_size, 1, fp);
        fclose(fp);
        free(json);
        free(header);
    }
}

static unsigned int read_log_file_meta_version(FILE *fp) {
    long offset = 2;
    fseek(fp, offset, SEEK_SET);
    unsigned int version;
    //读取4个字节
    fread(&version, 1, 4, fp);
    return version;
}

static char *read_log_file_header_json(FILE *fp) {
    long offset = 2 + 4;
    fseek(fp, offset, SEEK_SET);
    int length;
    fread(&length, 1, 4, fp); // get json length
    offset += 4;
    char * json = malloc(length + 1);
    memset(json, 0, length + 1);
    fseek(fp, offset, SEEK_SET);
    fread(json, 1, length, fp); // get json
    return json;
}

static char *init_new_log_file(unsigned int index, const char *file_name_suffix, const char *app_id, const char *dir, const unsigned int cipher_version) {
    char *fullname_again = init_full_filename(index, file_name_suffix);
    char *new_fullpath_again = init_filepath(dir, fullname_again);
    create_new_log_file(app_id, new_fullpath_again, cipher_version);
    free(fullname_again);
    return new_fullpath_again;
}

char **get_all_log_files(const char *dir, int *path_count) {  
    struct dirent *file;
    DIR *p_dir = opendir(dir);
    if (p_dir == NULL) {
        *path_count = 0;
        collect_error(OPEN_LOG_DIR_FAILED, dir);
        return NULL;
    }
    int count = 0;
    char **paths = init_string_array();
    while ((file = readdir(p_dir)) != NULL) {
        if (false == has_suffix(file->d_name, "lg"))
            continue;
        char *fullpath = init_filepath(dir, file->d_name);
        paths = insert_string(paths, fullpath);
        count ++;
        free(fullpath);
    }
    closedir(p_dir);
    *path_count = count;
    if (count == 0) {
        release_string_array(paths);
        return NULL;
    } else {
        return paths;
    }
}

char **get_log_file_paths(const char *dir, int day_span, int *path_count) {
    struct dirent *file;
    DIR *p_dir = opendir(dir);
    if (p_dir == NULL) {
        *path_count = 0;
        collect_error(OPEN_LOG_DIR_FAILED, dir);
        return NULL;
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tv.tv_sec -= day_span * (24 * 60 * 60);
    char *suffix = init_file_name_suffix(&tv.tv_sec);
    char **paths = init_string_array();
    int count = 0;
    while ((file = readdir(p_dir)) != NULL) {
        if (false == has_suffix(file->d_name, suffix))
            continue;
        char *fullpath = init_filepath(dir, file->d_name);
        paths = insert_string(paths, fullpath);
        count ++;
        free(fullpath);
    }
    
    free(suffix);
    closedir(p_dir);
    
    *path_count = count;
    
    if (count == 0) {
        release_string_array(paths);
        return NULL;
    } else {
        return paths;
    }
}

void clear_default_invalid_log_files(log *log) {
    clear_invalid_log_files(log->info->dir, log_file_valid_time_sec, log);
}

void clear_invalid_log_files(const char * dir, float valid_time_sec, log *log) {
    struct dirent *file;
    DIR *p_dir = opendir(dir);
    if (p_dir == NULL) {
        collect_error(OPEN_LOG_DIR_FAILED, dir);
        return;
    }
    while ((file = readdir(p_dir)) != NULL) {
        if (has_suffix_lg(file->d_name) == false)
            continue;
        char *fullpath = init_filepath(dir, file->d_name);
        if (NULL != log && strcmp(log->info->log_to_file, fullpath) == 0)
            continue;
        struct stat *p_stat = malloc(sizeof(struct stat));
        stat(fullpath, p_stat);
        if (time(NULL) - p_stat->st_mtime >= valid_time_sec) {
            remove(fullpath);
        }
        free(p_stat);
        free(fullpath);
    }
    closedir(p_dir);
}

char *init_log_file(const char *app_id, const char *dir, const unsigned int cipher_version) {
    char *current_log_path;
    time_t *rawtime = malloc(sizeof(time_t));
    time(rawtime);
    //通过当前日期构建日志文件后缀
    char *file_name_suffix = init_file_name_suffix(rawtime);
    //根据日志文件后缀获取当前日志文件的前缀 index
    unsigned int index = get_log_dir_filename_last_index(dir, file_name_suffix);
    //完整日志文件名 index + suffix
    char *fullname = init_full_filename(index, file_name_suffix);
    char *fullpath = init_filepath(dir, fullname);
    
    FILE *fp;
    fp = fopen(fullpath, "r");
    if (NULL != fp) {
        struct stat *p_stat = malloc(sizeof(struct stat));
        stat(fullpath, p_stat);
        // 当前日志文件超过日志大小，需要重新生成新的日志文件
        // 采用递增前缀 index 的方式
        if (p_stat->st_size >= max_log_file_size) {
            index ++;
            current_log_path = init_new_log_file(index, file_name_suffix, app_id, dir, cipher_version);
            free(fullpath);
        } else {
            int file_meta_version_on_file = read_log_file_meta_version(fp);
            if (file_meta_version_on_file == file_meta_version) {
                // 获取日志文件 meta 信息，并进行 check
                char *json_on_file = read_log_file_header_json(fp);
                char *current_json = init_file_meta_json_string(app_id, cipher_version);
                if (strcmp(json_on_file, current_json) == 0) {
                    current_log_path = fullpath;
                } else {
                    // meta 信息不一致时，需要生成新的日志文件
                    index ++;
                    current_log_path = init_new_log_file(index, file_name_suffix, app_id, dir, cipher_version);
                    free(fullpath);
                }
                free(json_on_file);
                free(current_json);
            } else {
                // meta_version 不一致，需要生成新的日志文件
                index ++;
                current_log_path = init_new_log_file(index, file_name_suffix, app_id, dir, cipher_version);
                free(fullpath);
            }
        }
        free(p_stat);
        fclose(fp);
    } else {
        // 文件不存在创建日志文件
        create_new_log_file(app_id, fullpath, cipher_version);
        current_log_path = fullpath;
    }
    
    free(file_name_suffix);
    free(fullname);
    free(rawtime);
    
    return current_log_path;
}

void log_to_file(unsigned char *bytes, size_t size, log *log, const char *file_name) {
    char *fullpath = init_filepath(log->info->dir, file_name);
    write_log_to_file(bytes, size, fullpath);
    free(fullpath);
}
