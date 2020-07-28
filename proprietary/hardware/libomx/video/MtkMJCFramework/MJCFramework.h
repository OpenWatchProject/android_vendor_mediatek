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

#ifndef MTK_MJC_FRAMEWORK
#define MTK_MJC_FRAMEWORK

#include "MJCScaler.h"
#include "mjc_private.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MTK_BASIC_PACKAGE

#include "mjc_lib.h"

#define MTK_MJC_DRIVER_LIB_NAME "/vendor/lib/libmjc.so"
#define MTK_MJC_FAKE_ENGINE_DRIVER_LIB_NAME "/vendor/lib/libmjcFakeEngine.so"


#define MTK_MJC_DRVIER_CREATE_NAME      "eMjcDrvCreate"
#define MTK_MJC_DRVIER_RELEASE_NAME     "eMjcDrvRelease"
#define MTK_MJC_DRVIER_INIT_NAME        "eMjcDrvInit"
#define MTK_MJC_DRVIER_DEINIT_NAME      "eMjcDrvDeInit"
#define MTK_MJC_DRVIER_PROCESS_NAME     "eMjcDrvProcess"
#define MTK_MJC_DRVIER_RESET_NAME       "eMjcDrvReset"
#define MTK_MJC_DRVIER_GETPARAM_NAME    "eMjcDrvGetParam"
#define MTK_MJC_DRVIER_SETPARAM_NAME    "eMjcDrvSetParam"

typedef MJC_DRV_RESULT_T(*eMjcDrvCreate_ptr)(unsigned int *);
typedef MJC_DRV_RESULT_T(*eMjcDrvRelease_ptr)(unsigned int);
typedef MJC_DRV_RESULT_T(*eMjcDrvInit_ptr)(unsigned int , MJC_DRV_CONFIG_T * , MJC_DRV_INIT_INFO_T *);
typedef MJC_DRV_RESULT_T(*eMjcDrvDeInit_ptr)(unsigned int);
typedef MJC_DRV_RESULT_T(*eMjcDrvProcess_ptr)(unsigned int , MJC_DRV_FRAME_T *, MJC_DRV_NEXT_INFO_T *);
typedef MJC_DRV_RESULT_T(*eMjcDrvReset_ptr)(unsigned int , MJC_DRV_RESET_CONFIG_T *, MJC_DRV_RESET_INFO_T *);
typedef MJC_DRV_RESULT_T(*eMjcDrvGetParam_ptr)(unsigned int , MJC_DRV_INDEXTYPE_T , void *);
typedef MJC_DRV_RESULT_T(*eMjcDrvSetParam_ptr)(unsigned int , MJC_DRV_INDEXTYPE_T , void *);

#endif

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

#define MTK_MJC_DEFAULT_OUTPUT_BUFFER_COUNT    3 + MIN_UNDEQUEUED_BUFS + EXTRA_CANCEL_BUFFER_CNT
#define MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT   4 + MIN_UNDEQUEUED_BUFS + EXTRA_CANCEL_BUFFER_CNT

#if (ANDROID_VER >= ANDROID_KK)
#define MAX_TOTAL_BUFFER_CNT     64
#else
#define MAX_TOTAL_BUFFER_CNT     32
#endif


#define MTK_MJC_SWCODEC_SUPPORT_RESOLUTION (1280*736)

typedef enum MJC_ERRORTYPE {
    MJC_ErrorNone = 0,
    MJC_ErrorInsufficientResources,
    MJC_ErrorBadParameter,
    MJC_ErrorDrvCreateFail,
    MJC_ErrorLoadDriverLib
} MJC_ERRORTYPE;

typedef enum MJC_PARAMTYPE {
    MJC_PARAMTYPE_NONE = 0,
    MJC_PARAM_FORMAT,
    MJC_PARAM_FRAME_RESOLUTION,
    MJC_PARAM_BUFFER_RESOLTUION,
    MJC_PARAM_ALIGN_RESOLTUION,
    MJC_PARAM_CALLBACKS,
    MJC_PARAM_MODE,
    MJC_PARAM_3DTYPE,
    MJC_PARAM_CODECTYPE,
    MJC_PARAM_SEEK,
    MJC_PARAM_ALIGH_SIZE,
    MJC_PARAM_CROP_INFO,
    //#ifdef MTK_16X_SLOWMOTION_VIDEO_SUPPORT
    MJC_PARAM_16XSLOWMOTION_MODE,
    MJC_PARAM_SLOWMOTION_SPEED,
    MJC_PARAM_SLOWMOTION_SECTION,
    //#endif
    //#ifdef MTK_DEINTERLACE_SUPPORT
    MJC_PARAM_RUNTIME_DISABLE,
    //#endif
    MJC_PARAM_PRESCALE,
    MJC_PARAM_CHIP_NAME,
    MJC_PARAM_DRIVER_REGISTER,
    MJC_PARAM_DRIVER_UNREGISTER,
    MJC_PARAM_DRAIN_VIDEO_BUFFER,
    MJC_PARAM_IS_HDRVIDEO,
    MJC_PARAM_SET_COLOR_DESC,
    MJC_PARAM_SET_DEMO_MODE
} MJC_PARAMTYPE;

