//
//  log_inflater.h
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#ifndef log_inflater_h
#define log_inflater_h

#include <zlib.h>

z_stream *init_uncompression_object(void);
int uncompress_log_buf(Bytef *dest, uLongf *dest_len, Bytef *source, uLong source_len, uInt avail_out, z_stream *stream);
void dealloc_compression_object(z_stream *deflater);

#endif /* log_inflater_h */
