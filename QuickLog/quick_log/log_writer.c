//
//  log_writer.c
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#include <stdio.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include "log.h"
#include "../log_constants.h"

#pragma mark - mmap

static int unmap_file(mmap_log_buffer *map_file) {
    return munmap(map_file->buffer->bytes, map_file->buffer->offset) == 0;
}

static void clear_mmap_file(mmap_log_buffer *map_file) {
    map_file->buffer->offset = 0;
    map_file->buffer->total_size = 0;
    map_file->handle = -1;
    free(map_file->buffer);
    free(map_file->file_path);
}

static void close_mmap_file(mmap_log_buffer *map_file) {
    unmap_file(map_file);
    if (map_file->handle > 0) {
        close(map_file->handle);
    }
    clear_mmap_file(map_file);
}

static bool open_mmap_file(char *mmap_filepath, mmap_log_buffer *mmap_file) {
    // Open file
    int flags = O_RDWR;
    bool file_exist = false;
    if (-1 == access(mmap_filepath, F_OK)) {
        flags |= (O_CREAT | O_TRUNC);
    } else {
        file_exist = true;
    }
    int handle = open(mmap_filepath, flags, S_IRWXU);
    
    if (handle < 0) {
        return false;
    }
    
    // Set file size
    if (false == file_exist) {
        int result = ftruncate(handle, buffer_total_size);
        if (result == -1) {
            close(handle);
            return false;
        }
    }
    
    // Determine file size
    unsigned int total_size;
    struct stat *_stat = malloc(sizeof(struct stat));
    if (-1 != fstat(handle, _stat)) {
        total_size = (unsigned int)_stat->st_size;
        free(_stat);
    } else {
        close(handle);
        free(_stat);
        return false;
    }
    
    // MMAP
    void *data = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
    if (data == MAP_FAILED) {
        close(handle);
        return false;
    }
    common_log_buffer *log_buffer = malloc(sizeof(common_log_buffer));
    log_buffer->bytes = (unsigned char *)(char *)data;
    log_buffer->offset = 0;
    log_buffer->total_size = total_size;
    
    mmap_file->buffer = log_buffer;
    mmap_file->file_path = mmap_filepath;
    mmap_file->handle = handle;
    
    set_file_attr_protection_none(mmap_filepath);
    
    return true;
}

#pragma mark - writer

void init_log_writer(const char *dir, log *log) {
    mmap_log_buffer * _mmap_log_buffer = malloc(sizeof(mmap_log_buffer));
    memset(_mmap_log_buffer, 0, sizeof(mmap_log_buffer));
    char * mmap_file_path = init_filepath(dir, "QuickLog.mmap");
    if (true == open_mmap_file(mmap_file_path, _mmap_log_buffer)) {
        log->log_buffer = _mmap_log_buffer;
        log->use_mmap = true;
        flush_log_buffer(log, true);
    } else {
        collect_error(USE_MEMORY_LOG, "mmap init failed");
        
        free(_mmap_log_buffer);
        free(mmap_file_path);
        unsigned char * bytes = malloc(buffer_total_size);
        memset(bytes, 0, buffer_block_length);
        memory_log_buffer * _memory_log_buffer = malloc(sizeof(memory_log_buffer));
        common_log_buffer * log_buffer = malloc(sizeof(common_log_buffer));
        log_buffer->bytes = bytes;
        log_buffer->offset = 0;
        log_buffer->total_size = buffer_block_length;
        _memory_log_buffer->buffer = log_buffer;
        log->log_buffer = _memory_log_buffer;
        log->use_mmap = false;
    }
    write_log_buffer_header(log);
}

void deinit_log_writer(log *log) {
    flush_log_buffer(log, false);
    if (true == log->use_mmap) {
        close_mmap_file(log->log_buffer);
    } else {
        free(((memory_log_buffer *)log->log_buffer)->buffer->bytes);
        free(((memory_log_buffer *)log->log_buffer)->buffer);
    }
    free(log->log_buffer);
}