typedef enum MJC_STATE {
    MJC_STATE_INIT = 0,
    MJC_STATE_READY,
    MJC_STATE_RUNNING,
    MJC_STATE_FLUSH,
    MJC_STATE_MAX
} MJC_STATE;

typedef enum MJC_MODE_OPERATION {
    MJC_MODE_SET = 0,
    MJC_MODE_ADD
} MJC_MODE_OPERATION;


typedef unsigned int MJC_MODE;

#define MJC_MODE_NORMAL 0x00
#define MJC_MODE_BYPASS 0x01
#define MJC_MODE_FLUSH 0x10

typedef unsigned int MJC_BUFFER_STATUS;

#define MJC_NO_NEW_OUTPUT_BUFFER 0x01
#define MJC_NO_NEW_REF_BUFFER 0x10
#define MJC_NO_NEW_INPUT_BUFFER 0x100



typedef struct MJC_VIDEORESOLUTION {
    unsigned int u4Width;
    unsigned int u4Height;
} MJC_VIDEORESOLUTION;

typedef struct MJC_VIDEOCROP {
    unsigned int mCropLeft;
    unsigned int mCropTop;
    unsigned int mCropWidth;
    unsigned int mCropHeight;
} MJC_VIDEOCROP;

typedef enum MJC_VIDEO_FORMAT {
    MJC_FORMAT_NONE = 0,
    MJC_FORMAT_BLK,
    MJC_FORMAT_LINE,
    MJC_FORMAT_BLK_10BIT_H,
    MJC_FORMAT_BLK_10BIT_V
} MJC_VIDEO_FORMAT;

typedef enum MJC_3DTYPE {
    MJC_3DTYPE_OFF = 0,
    MJC_3DTYPE_FRAME_SEQ,
    MJC_3DTYPE_SIDE_BY_SIDE,
    MJC_3DTYPE_TOP_BOTTOM,
    MJC_3DTYPE_MAX
} MJC_3DTYPE;

typedef enum MJC_DEMO_TYPE {
    MJC_DEMO_VERTICAL = 0,
    MJC_DEMO_HORIZONTAL
} MJC_DEMO_TYPE;


typedef enum MJC_EVENT_TYPE {
    MJC_EVENT_MODECHANGE = 0,
    MJC_EVENT_SCALERBYPASS,
    MJC_EVENT_MAX
} MJC_EVENT_TYPE;

typedef enum MJC_REF_STA {
    MJC_NO_REF_SHIFTED = 0,
    MJC_REF_SHIFTED_BACKWARD,
    MJC_REF_SHIFTED_FORWARD,
} MJC_REF_STA;

typedef struct MJC_CALLBACKTYPE {
    OMX_BUFFERHEADERTYPE *(*pfnGetInputFrame)(OMX_HANDLETYPE hComponent);
    OMX_BUFFERHEADERTYPE *(*pfnGetOutputFrame)(OMX_HANDLETYPE hComponent);
    void (*pfnPutDispFrame)(OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr);

    void (*pfnSetRef)(OMX_HANDLETYPE hComponent, MJC_INT_BUF_INFO *pMJCBufInto);
    void (*pfnClearRef)(OMX_HANDLETYPE hComponent, MJC_INT_BUF_INFO *pMJCBufInto);
    void *(*pfnGetBufVA)(OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr);
    void *(*pfnGetBufPA)(OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr);
    void *(*pfnGetBufGraphicHndl)(OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr);
    MJC_INT_BUF_INFO (*pfnGetBufInfo)(OMX_HANDLETYPE hComponent, OMX_BUFFERHEADERTYPE *pBuffHdr);

    void (*pfnEventHandler)(OMX_HANDLETYPE hComponent, const MJC_EVENT_TYPE eEventtype, void *param);
} MJC_CALLBACKTYPE;

typedef struct MJC_ThreadParam {
    void *pMJC;
    void *pUser;
} MJC_ThreadParam;

typedef struct MJC_CfgParam {
    unsigned int u4NrmFbLvlIdx;
    unsigned int u4BdrFbLvlIdx;
    unsigned int u4DemoMode;
    unsigned int u4MaxWidth;
    unsigned int u4MaxHeight;
    unsigned int u4MinWidth;
    unsigned int u4MinHeight;
    unsigned int u4MinFps;
    unsigned int u4MaxFps;
    unsigned int u4OutFrmRate;
} MJC_CfgParam;

