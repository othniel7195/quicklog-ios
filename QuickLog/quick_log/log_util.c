//
//  log_util.c
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#include <stdio.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifdef ANDROID
#include <sys/endian.h>
#else
#include <sys/_endian.h>
#endif

#include "log.h"

#if TARGET_OS_IPHONE
#import <Foundation/Foundation.h>
#endif

bool set_file_attr_protection_none(const char *path) {
#if !TARGET_OS_IPHONE 
    return true;
#else
    NSString *path = [[NSString alloc] initWithUTF8String: path];
    NSFileManager* fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:path]) {
        [path release];
        return false;
    }
    NSDictionary* attr = [NSDictionary dictionaryWithObject:NSFileProtectionNone forKey:NSFileProtectionKey];
    BOOL ret = [fileManager setAttributes:attr ofItemAtPath:path error:nil];
    [path release];
    return ret;
#endif
}

double tv2ms(const struct timeval *tv) {
    return tv->tv_sec + (double)tv->tv_usec / 1000000.0;
}

bool has_suffix(const char *str, const char *suffix) {
    if (!str || !suffix) return false;
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    if (len_suffix > len_str) return false;
    return strncmp(str + len_str - len_suffix, suffix, len_suffix) == 0;
}

char *init_filepath(const char *dir, const char *filename) {
    char *separator = "/";
    size_t length = strlen(dir) + strlen(filename) + strlen(separator) + 1;
    char *fullpath = malloc(length);
    if (has_suffix(dir, separator) == false) {
        snprintf(fullpath, length, "%s%s%s", dir, separator, filename);
        return fullpath;
    } else {
        snprintf(fullpath, length, "%s%s", dir, filename);
        return fullpath;
    }
}

// ["string", ...],   string is   char *,  数组 is *, 二维数组
char **init_string_array(void) {
    char **empty = malloc(sizeof(char *));
    *empty = 0;
    return empty;
}

char **insert_string(char **arr, const char *str) {
    char **cpy = arr;
    int arr_size = 1;
    while(*cpy) {
        arr_size++;
        cpy++;
    }
    //重新分配内存空间
    arr = realloc(arr, (arr_size + 1) * sizeof(char *));
    const char *stringcpy = str;
    int stringlength = 0;
    while (*stringcpy) {
        stringlength++;
        stringcpy++;
    }
    arr[arr_size - 1] = malloc((stringlength + 1) * sizeof(char));
    
    strcpy(arr[arr_size - 1], str);
    arr[arr_size] = 0;
    return arr;
}

void release_string_array(char **arr) {
    char **cpy = arr;
    while(*cpy) {
        free(*cpy);
        cpy++;
    }
    free(arr);
}

void add_int_value_to_bytes(unsigned char *bytes, uint32_t bytes_start, uint32_t value) {
    unsigned int number = htonl(value);
    bytes[bytes_start] = (number >> 24) & 0xFF;
    bytes[bytes_start + 1] = (number >> 16) & 0xFF;
    bytes[bytes_start + 2] = (number >> 8) & 0xFF;
    bytes[bytes_start + 3] = number & 0xFF;
}

char *log_strdup(const char *s) {
    char *ret = strdup(s);
    if (!ret) {
        printf("Out of memory in log_strdup\n");
    }
    return ret;
}
