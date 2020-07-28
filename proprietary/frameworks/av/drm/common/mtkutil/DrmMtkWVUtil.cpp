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
#define LOG_TAG "WV/DrmMtkWVUtil"
#include <utils/Log.h>

#include <drm/drm_framework_common.h>
#include <drm/DrmMtkWVUtil.h>

#include <ByteBuffer.h>
#include <CryptoHelper.h>
#include <DrmTypeDef.h>
#include <DrmUtil.h>

#include <utils/String8.h>

#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define AES128CBC_KEN_LEN 16
#define IV_LEN 16
#define KEYBOX_LEN 128

using namespace android;

const static String8 KB_LOCATION("/data/drm/");
const static String8 KB_DIR("/data/drm/wv/");
const static String8 KB_FILE("/data/drm/wv/kb.dat");



int DrmMtkWVUtil::loadKeybox(unsigned char* data, unsigned int offset, unsigned int length) {

    // open the kb data file
    FILE* fp = fopen(KB_FILE.string(), "rb");
    if (NULL == fp) {
        ALOGE("loadKeybox: failed to open kb data file, reason: [%s]", strerror(errno));
        return DRM_ERROR_UNKNOWN;
    }

    fseek(fp, 0, SEEK_SET);

    BYTE iv[IV_LEN];
    bzero(iv, sizeof(iv));
    if (IV_LEN != fread(iv, sizeof(BYTE), sizeof(iv), fp)) {
        ALOGE("loadKeybox: failed to read valid IV from kb data file, reason: [%s]", strerror(errno));
        fclose(fp);
        return DRM_ERROR_UNKNOWN;
    }

    BYTE data_in[KEYBOX_LEN + 16];
    bzero(data_in, sizeof(data_in));
    if ((KEYBOX_LEN + 16) != fread(data_in, sizeof(BYTE), sizeof(data_in), fp)) {
        ALOGE("loadKeybox: failed to read valid data from kb data file, reason: [%s]", strerror(errno));
        fclose(fp);
        return DRM_ERROR_UNKNOWN;
    }

    fclose(fp);

    ByteBuffer key(DrmUtil::getDrmKey());

    BYTE key_buf[AES128CBC_KEN_LEN];
    bzero(key_buf, sizeof(key_buf));
    memcpy(key_buf, key.buffer(), key.length());

    CryptoHelper hlp(CryptoHelper::CIPHER_AES128CBC, key_buf, 0);

    BYTE keybox[KEYBOX_LEN];
    bzero(keybox, sizeof(keybox));

    int out_len = 0;
    hlp.doCryption(data_in, sizeof(data_in), keybox, out_len, iv, true);

    int valid_cnt = ((int)(offset + length) <= out_len) ? length : (out_len - offset);
    valid_cnt = (valid_cnt < 0) ? 0 : valid_cnt;
    memcpy(data, keybox + offset, (size_t)valid_cnt);

    return DRM_NO_ERROR;
}

int DrmMtkWVUtil::saveKeybox(unsigned char* data, unsigned int length) {
    DrmUtil::checkDir(KB_DIR);
    FILE* fp = fopen(KB_FILE.string(), "wb");
    if (NULL == fp) {
        ALOGE("saveKeybox: failed to open kb data file for write, reason: [%s]", strerror(errno));
        return DRM_ERROR_UNKNOWN;
    }

    //chanage permission to avoid other and group users writing file
    int ret = chmod(KB_FILE.string(), 0644);
    if (ret != 0)
    {
        ALOGE("fchmod failed, reason: [%s]", strerror(errno));
    }

    fseek(fp, 0, SEEK_SET);

    ByteBuffer key(DrmUtil::getDrmKey());

    BYTE key_buf[AES128CBC_KEN_LEN];
    bzero(key_buf, sizeof(key_buf));
    memcpy(key_buf, key.buffer(), key.length());

    CryptoHelper hlp(CryptoHelper::CIPHER_AES128CBC, key_buf, 1);

    int out_size = hlp.desiredOutBufSize(length);
    BYTE* out = new BYTE[out_size];
    bzero(out, (size_t)out_size);

    String8 iv("4074BCC61602B5BE");
    int out_len = 0;
    hlp.doCryption(data, (int)length, out, out_len, (BYTE*)iv.string(), true);

    // write iv
    if (IV_LEN != fwrite(iv.string(), 1, iv.length(), fp)) {
        ALOGE("saveKeybox: failed to write IV to kb data file, reason: [%s]", strerror(errno));
        delete[] out;
        fclose(fp);
        return DRM_ERROR_UNKNOWN;
    }

    // write data
    if ((size_t)out_len != fwrite(out, sizeof(BYTE), (size_t)out_len, fp)) {
        ALOGE("saveKeybox: failed to write data to kb data file, reason: [%s]", strerror(errno));
        delete[] out;
        fclose(fp);
        return DRM_ERROR_UNKNOWN;
    }

    delete[] out;
    fclose(fp);
    return DRM_NO_ERROR;
}

String8 DrmMtkWVUtil::getDeviceId() {
    return DrmUtil::readId();
}

