//
//  quick_reader.c
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <zlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <inttypes.h>

#include "../quick_reader.h"
#include "../log_constants.h"
#include "../common/json_util.h"
#include "../common/log_cipher.h"
#include "log_inflater.h"

typedef struct
{
    int meta_version;
    off_t origin_file_size;
    off_t offset;
} log_read_file;

static bool read_log_file_header_file_type_supported(FILE *fp, log_read_file *file)
{
    fseek(fp, file->offset, SEEK_SET);
    char *file_type = malloc(3);
    memset(file_type, 0, 3);
    fread(file_type, 1, 2, fp);
    if (strcmp(file_type, file_header_format) == 0)
    {
        free(file_type);
        file->offset += 2;
        return true;
    }
    else
    {
        free(file_type);
        return false;
    }
}

static int read_log_file_buffer_length(FILE *fp, log_read_file *file)
{
    fseek(fp, file->offset, SEEK_SET);
    int length;
    fread(&length, 1, 4, fp);
    return length;
}

static bool read_log_file_header_meta_version_supported(FILE *fp, log_read_file *file)
{
    int version = read_log_file_buffer_length(fp, file);
    if (version == file_meta_version)
    {
        file->offset += 4;
        file->meta_version = version;
        return true;
    }
    else
    {
        return false;
    }
}

static int read_log_file_header_meta_json_length(FILE *fp, log_read_file *file)
{
    int log_length = read_log_file_buffer_length(fp, file);
    file->offset += 4;
    return log_length;
}

static void read_log_file_header_json_bytes(FILE *fp, log_read_file *file, int length, char *json)
{
    fseek(fp, file->offset, SEEK_SET);
    fread(json, 1, length, fp);
    file->offset += length;
}

static const char *read_log_file_header(const char *file_path, log_read_file *file, const char *(*get_key_with_file_json)(char *json))
{
    FILE *fp;
    fp = fopen(file_path, "r");
    if (NULL != fp)
    {
        if (true == read_log_file_header_file_type_supported(fp, file))
        {
            if (true == read_log_file_header_meta_version_supported(fp, file))
            {
                int log_length = read_log_file_header_meta_json_length(fp, file);
                char *json = malloc(log_length + 1);
                memset(json, 0, log_length + 1);
                read_log_file_header_json_bytes(fp, file, log_length, json);
                if (NULL != get_key_with_file_json)
                {
                    const char *key = get_key_with_file_json(json);
                    free(json);
                    return key;
                }
                else
                {
                    free(json);
                    return NULL;
                }
            }
        }
        fclose(fp);
    }
    return NULL;
}

static void d2tv(double d, struct timeval *tv)
{
    tv->tv_sec = d;
    tv->tv_usec = ((d - (double)tv->tv_sec) * 1000000.0);
}

