/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#define DEBUG_LOG_TAG "HDR_Utils"
#define LOG_TAG DEBUG_LOG_TAG

#include "HDR.h"

#include <thread>
#include <vector>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h> // mkdir

#include <cutils/properties.h>
//Android utils
#include <utils/ThreadDefs.h>
#include "BufferUtils.h"
#include <Debug.h>
#include <ImageBufferUtils.h>

using namespace android;
using namespace NS3Av3;

// round-robin time-sharing policy
#define THREAD_POLICY     SCHED_OTHER
// most threads run at normal priority
#define THREAD_PRIORITY   ANDROID_PRIORITY_NORMAL

// 0: 8bits; 1: 12 bits
#define HDR_AE_12BIT_FLARE 1

#define L1_CACHE_BYTES 32

//#define HDR_DEBUG_SKIP_MODIFY_POLICY

#define ALIGN_FLOOR(x,a)  ((x) & ~((a) - 1L))
#define ALIGN_CEIL(x,a)   (((x) + (a) - 1L) & ~((a) - 1L))

// 9-digit running number (range: 0 ~ 999999999)
MINT32 HDR::sSequenceNumber = 0;

// ---------------------------------------------------------------------------

sp<IHDR>
createInstance_MTKHDR_V1(char const * const pszShotName,
            uint32_t const u4ShotMode, int32_t const i4OpenId)
{
    return new NSCam::HDR(pszShotName, u4ShotMode, i4OpenId);
}

static MRect calCrop(MRect const& rSrc, MRect const& rDst, uint32_t ratio = 100)
{
    MRect rCrop;

    // srcW/srcH < dstW/dstH
    if (rSrc.s.w * rDst.s.h < rDst.s.w * rSrc.s.h)
    {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.w * rDst.s.h / rDst.s.w;
    }
    // srcW/srcH > dstW/dstH
    else if (rSrc.s.w * rDst.s.h > rDst.s.w * rSrc.s.h)
    {
        rCrop.s.w = rSrc.s.h * rDst.s.w / rDst.s.h;
        rCrop.s.h = rSrc.s.h;
    }
    // srcW/srcH == dstW/dstH
    else
    {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.h;
    }

    rCrop.s.w = ALIGN_FLOOR((rCrop.s.w * 100) / ratio, 2);
    rCrop.s.h = ALIGN_FLOOR((rCrop.s.h * 100) / ratio, 2);

    return rCrop;
}

static MUINT32 getAlignedSize(MUINT32 size)
{
    return (size + (L1_CACHE_BYTES)) & ~(L1_CACHE_BYTES - 1);
}

static MBOOL setThreadProp(int policy, int priority)
{
#ifndef HDR_DEBUG_SKIP_MODIFY_POLICY
    //@see http://www.kernel.org/doc/man-pages/online/pages/man2/sched_setscheduler.2.html
    //int const policy    = pthreadAttr_ptr->sched_policy;
    //int const priority  = pthreadAttr_ptr->sched_priority;
    //HDR_LOGD("policy=%d, priority=%d", policy, priority);

    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    switch (policy)
    {
        // non-real-time
        case SCHED_OTHER:
            sched_p.sched_priority = 0;
            sched_setscheduler(0, policy, &sched_p);
            // -20(high)~19(low)
            setpriority(PRIO_PROCESS, 0, priority);
            break;

        // real-time
        case SCHED_FIFO:
        default:
            // -20(high)~19(low)
            sched_p.sched_priority = priority;
            sched_setscheduler(0, policy, &sched_p);
    }
#endif

    return MTRUE;
}

static MBOOL getThreadProp(int* policy, int* priority)
{
#ifndef HDR_DEBUG_SKIP_MODIFY_POLICY
    //@see http://www.kernel.org/doc/man-pages/online/pages/man2/sched_setscheduler.2.html
    struct sched_param sched_p;
    *policy = ::sched_getscheduler(0);

    switch (*policy)
    {
        // non-real-time
        case SCHED_OTHER:
            // -20(high)~19(low)
            *priority = getpriority(PRIO_PROCESS, 0);
            break;

        // real-time
        case SCHED_FIFO:
        default:
            struct sched_param sched_p;
            ::sched_getparam(0, &sched_p);
            *priority = sched_p.sched_priority;
    }
#endif

    return MTRUE;
}

int HDR::dumpToFile(
        char const* fileName, unsigned char *vaddr, size_t size)
{
    ssize_t writtenBytes = 0;
    size_t doneBytes = 0;
    int loop = 0;

    HDR_LOGD("create file [%s]", fileName);
    int fd = ::open(fileName, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0)
    {
        HDR_LOGE("create file [%s] failed: %s", fileName, strerror(errno));
        return -errno;
    }

    HDR_LOGD("write %zu bytes to file [%s]", size, fileName);
    while (doneBytes < size)
    {
        writtenBytes = ::write(fd, vaddr + doneBytes, size - doneBytes);
        if (writtenBytes < 0)
        {
            HDR_LOGE("failed to write to file [%s]: %s", fileName, strerror(errno));
            break;
        }
        doneBytes += writtenBytes;
        loop++;
    }
    HDR_LOGD("%zu bytes have been written to file [%s] in %d passes",
            size, fileName, loop);

    ::close(fd);

    return 0;
}

// ---------------------------------------------------------------------------

HDR::HDR(char const * const /*pszShotName*/,
        uint32_t const u4ShotMode, int32_t const i4OpenId)
  : mu4OutputFrameNum(3),
    mu4W_yuv(0),
    mu4H_yuv(0),
    mu4W_small(0),
    mu4H_small(0),
    mu4W_se(0),
    mu4H_se(0),
    mu4W_dsmap(0),
    mu4H_dsmap(0),
    mRotPicWidth(0),
    mRotPicHeight(0),
    mOpenID(i4OpenId),
    mHDRShotMode(u4ShotMode),
    OriWeight(NULL),
    BlurredWeight(NULL),
    mrHdrCroppedResult(),
    mHdrSetBmapInfo(),
    mCompleteCB(NULL),
    mpCompleteCBUser(NULL),
    mCapturePolicy(SCHED_OTHER),
    mCapturePriority(ANDROID_PRIORITY_NORMAL),
    mforceBreak(MFALSE),
    mabort(MFALSE),
    mDebugMode(0)
{
    memset(&mExposureInputParam, 0, sizeof(HDRExpSettingInputParam_T));
    memset(&mExposureOutputParam, 0, sizeof(HDRExpSettingOutputParam_T));
    memset(&mShotParam, 0, sizeof(HDRProc_ShotParam));

    // get sensor type
    {
        //NSCam::SensorStaticInfo rSensorInfo;
        //querySensorInfo(mOpenID, rSensorInfo);

       // mSensorType = rSensorInfo.sensorType;
    }

    // initialize semaphores
    for (MINT32 i = 0; i < HDR_MAX_INPUT_FRAME; i++)
    {
        sem_init(&mSourceImgBufSem[i], 0, 0);
        sem_init(&mSmallImgBufSem[i], 0, 0);
    }

    sem_init(&mSEImgBufSem, 0, 0);
    sem_init(&mHdrWorkingBufSem, 0, 0);
    sem_init(&mWeightingBufSem, 0, 0);
    sem_init(&mDownSizedWeightMapBufAllocSem, 0, 0);
    sem_init(&mDownSizedWeightMapBufSem, 0, 0);
    sem_init(&mBlurredWeightMapBufSem, 0, 0);
    sem_init(&mBlendingBufSem, 0, 0);
}

