//
//  log_inflater.c
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#include "log_inflater.h"
#include <stdlib.h>
#include <string.h>

z_stream * init_uncompression_object(void) {
    z_stream * stream = malloc(sizeof(z_stream));
    memset(stream, 0, sizeof(z_stream));
    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = Z_NULL;
    int err = inflateInit2_(stream, -MAX_WBITS, ZLIB_VERSION, sizeof(z_stream));
    if (err != Z_OK) {
      free(stream);
      return NULL;
    }
    return stream;
}

int uncompress_log_buf(Bytef *dest, uLongf *dest_len, Bytef *source, uLong source_len, uInt avail_out, z_stream *stream) {
    if (NULL != stream) {
      stream->next_in = (Bytef *)source;
      stream->avail_in = (uInt)source_len;
      stream->next_out = dest;
      stream->avail_out = avail_out;
      int err = inflate(stream, Z_SYNC_FLUSH);
      *dest_len = avail_out - stream->avail_out;
      return err;
    } else {
      return Z_ERRNO;
    }
}

void dealloc_compression_object(z_stream * deflater) {
    inflateEnd(deflater);
    free(deflater);
}