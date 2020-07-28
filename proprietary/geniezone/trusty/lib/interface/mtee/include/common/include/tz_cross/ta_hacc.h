/*
* Copyright (c) 2015 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __TRUSTZONE_SEJ_TA__
#define __TRUSTZONE_SEJ_TA__

#define TZ_CRYPTO_TA_UUID   "0d5fe516-821d-11e2-bdb4-d485645c4311"

/* Data Structure for Test TA */
/* You should define data structure used both in REE/TEE here
   N/A for Test TA */

/* Command for Test TA */
#define TZCMD_HACC_INIT     0
#define TZCMD_HACC_INTERNAL 1
#define TZCMD_SECURE_ALGO   2

typedef struct _ta_crypto_data_ {
    unsigned int    size;
    unsigned char   bAC;
    unsigned int    user;     /* HACC_USER */
    unsigned char   bDoLock;
    unsigned int    aes_type; /* AES_OPS */
    unsigned char   bEn;
} ta_crypto_data;

typedef struct _ta_secure_algo_data_ {
    unsigned char  direction;
    unsigned int   contentAddr;
    unsigned int   contentLen;
    unsigned char *customSeed;
    unsigned char *resText;
} ta_secure_algo_data;

#endif /* __TRUSTZONE_SEJ_TA_TEST__ */