static void transform_json_log_to_string(const char *json_log, char *log_info_str, size_t *log_str_len)
{
    size_t log_info_str_len = 0;
    cJSON *cjson = cJSON_Parse(json_log);
    if (NULL != cjson)
    {
        cJSON *level = cJSON_GetObjectItem(cjson, level_key);
        if (NULL != level && cJSON_String == level->type && NULL != level->valuestring)
        {
            strcat(log_info_str, "[");
            log_info_str_len += 1;
            strcat(log_info_str, level->valuestring);
            log_info_str_len += strlen(level->valuestring);
            strcat(log_info_str, "]");
            log_info_str_len += 1;
        }

        cJSON *gmt_offset = cJSON_GetObjectItem(cjson, gmtoff_key);
        double offset = 0;
        if (NULL != gmt_offset && cJSON_Number == gmt_offset->type && 0 != gmt_offset->valuedouble)
        {
            offset = gmt_offset->valuedouble;
        }
        cJSON *time = cJSON_GetObjectItem(cjson, time_key);
        if (NULL != time && cJSON_Number == time->type && 0 != time->valuedouble)
        {
            struct timeval *tv = malloc(sizeof(struct timeval));
            d2tv(time->valuedouble, tv);
            time_t tv_sec = tv->tv_sec + offset;
            struct tm t = *gmtime((const time_t *)&tv_sec);
            int year = 1900 + t.tm_year;
            int mon = 1 + t.tm_mon;
            int day = t.tm_mday;
            int hour = t.tm_hour;
            int min = t.tm_min;
            int sec = t.tm_sec;
            strcat(log_info_str, "[");
            log_info_str_len += 1;

            char year_str[5] = {0};
            int year_len = snprintf(year_str, 5, "%d", year);
            strcat(log_info_str, year_str);
            log_info_str_len += year_len;

            strcat(log_info_str, "-");
            log_info_str_len += 1;

            char mon_str[3] = {0};
            int mon_len = snprintf(mon_str, 3, "%02d", mon);
            strcat(log_info_str, mon_str);
            log_info_str_len += mon_len;

            strcat(log_info_str, "-");
            log_info_str_len += 1;

            char day_str[3] = {0};
            int day_len = snprintf(day_str, 3, "%02d", day);
            strcat(log_info_str, day_str);
            log_info_str_len += day_len;

            strcat(log_info_str, " ");
            log_info_str_len += 1;

            char hour_str[3] = {0};
            int hour_len = snprintf(hour_str, 3, "%02d", hour);
            strcat(log_info_str, hour_str);
            log_info_str_len += hour_len;

            strcat(log_info_str, ":");
            log_info_str_len += 1;

            char min_str[3] = {0};
            int min_len = snprintf(min_str, 3, "%02d", min);
            strcat(log_info_str, min_str);
            log_info_str_len += min_len;

            strcat(log_info_str, ":");
            log_info_str_len += 1;

            char sec_str[3] = {0};
            int sec_len = snprintf(sec_str, 3, "%02d", sec);
            strcat(log_info_str, sec_str);
            log_info_str_len += sec_len;

            strcat(log_info_str, ".");
            log_info_str_len += 1;

            char mico_sec_str[4] = {0};
            int mico_len = snprintf(mico_sec_str, 4, "%.3d", tv->tv_usec / 1000);

            strcat(log_info_str, mico_sec_str);
            log_info_str_len += mico_len;

            strcat(log_info_str, "]");
            log_info_str_len += 1;

            free(tv);
        }

        cJSON *pid = cJSON_GetObjectItem(cjson, process_id_key);
        cJSON *tid = cJSON_GetObjectItem(cjson, thread_id_key);
        cJSON *is_main = cJSON_GetObjectItem(cjson, is_main_thread_key);

        intmax_t pid_value = -1;
        if (NULL != pid && cJSON_Number == pid->type && 0 != pid->valuedouble)
        {
            pid_value = pid->valuedouble;
        }

        intmax_t tid_value = -1;
        if (NULL != tid && cJSON_Number == tid->type && 0 != tid->valuedouble)
        {
            tid_value = tid->valuedouble;
        }

        bool is_main_value = false;
        if (NULL != is_main && 1 == is_main->valueint)
        {
            is_main_value = true;
        }

        char thead_temp[50] = {0};
        int thead_len = snprintf(thead_temp, 50, "[%" PRIdMAX ", %" PRIdMAX "", pid_value, tid_value);
        strcat(log_info_str, thead_temp);
        log_info_str_len += thead_len;

        if (is_main_value)
        {
            strcat(log_info_str, "*");
            log_info_str_len += 1;
        }

        strcat(log_info_str, "]");
        log_info_str_len += 1;

        cJSON *tag = cJSON_GetObjectItem(cjson, tag_key);
        if (NULL != tag && cJSON_String == tag->type && NULL != tag->valuestring)
        {
            strcat(log_info_str, "[");
            log_info_str_len += 1;
            strcat(log_info_str, tag->valuestring);
            log_info_str_len += strlen(tag->valuestring);
            strcat(log_info_str, "]");
            log_info_str_len += 1;
        }

        cJSON *filename = cJSON_GetObjectItem(cjson, fileneme_key);
        cJSON *func_name = cJSON_GetObjectItem(cjson, func_name_key);
        cJSON *line = cJSON_GetObjectItem(cjson, line_key);

        strcat(log_info_str, "[");
        log_info_str_len += 1;

        if (NULL != filename && cJSON_String == filename->type && NULL != filename->valuestring)
        {
            strcat(log_info_str, filename->valuestring);
            log_info_str_len += strlen(filename->valuestring);
            strcat(log_info_str, ", ");
            log_info_str_len += 2;
        }

        if (NULL != func_name && cJSON_String == func_name->type && NULL != func_name->valuestring)
        {
            strcat(log_info_str, func_name->valuestring);
            log_info_str_len += strlen(func_name->valuestring);
            strcat(log_info_str, ", ");
            log_info_str_len += 2;
        }

        if (NULL != line && cJSON_Number == line->type && -1 != line->valueint)
        {
            char line_str[10] = {0};
            int line_len = snprintf(line_str, 10, "%d", line->valueint);
            strcat(log_info_str, line_str);
            log_info_str_len += line_len;
        }

        strcat(log_info_str, "]");
        log_info_str_len += 1;

        strcat(log_info_str, "[");
        log_info_str_len += 1;

        cJSON *log = cJSON_GetObjectItem(cjson, log_key);
        if (NULL != log && cJSON_String == log->type && NULL != log->valuestring)
        {
            strcat(log_info_str, log->valuestring);
            log_info_str_len += strlen(log->valuestring);
        }
        else
        {
            strcat(log_info_str, "null");
            log_info_str_len += 4;
        }

        strcat(log_info_str, "]");
        log_info_str_len += 1;

        log_info_str[log_info_str_len] = '\n';
        log_info_str_len += 1;

        cJSON_Delete(cjson);
    }

    *log_str_len = log_info_str_len;
}

