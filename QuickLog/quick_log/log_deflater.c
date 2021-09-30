//
//  log_deflater.c
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#include <stdlib.h>
#include <zlib.h>
#include <stdbool.h>
#include <string.h>
#include "log.h"

z_stream *init_compression_object(void) {
    z_stream * stream = malloc(sizeof(z_stream));
    memset(stream, 0, sizeof(z_stream));
    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = Z_NULL;
    int err = deflateInit2(stream, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (Z_OK != err) {
        collect_error(INIT_ZLIB_FAILED, "deflateInit2 failed");
        free(stream);
        return NULL;
    }
    return stream;
}

int compress_log_buf(Bytef *dest, uInt *dest_len, Bytef *source, uInt source_len, uInt avail_out, z_stream *stream) {
    if (NULL == stream) {
        collect_error(INIT_ZLIB_FAILED, "deflate error");
        return Z_ERRNO;
    }
    stream->next_in = source;
    stream->avail_in = source_len;
    stream->next_out = dest;
    stream->avail_out = avail_out;
    int err = deflate(stream, Z_SYNC_FLUSH);
    *dest_len = avail_out - stream->avail_out;
    if (Z_OK != err) {
        collect_error(COMPRESS_ERROR_WITH_ZLIB, "deflate error");
    }
    return err;
}

void dealloc_compression_object(z_stream * deflater) {
    deflateEnd(deflater);
    free(deflater);
}

void reset_compression_object(log *log) {
    dealloc_compression_object(log->deflater);
    log->deflater = init_compression_object();
    log->init_compress_status = true;
}
