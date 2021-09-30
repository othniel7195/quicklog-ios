//
//  log_bytes.c
//  QuickLog
//
//  Created by jimmy on 2021/9/20.
//

#include <stdio.h>

#include "../log_constants.h"
#include "log.h"

void write_full_log_bytes(unsigned char *source, uint32_t source_len, unsigned int *offset, bool init_cipher, bool init_compress) {
    add_int_value_to_bytes(source, (uint32_t)*offset, source_len);
    
    if (true == init_cipher && true == init_compress) {
        source[*offset + 4] = compress_start_cipher_start;
    } else if (true == init_cipher) {
        source[*offset + 4] = cipher_start;
    } else if (true == init_compress) {
        source[*offset + 4] = compress_start;
    } else {
        source[*offset + 4] = no_cipher_no_compress;
    }
    *offset += source_len + 5;
}