typedef void *MJC_USERHANDLETYPE;

typedef void *MJC_PTR;

typedef unsigned int MJC_DRVHANDLE;

typedef struct MJC_POWER_SAVING_SETTING {
    int i4DummyBuffer;
    int i4LowPower;
    int i4Cafrc;
} MJC_PowerSavingSetting;

typedef struct MJC_HDRVideoInfo {
    bool isHDRVideo;
    unsigned int u4Width;
    unsigned int u4Height;
} MJC_HDRVideoInfo;

typedef struct MJC_ColorProfile {
    OMX_U32 u4VideoRange;         /* 0: narrow; 1: full  */
    OMX_BOOL bVideoRangeExist;   /* 0: not exist; 1: exist */
    OMX_COLOR_PRIMARIES_E eColourPrimaries;  /* OMX_COLOR_PRIMARIES_E */
    OMX_BOOL bColourPrimariesExist; /* 0: not exist; 1: exist */
} MJC_ColorProfile;


class MJC {
public:

    MJC();
    ~MJC();

    MJC_ERRORTYPE Create(MJC_IN MJC_USERHANDLETYPE hUser);
    MJC_ERRORTYPE Init(bool fgUsed);
    MJC_ERRORTYPE Deinit();

    MJC_ERRORTYPE SetParameter(MJC_IN MJC_PARAMTYPE nParamIndex, MJC_IN MJC_PTR pCompParam);
    MJC_ERRORTYPE GetParameter(MJC_IN  MJC_PARAMTYPE nParamIndex, MJC_INOUT MJC_PTR pCompParam);

    friend void *MtkMJCThread(void *pData);

    bool AtomicChangeMode(MJC_MODE_OPERATION eOP, MJC_MODE eNewMode);

    friend void *MtkOmxVdecDecodeThread(void *pData);
    friend class MtkOmxVdec;

    friend void *MtkMJCScalerThread(void *pData);
    friend class MJCScaler;

    void SetApkParameter(MJC_CfgParam *pParam);
    bool ParseMjcConfig(MJC_CfgParam *pParam);
    void SetFallBackIndex(MJC_CfgParam *pParam);

    // Called by main thread
    bool PrepareBuffer_CheckFrameRate(void *pData);
    void CheckInitCondition(void *pData);
    void InitDriver(void *pData);
    void InitDumpYUV(void *pData);
    void BypassModeRoutine(void *pData);
    void NormalModeRoutine(void *pData);
    void DefaultRoutine(void *pData);

    void PDFAndSetMJCBufferFlag(void *pdata, MJC_INT_BUF_INFO *pMJCBufInfo);
    MJC_INT_BUF_INFO *MJCGetBufInfoFromOMXHdr(void *pVdec, OMX_BUFFERHEADERTYPE *pBufHdr);
    void MJCSetBufRef(void *pVdec, MJC_INT_BUF_INFO *pMJCBufInfo);
    void MJCClearBufRef(void *pVdec, MJC_INT_BUF_INFO *pMJCBufInfo);

    bool CheckSupportSlowmotionSpeed(unsigned int u4ToBeCheckSpeed);

    bool LoadMJCDrvFunc();
    void UnLoadMJCDrvFunc();

private:

    unsigned int FrameRateDetect(bool *fgDFR, unsigned char *pBufDepth, unsigned char *pLowFRHint);
    void ShowDemoLine(unsigned long pBuffer, int u4Width, int u4Height, MJC_VIDEO_FORMAT eFormat, MJC_DEMO_TYPE eType);
    //#ifdef MTK_16X_SLOWMOTION_VIDEO_SUPPORT
    unsigned int ForceTriggerRatio(unsigned int u4InFrameRate);
    //#endif
    void PrepareOutputBuffer(void *pdata, MJC_BUFFER_STATUS *pBufferStatus);
    void PrepareInputBuffer(void *pdata, MJC_BUFFER_STATUS *pBufferStatus);
    void PrepareRefBuffer(void *pData, MJC_BUFFER_STATUS *pBufferStatus);
    void SendToDisplay(void *pData, void *pProcResult, void *pFrame);
    void SyncDumpFrmNum(void *pFrame);
    void DumpDispFrame(bool bOriginalFrame, unsigned long u4VA, OMX_TICKS nTimeStamp, OMX_U32 nFilledLen);
    void SendRefToDispInSeeking(void *pData);

    MJC_STATE mState;
    MJC_MODE mMode;
    pthread_mutex_t mModeLock;
    pthread_mutex_t mMJCBufInfoQLock;
    unsigned int mMJCBufInfoNum;

    MJC_USERHANDLETYPE hComponent;

    pthread_t mMJCThread;
    MJC_ThreadParam mThreadParam;

