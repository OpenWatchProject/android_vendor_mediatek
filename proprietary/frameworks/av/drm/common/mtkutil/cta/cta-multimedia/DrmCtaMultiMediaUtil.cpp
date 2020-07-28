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
 * MediaTek Inc. (C) 2014. All rights reserved.
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


#define LOG_NDEBUG 0
#define LOG_TAG "DrmCtaMultiMediaUtil"
#include <utils/Log.h>

#include <DrmCtaMultiMediaUtil.h>

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>


using namespace android;

const int DrmCtaMultiMediaUtil::MULTI_MEDIA_LIST_CNT;
const char* DrmCtaMultiMediaUtil::MULTI_MEDIA_LIST[MULTI_MEDIA_LIST_CNT] = {
    "video/mp4",
    "video/3gp",
    "video/3gpp",
    "video/quicktime",
    "video/x-flv",
    "video/avi",
    "video/x-ms-wmv",
    "video/asf",
    "audio/x-ms-wma",
    "audio/x-wav",
    "audio/flac"
};

uint64_t DrmCtaMultiMediaUtil::ntoh64(uint64_t x) {
    return ((uint64_t)ntohl(x & 0xffffffff) << 32) | ntohl(x >> 32);
}

void DrmCtaMultiMediaUtil::MakeFourCCString(uint32_t x, char *s) {
    s[0] = x >> 24;
    s[1] = (x >> 16) & 0xff;
    s[2] = (x >> 8) & 0xff;
    s[3] = x & 0xff;
    s[4] = '\0';
}

bool DrmCtaMultiMediaUtil::isCompatibleBrand(uint32_t fourcc) {
    static const uint32_t kCompatibleBrands[] = {
        FOURCC('i', 's', 'o', 'm'),
        FOURCC('i', 's', 'o', '2'),
        FOURCC('a', 'v', 'c', '1'),
        FOURCC('3', 'g', 'p', '4'),
        FOURCC('m', 'p', '4', '1'),
        FOURCC('m', 'p', '4', '2'),

        // Won't promise that the following file types can be played.
        // Just give these file types a chance.
        FOURCC('q', 't', ' ', ' '),  // Apple's QuickTime
        FOURCC('M', 'S', 'N', 'V'),  // Sony's PSP

        FOURCC('3', 'g', '2', 'a'),  // 3GPP2
        FOURCC('3', 'g', '2', 'b'),
    };

    for (size_t i = 0;
         i < sizeof(kCompatibleBrands) / sizeof(kCompatibleBrands[0]);
         ++i) {
        if (kCompatibleBrands[i] == fourcc) {
            return true;
        }
    }

    return false;
}

ssize_t DrmCtaMultiMediaUtil::readAt(int fd, off64_t offset, void *data, size_t size) {
    ssize_t bytes;

    if(offset != lseek64(fd, offset, SEEK_SET)) {
        ALOGE("[ERROR][CTA5]readAt - lseek fail, reason[%s]", strerror(errno));
        return 0;
    }

    if(-1 == (bytes = read(fd, data, size))) {
        ALOGE("[ERROR][CTA5]readAt - read fail, reason[%s]", strerror(errno));
        return 0;
    }

    return bytes;
}

bool DrmCtaMultiMediaUtil::isExistedInMultiMediaList(const String8& mime) {
    bool result = false;
    for(int i = 0; i < DrmCtaMultiMediaUtil::MULTI_MEDIA_LIST_CNT; i++)
    {
        ALOGD("isExistedInMultiMediaList: compare [%s] with [%s]",
            DrmCtaMultiMediaUtil::MULTI_MEDIA_LIST[i],
            mime.string());
        if(0 == strcmp(mime.string(), DrmCtaMultiMediaUtil::MULTI_MEDIA_LIST[i]))
        {
            ALOGD("isExistedInMultiMediaList: supported.");
            result = true;
            break;
        }
    }
    return result;
}


