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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define  LOG_TAG "MemInfo"
#include <mtkcam/utils/sys/MemoryInfo.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

#include <functional>
#include <memory>
#include <regex>


#include <stdio.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);



#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

namespace NSCam {
namespace NSMemoryInfo {

int64_t
getFreeMemorySize()
{
    /* Usually, we cat /proc/meminfo and sum up these three columns, which can
     * represents the "available" memory that application can use.
     * 1. MemFree
     * 2. Buffers
     * 3. Cached
     */
    size_t size = 256 * sizeof(char);
    char chunk[256];

    std::unique_ptr<FILE, std::function<void(FILE*)> > fp(
            ::fopen("/proc/meminfo","r"),
            [](FILE* p){ ::fclose(p); });

    /* open fd failed */
    if (fp.get() == nullptr) {
        return -1;
    }

    /* read every line */
    int memFreeVal = -1;
    int buffersVal = -1;
    int cachedVal  = -1;
    char *line = chunk;

    while( ::getline(&line, &size, fp.get()) >= 0) {
        if( ::strncmp(line, "MemFree", 7) == 0) {
            ::sscanf(line, "%*s%d", &memFreeVal);
        }
        else if( ::strncmp(line, "Buffers", 7) == 0) {
            ::sscanf(line, "%*s%d", &buffersVal);
        }
        else if( ::strncmp(line, "Cached", 6) == 0) {
            ::sscanf(line, "%*s%d", &cachedVal);
        }
    }

    fp = nullptr; // close fp

    if (memFreeVal >= 0 && buffersVal >= 0 && cachedVal >= 0) {
#if 0
        MY_LOGD("memFreeVal=%d, buffersVal=%d, cachedVal=%d",
                memFreeVal, buffersVal, cachedVal);
#endif
        int64_t val = memFreeVal + buffersVal + cachedVal; // unit: kB
        return val *= 1024LL; // convert unit to Bytes
    }

    return -1;
}


int64_t
getSystemTotalMemorySize()
{
    /* Usually, we cat /proc/meminfo and sum up these three columns, which can
     * represents the "Total" memory that application can use.
     * 1. MemTotal
     */
    size_t size = 256 * sizeof(char);
    char chunk[256];

    std::unique_ptr<FILE, std::function<void(FILE*)> > fp(
            ::fopen("/proc/meminfo","r"),
            [](FILE* p){ ::fclose(p); });

    /* open fd failed */
    if (fp.get() == nullptr) {
        return -1;
    }

    /* read every line */
    int memTotal = -1;
    char *line = chunk;

    while( ::getline(&line, &size, fp.get()) >= 0) {
        if( ::strncmp(line, "MemTotal", 7) == 0) {
            ::sscanf(line, "%*s%d", &memTotal);
        }
    }

    fp = nullptr; // close fp

    if (memTotal >= 0) {
        int64_t val = memTotal; // unit: kB
        return val *= 1024LL; // convert unit to Bytes
    }

    return -1;
}
};
};
