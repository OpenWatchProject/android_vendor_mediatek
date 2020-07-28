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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   MtkOmxVenc.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video Encoder component
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/

#include <signal.h>
#include <cutils/log.h>

#include <utils/Trace.h>
#include <utils/AndroidThreads.h>

#include "osal_utils.h"
#include "MtkOmxVenc.h"

#include <cutils/properties.h>
#include <sched.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVenc"

#include "OMX_IndexExt.h"

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_VIDEO
#endif//ATRACE_TAG

#define MTK_OMX_H263_ENCODER    "OMX.MTK.VIDEO.ENCODER.H263"
#define MTK_OMX_MPEG4_ENCODER   "OMX.MTK.VIDEO.ENCODER.MPEG4"
#define MTK_OMX_AVC_ENCODER     "OMX.MTK.VIDEO.ENCODER.AVC"
#define MTK_OMX_HEVC_ENCODER    "OMX.MTK.VIDEO.ENCODER.HEVC"
#define MTK_OMX_AVC_SEC_ENCODER "OMX.MTK.VIDEO.ENCODER.AVC.secure"
#define MTK_OMX_VP8_ENCODER     "OMX.MTK.VIDEO.ENCODER.VPX"

#define H264_MAX_BS_SIZE    1024*1024
#define HEVC_MAX_BS_SIZE    1024*1024
#define MP4_MAX_BS_SIZE     1024*1024
#define VP8_MAX_BS_SIZE     1024*1024

// Morris Yang 20120214 add for live effect recording [
#ifdef ANDROID_ICS
#include <ui/Rect.h>
//#include <ui/android_native_buffer.h> // for ICS
#include <android/native_window.h> // for JB
// #include <media/stagefright/HardwareAPI.h> // for ICS
#include <HardwareAPI.h> // for JB
//#include <media/stagefright/MetadataBufferType.h> // for ICS
#include <MetadataBufferType.h> // for JB

#include <hardware/gralloc.h>
#include <ui/gralloc_extra.h>
#endif
// ]
#define OMX_CHECK_DUMMY
#include "../../../omx/core/src/MtkOmxCore.h"

#include <poll.h>

#include <utils/Trace.h>
#include <utils/AndroidThreads.h>

////////for fence in M0/////////////
#include <ui/Fence.h>
#include <media/IOMX.h>
///////////////////end///////////

#define ATRACE_TAG ATRACE_TAG_VIDEO
#define USE_SYSTRACE

#define PROFILING 1

template<class T>
static OMX_BOOL checkOMXParams(T *params)
{
    if (params->nSize != sizeof(T)              ||
        params->nVersion.s.nVersionMajor != 1   ||
        params->nVersion.s.nVersionMinor != 0   ||
        params->nVersion.s.nRevision != 0       ||
        params->nVersion.s.nStep != 0) {
            return OMX_FALSE;
    }
    return OMX_TRUE;
}

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

static int64_t getTickCountUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec * 1000000LL + tv.tv_usec);
}

// MtkOmxBufQ [
MtkOmxBufQ::MtkOmxBufQ()
    : mId(MTK_OMX_VENC_BUFQ_INPUT),
      mPendingNum(0)
{
    INIT_MUTEX(mBufQLock);
    mBufQ.clear();
}

MtkOmxBufQ::~MtkOmxBufQ()
{
    DESTROY_MUTEX(mBufQLock);
    mBufQ.clear();
}

int MtkOmxBufQ::DequeueBuffer()
{
    int output_idx = -1;
    LOCK(mBufQLock);

    if (Size() <= 0)
    {
        UNLOCK(mBufQLock);
        return output_idx;
    }
#if CPP_STL_SUPPORT
    output_idx = *(mBufQ.begin());
    mBufQ.erase(mBufQ.begin());
#endif

#if ANDROID
    output_idx = mBufQ[0];
    mBufQ.removeAt(0);
#endif
    UNLOCK(mBufQLock);
    //MTK_OMX_LOGD("q:%d dequeue:%d", (int)mId, output_idx);
    return output_idx;
}

void MtkOmxBufQ::QueueBufferBack(int index)
{
    LOCK(mBufQLock);
#if CPP_STL_SUPPORT
    mBufQ.push_back(index);
#endif

#if ANDROID
    mBufQ.push(index);
#endif
    UNLOCK(mBufQLock);
}

void MtkOmxBufQ::QueueBufferFront(int index)
{
    LOCK(mBufQLock);
#if CPP_STL_SUPPORT
    mBufQ.push_front(index);
#endif

#if ANDROID
    mBufQ.insertAt(index, 0);
#endif
    UNLOCK(mBufQLock);
}

bool MtkOmxBufQ::IsEmpty()
{
#if CPP_STL_SUPPORT
    return (bool)mBufQ.empty();
#endif
#if ANDROID
    return (bool)mBufQ.isEmpty();
#endif
}

void MtkOmxBufQ::Push(int index)
{
#if CPP_STL_SUPPORT
    return mBufQ.push_back(index);
#endif

#if ANDROID
    return mBufQ.push(index);
#endif
}

void MtkOmxBufQ::PushFront(int index)
{
#if CPP_STL_SUPPORT
    mBufQ.push_front(index);
#endif

#if ANDROID
    mBufQ.insertAt(index, 0);
#endif
    return;
}

size_t MtkOmxBufQ::Size()
{
    return mBufQ.size();
}

void MtkOmxBufQ::Clear()
{
    return mBufQ.clear();
}
// ]

// Profile & Level tables [
MTK_VENC_PROFILE_LEVEL_ENTRY AvcProfileLevelTable[] =
{
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel3},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel3},
    //Hw codec
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel31},   // => MT6589 spec
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel4},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel4},
    {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel41},
    {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel41},   // => MT6589 spec
};

MTK_VENC_PROFILE_LEVEL_ENTRY HevcProfileLevelTable[] =
{
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel1},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel1},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel2},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel2},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel21},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel21},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel3},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel3},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel31},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel31},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel4},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel4},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel41},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel41},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel5},
    {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCHighTierLevel5},
};

MTK_VENC_PROFILE_LEVEL_ENTRY H263ProfileLevelTable[] =
{
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level10},  // => MFV spec
#if 1
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level20},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level30},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level40},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level45},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level50},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level60},
    {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level70},
#endif
};


MTK_VENC_PROFILE_LEVEL_ENTRY MPEG4ProfileLevelTable[] =
{
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level0},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level0b},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level1},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level2},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level3},  // => MFV spec
#if 1
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level4},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level4a},
    {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level5},
#endif
};

#define MAX_AVC_PROFILE_LEVEL_TABLE_SZIE    sizeof(AvcProfileLevelTable)/sizeof(MTK_VENC_PROFILE_LEVEL_ENTRY)
#define MAX_HEVC_PROFILE_LEVEL_TABLE_SZIE   sizeof(HevcProfileLevelTable)/sizeof(MTK_VENC_PROFILE_LEVEL_ENTRY)
#define MAX_H263_PROFILE_LEVEL_TABLE_SZIE   sizeof(H263ProfileLevelTable)/sizeof(MTK_VENC_PROFILE_LEVEL_ENTRY)
#define MAX_MPEG4_PROFILE_LEVEL_TABLE_SZIE  sizeof(MPEG4ProfileLevelTable)/sizeof(MTK_VENC_PROFILE_LEVEL_ENTRY)
// ]

OMX_BOOL MtkOmxVenc::isBufferSec(OMX_U8 *aInputBuf, OMX_U32 aInputSize, int *aBufferType)
{
    int sec_buffer_type = 0;
    OMX_U32 _handle = 0;
    GetMetaHandleFromBufferPtr(aInputBuf, &_handle);
    int usage = 0;
    int err = 0;
    uint32_t secHandle=0;
    err = gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_USAGE, &usage);
    if (usage & GRALLOC_USAGE_SECURE)
    {
        sec_buffer_type = 1;
        err |= gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE, &secHandle);
    }
    if (err != 0) {
        MTK_OMX_LOGE("query usage and sec handle fail %x", err);
    }
    //gralloc_extra_getSecureBuffer(_handle, &sec_buffer_type, &sec_handle);
    *aBufferType = sec_buffer_type;
    MTK_OMX_LOGD_ENG("isBufferSec %d 0x%8x\n",sec_buffer_type, secHandle);
    return OMX_TRUE;
}

const char *PixelFormatToString(unsigned int nPixelFormat)
{
    switch (nPixelFormat)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return "HAL_PIXEL_FORMAT_RGBA_8888";
        case HAL_PIXEL_FORMAT_RGBX_8888:
            return "HAL_PIXEL_FORMAT_RGBX_8888";
        case HAL_PIXEL_FORMAT_RGB_888:
            return "HAL_PIXEL_FORMAT_RGB_888";
        case HAL_PIXEL_FORMAT_RGB_565:
            return "HAL_PIXEL_FORMAT_RGB_565";
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return "HAL_PIXEL_FORMAT_BGRA_8888";
        case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            //extention format for ROME GPU
            return "HAL_PIXEL_FORMAT_IMG1_BGRX_8888";
        case HAL_PIXEL_FORMAT_YV12:
            return "HAL_PIXEL_FORMAT_YV12";
        default:
            return "Unknown Pixel Format";
    }
}

void MtkOmxVenc::PriorityAdjustment()
{
    if (1 == mVencAdjustPriority && MTK_VENC_CODEC_ID_AVC == mCodecId)
    {
        //apply this for CTS EncodeVirtualDisplayTest in low-end targets that Venc in RT thread with out of SPEC will
        //occupied CPU resources, ALPS1435942
        VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
        OMX_U32 uPriorityAdjustmentType = 0;
        mReturn= eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_PRIORITY_ADJUSTMENT_TYPE, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uPriorityAdjustmentType);
        //MTK_OMX_LOGD("uPriorityAdjustmentType %d", uPriorityAdjustmentType);

        switch(uPriorityAdjustmentType)
        {
            case VENC_DRV_PRIORITY_ADJUSTMENT_TYPE_ONE://8127
                if ((1280 == (int)mInputPortDef.format.video.nFrameWidth) &&
                    (720 == (int)mInputPortDef.format.video.nFrameHeight) &&
                    (15 == (mInputPortDef.format.video.xFramerate >> 16)))
                {
                    mVencAdjustPriority = 0;
                    MTK_OMX_LOGD("!!!!!    [MtkOmxVencEncodeThread] sched_setscheduler ok, nice 10");
                    androidSetThreadPriority(0, 20);
                }
                return;
            case VENC_DRV_PRIORITY_ADJUSTMENT_TYPE_TWO://Denali2
                if (OMX_TRUE == mIsMtklog)
                {
                    mVencAdjustPriority = 0;
                    MTK_OMX_LOGD("!!!!!    [MtkOmxVencEncodeThread] sched_setscheduler ok, ANDROID_PRIORITY_DISPLAY");
                    androidSetThreadPriority(0, ANDROID_PRIORITY_DISPLAY);
                }
                return;
            default:
                return;
        }
    }
}

OMX_VIDEO_AVCPROFILETYPE MtkOmxVenc::defaultAvcProfile(VAL_UINT32_T u4ChipName)
{
    VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
    OMX_U32 uDefaultAVCProfileType = 0;
    mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_DEFAULT_AVC_PROFILE_TYPE, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uDefaultAVCProfileType);
    MTK_OMX_LOGD("uDefaultAVCProfileType %d", uDefaultAVCProfileType);

    switch (uDefaultAVCProfileType)
    {
        case VENC_DRV_DEFAULT_AVC_PROFILE_TYPE_ONE://D2 70 80 8167
            return OMX_VIDEO_AVCProfileBaseline;
            break;

        case VENC_DRV_DEFAULT_AVC_PROFILE_TYPE_TWO://8135 8127 7623 8163, D1 D3 55 57 63 39 58 59 71 75 97 99
            return OMX_VIDEO_AVCProfileHigh;
            break;

        default:
            MTK_OMX_LOGE("%s [ERROR] VAL_CHIP_NAME_UNKNOWN", __FUNCTION__);
            return OMX_VIDEO_AVCProfileBaseline;
            break;
    }
    return OMX_VIDEO_AVCProfileBaseline;
}

OMX_VIDEO_AVCLEVELTYPE MtkOmxVenc::defaultAvcLevel(VAL_UINT32_T u4ChipName)
{

    VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
    OMX_U32 uDefaultAVCLevelType = VENC_DRV_DEFAULT_AVC_LEVEL_TYPE_UNKNOWN;
    mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_DEFAULT_AVC_LEVEL_TYPE, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uDefaultAVCLevelType);
    MTK_OMX_LOGD("uDefaultAVCLevelType %d", uDefaultAVCLevelType);

    switch (uDefaultAVCLevelType)
    {
        case VENC_DRV_DEFAULT_AVC_LEVEL_TYPE_ONE://D2 70 80 8167
            return OMX_VIDEO_AVCLevel21;
            break;

        case VENC_DRV_DEFAULT_AVC_LEVEL_TYPE_TWO://89
            return OMX_VIDEO_AVCLevel31;
            break;
        case VENC_DRV_DEFAULT_AVC_LEVEL_TYPE_THREE://8135 8173 8127 7623, 58 59 97 99
            return OMX_VIDEO_AVCLevel41;
            break;

        case VENC_DRV_DEFAULT_AVC_LEVEL_TYPE_FOUR://6752 8163 D1 D3 55 57 63 39 71 75
            if (OMX_ErrorNone != QueryVideoProfileLevel(VENC_DRV_VIDEO_FORMAT_H264,
                                                        Omx2DriverH264ProfileMap(defaultAvcProfile(mChipName)),
                                                        Omx2DriverH264LevelMap(OMX_VIDEO_AVCLevel41)))
            {
                return OMX_VIDEO_AVCLevel31;
            }
            return OMX_VIDEO_AVCLevel41;
            break;

        default:
            MTK_OMX_LOGE("%s [ERROR] VAL_CHIP_NAME_UNKNOWN", __FUNCTION__);
            return OMX_VIDEO_AVCLevel31;
            break;
    }
    return OMX_VIDEO_AVCLevel31;
}

VENC_DRV_VIDEO_FORMAT_T MtkOmxVenc::GetVencFormat(MTK_VENC_CODEC_ID codecId) {
    switch (codecId) {
        case MTK_VENC_CODEC_ID_MPEG4_SHORT:
            return VENC_DRV_VIDEO_FORMAT_H263;

        case MTK_VENC_CODEC_ID_MPEG4:
            return VENC_DRV_VIDEO_FORMAT_MPEG4;

        case MTK_VENC_CODEC_ID_MPEG4_1080P:
            return VENC_DRV_VIDEO_FORMAT_MPEG4_1080P;

        case MTK_VENC_CODEC_ID_AVC:
            return VENC_DRV_VIDEO_FORMAT_H264;

        case MTK_VENC_CODEC_ID_AVC_VGA:
            return VENC_DRV_VIDEO_FORMAT_H264_VGA;

        default:
            MTK_OMX_LOGE ("Unsupported video format");
            return VENC_DRV_VIDEO_FORMAT_MAX;
    }
}

OMX_VIDEO_HEVCPROFILETYPE MtkOmxVenc::defaultHevcProfile(VAL_UINT32_T u4ChipName)
{
    return OMX_VIDEO_HEVCProfileMain;
}

OMX_VIDEO_HEVCLEVELTYPE MtkOmxVenc::defaultHevcLevel(VAL_UINT32_T u4ChipName)
{
    OMX_S32 indexLevel;

    for (indexLevel = MAX_HEVC_PROFILE_LEVEL_TABLE_SZIE-1; indexLevel >= 0; indexLevel--){
        if (OMX_ErrorNone == QueryVideoProfileLevel(VENC_DRV_VIDEO_FORMAT_HEVC,
                                                    Omx2DriverHEVCProfileMap(defaultHevcProfile(mChipName)),
                                                    Omx2DriverHEVCLevelMap((OMX_VIDEO_HEVCLEVELTYPE)HevcProfileLevelTable[indexLevel].level)))
        {
            return (OMX_VIDEO_HEVCLEVELTYPE)HevcProfileLevelTable[indexLevel].level;
        }
    }
    return OMX_VIDEO_HEVCMainTierLevel1;
}

void MtkOmxVenc::EncodeVideo(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    if (NULL != mCoreGlobal)
    {
        ((mtk_omx_core_global *)mCoreGlobal)->video_operation_count++;
    }

    if (pInputBuf == NULL) {
        encodeHybridEOS(pOutputBuf);
        return;
    }

    if (false == mDoConvertPipeline)
    {
        //check MetaMode input format
        mInputMetaDataFormat = CheckOpaqueFormat(pInputBuf);
        if (0xFFFFFFFF == mGrallocWStride)
        {
            mGrallocWStride = CheckGrallocWStride(pInputBuf);
        }

        /* for DirectLink Meta Mode + */
        if ((OMX_TRUE == DLMetaModeEnable()))
        {
            DLMetaModeEncodeVideo(pInputBuf, pOutputBuf);
            return;
        }
        /* for DirectLink Meta Mode - */
    }

    //init converter buffer here
    if (!mDoConvertPipeline)
    {
        InitConvertBuffer();
    }

    EncodeFunc(pInputBuf, pOutputBuf);
}

void *MtkOmxVencEncodeThread(void *pData)
{
    MtkOmxVenc *pVenc = (MtkOmxVenc *)pData;
    #ifdef OMX_CHECK_DUMMY
    OMX_BOOL bGetDummyIdx = OMX_FALSE;
    #endif
#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxVencEncodeThread", 0, 0, 0);
#endif

    pVenc->mVencEncThreadTid = gettid();

    ALOGD("[0x%08x] ""MtkOmxVencEncodeThread created pVenc=0x%08X, tid=%d", pVenc, (unsigned int)pVenc, gettid());
    prctl(PR_SET_NAME, (unsigned long)"MtkOmxVencEncodeThread", 0, 0, 0);

    while (1)
    {
        //ALOGD ("[0x%08x] ""## Wait to encode (%d)", get_sem_value(&pVenc->mEncodeSem), pVenc);
        WAIT(pVenc->mEncodeSem);
        #ifdef OMX_CHECK_DUMMY
        bGetDummyIdx = OMX_FALSE;
        #endif
        pVenc->PriorityAdjustment();
        pVenc->watchdogTick = pVenc->getTickCountMs();
        if (OMX_FALSE == pVenc->mIsComponentAlive)
        {
            break;
        }

        if (pVenc->mEncodeStarted == OMX_FALSE)
        {
            ALOGD("[0x%08x] ""Wait for encode start.....", pVenc);
            SLEEP_MS(2);
            continue;
        }

        if (pVenc->mPortReconfigInProgress)
        {
            SLEEP_MS(2);
            ALOGD("[0x%08x] ""MtkOmxVencEncodeThread cannot encode when port re-config is in progress", pVenc);
            continue;
        }

        LOCK(pVenc->mEncodeLock);

        if (pVenc->CheckBufferAvailabilityAdvance(pVenc->mpVencInputBufQ, pVenc->mpVencOutputBufQ) == OMX_FALSE)
        {
            //ALOGD ("[0x%08x] ""No input avail...", pVenc);
            if (pVenc->mWaitPart == 1)
            {
                ALOGE("[0x%08x] ""it should not be here! (%d)", pVenc, pVenc->mPartNum);
                SIGNAL_COND(pVenc->mPartCond);
            }
            UNLOCK(pVenc->mEncodeLock);
            SLEEP_MS(1);
            sched_yield();
            continue;
        }

        // dequeue an input buffer
        int input_idx = pVenc->DequeueBufferAdvance(pVenc->mpVencInputBufQ);

        // dequeue an output buffer
        int output_idx = pVenc->DequeueBufferAdvance(pVenc->mpVencOutputBufQ);
        if (OMX_TRUE == pVenc->CheckNeedOutDummy()) //fix cts issue ALPS03040612 of miss last FBD
        {
            pVenc->mDummyIdx = pVenc->DequeueBufferAdvance(pVenc->mpVencOutputBufQ);
            #ifdef OMX_CHECK_DUMMY
            bGetDummyIdx = OMX_TRUE;
            #endif
        }

        if (!pVenc->allowEncodeVideo(input_idx, output_idx))
        {
            sched_yield();
            if (pVenc->mWaitPart == 1)
            {
                ALOGE("[0x%08x] ""in:%d out:%d , dummy[%d]part:%d", pVenc, input_idx, output_idx, pVenc->mDummyIdx, pVenc->mPartNum);
                SIGNAL_COND(pVenc->mPartCond);
            }
            UNLOCK(pVenc->mEncodeLock);
            continue;
        }
#ifdef OMX_CHECK_DUMMY
        if((bGetDummyIdx == OMX_TRUE) && (pVenc->mDummyIdx < 0))
        {
        	  sched_yield();
        	ALOGE("Error :[0x%08x] ""in:%d out:%d , dummy[%d] God2 dummy Failure", pVenc, input_idx, output_idx, pVenc->mDummyIdx);
            ALOGE("Error :[0x%08x] ""in-size:%d out-size:%d", pVenc->mpVencOutputBufQ->Size(), pVenc->mpVencOutputBufQ->Size());
            if (pVenc->mWaitPart == 1)
            {
                ALOGE("[0x%08x] ""in:%d out:%d , dummy[%d]part:%d", pVenc, input_idx, output_idx, pVenc->mDummyIdx, pVenc->mPartNum);
                SIGNAL_COND(pVenc->mPartCond);
            }
            UNLOCK(pVenc->mEncodeLock);
        }
#endif
        //ALOGD ("[0x%08x] ""Encode [%d, %d] (0x%08X, 0x%08X)", pVenc, input_idx, output_idx,
        //(unsigned int)pVenc->mInputBufferHdrs[input_idx], (unsigned int)pVenc->mOutputBufferHdrs[output_idx]);

        // send the input/output buffers to encoder
        if (pVenc->mDoConvertPipeline)
        {
            pVenc->EncodeVideo(pVenc->mVencInputBufferHdrs[input_idx], pVenc->mOutputBufferHdrs[output_idx]);
        }
        else
        {
            OMX_BUFFERHEADERTYPE *pInputBuf = NULL;
            if (input_idx >= 0) {
                pInputBuf = pVenc->mInputBufferHdrs[input_idx];
            }
            pVenc->EncodeVideo(pInputBuf, pVenc->mOutputBufferHdrs[output_idx]);
        }
        UNLOCK(pVenc->mEncodeLock);
    }

    ALOGD("[0x%08x] ""MtkOmxVencEncodeThread terminated pVenc=0x%08X", pVenc, (unsigned int)pVenc);
    return NULL;
}

