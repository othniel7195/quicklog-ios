//
//  log_cipher.h
//  QuickLog
//
//  Created by jimmy on 2021/9/15.
//

#ifndef log_cipher_h
#define log_cipher_h

#include <stdio.h>
#include "openssl/aes_cfb128.h"

#define AES_KEY_LENGTH 16
#define AES_KEY_BITSIZE_LENGTH 128

typedef struct {
    unsigned char vector[AES_KEY_LENGTH];
    unsigned char key[AES_KEY_LENGTH];
    AES_KEY aes_key;
    int number;
} aes_cfb;

aes_cfb* init_aes_cfb_cipher_context(const unsigned char *key, size_t key_length);

void reset_aes_cfb_cipher_context(aes_cfb *context);

void log_encrypt_aes_cfb(const unsigned char *input, unsigned char *output, size_t length, aes_cfb *context);

void log_decrypt_aes_cfb(const unsigned char *input, unsigned char *output, size_t length, aes_cfb *context);

#endif /* log_cipher_h */
