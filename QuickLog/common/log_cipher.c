//
//  log_cipher.c
//  QuickLog
//
//  Created by jimmy on 2021/9/15.
//

#include "log_cipher.h"
#include <stdlib.h>
#include <string.h>

aes_cfb* init_aes_cfb_cipher_context(const unsigned char *key, size_t key_length) {
    aes_cfb *context = malloc(sizeof(aes_cfb));
    memset(context, 0, sizeof(aes_cfb));
    if (key && key_length > 0) {
        //从第二个参数所指的内存地址的起始位置开始拷贝第三个参数个字节到第一个参数所指的内存地址的起始位置中
        memcpy(context->key, key, (key_length > AES_KEY_LENGTH) ? AES_KEY_LENGTH: key_length);
        memcpy(context->vector, context->key, AES_KEY_LENGTH);
        AES_set_encrypt_key(context->key, AES_KEY_BITSIZE_LENGTH, &context->aes_key);
    }
    return  context;
}

void reset_aes_cfb_cipher_context(aes_cfb *context) {
    context->number = 0;
    memcpy(context->vector, context->key, AES_KEY_LENGTH);
}

void log_encrypt_aes_cfb(const unsigned char *input, unsigned char *output, size_t length, aes_cfb *context) {
    if (!input || !output || length == 0 || !context) {
        return;
    }
    //数组是指针
    AES_cfb128_encrypt(input, output, length, &context->aes_key, context->vector, &context->number, AES_ENCRYPT);
}

void log_decrypt_aes_cfb(const unsigned char *input, unsigned char *output, size_t length, aes_cfb *context) {
    if (!input || !output || length == 0 || !context) {
        return;
    }
    AES_cfb128_encrypt(input, output, length, &context->aes_key, context->vector, &context->number, AES_DECRYPT);
}
