/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>

#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion

#include <vpu.h>                        // interface for vpu stream

#include <cutils/properties.h>  // For property_get().
#include <ctime>

#include <sys/types.h>

#undef LOG_TAG
#define LOG_TAG "[vpuDynamicTest]"

#define MY_LOGV(fmt, arg...)        printf(LOG_TAG "[ver]" fmt , ##arg)
#define MY_LOGD(fmt, arg...)        printf(LOG_TAG "[dbg]" fmt , ##arg)
#define MY_LOGI(fmt, arg...)        printf(LOG_TAG "[info]" fmt , ##arg)
#define MY_LOGW(fmt, arg...)        printf(LOG_TAG "[warn]" fmt , ##arg)
#define MY_LOGE(fmt, arg...)        printf(LOG_TAG "[err]" fmt , ##arg)

typedef unsigned long long  UINT64;
typedef long long           INT64;
typedef unsigned int        UINT32;
typedef long                INT32;
typedef unsigned char       UINT8;
typedef char                INT8;
typedef bool                BOOL;

#define VPU_CORE_SUPPORT 2
#define DYNAMIC_LOAD_PATH_CORE0 "/data/local/tmp/vpuDynamicLoadAlgo/core0/vpu_dynamic_algo_test"
#define DYNAMIC_LOAD_PATH_CORE1 "/data/local/tmp/vpuDynamicLoadAlgo/core1/vpu_dynamic_algo_test"
#define DYNAMIC_ALGO_NAME "user_helloInVpu"

static int getTestAlgo(const char *path, char **algoBuf, unsigned long &len)
{
    FILE *file;
    char *buf;
    unsigned int fileLen = 0;

    if(path == nullptr)
    {
        return -1;
    }

    /* open file */
    file = fopen(path, "rb");
    if(!file)
    {
        MY_LOGE("[create] open %s failed\n",path);
        return -1;
    }

    /* get file length */
    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* allocate buffer */
    buf = (char *)malloc(fileLen + 1);
    if(buf == 0)
    {
        MY_LOGE("[create] alloc memory for algo failed\n");
        fclose(file);
        return -1;
    }

    fread(buf, fileLen, 1, file);

    *algoBuf = buf;
    len = fileLen;

    fclose(file);

    return 0;
}

int main(int argc, char **argv)
{
    VpuStream *stream = nullptr;
    VpuAlgo *algo = nullptr;
    VpuCreateAlgo algoFd0, algoFd1;
    VpuRequest *request = nullptr, *deRequest = nullptr;
    char *algoBuf0 = nullptr, *algoBuf1 = nullptr;
    unsigned long len;
    int ret = 0, i = 0, idx = 0;
    uint32_t input;

    int loopCount = 1;

    MY_LOGI("[dynamicAlgo] test start, algo(hello world)...\n");
    MY_LOGI("[dynamicAlgo] input test loop count...\n");
    scanf("%u", &input);

    /* create stream inst */
    stream = VpuStream::createInstance();
    if(stream == nullptr)
    {
        MY_LOGE("[dynamicAlgo] create stream failed\n");
        return -1;
    }

    /* get algo buf from disk by file operation */
    if(getTestAlgo(DYNAMIC_LOAD_PATH_CORE0, &algoBuf0, len))
    {
        MY_LOGE("[dynamicAlgo] get algo buffer failed\n");
        ret = -1;
        goto vpu_get_disk_algo_fail;
    }
    if(getTestAlgo(DYNAMIC_LOAD_PATH_CORE1, &algoBuf1, len))
    {
        MY_LOGE("[dynamicAlgo] get algo buffer failed\n");
        ret = -1;
        goto vpu_get_disk_algo_fail;
    }
    MY_LOGE("[dynamicAlgo] create algo(%s) ok, len(%d)\n", DYNAMIC_ALGO_NAME, len);

    /* create dynamic algo for vpu */
    algoFd0 = stream->createAlgo(DYNAMIC_ALGO_NAME, algoBuf0, len, 0x0);
    if(!algoFd0)
    {
        MY_LOGE("[dynamicAlgo] create algo failed\n");
        ret = -1;
        goto vpu_get_algo_fail;
    }
    algoFd1 = stream->createAlgo(DYNAMIC_ALGO_NAME, algoBuf1, len, 0x1);
    if(!algoFd0)
    {
        MY_LOGE("[dynamicAlgo] create algo failed\n");
        ret = -1;
        goto vpu_get_algo_fail;
    }

    /* get specific algo */
    algo = stream->getAlgo(DYNAMIC_ALGO_NAME);
    if(algo == nullptr)
    {
        MY_LOGE("[dynamicAlgo] get algo failed\n");
        ret = -1;
        goto vpu_get_algo_fail;
    }

    /* get request from algo */
    request = stream->acquire(algo, eModuleTypeCv);
    if(request == nullptr)
    {
        MY_LOGE("[dynamicAlgo] acquire request from algo failed\n");
        ret = -1;
        goto vpu_acquire_request_fail;
    }

    /* add buffer to request */

    for(i = 0; i < input; i++)
    {
        idx = 1<<(i%VPU_CORE_SUPPORT);

        MY_LOGI("[dynamicAlgo] test(%d)\n", i);
        /* enque request */
        if(stream->enque(request, idx) != true)
        {
            MY_LOGE("[dynamicAlgo] enque request failed\n");
            ret = -1;
            goto vpu_enque_fail;
        }

        /* deque request */
        deRequest = stream->deque();
        if(deRequest == nullptr)
        {
            MY_LOGE("[dynamicAlgo] deque request failed\n");
            ret = -1;
            goto vpu_enque_fail;
        }
    }

/* error handle */
vpu_enque_fail:
    stream->release(request);

vpu_acquire_request_fail:
    if(stream->freeAlgo(algoFd0) != true)
    {
        MY_LOGE("[dynamicAlgo] free algo failed");
    }
    if(stream->freeAlgo(algoFd1) != true)
    {
        MY_LOGE("[dynamicAlgo] free algo failed");
    }

vpu_get_algo_fail:
    delete algoBuf0;
    delete algoBuf1;

vpu_get_disk_algo_fail:
    delete(stream);

    return ret;
}


