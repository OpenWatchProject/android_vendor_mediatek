/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_NDEBUG 1
#define LOG_TAG "DrmMtkUtil/CryptoHelper"
#include <utils/Log.h>

#include <CryptoHelper.h>
#include <DrmTypeDef.h>

#include <openssl/evp.h>
#include <openssl/err.h>

#include <string.h>

using namespace android;

CryptoHelper::CryptoHelper(CIPHER cipher, BYTE* key, int is_enc) : m_key(NULL), m_isEnc(is_enc)
{
    EVP_CIPHER* evp_cipher = getCipher(cipher);
    EVP_CIPHER_CTX_init(&m_ctx);
    EVP_CipherInit_ex(&m_ctx, evp_cipher, NULL, NULL, NULL, is_enc);

    int keyLen = EVP_CIPHER_CTX_key_length(&m_ctx);
    m_key = new BYTE[keyLen];
    memcpy(m_key, key, keyLen); // should make sure those data in {key} buffer are enough.
}

CryptoHelper::~CryptoHelper()
{
    EVP_CIPHER_CTX_cleanup(&m_ctx);
    if (NULL != m_key)
    {
        delete[] m_key;
    }
}

int CryptoHelper::cipherKeyLen(CIPHER cipher)
{
    return getCipher(cipher)->key_len;
}

int CryptoHelper::cipherIvLen(CIPHER cipher)
{
    return getCipher(cipher)->iv_len;
}

int CryptoHelper::cipherBlkLen(CIPHER cipher)
{
    return getCipher(cipher)->block_size;
}

// return 1 for success with {len_out} bytes of data placed in {buf_out};
// return 0 for failure and no data result is available.
int CryptoHelper::doCryption(BYTE* buf_in, int len_in, BYTE* buf_out, int& len_out, BYTE* iv, bool is_final /* =false */)
{
    // if for RC4 cryption, IV may be NULL
    if (len_in <= 0 || buf_in == NULL || buf_out == NULL)
    {
        ALOGE("doCryption() : error parameter: len_in:[%d], buf_in:[%p], buf_out:[%p]",
                len_in, buf_in, buf_out);
        return 0;
    }
    ALOGV("doCryption() : len_in:[%d], buf_in:[%p], buf_out:[%p], is_final:[%d]",
            len_in, buf_in, buf_out, (int)is_final);

    // the iv is changed each time (AES128CBC), so we need to reinit cipher
    if (NULL != iv)
    {
        EVP_CipherInit_ex(&m_ctx, NULL, NULL, m_key, iv, m_isEnc);
    }

    BYTE* bp = buf_out;
    int length = 0;
    // call encryption / decryption operation
    if (!EVP_CipherUpdate(&m_ctx, bp, &length, buf_in, len_in))
    {
        ALOGE("doCryption() : error with [EVP_CipherUpdate]");
        goto Error;
    }
    len_out = length;
    ALOGV("doCryption() : [EVP_CipherUpdate] result length:[%d]", length);

    if (is_final) // if it is the final part of data, need to deal with "padding"
    {
        bp += length; // move the pointer
        length = 0;
        if (!EVP_CipherFinal_ex(&m_ctx, bp, &length))
        {
            ALOGE("doCryption() : error with [EVP_CipherFinal_ex]");
            goto Error;
        }
        len_out += length;
        ALOGV("doCryption() : [EVP_CipherFinal_ex] result length:[%d]", length);
    }

    ALOGV("doCryption() : return 1 for success");
    return 1;

Error:
    ALOGE("doCryption() : return 0 for failure");
    return 0;
}

int CryptoHelper::getKeyLen(void)
{
    return EVP_CIPHER_CTX_key_length(&m_ctx);
}

int CryptoHelper::getIvLen(void)
{
    return EVP_CIPHER_CTX_iv_length(&m_ctx);
}

int CryptoHelper::getBlkSize(void)
{
    return EVP_CIPHER_CTX_block_size(&m_ctx);
}

int CryptoHelper::desiredOutBufSize(int len_in)
{
    return calc_cipher_length(len_in);
}

// deprecated
int CryptoHelper::desiredInDataSize(int req_out_len)
{
    if (m_isEnc == 1) // for encryption, the original data is enough
    {
        return req_out_len;
    }
    else // for decryption, in order to get X bytes result, you should pass in
         // ((X / block_size + 1) * block_size) bytes of data.
    {
        int block_size = EVP_CIPHER_CTX_block_size(&m_ctx);
        return (req_out_len / block_size + 1) * block_size;
    }
}

int CryptoHelper::calc_cipher_length(int len_in)
{
    int block_size = EVP_CIPHER_CTX_block_size(&m_ctx);

    if (len_in < block_size) // not enough for 1 block
    {
        return block_size;
    }

    int block_cnt = len_in / block_size;
    int cipher_length = block_cnt * block_size;

    int block_final = len_in - cipher_length;
    if (block_final >= 0)
    {
        cipher_length += block_size;
    }

    return cipher_length;
}

EVP_CIPHER* CryptoHelper::getCipher(CIPHER cipher)
{
    switch (cipher)
    {
        case CIPHER_AES128CBC:
            return (EVP_CIPHER*)EVP_aes_128_cbc();
            break;
        case CIPHER_RC4:
            return (EVP_CIPHER*)EVP_rc4();
            break;
        default:
            break;
    }

    return NULL;
}