//Bruce 20130709 [
void *MtkOmxVencConvertThread(void *pData)
{
    MtkOmxVenc *pVenc = (MtkOmxVenc *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxVencConvertThread", 0, 0, 0);
#endif

    pVenc->mVencCnvtThreadTid = gettid();

    ALOGD("[0x%08x] ""MtkOmxVencConvertThread created pVenc=0x%08X, tid=%d", pVenc, (unsigned int)pVenc, gettid());
    prctl(PR_SET_NAME, (unsigned long)"MtkOmxVencConvertThread", 0, 0, 0);

    while (1)
    {
        //ALOGD ("[0x%08x] ""## Wait to decode (%d)", pVenc, get_sem_value(&pVenc->mEncodeSem));
        WAIT(pVenc->mConvertSem);

        if (OMX_FALSE == pVenc->mIsComponentAlive)
        {
            break;
        }

        if (pVenc->mEncodeStarted == OMX_FALSE)
        {
            ALOGD("[0x%08x] ""Wait for encode start.....", pVenc);
            SLEEP_MS(2);
            continue;
        }

        if (pVenc->mPortReconfigInProgress)
        {
            SLEEP_MS(2);
            ALOGD("[0x%08x] ""MtkOmxVencConvertThread cannot convert when port re-config is in progress", pVenc);
            continue;
        }

        if (pVenc->mConvertStarted == false)
        {
            SLEEP_MS(2);
            ALOGD("[0x%08x] ""Wait for convert start.....", pVenc);
            continue;
        }

        LOCK(pVenc->mConvertLock);

        if (pVenc->CheckBufferAvailabilityAdvance(pVenc->mpConvertInputBufQ, pVenc->mpConvertOutputBufQ) == OMX_FALSE)
        {
            //ALOGD ("[0x%08x] ""No input avail...", pVenc);
            UNLOCK(pVenc->mConvertLock);
            SLEEP_MS(1);
            sched_yield();
            continue;
        }

        // dequeue an input buffer
        int input_idx = pVenc->DequeueBufferAdvance(pVenc->mpConvertInputBufQ);

        // dequeue an output buffer
        int output_idx = pVenc->DequeueBufferAdvance(pVenc->mpConvertOutputBufQ);

        if ((input_idx < 0) || (output_idx < 0))
        {
            sched_yield();
            UNLOCK(pVenc->mConvertLock);
            continue;
        }

        // send the input/output buffers to encoder
        pVenc->ConvertVideo(pVenc->mInputBufferHdrs[input_idx], pVenc->mConvertOutputBufferHdrs[output_idx]);

        UNLOCK(pVenc->mConvertLock);
    }

    ALOGD("[0x%08x] ""MtkOmxVencConvertThread terminated pVenc=0x%08X", pVenc, (unsigned int)pVenc);


    return NULL;
}
// ]

void *MtkOmxVencThread(void *pData)
{
    MtkOmxVenc *pVenc = (MtkOmxVenc *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxVencThread", 0, 0, 0);
#endif

    ALOGD("[0x%08x] ""MtkOmxVencThread created pVenc=0x%08X", pVenc, (unsigned int)pVenc);
    prctl(PR_SET_NAME, (unsigned long)"MtkOmxVencThread", 0, 0, 0);

    pVenc->mVencThreadTid = gettid();

    int status;
    ssize_t ret;

    OMX_COMMANDTYPE cmd;
    OMX_U32 cmdCat;
    OMX_U32 nParam1;
    OMX_PTR pCmdData;

    struct pollfd PollFd;
    PollFd.fd = pVenc->mCmdPipe[MTK_OMX_PIPE_ID_READ];
    PollFd.events = POLLIN;
    unsigned int buffer_type;

    while (1)
    {
        status = poll(&PollFd, 1, -1);
        // WaitForSingleObject
        if (-1 == status)
        {
            ALOGE("[0x%08x] ""poll error %d (%s), fd:%d", pVenc, errno, strerror(errno),
                  pVenc->mCmdPipe[MTK_OMX_PIPE_ID_READ]);
            //dump fd
            ALOGE("[0x%08x] ""pipe: %d %d", pVenc, pVenc->mCmdPipe[MTK_OMX_PIPE_ID_READ],
                  pVenc->mCmdPipe[MTK_OMX_PIPE_ID_WRITE]);
            if (errno == 4) // error 4 (Interrupted system call)
            {
            }
            else
            {
                abort();
            }
        }
        else if (0 == status)   // timeout
        {
        }
        else
        {
            if (PollFd.revents & POLLIN)
            {
                READ_PIPE(cmdCat, pVenc->mCmdPipe);
                if (cmdCat == MTK_OMX_GENERAL_COMMAND)
                {
                    READ_PIPE(cmd, pVenc->mCmdPipe);
                    READ_PIPE(nParam1, pVenc->mCmdPipe);
                    ALOGD("[0x%08x] ""# Got general command (%s)", pVenc, CommandToString(cmd));
                    switch (cmd)
                    {
                        case OMX_CommandStateSet:
                            pVenc->HandleStateSet(nParam1);
                            break;

                        case OMX_CommandPortEnable:
                            pVenc->HandlePortEnable(nParam1);
                            break;

                        case OMX_CommandPortDisable:
                            pVenc->HandlePortDisable(nParam1);
                            break;

                        case OMX_CommandFlush:
                            pVenc->HandlePortFlush(nParam1);
                            break;

                        case OMX_CommandMarkBuffer:
                            READ_PIPE(pCmdData, pVenc->mCmdPipe);
                            pVenc->HandleMarkBuffer(nParam1, pCmdData);
                            break;

                        default:
                            ALOGE("[0x%08x] ""Error unhandled command", pVenc);
                            break;
                    }
                }
                else if (cmdCat == MTK_OMX_BUFFER_COMMAND)
                {
                    OMX_BUFFERHEADERTYPE *pBufHead;
                    READ_PIPE(buffer_type, pVenc->mCmdPipe);
                    READ_PIPE(pBufHead, pVenc->mCmdPipe);
                    switch (buffer_type)
                    {
                        case MTK_OMX_EMPTY_THIS_BUFFER_TYPE:
                            //ALOGD ("[0x%08x] ""## EmptyThisBuffer pBufHead(0x%08X)", pVenc, pBufHead);
                            //handle input buffer from IL client
                            pVenc->HandleEmptyThisBuffer(pBufHead);
                            break;
                        case MTK_OMX_FILL_THIS_BUFFER_TYPE:
                            //ALOGD ("[0x%08x] ""## FillThisBuffer pBufHead(0x%08X)", pVenc, pBufHead);
                            // handle output buffer from IL client
                            pVenc->HandleFillThisBuffer(pBufHead);
                            break;
                        default:
                            break;
                    }
                }
                else if (cmdCat == MTK_OMX_STOP_COMMAND)
                {
                    // terminate
                    break;
                }
            }
            else
            {
                ALOGE("[0x%08x] ""Poll get unsupported event:0x%x", pVenc, PollFd.revents);
            }
        }

    }

EXIT:
    ALOGD("[0x%08x] ""MtkOmxVencThread terminated", pVenc);
    return NULL;
}

OMX_BOOL MtkOmxVenc::mEnableMoreLog = OMX_TRUE;

void *MtkOmxVencWatchdogThread(void *pData)
{
    MtkOmxVenc *pVenc = (MtkOmxVenc *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxVencWatchdogThread", 0, 0, 0);
#endif

    pVenc->mVencCnvtThreadTid = gettid();

    ALOGD("[0x%08x] ""MtkOmxVencWatchdogThread created pVenc=0x%08X, tid=%d",
          pVenc, (unsigned int)pVenc, gettid());
#ifdef CONFIG_MT_ENG_BUILD
        usleep(2000000);
#else
        usleep(500000);
#endif

    while (1)
    {
#ifdef CONFIG_MT_ENG_BUILD
        usleep(pVenc->watchdogTimeout*2);
#else
        usleep(pVenc->watchdogTimeout);
#endif

        if (OMX_FALSE == pVenc->mIsComponentAlive)
        {
            break;
        }
        if (!pVenc->mWFDMode && pVenc->mHaveAVCHybridPlatform &&
             pVenc->mIsHybridCodec &&
            (pVenc->getTickCountMs() - pVenc->watchdogTick >= pVenc->watchdogTimeout) &&
            (OMX_FALSE == pVenc->mIsTimeLapseMode))
        {
            LOCK(pVenc->mEncodeLock);
            if (pVenc->mEmptyThisBufQ.mPendingNum <= 1&&
                pVenc->mLastFrameBufHdr != NULL &&
                !(pVenc->mLastFrameBufHdr->nFlags & OMX_BUFFERFLAG_TRIGGER_OUTPUT))
            {
                ALOGW("[0x%08x]""mWFDMode is %d ,Watchdog timeout! %d ms,pVenc->mDoConvertPipeline is %d", pVenc,pVenc->mWFDMode,pVenc->watchdogTimeout,pVenc->mDoConvertPipeline);
                if(!pVenc->mDoConvertPipeline){
                    pVenc->mLastFrameBufHdr->nFlags |= OMX_BUFFERFLAG_TRIGGER_OUTPUT;
                    pVenc->HandleEmptyThisBuffer(pVenc->mLastFrameBufHdr);
                }
            }
            UNLOCK(pVenc->mEncodeLock);
        }else if(!pVenc->mWFDMode &&
                 !(pVenc->mHaveAVCHybridPlatform && pVenc->mIsHybridCodec ) &&
                 pVenc->getTickCountMs() - pVenc->watchdogTick >= pVenc->watchdogTimeout &&
            OMX_FALSE == pVenc->mIsTimeLapseMode)
        {
            LOCK(pVenc->mEncodeLock);
            if (pVenc->mLastFrameBufHdr != NULL)
            {
                ALOGW("[0x%08x] ""Watchdog timeout! %d ms", pVenc, pVenc->watchdogTimeout);
                pVenc->HandleEmptyBufferDone(pVenc->mLastFrameBufHdr);
                pVenc->mLastFrameBufHdr = NULL;
            }
            pVenc->ReturnPendingInputBuffers();
            UNLOCK(pVenc->mEncodeLock);
        }
    }

EXIT:
    ALOGD("[0x%08x] ""MtkOmxVencWatchdogThread terminated", pVenc);
    return NULL;
}

MtkOmxVenc::MtkOmxVenc()
{
    char buildType[PROPERTY_VALUE_MAX];
    char enableMoreLog[PROPERTY_VALUE_MAX];
    property_get("ro.build.type", buildType, "eng");
    if (!strcmp(buildType, "user") || !strcmp(buildType, "userdebug")){
        property_get("vendor.mtk.omx.enable.venc.log", enableMoreLog, "0");
        mEnableMoreLog = (OMX_BOOL) atoi(enableMoreLog);
    }

    MTK_OMX_LOGD("MtkOmxVenc::MtkOmxVenc this= 0x%08X", (unsigned int)this);
    MTK_OMX_MEMSET(&mCompHandle, 0x00, sizeof(OMX_COMPONENTTYPE));
    mCompHandle.nSize = sizeof(OMX_COMPONENTTYPE);
    mCompHandle.pComponentPrivate = this;
    mState = OMX_StateInvalid;

    mInputBufferHdrs = NULL;
    mOutputBufferHdrs = NULL;
    mInputBufferPopulatedCnt = 0;
    mOutputBufferPopulatedCnt = 0;
    mPendingStatus = 0;
    mEncodeStarted = OMX_FALSE;
    mPortReconfigInProgress = OMX_FALSE;

    mEncoderInitCompleteFlag = OMX_FALSE;
    mDrvHandle = (unsigned int)NULL;

    mHeaderLen = 0;
    mFrameCount = 0;

    /* for DirectLink Meta Mode + */
    mSeqHdrEncoded = OMX_FALSE;
    /* for DirectLink Meta Mode - */

    //#ifndef MT6573_MFV_HW
    mLastFrameBufHdr = NULL;
    mLastBsBufHdr = NULL;
    //#endif

#ifdef ANDROID_ICS
    mForceIFrame = OMX_FALSE;
    mIsTimeLapseMode = OMX_FALSE;
    mIsWhiteboardEffectMode = OMX_FALSE;
    mIsMCIMode = OMX_FALSE;
    // Morris Yang 20120214 add for live effect recording [
    mStoreMetaDataInBuffers = OMX_FALSE;
    mCnvtBuffer = NULL;
    mCnvtBufferSize = 0;
    // ]

    mBitRateUpdated = OMX_FALSE;
    mFrameRateUpdated = OMX_FALSE;

#endif

    mIsClientLocally = OMX_TRUE;

    mLastFrameTimeStamp = 0;

    m3DVideoRecordMode = OMX_VIDEO_H264FPA_NONE;// for MTK S3D SUPPORT

    mCodecId = MTK_VENC_CODEC_ID_INVALID;

    INIT_MUTEX(mCmdQLock);
    INIT_MUTEX(mEncodeLock);

    INIT_SEMAPHORE(mInPortAllocDoneSem);
    INIT_SEMAPHORE(mOutPortAllocDoneSem);
    INIT_SEMAPHORE(mInPortFreeDoneSem);
    INIT_SEMAPHORE(mOutPortFreeDoneSem);
    INIT_SEMAPHORE(mEncodeSem);

    INIT_COND(mPartCond);
    mPartNum = 0;
    mWaitPart = 0;

    //Bruce 20130709 [
    mEmptyThisBufQ.mId = MtkOmxBufQ::MTK_OMX_VENC_BUFQ_INPUT;
    mFillThisBufQ.mId = MtkOmxBufQ::MTK_OMX_VENC_BUFQ_OUTPUT;

    mDoConvertPipeline = false;
    mConvertOutputBufQ.mId = MtkOmxBufQ::MTK_OMX_VENC_BUFQ_CONVERT_OUTPUT;
    mVencInputBufQ.mId = MtkOmxBufQ::MTK_OMX_VENC_BUFQ_VENC_INPUT;
    mpConvertInputBufQ = NULL;
    mpConvertOutputBufQ = NULL;
    mpVencInputBufQ = &mEmptyThisBufQ;
    mpVencOutputBufQ = &mFillThisBufQ;

    mVencInputBufferHdrs = NULL;
    mConvertOutputBufferHdrs = NULL;

    mConvertStarted = false;
    // ]

    mIInterval = 0;
    mSetIInterval = OMX_FALSE;

    mSetWFDMode = OMX_FALSE;
    mWFDMode = OMX_FALSE;
    mSetStreamingMode = OMX_FALSE;

    mScaledWidth = 0;
    mScaledHeight = 0;
    mSkipFrame = 0;
    mDumpFlag = 0;

    char value[PROPERTY_VALUE_MAX];
    mInputScalingMode = OMX_FALSE;
    {
        property_get("vendor.mtk.omxvenc.input.scaling", value, "0");
        mInputScalingMode = (OMX_BOOL)atoi(value);
        property_get("vendor.mtk.omxvenc.input.scaling.max.wide", value, "1920");
        mMaxScaledWide = (OMX_U32)atoi(value);
        property_get("vendor.mtk.omxvenc.input.scaling.max.narrow", value, "1088");
        mMaxScaledNarrow = (OMX_U32)atoi(value);
        mMaxScaledWide = (mMaxScaledWide > 0) ?  mMaxScaledWide:1920;
        mMaxScaledNarrow = (mMaxScaledNarrow > 0) ?  mMaxScaledNarrow:1088;
    }

    mDrawStripe = false;
    {
        property_get("vendor.mtk.omxvenc.drawline", value, "0");
        mDrawStripe = (bool)atoi(value);
    }
    mDumpInputFrame = false;
    {
        property_get("vendor.mtk.omxvenc.dump", value, "0");
        mDumpInputFrame = (OMX_BOOL) atoi(value);
    }

    mDumpCts = false;
    {
        property_get("vendor.mtk.dumpCts", value, "0");
        mDumpCts = (OMX_BOOL) atoi(value);
    }

    mRTDumpInputFrame = false;
    {
        property_get("vendor.mtk.omxvenc.rtdump", value, "1");
        mRTDumpInputFrame = (OMX_BOOL) atoi(value);
    }

    mDumpColorConvertFrame = false;
    {
        property_get("vendor.mtk.omxvenc.ccdump", value, "0");
        mDumpColorConvertFrame = (OMX_BOOL) atoi(value);
    }
    mDumpCCNum = 5;
    {
        property_get("vendor.mtk.omxvenc.ccdumpnum", value, "5");
        mDumpCCNum = atoi(value);
    }
    {
        property_get("vendor.mtk.omxvenc.secrgb", value, "0");
        if (atoi(value))
        {
            mDumpFlag |= DUMP_SECURE_INPUT_Flag;
        }
        property_get("vendor.mtk.omxvenc.secyuv", value, "0");
        if (atoi(value))
        {
            mDumpFlag |= DUMP_SECURE_TMP_IN_Flag;
        }
        property_get("vendor.mtk.omxvenc.secbs", value, "0");
        if (atoi(value))
        {
            mDumpFlag |= DUMP_SECURE_OUTPUT_Flag;
        }
        property_get("vendor.mtk.omxvenc.dumpyv12test", value, "0");
        if (atoi(value))
        {
            mDumpFlag |= DUMP_YV12_Flag;
        }
        MTK_OMX_LOGD("dump flag=0x%x", mDumpFlag);
    }
    mEnableDummy = OMX_TRUE;
    {
        property_get("ro.vendor.wfd.dummy.enable", value, "1");
        mEnableDummy = (OMX_BOOL) atoi(value);
    }
    //DISABLE DUMMY FOR 8167 HYBRID H264
    //if (VAL_CHIP_NAME_MT8167 == mChipName)
    //{
    //    mEnableDummy = OMX_FALSE;
    //}

    mDumpDLBS = OMX_FALSE;
    {
        property_get("vendor.mtk.omxvenc.dump.dlbs", value, "0");
        mDumpDLBS = (OMX_BOOL) atoi(value);
    }

    mIsMtklog = OMX_FALSE;
    {
        property_get("vendor.debug.MB.running", value, "0");
        mIsMtklog = (OMX_BOOL) atoi(value);
    }

    watchdogTimeout = 2000;
    {
        property_get("vendor.mtk.omxvenc.watchdog.timeout", value, "2000");
        watchdogTimeout = atoi(value);
    }

    mCoreGlobal = NULL;
    //Bruce 20130709 [
    INIT_SEMAPHORE(mConvertSem);
    INIT_MUTEX(mConvertLock);

    mInitPipelineBuffer = false;
    // ]
    mDrawBlack = OMX_FALSE;//for Miracast test case SIGMA 5.1.11 workaround


    mSendDummyNAL = false;
    mDummyIdx = -1;

    mIsLivePhoto = false;

    mTmpColorConvertBuf = NULL;
    mTmpColorConvertBufSize = 0;

    mPrependSPSPPSToIDRFrames = OMX_FALSE;
    mPrependSPSPPSToIDRFramesNotify = OMX_FALSE;

    mStoreMetaDataInOutBuffers = OMX_FALSE;

    mInputAllocateBuffer = OMX_FALSE;
    mOutputAllocateBuffer = OMX_FALSE;
    mVencAdjustPriority = 1;

    mIsSecureSrc = OMX_FALSE;
    mIsSecureInst = OMX_FALSE;
    mTestSecInputHandle = 0xffffffff;
    bHasSecTlc = false;

    memset(&mBitStreamBuf, 0, sizeof(mBitStreamBuf));
    memset(&mFrameBuf, 0, sizeof(mFrameBuf));
    mReconfigCount = 0;
    mCnvtPortReconfigInProgress = OMX_FALSE;

    mEnableNonRefP = OMX_FALSE;

    mInputMetaDataFormat = 0;
    mGrallocWStride = 0xFFFFFFFF;

    memset(&mEncDrvSetting, 0, sizeof(mEncDrvSetting));
    memset(&mExtraEncDrvSetting, 0, sizeof(mExtraEncDrvSetting));

    mIsHybridCodec = OMX_FALSE;

    mTeeEncType = NONE_TEE;

    mSetQP = OMX_FALSE;
    mIsMultiSlice = OMX_FALSE;
    property_get("vendor.mtk.omxvenc.vilte", value, "0");
    mIsViLTE = (OMX_BOOL) atoi(value);
    mETBDebug = true;
#ifdef MTK_DUM_SEC_ENC
    pTmp_buf = 0;
    Tmp_bufsz =0;
#endif
    mCmdPipe[0] = -1;
    mCmdPipe[1] = -1;
    mIsCrossMount= false;
    mSetConstantBitrateMode = OMX_FALSE;

    property_get("vendor.mtk.omxvenc.avpf", value, "0");
    mAVPFEnable = (OMX_BOOL) atoi(value);
    mGotSLI = OMX_FALSE;
    mForceFullIFrame = OMX_FALSE;
    mForceFullIFramePrependHeader = OMX_FALSE;
    mIDRInterval = 0;
    mOperationRate = 0;
    mSetIDRInterval = OMX_FALSE;
    mbYUV420FlexibleMode = OMX_FALSE;

    mPrepareToResolutionChange = OMX_FALSE;
    u4EncodeWidth = 0;
    u4EncodeHeight = 0;
    nFilledLen = 0;
    mLastTimeStamp = 0;
    mSlotBitCount = 0;
    mIDRIntervalinSec = 0;
    mLastIDRTimeStamp = 0;

    mMeetHybridEOS = OMX_FALSE;
    mHaveAVCHybridPlatform = OMX_FALSE;
    mSLI.nSize = 0;
    mSLI.nPortIndex = 0;
    mSLI.nTimeStamp = 0;
    mSLI.nSliceCount = 0;
    for (int i = 0; i < 8; i++)
    {
        mSLI.SliceLoss[i] = 0;
    }

    // camera switch threshold init
    mCameraSwitchThreshold = 260;
    if( property_get("persist.vendor.vt.cam_nodata", value, NULL) )
    {
        mCameraSwitchThreshold = atoi(value);
    }

#ifdef SUPPORT_NATIVE_HANDLE
    mIsAllocateOutputNativeBuffers = OMX_FALSE;
    mIonDevFd = -1;
    mStoreMetaOutNativeHandle.clear();
    mIonBufferInfo.clear();
    mIsChangeBWC4WFD = OMX_FALSE;
    property_get("vendor.mtk.omxvenc.wfd.record", value, "0");
    mRecordBitstream  = (OMX_BOOL) atoi(value);
    property_get("vendor.mtk.omxvenc.wfd.loopback", value, "0");
    mWFDLoopbackMode  = (OMX_BOOL) atoi(value);
    MTK_OMX_LOGD_ENG("mRecordBitstream %d, mWFDLoopbackMode %d", mRecordBitstream, mWFDLoopbackMode);

    mIonDevFd = eVideoOpenIonDevFd();
    mInputMVAMgr = new OmxMVAManager("ion", mIonDevFd);
    mOutputMVAMgr = new OmxMVAManager("ion", mIonDevFd);
    mCnvtMVAMgr = new OmxMVAManager("ion", mIonDevFd);
#else
    mInputMVAMgr = new OmxMVAManager("ion");
    mOutputMVAMgr = new OmxMVAManager("ion");
    mCnvtMVAMgr = new OmxMVAManager("ion");
#endif

    mSubFrameIndex = 0;
    mSubFrameTotalCount = 0;
    mSubFrameTimeStamp = 0;
    mSubFrameLastTimeStamp = 0;
}

MtkOmxVenc::~MtkOmxVenc()
{
    MTK_OMX_LOGD("~MtkOmxVenc this= 0x%08X", (unsigned int)this);
#ifdef SUPPORT_NATIVE_HANDLE
    for (int i = 0; i < mStoreMetaOutNativeHandle.size(); ++i)
    {
        MTK_OMX_LOGE("[WARNING] Freeing 0x%x in deinit", mStoreMetaOutNativeHandle[i]);
        MTK_OMX_FREE(mStoreMetaOutNativeHandle[i]);
    }
    mStoreMetaOutNativeHandle.clear();
    if (mIonDevFd > 0)
    {
        for (int i = 0; i < mIonBufferInfo.size(); ++i)
        {
            MTK_OMX_LOGE("[WARNING] Freeing ion in deinit fd %d va %p handle %p",
                mIonBufferInfo[i].ion_share_fd, mIonBufferInfo[i].va, mIonBufferInfo[i].ion_handle);
#ifdef COPY_2_CONTIG
            MTK_OMX_LOGE("[WARNING] Freeing ion in deinit fd %d va %p handle %p",
                mIonBufferInfo[i].ion_share_fd_4_enc, mIonBufferInfo[i].va_4_enc, mIonBufferInfo[i].ion_handle_4_enc);
#endif
            if (0 == mIonBufferInfo[i].secure_handle)
            {
                if (NULL != mIonBufferInfo[i].va)
                {
                    ion_munmap(mIonDevFd, mIonBufferInfo[i].va, mIonBufferInfo[i].value[0]);
                }
#ifdef COPY_2_CONTIG
                if (NULL != mIonBufferInfo[i].va_4_enc)
                {
                    ion_munmap(mIonDevFd, mIonBufferInfo[i].va_4_enc, mIonBufferInfo[i].value[0]);
                }
#endif
            }
            ion_share_close(mIonDevFd, mIonBufferInfo[i].ion_share_fd);
            if (ion_free(mIonDevFd, mIonBufferInfo[i].ion_handle))
            {
                MTK_OMX_LOGE("[ERROR] omx dtr ion_free %d failed", mIonBufferInfo[i].ion_handle);
            }
#ifdef COPY_2_CONTIG
            ion_share_close(mIonDevFd, mIonBufferInfo[i].ion_share_fd_4_enc);
            if (ion_free(mIonDevFd, mIonBufferInfo[i].ion_handle_4_enc))
            {
                MTK_OMX_LOGE("[ERROR] omx dtr ion_free %d failed", mIonBufferInfo[i].ion_handle_4_enc);
            }
#endif
        }
        mIonBufferInfo.clear();
        eVideoCloseIonDevFd(mIonDevFd);
        mIonDevFd = -1;
    }
#endif

    if (mInputBufferHdrs)
    {
        MTK_OMX_FREE(mInputBufferHdrs);
    }

    if (mOutputBufferHdrs)
    {
        MTK_OMX_FREE(mOutputBufferHdrs);
    }

    //Bruce 20130709 [
    if (mDoConvertPipeline)
    {
        DeinitPipelineBuffer();
    }
    else
    {
        DeinitConvertBuffer();
    }
    // ]

    delete mCnvtMVAMgr;
    delete mOutputMVAMgr;
    delete mInputMVAMgr;

    DESTROY_MUTEX(mEncodeLock);
    DESTROY_MUTEX(mCmdQLock);

    DESTROY_SEMAPHORE(mInPortAllocDoneSem);
    DESTROY_SEMAPHORE(mOutPortAllocDoneSem);
    DESTROY_SEMAPHORE(mInPortFreeDoneSem);
    DESTROY_SEMAPHORE(mOutPortFreeDoneSem);
    DESTROY_SEMAPHORE(mEncodeSem);

    DESTROY_COND(mPartCond);
    //Bruce 20130709 [
    DESTROY_SEMAPHORE(mConvertSem);
    DESTROY_MUTEX(mConvertLock);
    // ]

    if (mTmpColorConvertBuf != NULL)
    {
        free(mTmpColorConvertBuf);
        mTmpColorConvertBuf = NULL;
    }
#ifdef MTK_DUM_SEC_ENC
    if(pTmp_buf && Tmp_bufsz)
    {
        MTK_OMX_LOGE("MtkOmxVenc :Free sec dump tmp buffer\n");
        free(pTmp_buf);
        pTmp_buf  =0;
        Tmp_bufsz =0;
    }
#endif
    DeInitSecEncParams();
}

OMX_ERRORTYPE MtkOmxVenc::ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING componentName)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD("MtkOmxVenc::ComponentInit (%s)", componentName);
    mState = OMX_StateLoaded;
    int ret;

    // query chip name
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvQueryCapability(VENC_DRV_QUERY_TYPE_CHIP_NAME, VAL_NULL, &mChipName))
    {
        MTK_OMX_LOGE("[ERROR] Cannot get encoder property, VENC_DRV_QUERY_TYPE_CHIP_NAME");
        goto EXIT;
    }

    InitOMXParams(&mInputPortDef);
    InitOMXParams(&mOutputPortDef);

    if (!strcmp(componentName, MTK_OMX_AVC_ENCODER) ||
        !strcmp(componentName, MTK_OMX_AVC_SEC_ENCODER))
    {
        if (OMX_FALSE == InitAvcEncParams())
        {
            err = OMX_ErrorInsufficientResources;
            MTK_OMX_LOGE("InitAvcEncParams 2 failed ");
            goto EXIT;
        }
        mCodecId = MTK_VENC_CODEC_ID_AVC;
        MTK_OMX_LOGD("%s init sec 2 mIsSecureInst %d", __FUNCTION__,mIsSecureInst);
#if 0
//#ifdef SUPPORT_NATIVE_HANDLE
        if (!strcmp(componentName, MTK_OMX_AVC_SEC_ENCODER))
        {
            BWC bwc;
            bwc.Profile_Change(BWCPT_VIDEO_WIFI_DISPLAY, true);
            MTK_OMX_LOGD("enter WFD BWCPT_VIDEO_WIFI_DISPLAY");
            mIsChangeBWC4WFD = OMX_TRUE;
        }
#endif
    }
    else if (!strcmp(componentName, MTK_OMX_HEVC_ENCODER))
    {
        if (OMX_FALSE == InitHevcEncParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VENC_CODEC_ID_HEVC;
    }
    else if (!strcmp(componentName, MTK_OMX_VP8_ENCODER))
    {
        if (OMX_FALSE == InitVP8EncParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VENC_CODEC_ID_VP8;
    }
    else if (!strcmp(componentName, MTK_OMX_H263_ENCODER))
    {
        if (OMX_FALSE == InitH263EncParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VENC_CODEC_ID_MPEG4_SHORT;
    }
    else if (!strcmp(componentName, MTK_OMX_MPEG4_ENCODER))
    {
        if (OMX_FALSE == InitMpeg4EncParams())
        {
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        mCodecId = MTK_VENC_CODEC_ID_MPEG4;
    }
    else
    {
        MTK_OMX_LOGE("MtkOmxVenc::ComponentInit ERROR: Unknown component name");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    {
        VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
        OMX_U32 uMpeg4SWPlatform = 0;
        OMX_U32 uAVCHybridPlatform = 0;
        mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_MPEG4_SW_PLATFORM, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uMpeg4SWPlatform);
        mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_AVC_HYBRID_PLATFORM, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uAVCHybridPlatform);
        MTK_OMX_LOGD_ENG("uMpeg4SWPlatform %d, uAVCHybridPlatform %d", uMpeg4SWPlatform, uAVCHybridPlatform);

        if (((mCodecId == MTK_VENC_CODEC_ID_MPEG4 || mCodecId == MTK_VENC_CODEC_ID_MPEG4_SHORT) && uMpeg4SWPlatform)||
            (mCodecId == MTK_VENC_CODEC_ID_AVC && uAVCHybridPlatform))
        {
            mHaveAVCHybridPlatform = (OMX_BOOL)uAVCHybridPlatform;
        mIsHybridCodec = OMX_TRUE;
    }
    }
    // query input color format
    VENC_DRV_YUV_FORMAT_T yuvFormat;
    if (VENC_DRV_MRESULT_OK == eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_GET_YUV_FORMAT, NULL, &yuvFormat))
    {
        switch (yuvFormat)
        {
            case VENC_DRV_YUV_FORMAT_420:
                mInputPortFormat.eColorFormat = OMX_COLOR_FormatYUV420Planar;
                mInputPortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
                break;

            case VENC_DRV_YUV_FORMAT_YV12:
                mInputPortFormat.eColorFormat = OMX_MTK_COLOR_FormatYV12;
                mInputPortDef.format.video.eColorFormat = OMX_MTK_COLOR_FormatYV12;
                break;

            default:
                break;
        }
    }
    else
    {
        MTK_OMX_LOGE("ERROR: query VENC_DRV_GET_TYPE_GET_YUV_FORMAT failed");
    }

    InitOMXParams(&mAvcType);
    InitOMXParams(&mH263Type);
    InitOMXParams(&mMpeg4Type);
    InitOMXParams(&mVp8Type);
    InitOMXParams(&mBitrateType);
    InitOMXParams(&mQuantizationType);
    InitOMXParams(&mVbsmcType);
    InitOMXParams(&mMvType);
    InitOMXParams(&mIntraRefreshType);
    InitOMXParams(&mAvcSliceFMO);
    InitOMXParams(&mErrorCorrectionType);
    InitOMXParams(&mProfileLevelType);
    InitOMXParams(&mFrameRateType);
    InitOMXParams(&mConfigBitrate);
    InitOMXParams(&mConfigIntraRefreshVopType);
    InitOMXParams(&mConfigAVCIntraPeriod);
    InitOMXParams(&mLayerParams);
    InitOMXParams(&mLayerConfig);

    // create command pipe
    ret = pipe(mCmdPipe);
    if (ret)
    {
        MTK_OMX_LOGE("mCmdPipe creation failure");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    mIsComponentAlive = OMX_TRUE;

    //MTK_OMX_LOGD ("mCmdPipe[0] = %d", mCmdPipe[0]);
    // create Venc thread
    ret = pthread_create(&mVencThread, NULL, &MtkOmxVencThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE("MtkOmxVencThread creation failure");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // create video encoding thread
    ret = pthread_create(&mVencEncodeThread, NULL, &MtkOmxVencEncodeThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE("MtkOmxVencEncodeThread creation failure");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // create color convert thread
    ret = pthread_create(&mVencConvertThread, NULL, &MtkOmxVencConvertThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE("MtkOmxVencConvertThread creation failure");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // create watchdog thread
    if (mIsHybridCodec == OMX_TRUE)
    {
        ret = pthread_create(&mVencWatchdogThread, NULL, &MtkOmxVencWatchdogThread, (void *)this);
        if (ret)
        {
            MTK_OMX_LOGE("MtkOmxVencWatchdogThread creation failure");
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
    }

    mExtensions.ComponentInit();

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    (void)(hComponent);
    MTK_OMX_LOGD("+MtkOmxVenc::ComponentDeInit");
    OMX_ERRORTYPE err = OMX_ErrorNone;
    ssize_t ret = 0;
#if 0
//#ifdef SUPPORT_NATIVE_HANDLE
    if (OMX_TRUE == mIsChangeBWC4WFD)
    {
        BWC bwc;
        bwc.Profile_Change(BWCPT_VIDEO_WIFI_DISPLAY, false);
        MTK_OMX_LOGD("leave WFD BWCPT_VIDEO_WIFI_DISPLAY  !");
        mIsChangeBWC4WFD = OMX_FALSE;
    }
#endif

    // terminate decode thread
    mIsComponentAlive = OMX_FALSE;
    SIGNAL(mEncodeSem);

    // terminate convert thread
    SIGNAL(mConvertSem);

    mExtensions.ComponentDeInit();

    // terminate command thread
    OMX_U32 CmdCat = MTK_OMX_STOP_COMMAND;
    if (mCmdPipe[MTK_OMX_PIPE_ID_WRITE] > -1)
    {
        WRITE_PIPE(CmdCat, mCmdPipe);
    }

    if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
    {
        SIGNAL(mInPortAllocDoneSem);
    }
    if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
    {
        SIGNAL(mOutPortAllocDoneSem);
        MTK_OMX_LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
    }
    if (IS_PENDING(MTK_OMX_IDLE_PENDING))
    {
        SIGNAL(mInPortAllocDoneSem);
        MTK_OMX_LOGD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
        SIGNAL(mOutPortAllocDoneSem);
        MTK_OMX_LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
    }

    if (!pthread_equal(pthread_self(), mVencConvertThread))
    {
        // wait for mVencConvertThread terminate
        pthread_join(mVencConvertThread, NULL);
    }
    if (mIsHybridCodec == OMX_TRUE)
    {
        if (!pthread_equal(pthread_self(), mVencWatchdogThread))
        {
            // wait for mVencWatchdogThread terminate
            pthread_join(mVencWatchdogThread, NULL);
        }
    }
#if 1
    if (!pthread_equal(pthread_self(), mVencEncodeThread))
    {
        // wait for mVencEncodeThread terminate
        pthread_join(mVencEncodeThread, NULL);
    }

    if (!pthread_equal(pthread_self(), mVencThread))
    {
        // wait for mVencThread terminate
        pthread_join(mVencThread, NULL);
    }
#endif

    if (NULL != mCoreGlobal)
    {
        ((mtk_omx_core_global *)mCoreGlobal)->video_instance_count--;
    }

    if (mCmdPipe[MTK_OMX_PIPE_ID_READ] > -1)
    {
        close(mCmdPipe[MTK_OMX_PIPE_ID_READ]);
    }
    if (mCmdPipe[MTK_OMX_PIPE_ID_WRITE] > -1)
    {
        close(mCmdPipe[MTK_OMX_PIPE_ID_WRITE]);
    }

    MTK_OMX_LOGD("-MtkOmxVenc::ComponentDeInit");

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                              OMX_IN OMX_STRING componentName,
                                              OMX_OUT OMX_VERSIONTYPE *componentVersion,
                                              OMX_OUT OMX_VERSIONTYPE *specVersion,
                                              OMX_OUT OMX_UUIDTYPE *componentUUID)

{
    (void)(hComponent);
    (void)(componentName);
    (void)(componentUUID);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD("MtkOmxVenc::GetComponentVersion");
    componentVersion->s.nVersionMajor = 1;
    componentVersion->s.nVersionMinor = 1;
    componentVersion->s.nRevision = 2;
    componentVersion->s.nStep = 0;
    specVersion->s.nVersionMajor = 1;
    specVersion->s.nVersionMinor = 1;
    specVersion->s.nRevision = 2;
    specVersion->s.nStep = 0;
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_COMMANDTYPE Cmd,
                                      OMX_IN OMX_U32 nParam1,
                                      OMX_IN OMX_PTR pCmdData)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD("MtkOmxVenc::SendCommand cmd=%s", CommandToString(Cmd));

    OMX_U32 CmdCat = MTK_OMX_GENERAL_COMMAND;

    ssize_t ret = 0;

    LOCK(mCmdQLock);

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (Cmd)
    {
        case OMX_CommandStateSet:   // write 8 bytes to pipe [cmd][nParam1]
            if (nParam1 == OMX_StateIdle)
            {
                MTK_OMX_LOGD("set MTK_OMX_VENC_IDLE_PENDING");
                SET_PENDING(MTK_OMX_IDLE_PENDING);
            }
            else if (nParam1 == OMX_StateLoaded)
            {
                MTK_OMX_LOGD("set MTK_OMX_VENC_LOADED_PENDING");
                SET_PENDING(MTK_OMX_LOADED_PENDING);
            }
            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandPortDisable:
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            // mark the ports to be disabled first, p.84
            if (nParam1 == MTK_OMX_INPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mInputPortDef.bEnabled = OMX_FALSE;
            }

            if (nParam1 == MTK_OMX_OUTPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mOutputPortDef.bEnabled = OMX_FALSE;
            }

            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandPortEnable:
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            // mark the ports to be enabled first, p.85
            if (nParam1 == MTK_OMX_INPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mInputPortDef.bEnabled = OMX_TRUE;

                if ((mState != OMX_StateLoaded) && (mInputPortDef.bPopulated == OMX_FALSE))
                {
                    SET_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
                }
            }

            if (nParam1 == MTK_OMX_OUTPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mOutputPortDef.bEnabled = OMX_TRUE;

                if ((mState != OMX_StateLoaded) && (mOutputPortDef.bPopulated == OMX_FALSE))
                {
                    //MTK_OMX_LOGD ("SET_PENDING(MTK_OMX_VENC_OUT_PORT_ENABLE_PENDING) mState(%d)", mState);
                    SET_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);
                }
            }

            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandFlush:  // p.84
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }
            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandMarkBuffer:    // write 12 bytes to pipe [cmd][nParam1][pCmdData]
            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            WRITE_PIPE(pCmdData, mCmdPipe);
            break;

        default:
            MTK_OMX_LOGE("[ERROR] Unknown command(0x%08X)", Cmd);
            break;
    }

EXIT:
    UNLOCK(mCmdQLock);
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                       OMX_IN OMX_PTR pAppData)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD_ENG("MtkOmxVenc::SetCallbacks");
    if (NULL == pCallBacks)
    {
        MTK_OMX_LOGE("[ERROR] MtkOmxVenc::SetCallbacks pCallBacks is NULL !!!");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }
    mCallback = *pCallBacks;
    mAppData = pAppData;
    mCompHandle.pApplicationPrivate = mAppData;

EXIT:
    return err;
}

#define callReturnIfNoError(NAME, ...) \
    do { \
    if(OMX_ErrorNone == NAME( __VA_ARGS__ )) { \
        return OMX_ErrorNone; \
    }} \
    while(0)

OMX_ERRORTYPE MtkOmxVenc::SetParameter(OMX_IN OMX_HANDLETYPE hComp,
                                       OMX_IN OMX_INDEXTYPE nParamIndex,
                                       OMX_IN OMX_PTR pCompParam)
{
    (void)(hComp);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD_ENG("MtkOmxVenc::SetParameter index(0x%08X)", nParamIndex);
    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    if (NULL == pCompParam)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    callReturnIfNoError(mExtensions.SetParameter, nParamIndex, pCompParam);

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;
            if (!checkOMXParams(pPortDef)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortDef->nPortIndex == mInputPortDef.nPortIndex)
            {
                if (pPortDef->nBufferCountActual > mInputPortDef.nBufferCountActual) //user need more buffer
                {
                    if (pPortDef->nBufferCountActual > 30)
                    {
                        MTK_OMX_LOGD("Can't support so many input buffers (%d)", pPortDef->nBufferCountActual);
                        err = OMX_ErrorBadParameter;
                        goto EXIT;
                    }
                    OMX_BUFFERHEADERTYPE **tmp;
                    tmp = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(
                              sizeof(OMX_BUFFERHEADERTYPE *)*pPortDef->nBufferCountActual);
                    if (tmp == NULL)//realloc fail
                    {
                        MTK_OMX_LOGE("realloc input fail");
                        err = OMX_ErrorBadParameter;
                        goto EXIT;
                    }
                    else
                    {
                        memcpy(tmp, mInputBufferHdrs, sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);
                        MTK_OMX_FREE(mInputBufferHdrs);
                        mInputBufferHdrs = tmp;
                    }
                }
                memcpy(&mInputPortDef, pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                mInputPortDef.nBufferSize = getInputBufferSizeByFormat(mInputPortDef.format.video);
                MTK_OMX_LOGD("@@ Set input port: nFrameWidth(%d), nFrameHeight(%d), nStride(%d), nSliceHeight(%d), "
                             "nBitrate(%d), xFramerate(0x%X), eColorFormat(0x%08X), nBufferSize(%d), "
                             "nBufferCountActual(%d)",
                             (int)mInputPortDef.format.video.nFrameWidth, (int)mInputPortDef.format.video.nFrameHeight,
                             (int)mInputPortDef.format.video.nStride, (int)mInputPortDef.format.video.nSliceHeight,
                             (int)mInputPortDef.format.video.nBitrate,
                             (unsigned int)mInputPortDef.format.video.xFramerate,
                             (unsigned int)mInputPortDef.format.video.eColorFormat, (int)mInputPortDef.nBufferSize,
                             (int)mInputPortDef.nBufferCountActual);
                {
                    if(mInputPortDef.format.video.xFramerate >> 16 > 0){
                        watchdogTimeout = (1000/(mInputPortDef.format.video.xFramerate >> 16))*10;
                    }
                    MTK_OMX_LOGD("betfor setprop watchdogTimeout is %d",watchdogTimeout);
                    char value[PROPERTY_VALUE_MAX];
                    int setValue;
#ifdef CONFIG_MT_ENG_BUILD
                    property_get("vendor.mtk.omxvenc.watchdog.timeout", value, "2000");
#else
                    property_get("vendor.mtk.omxvenc.watchdog.timeout", value, "300");
#endif
                    setValue = atoi(value);
                    MTK_OMX_LOGE("setprop setValue to %d",setValue);
                    if(setValue > watchdogTimeout){
                        watchdogTimeout = setValue;
                    }
                }

                // input scaling only for overspec
                if ( mInputPortDef.format.video.nFrameWidth <  mInputPortDef.format.video.nFrameHeight)
                {
                    if (mInputPortDef.format.video.nFrameWidth <= mMaxScaledNarrow && mInputPortDef.format.video.nFrameHeight <= mMaxScaledWide)
                    {
                        mInputScalingMode = OMX_FALSE;
                        break;
                    }
                } else {
                    if (mInputPortDef.format.video.nFrameWidth <= mMaxScaledWide && mInputPortDef.format.video.nFrameHeight <= mMaxScaledNarrow)
                    {
                        mInputScalingMode = OMX_FALSE;
                        break;
                    }
                }

                // keep aspect ratio scaling
                if (mInputPortDef.format.video.nFrameWidth > mInputPortDef.format.video.nFrameHeight && mInputPortDef.format.video.nFrameHeight > 0)
                {
                    mScaledHeight = mMaxScaledNarrow;
                    mScaledWidth = mInputPortDef.format.video.nFrameWidth * ((float)mMaxScaledNarrow / mInputPortDef.format.video.nFrameHeight);
                    if (mScaledWidth > mMaxScaledWide)
                    {
                        mScaledHeight *= ((float)mMaxScaledWide / mScaledWidth);
                        mScaledWidth = mMaxScaledWide;
                    }
                }
                else if (mInputPortDef.format.video.nFrameWidth > 0)
                {
                    mScaledWidth = mMaxScaledNarrow;
                    mScaledHeight = mInputPortDef.format.video.nFrameHeight * ((float)mMaxScaledNarrow / mInputPortDef.format.video.nFrameWidth);
                    if (mScaledHeight > mMaxScaledWide)
                    {
                        mScaledWidth *= ((float)mMaxScaledWide / mScaledHeight);
                        mScaledHeight = mMaxScaledWide;
                    }
                }
                // avoid MDP odd size green line
                mScaledWidth =  (mScaledWidth % 2)? (mScaledWidth+1):mScaledWidth;
                mScaledHeight = (mScaledHeight % 2)? (mScaledHeight+1):mScaledHeight;

            }
            else if (pPortDef->nPortIndex == mOutputPortDef.nPortIndex)
            {
                if (pPortDef->nBufferCountActual > mOutputPortDef.nBufferCountActual) //user need more buffer
                {
                    if (pPortDef->nBufferCountActual > 30)
                    {
                        MTK_OMX_LOGD("Can't support so many output buffers (%d)", pPortDef->nBufferCountActual);
                        err = OMX_ErrorBadParameter;
                        goto EXIT;
                    }
                    OMX_BUFFERHEADERTYPE **tmp;
                    tmp = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *) *
                                                                 pPortDef->nBufferCountActual);
                    if (tmp == NULL)//realloc fail
                    {
                        MTK_OMX_LOGE("realloc output fail");
                        err = OMX_ErrorBadParameter;
                        goto EXIT;
                    }
                    else
                    {
                        memcpy(tmp, mOutputBufferHdrs, sizeof(OMX_BUFFERHEADERTYPE *) *
                               mOutputPortDef.nBufferCountActual);
                        MTK_OMX_FREE(mOutputBufferHdrs);
                        mOutputBufferHdrs = tmp;
                    }
                }
                memcpy(&mOutputPortDef, pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                // for YUV 420 sampling frame width height should be even
                mOutputPortDef.format.video.nFrameWidth = (mOutputPortDef.format.video.nFrameWidth % 2)?
                    (mOutputPortDef.format.video.nFrameWidth + 1) : mOutputPortDef.format.video.nFrameWidth;
                mOutputPortDef.format.video.nFrameHeight = (mOutputPortDef.format.video.nFrameHeight % 2)?
                    (mOutputPortDef.format.video.nFrameHeight + 1) : mOutputPortDef.format.video.nFrameHeight;
                MTK_OMX_LOGD("mOutputPortDef.format.video.nFrameWidth=%d, mOutputPortDef.format.video.nFrameHeight=%d",
                             (int)mOutputPortDef.format.video.nFrameWidth,
                             (int)mOutputPortDef.format.video.nFrameHeight);
                #ifdef SUPPORT_NATIVE_HANDLE
                if(mIsAllocateOutputNativeBuffers){
                    mOutputPortDef.nBufferSize = sizeof(native_handle_t) + sizeof(int) * 12;
                }
                else
                #endif
                {
                    if ((mOutputPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingHEVC) &&
                        ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight)
                         > FHD_AREA))
                    {
                        mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_HEVC_4K;
                    }

                    if ((mOutputPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingAVC) &&
                        ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight)
                         > HD_AREA))
                    {
                        mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_AVC_1080P;
                    }
                    if ((mOutputPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingVP8) &&
                        ((mOutputPortDef.format.video.nFrameWidth * mOutputPortDef.format.video.nFrameHeight)
                         > HD_AREA))
                    {
                        mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_VP8_1080P;
                    }
                }


                MTK_OMX_LOGD("@@ Set output port: nFrameWidth(%d), nFrameHeight(%d), nStride(%d), nSliceHeight(%d),"
                             "nBitrate(%d), xFramerate(0x%X), compressionFormat(0x%08X), nBufferSize(%d),"
                             "nBufferCountActual(%d)",
                             (int)mOutputPortDef.format.video.nFrameWidth,
                             (int)mOutputPortDef.format.video.nFrameHeight, (int)mOutputPortDef.format.video.nStride,
                             (int)mOutputPortDef.format.video.nSliceHeight, (int)mOutputPortDef.format.video.nBitrate,
                             (unsigned int)mOutputPortDef.format.video.xFramerate,
                             (unsigned int)mOutputPortDef.format.video.eCompressionFormat,
                             (int)mOutputPortDef.nBufferSize, (int)mOutputPortDef.nBufferCountActual);
            }
            break;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pCompParam;
            if (!checkOMXParams(pPortFormat)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortFormat->nPortIndex == mInputPortFormat.nPortIndex)
            {
                // TODO: should we allow setting the input port param?
                mInputPortFormat.eColorFormat = pPortFormat->eColorFormat;
                mInputPortDef.format.video.eColorFormat = pPortFormat->eColorFormat;
            }
            else if (pPortFormat->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                mOutputPortFormat.eColorFormat = pPortFormat->eColorFormat;
            }
            MTK_OMX_LOGD_ENG("OMX_IndexParamVideoPortFormat port %d, eColorFormat 0x%x ",
                         pPortFormat->nPortIndex, pPortFormat->eColorFormat);
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pRoleParams = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;
            if (!checkOMXParams(pRoleParams)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            *((char *)mempcpy((char *)mCompRole, (char *)pRoleParams->cRole, sizeof(mCompRole) - 1)) = '\0';
            break;
        }

        case OMX_IndexParamVideoAvc:
        {
            OMX_VIDEO_PARAM_AVCTYPE *pAvcType = (OMX_VIDEO_PARAM_AVCTYPE *)pCompParam;
            if (!checkOMXParams(pAvcType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pAvcType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mAvcType, pCompParam, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoHevc:
        {
            OMX_VIDEO_PARAM_HEVCTYPE *pHevcType = (OMX_VIDEO_PARAM_HEVCTYPE *)pCompParam;
            if (pHevcType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mHevcType, pCompParam, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoH263:
        {
            OMX_VIDEO_PARAM_H263TYPE *pH263Type = (OMX_VIDEO_PARAM_H263TYPE *)pCompParam;
            if (!checkOMXParams(pH263Type)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pH263Type->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mH263Type, pCompParam, sizeof(OMX_VIDEO_PARAM_H263TYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoMpeg4:
        {
            OMX_VIDEO_PARAM_MPEG4TYPE *pMpeg4Type = (OMX_VIDEO_PARAM_MPEG4TYPE *)pCompParam;
            if (!checkOMXParams(pMpeg4Type)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pMpeg4Type->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mMpeg4Type, pCompParam, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoVp8:
        {
            OMX_VIDEO_PARAM_VP8TYPE *pVP8Type = (OMX_VIDEO_PARAM_VP8TYPE *)pCompParam;
            if (!checkOMXParams(pVP8Type)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pVP8Type->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mVp8Type, pCompParam, sizeof(OMX_VIDEO_PARAM_VP8TYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoBitrate:
        {
            OMX_VIDEO_PARAM_BITRATETYPE *pBitrateType = (OMX_VIDEO_PARAM_BITRATETYPE *)pCompParam;
            if (!checkOMXParams(pBitrateType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pBitrateType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mBitrateType, pBitrateType, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
                MTK_OMX_LOGD("nTargetBitrate=%d, mode=%x", (int)pBitrateType->nTargetBitrate,
                             pBitrateType->eControlRate);
                if (pBitrateType->eControlRate == OMX_Video_ControlRateMtkWFD)
                {
                    mSetWFDMode = OMX_TRUE;
                    mWFDMode = OMX_TRUE;
                }
                else if (pBitrateType->eControlRate == OMX_Video_ControlRateMtkStreaming)
                {
                    mSetStreamingMode = OMX_TRUE;
                    mSetWFDMode = OMX_FALSE;
                    mWFDMode = OMX_FALSE;
                }
                else if (pBitrateType->eControlRate == OMX_Video_ControlRateConstant)
                {
                    mSetConstantBitrateMode = OMX_TRUE;
                    mSetWFDMode = OMX_FALSE;
                    mWFDMode = OMX_FALSE;
                }
                else if(pBitrateType->eControlRate == OMX_Video_ControlRateConstantSkipFrames)
                {
                    setViLTEOn();
                    mSetWFDMode = OMX_FALSE;
                    mWFDMode = OMX_FALSE;
                }
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoQuantization:
        {
            OMX_VIDEO_PARAM_QUANTIZATIONTYPE *pQuantizationType = (OMX_VIDEO_PARAM_QUANTIZATIONTYPE *)pCompParam;
            if (!checkOMXParams(pQuantizationType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pQuantizationType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mQuantizationType, pQuantizationType, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoVBSMC:
        {
            OMX_VIDEO_PARAM_VBSMCTYPE *pVbsmcType = (OMX_VIDEO_PARAM_VBSMCTYPE *)pCompParam;
            if (!checkOMXParams(pVbsmcType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pVbsmcType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mVbsmcType, pVbsmcType, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoMotionVector:
        {
            OMX_VIDEO_PARAM_MOTIONVECTORTYPE *pMvType = (OMX_VIDEO_PARAM_MOTIONVECTORTYPE *)pCompParam;
            if (!checkOMXParams(pMvType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pMvType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mMvType, pMvType, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoIntraRefresh:
        {
            OMX_VIDEO_PARAM_INTRAREFRESHTYPE *pIntraRefreshType = (OMX_VIDEO_PARAM_INTRAREFRESHTYPE *)pCompParam;
            if (!checkOMXParams(pIntraRefreshType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pIntraRefreshType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mIntraRefreshType, pIntraRefreshType, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoSliceFMO:
        {
            OMX_VIDEO_PARAM_AVCSLICEFMO *pAvcSliceFMO = (OMX_VIDEO_PARAM_AVCSLICEFMO *)pCompParam;
            if (!checkOMXParams(pAvcSliceFMO)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pAvcSliceFMO->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mAvcSliceFMO, pAvcSliceFMO, sizeof(OMX_VIDEO_PARAM_AVCSLICEFMO));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoErrorCorrection:
        {
            OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE *pErrorCorrectionType =
                (OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE *)pCompParam;
            if (!checkOMXParams(pErrorCorrectionType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pErrorCorrectionType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mErrorCorrectionType, pErrorCorrectionType, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexVendorMtkOmxVenc3DVideoRecode:    // for MTK S3D SUPPORT
        {
            m3DVideoRecordMode = *((OMX_VIDEO_H264FPATYPE *)pCompParam);
            break;
        }

#ifdef ANDROID_ICS
        case OMX_IndexVendorMtkOmxVencSetTimelapseMode:
        {
            mIsTimeLapseMode = *((OMX_BOOL *)pCompParam);
            if (OMX_TRUE == mIsTimeLapseMode)
            {
                MTK_OMX_LOGD("Enable Timelapse mode");
            }
            break;
        }

        case OMX_IndexVendorMtkOmxVencSetWhiteboardEffectMode:
        {
            mIsWhiteboardEffectMode = *((OMX_BOOL *)pCompParam);
            if (OMX_TRUE == mIsWhiteboardEffectMode)
            {
                MTK_OMX_LOGD("Enable Whiteboard Effect mode");
            }
            break;
        }

        case OMX_IndexVendorMtkOmxVencSetMCIMode:
        {
            mIsMCIMode = *((OMX_BOOL *)pCompParam);
            if (OMX_TRUE == mIsMCIMode)
            {
                MTK_OMX_LOGD("Enable MCI mode");
                mInputMVAMgr->setBoolProperty("MCI", mIsMCIMode);
                mOutputMVAMgr->setBoolProperty("MCI", mIsMCIMode);
            }
            break;
        }

        case OMX_GoogleAndroidIndexStoreMetaDataInBuffers: // Morris Yang 20120214 add for live effect recording
        {
            StoreMetaDataInBuffersParams *pStoreMetaDataInBuffersParams = (StoreMetaDataInBuffersParams *)pCompParam;
            if (pStoreMetaDataInBuffersParams->nPortIndex == mInputPortFormat.nPortIndex)
            {
                mStoreMetaDataInBuffers = pStoreMetaDataInBuffersParams->bStoreMetaData;

                MTK_OMX_LOGD("@@ mStoreMetaDataInBuffers(%d)", mStoreMetaDataInBuffers);
                if (mStoreMetaDataInBuffers) {
                    mInputPortDef.nBufferSize = sizeof(VideoNativeMetadata);
                }
            }
            else if (pStoreMetaDataInBuffersParams->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                mStoreMetaDataInOutBuffers = pStoreMetaDataInBuffersParams->bStoreMetaData;
                MTK_OMX_LOGD("@@ mStoreMetaDataInOutBuffers(%d)", mStoreMetaDataInOutBuffers);
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }
        // Gary Wu add for MediaCodec encode with input data format is RGB
        case OMX_IndexVendorMtkOmxVdecACodecEncodeRGB2YUVMode:
        {
            MTK_OMX_LOGD("This function was removed!");
            err = OMX_ErrorUnsupportedIndex;
            break;
        }

        case OMX_IndexVendorMtkOmxVencSetScenario:
        {
            OMX_PARAM_U32TYPE *pSetScenarioInfo = (OMX_PARAM_U32TYPE *)pCompParam;
            OMX_U32 tmpVal = pSetScenarioInfo->nU32;
            if (tmpVal == OMX_VIDEO_MTKSpecificScenario_LivePhoto) { mIsLivePhoto = true; }
            if (tmpVal == OMX_VIDEO_MTKSpecificScenario_CrossMount) { mIsCrossMount = true; }
            MTK_OMX_LOGD("@@ set venc scenario %lu", tmpVal);
            break;
        }

        case OMX_IndexVendorMtkOmxVencPrependSPSPPS:
        {
            //caution that some platform's AVC driver don't support this feature!
            if (1)//always turn on
            {
                PrependSPSPPSToIDRFramesParams *tmpVal = (PrependSPSPPSToIDRFramesParams *)pCompParam;
                mPrependSPSPPSToIDRFrames = tmpVal->bEnable;
                mPrependSPSPPSToIDRFramesNotify = OMX_TRUE;
                //VTS will set prepend header, don't turn WFD settings on by this settings to avoid enable dummy NAL.
                MTK_OMX_LOGD("@@ set prepend header %d, mPrependSPSPPSToIDRFrames: %d", tmpVal->bEnable, mPrependSPSPPSToIDRFrames);

                {// temp solution to fix unsupport prepend issue for mt8167
                    char value[PROPERTY_VALUE_MAX];
                    bool unsupport_prepend;
                    property_get("vendor.mtk.omxvenc.unsupportprepend", value, "0");
                    unsupport_prepend = (bool)atoi(value);
                    if (unsupport_prepend)
                    {
                        MTK_OMX_LOGD("temp solution to fix unsupport prepend issue for mt8167");
                        err = OMX_ErrorUnsupportedIndex;
                    }
                }
            }
            else
            {
                MTK_OMX_LOGD("MtkOmxVenc::SetParameter unsupported prepend header");
                err = OMX_ErrorUnsupportedIndex;
            }
            break;
        }
        break;

        case OMX_IndexVendorMtkOmxVencNonRefPOp:
        {
            OMX_VIDEO_NONREFP *tmpVal = (OMX_VIDEO_NONREFP *)pCompParam;
            mEnableNonRefP = (OMX_BOOL)tmpVal->bEnable;
            MTK_OMX_LOGD("@@ enable non-ref P");
        }
        break;

        // Morris Yang 20121203
        case OMX_IndexVendorMtkOmxVideoUseIonBuffer:
        {
            MTK_OMX_LOGD("This function was removed!");
            err = OMX_ErrorUnsupportedIndex;
            break;
        }

        case OMX_IndexVendorMtkOmxVideoSetClientLocally:
        {
            OMX_CONFIG_BOOLEANTYPE *pClientLocallyInfo = (OMX_CONFIG_BOOLEANTYPE *)pCompParam;
            mIsClientLocally = pClientLocallyInfo->bEnabled;
            MTK_OMX_LOGD("@@ mIsClientLocally(%d)", mIsClientLocally);
            break;
        }

        case OMX_GoogleAndroidIndexstoreANWBufferInMetadata:
        {
            StoreMetaDataInBuffersParams *pStoreMetaDataInBuffersParams = (StoreMetaDataInBuffersParams *)pCompParam;
            if (pStoreMetaDataInBuffersParams->nPortIndex == mInputPortFormat.nPortIndex)
            {
                char value2[PROPERTY_VALUE_MAX];
                property_get("vendor.mtk.omxvdec.USANWInMetadata", value2, "0");
                OMX_BOOL  mDisableANWInMetadata = (OMX_BOOL) atoi(value2);
                if( 1 == mDisableANWInMetadata )
                {
                    MTK_OMX_LOGD("@@ OMX_GoogleAndroidIndexstoreANWBufferInMetadata return un-support by setting property");
                    err = OMX_ErrorUnsupportedIndex;
                    goto EXIT;

                }
                mStoreMetaDataInBuffers = pStoreMetaDataInBuffersParams->bStoreMetaData;
                MTK_OMX_LOGD("@@ OMX_GoogleAndroidIndexstoreANWBufferInMetadata");
                MTK_OMX_LOGD("@@ mStoreMetaDataInBuffers(%d) for port(%d)", mStoreMetaDataInBuffers, pStoreMetaDataInBuffersParams->nPortIndex);
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }
#ifdef SUPPORT_NATIVE_HANDLE
        case OMX_GoogleAndroidIndexEnableAndroidNativeHandle:
        {
            AllocateNativeHandleParams *pConfig =
                (AllocateNativeHandleParams *) pCompParam;
            if (NULL == pConfig)
            {
                MTK_OMX_LOGE("OMX_GoogleAndroidIndexAllocateAndroidNativeHandle: NULL input");
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }
            if (pConfig->nPortIndex != mOutputPortFormat.nPortIndex)
            {
                MTK_OMX_LOGE("OMX_GoogleAndroidIndexAllocateAndroidNativeHandle: does not support input port");
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }
            mIsAllocateOutputNativeBuffers = pConfig->enable;
            if (OMX_TRUE == mIsAllocateOutputNativeBuffers)
            {
                mStoreMetaDataInOutBuffers = OMX_TRUE;
            }
            MTK_OMX_LOGD("OMX_GoogleAndroidIndexAllocateAndroidNativeHandle enable(%d), mStoreMetaDataInOutBuffers(%d)",
                pConfig->enable, mStoreMetaDataInOutBuffers);
            break;
        }
#endif
#endif

        case OMX_IndexParamAndroidVideoTemporalLayering:
        {
            OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE *pLayerParams =
                (OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE *)pCompParam;
            if (!checkOMXParams(pLayerParams)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            if (pLayerParams->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                MTK_OMX_LOGD("SetParameter VideoTemporalLayering eSupportedPatterns %d, ePattern %d, "
                    "nLayerCountMax %d, nBLayerCountMax %d, nPLayerCountActual %d, nBLayerCountActual %d, "
                    "bBitrateRatiosSpecified %d, nBitrateRatios[0] %d",
                    pLayerParams->eSupportedPatterns, pLayerParams->ePattern, pLayerParams->nLayerCountMax,
                    pLayerParams->nBLayerCountMax, pLayerParams->nPLayerCountActual, pLayerParams->nBLayerCountActual,
                    pLayerParams->bBitrateRatiosSpecified, pLayerParams->nBitrateRatios[0]);

                memcpy(&mLayerParams, pLayerParams, sizeof(mLayerParams));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        default:
        {
            MTK_OMX_LOGE("MtkOmxVenc::SetParameter unsupported nParamIndex(0x%08X)", nParamIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN  OMX_INDEXTYPE nParamIndex,
                                       OMX_INOUT OMX_PTR pCompParam)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD_ENG("MtkOmxVenc::GetParameter (0x%08X)", nParamIndex);
    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    if (NULL == pCompParam)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    callReturnIfNoError(mExtensions.GetParameter, nParamIndex, pCompParam);

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;
            if (!checkOMXParams(pPortDef)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortDef->nPortIndex == mInputPortDef.nPortIndex)
            {
                memcpy(pCompParam, &mInputPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else if (pPortDef->nPortIndex == mOutputPortDef.nPortIndex)
            {
                memcpy(pCompParam, &mOutputPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            break;
        }

        case OMX_IndexParamVideoInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = MTK_OMX_INPUT_PORT;
            pPortParam->nPorts = 2;
            break;
        }

        case OMX_IndexParamAudioInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }

        case OMX_IndexParamImageInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }

        case OMX_IndexParamOtherInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pPortFormat = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)pCompParam;
            if (!checkOMXParams(pPortFormat)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortFormat->nPortIndex == mInputPortFormat.nPortIndex)
            {
                if (pPortFormat->nIndex == 0)
                {
                    pPortFormat->eColorFormat =  mInputPortFormat.eColorFormat;
                    pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                }
                // Morris Yang 20120214 add for live effect recording [
                else if (pPortFormat->nIndex == 1)
                {
                    pPortFormat->eColorFormat =  OMX_COLOR_FormatAndroidOpaque;
                    pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                }
                // ]
                else if (pPortFormat->nIndex == 2)
                {
                    pPortFormat->eColorFormat =  OMX_COLOR_FormatYUV420Flexible;
                    pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                }
                else if (pPortFormat->nIndex == 3)
                {
                    pPortFormat->eColorFormat =  OMX_MTK_COLOR_FormatYV12;
                    pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                }
                else if (pPortFormat->nIndex == 4)
                {
                    pPortFormat->eColorFormat =  OMX_COLOR_FormatYUV420Planar;
                    pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                }
                else if (pPortFormat->nIndex == 5)
                {
                    pPortFormat->eColorFormat =  OMX_COLOR_Format16bitRGB565;
                    pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                }
                else if (pPortFormat->nIndex == 6)
                {
                    pPortFormat->eColorFormat =  OMX_COLOR_Format24bitRGB888;
                    pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                }
                else if (pPortFormat->nIndex == 7)
                {
                    pPortFormat->eColorFormat =  OMX_COLOR_Format32bitARGB8888;
                    pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                }
                /* for DirectLink Meta Mode + */
                else if (pPortFormat->nIndex == 8)
                {
                    pPortFormat->eColorFormat =  OMX_MTK_COLOR_FormatBitStream;
                    pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                }
                /* for DirectLink Meta Mode - */
                else if (pPortFormat->nIndex == 9)
                {
                    pPortFormat->eColorFormat =  OMX_COLOR_Format32bitBGRA8888;
                    pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                }
                else if (pPortFormat->nIndex == 10)
                {
                    if (supportNV12())
                    {
                        pPortFormat->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
                        pPortFormat->eCompressionFormat = OMX_VIDEO_CodingUnused;
                    }
                    else
                    {
                        err = OMX_ErrorNoMore;
                    }
                }
                else
                {
                    err = OMX_ErrorNoMore;
                }
                MTK_OMX_LOGD_ENG("[GetParameter] OMX_IndexParamVideoPortFormat index = %lu, color format = %d\n",
                             pPortFormat->nIndex, pPortFormat->eColorFormat);
            }
            else if (pPortFormat->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                if (pPortFormat->nIndex == 0)
                {
                    pPortFormat->eColorFormat = OMX_COLOR_FormatUnused;
                    pPortFormat->eCompressionFormat =  mOutputPortFormat.eCompressionFormat;
                }
                else
                {
                    err = OMX_ErrorNoMore;
                }
            }
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pRoleParams = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;
            if (!checkOMXParams(pRoleParams)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            *((char *)mempcpy((char *)pRoleParams->cRole, (char *)mCompRole, sizeof(pRoleParams->cRole) - 1)) = '\0';
            break;
        }

        case OMX_IndexParamVideoAvc:
        {
            OMX_VIDEO_PARAM_AVCTYPE *pAvcType = (OMX_VIDEO_PARAM_AVCTYPE *)pCompParam;
            if (!checkOMXParams(pAvcType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pAvcType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mAvcType, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoHevc:
        {
            OMX_VIDEO_PARAM_HEVCTYPE *pHevcType = (OMX_VIDEO_PARAM_HEVCTYPE *)pCompParam;
            if (pHevcType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mHevcType, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoH263:
        {
            OMX_VIDEO_PARAM_H263TYPE *pH263Type = (OMX_VIDEO_PARAM_H263TYPE *)pCompParam;
            if (!checkOMXParams(pH263Type)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pH263Type->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mH263Type, sizeof(OMX_VIDEO_PARAM_H263TYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoMpeg4:
        {
            OMX_VIDEO_PARAM_MPEG4TYPE *pMpeg4Type = (OMX_VIDEO_PARAM_MPEG4TYPE *)pCompParam;
            if (!checkOMXParams(pMpeg4Type)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pMpeg4Type->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mMpeg4Type, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoVp8:
        {
            OMX_VIDEO_PARAM_VP8TYPE *pVP8Type = (OMX_VIDEO_PARAM_VP8TYPE *)pCompParam;
            if (!checkOMXParams(pVP8Type)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pVP8Type->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mVp8Type, sizeof(OMX_VIDEO_PARAM_VP8TYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *pProfileLevel = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)pCompParam;
            if (!checkOMXParams(pProfileLevel)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pProfileLevel->nPortIndex == mOutputPortFormat.nPortIndex)
            {

                switch (mOutputPortDef.format.video.eCompressionFormat)
                {
                    case OMX_VIDEO_CodingAVC:
                        if (pProfileLevel->nProfileIndex >= MAX_AVC_PROFILE_LEVEL_TABLE_SZIE)
                        {
                            err = OMX_ErrorNoMore;;
                            goto EXIT;
                        }
                        else
                        {
                            pProfileLevel->eProfile = AvcProfileLevelTable[pProfileLevel->nProfileIndex].profile;
                            pProfileLevel->eLevel = AvcProfileLevelTable[pProfileLevel->nProfileIndex].level;

                            if (OMX_ErrorNone !=
                                QueryVideoProfileLevel(VENC_DRV_VIDEO_FORMAT_H264,
                                                       Omx2DriverH264ProfileMap((OMX_VIDEO_AVCPROFILETYPE)pProfileLevel->eProfile),
                                                       Omx2DriverH264LevelMap((OMX_VIDEO_AVCLEVELTYPE)pProfileLevel->eLevel)))
                            {
                                err = OMX_ErrorNoMore;
                                goto EXIT;
                            }
                        }
                        break;

                    case OMX_VIDEO_CodingHEVC:
                        if (pProfileLevel->nProfileIndex >= MAX_HEVC_PROFILE_LEVEL_TABLE_SZIE)
                        {
                            err = OMX_ErrorNoMore;;
                            goto EXIT;
                        }
                        else
                        {
                            pProfileLevel->eProfile = HevcProfileLevelTable[pProfileLevel->nProfileIndex].profile;
                            pProfileLevel->eLevel = HevcProfileLevelTable[pProfileLevel->nProfileIndex].level;
                        }
                        break;

                    case OMX_VIDEO_CodingMPEG4:
                        if (pProfileLevel->nProfileIndex >= MAX_MPEG4_PROFILE_LEVEL_TABLE_SZIE)
                        {
                            err = OMX_ErrorNoMore;;
                            goto EXIT;
                        }
                        else
                        {
                            pProfileLevel->eProfile = MPEG4ProfileLevelTable[pProfileLevel->nProfileIndex].profile;
                            pProfileLevel->eLevel = MPEG4ProfileLevelTable[pProfileLevel->nProfileIndex].level;

                            if (OMX_ErrorNone != QueryVideoProfileLevel(VENC_DRV_VIDEO_FORMAT_MPEG4,
                                                                        Omx2DriverMPEG4ProfileMap((OMX_VIDEO_MPEG4PROFILETYPE)pProfileLevel->eProfile),
                                                                        Omx2DriverMPEG4LevelMap((OMX_VIDEO_MPEG4LEVELTYPE)pProfileLevel->eLevel)))
                            {
                                err = OMX_ErrorNoMore;
                                goto EXIT;
                            }
                        }
                        break;

                    case OMX_VIDEO_CodingH263:
                        if (pProfileLevel->nProfileIndex >= MAX_H263_PROFILE_LEVEL_TABLE_SZIE)
                        {
                            err = OMX_ErrorNoMore;;
                            goto EXIT;
                        }
                        else
                        {
                            pProfileLevel->eProfile = H263ProfileLevelTable[pProfileLevel->nProfileIndex].profile;
                            pProfileLevel->eLevel = H263ProfileLevelTable[pProfileLevel->nProfileIndex].level;

                            if (OMX_ErrorNone != QueryVideoProfileLevel(VENC_DRV_VIDEO_FORMAT_H263,
                                                                        Omx2DriverH263ProfileMap((OMX_VIDEO_H263PROFILETYPE)pProfileLevel->eProfile),
                                                                        Omx2DriverH263LevelMap((OMX_VIDEO_H263LEVELTYPE)pProfileLevel->eLevel)))
                            {
                                err = OMX_ErrorNoMore;
                                goto EXIT;
                            }
                        }
                        break;

                    case OMX_VIDEO_CodingVP8:
                    {
                        switch (pProfileLevel->nProfileIndex)
                        {
                            case 0:
                                pProfileLevel->eLevel = OMX_VIDEO_VP8Level_Version0;
                                break;

                            case 1:
                                pProfileLevel->eLevel = OMX_VIDEO_VP8Level_Version1;
                                break;

                            case 2:
                                pProfileLevel->eLevel = OMX_VIDEO_VP8Level_Version2;
                                break;

                            case 3:
                                pProfileLevel->eLevel = OMX_VIDEO_VP8Level_Version3;
                                break;

                            default:
                                return OMX_ErrorNoMore;
                        }

                        pProfileLevel->eProfile = OMX_VIDEO_VP8ProfileMain;
                        return OMX_ErrorNone;
                    }
                    default:
                        err = OMX_ErrorBadParameter;
                        goto EXIT;
                }
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoBitrate:
        {
            OMX_VIDEO_PARAM_BITRATETYPE *pBitrateType = (OMX_VIDEO_PARAM_BITRATETYPE *)pCompParam;
            if (!checkOMXParams(pBitrateType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pBitrateType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mBitrateType, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoQuantization:
        {
            OMX_VIDEO_PARAM_QUANTIZATIONTYPE *pQuantizationType = (OMX_VIDEO_PARAM_QUANTIZATIONTYPE *)pCompParam;
            if (!checkOMXParams(pQuantizationType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pQuantizationType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mQuantizationType, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoVBSMC:
        {
            OMX_VIDEO_PARAM_VBSMCTYPE *pVbsmcType = (OMX_VIDEO_PARAM_VBSMCTYPE *)pCompParam;
            if (!checkOMXParams(pVbsmcType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pVbsmcType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mVbsmcType, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoMotionVector:
        {
            OMX_VIDEO_PARAM_MOTIONVECTORTYPE *pMvType = (OMX_VIDEO_PARAM_MOTIONVECTORTYPE *)pCompParam;
            if (!checkOMXParams(pMvType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pMvType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mMvType, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoIntraRefresh:
        {
            OMX_VIDEO_PARAM_INTRAREFRESHTYPE *pIntraRefreshType = (OMX_VIDEO_PARAM_INTRAREFRESHTYPE *)pCompParam;
            if (!checkOMXParams(pIntraRefreshType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pIntraRefreshType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mIntraRefreshType, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoSliceFMO:
        {
            OMX_VIDEO_PARAM_AVCSLICEFMO *pAvcSliceFMO = (OMX_VIDEO_PARAM_AVCSLICEFMO *)pCompParam;
            if (!checkOMXParams(pAvcSliceFMO)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pAvcSliceFMO->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mAvcSliceFMO, sizeof(OMX_VIDEO_PARAM_AVCSLICEFMO));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexParamVideoErrorCorrection:
        {
            OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE *pErrorCorrectionType =
                (OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE *)pCompParam;

            if (!checkOMXParams(pErrorCorrectionType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pErrorCorrectionType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompParam, &mErrorCorrectionType, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexVendorMtkOmxVencNonRefPOp:
        {
            OMX_VIDEO_NONREFP *tmpVal = (OMX_VIDEO_NONREFP *)pCompParam;
            if (mEnableNonRefP == OMX_TRUE)
            {
                //8173 8163 D1 D2 D3 70 80 55 57 97 99
                VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
                OMX_U32 uAVCNonRefPFreq = 0;
                mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_NON_REF_P_FREQUENCY, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uAVCNonRefPFreq);

                MTK_OMX_LOGD("uAVCNonRefPFreq %d", uAVCNonRefPFreq);

                if( uAVCNonRefPFreq && (mOutputPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC) )
                {
                    tmpVal->nFreq = ((3 << 16) | 4); //give non ref p frequency = (3/4) on ROME
                }
            }
            else
            {
                tmpVal->nFreq = 0;
            }
            MTK_OMX_LOGD("@@ get non-ref P freq %d", tmpVal->nFreq);
        }
        break;

        case OMX_IndexVendorMtkOmxHandle:
        {
            OMX_U32 *pHandle = (OMX_U32 *)pCompParam;
            *pHandle = (OMX_U32)this;
            break;
        }

        case OMX_IndexVendorMtkQueryDriverVersion:
        {
            OMX_VIDEO_PARAM_DRIVERVER *pDriver = (OMX_VIDEO_PARAM_DRIVERVER *)pCompParam;
            pDriver->nDriverVersion = (DRIVERVER_RESERVED << 32) + (DRIVERVER_MAJOR << 16) + (DRIVERVER_MINOR);
            MTK_OMX_LOGD("Query Driver version: %d, %d, %llu",
                         pDriver->nSize, sizeof(OMX_VIDEO_PARAM_DRIVERVER), pDriver->nDriverVersion);
            break;
        }

        case OMX_IndexVendorMtkOmxVencQueryCodecsSizes:
        {
            OMX_VIDEO_PARAM_SPEC_QUERY* pSpecQuery = (OMX_VIDEO_PARAM_SPEC_QUERY*)pCompParam;
            VENC_DRV_QUERY_VIDEO_FORMAT_T qinfoOut;
            MTK_OMX_MEMSET(&qinfoOut, 0, sizeof(qinfoOut));
            if (OMX_FALSE == QueryDriverFormat(&qinfoOut))
            {
                MTK_OMX_LOGE("Error!! Cannot get decoder property.");
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }
            pSpecQuery->profile = qinfoOut.u4Profile;
            pSpecQuery->level = qinfoOut.eLevel;
            pSpecQuery->nFrameWidth = qinfoOut.u4Width;
            pSpecQuery->nFrameHeight = qinfoOut.u4Height;

            break;
        }

        case OMX_GoogleAndroidIndexDescribeColorFormat:
        {
            DescribeColorFormatParams *describeParams = (DescribeColorFormatParams *)pCompParam;
            MTK_OMX_LOGD_ENG("DescribeColorFormat %lx, bUsingNativeBuffers %d, mbYUV420FlexibleMode %d", describeParams->eColorFormat,
                         describeParams->bUsingNativeBuffers, mbYUV420FlexibleMode);
            #if 1
            if ((OMX_FALSE == describeParams->bUsingNativeBuffers) &&
                 ((HAL_PIXEL_FORMAT_I420 == describeParams->eColorFormat)||
                  (OMX_COLOR_FormatYUV420Planar == describeParams->eColorFormat) ||
                  (OMX_COLOR_FormatYUV420Flexible == describeParams->eColorFormat)) )
            {
                bool err_return = 0;
                mbYUV420FlexibleMode = OMX_TRUE;
                err_return = DescribeFlexibleColorFormat((DescribeColorFormatParams *)describeParams);

                MTK_OMX_LOGD("client query OMX_COLOR_FormatYUV420Flexible mbYUV420FlexibleMode %d, ret: %d", mbYUV420FlexibleMode, err_return);
            }
            else
            #endif
            {
                //treat the framework to push YUVFlexible format in codeccodec::queryCodecs()
                err = OMX_ErrorUnsupportedIndex;
            }
            break;
        }

        case OMX_GoogleAndroidIndexDescribeColorFormat2:
        {
            DescribeColorFormat2Params *describeParams = (DescribeColorFormat2Params *)pCompParam;
            MTK_OMX_LOGD_ENG("DescribeColorFormat2 %lx, bUsingNativeBuffers %d, mbYUV420FlexibleMode %d",
                describeParams->eColorFormat, describeParams->bUsingNativeBuffers, mbYUV420FlexibleMode);

            err = OMX_ErrorUnsupportedIndex;

            break;
        }

        case OMX_IndexParamAndroidVideoTemporalLayering:
        {
            OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE *pLayerParams =
                (OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE *)pCompParam;
            if (!checkOMXParams(pLayerParams)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            if (pLayerParams->nPortIndex == mOutputPortFormat.nPortIndex)
            {

                //video encoder not support B frame(HW&SW), so return necessary variables
                //see OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE class in OMX_VideoExt.h
                mLayerParams.nLayerCountMax = 1;
                mLayerParams.nBLayerCountMax = 0;
                mLayerParams.nPLayerCountActual = 1;
                mLayerParams.nBLayerCountActual = 0;

                MTK_OMX_LOGD("GetParameter VideoTemporalLayering eSupportedPatterns %d, ePattern %d, "
                    "nLayerCountMax %d, nBLayerCountMax %d, nPLayerCountActual %d, nBLayerCountActual %d, "
                    "bBitrateRatiosSpecified %d, nBitrateRatios[0] %d",
                    mLayerParams.eSupportedPatterns, mLayerParams.ePattern, mLayerParams.nLayerCountMax,
                    mLayerParams.nBLayerCountMax, mLayerParams.nPLayerCountActual, mLayerParams.nBLayerCountActual,
                    mLayerParams.bBitrateRatiosSpecified, mLayerParams.nBitrateRatios[0]);

                memcpy(pCompParam, &mLayerParams, sizeof(mLayerParams));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        default:
        {
            MTK_OMX_LOGE("MtkOmxVenc::GetParameter unsupported nParamIndex(0x%08X)", nParamIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_IN OMX_PTR pCompConfig)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD_ENG("MtkOmxVenc::SetConfig (0x%08X)", nConfigIndex);

    callReturnIfNoError(mExtensions.SetConfig, nConfigIndex, pCompConfig);

    switch (nConfigIndex)
    {
        case OMX_IndexConfigVideoFramerate:
        {
            OMX_CONFIG_FRAMERATETYPE *pFrameRateType = (OMX_CONFIG_FRAMERATETYPE *)pCompConfig;
            if (!checkOMXParams(pFrameRateType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nConfigIndex);
                return OMX_ErrorBadParameter;
            }

            if (pFrameRateType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mFrameRateType, pCompConfig, sizeof(OMX_CONFIG_FRAMERATETYPE));

                mFrameRateUpdated = OMX_TRUE;
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexConfigVideoBitrate:
        {
            OMX_VIDEO_CONFIG_BITRATETYPE *pConfigBitrate = (OMX_VIDEO_CONFIG_BITRATETYPE *)pCompConfig;
            if (!checkOMXParams(pConfigBitrate)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nConfigIndex);
                return OMX_ErrorBadParameter;
            }

            if (pConfigBitrate->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mConfigBitrate, pCompConfig, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));

                mBitRateUpdated = OMX_TRUE;

            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexConfigVideoIntraVOPRefresh:
        {
            OMX_CONFIG_INTRAREFRESHVOPTYPE *pConfigIntraRefreshVopType = (OMX_CONFIG_INTRAREFRESHVOPTYPE *)pCompConfig;
            if (!checkOMXParams(pConfigIntraRefreshVopType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nConfigIndex);
                return OMX_ErrorBadParameter;
            }

            if (pConfigIntraRefreshVopType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mConfigIntraRefreshVopType, pCompConfig, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
                mForceIFrame = mConfigIntraRefreshVopType.IntraRefreshVOP;
                MTK_OMX_LOGD("MtkOmxVenc::SetConfig -> Refresh Vop to %d", mConfigIntraRefreshVopType.IntraRefreshVOP);
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexConfigVideoAVCIntraPeriod:
        {
            OMX_VIDEO_CONFIG_AVCINTRAPERIOD *pConfigAVCIntraPeriod = (OMX_VIDEO_CONFIG_AVCINTRAPERIOD *)pCompConfig;
            if (pConfigAVCIntraPeriod->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mConfigAVCIntraPeriod, pCompConfig, sizeof(OMX_VIDEO_CONFIG_AVCINTRAPERIOD));
                mIInterval = mConfigAVCIntraPeriod.nPFrames;
                MTK_OMX_LOGD("OMX_IndexConfigVideoAVCIntraPeriod MtkOmxVenc::SetConfig -> I interval set to %d", mIInterval);
                mSetIInterval = OMX_TRUE;
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

#ifdef ANDROID_ICS
        case OMX_IndexVendorMtkOmxVencSetForceIframe:
        {
            OMX_PARAM_U32TYPE *pSetForceIframeInfo = (OMX_PARAM_U32TYPE *)pCompConfig;
            mForceIFrame = (OMX_BOOL)pSetForceIframeInfo->nU32;
            MTK_OMX_LOGD("MtkOmxVenc::SetConfig -> Force I frame set to %d", mForceIFrame);
            break;
        }
#endif

        case OMX_IndexVendorMtkOmxVencSetIInterval:
        {
            OMX_PARAM_U32TYPE *pIntervalInfo = (OMX_PARAM_U32TYPE *)pCompConfig;
            mIInterval = pIntervalInfo->nU32;
            MTK_OMX_LOGD("OMX_IndexVendorMtkOmxVencSetIInterval MtkOmxVenc::SetConfig -> I interval set to %d", mIInterval);
            mSetIInterval = OMX_TRUE;
            break;
        }

        case OMX_IndexVendorMtkOmxVencSkipFrame:
        {
            OMX_PARAM_U32TYPE *pSkipFrameInfo = (OMX_PARAM_U32TYPE *)pCompConfig;
            mSkipFrame = pSkipFrameInfo->nU32;
            MTK_OMX_LOGD("MtkOmxVenc::SetConfig -> Skip frame");
            break;
        }

        case OMX_IndexVendorMtkOmxVencDrawBlack://for Miracast test case SIGMA 5.1.11 workaround
        {
            OMX_PARAM_U32TYPE *pDrawBlackInfo = (OMX_PARAM_U32TYPE *)pCompConfig;
            int enable = pDrawBlackInfo->nU32;
            mDrawBlack = (enable == 0) ? OMX_FALSE : OMX_TRUE;
            break;
        }

        case OMX_IndexVendorMtkConfigQP:
        {
            OMX_VIDEO_CONFIG_QP *pConfig = (OMX_VIDEO_CONFIG_QP *)pCompConfig;
            mSetQP = OMX_TRUE;
            mQP = pConfig->nQP;
            break;
        }

        case OMX_IndexVendorMtkOmxVencSetForceFullIframe:
        {
            OMX_PARAM_U32TYPE *pForceFullIInfo = (OMX_PARAM_U32TYPE *)pCompConfig;
            mForceFullIFrame = (OMX_BOOL)pForceFullIInfo->nU32;
            mForceFullIFramePrependHeader = (OMX_BOOL)pForceFullIInfo->nU32;
            mPrependSPSPPSToIDRFramesNotify = OMX_TRUE;
            MTK_OMX_LOGD("MtkOmxVenc::SetConfig -> Force Full I frame set to %d, mForceFullIFramePrependHeader: %d, mPrependSPSPPSToIDRFrames: %d",
                          mForceFullIFrame, mForceFullIFramePrependHeader, mPrependSPSPPSToIDRFrames);
            break;
        }
        case OMX_IndexVendorMtkOmxSliceLossIndication:
        {
            OMX_CONFIG_SLICE_LOSS_INDICATION *pSLI = (OMX_CONFIG_SLICE_LOSS_INDICATION *) pCompConfig;
            memcpy(&mSLI, pSLI, sizeof(OMX_CONFIG_SLICE_LOSS_INDICATION));
            MTK_OMX_LOGD("[SLI][%d][%d]start:%d, count:%d", mSLI.nSliceCount,
                                      mSLI.SliceLoss[0] & 0x3F,
                                      mSLI.SliceLoss[0] >> 19,
                                      ((mSLI.SliceLoss[0] >> 6) & 0x1FFF));

            mGotSLI = OMX_TRUE;
            break;
        }
        case OMX_IndexConfigAndroidIntraRefresh:
        {
            OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE *pConfigAndroidIntraPeriod = (OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE *)pCompConfig;
            if (pConfigAndroidIntraPeriod->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(&mConfigAndroidIntraPeriod, pCompConfig, sizeof(OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE));
                mIDRInterval = mConfigAndroidIntraPeriod.nRefreshPeriod; // in  frames
                MTK_OMX_LOGD("MtkOmxVenc::SetConfig -> IDR interval set to %d", mIDRInterval);
                mSetIDRInterval = OMX_TRUE;
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }
        case OMX_IndexConfigOperatingRate:
        {
            OMX_PARAM_U32TYPE *pOperationRate = (OMX_PARAM_U32TYPE *)pCompConfig;
            mOperationRate = (unsigned int)(pOperationRate->nU32 / 65536.0f);
            MTK_OMX_LOGD("MtkOmxVenc::SetConfig ->operation rate set to %d", mOperationRate);
            break;
        }

        case OMX_IndexConfigAndroidVideoTemporalLayering:
        {
            OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE *pLayerConfig = (OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE *)pCompConfig;;

            if (pLayerConfig->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                MTK_OMX_LOGD("MtkOmxVenc::SetConfig -> VideoTemporalLayering ePattern %d, nPLayerCountActual %d, "
                    "nBLayerCountActual %d, bBitrateRatiosSpecified %d, nBitrateRatios[0] %d",
                    pLayerConfig->ePattern, pLayerConfig->nPLayerCountActual,
                    pLayerConfig->nBLayerCountActual, pLayerConfig->bBitrateRatiosSpecified, pLayerConfig->nBitrateRatios[0]);

                //todo: copy to venc member
                //memcpy(&mLayerConfig, &pCompConfig, sizeof(OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE));

                //return error temporary till implement this
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }
        case OMX_IndexVendorMtkOmxVencSeResolutionChange:
        {
            if (mPrepareToResolutionChange == OMX_TRUE)
            {
                MTK_OMX_LOGE("resolution change error!!!");
                err = OMX_ErrorUndefined;
            }
            else
            {
                OMX_VIDEO_PARAM_RESOLUTION *config = (OMX_VIDEO_PARAM_RESOLUTION *)pCompConfig;

                if (config->nFrameWidth > mInputPortDef.format.video.nFrameWidth || config->nFrameHeight > mInputPortDef.format.video.nFrameHeight)
                {
                    MTK_OMX_LOGD("MtkOmxVenc::SetConfig -> resolution change error %d, %d", config->nFrameWidth, config->nFrameHeight);
                    err = OMX_ErrorBadParameter;
                }
                else
                {
                    if (u4EncodeWidth == config->nFrameWidth && u4EncodeHeight == config->nFrameHeight)
                    {
                        MTK_OMX_LOGD("MtkOmxVenc::SetConfig -> resolution change error, the same resolution %d, %d", config->nFrameWidth, config->nFrameHeight);
                        err = OMX_ErrorBadParameter;
                    }
                    else
                    {
                        mPrepareToResolutionChange = OMX_TRUE;
                        u4EncodeWidth = config->nFrameWidth;
                        u4EncodeHeight = config->nFrameHeight;

                        MTK_OMX_LOGD("MtkOmxVenc::SetConfig -> resolution change to %d, %d", u4EncodeWidth, u4EncodeHeight);
                    }
                }
            }
            break;
        }
        case OMX_IndexVendorMtkOmxVencInputScaling:
        {
            OMX_CONFIG_BOOLEANTYPE *pInputScalingMode = (OMX_CONFIG_BOOLEANTYPE *)pCompConfig;
            // input scaling only for overspec
            if ( mInputPortDef.format.video.nFrameWidth <  mInputPortDef.format.video.nFrameHeight)
            {
                if (mInputPortDef.format.video.nFrameWidth <= mMaxScaledNarrow && mInputPortDef.format.video.nFrameHeight <= mMaxScaledWide)
                {
                    mInputScalingMode = OMX_FALSE;
                    break;
                }
            } else {
                if (mInputPortDef.format.video.nFrameWidth <= mMaxScaledWide && mInputPortDef.format.video.nFrameHeight <= mMaxScaledNarrow)
                {
                    mInputScalingMode = OMX_FALSE;
                    break;
                }
            }
            mInputScalingMode = (OMX_BOOL)pInputScalingMode->bEnabled;
            MTK_OMX_LOGD("MtkOmxVenc set mInputScalingMode %d; Scaling target resolution: wide: %d narrow %d", mInputScalingMode, mMaxScaledWide, mMaxScaledNarrow);
            break;
        }
        case OMX_IndexConfigCommonRotate:
        {
            OMX_CONFIG_ROTATIONTYPE *pRotate = (OMX_CONFIG_ROTATIONTYPE *)pCompConfig;

            /*
            typedef struct OMX_CONFIG_ROTATIONTYPE {
            OMX_U32 nSize;
            OMX_VERSIONTYPE nVersion;
            OMX_U32 nPortIndex;
            OMX_S32 nRotation;
            } OMX_CONFIG_ROTATIONTYPE;
            */

            // input scaling only for overspec
            if (pRotate->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                MTK_OMX_LOGD("MtkOmxVenc::SetConfig OMX_IndexConfigCommonRotate size %d nVersion %d portindex %d nRotation %d", pRotate->nSize, pRotate->nVersion.nVersion, pRotate->nPortIndex, pRotate->nRotation);

                mRotationType.nRotation = pRotate->nRotation;
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }
        default:
        {
            MTK_OMX_LOGE("MtkOmxVenc::GetParameter unsupported nConfigIndex(0x%08X)", nConfigIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_INOUT OMX_PTR pCompConfig)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD_ENG("MtkOmxVenc::GetConfig (0x%08X)", nConfigIndex);

    callReturnIfNoError(mExtensions.GetConfig, nConfigIndex, pCompConfig);

    switch (nConfigIndex)
    {
        case OMX_IndexConfigVideoFramerate:
        {
            OMX_CONFIG_FRAMERATETYPE *pFrameRateType = (OMX_CONFIG_FRAMERATETYPE *)pCompConfig;
            if (!checkOMXParams(pFrameRateType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nConfigIndex);
                return OMX_ErrorBadParameter;
            }

            if (pFrameRateType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompConfig, &mFrameRateType, sizeof(OMX_CONFIG_FRAMERATETYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexConfigVideoBitrate:
        {
            OMX_VIDEO_CONFIG_BITRATETYPE *pConfigBitrate = (OMX_VIDEO_CONFIG_BITRATETYPE *)pCompConfig;
            if (!checkOMXParams(pConfigBitrate)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nConfigIndex);
                return OMX_ErrorBadParameter;
            }

            if (pConfigBitrate->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompConfig, &mConfigBitrate, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }

        case OMX_IndexConfigVideoIntraVOPRefresh:
        {
            OMX_CONFIG_INTRAREFRESHVOPTYPE *pConfigIntraRefreshVopType = (OMX_CONFIG_INTRAREFRESHVOPTYPE *)pCompConfig;
            if (!checkOMXParams(pConfigIntraRefreshVopType)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nConfigIndex);
                return OMX_ErrorBadParameter;
            }

            if (pConfigIntraRefreshVopType->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompConfig, &mConfigIntraRefreshVopType, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            break;
        }
        case OMX_IndexConfigVideoAVCIntraPeriod:
        {
            OMX_VIDEO_CONFIG_AVCINTRAPERIOD *pConfigAVCIntraPeriod = (OMX_VIDEO_CONFIG_AVCINTRAPERIOD *)pCompConfig;
            if (pConfigAVCIntraPeriod->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompConfig, &mConfigAVCIntraPeriod, sizeof(OMX_VIDEO_CONFIG_AVCINTRAPERIOD));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }
        case OMX_IndexConfigAndroidIntraRefresh:
        {
            OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE *pConfigAndroidIntraPeriod = (OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE *)pCompConfig;
            if (pConfigAndroidIntraPeriod->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                memcpy(pCompConfig, &mConfigAndroidIntraPeriod, sizeof(OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexConfigAndroidVideoTemporalLayering:
        {
            OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE *pLayerConfig = (OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE *)pCompConfig;

            MTK_OMX_LOGD("there shouldn't have caller from framework");
            if (pLayerConfig->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                //memcpy(pCompConfig, &mConfigAndroidIntraPeriod, sizeof(OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        case OMX_IndexConfigCommonRotate:
        {
            OMX_CONFIG_ROTATIONTYPE *pRotate = (OMX_CONFIG_ROTATIONTYPE *)pCompConfig;

            /*
            typedef struct OMX_CONFIG_ROTATIONTYPE {
            OMX_U32 nSize;
            OMX_VERSIONTYPE nVersion;
            OMX_U32 nPortIndex;
            OMX_S32 nRotation;
            } OMX_CONFIG_ROTATIONTYPE;
            */
            MTK_OMX_LOGD("GetConfig OMX_IndexConfigCommonRotate port index %d\n", pRotate->nPortIndex);
            if (pRotate->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                mRotationType.nPortIndex = pRotate->nPortIndex;
                memcpy(pCompConfig, &mRotationType, sizeof(OMX_CONFIG_ROTATIONTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            break;
        }

        default:
        {
            MTK_OMX_LOGE("MtkOmxVenc::GetConfig unsupported nConfigIndex(0x%08X)", nConfigIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_STRING parameterName,
                                            OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorUnsupportedSetting;
    MTK_OMX_LOGD_ENG("MtkOmxVenc::GetExtensionIndex");

#ifdef ANDROID_ICS
    err = OMX_ErrorNone;

    callReturnIfNoError(mExtensions.GetExtensionIndex, parameterName, pIndexType);

    if (!strncmp(parameterName, "OMX.MTK.index.param.video.EncSetForceIframe",
                 strlen("OMX.MTK.index.param.video.EncSetForceIframe")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVencSetForceIframe;
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.3DVideoEncode",
                      strlen("OMX.MTK.index.param.video.3DVideoEncode")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVenc3DVideoRecode;//for MTK S3D SUPPORT
    }
    // Morris Yang 20120214 add for live effect recording [
    else if (!strncmp(parameterName, "OMX.google.android.index.storeMetaDataInBuffers",
                      strlen("OMX.google.android.index.storeMetaDataInBuffers")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexStoreMetaDataInBuffers;
    }
    // ]
    else if (!strncmp(parameterName, "OMX.MTK.VIDEO.index.useIonBuffer",
                      strlen("OMX.MTK.VIDEO.index.useIonBuffer")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVideoUseIonBuffer;
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.EncSetIFrameRate",
                      strlen("OMX.MTK.index.param.video.EncSetIFrameRate")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVencSetIInterval;
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.EncSetSkipFrame",
                      strlen("OMX.MTK.index.param.video.EncSetSkipFrame")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVencSkipFrame;
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.SetVencScenario",
                      strlen("OMX.MTK.index.param.video.SetVencScenario")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVencSetScenario;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.prependSPSPPSToIDRFrames",
                      strlen("OMX.google.android.index.prependSPSPPSToIDRFrames")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVencPrependSPSPPS;
    }
    else if (!strncmp(parameterName, "OMX.microsoft.skype.index.driverversion",
                      strlen("OMX.microsoft.skype.index.driverversion")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkQueryDriverVersion;
    }
    else if (!strncmp(parameterName, "OMX.microsoft.skype.index.qp",
                      strlen("OMX.microsoft.skype.index.qp")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkConfigQP;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.storeGraphicBufferInMetaData",
                strlen("OMX.google.android.index.storeGraphicBufferInMetaData")))
    {
        mInputPortDef.nBufferCountActual = 4;
        err = OMX_ErrorUnsupportedIndex;
        MTK_OMX_LOGD("try to do storeGraphicBufferInMetaData");
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.EncInputScaling",
                      strlen("OMX.MTK.index.param.video.EncInputScaling")))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxVencInputScaling;
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.SlicelossIndication",
                      strlen("OMX.MTK.index.param.video.SlicelossIndication")))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxSliceLossIndication;
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.EncSetForceFullIframe",
                 strlen("OMX.MTK.index.param.video.EncSetForceFullIframe")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVencSetForceFullIframe;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.describeColorFormat",
        strlen("OMX.google.android.index.describeColorFormat")))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_GoogleAndroidIndexDescribeColorFormat;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.describeColorFormat2",
        strlen("OMX.google.android.index.describeColorFormat2")))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_GoogleAndroidIndexDescribeColorFormat2;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.storeANWBufferInMetadata",
                 strlen("OMX.google.android.index.storeANWBufferInMetadata")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexstoreANWBufferInMetadata;
    }
#ifdef SUPPORT_NATIVE_HANDLE
    else if (!strncmp(parameterName, "OMX.google.android.index.allocateNativeHandle",
                strlen("OMX.google.android.index.allocateNativeHandle")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexEnableAndroidNativeHandle;
    }
#endif
    else
    {
        MTK_OMX_LOGE("MtkOmxVenc::GetExtensionIndex Unknown parameter name: %s", parameterName);
        err = OMX_ErrorUnsupportedIndex;
    }
#endif

    return err;
}

OMX_ERRORTYPE MtkOmxVenc::GetState(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_INOUT OMX_STATETYPE *pState)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (NULL == pState)
    {
        MTK_OMX_LOGE("[ERROR] MtkOmxVenc::GetState pState is NULL !!!");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }
    *pState = mState;

    MTK_OMX_LOGD("MtkOmxVenc::GetState (mState=%s)", StateToString(mState));

EXIT:
    return err;
}

#ifdef SUPPORT_NATIVE_HANDLE
OMX_BOOL MtkOmxVenc::SetDbgInfo2Ion(
    ion_user_handle_t& ion_handle,
    int& ion_share_fd,
    void* va,
    int secure_handle,
    int value1,
    int value2,
    int value3,
    int value4,
    ion_user_handle_t& ion_handle_4_enc,
    int& ion_share_fd_4_enc,
    void* va_4_enc,
    int secure_handle_4_enc,
    int value5,
    int value6,
    int value7,
    int value8
    )
{
    MtkVencIonBufferInfo rIonBufferInfo;
    rIonBufferInfo.ion_handle = ion_handle;
    rIonBufferInfo.ion_share_fd = ion_share_fd;
    rIonBufferInfo.va = va;
    rIonBufferInfo.secure_handle = secure_handle;
    rIonBufferInfo.value[0] = value1;
    rIonBufferInfo.value[1] = value2;
    rIonBufferInfo.value[2] = value3;
    rIonBufferInfo.value[3] = value4;
#ifdef COPY_2_CONTIG
    rIonBufferInfo.ion_handle_4_enc = ion_handle_4_enc;
    rIonBufferInfo.ion_share_fd_4_enc= ion_share_fd_4_enc;
    rIonBufferInfo.va_4_enc = va_4_enc;
    rIonBufferInfo.secure_handle = secure_handle_4_enc;
    rIonBufferInfo.value_4_enc[0] = value5;
    rIonBufferInfo.value_4_enc[1] = value6;
    rIonBufferInfo.value_4_enc[2] = value7;
    rIonBufferInfo.value_4_enc[3] = value8;
    MTK_OMX_LOGD_ENG("[4enc]ion_handle %x, fd %d, va %p", ion_handle_4_enc, ion_share_fd_4_enc, va_4_enc);
#endif
    mIonBufferInfo.push(rIonBufferInfo);
    return OMX_TRUE;
}
#endif

OMX_ERRORTYPE MtkOmxVenc::AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                         OMX_IN OMX_U32 nPortIndex,
                                         OMX_IN OMX_PTR pAppPrivate,
                                         OMX_IN OMX_U32 nSizeBytes)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == mInputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mInputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mInputPortDef.bPopulated)
        {
            MTK_OMX_LOGE("Errorin MtkOmxVenc::AllocateBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        mInputAllocateBuffer = OMX_TRUE;

        *ppBufferHdr = mInputBufferHdrs[mInputBufferPopulatedCnt] =
                           (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));

        if (OMX_TRUE == mStoreMetaDataInBuffers)
        {
            // For meta mode, allocate input from dram,
            // And we don't need to map this VA for MVA.
            (*ppBufferHdr)->pBuffer = (OMX_U8 *)MTK_OMX_MEMALIGN(MEM_ALIGN_32, nSizeBytes);
            memset((*ppBufferHdr)->pBuffer, 0x0, nSizeBytes);
        }
        else
        {
            // If not meta mode, we will allocate VA and map MVA
            mInputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, (int)nSizeBytes,
                                         (void *)*ppBufferHdr, (void **)(&(*ppBufferHdr)->pBuffer));
        }
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INPUT_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_INVALID_PORT;
        //(*ppBufferHdr)->pInputPortPrivate = NULL; // TBD
        *((OMX_U32 *)(*ppBufferHdr)->pBuffer) = kMetadataBufferTypeInvalid;

        MTK_OMX_LOGD_ENG("MtkOmxVenc::AllocateBuffer port_idx(0x%X), idx[%d],"
                     "nSizeBytes %d, pBuffHead(0x%08X), pBuffer(0x%08X)",
                     (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, nSizeBytes,
                     (unsigned int)mInputBufferHdrs[mInputBufferPopulatedCnt], (unsigned int)((*ppBufferHdr)->pBuffer));

        mInputBufferPopulatedCnt++;
        if (mInputBufferPopulatedCnt == mInputPortDef.nBufferCountActual)
        {
            mInputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            MTK_OMX_LOGD("AllocateBuffer:: input port populated");
        }
    }
    else if (nPortIndex == mOutputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mOutputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mOutputPortDef.bPopulated)
        {
            MTK_OMX_LOGE("Errorin MtkOmxVenc::AllocateBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        mOutputAllocateBuffer = OMX_TRUE;

        *ppBufferHdr = mOutputBufferHdrs[mOutputBufferPopulatedCnt] =
                           (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));


        if (OMX_TRUE == mStoreMetaDataInOutBuffers)
        {
            uint32_t offset =9;
            if(mOutputPortDef.nBufferSize >= MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_HEVC_4K)
            {
                offset =11;
            }
            else if((mOutputPortDef.nBufferSize >= MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_VP8_1080P) ||
            (mOutputPortDef.nBufferSize >= MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_AVC_1080P))
            {
                offset = 10;
            }
#ifdef SUPPORT_NATIVE_HANDLE
            if (OMX_FALSE == mIsAllocateOutputNativeBuffers)
            {
                (*ppBufferHdr)->pBuffer = (OMX_U8 *)MTK_OMX_MEMALIGN(MEM_ALIGN_32, nSizeBytes);
                memset((*ppBufferHdr)->pBuffer, 0x0, nSizeBytes);
            }
#endif
#ifdef SUPPORT_NATIVE_HANDLE
            size_t bistreamBufferSize = (1 << offset) * (1 << 9);
            char value[PROPERTY_VALUE_MAX];
            int bsSize = 0;
            property_get("vendor.mtk.omx.venc.bssize", value, "0");
            bsSize = atoi(value);
            if (0 != bsSize)
            {
                MTK_OMX_LOGD("mtk.omx.venc.bssize set buffer size to %d", bsSize);
                bistreamBufferSize = bsSize;
            }
            else
            {
                MTK_OMX_LOGD("no mtk.omx.venc.bssize, buffer size is %d", bistreamBufferSize);
            }
            if (OMX_TRUE == mIsAllocateOutputNativeBuffers)
            {
                native_handle_t* native_handle = NULL;
                ion_user_handle_t ion_handle;
                int ion_share_fd = -1;
                mIonDevFd = ((mIonDevFd < 0) ? eVideoOpenIonDevFd(): mIonDevFd);
                if (OMX_TRUE == mIsSecureSrc)
                {
                    bistreamBufferSize = 1024*1024;
                    MTK_OMX_LOGD("Set bistreamBufferSize to %d for secure buffer", bistreamBufferSize);
                    OMX_U32 flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC | ION_FLAG_MM_HEAP_INIT_ZERO;
                    if (mIonDevFd < 0)
                    {
                        MTK_OMX_LOGE("[ERROR] cannot open ION device. LINE:%d", __LINE__);
                        mIonDevFd = -1;
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                    if (0 != ion_alloc(mIonDevFd, bistreamBufferSize, 1024, ION_HEAP_MULTIMEDIA_SEC_MASK, flags, &ion_handle))
                    {
                        MTK_OMX_LOGE("[ERROR] Failed to ion_alloc (%d) from mIonDevFd(%d)!\n", bistreamBufferSize, mIonDevFd);
                        err = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                    int ret = ion_share( mIonDevFd, ion_handle, &ion_share_fd );
                    if (0 != ret)
                    {
                        MTK_OMX_LOGE("[ERROR] ion_share(ion fd = %d) failed(%d), LINE:%d",mIonDevFd, ret, __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                    struct ion_mm_data mm_data;
                    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
                    mm_data.config_buffer_param.handle = ion_handle;
                    mm_data.config_buffer_param.eModuleID = 1;
                    mm_data.config_buffer_param.security = 1;
                    mm_data.config_buffer_param.coherent = 1;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_MULTIMEDIA, &mm_data))
                    {
                        MTK_OMX_LOGE("IOCTL[ION_IOC_CUSTOM] Config Buffer failed! LINE(%d)", __LINE__);
                    }
                    struct ion_mm_data set_debug_info_mm_data;
                    set_debug_info_mm_data.mm_cmd = ION_MM_SET_DEBUG_INFO;
                    set_debug_info_mm_data.buf_debug_info_param.handle = ion_handle;
                    set_debug_info_mm_data.buf_debug_info_param.value1 = 1;//security
                    set_debug_info_mm_data.buf_debug_info_param.value2 = bistreamBufferSize;//buffersize
                    set_debug_info_mm_data.buf_debug_info_param.value3 = 0;
                    set_debug_info_mm_data.buf_debug_info_param.value4 = 0;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_MULTIMEDIA, &set_debug_info_mm_data)) {
                        MTK_OMX_LOGE("IOCTL[ION_IOC_CUSTOM] Config Buffer failed! LINE(%d)", __LINE__);
                    }
                    struct ion_sys_data sys_data;
                    sys_data.sys_cmd = ION_SYS_GET_PHYS;
                    sys_data.get_phys_param.handle = ion_handle;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_SYSTEM, &sys_data))
                    {
                        MTK_OMX_LOGE("IOCTL[ION_IOC_CUSTOM] Get Phys failed! line(%d)", __LINE__);
                    }
                    if (OMX_FALSE == SetDbgInfo2Ion(
                            ion_handle, ion_share_fd, 0, sys_data.get_phys_param.phy_addr,
                            bistreamBufferSize, ion_share_fd, ion_handle, 0,
                            ion_handle, ion_share_fd, 0, sys_data.get_phys_param.phy_addr,
                            bistreamBufferSize, ion_share_fd, ion_handle, 0))
                    {
                        MTK_OMX_LOGE("IOCTL[ION_IOC_CUSTOM] set buffer debug info!\n");
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                    native_handle = (native_handle_t*)MTK_OMX_ALLOC(sizeof(native_handle_t) + sizeof(int) * 12);
                    native_handle->version  = sizeof(native_handle_t);
                    native_handle->numFds   = 1;
                    native_handle->numInts  = 4;
#ifdef SECURE_BUF_HINT
                    native_handle->data[0]  = (ion_share_fd | (1<<31));  // ION fd for output buffer
                    MTK_OMX_LOGD("[Test] Set data[0] to %x for ion fd %x",native_handle->data[0], ion_share_fd );
#else
                    native_handle->data[0]  = ion_share_fd;  // ION fd for output buffer
#endif
                    native_handle->data[1]  = 0;             // rangeOffset
                    native_handle->data[2]  = 0;             // rangeLength
                    native_handle->data[3]  = 0;
                    native_handle->data[4]  = 1;             // 1: secure buffer; 0: normal buffer
                    native_handle->data[5]  = 0;             // secure PA [63:32]
                    native_handle->data[6]  = 0;             // secure PA [31: 0]
                    native_handle->data[7]  = sys_data.get_phys_param.phy_addr;
                    native_handle->data[8]  = bistreamBufferSize;
                    native_handle->data[9]  = (uint32_t)ion_handle;
                    native_handle->data[10]  = sys_data.get_phys_param.len;
                    (*ppBufferHdr)->pBuffer = (OMX_U8 *)native_handle;
                    MTK_OMX_LOGD("[secure]native_handle %p, ion fd %d, buffer size %d",
                        native_handle, ion_share_fd, bistreamBufferSize);
                    mStoreMetaOutNativeHandle.push(native_handle);
                }
                else
                {
                    uint32_t flags = 0;
#ifdef ALLOC_CONTIG_PHY_ADDR
#ifdef ALLOC_FROM_MM_CONTIG
                    int ret = ion_alloc(mIonDevFd, bistreamBufferSize, 0, ION_HEAP_MULTIMEDIA_CONTIG_MASK, flags, &ion_handle);
                    MTK_OMX_LOGD("[DBG]Allocated from ION_HEAP_MULTIMEDIA_CONTIG");
#else
                    int ret = ion_alloc(mIonDevFd, bistreamBufferSize, 0, ION_HEAP_DMA_RESERVED_MASK, flags, &ion_handle);
                    MTK_OMX_LOGD_ENG("[DBG]Allocated from ION_HEAP_DMA_RESERVED");
#endif
                    if (0 != ret)
                    {
                        MTK_OMX_LOGE("[ERROR] ion_alloc failed (%d), size(%d), LINE:%d",
                            ret, bistreamBufferSize, __LINE__);
                        goto EXIT;
                    }
#else //ALLOC_CONTIG_PHY_ADDR
                    int ret = ion_alloc_mm(mIonDevFd, bistreamBufferSize, 0, flags, &ion_handle);
                    if (0 != ret)
                    {
                        MTK_OMX_LOGE("[ERROR] ion_alloc_mm failed (%d), LINE:%d", ret, __LINE__);
                        goto EXIT;
                    }
#endif
                    if (ion_share(mIonDevFd, ion_handle, &ion_share_fd))
                    {
                        MTK_OMX_LOGE("[ERROR] ion_share failed, LINE:%d", __LINE__);
                        goto EXIT;
                    }
                    void* va = ion_mmap(mIonDevFd, NULL, bistreamBufferSize,
                        PROT_READ | PROT_WRITE, MAP_SHARED, ion_share_fd, 0);
                    if ((0 == va)|| (0xffffffff == (uint32_t)va))
                    {
                        MTK_OMX_LOGE("[ERROR] ion_mmap failed, LINE:%d", __LINE__);
                        close(ion_share_fd);
                        goto EXIT;
                    }
                    MTK_OMX_LOGD("VA %p", va);
#ifdef CHECK_OUTPUT_CONSISTENCY
                    MTK_OMX_LOGD("Initialize VA %p len %d to 0", va, bistreamBufferSize);
                    memset(va, 0, bistreamBufferSize);
#endif
#ifdef ALLOC_CONTIG_PHY_ADDR
#else
                    {
                        struct ion_mm_data mm_data;
                        mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
                        mm_data.config_buffer_param.handle = ion_handle;
                        mm_data.config_buffer_param.eModuleID = eVideoGetM4UModuleID(VAL_MEM_CODEC_FOR_VENC);
                        mm_data.config_buffer_param.security = 0;
                        mm_data.config_buffer_param.coherent = 1;
                        if (ion_custom_ioctl(mIonDevFd, ION_CMD_MULTIMEDIA, &mm_data))
                        {
                            MTK_OMX_LOGE("[ERROR]IOCTL[ION_IOC_CUSTOM] Config Buffer failed!\n");
                        }
                        struct ion_mm_data set_debug_info_mm_data;
                        set_debug_info_mm_data.mm_cmd = ION_MM_SET_DEBUG_INFO;
                        set_debug_info_mm_data.buf_debug_info_param.handle = ion_handle;
                        set_debug_info_mm_data.buf_debug_info_param.value1 = 0;//1: secure buffer; 0: normal buffer
                        set_debug_info_mm_data.buf_debug_info_param.value2 = bistreamBufferSize;//buffersize
                        set_debug_info_mm_data.buf_debug_info_param.value3 = 0;
                        set_debug_info_mm_data.buf_debug_info_param.value4 = 0;
                        if (ion_custom_ioctl(mIonDevFd, ION_CMD_MULTIMEDIA, &set_debug_info_mm_data)) {
                            MTK_OMX_LOGE("IOCTL[ION_IOC_CUSTOM] Config Buffer failed! LINE(%d)", __LINE__);
                        }
                    }
#endif
                    unsigned int mva = 0;
                    struct ion_sys_data sys_data;
#ifdef ALLOC_CONTIG_PHY_ADDR
                    sys_data.sys_cmd = ION_SYS_GET_IOVA;
#else
                    sys_data.sys_cmd = ION_SYS_GET_PHYS;
#endif
                    sys_data.get_phys_param.handle = ion_handle;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_SYSTEM, &sys_data))
                    {
                        MTK_OMX_LOGE("[ERROR] cannot get buffer physical address");
                        goto EXIT;
                    }
                    mva = sys_data.get_phys_param.phy_addr;
                    MTK_OMX_LOGD_ENG("MVA 0x%x", mva);
#ifdef COPY_2_CONTIG
                    sys_data.sys_cmd = ION_SYS_GET_PHYS;
                    sys_data.get_phys_param.handle = ion_handle;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_SYSTEM, &sys_data))
                    {
                        MTK_OMX_LOGE("[ERROR] cannot get buffer physical address");
                        goto EXIT;
                    }
                    MTK_OMX_LOGD_ENG("PA 0x%x", sys_data.get_phys_param.phy_addr);
#endif
#ifdef COPY_2_CONTIG
                    ion_user_handle_t ion_handle_4_enc;
                    ret = ion_alloc_mm(mIonDevFd, bistreamBufferSize, 0, flags, &ion_handle_4_enc);
                    if (0 != ret)
                    {
                        MTK_OMX_LOGE("[ERROR] ion_alloc_mm failed (%d), LINE:%d", ret, __LINE__);
                        goto EXIT;
                    }
                    int ion_share_fd_4_enc = -1;
                    if (ion_share(mIonDevFd, ion_handle_4_enc, &ion_share_fd_4_enc))
                    {
                        MTK_OMX_LOGE("[ERROR] ion_share failed, LINE:%d", __LINE__);
                        goto EXIT;
                    }
                    void* va_4_enc = ion_mmap(mIonDevFd, NULL, bistreamBufferSize,
                        PROT_READ | PROT_WRITE, MAP_SHARED, ion_share_fd_4_enc, 0);
                    if ((0 == va_4_enc)|| (0xffffffff == (uint32_t)va_4_enc))
                    {
                        MTK_OMX_LOGE("[ERROR] ion_mmap failed, LINE:%d", __LINE__);
                        close(ion_share_fd);
                        goto EXIT;
                    }
                    MTK_OMX_LOGD("va_4_enc %p", va_4_enc);
                    struct ion_mm_data mm_data;
                    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
                    mm_data.config_buffer_param.handle = ion_handle_4_enc;
                    mm_data.config_buffer_param.eModuleID = eVideoGetM4UModuleID(VAL_MEM_CODEC_FOR_VENC);
                    mm_data.config_buffer_param.security = 0;
                    mm_data.config_buffer_param.coherent = 1;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_MULTIMEDIA, &mm_data))
                    {
                        MTK_OMX_LOGE("[ERROR]IOCTL[ION_IOC_CUSTOM] Config Buffer failed!\n");
                    }
                    unsigned int mva_4_enc = 0;
                    sys_data.sys_cmd = ION_SYS_GET_PHYS;
                    sys_data.get_phys_param.handle = ion_handle_4_enc;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_SYSTEM, &sys_data))
                    {
                        MTK_OMX_LOGE("[ERROR] cannot get buffer physical address");
                        goto EXIT;
                    }
                    mva_4_enc = sys_data.get_phys_param.phy_addr;
                    MTK_OMX_LOGD("MVA_4_enc 0x%x", mva_4_enc);
#else
#ifdef CHECK_OUTPUT_CONSISTENCY
                    ion_user_handle_t ion_handle_4_enc;
                    ret = ion_alloc_mm(mIonDevFd, bistreamBufferSize, 0, flags, &ion_handle_4_enc);
                    if (0 != ret)
                    {
                        MTK_OMX_LOGE("[ERROR] ion_alloc_mm failed (%d), LINE:%d", ret, __LINE__);
                        goto EXIT;
                    }
                    int ion_share_fd_4_enc = -1;
                    if (ion_share(mIonDevFd, ion_handle_4_enc, &ion_share_fd_4_enc))
                    {
                        MTK_OMX_LOGE("[ERROR] ion_share failed, LINE:%d", __LINE__);
                        goto EXIT;
                    }
                    void* va_4_enc = ion_mmap(mIonDevFd, NULL, bistreamBufferSize,
                        PROT_READ | PROT_WRITE, MAP_SHARED, ion_share_fd_4_enc, 0);
                    if ((0 == va)|| (0xffffffff == (uint32_t)va))
                    {
                        MTK_OMX_LOGE("[ERROR] ion_mmap failed, LINE:%d", __LINE__);
                        close(ion_share_fd);
                        goto EXIT;
                    }
                    MTK_OMX_LOGD("va_4_enc %p", va_4_enc);
                    memset(va_4_enc, 0, bistreamBufferSize);
                    struct ion_mm_data mm_data;
                    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
                    mm_data.config_buffer_param.handle = ion_handle_4_enc;
                    mm_data.config_buffer_param.eModuleID = eVideoGetM4UModuleID(VAL_MEM_CODEC_FOR_VENC);
                    mm_data.config_buffer_param.security = 0;
                    mm_data.config_buffer_param.coherent = 1;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_MULTIMEDIA, &mm_data))
                    {
                        MTK_OMX_LOGE("[ERROR]IOCTL[ION_IOC_CUSTOM] Config Buffer failed!\n");
                    }
                    unsigned int mva_4_enc = 0;
                    sys_data.sys_cmd = ION_SYS_GET_PHYS;
                    sys_data.get_phys_param.handle = ion_handle_4_enc;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_SYSTEM, &sys_data))
                    {
                        MTK_OMX_LOGE("[ERROR] cannot get buffer physical address");
                        goto EXIT;
                    }
                    mva_4_enc = sys_data.get_phys_param.phy_addr;
                    MTK_OMX_LOGD("MVA_4_enc 0x%x", mva_4_enc);
#else
                    ion_user_handle_t ion_handle_4_enc;
                    int ion_share_fd_4_enc = -1;
                    void* va_4_enc = NULL;
                    unsigned int mva_4_enc = 0;
#endif
#endif
                    if (OMX_FALSE == SetDbgInfo2Ion(
                            ion_handle, ion_share_fd, va, 0,
                            bistreamBufferSize, ion_share_fd, (uint32_t)va, mva,
                            ion_handle_4_enc, ion_share_fd_4_enc, va_4_enc, 0,
                            bistreamBufferSize, ion_share_fd_4_enc, (uint32_t)va_4_enc, mva_4_enc))
                    {
                        MTK_OMX_LOGE("IOCTL[ION_IOC_CUSTOM] set buffer debug info!\n");
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                    native_handle = (native_handle_t*)MTK_OMX_ALLOC(sizeof(native_handle_t) + sizeof(int) * 12);
                    MTK_OMX_LOGD_ENG("Allocated %d bytes for a native handle", sizeof(native_handle_t) + sizeof(int) * 9);
                    native_handle->version  = sizeof(native_handle_t);
                    native_handle->numFds   = 1;
                    native_handle->numInts  = 4;
                    native_handle->data[0]  = ion_share_fd;  // ION fd for output buffer
                    native_handle->data[1]  = bistreamBufferSize;             // rangeOffset
                    native_handle->data[2]  = 0;             // rangeLength
                    native_handle->data[3]  = (uint32_t)va;
                    native_handle->data[4]  = 0;             // 1: secure buffer; 0: normal buffer
                    native_handle->data[5]  = 0;             // secure PA [63:32]
                    native_handle->data[6]  = 0;             // secure PA [31: 0]
                    native_handle->data[7]  = (uint32_t)va_4_enc;
                    native_handle->data[8]  = bistreamBufferSize;
                    native_handle->data[9]  = (uint32_t)ion_handle;
                    native_handle->data[10]  = (uint32_t)ion_handle_4_enc;
                    native_handle->data[11]  = ion_share_fd_4_enc;
                    (*ppBufferHdr)->pBuffer = (OMX_U8 *)native_handle;
                    MTK_OMX_LOGD("[normal]native_handle %p, va %p, ion fd %d, buffer size %d",
                        native_handle, va, ion_share_fd, bistreamBufferSize);
                    mStoreMetaOutNativeHandle.push(native_handle);
                }
            }
            else
#endif
            {
            (*ppBufferHdr)->pBuffer = (OMX_U8 *)MTK_OMX_MEMALIGN(MEM_ALIGN_32, nSizeBytes);
            memset((*ppBufferHdr)->pBuffer, 0x0, nSizeBytes);
            //When output buffers are meta mode, the handles are allocated in venc
            if (mIsSecureSrc)
            {
                uint32_t flags = GRALLOC_USAGE_HW_VIDEO_ENCODER | GRALLOC_USAGE_SECURE  | GRALLOC_USAGE_SW_READ_OFTEN;
                //allocate secure buffer by GraphicBuffer
                mStoreMetaOutHandle.push(
                    new GraphicBuffer(
                        1 << offset,
                        1 << 9,
                        PIXEL_FORMAT_RGBA_8888,
                        flags)
                );
                if (NULL == mStoreMetaOutHandle.top()->handle)
                {
                    err = OMX_ErrorInsufficientResources;
                }
                //Don't map MVA when secure buffer, we will query for each frame.
            }
            else
            {
                //allocate normal buffer by GraphicBuffer,
                uint32_t flags = GRALLOC_USAGE_HW_VIDEO_ENCODER | GRALLOC_USAGE_SW_READ_OFTEN;
                mStoreMetaOutHandle.push(
                    new GraphicBuffer(
                        1 << offset,
                        1 << 9,
                        PIXEL_FORMAT_RGBA_8888,
                        flags)
                );

                //int ionfd, size;
                buffer_handle_t _handle = (buffer_handle_t)mStoreMetaOutHandle.top()->handle;
                mOutputMVAMgr->newOmxMVAwithHndl((void *)_handle, (void *)(*ppBufferHdr));
            }
            //assign handle to real buffer
            struct VideoGrallocMetadata *metadata = (struct VideoGrallocMetadata *)(*ppBufferHdr)->pBuffer;
            metadata->eType = kMetadataBufferTypeGrallocSource;
            metadata->pHandle = mStoreMetaOutHandle.top()->handle;
            //struct VideoNativeHandleMetadata *metadata = (struct VideoNativeHandleMetadata*)(*ppBufferHdr)->pBuffer;
            //metadata->eType = kMetadataBufferTypeNativeHandleSource;
            //metadata->pHandle = (native_handle_t*)mStoreMetaOutHandle.top()->handle;
            int *bitstreamLen = (int*)((*ppBufferHdr)->pBuffer + sizeof(struct VideoNativeHandleMetadata));
            *bitstreamLen = 0;

            MTK_OMX_LOGD_ENG("GB(0x%08x), GB->handle(0x%08x), meta type:%d, bitstreamLen:0x%x",
                         (unsigned int)mStoreMetaOutHandle.top().get(),
                         (unsigned int)mStoreMetaOutHandle.top()->handle,
                         metadata->eType, (unsigned int)bitstreamLen);
        }
        }
        else
        {
            // If not meta mode, we will allocate VA and map MVA.
            mOutputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, (int)nSizeBytes,
                                          (void *)*ppBufferHdr, (void **)(&(*ppBufferHdr)->pBuffer));
        }
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INVALID_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_OUTPUT_PORT;
        //(*ppBufferHdr)->pOutputPortPrivate = NULL; // TBD

        MTK_OMX_LOGD_ENG("MtkOmxVenc::AllocateBuffer port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X), nAllocLen(%d)",
                     (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt,
                     (unsigned int)mOutputBufferHdrs[mOutputBufferPopulatedCnt],
                     (unsigned int)((*ppBufferHdr)->pBuffer),
                     nSizeBytes);

        if (err == OMX_ErrorNone)//no error
        {
            mOutputBufferPopulatedCnt++;
            if (mOutputBufferPopulatedCnt == mOutputPortDef.nBufferCountActual)
            {
                mOutputPortDef.bPopulated = OMX_TRUE;

                if (IS_PENDING(MTK_OMX_IDLE_PENDING))
                {
                    SIGNAL(mOutPortAllocDoneSem);
                    MTK_OMX_LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
                }

                if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
                {
                    SIGNAL(mOutPortAllocDoneSem);
                    MTK_OMX_LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
                }

                MTK_OMX_LOGD("AllocateBuffer:: output port populated");
            }
        }
    }
    else
    {
        err = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                    OMX_IN OMX_U32 nPortIndex,
                                    OMX_IN OMX_PTR pAppPrivate,
                                    OMX_IN OMX_U32 nSizeBytes,
                                    OMX_IN OMX_U8 *pBuffer)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == mInputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mInputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mInputPortDef.bPopulated)
        {
            MTK_OMX_LOGE("Errorin MtkOmxVenc::UseBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        *ppBufferHdr = mInputBufferHdrs[mInputBufferPopulatedCnt] =
                           (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));
        (*ppBufferHdr)->pBuffer = pBuffer;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INPUT_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_INVALID_PORT;
        //(*ppBufferHdr)->pInputPortPrivate = NULL; // TBD

        if (OMX_FALSE == mStoreMetaDataInBuffers)
        {
            // If meta mode, we will map handle & ion MVA dynamically at GetVEncDrvFrmBuffer()
            // If not meta mode,
            // UseBuffer doesn't support mapping MVA by ION.
            // This should be M4U :
            if (strncmp("m4u", mInputMVAMgr->getType(), strlen("m4u")))
            {
                //if not m4u map
                delete mInputMVAMgr;
                mInputMVAMgr = new OmxMVAManager("m4u");
            }
            mInputMVAMgr->newOmxMVAwithVA((void *)pBuffer, (int)nSizeBytes, (void *)(*ppBufferHdr));
        }

        MTK_OMX_LOGD("MtkOmxVenc::UseBuffer port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X), mapType:%s",
                     (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt,
                     (unsigned int)mInputBufferHdrs[mInputBufferPopulatedCnt],
                     (unsigned int)pBuffer, mInputMVAMgr->getType());

        mInputBufferPopulatedCnt++;
        if (mInputBufferPopulatedCnt == mInputPortDef.nBufferCountActual)
        {
            mInputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            MTK_OMX_LOGD("input port populated");
        }
    }
    else if (nPortIndex == mOutputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mOutputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mOutputPortDef.bPopulated)
        {
            MTK_OMX_LOGE("Errorin MtkOmxVenc::UseBuffer, output port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        *ppBufferHdr = mOutputBufferHdrs[mOutputBufferPopulatedCnt] =
                           (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));
        (*ppBufferHdr)->pBuffer = pBuffer;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INVALID_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_OUTPUT_PORT;
        //(*ppBufferHdr)->pOutputPortPrivate = NULL; // TBD

        if (OMX_FALSE == mStoreMetaDataInOutBuffers)
        {
            // If not meta mode,
            // UseBuffer doesn't support mapping MVA by ION.
            // This should be M4U :
            if (strncmp("m4u", mOutputMVAMgr->getType(), strlen("m4u")))
            {
                //if not m4u map
                delete mOutputMVAMgr;
                mOutputMVAMgr = new OmxMVAManager("m4u");
            }
            mOutputMVAMgr->newOmxMVAwithVA((void *)pBuffer, (int)nSizeBytes, (void *)(*ppBufferHdr));
        }

        MTK_OMX_LOGD("MtkOmxVenc::UseBuffer port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X), mapType:%s",
                     (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt,
                     (unsigned int)mOutputBufferHdrs[mOutputBufferPopulatedCnt],
                     (unsigned int)pBuffer, mOutputMVAMgr->getType());

        mOutputBufferPopulatedCnt++;
        if (mOutputBufferPopulatedCnt == mOutputPortDef.nBufferCountActual)
        {
            mOutputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                MTK_OMX_LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                MTK_OMX_LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            MTK_OMX_LOGD("output port populated");
        }
    }
    else
    {
        err = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::FreeBuffer nPortIndex(%d)", nPortIndex);
    OMX_BOOL bAllowFreeBuffer = OMX_FALSE;

    if (NULL == pBuffHead)
    {
        MTK_OMX_LOGE("pBuffHead is empty!");
        return OMX_ErrorBadParameter;
    }

    //MTK_OMX_LOGD ("@@ mState=%d, Is LOADED PENDING(%d)", mState, IS_PENDING (MTK_OMX_VENC_LOADED_PENDING));
    if (mState == OMX_StateExecuting || mState == OMX_StateIdle || mState == OMX_StatePause)
    {
        if (((nPortIndex == MTK_OMX_INPUT_PORT) && (mInputPortDef.bEnabled == OMX_FALSE)) ||
            ((nPortIndex == MTK_OMX_OUTPUT_PORT) && (mOutputPortDef.bEnabled == OMX_FALSE)))
            // in port disabled case, p.99
        {
            bAllowFreeBuffer = OMX_TRUE;
        }
        else if ((mState == OMX_StateIdle) && (IS_PENDING(MTK_OMX_LOADED_PENDING)))
            // de-initialization, p.128
        {
            bAllowFreeBuffer = OMX_TRUE;
        }
        else
        {
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorPortUnpopulated,
                                   (OMX_U32)NULL,
                                   NULL);
            err = OMX_ErrorPortUnpopulated;
            goto EXIT;
        }
    }

    if ((nPortIndex == MTK_OMX_INPUT_PORT) && bAllowFreeBuffer)
    {

        if (OMX_FALSE == mStoreMetaDataInBuffers)
        {
            mInputMVAMgr->freeOmxMVAByVa((void *)pBuffHead->pBuffer);
        }
        else//if meta mode
        {
            OMX_U32 _handle = 0;
            GetMetaHandleFromOmxHeader(pBuffHead, &_handle);
            mInputMVAMgr->freeOmxMVAByHndl((void *)_handle);
        }

        // free input buffers
        for (OMX_U32 i = 0 ; i < mInputPortDef.nBufferCountActual ; i++)
        {
            if (pBuffHead == mInputBufferHdrs[i])
            {
                if (pBuffHead != NULL)
                {
                    MTK_OMX_LOGD_ENG("MtkOmxVenc::FreeBuffer input hdr (0x%08X), buf (0x%08X)",
                            (unsigned int)pBuffHead, (unsigned int)pBuffHead->pBuffer);
                }
                // do this only when input is meta mode, and allocateBuffer
                if ((mStoreMetaDataInBuffers == OMX_TRUE) && (mInputAllocateBuffer == OMX_TRUE) &&
                    (pBuffHead->pBuffer != NULL))
                {
                    MTK_OMX_FREE(pBuffHead->pBuffer);
                }
                MTK_OMX_FREE(mInputBufferHdrs[i]);
                mInputBufferHdrs[i] = NULL;
                mInputBufferPopulatedCnt--;
            }
        }

        if (mInputBufferPopulatedCnt == 0)       // all input buffers have been freed
        {
            mInputPortDef.bPopulated = OMX_FALSE;
            SIGNAL(mInPortFreeDoneSem);
            MTK_OMX_LOGD("MtkOmxVenc::FreeBuffer all input buffers have been freed!!! signal mInPortFreeDoneSem(%d)",
                         get_sem_value(&mInPortFreeDoneSem));
        }
    }

    if ((nPortIndex == MTK_OMX_OUTPUT_PORT) && bAllowFreeBuffer)
    {
        if (OMX_TRUE == mStoreMetaDataInOutBuffers)
        {
#ifdef SUPPORT_NATIVE_HANDLE
            if (OMX_TRUE == mIsAllocateOutputNativeBuffers)
            {
                native_handle* pHandle = (native_handle*)(pBuffHead->pBuffer + pBuffHead->nOffset);
                OMX_BOOL foundNativeHandle = OMX_FALSE;
                for (int i = 0; i < mStoreMetaOutNativeHandle.size(); ++i)
                {
                    if (pHandle == mStoreMetaOutNativeHandle[i])
                    {
                        foundNativeHandle = OMX_TRUE;
                        OMX_BOOL foundIONInfo = OMX_FALSE;
                        for (int i = 0; i < mIonBufferInfo.size(); ++i)
                        {
                            if (pHandle->data[0] == mIonBufferInfo[i].ion_share_fd)
                            {
                                foundIONInfo = OMX_TRUE;
                                if (0 == mIonBufferInfo[i].secure_handle)
                                {
                                    if (NULL != mIonBufferInfo[i].va)
                                    {
                                        MTK_OMX_LOGD_ENG("ion_munmap %p w/ size %d",
                                            mIonBufferInfo[i].va,
                                            mIonBufferInfo[i].value[0]);
                                        ion_munmap(mIonDevFd, mIonBufferInfo[i].va, mIonBufferInfo[i].value[0]);
#ifdef COPY_2_CONTIG
                                        MTK_OMX_LOGD_ENG("ion_munmap %p w/ size %d",
                                            mIonBufferInfo[i].va_4_enc,
                                            mIonBufferInfo[i].value[0]);
                                        ion_munmap(mIonDevFd, mIonBufferInfo[i].va_4_enc, mIonBufferInfo[i].value[0]);
#endif
                                    }
                                }
                                ion_share_close(mIonDevFd, mIonBufferInfo[i].ion_share_fd);
                                if (ion_free(mIonDevFd, mIonBufferInfo[i].ion_handle))
                                {
                                    MTK_OMX_LOGE("[ERROR] ion_free %d failed", mIonBufferInfo[i].ion_handle);
                                }
#ifdef COPY_2_CONTIG
                                ion_share_close(mIonDevFd, mIonBufferInfo[i].ion_share_fd_4_enc);
                                if (ion_free(mIonDevFd, mIonBufferInfo[i].ion_handle_4_enc))
                                {
                                    MTK_OMX_LOGE("[ERROR] ion_free %d failed", mIonBufferInfo[i].ion_handle_4_enc);
                                }
#endif
                                mIonBufferInfo.removeAt(i);
                                break;
                            }
                        }
                        if (OMX_TRUE == foundIONInfo)
                        {
                            MTK_OMX_LOGD_ENG("[Output][FreeBuffer] Buffer Header = %p, handle=%p, c:%d",
                                         pBuffHead, pHandle, mStoreMetaOutNativeHandle.size());
                            MTK_OMX_FREE(mStoreMetaOutNativeHandle[i]);
                            mStoreMetaOutNativeHandle.removeAt(i);
                        }
                        else
                        {
                            MTK_OMX_LOGE("[Output][FreeBuffer][ERR] No matched ION info for ion fd %d",
                                pHandle->data[0]);
                            for (int i = 0; i < mIonBufferInfo.size(); ++i)
                            {
                                MTK_OMX_LOGE("mIonBufferInfo[%d].ion_share_fd: %d", i, mIonBufferInfo[i].ion_share_fd);
                            }
                        }
                        break;
                    }
                }
                if (OMX_FALSE == foundNativeHandle)
                {
                    MTK_OMX_LOGE("[Output][FreeBuffer][ERR] No matched native handle for %p", pHandle);
                    for (int i = 0; i < mStoreMetaOutNativeHandle.size(); ++i)
                    {
                        MTK_OMX_LOGE("mStoreMetaOutNativeHandle[%d]: %d", i, mStoreMetaOutNativeHandle[i]);
                    }
                }
            }
            else
#endif
            {
            VAL_UINT32_T u4I;
            OMX_U32 _handle = 0;
            GetMetaHandleFromOmxHeader(pBuffHead, &_handle);
            // When output meta mode, the handles are created by venc, release handles here.
            for (u4I = 0; u4I < mStoreMetaOutHandle.size(); ++u4I)
            {
                if ((void *)mStoreMetaOutHandle[u4I]->handle == (void *)_handle)
                {
                    //unmap MVA here
                    mOutputMVAMgr->freeOmxMVAByHndl((void *)_handle);
                    MTK_OMX_LOGD("[Output][FreeBuffer] Buffer Header = 0x%u, handle=%u, c:%d",
                                 (unsigned int)pBuffHead,
                                 (unsigned int)mStoreMetaOutHandle[u4I]->handle,
                                 mStoreMetaOutHandle.size());
                    mStoreMetaOutHandle.removeAt(u4I);
                    break;
                    }
                }
            }
        }
        else
        {
            mOutputMVAMgr->freeOmxMVAByVa((void *)pBuffHead->pBuffer);
        }

        // free output buffers
        for (OMX_U32 i = 0 ; i < mOutputPortDef.nBufferCountActual ; i++)
        {
            if (pBuffHead == mOutputBufferHdrs[i])
            {
                MTK_OMX_LOGD_ENG("MtkOmxVenc::FreeBuffer output hdr (0x%08X), buf (0x%08X)",
                             (unsigned int)pBuffHead, (unsigned int)pBuffHead->pBuffer);
                // do this only when output is meta mode, and allocateBuffer
                if ((mStoreMetaDataInOutBuffers == OMX_TRUE) && (mOutputAllocateBuffer == OMX_TRUE) &&
                    (pBuffHead->pBuffer != NULL))
                {
#ifdef SUPPORT_NATIVE_HANDLE
                    if (OMX_FALSE == mIsAllocateOutputNativeBuffers)
                    {
                    MTK_OMX_FREE(pBuffHead->pBuffer);
                    }
#endif
                }
                MTK_OMX_FREE(mOutputBufferHdrs[i]);
                mOutputBufferHdrs[i] = NULL;
                mOutputBufferPopulatedCnt--;
            }
        }

        if (mOutputBufferPopulatedCnt == 0)      // all output buffers have been freed
        {
            mOutputPortDef.bPopulated = OMX_FALSE;
            SIGNAL(mOutPortFreeDoneSem);
            MTK_OMX_LOGD("MtkOmxVenc::FreeBuffer all output buffers have been freed!!! signal mOutPortFreeDoneSem(%d)",
                         get_sem_value(&mOutPortFreeDoneSem));
        }
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if (mETBDebug == true)
    {
        MTK_OMX_LOGD("MtkOmxVenc::EmptyThisBuffer pBuffHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)",
                     pBuffHead, pBuffHead->pBuffer, pBuffHead->nFilledLen);
    }
    int ret;
    OMX_U32 CmdCat = MTK_OMX_BUFFER_COMMAND;
    OMX_U32 buffer_type = MTK_OMX_EMPTY_THIS_BUFFER_TYPE;
    // write 8 bytes to mEmptyBufferPipe  [buffer_type][pBuffHead]
    LOCK(mCmdQLock);
    WRITE_PIPE(CmdCat, mCmdPipe);
    WRITE_PIPE(buffer_type, mCmdPipe);
    WRITE_PIPE(pBuffHead, mCmdPipe);
    UNLOCK(mCmdQLock);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::FillThisBuffer pBuffHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)",
    //pBuffHead, pBuffHead->pBuffer, pBuffHead->nAllocLen);
    int ret;
    OMX_U32 CmdCat = MTK_OMX_BUFFER_COMMAND;
    OMX_U32 buffer_type = MTK_OMX_FILL_THIS_BUFFER_TYPE;
    // write 8 bytes to mFillBufferPipe  [bufId][pBuffHead]
    LOCK(mCmdQLock);
    WRITE_PIPE(CmdCat, mCmdPipe);
    WRITE_PIPE(buffer_type, mCmdPipe);
    WRITE_PIPE(pBuffHead, mCmdPipe);
    UNLOCK(mCmdQLock);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_OUT OMX_U8 *cRole,
                                            OMX_IN OMX_U32 nIndex)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if ((0 == nIndex) && (NULL != cRole))
    {
        // Unused callback. enum set to 0
        *cRole = 0;
        MTK_OMX_LOGD("MtkOmxVenc::ComponentRoleEnum: Role[%s]", cRole);
    }
    else
    {
        err = OMX_ErrorNoMore;
    }

    return err;
}

OMX_BOOL MtkOmxVenc::PortBuffersPopulated()
{
    if ((OMX_TRUE == mInputPortDef.bPopulated) && (OMX_TRUE == mOutputPortDef.bPopulated))
    {
        return OMX_TRUE;
    }
    else
    {
        return OMX_FALSE;
    }
}


OMX_ERRORTYPE MtkOmxVenc::HandleStateSet(OMX_U32 nNewState)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //    MTK_OMX_LOGD ("MtkOmxVenc::HandleStateSet");
    switch (nNewState)
    {
        case OMX_StateIdle:
            if ((mState == OMX_StateLoaded) || (mState == OMX_StateWaitForResources))
            {
                MTK_OMX_LOGD("Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                // wait until input/output buffers allocated
                MTK_OMX_LOGD("wait on mInPortAllocDoneSem(%d), mOutPortAllocDoneSem(%d)!!",
                             get_sem_value(&mInPortAllocDoneSem), get_sem_value(&mOutPortAllocDoneSem));
                WAIT(mInPortAllocDoneSem);
                WAIT(mOutPortAllocDoneSem);

                if ((OMX_TRUE == mInputPortDef.bEnabled) && (OMX_TRUE == mOutputPortDef.bEnabled) &&
                    (OMX_TRUE == PortBuffersPopulated()))
                {
                    mState = OMX_StateIdle;
                    CLEAR_PENDING(MTK_OMX_IDLE_PENDING);
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                           mAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandStateSet,
                                           mState,
                                           NULL);
                }
            }
            else if ((mState == OMX_StateExecuting) || (mState == OMX_StatePause))
            {
                MTK_OMX_LOGD("Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                // flush all ports
                // Bruce 20130709 [
                if (mDoConvertPipeline)
                {
                    LOCK(mConvertLock);
                }
                // ]
                LOCK(mEncodeLock);

                if (mPartNum != 0)
                {
                    MTK_OMX_LOGD("Wait Partial Frame Done! now (%d)", mPartNum);
                    mWaitPart = 1;
                    WAIT_COND(mPartCond, mEncodeLock);
                    mWaitPart = 0;
                }
                // Morris Yang 20111223 handle EOS [
                // Morris Yang 20120801 [
                if (mLastBsBufHdr != NULL)
                {
                    mLastBsBufHdr->nFilledLen = 0;
                    mLastBsBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                    HandleFillBufferDone(mLastBsBufHdr);
                    MTK_OMX_LOGD("@@ EOS 2-1");
                }
                if (mLastFrameBufHdr != NULL)
                {
                    HandleEmptyBufferDone(mLastFrameBufHdr);
                    mLastFrameBufHdr = NULL;
                    MTK_OMX_LOGD("@@ EOS 2-2");
                }
                // ]
                if (mDoConvertPipeline)
                {
                    ReturnPendingInternalBuffers();
                }
                FlushInputPort();
                FlushOutputPort();
                UNLOCK(mEncodeLock);
                // Bruce 20130709 [
                if (mDoConvertPipeline)
                {
                    UNLOCK(mConvertLock);
                }
                // ]

                // de-initialize decoder
                DeInitVideoEncodeHW();

                mState = OMX_StateIdle;
                CLEAR_PENDING(MTK_OMX_IDLE_PENDING);
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
            else if (mState == OMX_StateIdle)
            {
                MTK_OMX_LOGD("Request [%s]-> [OMX_StateIdle]", StateToString(mState));
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       (OMX_U32)NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       (OMX_U32)NULL,
                                       NULL);
            }

            break;

        case OMX_StateExecuting:
            MTK_OMX_LOGD("Request [%s]-> [OMX_StateExecuting]", StateToString(mState));
            if (mState == OMX_StateIdle || mState == OMX_StatePause)
            {
                // Bruce 20130709 [
                TryTurnOnMDPPipeline();
                if (mDoConvertPipeline)
                {
                    InitPipelineBuffer();
                }
                // ]

                // change state to executing
                mState = OMX_StateExecuting;

                // trigger encode start
                mEncodeStarted = OMX_TRUE;

                // send event complete to IL client
                MTK_OMX_LOGD("state changes to OMX_StateExecuting");
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
            else if (mState == OMX_StateExecuting)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       (OMX_U32)NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       (OMX_U32)NULL,
                                       NULL);
            }
            break;

        case OMX_StatePause:
            MTK_OMX_LOGD("Request [%s]-> [OMX_StatePause]", StateToString(mState));
            if (mState == OMX_StateIdle || mState == OMX_StateExecuting)
            {
                // TODO: ok
            }
            else if (mState == OMX_StatePause)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       (OMX_U32)NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       (OMX_U32)NULL,
                                       NULL);
            }
            break;

        case OMX_StateLoaded:
            MTK_OMX_LOGD("Request [%s]-> [OMX_StateLoaded]", StateToString(mState));
            if (mState == OMX_StateIdle)    // IDLE  to LOADED
            {
                if (IS_PENDING(MTK_OMX_LOADED_PENDING))
                {

                    // wait until all input buffers are freed
                    MTK_OMX_LOGD("wait on mInPortFreeDoneSem(%d), mOutPortFreeDoneSem(%d)",
                                 get_sem_value(&mInPortFreeDoneSem), get_sem_value(&mOutPortFreeDoneSem));
                    WAIT(mInPortFreeDoneSem);

                    // wait until all output buffers are freed
                    WAIT(mOutPortFreeDoneSem);

                    mState = OMX_StateLoaded;
                    CLEAR_PENDING(MTK_OMX_LOADED_PENDING);
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                           mAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandStateSet,
                                           mState,
                                           NULL);
                }
            }
            else if (mState == OMX_StateLoaded)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       (OMX_U32)NULL,
                                       NULL);
            }

        default:
            break;
    }
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::HandlePortEnable(OMX_U32 nPortIndex)
{
    MTK_OMX_LOGD("MtkOmxVenc::HandlePortEnable nPortIndex(0x%X)", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            // p.86 component is not in LOADED state and the port is not populated
        {
            MTK_OMX_LOGD("Wait on mInPortAllocDoneSem(%d)", get_sem_value(&mInPortAllocDoneSem));
            WAIT(mInPortAllocDoneSem);
            CLEAR_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
        }

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortEnable,
                               MTK_OMX_INPUT_PORT,
                               NULL);
    }

    if (nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
        {
            MTK_OMX_LOGD("Wait on mOutPortAllocDoneSem(%d)", get_sem_value(&mOutPortAllocDoneSem));
            WAIT(mOutPortAllocDoneSem);
            CLEAR_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);

            if (mState == OMX_StateExecuting && mPortReconfigInProgress == OMX_TRUE)
            {
                mPortReconfigInProgress = OMX_FALSE;
            }
        }

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortEnable,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }

EXIT:
    return err;
}


OMX_BOOL MtkOmxVenc::CheckBufferAvailability()
{
    if (mEmptyThisBufQ.IsEmpty() || mFillThisBufQ.IsEmpty())
    {
        return OMX_FALSE;
    }
    else
    {
        return OMX_TRUE;
    }
}

void MtkOmxVenc::QueueOutputBuffer(int index)
{
    LOCK(mFillThisBufQ.mBufQLock);

    MTK_OMX_LOGD("@@ QueueOutputBuffer");

    mFillThisBufQ.Push(index);

    UNLOCK(mFillThisBufQ.mBufQLock);
}


void MtkOmxVenc::QueueInputBuffer(int index)
{
    LOCK(mEmptyThisBufQ.mBufQLock);

    MTK_OMX_LOGD("@@ QueueInputBuffer (%d)", index);

    mEmptyThisBufQ.PushFront(index);

    UNLOCK(mEmptyThisBufQ.mBufQLock);
}

OMX_BOOL MtkOmxVenc::QueryDriverFormat(VENC_DRV_QUERY_VIDEO_FORMAT_T *pQinfoOut)
{
    VAL_UINT32_T is_support;
    VENC_DRV_QUERY_VIDEO_FORMAT_T   qinfo;
    VENC_DRV_QUERY_VIDEO_FORMAT_T   *pQinfoIn = &qinfo;

    pQinfoIn->eVideoFormat = GetVencFormat(mCodecId);
    pQinfoIn->eResolution = VENC_DRV_RESOLUTION_SUPPORT_720P;
    pQinfoIn->u4Width = mOutputPortDef.format.video.nFrameWidth;
    pQinfoIn->u4Height = mOutputPortDef.format.video.nFrameHeight;

    switch (mCodecId) {
        case MTK_VENC_CODEC_ID_MPEG4:
        case MTK_VENC_CODEC_ID_MPEG4_SHORT:
            pQinfoIn->eVideoFormat = VENC_DRV_VIDEO_FORMAT_MPEG4;
            pQinfoIn->u4Profile = VENC_DRV_MPEG_VIDEO_PROFILE_MPEG4_SIMPLE;
            pQinfoIn->eLevel = VENC_DRV_VIDEO_LEVEL_3;
            break;
        case MTK_VENC_CODEC_ID_AVC:
            pQinfoIn->eVideoFormat = VENC_DRV_VIDEO_FORMAT_H264;
            pQinfoIn->u4Profile = Omx2DriverH264ProfileMap(mAvcType.eProfile);
            pQinfoIn->eLevel = (VENC_DRV_VIDEO_LEVEL_T)Omx2DriverH264LevelMap(mAvcType.eLevel);
            break;
        default:
            break;
    }

    // query driver property
    is_support = eVEncDrvQueryCapability(VENC_DRV_QUERY_TYPE_VIDEO_FORMAT, pQinfoIn, pQinfoOut);
    if(VENC_DRV_MRESULT_FAIL == is_support ) {
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::FlushInputPort()
{
    MTK_OMX_LOGD_ENG("+FlushInputPort");

    DumpETBQ();
    // return all input buffers currently we have
    ReturnPendingInputBuffers();

    MTK_OMX_LOGD("FlushInputPort -> mNumPendingInput(%d)", (int)mEmptyThisBufQ.mPendingNum);
    int count = 0;
    while (mEmptyThisBufQ.mPendingNum > 0)
    {
        if ((count % 100) == 0)
        {
            MTK_OMX_LOGD("Wait input buffer release....");
        }
        if (count >= 2000)
        {
            MTK_OMX_LOGE("Wait input buffer release...., go to die");
            abort();
        }
        ++count;
        SLEEP_MS(1);
    }

    MTK_OMX_LOGD_ENG("-FlushInputPort");
    return OMX_TRUE;
}


OMX_BOOL MtkOmxVenc::FlushOutputPort()
{
    MTK_OMX_LOGD_ENG("+FlushOutputPort");

    DumpFTBQ();
    // return all output buffers currently we have
    ReturnPendingOutputBuffers();

    // return all output buffers from decoder
    //  FlushEncoder();

    MTK_OMX_LOGD("-FlushOutputPort -> mNumPendingOutput(%d)", (int)mFillThisBufQ.mPendingNum);

    return OMX_TRUE;
}

OMX_ERRORTYPE MtkOmxVenc::HandlePortDisable(OMX_U32 nPortIndex)
{
    MTK_OMX_LOGD("MtkOmxVenc::HandlePortDisable nPortIndex=0x%X", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    // TODO: should we hold mEncodeLock here??

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {

        if ((mState != OMX_StateLoaded) && (mInputPortDef.bPopulated == OMX_TRUE))
        {

            if (mState == OMX_StateExecuting || mState == OMX_StatePause)
            {
                // flush input port
                FlushInputPort();
            }

            // wait until the input buffers are freed
            MTK_OMX_LOGD("@@wait on mInPortFreeDoneSem(%d)", get_sem_value(&mInPortFreeDoneSem));
            WAIT(mInPortFreeDoneSem);
            SIGNAL(mInPortFreeDoneSem);
            MTK_OMX_LOGD("@@wait on mInPortFreeDoneSem(%d)", get_sem_value(&mInPortFreeDoneSem));
        }

        // send OMX_EventCmdComplete back to IL client
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortDisable,
                               MTK_OMX_INPUT_PORT,
                               NULL);
    }

    if (nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        mOutputPortDef.bEnabled = OMX_FALSE;

        if ((mState != OMX_StateLoaded) && (mOutputPortDef.bPopulated == OMX_TRUE))
        {

            if (mState == OMX_StateExecuting || mState == OMX_StatePause)
            {
                // flush output port
                FlushOutputPort();
            }

            // wait until the output buffers are freed
            MTK_OMX_LOGD("@@wait on mOutPortFreeDoneSem(%d)", get_sem_value(&mOutPortFreeDoneSem));
            WAIT(mOutPortFreeDoneSem);
            MTK_OMX_LOGD("@@wait on mOutPortFreeDoneSem(%d)", get_sem_value(&mOutPortFreeDoneSem));
        }

        // send OMX_EventCmdComplete back to IL client
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortDisable,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }


EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::HandlePortFlush(OMX_U32 nPortIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD("MtkOmxVenc::HandleFlush nPortIndex(0x%X)", (unsigned int)nPortIndex);

    // Bruce 20130709 [
    if (mDoConvertPipeline)
    {
        LOCK(mConvertLock);
        // Because when meta mode we pass handle from convert thread to encode thread, real input
        // will be used in encoding thread. We need LOCK mEncodeLock before do FlushInputPort().
        // If not meta mode we actually only need LOCK mEncodeLock before FlushOutputPort().
        LOCK(mEncodeLock);
    }
    else
    {
        LOCK(mEncodeLock);
    }
    // ]

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        if (mDoConvertPipeline)
        {
            ReturnPendingInternalBuffers();
        }

        if (mHaveAVCHybridPlatform && (mIsHybridCodec == OMX_TRUE))
        {
            MTK_OMX_LOGD("@@ Check Last frame for 8167\n");

            if (mLastFrameBufHdr != NULL)
            {
                HandleEmptyBufferDone(mLastFrameBufHdr);
                mLastFrameBufHdr = NULL;
                MTK_OMX_LOGD("@@ EOS 2-2");
            }
        }

        FlushInputPort();

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandFlush,
                               MTK_OMX_INPUT_PORT,
                               NULL);
    }

    if (nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {

        FlushOutputPort();

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandFlush,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }

    UNLOCK(mEncodeLock);
    // Bruce 20130709 [
    if (mDoConvertPipeline)
    {
        UNLOCK(mConvertLock);
    }
    // ]

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::HandleMarkBuffer(OMX_U32 nParam1, OMX_PTR pCmdData)
{
    (void)(nParam1);
    (void)(pCmdData);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD("MtkOmxVenc::HandleMarkBuffer");

EXIT:
    return err;
}



OMX_ERRORTYPE MtkOmxVenc::HandleEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::HandleEmptyThisBuffer pBufHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)",
    //pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nFilledLen);

    int index = findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pBuffHdr);
    if (index < 0)
    {
        MTK_OMX_LOGE("[ERROR] ETB invalid index(%d)", index);
    }
    //MTK_OMX_LOGD ("ETB idx(%d)", index);

    LOCK(mEmptyThisBufQ.mBufQLock);
    if(!(pBuffHdr->nFlags & OMX_BUFFERFLAG_TRIGGER_OUTPUT)){
        ++mEmptyThisBufQ.mPendingNum;
    }
    mETBDebug = false;
    MTK_OMX_LOGD_ENG("%06x ETB (0x%08X) (0x%08X) (%lu), mNumPendingInput(%d), t(%llu)",
                 (unsigned int)this, (unsigned int)pBuffHdr, (unsigned int)pBuffHdr->pBuffer,
                 pBuffHdr->nFilledLen, mEmptyThisBufQ.mPendingNum, pBuffHdr->nTimeStamp);

    mEmptyThisBufQ.Push(index);
    //DumpETBQ();

    UNLOCK(mEmptyThisBufQ.mBufQLock);

    // trigger encode
    // SIGNAL (mEncodeSem);

    // Bruce 20130709 [
    if (mDoConvertPipeline)
    {
        SIGNAL(mConvertSem);
    }
    else
    {
        SIGNAL(mEncodeSem);
    }
    if (mDoConvertPipeline && mConvertStarted == false && CheckBufferAvailability() == OMX_TRUE)
    {
        MTK_OMX_LOGD("convert start in ETB");
        mConvertStarted = true;
    }
    // ]

    mExtensions.EmptyThisBuffer(pBuffHdr);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::HandleFillThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::HandleFillThisBuffer pBufHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)",
    //pBuffHeader, pBuffHeader->pBuffer, pBuffHeader->nAllocLen);

    int index = findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pBuffHdr);
    if (index < 0)
    {
        MTK_OMX_LOGE("[ERROR] FTB invalid index(%d)", index);
    }
    //MTK_OMX_LOGD ("FTB idx(%d)", index);

    LOCK(mFillThisBufQ.mBufQLock);

    ++mFillThisBufQ.mPendingNum;
    MTK_OMX_LOGD_ENG("%06x FTB (0x%08X) (0x%08X) (%lu), mNumPendingOutput(%d), t(%llu)",
                 (unsigned int)this, (unsigned int)pBuffHdr, (unsigned int)pBuffHdr->pBuffer,
                 pBuffHdr->nAllocLen, mFillThisBufQ.mPendingNum, pBuffHdr->nTimeStamp);

    mFillThisBufQ.Push(index);
    //DumpFTBQ();

    UNLOCK(mFillThisBufQ.mBufQLock);

#ifdef CHECK_OUTPUT_CONSISTENCY
    OMX_U8 *aOutputBuf = pBuffHdr->pBuffer + pBuffHdr->nOffset;
    native_handle_t *handle = (native_handle_t *)(aOutputBuf);
    MTK_OMX_LOGE("[CONSISTENCY]Compare %p and %p, len %d", handle->data[3], handle->data[7], handle->data[2]);
    if(0 != memcmp((void*)handle->data[3], (void*)handle->data[7], handle->data[2]))
    {
        MTK_OMX_LOGE("[Error][CONSISTENCY] and it is not");
    }
    else
    {
        int * temp = (int*)(handle->data[3]);
        MTK_OMX_LOGE("[CONSISTENCY]Clear! first few bytes: %x %x %x %x", *temp, *(temp + 1), *(temp + 2), *(temp + 3));
    }
#endif
    // trigger encode
    SIGNAL(mEncodeSem);

    // Bruce 20130709 [
    if (mDoConvertPipeline && mConvertStarted == false && CheckBufferAvailability() == OMX_TRUE)
    {
        MTK_OMX_LOGD("convert start in FTB");
        mConvertStarted = true;
        SIGNAL(mConvertSem);
    }
    // ]

    mExtensions.FillThisBuffer(pBuffHdr);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::HandleEmptyBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::HandleEmptyBufferDone pBufHead(0x%08X), pBuffer(0x%08X)",
    //pBuffHeader, pBuffHeader->pBuffer);

    mExtensions.EmptyBufferDone(pBuffHdr);

    WaitFence((OMX_U8 *)pBuffHdr->pBuffer, OMX_FALSE);

    LOCK(mEmptyThisBufQ.mBufQLock);
    --mEmptyThisBufQ.mPendingNum;
    UNLOCK(mEmptyThisBufQ.mBufQLock);

    MTK_OMX_LOGD_ENG("%06x EBD (0x%08X) (0x%08X), mNumPendingInput(%d), t(%llu)",
                 (unsigned int)this, (unsigned int)pBuffHdr,
                 (unsigned int)pBuffHdr->pBuffer, mEmptyThisBufQ.mPendingNum,
                 pBuffHdr->nTimeStamp);
    mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                              mAppData,
                              pBuffHdr);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::HandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::HandleFillBufferDone pBufHead(0x%08X), pBuffer(0x%08X),
    //nFilledLen(%u)", pBuffHeader, pBuffHeader->pBuffer, pBuffHeader->nFilledLen);

    mExtensions.FillBufferDone(pBuffHdr);

    if (pBuffHdr->nFlags & OMX_BUFFERFLAG_EOS)
    {
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventBufferFlag,
                               MTK_OMX_OUTPUT_PORT,
                               pBuffHdr->nFlags,
                               NULL);
    }

    LOCK(mFillThisBufQ.mBufQLock);
    --mFillThisBufQ.mPendingNum;
    UNLOCK(mFillThisBufQ.mBufQLock);

    dumpNativeHandle((void*)pBuffHdr->pBuffer);
    MTK_OMX_LOGD_ENG("%06x FBD (0x%08X) (0x%08X) %lld (%lu), mNumPendingOutput(%d)",
                 (unsigned int)this, (unsigned int)pBuffHdr, (unsigned int)pBuffHdr->pBuffer,
                 pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen, mFillThisBufQ.mPendingNum);
    mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                             mAppData,
                             pBuffHdr);

EXIT:
    return err;
}


void MtkOmxVenc::ReturnPendingInputBuffers()
{
    LOCK(mEmptyThisBufQ.mBufQLock);

    for (size_t i = 0 ; i < mEmptyThisBufQ.Size() ; ++i)
    {
        WaitFence((OMX_U8 *)mInputBufferHdrs[mEmptyThisBufQ.mBufQ[i]]->pBuffer, OMX_FALSE);
        --mEmptyThisBufQ.mPendingNum;
        mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                  mAppData,
                                  mInputBufferHdrs[mEmptyThisBufQ.mBufQ[i]]);
    }
    mEmptyThisBufQ.Clear();

    UNLOCK(mEmptyThisBufQ.mBufQLock);
}


void MtkOmxVenc::ReturnPendingOutputBuffers()
{
    LOCK(mFillThisBufQ.mBufQLock);

    for (size_t i = 0 ; i < mFillThisBufQ.Size() ; ++i)
    {
        --mFillThisBufQ.mPendingNum;
        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                 mAppData,
                                 mOutputBufferHdrs[mFillThisBufQ.mBufQ[i]]);
    }
    mFillThisBufQ.Clear();

    UNLOCK(mFillThisBufQ.mBufQLock);
}


void MtkOmxVenc::DumpETBQ()
{
    MTK_OMX_LOGD_ENG("--- ETBQ: ");

    for (size_t i = 0 ; i < mEmptyThisBufQ.Size() ; ++i)
    {
        MTK_OMX_LOGD_ENG("[%d] - pBuffHead(0x%08X)", mEmptyThisBufQ.mBufQ[i],
                     (unsigned int)mInputBufferHdrs[mEmptyThisBufQ.mBufQ[i]]);
    }
}


void MtkOmxVenc::DumpFTBQ()
{
    MTK_OMX_LOGD_ENG("--- FTBQ size: %d \n",mFillThisBufQ.Size());

    for (size_t i = 0 ; i < mFillThisBufQ.Size() ; ++i)
    {
        MTK_OMX_LOGD_ENG("[%d] - pBuffHead(0x%08X)", mFillThisBufQ.mBufQ[i],
                     (unsigned int)mOutputBufferHdrs[mFillThisBufQ.mBufQ[i]]);
    }
}



int MtkOmxVenc::findBufferHeaderIndex(OMX_U32 nPortIndex, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_BUFFERHEADERTYPE **pBufHdrPool = NULL;
    int bufCount;

    if (nPortIndex == MTK_OMX_INPUT_PORT)
    {
        pBufHdrPool = mInputBufferHdrs;
        bufCount = mInputPortDef.nBufferCountActual;
    }
    else if (nPortIndex == MTK_OMX_OUTPUT_PORT)
    {
        pBufHdrPool = mOutputBufferHdrs;
        bufCount = mOutputPortDef.nBufferCountActual;
    }
    else
    {
        MTK_OMX_LOGE("[ERROR] findBufferHeaderIndex invalid index(0x%X)", (unsigned int)nPortIndex);
        return -1;
    }

    for (int i = 0 ; i < bufCount ; i++)
    {
        if (pBuffHdr == pBufHdrPool[i])
        {
            // index found
            return i;
        }
    }

    return -1; // nothing found
}

OMX_BOOL MtkOmxVenc::CheckBufferAvailabilityAdvance(MtkOmxBufQ *pvInputBufQ, MtkOmxBufQ *pvOutputBufQ)
{
    //MTK_OMX_LOGD("CheckBufferAvailabilityAdvance 0x%X=%d 0x%X=%d",
    //pvInputBufQ, pvInputBufQ->mId, pvOutputBufQ, pvOutputBufQ->mId);
    int inIdx = (pvInputBufQ->IsEmpty()) ? -1 : 1;
    int outIdx = (pvOutputBufQ->IsEmpty()) ? -1 : 1;
    if (!allowEncodeVideo(inIdx, outIdx))
    {
        return OMX_FALSE;
    }
#ifdef OMX_CHECK_DUMMY
    else if(CheckNeedOutDummy())
    {
    	if(pvOutputBufQ->Size()<2)
    	{
    		//MTK_OMX_LOGD("CheckBufferAvailabilityAdvance pvOutputBufQ->Size=%d\n", pvOutputBufQ->Size());
    		return OMX_FALSE;
    	}
        return OMX_TRUE;
    }
#endif
    return OMX_TRUE;
}

int MtkOmxVenc::DequeueBufferAdvance(MtkOmxBufQ *pvBufQ)
{
    //MTK_OMX_LOGD("DequeueBufferAdvance 0x%X=%d", pvBufQ, pvBufQ->mId);
    return pvBufQ->DequeueBuffer();
}
void MtkOmxVenc::QueueBufferAdvance(MtkOmxBufQ *pvBufQ, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    if (MtkOmxBufQ::MTK_OMX_VENC_BUFQ_INPUT == pvBufQ->mId)
    {
        QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pBuffHdr));
    }
    else if (MtkOmxBufQ::MTK_OMX_VENC_BUFQ_OUTPUT == pvBufQ->mId)
    {
        QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pBuffHdr));
    }
    else if (MtkOmxBufQ::MTK_OMX_VENC_BUFQ_CONVERT_OUTPUT == pvBufQ->mId)
    {
        pvBufQ->QueueBufferBack(findBufferHeaderIndexAdvance((int)pvBufQ->mId, pBuffHdr));
    }
    else if (MtkOmxBufQ::MTK_OMX_VENC_BUFQ_VENC_INPUT == pvBufQ->mId)
    {
        pvBufQ->QueueBufferFront(findBufferHeaderIndexAdvance((int)pvBufQ->mId, pBuffHdr));
    }
    else
    {
        MTK_OMX_LOGE("[ERROR] Unknown bufQ ID!!");
    }
}

//for 89, 8135 Avc Encoder need 16/16/16 YUV plane
int MtkOmxVenc::ShiftUVforAndroidYV12(unsigned char *dst, unsigned int dstWidth, unsigned int dstHeight)
{
    //if ((dstWidth % 32) && (VAL_CHIP_NAME_MT6589 == mChipName) &&
    //mOutputPortFormat.eCompressionFormat == OMX_VIDEO_CodingAVC)
    {
        if (mTmpColorConvertBuf == NULL)
        {
            mTmpColorConvertBufSize = VENC_ROUND_N(dstWidth, 32) * VENC_ROUND_N(dstHeight, 32);
            mTmpColorConvertBuf = (unsigned char *)malloc(mTmpColorConvertBufSize);
        }
        unsigned char *tmpsrc, *tmpdst;

        unsigned int mYSize = dstWidth * dstHeight;
        unsigned int mUSize = VENC_ROUND_N((dstWidth / 2), 16) * (dstHeight / 2);

        tmpdst = mTmpColorConvertBuf;
        tmpsrc = dst + mYSize;
        MTK_OMX_LOGD("tmpdst 1:0x%x", tmpdst);
        //copy U
        int i;
        for (i = 0; i < (dstHeight >> 1); i++)
        {
            memcpy((void *)tmpdst, (void *)tmpsrc, (dstWidth >> 1));
            tmpdst += VENC_ROUND_N((dstWidth >> 1), 16);
            tmpsrc += dstWidth >> 1;
        }
        MTK_OMX_LOGD("tmpdst 2:0x%x", tmpdst);
        //copy V
        tmpsrc = dst + mYSize + mUSize;
        for (i = 0; i < (dstHeight >> 1); i++)
        {
            memcpy((void *)tmpdst, (void *)tmpsrc, (dstWidth >> 1));
            tmpdst += VENC_ROUND_N((dstWidth >> 1), 16);
            tmpsrc += dstWidth >> 1;
        }
        MTK_OMX_LOGD("tmpdst 3:0x%x", tmpdst);
        memcpy((void *)(dst + mYSize), (void *)mTmpColorConvertBuf, 2 * mUSize);
    }
    return 1;
}

OMX_BOOL MtkOmxVenc::GetVEncDrvBSBuffer(OMX_U8 *aOutputBuf, OMX_U32 aOutputSize)
{
    if (OMX_TRUE == mStoreMetaDataInOutBuffers)
    {
#ifdef SUPPORT_NATIVE_HANDLE
        if (OMX_TRUE == mIsAllocateOutputNativeBuffers)
        {
            native_handle* pHandle =  (native_handle*)aOutputBuf;
            ion_user_handle_t handle = pHandle->data[9];
            if (OMX_TRUE == mIsSecureSrc)
            {
                mBitStreamBuf.rBSAddr.u4VA      = 0;
                mBitStreamBuf.rBSAddr.u4PA      = 0;
                mBitStreamBuf.rBSAddr.u4Size    = pHandle->data[10];
                mBitStreamBuf.u4BSStartVA       = 0;
                mBitStreamBuf.rSecMemHandle     = pHandle->data[7];
                mBitStreamBuf.pIonBufhandle     = 0;
                MTK_OMX_LOGD_ENG("Get BS buffer secure handle 0x%x, size %d aOutputSize %d",
                    mBitStreamBuf.rSecMemHandle, mBitStreamBuf.rBSAddr.u4Size, aOutputSize);
	          }
	        else
	        {
	            OMX_BOOL foundIonInfo = OMX_FALSE;
                for (int i = 0; i < mIonBufferInfo.size(); ++i)
                {
                    if (handle == mIonBufferInfo[i].ion_handle)
                    {
                        foundIonInfo = OMX_TRUE;
#ifdef COPY_2_CONTIG
#ifdef CHECK_OVERFLOW
                        MTK_OMX_LOGE("Initialize %p to 0, size %d",
                            mIonBufferInfo[i].va_4_enc, mIonBufferInfo[i].value_4_enc[0]);
                        memset(mIonBufferInfo[i].va_4_enc, TITAN_CHECK_PATTERN, mIonBufferInfo[i].value_4_enc[0]);
#endif
                        mBitStreamBuf.rBSAddr.u4VA = (uint32_t)mIonBufferInfo[i].va_4_enc + TITAN_BS_SHIFT;
                        mBitStreamBuf.rBSAddr.u4PA = mIonBufferInfo[i].value_4_enc[3] + TITAN_BS_SHIFT;
                        mBitStreamBuf.rBSAddr.u4Size = mIonBufferInfo[i].value_4_enc[0] - TITAN_BS_SHIFT;
                        mBitStreamBuf.u4BSStartVA = (uint32_t)mIonBufferInfo[i].va_4_enc + TITAN_BS_SHIFT;
                        mBitStreamBuf.pIonBufhandle = mIonBufferInfo[i].ion_handle_4_enc;
#else
#ifdef CHECK_OVERFLOW
                        MTK_OMX_LOGE("Initialize %p to 0, size %d",
                            mIonBufferInfo[i].va, mIonBufferInfo[i].value[0]);
                        memset(mIonBufferInfo[i].va, TITAN_CHECK_PATTERN, mIonBufferInfo[i].value[0]);
#endif
                        mBitStreamBuf.rBSAddr.u4VA =  (uint32_t)mIonBufferInfo[i].va + BS_SHIFT;
                        mBitStreamBuf.rBSAddr.u4PA = mIonBufferInfo[i].value[3] + BS_SHIFT;
                        mBitStreamBuf.rBSAddr.u4Size = mIonBufferInfo[i].value[0] - BS_SHIFT;
                        mBitStreamBuf.u4BSStartVA =  (uint32_t)mIonBufferInfo[i].va + BS_SHIFT;
                        mBitStreamBuf.rSecMemHandle = 0;
                        mBitStreamBuf.pIonBufhandle = handle;
#endif
                        break;
                    }
                }
                if (OMX_TRUE == foundIonInfo)
                {
                    MTK_OMX_LOGD("Get BS buffer va 0x%x, size %d mva 0x%x",
                        mBitStreamBuf.rBSAddr.u4VA, mBitStreamBuf.rBSAddr.u4Size, mBitStreamBuf.rBSAddr.u4PA);
                }
                else
                {
                    MTK_OMX_LOGE("Didn't find handle %p", handle);
                    for (int i = 0; i < mIonBufferInfo.size(); ++i)
                    {
                        MTK_OMX_LOGE("handle[%d] = %p", i, mIonBufferInfo[i].ion_handle);
#ifdef COPY_2_CONTIG
                        MTK_OMX_LOGE("handle_4_enc[%d] = %p", i, mIonBufferInfo[i].ion_handle_4_enc);
#endif
                    }
                }
	        }
        }
        else
#endif
        {
        (void)(aOutputSize);
        OMX_U32 _handle = 0;
        GetMetaHandleFromBufferPtr(aOutputBuf, &_handle);
        //output is meta mode
        if (mIsSecureSrc)
        {
            int hSecHandle = 0, size = 0;

            //gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);
            //use GET_SECURE_HANDLE for buffer not pass HWC
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE, &hSecHandle);
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);
            mBitStreamBuf.rBSAddr.u4VA = 0;
            mBitStreamBuf.rBSAddr.u4PA = 0;
            mBitStreamBuf.rBSAddr.u4Size = size;
            mBitStreamBuf.u4BSStartVA = 0;
            mBitStreamBuf.rSecMemHandle = hSecHandle;
            mBitStreamBuf.pIonBufhandle = 0;
            mBitStreamBuf.u4IonDevFd = 0;
            //mBitStreamBuf.u4Flags = FRM_BUF_FLAG_SECURE;  //for in-house tee
        }
        else
        {
            //MTK_OMX_LOGD("get normal buf");
            mOutputMVAMgr->getOmxMVAFromHndlToVencBS((void *)_handle, &mBitStreamBuf);
            }
        }
    }
    else
    {
        if (mOutputMVAMgr->getOmxMVAFromVAToVencBS((void *)aOutputBuf, &mBitStreamBuf) < 0)
        {
            MTK_OMX_LOGE("[ERROR][Output][VideoEncode]\n");
            return OMX_FALSE;
        }
    }
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::GetVEncDrvFrmBuffer(OMX_U8 *aInputBuf, OMX_U32 aInputSize)
{
    if (MtkOmxBufQ::MTK_OMX_VENC_BUFQ_VENC_INPUT == mpVencInputBufQ->mId)//convert pipeline output
    {
        if (OMX_FALSE == NeedConversion())
        {
            // when pipeline without convert, input handle should passed here.
            if (OMX_FALSE == mStoreMetaDataInBuffers)
            {
                MTK_OMX_LOGE("[ERROR] MUST be meta mode!!");
                abort();
            }
            OMX_U32 _handle = 0;
            GetMetaHandleFromBufferPtr(aInputBuf, &_handle);

            int ionfd, bufSize;
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ION_FD, &ionfd);
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &bufSize);

            // free MVA and query it again
            mInputMVAMgr->freeOmxMVAByHndl((void *)_handle);

            mInputMVAMgr->newOmxMVAwithHndl((void *)_handle, NULL);
            mInputMVAMgr->getOmxMVAFromHndlToVencFrm((void *)_handle, &mFrameBuf);

            int format;
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_FORMAT, &format);
            MTK_OMX_LOGD_ENG("FrameBuf : handle = 0x%x, VA = 0x%x, PA = 0x%x, format=%s(0x%x), ion=%d",
                         (unsigned int)_handle, mFrameBuf.rFrmBufAddr.u4VA, mFrameBuf.rFrmBufAddr.u4PA,
                         PixelFormatToString((unsigned int)format), (unsigned int)format, ionfd);

            //for dump counting
            if (mYV12State != 1)//YV12
            {
                mYV12State = 1;
                ++mYV12Switch;
            }
        }
        else    //pipeline and after convert
        {
            if (mCnvtMVAMgr->getOmxMVAFromVAToVencFrm(aInputBuf, &mFrameBuf) < 0)
            {
                MTK_OMX_LOGE("[ERROR] Can't find Frm in Cnvt MVA");
                return OMX_FALSE;
            }
            MTK_OMX_LOGD_ENG("FrameBuf : VA = 0x%x, PA = 0x%x, format = not YV12",
                         mFrameBuf.rFrmBufAddr.u4VA, mFrameBuf.rFrmBufAddr.u4PA);

            //for dump counting
            if (mYV12State != 0)//after convert (RGB)
            {
                mYV12State = 0;
                ++mYV12Switch;
            }
        }
    }
    else if (MtkOmxBufQ::MTK_OMX_VENC_BUFQ_INPUT == mpVencInputBufQ->mId)//input
    {
        if (OMX_TRUE == mStoreMetaDataInBuffers)//if meta mode
        {
            if (mIsSecureSrc)
            {
                uint32_t hSecHandle = 0;
                int size = 0;
                OMX_U32 _handle = 0;
                if (OMX_TRUE == NeedConversion())
                {
                    //when secure color convert, the input handle is from mCnvtBuffer
                    GetMetaHandleFromBufferPtr(mCnvtBuffer, &_handle);
                    gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);
                    gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);
                }
                else
                {
                    GetMetaHandleFromBufferPtr(aInputBuf, &_handle);
                    int usage = 0;
                    int err = 0;
                    err = gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_USAGE, &usage);
                    if (usage & GRALLOC_USAGE_SECURE)
                    {
                        err |= gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE,
                                                   &hSecHandle);
                        if (err != 0) {
                            MTK_OMX_LOGE("query usage and sec handle fail %x", err);
                        }
                    }
                    else
                    {
                        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC,
                                            &hSecHandle);
                    }
                gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);
                }

                mFrameBuf.rFrmBufAddr.u4VA = 0;
                mFrameBuf.rFrmBufAddr.u4PA = 0;
                mFrameBuf.rFrmBufAddr.u4Size = size;
                mFrameBuf.rSecMemHandle = (VAL_UINT32_T)hSecHandle;
            }
            else//non-secure buffer
            {
                //meta mode and normal buffer
                int ionfd, size;
                int format=0;
                OMX_U32 _handle = 0;
                GetMetaHandleFromBufferPtr(aInputBuf, &_handle);
                if (NeedConversion())
                {
                    mCnvtMVAMgr->getOmxMVAFromVAToVencFrm(mCnvtBuffer, &mFrameBuf);
                    //for dump counting
                    if (mYV12State != 0)
                    {
                        mYV12State = 0;
                        ++mYV12Switch;
                    }
                    gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_FORMAT, &format);
                    MTK_OMX_LOGD_ENG("FrameBuf : handle = 0x%x, VA = 0x%x, PA = 0x%x, format=%s(0x%x)",
                                 (unsigned int)_handle, mFrameBuf.rFrmBufAddr.u4VA, mFrameBuf.rFrmBufAddr.u4PA,
                                 PixelFormatToString((unsigned int)format), (unsigned int)format);
                }
                else
                {
                    gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ION_FD, &ionfd);
                    gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);

                    // free MVA and query it again
                    mInputMVAMgr->freeOmxMVAByHndl((void *)_handle);

                    mInputMVAMgr->newOmxMVAwithHndl((void *)_handle, NULL);
                    mInputMVAMgr->getOmxMVAFromHndlToVencFrm((void *)_handle, &mFrameBuf);

                    gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_FORMAT, &format);

                    ge_smvr_info_t smvr;
                    OMX_U32 offset = 0;

                    gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SMVR_INFO, &smvr);
                    mSubFrameTotalCount = smvr.frame_count;
                    mSubFrameTimeStamp = smvr.timestamp[mSubFrameIndex]/1000; /* P1 output is nano second, change to macro second*/

                    if (mSubFrameIndex > 0)
                        offset = (VENC_ROUND_N(mEncDrvSetting.u4Width, 32) * VENC_ROUND_N(mEncDrvSetting.u4Height, 32) * 3 >> 1) * mSubFrameIndex;

                    mFrameBuf.rFrmBufAddr.u4VA += offset;
                    mFrameBuf.rFrmBufAddr.u4PA += offset;

                    MTK_OMX_LOGD_ENG("SMVR: [%d] handle = 0x%x, frame_count=%d, mSubFrameTotalCount=%d, offset = %d, size = %d, mSubFrameTimeStamp= %lld",
                                 mSubFrameIndex, (unsigned int)_handle, smvr.frame_count, mSubFrameTotalCount, offset, size, mSubFrameTimeStamp);


                    MTK_OMX_LOGD_ENG("FrameBuf : handle = 0x%x, VA = 0x%x, PA = 0x%x, format=%s(0x%x), ion=%d",
                                 (unsigned int)_handle, mFrameBuf.rFrmBufAddr.u4VA, mFrameBuf.rFrmBufAddr.u4PA,
                                 PixelFormatToString((unsigned int)format), (unsigned int)format, ionfd);
                    //for dump counting
                    if (mYV12State != 1)
                    {
                        mYV12State = 1;
                        ++mYV12Switch;
                    }
                }
                mFrameBuf.rSecMemHandle = 0;
            }
            //MTK_OMX_LOGD ("@@@ mFrameBuf.rFrmBufAddr.u4VA = 0x%08X, mFrameBuf.rFrmBufAddr.u4PA = 0x%08X, "
            //"mFrameBuf.rFrmBufAddr.u4Size = %d",
            //mFrameBuf.rFrmBufAddr.u4VA, mFrameBuf.rFrmBufAddr.u4PA, mFrameBuf.rFrmBufAddr.u4Size);
            return OMX_TRUE;
        }
        else//not meta mode
        {
            //don't have secure buffers in non meta mode
            if (NeedConversion())
            {
                mCnvtMVAMgr->getOmxMVAFromVAToVencFrm(mCnvtBuffer, &mFrameBuf);
                //for dump counting
                if (mYV12State != 0)
                {
                    mYV12State = 0;
                    ++mYV12Switch;
                }
                MTK_OMX_LOGD("FrameBuf : VA = 0x%x, PA = 0x%x, format=(0x%x)",
                             mFrameBuf.rFrmBufAddr.u4VA, mFrameBuf.rFrmBufAddr.u4PA,
                             mInputPortDef.format.video.eColorFormat);
            }
            else
            {
                if (mInputMVAMgr->getOmxMVAFromVAToVencFrm((void *)aInputBuf, &mFrameBuf) < 0)
                {
                    MTK_OMX_LOGE("[ERROR][Input][VideoEncode]\n");
                    return OMX_FALSE;
                }
            }
            return OMX_TRUE;
        }
    }
   // mFrameBuf.rFrmBufAddr.u4Size = (VAL_UINT32_T)aInputSize;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::SetVEncDrvFrmBufferFlag(OMX_BUFFERHEADERTYPE *pInputBuf) // to-do: cameraswitch
{
    MTK_OMX_LOGD_ENG("Last FrameTimestamp %llu vs current %llu", mLastFrameTimeStamp, pInputBuf->nTimeStamp);

    mFrameBuf.u4InputFlag &= ~VENC_DRV_INPUT_BUF_CAMERASWITCH;

    if(pInputBuf->nTimeStamp/1000000ULL - mLastFrameTimeStamp/1000000ULL > mCameraSwitchThreshold)
    {
        pInputBuf->nFlags |= OMX_BUFFERFLAG_CAMERASWITCH;
        mFrameBuf.u4InputFlag |= VENC_DRV_INPUT_BUF_CAMERASWITCH;

        MTK_OMX_LOGD("SetVEncDrvFrmBufferFlag VENC_DRV_INPUT_BUF_CAMERASWITCH");
    }

    return OMX_TRUE;
}

OMX_ERRORTYPE MtkOmxVenc::EncHandleEmptyBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if (MtkOmxBufQ::MTK_OMX_VENC_BUFQ_VENC_INPUT == mpVencInputBufQ->mId)//convert pipeline output
    {
        LOCK(mpVencInputBufQ->mBufQLock);
        --mpVencInputBufQ->mPendingNum;
        UNLOCK(mpVencInputBufQ->mBufQLock);

        MTK_OMX_LOGD_ENG("%06x VENC_p EBD (0x%08X) (0x%08X), mNumPendingInput(%d)",
                     (unsigned int)this, (unsigned int)pBuffHdr,
                     (unsigned int)pBuffHdr->pBuffer, mpVencInputBufQ->mPendingNum);
#if 1
        //MTK_OMX_LOGD ("MtkOmxVenc::HandleFillThisBuffer pBufHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)",
        //pBuffHeader, pBuffHeader->pBuffer, pBuffHeader->nAllocLen);
        int index = findBufferHeaderIndexAdvance(MtkOmxBufQ::MTK_OMX_VENC_BUFQ_CONVERT_OUTPUT,
                                                 MtkOmxBufQ::MTK_OMX_VENC_BUFQ_VENC_INPUT, pBuffHdr);
        if (index < 0)
        {
            MTK_OMX_LOGE("[ERROR] CNVT_p FTB invalid index(%d)", index);
        }
        //MTK_OMX_LOGD ("FTB idx(%d)", index);

        LOCK(mpConvertOutputBufQ->mBufQLock);

        ++mpConvertOutputBufQ->mPendingNum;
        MTK_OMX_LOGD_ENG("%06x CNVT_p FTB (0x%08X) (0x%08X) (%lu), mNumPendingOutput(%d)",
                     (unsigned int)this, (unsigned int)pBuffHdr, (unsigned int)pBuffHdr->pBuffer,
                     pBuffHdr->nAllocLen, mpConvertOutputBufQ->mPendingNum);

        mpConvertOutputBufQ->Push(index);
        UNLOCK(mpConvertOutputBufQ->mBufQLock);

        // trigger convert
        SIGNAL(mConvertSem);
#endif//0
    }
    else if (MtkOmxBufQ::MTK_OMX_VENC_BUFQ_INPUT == mpVencInputBufQ->mId)//input
    {
        return HandleEmptyBufferDone(pBuffHdr);
    }
    return err;
}

int MtkOmxVenc::colorConvert(OMX_U8 *aInputBuf, OMX_U32 aInputSize, OMX_U8 *aOutputBuf, OMX_U32 aOutputSize)
{
    //config color convert src
    uint32_t srcWidth       = mInputPortDef.format.video.nFrameWidth;
    uint32_t srcHeight      = mInputPortDef.format.video.nFrameHeight;
    uint32_t srcStride      = 0;
    uint32_t srcUVStride    = 0;
    uint32_t srcSliceHeight = mInputPortDef.format.video.nSliceHeight;
    DpColorFormat srcFormat = DP_COLOR_RGBA8888;
    DpSecure srcSecure = DP_SECURE_NONE;

    DpRect srcRoi;
    srcRoi.x = 0;
    srcRoi.y = 0;
    srcRoi.w = srcWidth;
    srcRoi.h = srcHeight;

    uint8_t *srcYUVbufArray[3];
    uint8_t *srcYUVMVAbufArray[3];
    unsigned int srcYUVbufSizeArray[3];
    int ret = 0;

    if (mStoreMetaDataInBuffers)//meta mode
    {
        int ionfd, size, hSecHandle = 0;
        int bufSize[3] = {0};
        OMX_U32 _handle = 0;
        GetMetaHandleFromBufferPtr(aInputBuf, &_handle);

        //get ion fd
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ION_FD, &ionfd);//get ion fd
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);//get real size
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_STRIDE, &srcStride);//get real stride
        MTK_OMX_LOGD_ENG("Meta mode, CC gralloc_extra_query : ion fd %d, size %d, input size %d", ionfd, size, aInputSize);
        switch (mInputMetaDataFormat)
        {
            case HAL_PIXEL_FORMAT_YV12:
                srcUVStride = VENC_ROUND_N(srcStride >> 1, 16);
                //srcWidth = srcStride;//width need be the same with stride...
                srcFormat = DP_COLOR_YV12;
                bufSize[0] = srcStride * srcSliceHeight; // YV12 is 3-plane data
                bufSize[1] = bufSize[0] >> 2;
                bufSize[2] = bufSize[1];
                break;
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcWidth << 2;  //RGBX is a 1-plane data
                srcFormat = DP_COLOR_RGBA8888;
                bufSize[0] = size;
                break;
            case HAL_PIXEL_FORMAT_BGRA_8888:
            case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcWidth << 2;  //BGRA is a 1-plane data
                srcFormat = DP_COLOR_BGRA8888;
                bufSize[0] = size;
                break;
            case HAL_PIXEL_FORMAT_RGB_888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcWidth * 3;   //RGB is a 1-plane data
                srcFormat = DP_COLOR_RGB888;
                bufSize[0] = size;
                break;
            case HAL_PIXEL_FORMAT_YCRCB_420_SP:
                srcUVStride = srcStride;
                srcFormat = DP_COLOR_NV21;
                break;
            default:
                MTK_OMX_LOGE("Unsupport CC PIXEL FORMAT: %d, %s", mInputMetaDataFormat,
                             PixelFormatToString(mInputMetaDataFormat));
                break;
        }

        if (mIsSecureSrc)
        {
            srcSecure = DP_SECURE;
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);

            //mBlitStream.setSrcBuffer((void**)&hSecHandle, &size, 1);
            ret = mBlitStream.setSrcBuffer((void *)hSecHandle, size); //new way, need test
        }
        else
        {

            if (mInputMetaDataFormat == HAL_PIXEL_FORMAT_YCRCB_420_SP) //2 plane
            {
                uint32_t size_array[3];
                size_array[0] = srcStride * srcSliceHeight;
                size_array[1] = size_array[0] / 2;
                ret = mBlitStream.setSrcBuffer((int)ionfd, size_array, 2); //set ion src
            }
            else
            {
                mBlitStream.setSrcBuffer((int)ionfd, (uint32_t *)bufSize, DP_COLOR_GET_PLANE_COUNT(srcFormat)); //set ion src
            }

            if (true == mDrawStripe)
            {
                drawRGBStripe((void *)_handle, srcWidth, srcHeight, size);
            }
        }
    }
    else//not meta mode
    {
        srcStride = mInputPortDef.format.video.nStride;

        //int size = srcStride * srcHeight;
        int size = aInputSize;  //size should be aInputSize
        VBufInfo srcInfo;

        switch (mInputPortDef.format.video.eColorFormat)
        {
            case OMX_COLOR_Format16bitRGB565:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcStride * 2;
                srcFormat = DP_COLOR_RGB565;
                //size = size * 2;
                ret = mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
                break;
            case OMX_COLOR_Format24bitRGB888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcStride * 3;
                srcFormat = DP_COLOR_RGB888;
                //size = size * 3;
                ret = mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
                break;
            case OMX_COLOR_Format32bitARGB8888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcStride * 4;
                srcFormat = DP_COLOR_RGBA8888;
                //size = size * 4;
                ret = mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
                break;
            case OMX_COLOR_Format32bitBGRA8888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcStride * 4;
                srcFormat = DP_COLOR_BGRA8888;
                //size = size * 4;
                ret = mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
                break;
            case OMX_COLOR_FormatYUV420Planar:
                srcUVStride = srcWidth >> 1;
                srcWidth = srcStride;//width need be the same with stride...
                srcFormat = DP_COLOR_I420;
                //size = size + (srcUVStride * srcHeight);
                {
                    mInputMVAMgr->getOmxInfoFromVA((void *)aInputBuf, &srcInfo);
                    setYUVMVAForCC((void *)aInputBuf, NULL, srcStride, srcUVStride, srcSliceHeight,
                                   srcYUVbufArray, srcYUVMVAbufArray, srcYUVbufSizeArray);
                    ret = mBlitStream.setSrcBuffer((int32_t)srcInfo.iIonFd,
                                             (unsigned int *)srcYUVbufSizeArray, 3);
                }
                //mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
                break;
            case OMX_COLOR_FormatYUV420SemiPlanar:
                srcUVStride = srcWidth;
                srcWidth = srcStride;//width need be the same with stride...
                srcFormat = DP_COLOR_NV12;
                {
                    mInputMVAMgr->getOmxInfoFromVA((void *)aInputBuf, &srcInfo);
                    setYUVMVAForCC((void *)aInputBuf, NULL, srcStride, srcUVStride, srcSliceHeight,
                                   srcYUVbufArray, srcYUVMVAbufArray, srcYUVbufSizeArray, 1);
                    ret = mBlitStream.setSrcBuffer((int32_t)srcInfo.iIonFd,
                                             (unsigned int *)srcYUVbufSizeArray, 2);
                }
                break;
            case OMX_MTK_COLOR_FormatYV12:
                srcUVStride = VENC_ROUND_N(srcWidth >> 1, 16);
                srcWidth = srcStride;//width need be the same with stride...
                srcFormat = DP_COLOR_YV12;
                {
                    uint32_t srcMVA = 0;
                    mInputMVAMgr->getOmxMVAFromVA((void *)aInputBuf, &srcMVA);
                    setYUVMVAForCC((void *)aInputBuf, (void *)srcMVA, srcStride, srcUVStride, srcSliceHeight,
                                   srcYUVbufArray, srcYUVMVAbufArray, srcYUVbufSizeArray);
                    ret = mBlitStream.setSrcBuffer((void **)srcYUVbufArray, (void **)srcYUVMVAbufArray,
                                             (unsigned int *)srcYUVbufSizeArray, 3);
                }
                break;
            default:
                ret = DP_STATUS_UNKNOWN_ERROR;
                MTK_OMX_LOGE("Unsupport CC FORMAT: %d", mInputPortDef.format.video.eColorFormat);
                break;
        }

        //mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
    }

    MTK_OMX_LOGD_ENG("cc src: w=%u, h=%u, s=%u, uvs=%u, f=0x%x, sec=%u, size=%d",
                 srcWidth, srcHeight, srcStride, srcUVStride, srcFormat, srcSecure, aInputSize);
    //flush src due to 82 some mpeg4 enc problem
    ret = mBlitStream.setSrcConfig((int32_t)srcWidth, (int32_t)srcHeight, (int32_t)srcStride, (int32_t)srcUVStride,
                             srcFormat, DP_PROFILE_BT601, eInterlace_None, &srcRoi, srcSecure, true);

    if(ret < 0)
    {
        MTK_OMX_LOGE("mBlitStream.setSrcConfig fail: %d", ret);
        return -1;
    }

    //config color convert dst
    //the output setting of color convert and drv setting shoult be the same
    uint32_t dstWidth       = mEncDrvSetting.u4Width;
    uint32_t dstHeight      = mEncDrvSetting.u4Height;
    uint32_t dstStride      = mEncDrvSetting.u4BufWidth;
    uint32_t dstUVStride    = 0;
    uint32_t dstSliceHeight = mEncDrvSetting.u4BufHeight;

    if (VENC_DRV_YUV_FORMAT_YV12 == mEncDrvSetting.eVEncFormat)
    {
        dstUVStride = VENC_ROUND_N(dstWidth >> 1, 16);
    }
    else if (VENC_DRV_YUV_FORMAT_NV12 == mEncDrvSetting.eVEncFormat
        || VENC_DRV_YUV_FORMAT_NV21 == mEncDrvSetting.eVEncFormat)
    {
        dstUVStride = dstStride;
    }
    else
    {
        //I420
        if ((dstWidth >> 1) & 0x07)
        {
            //uv width not 8-align
            dstUVStride = VENC_ROUND_N(dstWidth >> 1, 8);
        }
        else
        {
            dstUVStride = dstWidth >> 1;
        }
    }

    DpColorFormat dstFormat = dpFormatMap(mEncDrvSetting.eVEncFormat);
    DpSecure dstSecure = DP_SECURE_NONE;

    DpRect dstRoi;
    dstRoi.x = 0;
    dstRoi.y = 0;
    dstRoi.w = mEncDrvSetting.u4Width;
    dstRoi.h = mEncDrvSetting.u4Height;

    uint8_t *dstYUVbufArray[3];
    uint8_t *dstYUVMVAbufArray[3];
    unsigned int dstYUVbufSizeArray[3];

    VBufInfo dstInfo;
    bool isNeedInvalidate = false;

    if (mIsSecureSrc)//secure buffer dst MUST be meta mode
    {
        int size=0;
        int hSecHandle=0;
        OMX_U32 _handle = 0;
        GetMetaHandleFromBufferPtr(aOutputBuf, &_handle);

        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);

        dstSecure = DP_SECURE;
        //mBlitStream.setDstBuffer((void **)&dstAddr, dstYUVbufSizeArray, 3);
        ret = mBlitStream.setDstBuffer((void *)hSecHandle, size);//new way, need test
    }
    else//normal buffer
    {
        mCnvtMVAMgr->getOmxInfoFromVA((void *)aOutputBuf, &dstInfo);
        setYUVMVAForCC((void *)aOutputBuf, NULL, dstStride, dstUVStride, dstSliceHeight,
                       dstYUVbufArray, dstYUVMVAbufArray, dstYUVbufSizeArray);
        ret = mBlitStream.setDstBuffer(dstInfo.iIonFd, (unsigned int *)dstYUVbufSizeArray, 3);
    }

    VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
    OMX_U32 uGetIsNoTHWSolution = 0;
    mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_SW_SOLUTION, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uGetIsNoTHWSolution);

    if( uGetIsNoTHWSolution != 0 && mReturn == VENC_DRV_MRESULT_OK)
    {
        isNeedInvalidate = true;
    }

    if (ret < 0)
    {
        MTK_OMX_LOGE("mBlitStream.setDstBuffer fail: %d", ret);
        return -1;
    }

    MTK_OMX_LOGD_ENG("cc dst: ionfd=%d, w=%u, h=%u, s=%u, uvs=%u, f=0x%x, sec=%u rotate %d, set MDP invalidate %d", dstInfo.iIonFd,
                 dstWidth, dstHeight, dstStride, dstUVStride, dstFormat, dstSecure, mRotationType.nRotation, (int32_t)isNeedInvalidate);
    ret = mBlitStream.setDstConfig((int32_t)dstWidth, (int32_t)dstHeight, (int32_t)dstStride, (int32_t)dstUVStride,
                             dstFormat, DP_PROFILE_BT601, eInterlace_None, &dstRoi, dstSecure, isNeedInvalidate);

    if (mRotationType.nRotation == 90 || mRotationType.nRotation == 180 || mRotationType.nRotation == 270)
    {
        ret = mBlitStream.setRotate(mRotationType.nRotation);
        MTK_OMX_LOGD_ENG("cc set rotation %d", mRotationType.nRotation);
    }

    if (ret < 0)
    {
        MTK_OMX_LOGE("mBlitStream.setRotate fail: %d", ret);
        return -1;
    }

    dumpCCInput(aInputBuf, aInputSize);

#ifdef MTK_QOS_SUPPORT
    struct timeval target_end;
    gettimeofday(&target_end, NULL);
    target_end.tv_usec += 1600;
    if (target_end.tv_usec > 1000000)
    {
        target_end.tv_sec++;
        target_end.tv_usec -= 1000000;
    }
    if (DP_STATUS_RETURN_SUCCESS != mBlitStream.invalidate(&target_end))//trigger convert
#else
    if (DP_STATUS_RETURN_SUCCESS != mBlitStream.invalidate())//trigger convert
#endif
    {
        MTK_OMX_LOGE("DpBlitStream invalidate failed");
    }

    dumpCCOutput(aOutputBuf, aOutputSize);

    return 1;
}

void MtkOmxVenc::dumpCCInput(OMX_U8 *aInputBuf, OMX_U32 aInputSize)
{
    //config color convert src
    uint32_t srcWidth       = mInputPortDef.format.video.nFrameWidth;
    uint32_t srcHeight      = mInputPortDef.format.video.nFrameHeight;

    char name[128]="";

    if (OMX_TRUE == mDumpColorConvertFrame && mFrameCount < (VAL_UINT32_T)mDumpCCNum)
    {
        if (mStoreMetaDataInBuffers)//meta mode
        {
            int ionfd, size, hSecHandle = 0;
            OMX_U32 _handle = 0;
            GetMetaHandleFromBufferPtr(aInputBuf, &_handle);

            //get ion fd
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ION_FD, &ionfd);//get ion fd
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);//get real size
            if(size <= 0)
            {
                MTK_OMX_LOGE("%s, line %d, invalide size[0x%x]\n", __FUNCTION__, __LINE__,size);
            }
            switch (mInputMetaDataFormat)
            {
                case HAL_PIXEL_FORMAT_YV12:
                    if (mIsSecureSrc)
                    {
                        sprintf(name, "/sdcard/inputbc_%ux%u_%d_r%u_s.yuv", srcWidth, srcHeight,
                                gettid(), mReconfigCount);
                    }
                    else
                    {
                        if (OMX_FALSE == mDumpCts) {
                            sprintf(name, "/sdcard/inputbc_%ux%u_%d_r%u.yuv", srcWidth, srcHeight,
                                    gettid(), mReconfigCount);
                        } else {
                            sprintf(name, "/sdcard/vdump/Venc_inputbc_%u_%u_t%05d.yuv", srcWidth, srcHeight,
                                    gettid());
                        }
                    }
                    break;
                case HAL_PIXEL_FORMAT_RGBA_8888:
                case HAL_PIXEL_FORMAT_RGBX_8888:
                case HAL_PIXEL_FORMAT_BGRA_8888:
                case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
                case HAL_PIXEL_FORMAT_RGB_888:
                case HAL_PIXEL_FORMAT_YCRCB_420_SP:
                    if (mIsSecureSrc)
                    {
                        sprintf(name, "/sdcard/inputbc_%u_%u_%d_%d_r%u_s.rgb", srcWidth, srcHeight,
                                4, gettid(), mReconfigCount);
                    }
                    else
                    {
                        if (OMX_FALSE == mDumpCts) {
                            sprintf(name, "/sdcard/inputbc_%u_%u_%d_%d_r%u.rgb", srcWidth, srcHeight,
                                    4, gettid(), mReconfigCount);
                        } else {
                            sprintf(name, "/sdcard/vdump/Venc_inputbc_%u_%u_t%05d.rgb", srcWidth, srcHeight,
                                    gettid());
                        }
                    }
                    break;
                default:
                    MTK_OMX_LOGE("Unsupport CC PIXEL FORMAT: %d, %s", mInputMetaDataFormat,
                                 PixelFormatToString(mInputMetaDataFormat));
                    break;
            }

            if (mIsSecureSrc)
            {
                gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);

                sprintf(name, "/sdcard/input_%u_%u_%d_s.rgb", srcWidth, srcHeight, gettid());
                size = srcWidth * srcHeight * 4;
                dumpSecBuffer(name, hSecHandle, size);
            }
            else
            {
                VENC_DRV_PARAM_FRM_BUF_T    tmpFrm;
                if (mInputMVAMgr->getOmxMVAFromHndlToVencFrm((void *)_handle, &tmpFrm) < 0)
                {
                    mInputMVAMgr->newOmxMVAwithHndl((void *)_handle, NULL);
                    mInputMVAMgr->getOmxMVAFromHndlToVencFrm((void *)_handle, &tmpFrm);
                }
                dumpBuffer(name, (unsigned char *)tmpFrm.rFrmBufAddr.u4VA, size);
            }
        }
        else//not meta mode
        {
            switch (mInputPortDef.format.video.eColorFormat)
            {
                case OMX_COLOR_Format16bitRGB565:
                case OMX_COLOR_Format24bitRGB888:
                case OMX_COLOR_Format32bitARGB8888:
                case OMX_COLOR_Format32bitBGRA8888:
                    if (OMX_FALSE == mDumpCts) {
                        sprintf(name, "/sdcard/inputbc_%u_%u_%d_%d_r%u.rgb", srcWidth, srcHeight,
                                4, gettid(), mReconfigCount);
                    } else {
                        sprintf(name, "/sdcard/vdump/Venc_inputbc_%u_%u_t%05d.rgb", srcWidth, srcHeight,
                                gettid());
                    }
                    dumpBuffer(name, aInputBuf, aInputSize);
                    break;
                case OMX_COLOR_FormatYUV420Planar:
                case OMX_COLOR_FormatYUV420SemiPlanar:
                case OMX_MTK_COLOR_FormatYV12:
                    if (OMX_FALSE == mDumpCts) {
                        sprintf(name, "/sdcard/inputbc_%ux%u_%d_r%u.yuv", srcWidth, srcHeight,
                                gettid(), mReconfigCount);
                    } else {
                        sprintf(name, "/sdcard/vdump/Venc_inputbc_%u_%u_t%05d.yuv", srcWidth, srcHeight,
                                gettid());
                    }
                    dumpBuffer(name, aInputBuf, srcWidth * srcHeight * 3 / 2);
                    break;
                default:
                    if (OMX_FALSE == mDumpCts) {
                        sprintf(name, "/sdcard/inputbc_%ux%u_%d_r%u.yuv", srcWidth, srcHeight,
                                gettid(), mReconfigCount);
                    } else {
                        sprintf(name, "/sdcard/vdump/Venc_inputbc_%u_%u_t%05d.yuv", srcWidth, srcHeight,
                                gettid());
                    }
                    MTK_OMX_LOGE("Unsupport CC FORMAT: %d", mInputPortDef.format.video.eColorFormat);
                    break;
            }
            //dumpBuffer(name, aInputBuf, aInputSize);
        }
    }
}

void MtkOmxVenc::dumpCCOutput(OMX_U8 *aOutputBuf, OMX_U32 aOutputSize)
{
    (void)(aOutputSize);
    //the output setting of color convert and drv setting shoult be the same
    uint32_t dstWidth       = mEncDrvSetting.u4Width;
    uint32_t dstHeight      = mEncDrvSetting.u4Height;
    uint32_t dstStride      = mEncDrvSetting.u4BufWidth;
    uint32_t dstUVStride    = 0;
    uint32_t dstSliceHeight = mEncDrvSetting.u4BufHeight;

    if (OMX_TRUE == mDumpColorConvertFrame && mFrameCount < (VAL_UINT32_T)mDumpCCNum)
    {
        char name[128];
        unsigned int uYSize = dstWidth * dstHeight;
        unsigned int uUVSize = uYSize >> 2;
        unsigned char *uYAddr = aOutputBuf;
        unsigned char *u2ndPlaneAddr = NULL;
        unsigned char *u3rdPlaneAddr = NULL;

        if (VENC_DRV_YUV_FORMAT_YV12 == mEncDrvSetting.eVEncFormat)
        {
            dstUVStride = VENC_ROUND_N(dstWidth >> 1, 16);
        }
        else if (VENC_DRV_YUV_FORMAT_NV12 == mEncDrvSetting.eVEncFormat
            || VENC_DRV_YUV_FORMAT_NV21 == mEncDrvSetting.eVEncFormat)
        {
            dstUVStride = dstStride;
        }
        else
        {
            //I420
            if ((dstWidth >> 1) & 0x07)
            {
                //uv width not 8-align
                dstUVStride = VENC_ROUND_N(dstWidth >> 1, 8);
            }
            else
            {
                dstUVStride = dstWidth >> 1;
            }
        }

        if (mIsSecureSrc)
        {
            int hSecHandle=0;
            OMX_U32 _handle = 0;
            GetMetaHandleFromBufferPtr(aOutputBuf, &_handle);
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);

            sprintf(name, "/sdcard/input_%u_%u_%d_s.yuv", dstWidth, dstHeight, gettid());
            dumpSecBuffer(name, hSecHandle, dstStride * dstSliceHeight + dstUVStride * dstSliceHeight);
        }
        else
        {
            if (OMX_FALSE == mDumpCts) {
                sprintf(name, "/sdcard/inputac_%u_%u_%u_%d_%d.yuv", dstWidth, dstHeight, uYSize, uUVSize, gettid());
            } else {
                sprintf(name, "/sdcard/vdump/Venc_inputac_%u_%u_t%05d.yuv", dstWidth, dstHeight, gettid());
            }
            dumpBuffer(name, uYAddr, dstStride * dstSliceHeight + dstUVStride * dstSliceHeight);
        }
    }
    return;
}

OMX_ERRORTYPE MtkOmxVenc::CnvtHandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if (MtkOmxBufQ::MTK_OMX_VENC_BUFQ_CONVERT_OUTPUT != mpConvertOutputBufQ->mId)//check convert pipeline output
    {
        MTK_OMX_LOGE("[ERROR] convert output id is wrong!!");
        return OMX_ErrorUndefined;
    }

    LOCK(mpConvertOutputBufQ->mBufQLock);
    --mpConvertOutputBufQ->mPendingNum;
    UNLOCK(mpConvertOutputBufQ->mBufQLock);

    MTK_OMX_LOGD_ENG("%06x CNVT_p FBD (0x%08X) (0x%08X), mNumPendingInput(%d)",
                 (unsigned int)this, (unsigned int)pBuffHdr, (unsigned int)pBuffHdr->pBuffer,
                 mpConvertOutputBufQ->mPendingNum);

#if 1
    //MTK_OMX_LOGD ("MtkOmxVenc::HandleFillThisBuffer pBufHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)",
    //pBuffHeader, pBuffHeader->pBuffer, pBuffHeader->nAllocLen);
    int index = findBufferHeaderIndexAdvance(MtkOmxBufQ::MTK_OMX_VENC_BUFQ_VENC_INPUT,
                                             MtkOmxBufQ::MTK_OMX_VENC_BUFQ_CONVERT_OUTPUT, pBuffHdr);
    if (index < 0)
    {
        MTK_OMX_LOGE("[ERROR] VENC_p ETB invalid index(%d)", index);
    }
    //MTK_OMX_LOGD ("FTB idx(%d)", index);

    mVencInputBufferHdrs[index]->nFlags     = pBuffHdr->nFlags;
    mVencInputBufferHdrs[index]->nTimeStamp = pBuffHdr->nTimeStamp;
    mVencInputBufferHdrs[index]->nFilledLen = pBuffHdr->nFilledLen;
    mVencInputBufferHdrs[index]->nOffset    = pBuffHdr->nOffset;
    mVencInputBufferHdrs[index]->pMarkData  = pBuffHdr->pMarkData;
    //MTK_OMX_LOGD("transfer 0x%x, 0x%x", mVencInputBufferHdrs[index]->pMarkData,
    //mVencInputBufferHdrs[index]->nFlags);

    LOCK(mpVencInputBufQ->mBufQLock);

    ++mpVencInputBufQ->mPendingNum;
    MTK_OMX_LOGD_ENG("%06x VENC_p ETB (0x%08X) (0x%08X) (%lu), mNumPendingOutput(%d)",
                 (unsigned int)this, (unsigned int)pBuffHdr, (unsigned int)pBuffHdr->pBuffer,
                 pBuffHdr->nAllocLen, mpVencInputBufQ->mPendingNum);

    mpVencInputBufQ->Push(index);
    UNLOCK(mpVencInputBufQ->mBufQLock);

    // trigger convert
    SIGNAL(mEncodeSem);
#endif//0
    return err;
}

int MtkOmxVenc::findBufferHeaderIndexAdvance(int iBufQId, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_BUFFERHEADERTYPE **pBufHdrPool = NULL;
    MtkOmxBufQ::MtkOmxBufQId    id = (MtkOmxBufQ::MtkOmxBufQId)iBufQId;
    int bufCount;

    if (id == MtkOmxBufQ::MTK_OMX_VENC_BUFQ_INPUT)
    {
        pBufHdrPool = mInputBufferHdrs;
        bufCount = mInputPortDef.nBufferCountActual;
    }
    else if (id == MtkOmxBufQ::MTK_OMX_VENC_BUFQ_OUTPUT)
    {
        pBufHdrPool = mOutputBufferHdrs;
        bufCount = mOutputPortDef.nBufferCountActual;
    }
    else if (id == MtkOmxBufQ::MTK_OMX_VENC_BUFQ_CONVERT_OUTPUT)
    {
        pBufHdrPool = mConvertOutputBufferHdrs;
        bufCount = CONVERT_MAX_BUFFER;
    }
    else if (id == MtkOmxBufQ::MTK_OMX_VENC_BUFQ_VENC_INPUT)
    {
        pBufHdrPool = mVencInputBufferHdrs;
        bufCount = CONVERT_MAX_BUFFER;
    }
    else
    {
        MTK_OMX_LOGE("[ERROR] findBufferHeaderIndex invalid index(%d)", (unsigned int)iBufQId);
        return -1;
    }

    for (int i = 0 ; i < bufCount ; i++)
    {
        if (pBuffHdr == pBufHdrPool[i])
        {
            // index found
            return i;
        }
    }

    return -1; // nothing found
}

//find the BufferHdr index of OutBufQ which has the same buffer with pBufferHdr in InBufQ
int MtkOmxVenc::findBufferHeaderIndexAdvance(int iOutBufQId, int iInBufQId, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    (void)(iInBufQId);
    OMX_BUFFERHEADERTYPE **pBufHdrPool = NULL;
    MtkOmxBufQ::MtkOmxBufQId    id = (MtkOmxBufQ::MtkOmxBufQId)iOutBufQId;
    int bufCount;

    if (id == MtkOmxBufQ::MTK_OMX_VENC_BUFQ_CONVERT_OUTPUT)
    {
        pBufHdrPool = mConvertOutputBufferHdrs;
        bufCount = CONVERT_MAX_BUFFER;
    }
    else if (id == MtkOmxBufQ::MTK_OMX_VENC_BUFQ_VENC_INPUT)
    {
        pBufHdrPool = mVencInputBufferHdrs;
        bufCount = CONVERT_MAX_BUFFER;
    }
    else
    {
        MTK_OMX_LOGE("[ERROR] findBufferHeaderIndex invalid index(%d)", (unsigned int)iOutBufQId);
        return -1;
    }

    for (int i = 0 ; i < bufCount ; i++)
    {
        if (pBuffHdr->pBuffer == pBufHdrPool[i]->pBuffer)
        {
            // index found
            return i;
        }
    }

    return -1; // nothing found
}

OMX_BOOL MtkOmxVenc::NeedConversion()
{
    if (mInputScalingMode)
    {
        return OMX_TRUE;
    }

    if (mEncDrvSetting.u4Width != mInputPortDef.format.video.nFrameWidth
        || mEncDrvSetting.u4Height != mInputPortDef.format.video.nFrameHeight)
    {
        MTK_OMX_LOGD_ENG("NeedConversion for resize");
        return OMX_TRUE;
    }

    if (mRotationType.nRotation == 90 || mRotationType.nRotation == 180 || mRotationType.nRotation == 270 )
    {
        MTK_OMX_LOGD_ENG("NeedConversion for rotate %d degree", mRotationType.nRotation);
        return OMX_TRUE;
    }

    if (OMX_FALSE == mStoreMetaDataInBuffers)
    {
        switch (mInputPortDef.format.video.eColorFormat)
        {
            case OMX_COLOR_Format16bitRGB565:
            case OMX_COLOR_Format24bitRGB888:
            case OMX_COLOR_Format32bitARGB8888:
            case OMX_COLOR_Format32bitBGRA8888:
            case OMX_COLOR_FormatYUV420SemiPlanar:
                return OMX_TRUE;
            case OMX_COLOR_FormatYUV420Planar:
            case OMX_MTK_COLOR_FormatYV12:
                if ((mInputPortDef.format.video.nStride & 0x0f) ||
                    (mInputPortDef.format.video.nSliceHeight & 0x0f))
                {
                    //if stride/sliceheight not 16-align
                    return OMX_TRUE;
                }
                else
                {
                    return OMX_FALSE;
                }
            default:
                return OMX_FALSE;
        }
    }
    else
    {
        switch (mInputMetaDataFormat)
        {
            case HAL_PIXEL_FORMAT_YV12:
                return OMX_FALSE;
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
            case HAL_PIXEL_FORMAT_BGRA_8888:
            case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            case HAL_PIXEL_FORMAT_RGB_888:
                return OMX_TRUE;
            case HAL_PIXEL_FORMAT_YCRCB_420_SP:
                //h264 support NV21
                if (mCodecId == MTK_VENC_CODEC_ID_AVC || mCodecId == MTK_VENC_CODEC_ID_AVC_VGA)
                {
                    return OMX_FALSE;
                }
                else // mpeg4 sw encode does not support NV21 , need convert to yv12
                {
                    return OMX_TRUE;
                }
            default:
                return OMX_FALSE;
        }
    }
    //MTK_OMX_LOGD("@@@ NeedConversion bNeedConversion %x, eColorFormat %x ",
    //bNeedConversion, mInputPortFormat.eColorFormat);
    return OMX_FALSE;
}

void MtkOmxVenc::InitPipelineBuffer()
{
    int i, u4PicAllocSize;
    OMX_U32 iWidth = 0;
    OMX_U32 iHeight = 0;

    if (mInputScalingMode)
    {
        iWidth = mScaledWidth;
        iHeight = mScaledHeight;
    }
    else if (mRotationType.nRotation == 90 || mRotationType.nRotation == 270 )
    {
        iWidth =  mOutputPortDef.format.video.nFrameHeight;
        iHeight = mOutputPortDef.format.video.nFrameWidth;
    }
    else
    {
        iWidth = mOutputPortDef.format.video.nFrameWidth;
        iHeight = mOutputPortDef.format.video.nFrameHeight;
    }

    u4PicAllocSize = VENC_ROUND_N(iWidth, 32) * VENC_ROUND_N(iHeight, 32);
    mCnvtBufferSize = (u4PicAllocSize* 3) >> 1;
    mCnvtBufferSize = getHWLimitSize(mCnvtBufferSize);
    MTK_OMX_LOGD("Cnvt Buffer:w=%lu, h=%lu, size=%u",
                 VENC_ROUND_N(iWidth, 32), VENC_ROUND_N(iHeight, 32), (unsigned int)mCnvtBufferSize);

    if (mInitPipelineBuffer == true)
    {
        return;
    }

    //allocate convert buffer headers
    // allocate convert output buffer headers address array
    mConvertOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *) *
                                                                      CONVERT_MAX_BUFFER);
    MTK_OMX_MEMSET(mConvertOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*CONVERT_MAX_BUFFER);
    // allocate venc input buffer headers address array
    mVencInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *) * CONVERT_MAX_BUFFER);
    MTK_OMX_MEMSET(mVencInputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*CONVERT_MAX_BUFFER);

    for (i = 0; i < CONVERT_MAX_BUFFER; ++i)
    {
        void *CnvtBuf = NULL;
        mCnvtMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, mCnvtBufferSize, NULL, &CnvtBuf);

        mConvertOutputBufferHdrs[i] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));
        memset(mConvertOutputBufferHdrs[i], 0, sizeof(OMX_BUFFERHEADERTYPE));
        mConvertOutputBufferHdrs[i]->pBuffer    = (OMX_U8 *)CnvtBuf;
        mConvertOutputBufferHdrs[i]->nAllocLen  = mCnvtBufferSize;

        mVencInputBufferHdrs[i] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));
        memset(mVencInputBufferHdrs[i], 0, sizeof(OMX_BUFFERHEADERTYPE));
        mVencInputBufferHdrs[i]->pBuffer        = (OMX_U8 *)CnvtBuf;
        mVencInputBufferHdrs[i]->nAllocLen      = mCnvtBufferSize;

        // ALPS03676091 Clear convert dst buffer to black
        memset(((OMX_U8 *)CnvtBuf) + u4PicAllocSize, 0x80, u4PicAllocSize / 2);
        memset((OMX_U8 *)CnvtBuf, 0x10, u4PicAllocSize);

        mCnvtMVAMgr->syncBufferCacheFrm((void *)CnvtBuf, (unsigned int)ION_CACHE_FLUSH_BY_RANGE);
    }

    LOCK(mpConvertOutputBufQ->mBufQLock);
    //fill convert output bufq
    for (i = 0; i < CONVERT_MAX_BUFFER; ++i)
    {
        mpConvertOutputBufQ->Push(i);
        ++mpConvertOutputBufQ->mPendingNum;
    }
    UNLOCK(mpConvertOutputBufQ->mBufQLock);

    mInitPipelineBuffer = true;
}

void MtkOmxVenc::DeinitPipelineBuffer()
{
    int i;

    if (mInitPipelineBuffer == false)
    {
        return;
    }

    //deallocate convert buffer headers
    if (mVencInputBufferHdrs != NULL)
    {
        for (i = 0; i < CONVERT_MAX_BUFFER; ++i)
        {
            if (mVencInputBufferHdrs[i] != NULL)
            {
                // unamp only once
                mCnvtMVAMgr->freeOmxMVAByVa(mVencInputBufferHdrs[i]->pBuffer);
                MTK_OMX_FREE(mVencInputBufferHdrs[i]);
            }
        }
        MTK_OMX_FREE(mVencInputBufferHdrs);
        mVencInputBufferHdrs = NULL;
    }
    if (mConvertOutputBufferHdrs != NULL)
    {
        for (i = 0; i < CONVERT_MAX_BUFFER; ++i)
        {
            if (mConvertOutputBufferHdrs[i] != NULL)
            {
                // don't unmap here...
                MTK_OMX_FREE(mConvertOutputBufferHdrs[i]);
            }
        }
        MTK_OMX_FREE(mConvertOutputBufferHdrs);
        mConvertOutputBufferHdrs = NULL;
    }

    mInitPipelineBuffer = false;
}

void MtkOmxVenc::TryTurnOnMDPPipeline()
{
    if (mStoreMetaDataInBuffers == OMX_TRUE &&
        (mInputScalingMode == OMX_TRUE || mSetWFDMode == OMX_TRUE ||(mSetWFDMode == OMX_TRUE && mPrependSPSPPSToIDRFrames == OMX_TRUE))) // check Prepend when basic AOSP WFD
    {
        // all chip turn on MDP pipeline
        if(OMX_TRUE == mIsSecureInst){
            mDoConvertPipeline = false;
        }else{
            mDoConvertPipeline = true;
        }
        MTK_OMX_LOGD_ENG("mIsSecureInst is %d, and mDoConvertPipeline is %d",mIsSecureInst,mDoConvertPipeline);
        /*VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
        OMX_U32 uGetMDPPipeLineEnableType = 0;
        mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_MDPPIPELINE_ENABLE_TYPE, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uGetMDPPipeLineEnableType);
        MTK_OMX_LOGD("uGetMDPPipeLineEnableType%d", uGetMDPPipeLineEnableType);

        //if (mChipName == VAL_CHIP_NAME_MT8135 && mInputPortDef.format.video.nFrameWidth >= 1920)
        if ((uGetMDPPipeLineEnableType == VENC_DRV_MDP_PIPELINE_TYPE_ONE) && mInputPortDef.format.video.nFrameWidth >= 1920)
        {
            mDoConvertPipeline = true;
        }
        //else if ((mChipName == VAL_CHIP_NAME_ROME || mChipName == VAL_CHIP_NAME_MT8173 || mChipName == VAL_CHIP_NAME_MT6795) && OMX_FALSE == mIsSecureInst)
        else if ((uGetMDPPipeLineEnableType == VENC_DRV_MDP_PIPELINE_TYPE_TWO) && OMX_FALSE == mIsSecureInst)
        {
            //1080p or 720p@60 do pipeline, because they say (ME2?) sometimes input is RGBA format...
            if (mInputPortDef.format.video.nFrameWidth >= 1920 ||
                (mInputPortDef.format.video.nFrameWidth >= 1280 && 60 == (mInputPortDef.format.video.xFramerate >> 16)))
            {
                mDoConvertPipeline = true;
            }
        }
        else
        {
            mDoConvertPipeline = false;
        }*/
    }
    if (mDoConvertPipeline)
    {
        mpConvertInputBufQ  = &mEmptyThisBufQ;
        mpConvertOutputBufQ = &mConvertOutputBufQ;
        mpVencInputBufQ     = &mVencInputBufQ;
        mpVencOutputBufQ    = &mFillThisBufQ;
    }
    else
    {
        mpConvertInputBufQ  = NULL;
        mpConvertOutputBufQ = NULL;
        mpVencInputBufQ     = &mEmptyThisBufQ;
        mpVencOutputBufQ    = &mFillThisBufQ;
    }
}

void MtkOmxVenc::InitConvertBuffer()
{
    OMX_U32 iWidth  = mOutputPortDef.format.video.nFrameWidth;
    OMX_U32 iHeight = mOutputPortDef.format.video.nFrameHeight;

    if (mInputScalingMode)
    {
        iWidth = mScaledWidth;
        iHeight = mScaledHeight;
    }
    else if (mRotationType.nRotation == 90 || mRotationType.nRotation == 270 )
    {
        iWidth = mOutputPortDef.format.video.nFrameHeight;
        iHeight = mOutputPortDef.format.video.nFrameWidth;
    }

    if (mCnvtMVAMgr->count() == 0 && (NeedConversion() || mIsSecureInst))
    {
        int u4PicAllocSize = VENC_ROUND_N(iWidth, 32) * VENC_ROUND_N(iHeight, 32);
        //when secure instance, prepare convert buffer at first.
        mCnvtBufferSize = (u4PicAllocSize * 3) >> 1;
        mCnvtBufferSize = getHWLimitSize(mCnvtBufferSize);
        MTK_OMX_LOGD("InitConvertBuffer Cnvt Buffer:w=%lu, h=%lu, size=%u",
                     VENC_ROUND_N(iWidth, 32), VENC_ROUND_N(iHeight, 32), (unsigned int)mCnvtBufferSize);

        mCnvtMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, mCnvtBufferSize, NULL, (void **)(&mCnvtBuffer));

        // ALPS03676091 Clear convert dst buffer to black
        memset(((OMX_U8 *)mCnvtBuffer) + u4PicAllocSize, 0x80, u4PicAllocSize / 2);
        memset((OMX_U8 *)mCnvtBuffer, 0x10, u4PicAllocSize);

        mCnvtMVAMgr->syncBufferCacheFrm((void *)mCnvtBuffer, (unsigned int)ION_CACHE_FLUSH_BY_RANGE);
    }
}

void MtkOmxVenc::DeinitConvertBuffer()
{
    if (mCnvtMVAMgr->count() > 0)
    {
        mCnvtMVAMgr->freeOmxMVAByVa((void *)mCnvtBuffer);
        mCnvtBuffer = NULL;
    }
}

OMX_BOOL MtkOmxVenc::InitSecCnvtBuffer(int num)
{
    int count = 0, i;
    if (num > CONVERT_MAX_BUFFER)
    {
        count = CONVERT_MAX_BUFFER;
    }
    else if (num > 0)
    {
        count = num;
    }
    //else//num <= 0
    OMX_U32 iWidth  = mOutputPortDef.format.video.nFrameWidth;
    OMX_U32 iHeight = mOutputPortDef.format.video.nFrameHeight;
    uint32_t flags = GRALLOC_USAGE_HW_VIDEO_ENCODER | GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SECURE;
    for (i = 0; i < count; ++i)
    {
        mSecConvertBufs.push(new GraphicBuffer(VENC_ROUND_N(iWidth, 32), VENC_ROUND_N(iHeight, 32),
                                               HAL_PIXEL_FORMAT_YV12, flags));
        MTK_OMX_LOGD("mSecConvertBufs[%d] h=0x%x", i, (unsigned int)mSecConvertBufs[0]->handle);
    }
    return OMX_TRUE;
}

void MtkOmxVenc::DrawYUVStripeLine(OMX_U8 *pYUV)
{
    unsigned char *pY, *pY2, *pU, *pU2, *pV, *pV2;
    OMX_U32 iDrawWidth = mOutputPortDef.format.video.nFrameWidth;
    OMX_U32 iDrawHeight = mOutputPortDef.format.video.nFrameHeight;
    pY = (unsigned char *)pYUV;
    if (OMX_TRUE == mWFDMode)
    {
        pU = pY + VENC_ROUND_N(iDrawWidth, 16) * VENC_ROUND_N(iDrawHeight, 16);
        pV = pU + ((VENC_ROUND_N((iDrawWidth >> 1), 16) * VENC_ROUND_N(iDrawHeight, 16) >> 1));
        //pU = pY+VENC_ROUND_N(iDrawWidth, 16)*VENC_ROUND_N(iDrawHeight, 32);
        //pV = pU+((VENC_ROUND_N((iDrawWidth>>1), 16)*VENC_ROUND_N(iDrawHeight, 32)>>1));
    }
    else
    {
        pU = pY + VENC_ROUND_N(iDrawWidth, 16) * VENC_ROUND_N(iDrawHeight, 16);
        pV = pU + ((VENC_ROUND_N((iDrawWidth >> 1), 16) * VENC_ROUND_N(iDrawHeight, 16) >> 1));
    }
    unsigned int i;
    for (i = 0; i <= (iDrawHeight - 4) / 100; ++i)
    {
        pY2 = pY + 100 * i * VENC_ROUND_N(iDrawWidth, 16);
        pU2 = pU + 50 * i * VENC_ROUND_N((iDrawWidth >> 1), 16);
        pV2 = pV + 50 * i * VENC_ROUND_N((iDrawWidth >> 1), 16);
        //MTK_OMX_LOGE("%d %d %d", 100*i*VENC_ROUND_N(iDrawWidth, 16), 50*i*VENC_ROUND_N((iDrawWidth>>1), 16),
        //50*i*VENC_ROUND_N((iDrawWidth>>1), 16));
        for (int j = 0; j < 4; ++j)
        {
            memset(pY2, 0, iDrawWidth);
            pY2 += VENC_ROUND_N(iDrawWidth, 16);
            if (j & 1)
            {
                memset(pU2, 0, iDrawWidth >> 1);
                memset(pV2, 0, iDrawWidth >> 1);
                pU2 += VENC_ROUND_N((iDrawWidth >> 1), 16);
                pV2 += VENC_ROUND_N((iDrawWidth >> 1), 16);
            }
        }
    }
}

uint32_t MtkOmxVenc::CheckOpaqueFormat(OMX_BUFFERHEADERTYPE *pInputBuf)
{
    OMX_U8 *aInputBuf = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_U32 _handle = 0;
    unsigned int format = 0;

    if (pInputBuf->nFilledLen == 0 || OMX_TRUE != mStoreMetaDataInBuffers)
    {
        //if buffer is empty or not meta mode
        //if ((pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (0xCDCDCDCD == (OMX_U32)_handle))
        return 0;
    }
    else
    {
        GetMetaHandleFromBufferPtr(aInputBuf, &_handle);
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_FORMAT, &format);
        if (format != mInputMetaDataFormat)
        {
            MTK_OMX_LOGD("CheckOpaqueFormat : 0x%x", format);
        }

        return (uint32_t)format;
    }
}

void MtkOmxVenc::ReturnPendingInternalBuffers()
{
    OMX_BUFFERHEADERTYPE *pBuffHdr;
    MTK_OMX_LOGD("ReturnPending_I_Buffers");
    while (mpVencInputBufQ->Size() > 0)
    {
        LOCK(mpVencInputBufQ->mBufQLock);
        --mpVencInputBufQ->mPendingNum;
        UNLOCK(mpVencInputBufQ->mBufQLock);

        int input_idx = DequeueBufferAdvance(mpVencInputBufQ);
        pBuffHdr = (OMX_BUFFERHEADERTYPE *)mVencInputBufferHdrs[input_idx];
        MTK_OMX_LOGD("%06x VENC_p pBuffHdr(0x%08X) pBuffer(0x%08X) pMarkData(0x%08X) flag(0x%08X), mNum(%d)", (unsigned int)this, (unsigned int)pBuffHdr,
                     (unsigned int)pBuffHdr->pBuffer, (unsigned int)pBuffHdr->pMarkData, (unsigned int)pBuffHdr->nFlags, mpVencInputBufQ->mPendingNum);

        if (((pBuffHdr->nFlags&OMX_BUFFERFLAG_COLORCONVERT_NEEDRETURN) != 0) && pBuffHdr->pMarkData != 0) {
            pBuffHdr = (OMX_BUFFERHEADERTYPE *)mVencInputBufferHdrs[input_idx]->pMarkData;
            HandleEmptyBufferDone(pBuffHdr);
        }
    }
}

OMX_ERRORTYPE MtkOmxVenc::QueryVideoProfileLevel(VENC_DRV_VIDEO_FORMAT_T eVideoFormat,
                                                 VAL_UINT32_T u4Profile, VAL_UINT32_T eLevel)
{
    VENC_DRV_QUERY_VIDEO_FORMAT_T qInfo;
    memset(&qInfo, 0, sizeof(qInfo));
    // Query driver to see if supported
    qInfo.eVideoFormat = eVideoFormat;
    qInfo.u4Profile = u4Profile;
    qInfo.eLevel = (VENC_DRV_VIDEO_LEVEL_T)eLevel;
    VENC_DRV_MRESULT_T nDrvRet = eVEncDrvQueryCapability(VENC_DRV_QUERY_TYPE_VIDEO_FORMAT, &qInfo, 0);

    if (VENC_DRV_MRESULT_OK != nDrvRet)
    {
        MTK_OMX_LOGE("QueryVideoProfileLevel(%d) fail, profile(%d)/level(%d)", eVideoFormat, qInfo.u4Profile,
                     qInfo.eLevel);
        return OMX_ErrorNoMore;
    }
    return OMX_ErrorNone;
}

unsigned int MtkOmxVenc::getLatencyToken(OMX_BUFFERHEADERTYPE *pInputBufHdr, OMX_U8 *pInputBuf)
{
    if (OMX_TRUE == mStoreMetaDataInBuffers && OMX_TRUE == mWFDMode)//WFD mode
    {
        OMX_U32 _handle = 0;
        GetMetaHandleFromBufferPtr(pInputBuf, &_handle);

        gralloc_extra_ion_sf_info_t ext_info;
        ext_info.sequence = 0;
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &ext_info);
        //MTK_OMX_LOGD("get sequence info: %u", ext_info.sequence);
        pInputBufHdr->nTickCount = (OMX_U32)ext_info.sequence;
        return ext_info.sequence;
    }
    else
    {
        pInputBufHdr->nTickCount = (OMX_U32)mFrameCount;
        return 0;
    }
}

OMX_U32 MtkOmxVenc::getHWLimitSize(OMX_U32 bufferSize)
{
    // for AVC HW VENC Solution
    // xlmtc * ylmt * 64 + (( xlmtc % 8 == 0 ) ? 0 : (( 8 - ( xlmtc % 8 )) * 64 ))
    // worse case is 8 * 64 = 512 bytes

    // don't care platform, always add 512
    return bufferSize + 512;
}

OMX_BOOL MtkOmxVenc::CheckNeedOutDummy(void)
{
  if(mWFDMode == OMX_TRUE && mEnableDummy == OMX_TRUE ||
    (mHaveAVCHybridPlatform && (mIsHybridCodec))) //fix cts issue ALPS03040612 of miss last FBD
  {
      if (OMX_FALSE == mIsSecureSrc)
      {
         return OMX_TRUE;
      }
   }
   return OMX_FALSE;
}

OMX_U32 MtkOmxVenc::CheckGrallocWStride(OMX_BUFFERHEADERTYPE *pInputBuf)
{
    OMX_U8 *aInputBuf = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_U32 _handle = 0;
    unsigned int stride = 0;

    if (pInputBuf->nFilledLen == 0 || OMX_TRUE != mStoreMetaDataInBuffers)
    {
        //if buffer is empty or not meta mode
        //if ((pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (0xCDCDCDCD == (OMX_U32)_handle))
        if ((pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (pInputBuf->nFilledLen == 0)
            &&(OMX_TRUE == mStoreMetaDataInBuffers)){
                MTK_OMX_LOGE("CheckGrallocWStride nStride=%d\n",mInputPortDef.format.video.nStride);
                return mInputPortDef.format.video.nStride;
        }
        return 0;
    }
    else
    {
        GetMetaHandleFromBufferPtr(aInputBuf, &_handle);
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_STRIDE, &stride);
        MTK_OMX_LOGE("CheckGrallocWStride: %d", stride);
        return (uint32_t)stride;
    }
}

VENC_DRV_YUV_FORMAT_T MtkOmxVenc::CheckFormatToDrv()
{
    VENC_DRV_YUV_FORMAT_T ret = VENC_DRV_YUV_FORMAT_YV12;

    if (mStoreMetaDataInBuffers)//if meta mode
    {
        switch (mInputMetaDataFormat)
        {
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
            case HAL_PIXEL_FORMAT_BGRA_8888:
            case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            case HAL_PIXEL_FORMAT_RGB_888:
                //drv format is the format after color converting
                ret = VENC_DRV_YUV_FORMAT_YV12;
                break;
            case HAL_PIXEL_FORMAT_YV12:
                //only support YV12 (16/16/16) right now
                ret = VENC_DRV_YUV_FORMAT_YV12;
                break;
            case HAL_PIXEL_FORMAT_YCRCB_420_SP:
                //h264 support NV21 format
                if (mCodecId == MTK_VENC_CODEC_ID_AVC || mCodecId == MTK_VENC_CODEC_ID_AVC_VGA)
                {
                    ret = VENC_DRV_YUV_FORMAT_NV21;
                    MTK_OMX_LOGD("[EncSettingCodec] Input Format NV21\n");
                }
                else //mpeg4 sw encode does not support NV21, need convert
                {
                    ret = VENC_DRV_YUV_FORMAT_YV12;
                }

                break;
            default:
                MTK_OMX_LOGD("unsupported format:0x%x %s", mInputMetaDataFormat,
                             PixelFormatToString(mInputMetaDataFormat));
                ret = VENC_DRV_YUV_FORMAT_YV12;
                break;
        }
    }
    else
    {
        switch (mInputPortFormat.eColorFormat)
        {
            case OMX_COLOR_FormatYUV420Planar:
            case OMX_COLOR_FormatYUV420Flexible:
                ret = VENC_DRV_YUV_FORMAT_420;
                break;

            case OMX_MTK_COLOR_FormatYV12:
                ret = VENC_DRV_YUV_FORMAT_YV12;
                break;

            case OMX_COLOR_FormatAndroidOpaque:
                //should not be here, metaMode MUST on when format is AndroidQpaque...
                ret = VENC_DRV_YUV_FORMAT_420;
                break;

            // Gary Wu add for MediaCodec encode with input data format is RGB
            case OMX_COLOR_Format16bitRGB565:
            case OMX_COLOR_Format24bitRGB888:
            case OMX_COLOR_Format32bitARGB8888:
            case OMX_COLOR_Format32bitBGRA8888:
            case OMX_COLOR_FormatYUV420SemiPlanar:
                ret = VENC_DRV_YUV_FORMAT_420;
                break;

            //case OMX_COLOR_FormatYUV420SemiPlanar:
            //    ret = VENC_DRV_YUV_FORMAT_NV12;
            //    break;

            default:
                MTK_OMX_LOGE("[ERROR][EncSettingCodec] ColorFormat = %d, not supported ?\n",
                             mInputPortFormat.eColorFormat);
                ret = VENC_DRV_YUV_FORMAT_NONE;
                break;
        }
    }
    MTK_OMX_LOGD("[EncSettingCodec] Input Format = 0x%x, ColorFormat = 0x%x\n", mInputPortFormat.eColorFormat, ret);
    return ret;
}

DpColorFormat MtkOmxVenc::dpFormatMap(VENC_DRV_YUV_FORMAT_T format)
{
    switch (format)
    {
        case VENC_DRV_YUV_FORMAT_YV12:
            return DP_COLOR_YV12;
        case VENC_DRV_YUV_FORMAT_420:
            return DP_COLOR_I420;
        case VENC_DRV_YUV_FORMAT_NV12:
            return DP_COLOR_NV12;
        case VENC_DRV_YUV_FORMAT_NV21:
            return DP_COLOR_NV21;
        default:
            MTK_OMX_LOGE("Unsupport drv Format to CC: %d", format);
            return DP_COLOR_YV12;
    }
}

OMX_U32 MtkOmxVenc::getInputBufferSizeByFormat(OMX_VIDEO_PORTDEFINITIONTYPE video)
{
    OMX_U32 mBufferTmpSize = (VENC_ROUND_N(video.nStride, 32) * VENC_ROUND_N(video.nSliceHeight, 32) * 3 >> 1);
    OMX_U32 mBytesperPixel = 2;
    OMX_U32 mNeedDivideTwo = 0;
    switch (mInputPortDef.format.video.eColorFormat)
    {
        case OMX_COLOR_FormatYUV420Planar:
        case OMX_COLOR_FormatYUV420Flexible:
        case OMX_MTK_COLOR_FormatYV12:
        case OMX_COLOR_FormatAndroidOpaque:
        case OMX_COLOR_FormatYUV420SemiPlanar:
        {
            mBytesperPixel = 3;
            mNeedDivideTwo = 1;
        }
        break;
        case OMX_COLOR_Format32bitBGRA8888:
        case OMX_COLOR_Format32bitARGB8888:
        {
            mBytesperPixel = 4;
        }
        break;
        case OMX_COLOR_Format16bitRGB565:
        case OMX_COLOR_Format16bitBGR565:
        {
            mBytesperPixel = 2;
        }
        break;
        case OMX_COLOR_Format24bitRGB888:
        case OMX_COLOR_Format24bitBGR888:
        {
            mBytesperPixel = 3;
        }
        break;
        default:
        {
            //2 bytes per pixel
            mBytesperPixel = 3;
            mNeedDivideTwo = 1;
        }
        break;
    }

    if (mNeedDivideTwo)
    {
        mBufferTmpSize = (VENC_ROUND_N(video.nStride, 32) * VENC_ROUND_N(video.nSliceHeight, 32) * mBytesperPixel >> 1);
    }
    else
    {
        mBufferTmpSize = (VENC_ROUND_N(video.nStride, 32) * VENC_ROUND_N(video.nSliceHeight, 32) * mBytesperPixel);
    }

    return mBufferTmpSize + 512;
}

int MtkOmxVenc::drawRGBStripe(void *handle, int imgWidth, int imgHeight, int size)
{
    VENC_DRV_PARAM_FRM_BUF_T    tmpBuf;
    if (mInputMVAMgr->getOmxMVAFromHndlToVencFrm((void *)handle, &tmpBuf) < 0)
    {
        mInputMVAMgr->newOmxMVAwithHndl((void *)handle, NULL);
        mInputMVAMgr->getOmxMVAFromHndlToVencFrm((void *)handle, &tmpBuf);
    }
    unsigned char *in = (unsigned char *)tmpBuf.rFrmBufAddr.u4VA;
    MTK_OMX_LOGD("!!!!! in addr=0x%x, size = %d, format=0x%x (%s)", in, size, mInputMetaDataFormat, PixelFormatToString(mInputMetaDataFormat));
    int y, x;
    for (y = 0; y < imgHeight; ++y)
    {
        int xShift = mFrameCount % (imgWidth - 32);
        for (x = 0; x < 16; ++x)
        {
            in[(y * imgWidth * 4) + ((xShift + x) * 4)] = 0xff;
            in[(y * imgWidth * 4) + ((xShift + x) * 4) + 1] = 0x00;
            in[(y * imgWidth * 4) + ((xShift + x) * 4) + 2] = 0x00;
            in[(y * imgWidth * 4) + ((xShift + x) * 4) + 3] = 0x00;
        }
    }
    return 1;
}

int MtkOmxVenc::setYUVMVAForCC(void *va, void *mva, int stride, int uvStride, int sliceHeight,
                               uint8_t *vaArray[], uint8_t *mvaArray[], unsigned int sizeArray[],
                               int format/*0*/)
{
    // format : 0=I420, 1=NV12
    vaArray[0] = (uint8_t *)va;
    vaArray[1] = vaArray[0] + (uvStride * sliceHeight);
    if (0 == format) {
        vaArray[2] = vaArray[1] + (uvStride * (sliceHeight >> 1));
    }
    else {
        vaArray[2] = 0;
    }

    sizeArray[0] = stride * sliceHeight;
    if (0 == format) {
        sizeArray[1] = uvStride * (sliceHeight >> 1);
        sizeArray[2] = sizeArray[1];
    }
    else {
        sizeArray[1] = uvStride * (sliceHeight >> 1);
        sizeArray[2] = 0;
    }

    mvaArray[0] = (uint8_t *)mva;
    mvaArray[1] = mvaArray[0] + (stride * sliceHeight);
    if (0 == format) {
        mvaArray[2] = mvaArray[1] + (uvStride * (sliceHeight >> 1));
    }
    else {
        mvaArray[2] = 0;
    }

    MTK_OMX_LOGD_ENG("sizeArray %d, %d, %d, vaArray 0x%x, 0x%x, 0x%x mvaArray 0x%x 0x%x 0x%x",
                 sizeArray[0], sizeArray[1], sizeArray[2],
                 (unsigned int)vaArray[0], (unsigned int)vaArray[1], (unsigned int)vaArray[2], (unsigned int)mvaArray[0],(unsigned int)mvaArray[1],(unsigned int)mvaArray[2]);

    return 1;
}

bool MtkOmxVenc::supportNV12(void)
{
    //right now only AVC VENC and HW solution support NV12
    if(MTK_VENC_CODEC_ID_AVC == mCodecId && isHWSolution())
    {
        return true;
    }
    return false;
}

//Codecs on each platform may be different kinds of solution.
//That's why we check codec ID in thie function.
bool MtkOmxVenc::isHWSolution(void)
{
    switch (mCodecId)
    {
        case MTK_VENC_CODEC_ID_AVC:
        case MTK_VENC_CODEC_ID_MPEG4:
        case MTK_VENC_CODEC_ID_MPEG4_SHORT:
            {
                // !(70 80 D2 8167)
                VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
                OMX_U32 uGetIsNoTHWSolution = 0;
                mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_SW_SOLUTION, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uGetIsNoTHWSolution);
                MTK_OMX_LOGD("uGetIsNoTHWSolution%d", uGetIsNoTHWSolution);

                if(!uGetIsNoTHWSolution)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        default:
            return false;
            break;
    }
    return false;
}

OMX_BOOL MtkOmxVenc::DescribeFlexibleColorFormat(DescribeColorFormatParams *params)
{
    MediaImage &imageInfo = params->sMediaImage;
    memset(&imageInfo, 0, sizeof(imageInfo));

    imageInfo.mType = MediaImage::MEDIA_IMAGE_TYPE_UNKNOWN;
    imageInfo.mNumPlanes = 0;

    const OMX_COLOR_FORMATTYPE fmt = params->eColorFormat;
    imageInfo.mWidth = params->nFrameWidth;
    imageInfo.mHeight = params->nFrameHeight;

    MTK_OMX_LOGD_ENG("DescribeFlexibleColorFormat %d fmt %x, W/H(%d, %d), WS/HS(%d, %d), (%d, %d)", sizeof(size_t), fmt, imageInfo.mWidth, imageInfo.mHeight,
        params->nStride, params->nSliceHeight, mOutputPortDef.format.video.nStride, mOutputPortDef.format.video.nSliceHeight);

    // only supporting YUV420
    if (fmt != OMX_COLOR_FormatYUV420Planar &&
        fmt != OMX_COLOR_FormatYUV420PackedPlanar &&
        fmt != OMX_COLOR_FormatYUV420SemiPlanar &&
        fmt != HAL_PIXEL_FORMAT_I420 &&
        fmt != OMX_COLOR_FormatYUV420Flexible &&
        fmt != OMX_COLOR_FormatYUV420PackedSemiPlanar)
    {
        ALOGW("do not know color format 0x%x = %d", fmt, fmt);
        return OMX_FALSE;
    }

    // set-up YUV format
    imageInfo.mType = MediaImage::MEDIA_IMAGE_TYPE_YUV;
    imageInfo.mNumPlanes = 3;
    imageInfo.mBitDepth = 8;
    imageInfo.mPlane[MediaImage::Y].mOffset = 0;
    imageInfo.mPlane[MediaImage::Y].mColInc = 1;
    imageInfo.mPlane[MediaImage::Y].mRowInc = params->nFrameWidth;
    imageInfo.mPlane[MediaImage::Y].mHorizSubsampling = 1;
    imageInfo.mPlane[MediaImage::Y].mVertSubsampling = 1;

    switch (fmt)
    {
        case OMX_COLOR_FormatYUV420Planar: // used for YV12
        case OMX_COLOR_FormatYUV420PackedPlanar:
        case OMX_COLOR_FormatYUV420Flexible:
        case HAL_PIXEL_FORMAT_I420:
            imageInfo.mPlane[MediaImage::U].mOffset = params->nFrameWidth * params->nFrameHeight;
            imageInfo.mPlane[MediaImage::U].mColInc = 1;
            imageInfo.mPlane[MediaImage::U].mRowInc = params->nFrameWidth / 2;
            imageInfo.mPlane[MediaImage::U].mHorizSubsampling = 2;
            imageInfo.mPlane[MediaImage::U].mVertSubsampling = 2;

            imageInfo.mPlane[MediaImage::V].mOffset = imageInfo.mPlane[MediaImage::U].mOffset
                                            + (params->nFrameWidth * params->nFrameHeight / 4);
            imageInfo.mPlane[MediaImage::V].mColInc = 1;
            imageInfo.mPlane[MediaImage::V].mRowInc = params->nFrameWidth / 2;
            imageInfo.mPlane[MediaImage::V].mHorizSubsampling = 2;
            imageInfo.mPlane[MediaImage::V].mVertSubsampling = 2;
            break;

        case OMX_COLOR_FormatYUV420SemiPlanar:
            // FIXME: NV21 for sw-encoder, NV12 for decoder and hw-encoder
        case OMX_COLOR_FormatYUV420PackedSemiPlanar:
            // NV12
            imageInfo.mPlane[MediaImage::U].mOffset = params->nStride * params->nSliceHeight;
            imageInfo.mPlane[MediaImage::U].mColInc = 2;
            imageInfo.mPlane[MediaImage::U].mRowInc = params->nStride;
            imageInfo.mPlane[MediaImage::U].mHorizSubsampling = 2;
            imageInfo.mPlane[MediaImage::U].mVertSubsampling = 2;

            imageInfo.mPlane[MediaImage::V].mOffset = imageInfo.mPlane[MediaImage::U].mOffset + 1;
            imageInfo.mPlane[MediaImage::V].mColInc = 2;
            imageInfo.mPlane[MediaImage::V].mRowInc = params->nStride;
            imageInfo.mPlane[MediaImage::V].mHorizSubsampling = 2;
            imageInfo.mPlane[MediaImage::V].mVertSubsampling = 2;
            break;

        default:
            MTK_OMX_LOGE("default %x", fmt);
    }
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::GetMetaHandleFromOmxHeader(OMX_BUFFERHEADERTYPE *pBufHdr, OMX_U32 *pBufferHandle)
{
    if( (NULL == pBufHdr) || (NULL == pBufHdr->pBuffer) )
    {
        MTK_OMX_LOGD("Warning: BufferHdr is NULL !!!! LINE: %d", __LINE__);
        return OMX_FALSE;
    }
    OMX_U32 bufferType = *((OMX_U32 *)pBufHdr->pBuffer);
    // check buffer type
    //MTK_OMX_LOGD("bufferType %d", bufferType);
    if((kMetadataBufferTypeNativeHandleSource == bufferType)||(kMetadataBufferTypeGrallocSource == bufferType))
    {
        *pBufferHandle = *((OMX_U32 *)(pBufHdr->pBuffer + 4));
    }
    else if(kMetadataBufferTypeANWBuffer == bufferType)
    {
        ANativeWindowBuffer* pNWBuffer = *((ANativeWindowBuffer**)(pBufHdr->pBuffer + 4));

        if( NULL == pNWBuffer )
        {
            MTK_OMX_LOGD("Warning: pNWBuffer is NULL !!!! LINE: %d", __LINE__);
            return OMX_FALSE;
        }
        *pBufferHandle = (OMX_U32)pNWBuffer->handle;
    }
    else
    {
        MTK_OMX_LOGD("Warning: BufferType is not Gralloc Source !!!! LINE: %d", __LINE__);
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::GetMetaHandleFromBufferPtr(OMX_U8 *pBuffer, OMX_U32 *pBufferHandle)
{
    if( NULL == pBuffer )
    {
        MTK_OMX_LOGE("Warning: Buffer is NULL !!!! LINE: %d", __LINE__);
        return OMX_FALSE;
    }
    OMX_U32 bufferType = *((OMX_U32 *)pBuffer);
    // check buffer type
    //MTK_OMX_LOGD("bufferType %d", bufferType);
    if((kMetadataBufferTypeNativeHandleSource == bufferType)||(kMetadataBufferTypeGrallocSource == bufferType))
    {
        *pBufferHandle = *((OMX_U32 *)(pBuffer + 4));
    }
    else if(kMetadataBufferTypeANWBuffer == bufferType)
    {
        ANativeWindowBuffer* pNWBuffer = *((ANativeWindowBuffer**)(pBuffer + 4));

        if( NULL == pNWBuffer )
        {
            MTK_OMX_LOGD("Warning: pNWBuffer is NULL !!!! LINE: %d", __LINE__);
            return OMX_FALSE;
        }
        *pBufferHandle = (OMX_U32)pNWBuffer->handle;
    }
    else
    {
        MTK_OMX_LOGD("Warning: BufferType is not Gralloc Source !!!! LINE: %d", __LINE__);
        return OMX_FALSE;
    }

    return OMX_TRUE;
}


OMX_BOOL MtkOmxVenc::WaitFence(OMX_U8 *mBufHdr, OMX_BOOL mWaitFence)
{
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        VideoNativeMetadata &nativeMeta = *(VideoNativeMetadata *)(mBufHdr);
        //MTK_OMX_LOGD(" nativeMeta.eType %d, fd: %x", nativeMeta.eType, nativeMeta.nFenceFd);
        if(kMetadataBufferTypeANWBuffer == nativeMeta.eType)
        {
            if( 0 <= nativeMeta.nFenceFd )
            {
                MTK_OMX_LOGD_ENG(" %s for fence %d", (OMX_TRUE == mWaitFence?"wait":"noWait"), nativeMeta.nFenceFd);

                //OMX_FLASE for flush and other FBD without getFrmBuffer case
                //should close FD directly
                if(OMX_TRUE == mWaitFence)
                {
                    // Construct a new Fence object to manage a given fence file descriptor.
                    // When the new Fence object is destructed the file descriptor will be
                    // closed.
                    // from: frameworks\native\include\ui\Fence.h
                    sp<Fence> fence = new Fence(nativeMeta.nFenceFd);
                    int64_t startTime = getTickCountUs();
                    status_t ret = fence->wait(IOMX::kFenceTimeoutMs);
                    int64_t duration = getTickCountUs() - startTime;
                    //Log waning on long duration. 10ms is an empirical value.
                    if (duration >= 10000){
                        MTK_OMX_LOGD("ret %x, wait fence %d took %lld us", ret, nativeMeta.nFenceFd, (long long)duration);
                    }
                }
                else
                {
                    //Fence::~Fence() would close fd automatically so encoder should not close
                    close(nativeMeta.nFenceFd);
                }
                //client need close and set -1 after waiting fence
                nativeMeta.nFenceFd = -1;
            }
        }
    }
    return OMX_TRUE;
}

bool MtkOmxVenc::allowEncodeVideo(int inputIdx, int outputIdx)
{
    //allow encode do one frame
    return ((inputIdx >= 0 && outputIdx >= 0) ||
            (mIsHybridCodec && !mDoConvertPipeline &&
             (mMeetHybridEOS && inputIdx < 0) && outputIdx >= 0));
}

void MtkOmxVenc::dumpNativeHandle(void *native_handle)
{
    native_handle_t *nh = (native_handle_t*)native_handle;
    if (OMX_FALSE == mIsAllocateOutputNativeBuffers) {
        return;
    }
    if (nh == NULL) {
        MTK_OMX_LOGE("handle is NULL");
        return;
    }
    MTK_OMX_LOGD_ENG("handle version:%u, fdNum:%d, intNum:%d, data[0]:%d, data[1]:%d, data[2]:%d, data[4]:%d",
                     nh->version, nh->numFds, nh->numInts, nh->data[0], nh->data[1], nh->data[2], nh->data[4]);
    return;
}

void MtkOmxVenc::setViLTEOn()
{
    mIsViLTE = true;

    //reduce ViLTE mem usage
    mInputPortDef.nBufferCountActual = MTK_VENC_DEFAULT_INPUT_BUFFER_COUNT >> 1;
    mOutputPortDef.nBufferCountActual = MTK_VENC_DEFAULT_OUTPUT_BUFFER_COUNT >> 1;

    if (mInputBufferHdrs)
    {
        MTK_OMX_FREE(mInputBufferHdrs);
    }

    if (mOutputBufferHdrs)
    {
        MTK_OMX_FREE(mOutputBufferHdrs);
    }

    // allocate input buffer headers address array
    mInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(
                           sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);

    // allocate output buffer headers address array
    mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(
                            sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);

    MTK_OMX_LOGD("@@ ViLTE : reduce input/output buffer count to %d/%d",
        mInputPortDef.nBufferCountActual, mOutputPortDef.nBufferCountActual);
}

/////////////////////////// -------------------   globalc functions -----------------------------------------///////////
OMX_ERRORTYPE MtkVenc_ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_STRING componentName)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_ComponentInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentInit(hComponent, componentName);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                   OMX_IN OMX_PTR pAppData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_SetCallbacks");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetCallbacks(hComponent, pCallBacks, pAppData);
    }

    return err;
}


OMX_ERRORTYPE MtkVenc_ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_ComponentDeInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentDeInit(hComponent);
        delete(MtkOmxBase *)pHandle->pComponentPrivate;
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_STRING componentName,
                                          OMX_OUT OMX_VERSIONTYPE *componentVersion,
                                          OMX_OUT OMX_VERSIONTYPE *specVersion,
                                          OMX_OUT OMX_UUIDTYPE *componentUUID)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_GetComponentVersion");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetComponentVersion(hComponent, componentName,
                                                                              componentVersion, specVersion,
                                                                              componentUUID);
    }
    return err;
}

OMX_ERRORTYPE MtkVenc_SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                  OMX_IN OMX_COMMANDTYPE Cmd,
                                  OMX_IN OMX_U32 nParam1,
                                  OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_SendCommand");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SendCommand(hComponent, Cmd, nParam1, pCmdData);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN OMX_INDEXTYPE nParamIndex,
                                   OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_SetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetParameter(hComponent, nParamIndex, pCompParam);
    }
    return err;
}

OMX_ERRORTYPE MtkVenc_GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN  OMX_INDEXTYPE nParamIndex,
                                   OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_GetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetParameter(hComponent, nParamIndex,
                                                                       ComponentParameterStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING parameterName,
                                        OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_GetExtensionIndex");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetExtensionIndex(hComponent, parameterName, pIndexType);
    }
    return err;
}

OMX_ERRORTYPE MtkVenc_GetState(OMX_IN OMX_HANDLETYPE hComponent,
                               OMX_INOUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_GetState");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetState(hComponent, pState);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_INDEXTYPE nConfigIndex,
                                OMX_IN OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_SetConfig");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_INDEXTYPE nConfigIndex,
                                OMX_INOUT OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_GetConfig");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_INOUT OMX_BUFFERHEADERTYPE **pBuffHead,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_PTR pAppPrivate,
                                     OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_AllocateBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->AllocateBuffer(hComponent, pBuffHead, nPortIndex,
                                                                         pAppPrivate, nSizeBytes);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                OMX_IN OMX_U32 nPortIndex,
                                OMX_IN OMX_PTR pAppPrivate,
                                OMX_IN OMX_U32 nSizeBytes,
                                OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    // MTK_OMX_LOGD ("MtkVenc_UseBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->UseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate,
                                                                    nSizeBytes, pBuffer);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                 OMX_IN OMX_U32 nPortIndex,
                                 OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_FreeBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FreeBuffer(hComponent, nPortIndex, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_EmptyThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->EmptyThisBuffer(hComponent, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_FillThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FillThisBuffer(hComponent, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVenc_ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_OUT OMX_U8 *cRole,
                                        OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVenc_ComponentRoleEnum");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentRoleEnum(hComponent, cRole, nIndex);
    }
    return err;
}


// Note: each MTK OMX component must export 'MtkOmxComponentCreate" to MtkOmxCore
extern "C" OMX_COMPONENTTYPE *MtkOmxComponentCreate(OMX_STRING componentName)
{

    MtkOmxBase *pVenc  = new MtkOmxVenc;

    if (NULL == pVenc)
    {
        ALOGE("[0x%08x] ""MtkOmxComponentCreate out of memory!!!", pVenc);
        return NULL;
    }

    OMX_COMPONENTTYPE *pHandle = pVenc->GetComponentHandle();
    ALOGD("[0x%08x] ""MtkOmxComponentCreate mCompHandle(0x%08X)", pVenc, (unsigned int)pHandle);

    pHandle->SetCallbacks                  = MtkVenc_SetCallbacks;
    pHandle->ComponentDeInit               = MtkVenc_ComponentDeInit;
    pHandle->SendCommand                   = MtkVenc_SendCommand;
    pHandle->SetParameter                  = MtkVenc_SetParameter;
    pHandle->GetParameter                  = MtkVenc_GetParameter;
    pHandle->GetExtensionIndex        = MtkVenc_GetExtensionIndex;
    pHandle->GetState                      = MtkVenc_GetState;
    pHandle->SetConfig                     = MtkVenc_SetConfig;
    pHandle->GetConfig                     = MtkVenc_GetConfig;
    pHandle->AllocateBuffer                = MtkVenc_AllocateBuffer;
    pHandle->UseBuffer                     = MtkVenc_UseBuffer;
    pHandle->FreeBuffer                    = MtkVenc_FreeBuffer;
    pHandle->GetComponentVersion           = MtkVenc_GetComponentVersion;
    pHandle->EmptyThisBuffer            = MtkVenc_EmptyThisBuffer;
    pHandle->FillThisBuffer                 = MtkVenc_FillThisBuffer;

    MtkVenc_ComponentInit((OMX_HANDLETYPE)pHandle, componentName);

    return pHandle;
}

extern "C" void MtkOmxSetCoreGlobal(OMX_COMPONENTTYPE *pHandle, void *data)
{
    ((mtk_omx_core_global *)data)->video_instance_count++;
    ((MtkOmxBase *)(pHandle->pComponentPrivate))->SetCoreGlobal(data);
}

