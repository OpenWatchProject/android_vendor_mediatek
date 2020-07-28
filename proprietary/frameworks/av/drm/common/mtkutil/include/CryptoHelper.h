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

#ifndef CRYPTOHELPER_H_
#define CRYPTOHELPER_H_

#include "DrmTypeDef.h"
#include <openssl/evp.h>

namespace android
{

class CryptoHelper
{
public:
    enum CIPHER
    {
        CIPHER_AES128CBC,
        CIPHER_RC4
    };

public:
    /*
     * constructs a CryptoHelper object.
     *
     * Parameter:
     *     cipher: an enum CiPHER constant value representing the cipher to use. e.g. CIPHER_RC4
     *     key:    a BYTE buffer containing the cryption key value. SHOULD contain enough key data (16 bytes).
                     call static function: cipherKeyLen(CIPHER cipher) to find out the required length.
     *               you can release the buffer after the CryptoHelper object is created.
     *     is_enc: 1 for encryption & 0 for decryption.
     *
     * Remarks:
     *
     */
    CryptoHelper(CIPHER cipher, BYTE* key, int is_enc);
    ~CryptoHelper();

private:
    CryptoHelper(const CryptoHelper& copy);
    CryptoHelper& operator=(const CryptoHelper& other);

public:
    /* returns the length of KEY / IV / BLOCK (in bytes) according to different cipher */
    static int cipherKeyLen(CIPHER cipher);
    static int cipherIvLen(CIPHER cipher);
    static int cipherBlkLen(CIPHER cipher);

public:
    /*
     * Call this function to make encryption / decryption operations.
     * You may call this function repeatedly. When it's the very last part of data you are
     * operating on, call this function with parameter {is_final} as true.
     *
     * Parameter:
     *     [in] buf_in:   pointer of the input buffer.
     *     [in] len_in:   the actual data amount (in bytes) in the input buffer.
     *     [in] buf_out:  pointer of the output buffer.
     *     [out] len_out: returns the actual valid data amount (in bytes) in output buffer.
     *     [in] iv:       a BYTE buffer containing the Initial Vector (IV). SHOULD contain enough
     *                      IV data (16 bytes).
                          call getIvLen(), or static member function: cipherIvLen(CIPHER cipher)
                            to find out the required IV length.
     *     [in] is_final: true indicating that it is the last part of data. by default false.
     *
     * Return value:
     *     1 for success and 0 for failure.
     *
     * Remarks:
     *     Best practices:
     *     1. the {buf_in} pointer MUST align to the cipher block boundary, i.e. 0,
                16, 32, ... and you SHOULD pass in multiple cipher_block_size
                amount of data, i.e. 1024 bytes.
     *          (except it is the last part of data when doing encryption).
     *        you can call getBlkSize() or static function:
                cipherBlkLen(CIPHER cipher) to find out the size of a single block.
     *
     *     2. the {buf_out} should be large enough to contain possible output data.
     *          call desiredOutBufSize() to find out the length required.
     *
     *     3. when doing encryption on the last part of data, and specifying
     *          {is_final} as true, the function deals with "padding" and the result
     *          could be larger than the original input buffer.
     *
     *     4. when doing decryption on the last part of data, the function also
     *          deals with "padding" and remove it, so the {buf_out} buffer contains
     *          less valid data bytes than the original input buffer.
     */
    int doCryption(BYTE* buf_in, int len_in, BYTE* buf_out, int& len_out, BYTE* iv, bool is_final = false);

    /* returns the length of KEY / IV / BLK (in bytes) for this cipher */
    int getKeyLen(void);
    int getIvLen(void);
    int getBlkSize(void);

    /* returns the desired output buffer size according to the input (in bytes) */
    int desiredOutBufSize(int len_in);

    /* deprecated */
    int desiredInDataSize(int req_out_len);

private:
    int calc_cipher_length(int len_in);
    static EVP_CIPHER* getCipher(CIPHER cipher);

private:
    EVP_CIPHER_CTX m_ctx;
    BYTE* m_key;
    int m_isEnc;
};

} // namespace android

#endif /* CRYPTOHELPER_H_ */