HDR::~HDR()
{
    uninit();
}

MBOOL HDR::uninit()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    mu4W_yuv = 0;
    mu4H_yuv = 0;

    FUNCTION_LOG_END;
    return  ret;
}

MBOOL HDR::releaseSourceImgBufLocked(MUINT32 index)
{
    mpSourceImgBuf[index].clear();

    return MTRUE;
}

MBOOL HDR::releaseSmallImgBufLocked(MUINT32 index)
{
    mpSmallImgBuf[index].clear();

    return MTRUE;
}

MBOOL HDR::releaseSEImgBufLocked()
{
    for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        ImageBufferUtils::getInstance().deallocBuffer(mpSEImgBuf[i]);
        mpSEImgBuf[i].clear();
    }

    return MTRUE;
}

MUINT32 HDR::getHdrWorkingbufferSize()
{
    FUNCTION_LOG_START;

    // get working buffer size for HDR & FEFM
    MUINT32 workingBufferSize = mpHdrHal->HdrWorkingBuffSizeGet();
    HDR_LOGD("workingBufferSize(%d)", workingBufferSize);

    FUNCTION_LOG_END_MUM;
    return workingBufferSize;
}

MBOOL HDR::releaseHdrWorkingBufLocked()
{
    mpHdrWorkingBuf.clear();

    return MTRUE;
}

MBOOL HDR::requestOriWeightMapBuf()
{
    FUNCTION_LOG_START;

    Mutex::Autolock _l(mWeightingBufLock);

    MBOOL ret = MTRUE;
    // allocate memory for the original weighting map
    MUINT32 u4Size = sizeof(HDR_PIPE_WEIGHT_TBL_INFO*) * mu4OutputFrameNum;
    MUINT32 u4AlignedSize = getAlignedSize(u4Size);
    MUINT32 u4TableSize = sizeof(HDR_PIPE_WEIGHT_TBL_INFO);
    MUINT32 u4AlignedTableSize = getAlignedSize(u4TableSize);

    HDR_LOGD("[requestOriWeightMapBuf] u4Size(%d) -> u4AlignedSize(%d), " \
            "u4TableSize(%d) -> u4AlignedTableSize(%d)",
            u4Size, u4AlignedSize, u4TableSize, u4AlignedTableSize);

    OriWeight = (HDR_PIPE_WEIGHT_TBL_INFO**) memalign(L1_CACHE_BYTES, u4AlignedSize);
    MUINT32 bufferWidth = mu4W_yuv * mu4H_yuv / 4;
    if(OriWeight == nullptr) {
        HDR_LOGE("[requestOriWeightMapBuf] OriWeight is null!");
        ret = MFALSE;
        goto lbExit;
    }

    for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        OriWeight[i] =
            (HDR_PIPE_WEIGHT_TBL_INFO*) memalign(L1_CACHE_BYTES, u4AlignedTableSize);
        {
            OriWeight[i]->wt_table_width  = 0;
            OriWeight[i]->wt_table_height = 0;
            OriWeight[i]->wt_table_data = nullptr;
        }

        // allocate original weight map buffer
        ret = ImageBufferUtils::getInstance().allocBuffer(
                mWeightingBuf[i], bufferWidth, 1, eImgFmt_BLOB);
        mHdrSetBmapInfo.bmap_image_addr[i] = (MUINT8*) mWeightingBuf[i]->getBufVA(0);
    }

    mHdrSetBmapInfo.bmap_width  = mu4W_yuv / 2;
    mHdrSetBmapInfo.bmap_height = mu4H_yuv / 2;
    mHdrSetBmapInfo.bmap_image_size =
        mWeightingBuf[0]->getBufSizeInBytes(0) * mu4OutputFrameNum;

lbExit:
    if (!ret)
    {
        releaseOriWeightMapBufLocked();
    }

    FUNCTION_LOG_END;
    return ret;
}

// NOTE: Some info of OriWeightMap are needed when
// requestImageBuffer(HDR_BUFFER_BLURRED_WEIGHT_MAP),
// so must release it after requestImageBuffer(HDR_BUFFER_BLURRED_WEIGHT_MAP)
MBOOL HDR::releaseOriWeightMapBufLocked()
{
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    if (OriWeight)
    {
        for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
        {
            free(OriWeight[i]);
            ImageBufferUtils::getInstance().deallocBuffer(
                    mWeightingBuf[i]);
            mWeightingBuf[i].clear();
        }

        delete [] OriWeight;
        OriWeight = NULL;
    }

    FUNCTION_LOG_END;
    return ret;
}

// NOTE: must execute after OriWeight is gotten
MBOOL HDR::requestBlurredWeightMapBuf()
{
    FUNCTION_LOG_START;

    Mutex::Autolock _l(mBlurredWeightMapBufLock);

    MBOOL ret = MTRUE;
    // allocate memory for blurred weighting map
    MUINT32 u4Size = sizeof(HDR_PIPE_WEIGHT_TBL_INFO*) * mu4OutputFrameNum;
    MUINT32 u4AlignedSize = getAlignedSize(u4Size);
    MUINT32 u4TableSize = sizeof(HDR_PIPE_WEIGHT_TBL_INFO);
    MUINT32 u4AlignedTableSize = getAlignedSize(u4TableSize);

    HDR_LOGD("[requestBlurredWeightMapBuf] u4Size(%d) -> u4AlignedSize(%d), " \
            "u4TableSize(%d) -> u4AlignedTableSize(%d)",
            u4Size, u4AlignedSize, u4TableSize, u4AlignedTableSize);

    // check invalid setting
    for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        if ((OriWeight[i]->wt_table_width != 0) && (OriWeight[i]->wt_table_height != 0))
            continue;

        HDR_LOGE("invalid OriWeight[%d] w(%u) h(%u)",
                i, OriWeight[i]->wt_table_width, OriWeight[i]->wt_table_height);
        goto lbExit;
    }

    BlurredWeight = (HDR_PIPE_WEIGHT_TBL_INFO**) memalign(L1_CACHE_BYTES, u4AlignedSize);

    if(BlurredWeight == nullptr) {
        HDR_LOGE("[requestBlurredWeightMapBuf] BlurredWeight is null!");
        ret = MFALSE;
        goto lbExit;
    }
    for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        BlurredWeight[i] =
            (HDR_PIPE_WEIGHT_TBL_INFO*) memalign(L1_CACHE_BYTES, u4AlignedTableSize);

        BlurredWeight[i]->wt_table_width  = OriWeight[i]->wt_table_width;
        BlurredWeight[i]->wt_table_height = OriWeight[i]->wt_table_height;

        ret = ImageBufferUtils::getInstance().allocBuffer(
                mpBlurredWeightMapBuf[i],
                BlurredWeight[i]->wt_table_width,
                BlurredWeight[i]->wt_table_height,
                eImgFmt_Y8);
        BlurredWeight[i]->wt_table_data = (MUINT8*)mpBlurredWeightMapBuf[i]->getBufVA(0);
    }

