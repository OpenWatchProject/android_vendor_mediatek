/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef BASE64_H
#define BASE64_H
#include <stdio.h>
#include <stdlib.h>

unsigned char * base64_encode(const unsigned char *src, size_t len,
                  size_t *out_len);
unsigned char * base64_decode(const unsigned char *src, size_t len,
                  size_t *out_len);
unsigned char * base64_decode_to_str(const unsigned char *src, size_t len);
unsigned char * base64_encode_to_str(const unsigned char *src, size_t len);
unsigned char* byteArrayToHexStringB64(unsigned char* array,size_t length);
size_t hexStringToByteArrayB64(const unsigned char* hexString, unsigned char ** byte);

#define BASE64_UNUSED(x) (x)


#endif /* BASE64_H */