    MJC_INT_BUF_INFO mMJCBufInfoQ[MAX_TOTAL_BUFFER_CNT];
    Vector<MJC_INT_BUF_INFO *> mRefBufQ;
    Vector<MJC_INT_BUF_INFO *> mOutputBufQ;
    Vector<OMX_TICKS> mRefBufQTS;

    MJC_CALLBACKTYPE mCallbacks;

    bool mCheckDFR;
    bool mDFR;
    unsigned int mDetectNum;

    unsigned int mInFrmRate;    // Real framerate x 10, ie 100 as 10fps.
    unsigned int mOutFrmRate;   // Real framerate x 10, ie 100 as 10fps.    Init value = 600;
    unsigned int mFRRatio;

    unsigned int mInFrmDuration;
    unsigned int mOutFrmDuration;

    OMX_TICKS mRefTimestamp;
    OMX_TICKS mLastTimestamp;
    unsigned int mFrmCount;

    MJC_VIDEO_FORMAT mFormat;
    MJC_VIDEO_FORMAT mMJCInputFormat;
    unsigned int mCodecType;
    MJC_3DTYPE m3DType;

    MJC_VIDEORESOLUTION mFrame;
    MJC_VIDEORESOLUTION mBuffer;
    MJC_VIDEORESOLUTION mDrvConfig;

    MJC_VIDEORESOLUTION mAlignment;
    MJC_VIDEOCROP mCropInfo;

    unsigned int mInputNumRequire;
    unsigned int mOutputNumRequire;


#ifndef MTK_BASIC_PACKAGE
    void *mpMTK_MJC_Lib;

    eMjcDrvCreate_ptr       mfn_eMjcDrvCreate;
    eMjcDrvRelease_ptr      mfn_eMjcDrvRelease;
    eMjcDrvInit_ptr         mfn_eMjcDrvInit;
    eMjcDrvDeInit_ptr       mfn_eMjcDrvDeInit;
    eMjcDrvProcess_ptr      mfn_eMjcDrvProcess;
    eMjcDrvReset_ptr        mfn_eMjcDrvReset;
    eMjcDrvGetParam_ptr     mfn_eMjcDrvGetParam;
    eMjcDrvSetParam_ptr     mfn_eMjcDrvSetParam;
#endif

    MJC_DRVHANDLE mDrv;
    bool mDrvCreate;
    bool mDrvInit;

    bool mSeek;
    bool mEOSReceived;

    MJCScaler mScaler;
    bool mTerminated;
    sem_t mMJCFrameworkSem;
    bool mUseScaler;

    unsigned int u4DumpYUV;
    unsigned int u4DumpCount;
    unsigned int u4DumpStartTime;
    unsigned int u4DumpType; // 0: interpolated frame only, 1: original frame only, 2: all frame
    bool mbStartDump;
    bool mRunTimeDisable;
    //#ifdef MTK_16X_SLOWMOTION_VIDEO_SUPPORT
    bool mbSlowMotionMode;
    unsigned int mu4SlowmotionSpeed;
    OMX_MTK_SLOWMOTION_SECTION meSlowMotionSection;
    //#endif
    MJC_CfgParam cfgParam;

    int mThreadTid;

    int mPerfRegHandle;
    bool mbNeedToEnablePerfService;

    bool mbSupport120Hz;
    void *pRRC;
    bool mbNotifyRRC;

    bool mbUseDummyBuffer;

    MJC_REF_STA eOutputShiftedOrg;
    unsigned int mu4ChipName;
    MJC_POWER_SAVING_SETTING mePowerSavingSetting;

    bool mMJCLog;
    bool mMJCFrameworktagLog;

    bool mMJCDrainInputBuffer;

    bool mIsHDRVideo;
    unsigned int mApkSetDemoMode;

    MJC_ColorProfile mOutputColorProfile;
};

#define ALIGN(val, align) ((val + align - 1) & ~(align - 1))


MJC_ERRORTYPE MJCCreateInstance(MJC **ppMJCInstance);
MJC_ERRORTYPE MJCDestroyInstance(MJC *pMJCInstance);
MJC_ERRORTYPE MJCCreate(MJC *pMJCInstance, MJC_USERHANDLETYPE hUser);
MJC_ERRORTYPE MJCInit(MJC *pMJCInstance, bool fgUsed);
MJC_ERRORTYPE MJCDeInit(MJC *pMJCInstance);
MJC_ERRORTYPE MJCSetParameter(MJC *pMJCInstance, MJC_PARAMTYPE nParamIndex, MJC_PTR pCompParam);
MJC_ERRORTYPE MJCGetParameter(MJC *pMJCInstance, MJC_PARAMTYPE nParamIndex, MJC_PTR pCompParam);

#ifdef __cplusplus
}
#endif

#endif
