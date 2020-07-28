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

#ifndef MTK_MJC_SCALER
#define MTK_MJC_SCALER

#include "mjc_private.h"
#include "DpBlitStream.h"
#include "vdec_drv_if_public.h"

/** MJC_IN is used to identify inputs to an MJC function.  This designation
    will also be used in the case of a pointer that points to a parameter
    that is used as an output. */
#ifndef MJC_IN
#define MJC_IN
#endif

/** MJC_OUT is used to identify outputs from an MJC function.  This
    designation will also be used in the case of a pointer that points
    to a parameter that is used as an input. */
#ifndef MJC_OUT
#define MJC_OUT
#endif


/** MJC_INOUT is used to identify parameters that may be either inputs or
    outputs from an MJC function at the same time.  This designation will
    also be used in the case of a pointer that  points to a parameter that
    is used both as an input and an output. */
#ifndef MJC_INOUT
#define MJC_INOUT
#endif


#define MJC_SCALE_WIDTH   1280
#define MJC_SCALE_HEIGHT  736
#define MJC_SCALE_MIN_WIDTH     256
#define MJC_SCALE_MIN_HEIGHT    120

#define ALIGN_MTKYV12_W 32
#define ALIGN_MTKYV12_H 16
#define ALIGN_YV12_W 16
#define ALIGN_YV12_H 16
#define ALIGN_BLK_W 16
#define ALIGN_BLK_H 32


typedef enum MJCScaler_ERRORTYPE {
    MJCScaler_ErrorNone = 0,
    MJCScaler_ErrorInsufficientResources,
    MJCScaler_ErrorBadParameter,
    MJCScaler_ErrorDrvCreateFail
} MJCScaler_ERRORTYPE;

typedef enum MJCScaler_PARAMTYPE {
    MJCScaler_PARAMTYPE_NONE = 0,
    MJCScaler_PARAM_FRAME_RESOLUTION,
    MJCScaler_PARAM_SCALE_RESOLUTION,
    MJCScaler_PARAM_MODE,
    MJCScaler_PARAM_FORMAT,
    MJCScaler_PARAM_ALIGH_SIZE,
    MJCScaler_PARAM_DRAIN_VIDEO_BUFFER,
    MJCScaler_PARAM_IS_HDRVIDEO,
    MJCScaler_PARAM_SET_COLOR_DESC,
    MJCScaler_PARAM_CROP_INFO
} MJCScaler_PARAMTYPE;

typedef enum MJCScaler_STATE {
    MJCScaler_STATE_INIT = 0,
    MJCScaler_STATE_READY,
    MJCScaler_STATE_RUNNING,
    MJCScaler_STATE_FLUSH,
    MJCScaler_STATE_MAX
} MJCScaler_STATE;

typedef unsigned int MJCScaler_MODE;

#define MJCScaler_MODE_NORMAL 0x00
#define MJCScaler_MODE_BYPASS 0x01
#define MJCScaler_MODE_FLUSH 0x10

typedef struct MJCScaler_VIDEORESOLUTION {
    unsigned int u4Width;
    unsigned int u4Height;
} MJCScaler_VIDEORESOLUTION;

typedef struct MJCScaler_VIDEOCROP {
    unsigned int mCropLeft;
    unsigned int mCropTop;
    unsigned int mCropWidth;
    unsigned int mCropHeight;
} MJCScaler_VIDEOCROP;

typedef enum MJCScaler_VIDEO_FORMAT {
    MJCScaler_FORMAT_NONE = 0,
    MJCScaler_FORMAT_BLK,
    MJCScaler_FORMAT_LINE,
    MJCScaler_FORMAT_BLK_10BIT_H,
    MJCScaler_FORMAT_BLK_10BIT_V,
} MJCScaler_VIDEO_FORMAT;

typedef struct MJCScaler_ThreadParam {
    void *pMJCScaler;
    void *pUser;
} MJCScaler_ThreadParam;

typedef struct MJCScaler_HDRVideoInfo {
    bool isHDRVideo;
} MJCScaler_HDRVideoInfo;

typedef void *MJCScaler_USERHANDLETYPE;

typedef void *MJCScaler_PTR;

class MJCScaler {
public:

    MJCScaler();
    ~MJCScaler();

    MJCScaler_ERRORTYPE Create(MJC_IN MJCScaler_USERHANDLETYPE hUser);
    MJCScaler_ERRORTYPE Init();
    MJCScaler_ERRORTYPE Deinit();

    MJCScaler_ERRORTYPE SetParameter(MJC_IN MJCScaler_PARAMTYPE nParamIndex, MJC_IN MJCScaler_PTR pCompParam);
    MJCScaler_ERRORTYPE GetParameter(MJC_IN  MJCScaler_PARAMTYPE nParamIndex, MJC_INOUT MJCScaler_PTR pCompParam);

    void EmptyThisBuffer(MJC_INT_BUF_INFO *pMJCBufInfo);
    void FillThisBuffer(MJC_INT_BUF_INFO *pMJCBufInfo);

    MJC_INT_BUF_INFO *GetScaledBuffer();

    friend void *MtkMJCScalerThread(void *pData);
    friend void *MtkMJCThread(void *pData);
    friend class MJC;


    void NormalModeRoutine(void *pData);
    void BypassModeRoutine(void *pData);
    void DefaultRoutine(void *pData);

    void InitDumpYUV(void *pData);

private:

    void ConfigAndTriggerMDP(void *pData, MJC_INT_BUF_INFO *pScalerInBufInfo, MJC_INT_BUF_INFO *pScalerOutBufInfo);
    MJCScaler_STATE mState;
    MJCScaler_MODE mMode;
    pthread_mutex_t mModeLock;

    MJCScaler_USERHANDLETYPE mhFramework;

    pthread_t mMJCScalerThread;
    MJCScaler_ThreadParam mThreadParam;

    Vector<MJC_INT_BUF_INFO *> mInputBufQ;
    pthread_mutex_t mInputBufQLock;

    Vector<MJC_INT_BUF_INFO *> mOutputBufQ;
    pthread_mutex_t mOutputBufQLock;

    Vector<MJC_INT_BUF_INFO *> mScaledBufQ;
    pthread_mutex_t mScaledBufQLock;

    sem_t mMJCScalerSem;
    sem_t mMJCScalerFlushDoneSem;

    MJCScaler_VIDEORESOLUTION mFrame;
    MJCScaler_VIDEORESOLUTION mScale;
    MJCScaler_VIDEORESOLUTION mAlignment;

    MJCScaler_VIDEO_FORMAT mFormat;

    unsigned int u4DumpYUV;
    unsigned int u4DumpCount;
    unsigned int u4DumpStartTime;

    DpBlitStream   *pStream;

    bool mMJCLog;
    bool mMJCScalertagLog;

    bool mScalerDrainInputBuffer;
    bool mScalerInputDrained;
    pthread_mutex_t mScalerInputDrainedLock;

    bool mIsHDRVideo;
    VDEC_DRV_COLORDESC_T *mpColorDesc;
    MJCScaler_VIDEOCROP mCropInfo;
};


#endif
