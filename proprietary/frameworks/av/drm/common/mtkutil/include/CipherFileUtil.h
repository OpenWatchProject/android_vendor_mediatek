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

#ifndef CIPHERFILEUTIL_H_
#define CIPHERFILEUTIL_H_

#include <utils/String8.h>
#include <stdio.h>

namespace android
{

class CryptoHelper;

class CipherFileUtil
{
public:
    /*
     * Read {byte_num} bytes from cipher file (DCF) referred by {fd}.
     *
     * Parameter:
     *     fd:          linux file descriptor
     *     buf:         buffer for storing the bytes read (after decryption).
     *     byte_num:    the desired number of bytes to read.
     *     offset:      the start position to read (from the beginning of REAL content)
     *     data_offset: the position of the beginning of content in whole DCF file.
     *     data_size:   the size of content (in bytes).
     *     hlp:         the CryptoHelper object used for decryption.
     *
     * Return value:
     *     The actual number of bytes which have been read (after decryption).
     *     0 or -1 if the reading operation fails.
     *
     * Remarks:
     *     1. The DCF file, is consist of the header & the content. the function
     *        only reads desired bytes from the content part, and the decrypted
     *        raw data is returned in {buf}.
     *
     *     2. DCF file uses AES128CBC encryption. Although there's a 16-bytes IV (Initial Vector)
     *        at the beginning of the content part, you are really specifying the {offset} from
     *        the BEGINNING OF the raw data (not encrypted), not considering the IV.
     */
    static int CipherFileRead(int fd, void* buf, int byte_num, int offset,
                              int data_offset, int data_size, CryptoHelper& hlp);

    // Add for Decode image with open decrypt seesion with ashmem
    static int CipherFileRead(char* data, void* buf, int byte_num, int offset,
                                   int data_offset, int data_size, CryptoHelper& hlp);

    static int CipherFileDump(int fd, int data_offset, int data_size, CryptoHelper& hlp); // deprecated

    static int CipherFileDump(String8 file_name, int fd, int data_offset, int data_size, CryptoHelper& hlp);

    /*
     * Encrypt the data of the file referred by {origin} and save the encryted data to the file referred by {out}.
     *
     * Parameter:
     *     origin:         file to be encrypted.
     *     data_offset:    the offset of data to be encrypted.
     *     data_size:      the length of data to be encrypted.
     *     out:            file where to save the encryted data.
     *     hlp:            the CryptoHelper object used for encryption.
     *     iv:             initial vector
     *
     * Return value:
     *     0 for succeed while -1 for failure.
     */
    static int CipherFileInstall(FILE* origin, int data_offset, int data_size, FILE* out,
                                 CryptoHelper& hlp, unsigned char* iv);
private:
    static bool isEnableDebug();
private:
    static bool sDebug;
};

} // namespace android

#endif /* CIPHERFILEUTIL_H_ */