lbExit:
    if (!ret)
    {
        releaseBlurredWeightMapBufLocked();
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::releaseBlurredWeightMapBufLocked()
{
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    if (BlurredWeight)
    {
        for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
        {
            free(BlurredWeight[i]);
            ImageBufferUtils::getInstance().deallocBuffer(
                    mpBlurredWeightMapBuf[i]);
            mpBlurredWeightMapBuf[i].clear();
        }

        delete [] BlurredWeight;
        BlurredWeight = NULL;
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::requestDownSizedWeightMapBuf()
{
    FUNCTION_LOG_START;

    Mutex::Autolock _l(mDownSizedWeightMapBufLock);

    MBOOL ret = MTRUE;

    // fit scaler ability (divided by 32 and then aligned to a multiple of 2)
    mu4W_dsmap = ALIGN_CEIL((OriWeight[0]->wt_table_width + 31) / 32, 2);
    mu4H_dsmap = ALIGN_CEIL((OriWeight[0]->wt_table_height + 31) / 32, 2);

    if ((mu4W_dsmap == 0) || (mu4H_dsmap == 0))
    {
        HDR_LOGE("invalid mu4W_dsmap(%u) mu4H_dsmap(%u)", mu4W_dsmap, mu4H_dsmap);
        goto lbExit;
    }

    for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        ret = ImageBufferUtils::getInstance().allocBuffer(
                mpDownSizedWeightMapBuf[i], mu4W_dsmap, mu4H_dsmap, eImgFmt_Y8);
    }

lbExit:
    if (!ret)
    {
        releaseDownSizedWeightMapBufLocked();
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::releaseDownSizedWeightMapBufLocked()
{
    for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        ImageBufferUtils::getInstance().deallocBuffer(
                mpDownSizedWeightMapBuf[i]);
        mpDownSizedWeightMapBuf[i].clear();
    }

    return MTRUE;
}

MBOOL HDR::releaseBlendingBufLocked()
{
    ImageBufferUtils::getInstance().deallocBuffer(mBlendingBuf);
    mBlendingBuf.clear();

    return MTRUE;
}

MBOOL HDR::requestImageBuffer(HDRBufferType type)
{
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    switch (type)
    {
        case HDR_BUFFER_SOURCE:
            ret = MFALSE;
            HDR_LOGE("source image should be set via addInputFrame(%d)", type);
            break;
        case HDR_BUFFER_SMALL:
            ret = MFALSE;
            HDR_LOGE("small image should be set via addInputFrame(%d)", type);
            break;
        case HDR_BUFFER_SE:
            {
                Mutex::Autolock _l(mSEImgBufLock);
                for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
                {
                    ret = ImageBufferUtils::getInstance().allocBuffer(
                            mpSEImgBuf[i], mu4W_se, mu4H_se, eImgFmt_Y8);
                }

                if (ret != MTRUE)
                {
                    HDR_LOGE("allocate se buffer failed");
                    releaseSEImgBufLocked();
                }
            }
            break;
        case HDR_BUFFER_WORKING:
            {
                Mutex::Autolock _l(mHdrWorkingBufLock);
                mpHdrWorkingBuf = new Memory(getHdrWorkingbufferSize());
                ret = (mpHdrWorkingBuf != NULL);

                if (ret != MTRUE)
                {
                    HDR_LOGE("allocate hdr working buffer failed");
                    releaseHdrWorkingBufLocked();
                }
            }
            break;
        case HDR_BUFFER_ORI_WEIGHT_MAP:
            ret = requestOriWeightMapBuf();
            break;
        case HDR_BUFFER_BLURRED_WEIGHT_MAP:
            ret = requestBlurredWeightMapBuf();
            break;
        case HDR_BUFFER_DOWNSIZED_WEIGHT_MAP:
            ret = requestDownSizedWeightMapBuf();
            break;
        case HDR_BUFFER_BLENDING:
            {
                Mutex::Autolock _l(mBlendingBufLock);
                ret = ImageBufferUtils::getInstance().allocBuffer(
                        mBlendingBuf,
                        mu4W_yuv * mu4H_yuv * 3 / 2, 1, eImgFmt_BLOB);

                if (ret != MTRUE)
                {
                    HDR_LOGE("allocate blending buffer failed");
                    releaseBlendingBufLocked();
                }
            }
            break;
        default:
            ret = MFALSE;
            HDR_LOGE("undefined hdr buffer type(%d)", type);
            break;
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::releaseImageBuffer(HDRBufferType type)
{
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    switch (type)
    {
        case HDR_BUFFER_SOURCE:
            {
                for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
                {
                    Mutex::Autolock _l(mSourceImgBufLock[i]);
                    releaseSourceImgBufLocked(i);
                }
            }
            break;
        case HDR_BUFFER_SMALL:
            {
                for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
                {
                    Mutex::Autolock _l(mSmallImgBufLock[i]);
                    releaseSmallImgBufLocked(i);
                }
            }
            break;
        case HDR_BUFFER_SE:
            {
                Mutex::Autolock _l(mSEImgBufLock);
                ret = releaseSEImgBufLocked();
            }
            break;
        case HDR_BUFFER_WORKING:
            {
                Mutex::Autolock _l(mHdrWorkingBufLock);
                ret = releaseHdrWorkingBufLocked();
            }
            break;
        case HDR_BUFFER_ORI_WEIGHT_MAP:
            {
                Mutex::Autolock _l(mWeightingBufLock);
                ret = releaseOriWeightMapBufLocked();
            }
            break;
        case HDR_BUFFER_BLURRED_WEIGHT_MAP:
            {
                Mutex::Autolock _l(mBlurredWeightMapBufLock);
                ret = releaseBlurredWeightMapBufLocked();
            }
            break;
        case HDR_BUFFER_DOWNSIZED_WEIGHT_MAP:
            {
                Mutex::Autolock _l(mDownSizedWeightMapBufLock);
                releaseDownSizedWeightMapBufLocked();
            }
            break;
        case HDR_BUFFER_BLENDING:
            {
                Mutex::Autolock _l(mBlendingBufLock);
                releaseBlendingBufLocked();
            }
            break;
        default:
            HDR_LOGE("undefined hdr buffer type(%d)", type);
            break;
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::CDPResize(
        IImageBuffer* pInputBuf, IImageBuffer* pOutputBuf, MUINT32 transform)
{
    MBOOL ret = MTRUE;

    sp<IImageBuffer> tempInfo[2];
    MUINT32 tempWidth[2];
    MUINT32 tempHeight[2];
    //
    MSize srcSize = pInputBuf->getImgSize();
    MUINT32 srcWidth = srcSize.w;
    MUINT32 srcHeight = srcSize.h;
    EImageFormat srcFormat = (EImageFormat)pInputBuf->getImgFormat();
    //
    MSize desSize = pOutputBuf->getImgSize();
    MUINT32 desWidth = desSize.w;
    MUINT32 desHeight = desSize.h;
    EImageFormat desFormat = (EImageFormat)pOutputBuf->getImgFormat();

    //init
    if((desWidth>32*srcWidth)
            || (desHeight>32*srcHeight)) {
        //prepare source
        tempWidth[0] = srcWidth;
        tempHeight[0] = srcHeight;
        MBOOL isFirstRun = MTRUE;
        tempInfo[0] = pInputBuf;

        while(1) {
            //prepare target
            HDR_LOGD("[CDPResize] - prepare target");
            tempWidth[1] = desWidth;
            tempHeight[1] = desHeight;

            while(tempWidth[1] > tempWidth[0]*32)
                tempWidth[1] = (tempWidth[1]+31)/32;
            while(tempHeight[1] > tempHeight[0]*32)
                tempHeight[1] = (tempHeight[1]+31)/32;
            tempWidth[1] = (tempWidth[1]+1)&~1;
            tempHeight[1] = (tempHeight[1]+1)&~1;
            HDR_LOGD("[CDPResize] - desWidth=%d desHeight=%d", desWidth, desHeight);
            HDR_LOGD("[CDPResize] - tempWidth[0]=%d tempHeight[0]=%d", tempWidth[0], tempHeight[0]);
            HDR_LOGD("[CDPResize] - tempWidth[1]=%d tempHeight[1]=%d", tempWidth[1], tempHeight[1]);

            //scale up - last round
            if(tempWidth[1]==desWidth && tempHeight[1]==desHeight) {
                HDR_LOGD("[CDPResize] - scale up - final round");
                MBOOL ret;
                //ret = CDPResize_simple(tempInfo[0], desMem, rotation);
                ret = CDPResize_simple(tempInfo[0].get(), pOutputBuf, transform);
                ImageBufferUtils::getInstance().deallocBuffer(tempInfo[0]);
                tempInfo[0] = NULL;
                return ret;
            }

            //scale up
            HDR_LOGD("[CDPResize] - scale up");
            if(!ImageBufferUtils::getInstance().allocBuffer(
                        tempInfo[1], tempWidth[1], tempHeight[1], srcFormat)) {
                ret = MFALSE;
                goto lbExit;
            }
            CDPResize_simple(tempInfo[0].get(), tempInfo[1].get());
            if(!isFirstRun)
            {
                ImageBufferUtils::getInstance().deallocBuffer(tempInfo[0]);
                tempInfo[0] = NULL;
            }
            tempWidth[0] = tempWidth[1];
            tempHeight[0] = tempHeight[1];
            tempInfo[0] = tempInfo[1];

            isFirstRun = MFALSE;
        }

    }

    ret = CDPResize_simple(pInputBuf, pOutputBuf, transform);
lbExit:
    return ret;
}

MBOOL HDR::CDPResize_simple(
        IImageBuffer* pInputBuf, IImageBuffer* pOutputBuf, MUINT32 transform)
{
    MBOOL ret = MTRUE;

    //
    MSize srcSize = pInputBuf->getImgSize();
    MUINT32 srcWidth = srcSize.w;
    MUINT32 srcHeight = srcSize.h;
    EImageFormat srcFormat = (EImageFormat)pInputBuf->getImgFormat();
    //
    MSize desSize = pOutputBuf->getImgSize();
    MUINT32 desWidth = desSize.w;
    MUINT32 desHeight = desSize.h;
    EImageFormat desFormat = (EImageFormat)pOutputBuf->getImgFormat();

    HDR_LOGD("[CDPResize] - srcMem=%p", pInputBuf);
    HDR_LOGD("[CDPResize] - srcWidth=%d, srcHeight=%d", srcWidth, srcHeight);
    HDR_LOGD("[CDPResize] - srcFormat=%d", srcFormat);
    HDR_LOGD("[CDPResize] - desMem=%p", pOutputBuf);
    HDR_LOGD("[CDPResize] - desWidth=%d, desHeight=%d", desWidth, desHeight);
    HDR_LOGD("[CDPResize] - desFormat=%d", desFormat);
    HDR_LOGD("[CDPResize] - transform=%d", transform);
    //MDP convert
    BufferUtils::mdpResizeAndConvert(pInputBuf, pOutputBuf);
lbExit:
    return ret;
}

MBOOL HDR::createSEImg()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        HDR_LOGD("[createSEImg] CDP(%d/%d)", i, mu4OutputFrameNum);
        ret = CDPResize(mpSmallImgBuf[i].get(), mpSEImgBuf[i].get());
    }

    if (mDebugMode)
    {
        for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
        {
            char szFileName[100];
            ::sprintf(szFileName, HDR_DUMP_PATH \
                    "%09d-%04d-4_mpSEImgBuf[%d]_%dx%d.y",
                    sSequenceNumber, mExposureOutputParam.i4RunningNumber,
                    i, mu4W_se, mu4H_se);
            mpSEImgBuf[i]->saveToFile(szFileName);
        }
    }

    FUNCTION_LOG_END;
    return ret;
}

// ---------------------------------------------------------------------------

MBOOL HDR::updateInfo()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.debug.camera.hdr.dump", value, "0");
    mDebugMode = atoi(value);
    HDR_LOGD_IF(mDebugMode == 1, "[updateInfo] debug mode ON");

    // make debug dump path
    if (mDebugMode)
    {
        int result = mkdir(HDR_DUMP_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
        if ((result == -1) && (errno != EEXIST)) {
            HDR_LOGW("make debug dump path %s failed", HDR_DUMP_PATH);
        }
    }
    // show capture policy & priority
    getThreadProp(&mCapturePolicy, &mCapturePriority);
    HDR_LOGD("[updateInfo] capture policy(%d) priority(%d)",
            mCapturePolicy, mCapturePriority);

    // set YUV buffer size
    mu4W_yuv = mShotParam.pictureSize.w;
    mu4H_yuv = mShotParam.pictureSize.h;
    HDR_LOGD("[updateInfo] HDR picture size(%dx%d)", mu4W_yuv, mu4H_yuv);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::prepare()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    if (mforceBreak)
    {
        HDR_LOGD("force break at %s", __FUNCTION__);
        return MFALSE;
    }

    ret = init();

    // create HDR working buffer asynchronously
    allocateProcessMemory();

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::addInputFrame(
        MINT32 frameIndex, const android::sp<IImageBuffer>& inBuffer)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    if ((frameIndex < 0) || (frameIndex > (mu4OutputFrameNum << 1)))
    {
        HDR_LOGE("[addInputFrame] invalid frameIndex(%d)", frameIndex);
        return MFALSE;
    }

    if (inBuffer == NULL)
    {
        HDR_LOGE("[addInputFrame] inBuffer is NULL");
        return MFALSE;
    }

    const MBOOL isMainYUV = ((frameIndex & 0x1) == 0) ? MTRUE : MFALSE;
    const MINT32 BUFFER_INDEX = frameIndex / 2;

    if (isMainYUV)
    {
        // main YUV
        mpSourceImgBuf[BUFFER_INDEX] = inBuffer;

        if (mDebugMode)
        {
            char szFileName[100];
            ::sprintf(szFileName, HDR_DUMP_PATH \
                    "%09d-%04d-1_mpSourceImgBuf[%d]_%dx%d.i420",
                    sSequenceNumber, mExposureOutputParam.i4RunningNumber,
                    BUFFER_INDEX, mu4W_yuv, mu4H_yuv);
            mpSourceImgBuf[BUFFER_INDEX]->saveToFile(szFileName);
        }

        sem_post(&mSourceImgBufSem[BUFFER_INDEX]);
    }
    else
    {
        // small Y8
        mpSmallImgBuf[BUFFER_INDEX] = inBuffer;

        if (mDebugMode)
        {
            char szFileName[100];
            ::sprintf(szFileName, HDR_DUMP_PATH \
                    "%09d-%04d-2_mpSmallImgBuf[%d]_%dx%d.y",
                    sSequenceNumber, mExposureOutputParam.i4RunningNumber,
                    BUFFER_INDEX, mu4W_small, mu4H_small);
            mpSmallImgBuf[BUFFER_INDEX]->saveToFile(szFileName);
        }

        sem_post(&mSmallImgBufSem[BUFFER_INDEX]);
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::process()
{
    // a workter thread that process HDR post-processing
    std::thread(HDR::hdrProcess, this).detach();

    return MTRUE;
}

MVOID HDR::hdrProcess(MVOID* arg)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    HDR *self = static_cast<HDR *>(arg);

    // set thread's name
    ::prctl(PR_SET_NAME, "HDRWorker", 0, 0, 0);
    HDR_LOGD("[hdrProcess] setThreadProp");
    setThreadProp(SCHED_OTHER, ANDROID_PRIORITY_FOREGROUND);

    // wait for HDR input buffers
    self->waitInputFrame();

    self->mforceBreak = !ret;
    ret = self->ImageRegistratoin();
    HDR_LOGE_IF(MTRUE != ret, "[hdrProcess] ImageRegistratoin failed");

    self->mforceBreak = !ret;
    ret = self->WeightingMapGeneration();
    HDR_LOGE_IF(MTRUE != ret, "[hdrProcess] WeightingMapGeneration failed");

    self->mforceBreak = !ret;
    ret = self->Blending();
    HDR_LOGE_IF(MTRUE != ret, "[hdrProcess] Blending failed");

    self->mforceBreak = !ret;
    ret = self->writeHDROutputFrame();
    HDR_LOGE_IF(MTRUE != ret, "[hdrProcess] writeHDROutputFrame failed");

    // notify the caller that HDR post-processing is done
    self->notify(ret);
}

MVOID HDR::notify(MBOOL ret) const
{
    AutoMutex l(mCompleteCBLock);

    if (mCompleteCB && mpCompleteCBUser)
    {
        mCompleteCB(mpCompleteCBUser, mHdrResult, ret);
        HDR_LOGD("hdrProcess mCompleteCB(%d)", ret);
    }
}

MBOOL HDR::release()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    // raise the flag to handle the case that
    // allocate memory task has not finished
    mabort    = MTRUE;
    sem_post(&mDownSizedWeightMapBufAllocSem);

    // start to release all buffer
    releaseImageBuffer(HDR_BUFFER_SOURCE);
    releaseImageBuffer(HDR_BUFFER_SMALL);

    releaseImageBuffer(HDR_BUFFER_SE);
    releaseImageBuffer(HDR_BUFFER_WORKING);

    releaseImageBuffer(HDR_BUFFER_ORI_WEIGHT_MAP);
    releaseImageBuffer(HDR_BUFFER_DOWNSIZED_WEIGHT_MAP);
    releaseImageBuffer(HDR_BUFFER_BLURRED_WEIGHT_MAP);

    releaseImageBuffer(HDR_BUFFER_BLENDING);

    FUNCTION_LOG_END;
    return ret;
}

MVOID HDR::waitInputFrame()
{
    HDR_TRACE_CALL();

    for (MINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        // TODO: use sem_timedwait() to have informative messages
        while(sem_wait(&mSourceImgBufSem[i]) == -1)
        {
            HDR_LOGW("system interrupt sem_wait!");
        }
        while(sem_wait(&mSmallImgBufSem[i]) == -1)
        {
            HDR_LOGW("system interrupt sem_wait!");
        }

    }
}

MBOOL HDR::ImageRegistratoin()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    if (mforceBreak)
    {
        HDR_LOGW("force break at %s", __FUNCTION__);
        return MFALSE;
    }

    MBOOL ret = MTRUE;

    // (1) normalization
    {
        HDR_TRACE_NAME("normalization");

        // normalize small images and put them back to SmallImg[]
        // NOTE: default rank image method
        ret = do_Normalization(1);
        if (ret != MTRUE)
        {
            HDR_LOGE("do normalization failed");
            goto lbExit;
        }
    }

    // (2) SE
    {
        HDR_TRACE_NAME("se");

        // wait for SEImg buffers
        while(sem_wait(&mSEImgBufSem) == -1)
        {
            HDR_LOGW("system interrupt sem_wait!");
        }

        // create SEImg (resize 3 Small Img to 3 SE Img)
        ret = createSEImg();
        if (ret != MTRUE)
        {
            HDR_LOGE("create SE image failed");
            goto lbExit;
        }

        // do SE to get GMV
        ret = do_SE();
        if (ret != MTRUE)
        {
            HDR_LOGE("go SE failed");
            goto lbExit;
        }
    }

    // (3) MAV
    {
        HDR_TRACE_NAME("mav");

        // wait for HDR working buffers
        while(sem_wait(&mHdrWorkingBufSem) == -1)
        {
            HDR_LOGW("system interrupt sem_wait!");
        }

        // do feature extraciton
        ret = do_FeatureExtraction();

        if (ret != MTRUE)
        {
            HDR_TRACE_NAME("ImageRegistratoin_again");

            HDR_LOGD("rank image method failed, try normalization again");
            // normalization method
            ret = do_Normalization(0);
            if (ret != MTRUE)
            {
                HDR_LOGE("do normalization failed");
                goto lbExit;
            }

            ret = createSEImg();
            if (ret != MTRUE)
            {
                HDR_LOGE("create SE image failed");
                goto lbExit;
            }

            ret = do_SE();
            if (ret != MTRUE)
            {
                HDR_LOGE("go SE failed");
                goto lbExit;
            }

            ret = do_FeatureExtraction();
            if (ret != MTRUE)
            {
                HDR_LOGE("do feature extraction failed");
                goto lbExit;
            }
        }
    }

lbExit:
    // release MAV working buffer
    releaseImageBuffer(HDR_BUFFER_SMALL);

    // release SEImg Buffers
    releaseImageBuffer(HDR_BUFFER_SE);

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::WeightingMapGeneration()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    if (mforceBreak)
    {
        HDR_LOGW("force break at %s", __FUNCTION__);
        // signal memory allocator thread to exit
        sem_post(&mDownSizedWeightMapBufAllocSem);
        return MFALSE;
    }

    MBOOL ret = MTRUE;

    // do alignment (including "Feature Matching" and "Weighting Map Generation")
    ret = do_Alignment();

    // wait for the original weighting map buffer
    while(sem_wait(&mWeightingBufSem) == -1)
    {
        HDR_LOGW("system interrupt sem_wait!");
    }

    // request original weighting table buffer and get original weighting map
    ret = ret && do_OriWeightMapGet();
    sem_post(&mDownSizedWeightMapBufAllocSem);

    // blur the weighting map by downsizing and then upsizing it

    // wait for the downsized weighting map buffer
    while(sem_wait(&mDownSizedWeightMapBufSem) == -1)
    {
        HDR_LOGW("system interrupt sem_wait!");
    }

    // downsize the original weighting map and then put it into DownSizedWeightMapBuf
    ret = ret && do_DownScaleWeightMap();

    // wait for the blurred weighting map buffer
    while(sem_wait(&mBlurredWeightMapBufSem) == -1)
    {
        HDR_LOGW("system interrupt sem_wait!");
    }

    //  upsize the downsized weighting map buffer and then put it into blurred weighting map
    ret = ret && do_UpScaleWeightMap();

    // release OriWeightMapBuf
    // NOTE: some info of OriWeightMap are needed when
    // requestImageBuffer(HDR_BUFFER_BLURRED_WEIGHT_MAP),
    // so must release it after requestBlurredWeightMapBuf()
    releaseImageBuffer(HDR_BUFFER_ORI_WEIGHT_MAP);

    // release DownSizedWeightMapBuf
    releaseImageBuffer(HDR_BUFFER_DOWNSIZED_WEIGHT_MAP);

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::Blending()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    if (mforceBreak)
    {
        HDR_LOGW("force break at %s", __FUNCTION__);
        return MFALSE;
    }

    MBOOL ret = MTRUE;

    // fusion
    {
        HDR_TRACE_NAME("fusion");

        while(sem_wait(&mBlendingBufSem) == -1)
        {
            HDR_LOGW("system interrupt sem_wait!");
        }

        // do fusion
        ret = do_Fusion();
    }

    // release the blurred weighting map
    releaseImageBuffer(HDR_BUFFER_BLURRED_WEIGHT_MAP);

    // get HDR cropped result image
    ret = ret && do_HdrCroppedResultGet();

    // release HDR working buffer
    releaseImageBuffer(HDR_BUFFER_WORKING);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::writeHDROutputFrame()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    if (mforceBreak)
    {
        HDR_LOGW("force break at %s", __FUNCTION__);
        return MFALSE;
    }

    MBOOL ret = MTRUE;

    sp<IImageBuffer>& hdrResult(mHdrResult);

    // create hdr result image buffer (I420 format)
    ret = ImageBufferUtils::getInstance().allocBuffer(
            hdrResult,
            mrHdrCroppedResult.output_image_width,
            mrHdrCroppedResult.output_image_height, eImgFmt_I420);
    if (hdrResult == NULL || ret != MTRUE)
    {
        HDR_LOGE("image buffer is NULL or allocate fail");
        return false;
    }

    // get buffer size and copy hdr result to image buffer
    size_t hdrResultSize = 0;
    MUINT8 *srcPtr = mrHdrCroppedResult.output_image_addr;
    for (size_t i = 0; i < hdrResult->getPlaneCount(); i++)
    {
        size_t planeBufSize = hdrResult->getBufSizeInBytes(i);
        void *dstPtr = (void *)hdrResult->getBufVA(i);
        MUINT8 offset = 0;
        memcpy(dstPtr, srcPtr, planeBufSize);

        srcPtr += planeBufSize;
        hdrResultSize += planeBufSize;
    }
    HDR_LOGD("hdr result YUV size(%zu)", hdrResultSize);

    // dump hdr result
    if (mDebugMode)
    {
        char szFileName[100];
        ::sprintf(szFileName, HDR_DUMP_PATH "%09d-%04d-8_hdrResult_%dx%d.i420",
                sSequenceNumber, mExposureOutputParam.i4RunningNumber,
                mrHdrCroppedResult.output_image_width,
                mrHdrCroppedResult.output_image_height);
        hdrResult->saveToFile(szFileName);
    }

lbExit:
    // release blending buffer
    releaseImageBuffer(HDR_BUFFER_BLENDING);

    FUNCTION_LOG_END;
    return ret;
}
#if 0
MBOOL HDR::getCaptureInfo(
        Vector<NS3Av3::CaptureParam_T>& vCapExpParam, MINT32& hdrFrameNum)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    ret = getCaptureExposureSettings(mExposureOutputParam);
    if (mExposureOutputParam.u4OutputFrameNum == 0)
    {
        HDR_LOGE("u4OutputFrameNum=%d, this should be 2 or 3",
                mExposureOutputParam.u4OutputFrameNum);
        return MFALSE;
    }

    // fill vCapExpParam with HDR target exposure condition
    CaptureParam_T tmpExpParam;
    for (MUINT32 i = 0; i < mExposureOutputParam.u4OutputFrameNum; i++)
    {
        tmpExpParam.u4Eposuretime = mExposureOutputParam.u4ExpTimeInUS[i];
        tmpExpParam.u4AfeGain     = mExposureOutputParam.u4SensorGain[i];
        tmpExpParam.u4IspGain     = 1024; // fix ISP gain to 1x
        tmpExpParam.u4FlareOffset = mExposureOutputParam.u1FlareOffset[i];

        vCapExpParam.push_back(tmpExpParam);

        HDR_LOGD("[getCaptureInfo] modify output frame(%u) parameters: " \
                "Exposuretime(%u) AfeGain(%u) IspGain(%u) FlareOffset(%u)",
                i, tmpExpParam.u4Eposuretime, tmpExpParam.u4AfeGain,
                tmpExpParam.u4IspGain, tmpExpParam.u4FlareOffset);
    }

    if (mSensorType == NSCam::SENSOR_TYPE_YUV)
    {
        // for yuv sensor - take 2 pictures, -1.0 & 1.5 ev
        HDR_LOGD("[getCaptureInfo] modify output frame's parameters: " \
                "YuvEvIdx(%d,%d)",
                vCapExpParam[0].i4YuvEvIdx, vCapExpParam[1].i4YuvEvIdx);
    }

    hdrFrameNum = mExposureOutputParam.u4OutputFrameNum;

    // record the number of HDR input frames
    mu4OutputFrameNum = mExposureOutputParam.u4OutputFrameNum;

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::getCaptureInfo(std::vector<HDRCaptureParam>& vCapParam)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    ret = getCaptureExposureSettings(mExposureOutputParam);
    if (mExposureOutputParam.u4OutputFrameNum == 0)
    {
        HDR_LOGE("u4OutputFrameNum=%d, this should be 2 or 3",
                mExposureOutputParam.u4OutputFrameNum);
        return MFALSE;
    }

    // Resizes the container so that it contains n elements
    vCapParam.resize(mExposureOutputParam.u4OutputFrameNum);

    // fill vCapParam with HDR target exposure and ISP conditions
    for (std::size_t i = 0; i < vCapParam.size(); i++)
    {
        HDRCaptureParam& tmpCapParam(vCapParam.at(i));
        CaptureParam_T& tmpExpParam(tmpCapParam.exposureParam);
        tmpExpParam.u4Eposuretime = mExposureOutputParam.u4ExpTimeInUS[i];
        tmpExpParam.u4AfeGain     = mExposureOutputParam.u4SensorGain[i];
        tmpExpParam.u4IspGain     = 1024; // fix ISP gain to 1x
        tmpExpParam.u4FlareOffset = mExposureOutputParam.u1FlareOffset[i];

        // LTM is controlled by exposure bracketing
        tmpCapParam.ltmMode = (mExposureOutputParam.u4BypassLTM[i] != 0) ?
            LTMMode::OFF : LTMMode::ON;

        HDR_LOGD("[getCaptureInfo] modify output frame(%zu) parameters: " \
                "Exposuretime(%u) AfeGain(%u) IspGain(%u) FlareOffset(%u) bypassLTM(%d)",
                i, tmpExpParam.u4Eposuretime, tmpExpParam.u4AfeGain,
                tmpExpParam.u4IspGain, tmpExpParam.u4FlareOffset,
                tmpCapParam.bypassLTM());
    }

    // record the number of HDR input frames
    mu4OutputFrameNum = mExposureOutputParam.u4OutputFrameNum;

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::getCaptureExposureSettings(
        HDRExpSettingOutputParam_T& exposureOutputParam)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    // YUV sensor
    if (mSensorType == NSCam::SENSOR_TYPE_YUV)
    {
        // for yuv sensor - take 2 pictures, -1.0 & 1.5 ev
        exposureOutputParam.u4OutputFrameNum = 2;
        // capture -1.0, 1.5 ev, 2^(1.5 - -1.0) * 1024 = 5793
        exposureOutputParam.u4FinalGainDiff[0] = 5793;
        // capture -1.0, 1.5 ev, 2^(1.5 - -1.0) * 1024 = 5793
        exposureOutputParam.u4FinalGainDiff[1] = 5793;
        exposureOutputParam.u4TargetTone = 150;
    }
    else
    {
#if HDR_AE_12BIT_FLARE
        // getHDRExpSetting() suggests flare offset is in 8Bit
        mExposureInputParam.u1FlareOffset0EV =
            mExposureInputParam.u1FlareOffset0EV >> 4;
#endif

        getHDRExpSetting(mExposureInputParam, exposureOutputParam);

#if HDR_AE_12BIT_FLARE
        for (MINT32 i = 0; i < HDR_MAX_INPUT_FRAME; i++)
        {
            exposureOutputParam.u1FlareOffset[i] =
                exposureOutputParam.u1FlareOffset[i] << 4;
        }
#endif
    }

    // dump input/output exposure settings here
    dumpExposureSetting(mExposureInputParam, exposureOutputParam);

lbExit:

    FUNCTION_LOG_END;
    return ret;
}
#endif

MBOOL HDR::init()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;
    HDR_PIPE_INIT_INFO rHdrPipeInitInfo;

    // configure HDR pipe init information for HDR HAL
    rHdrPipeInitInfo.u4OutputFrameNum = mu4OutputFrameNum;
    // init params for input
    rHdrPipeInitInfo.u4FinalGainDiff0 = 256 /*mExposureOutputParam.u4FinalGainDiff[0]*/;
    rHdrPipeInitInfo.u4FinalGainDiff1 = 256 /*mExposureOutputParam.u4FinalGainDiff[1]*/;
    rHdrPipeInitInfo.u4ImgW           = mu4W_yuv;
    rHdrPipeInitInfo.u4ImgH           = mu4H_yuv;
    rHdrPipeInitInfo.u4TargetTone     = 150 /*mExposureOutputParam.u4TargetTone*/;
    // NOTE:
    // rHdrPipeInitInfo.pSourceImgBufAddr is not used when initializing HDR HAL,
    // hdrProcess() will be stalled until input frames are added via addInputFrame()

    rHdrPipeInitInfo.u4SensorType = mSensorType;
    HDR_LOGD("rHdrPipeInitInfo.u4SensorType(%d)", rHdrPipeInitInfo.u4SensorType);

#if 0
    // we use default, so skip
    //HDR 2.3 version
    // set ISP gamma if needed
    {
        IHal3A *p3AHal = MAKE_Hal3A(
                mOpenID, LOG_TAG);
        if (!p3AHal)
        {
            HDR_LOGE("create 3A HAL failed");
            ret = MFALSE;
            goto lbExit;
        }

        S3ACtrl_GetIspGamma params;
        //query LUT size & enabled
        params.gamma_lut = NULL;
        p3AHal->send3ACtrl(
                E3ACtrl_GetIspGamma,
                reinterpret_cast<MINTPTR>(&params), 0);

        if (params.enabled == MFALSE)
        {
            HDR_LOGD("[init] - GGM disabled");
            rHdrPipeInitInfo.pIsp_gamma = NULL;
        }
        else
        {
            HDR_LOGD("[init] - GGM enabled");

            std::vector<MUINT32> ispGamma;

            // prepare LUT buffer and then get LUT data from 3A HAL
            ispGamma.resize(params.gamma_lut_size);
            params.gamma_lut = ispGamma.data();
            p3AHal->send3ACtrl(
                    E3ACtrl_GetIspGamma,
                    reinterpret_cast<MINTPTR>(&params), 0);
            HDR_LOGD("ISP Gamma size(%zu)", ispGamma.size());

            rHdrPipeInitInfo.pIsp_gamma = ispGamma.data();
            rHdrPipeInitInfo.pIsp_gamma_size = ispGamma.size();
        }

        p3AHal->destroyInstance(LOG_TAG);
    }
#endif
#if 0
    // we use default, so skip
    // HDR 2.0 version
    // set ISP gamma if needed
    MINT32 ispGamma[ISP_GAMMA_SIZE];
    MBOOL isGGMEnabled = MFALSE;
    {
        IHal3A *p3AHal = IHal3A::createInstance(
                IHal3A::E_Camera_3, mOpenID, LOG_TAG);
        if (!p3AHal)
        {
            HDR_LOGE("create 3A HAL failed");
            ret = MFALSE;
            goto lbExit;
        }

        p3AHal->send3ACtrl(
                E3ACtrl_GetIspGamma,
                (MINTPTR)(&ispGamma[0]), (MINTPTR)(&isGGMEnabled));
        if (isGGMEnabled == MFALSE)
        {
            HDR_LOGD("[init] - GGM disabled");
            rHdrPipeInitInfo.pIsp_gamma = NULL;
        } else {
            HDR_LOGD("[init] - GGM enabled");
            rHdrPipeInitInfo.pIsp_gamma = &ispGamma[0];
        }
        p3AHal->destroyInstance(LOG_TAG);
    }
#endif


    // create and initialize HDR HAL
    mpHdrHal = IHDRHAL::createInstance();
    if  (mpHdrHal == NULL)
    {
        HDR_LOGE("create HDR HAL failed");
        goto lbExit;
    }

    ret = mpHdrHal->init((void*)(&rHdrPipeInitInfo));
    if  (ret != MTRUE)
    {
        HDR_LOGE("HDR HAL init failed");
        goto lbExit;
    }

    // for small image buffer
    mpHdrHal->QuerySmallImgResolution(mu4W_small, mu4H_small);

    // for SE image buffer
    mpHdrHal->QuerySEImgResolution(mu4W_se, mu4H_se);

lbExit:
    if (!ret)
    {
        uninit();
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDR::allocateProcessMemory()
{
    // a worker thread that allocates HDR working buffers asynchronously
    std::thread(HDR::allocateProcessMemoryTask, this).detach();

    return MTRUE;
}

MVOID HDR::allocateProcessMemoryTask(MVOID* arg)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    HDR *self = static_cast<HDR *>(arg);

    // allocate buffers for SE
    ret = self->requestImageBuffer(HDR_BUFFER_SE);
    if (ret != MTRUE)
    {
        HDR_LOGE("can't alloc SE buffer");
    }
    sem_post(&self->mSEImgBufSem);

    // allocate buffers for MAV & HDR Core
    ret = self->requestImageBuffer(HDR_BUFFER_WORKING);
    if (ret != MTRUE)
    {
        HDR_LOGE("can't alloc HDR working buffer");
    }
    sem_post(&self->mHdrWorkingBufSem);

    // allocate buffers for weighting map
    ret = self->requestImageBuffer(HDR_BUFFER_ORI_WEIGHT_MAP);
    if (ret != MTRUE)
    {
        HDR_LOGE("can't alloc weighting map buffer");
    }
    sem_post(&self->mWeightingBufSem);

    // wait for the downsized weighting map buffer
    while(sem_wait(&self->mDownSizedWeightMapBufAllocSem) == -1)
    {
        HDR_LOGW("system interrupt sem_wait!");
    }

    // if release() is called, goto exit directly
    if(self->mabort == MTRUE) {
        HDR_LOGW("allocat memory is aborted!");
        goto lbExit;
    }

    // allocate buffers for down-sized weighting map
    ret = self->requestImageBuffer(HDR_BUFFER_DOWNSIZED_WEIGHT_MAP);
    if (ret != MTRUE)
    {
        HDR_LOGE("can't alloc down-sized weighting map buffer");
    }
    sem_post(&self->mDownSizedWeightMapBufSem);

    // allocate buffers for blurred weighting map
    ret = self->requestImageBuffer(HDR_BUFFER_BLURRED_WEIGHT_MAP);
    if (ret != MTRUE)
    {
        HDR_LOGE("can't alloc blurred weighting map buffer");
    }
    sem_post(&self->mBlurredWeightMapBufSem);

    // allocate buffers for blending
    ret = self->requestImageBuffer(HDR_BUFFER_BLENDING);
    if (ret != MTRUE)
    {
        HDR_LOGE("can't alloc blending buffer");
    }
    sem_post(&self->mBlendingBufSem);

lbExit:
    FUNCTION_LOG_END;
}

MBOOL HDR::setParam(MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2)
{
    // TODO: need to use mutex to protect

    switch (paramId)
    {
        case HDRProcParam_Set_sensor_size:
            mSensorSize.w = iArg1;
            mSensorSize.h = iArg2;
            break;
        case HDRProcParam_Set_sensor_type:
            mSensorType = iArg1;
            break;
        case HDRProcParam_Set_AOEMode:
            mExposureInputParam.u4AOEMode = iArg1;
            break;
        case HDRProcParam_Set_MaxSensorAnalogGain:
            mExposureInputParam.u4MaxSensorAnalogGain = iArg1;
            break;
        case HDRProcParam_Set_MaxAEExpTimeInUS:
            mExposureInputParam.u4MaxAEExpTimeInUS = iArg1;
            break;
        case HDRProcParam_Set_MinAEExpTimeInUS:
            mExposureInputParam.u4MinAEExpTimeInUS = iArg1;
            break;
        case HDRProcParam_Set_ShutterLineTime:
            mExposureInputParam.u4ShutterLineTime = iArg1;
            break;
        case HDRProcParam_Set_MaxAESensorGain:
            mExposureInputParam.u4MaxAESensorGain = iArg1;
            break;
        case HDRProcParam_Set_MinAESensorGain:
            mExposureInputParam.u4MinAESensorGain = iArg1;
            break;
        case HDRProcParam_Set_ExpTimeInUS0EV:
            mExposureInputParam.u4ExpTimeInUS0EV = iArg1;
            break;
        case HDRProcParam_Set_SensorGain0EV:
            mExposureInputParam.u4SensorGain0EV = iArg1;
            break;
        case HDRProcParam_Set_FlareOffset0EV:
            mExposureInputParam.u1FlareOffset0EV = iArg1;
            break;
        case HDRProcParam_Set_GainBase0EV:
            mExposureInputParam.i4GainBase0EV = iArg1;
            break;
        case HDRProcParam_Set_LE_LowAvg:
            mExposureInputParam.i4LE_LowAvg = iArg1;
            break;
        case HDRProcParam_Set_SEDeltaEVx100:
            mExposureInputParam.i4SEDeltaEVx100 = iArg1;
            break;
        case HDRProcParam_Set_Histogram:
            {
                MUINT32 * pHistogram = (MUINT32*) iArg1;
                if (NULL != pHistogram)
                {
                    memcpy((void*) mExposureInputParam.u4Histogram,
                           (void*) pHistogram,
                            sizeof(mExposureInputParam.u4Histogram));
                }
            }
            break;
        case HDRProcParam_Set_DetectFace:
            mExposureInputParam.bDetectFace = iArg1;
            break;
        case HDRProcParam_Set_FlareHistogram:
            {
                MUINT32 * pFlareHistogram = (MUINT32*) iArg1;
                if (NULL != pFlareHistogram)
                {
                    memcpy((void*) mExposureInputParam.u4FlareHistogram,
                           (void*) pFlareHistogram,
                            sizeof(mExposureInputParam.u4FlareHistogram));
                }
            }
            break;
            #if 0
        case HDRProcParam_Set_PLineAETable:
            {
                MUINT32 * pPLineAETable = (MUINT32*) iArg1;
                if (NULL != pPLineAETable)
                {
                    memcpy((void*) &mExposureInputParam.PLineAETable,
                           (void*) pPLineAETable,
                            sizeof(mExposureInputParam.PLineAETable));
                }
                mExposureInputParam.i4aeTableCurrentIndex = iArg2;
            }
            break;
            #endif
        case HDRProcParam_Set_SequenceNumber:
            sSequenceNumber = iArg1;
            break;
        default:
            HDR_LOGE("[setParam] undefined paramId(%u)", paramId);
            return MFALSE;
    }

    HDR_LOGV("[setParam] paramId(%u) Arg1(%" PRIxPTR ") Arg2(%" PRIxPTR ")",
            paramId, iArg1, iArg2);

    return MTRUE;
}

MBOOL HDR::setShotParam(void const* pParam)
{
    if (NULL == pParam)
    {
        HDR_LOGE("[setShotParam] NULL pParam");
        return MFALSE;
    }

    // TODO: need to use mutex to protect

    HDRProc_ShotParam *in_param = (HDRProc_ShotParam *)pParam;

    memcpy(&mShotParam, in_param, sizeof(HDRProc_ShotParam));

    return MTRUE;
}

MBOOL HDR::getParam(MUINT32 paramId, MUINT32& rArg1, MUINT32& rArg2)
{
    switch (paramId)
    {
        case HDRProcParam_Get_src_main_format:
            rArg1 = eImgFmt_I420;
            break;
        case HDRProcParam_Get_src_main_size:
            rArg1 = mu4W_yuv;
            rArg2 = mu4H_yuv;
            break;
        case HDRProcParam_Get_src_small_format:
            rArg1 = eImgFmt_Y8;
            break;
        case HDRProcParam_Get_src_small_size:
            rArg1 = mu4W_small;
            rArg2 = mu4H_small;
            break;
        default:
            HDR_LOGE("[getParam] undefined paramId(%u)", paramId);
            return MFALSE;
    }

    HDR_LOGV("[getParam] paramId(%u) Arg1(%u) Arg2(%u)",
            paramId, rArg1, rArg2);

    return MTRUE;
}

MVOID HDR::setCompleteCallback(
        HDRProcCompleteCallback_t completeCB, MVOID* user)
{
    AutoMutex l(mCompleteCBLock);

    if (completeCB)
    {
        mCompleteCB = completeCB;
        mpCompleteCBUser = user;
        HDR_LOGD("HDRProc callback is set(%#" PRIxPTR ")",
                reinterpret_cast<uintptr_t>(completeCB));
    }
}

MINT32 HDR::queryHDRAlgoType(void)
{
    return YuvDomainHDR;
}

// ---------------------------------------------------------------------------

HDR::Memory::Memory(const size_t size)
    : mSize(size)
    , mAllocatedMemory(malloc(size))
{
    HDR_LOGW_IF(nullptr == mAllocatedMemory,
            "pointer to the allocated memory is NULL, input size(%zu)", mSize);
}

HDR::Memory::~Memory()
{
    HDR_LOGD_IF(mSize != 0,
            "free allocated memory(%#" PRIxPTR ")",
            reinterpret_cast<uintptr_t>(mAllocatedMemory));

    mSize = 0;
    free(mAllocatedMemory);
}