static bool transform_log_v1(const char *file_path, const char *destination_path, const char *key, log_read_file *file, void (*take_log)(char *log, int format_type))
{
    FILE *writer;
    writer = fopen(destination_path, "w+");
    FILE *reader;
    reader = fopen(file_path, "r");
    
    if (NULL != writer && NULL != reader)
    {
        size_t key_len = strlen(key) + 1;
        unsigned char *cipher_key = malloc(key_len);
        memset(cipher_key, 0, key_len);
        memcpy(cipher_key, key, key_len - 1);
        aes_cfb *cfb = init_aes_cfb_cipher_context(cipher_key, key_len);
        free(cipher_key);
        z_stream *stream = init_uncompression_object();
        while (file->offset < file->origin_file_size)
        {
            fseek(reader, file->offset, SEEK_SET);
            int length;
            fread(&length, 1, 4, reader);
            if (length > 0)
            {
                file->offset += 4;
                fseek(reader, file->offset, SEEK_SET);

                uint8_t flag;
                fread(&flag, 1, 1, reader);
                if (compress_start_cipher_start == flag)
                {
                    reset_aes_cfb_cipher_context(cfb);
                    dealloc_compression_object(stream);
                    stream = init_uncompression_object();
                }
                else if (compress_start == flag)
                {
                    dealloc_compression_object(stream);
                    stream = init_uncompression_object();
                }
                else if (cipher_start == flag)
                {
                    reset_aes_cfb_cipher_context(cfb);
                }
                file->offset += 1;
                fseek(reader, file->offset, SEEK_SET);

                unsigned char *one_log_begin = malloc(length);
                memset(one_log_begin, 0, length);
                fread(one_log_begin, 1, length, reader);
                log_decrypt_aes_cfb(one_log_begin, one_log_begin, length, cfb);

                size_t *chunk_size = malloc(sizeof(size_t));
                unsigned char *uncompress_log_bytes = malloc(buffer_block_length);
                memset(uncompress_log_bytes, 0, buffer_block_length);
                uncompress_log_buf(uncompress_log_bytes, chunk_size, one_log_begin, length, buffer_block_length, stream);
                free(one_log_begin);

                if (*chunk_size > 0)
                {
                    if (NULL != take_log)
                    {
                        take_log((char *)uncompress_log_bytes, 1);
                    }
                    char *log_info_str = malloc(*chunk_size + 1);
                    size_t *log_info_len = malloc(sizeof(size_t));
                    memset(log_info_str, 0, *chunk_size + 1);
                    transform_json_log_to_string((char *)uncompress_log_bytes, log_info_str, log_info_len);
                    fwrite(log_info_str, *log_info_len, 1, writer);
                    free(log_info_str);
                    free(log_info_len);
                }
                else
                {
                    reset_aes_cfb_cipher_context(cfb);
                    dealloc_compression_object(stream);
                    stream = init_uncompression_object();
                }

                free(uncompress_log_bytes);
                free(chunk_size);

                file->offset += length;
            }
            else
            {
                printf("offset - %lld\n", file->offset);
                printf("origin_file_size - %lld\n", file->origin_file_size);
                break;
            }
        }
        fclose(writer);
        fclose(reader);
        dealloc_compression_object(stream);
        reset_aes_cfb_cipher_context(cfb);
        free(cfb);
        return true;
    }
    else
    {
        return false;
    }
}

static bool transform_log(const char *file_path, const char *destination_path, const char *key, log_read_file *file, void (*take_log)(char *log, int format_type))
{
    if (file_meta_version == file->meta_version)
    {
        return transform_log_v1(file_path, destination_path, key, file, take_log);
    }
    return false;
}

bool read_log_file(const char *file_path, const char *destination_path, const char *(*get_key_with_file_json)(char *json), void (*take_log)(char *log, int format_type))
{
    log_read_file *file = malloc(sizeof(log_read_file));
    memset(file, 0, sizeof(log_read_file));
    file->offset = 0;

    if (-1 == access(file_path, F_OK))
    {
        free(file);
        return false;
    }

    struct stat *p_stat = malloc(sizeof(struct stat));
    stat(file_path, p_stat);
    file->origin_file_size = p_stat->st_size;
    free(p_stat);

    if (file->origin_file_size <= 0)
    {
        free(file);
        return false;
    }

    const char *key = read_log_file_header(file_path, file, get_key_with_file_json);
    if (NULL != key)
    {
        return transform_log(file_path, destination_path, key, file, take_log);
    }

    return false;
}
