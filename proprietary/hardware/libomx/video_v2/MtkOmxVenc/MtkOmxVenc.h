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

#ifndef MTK_OMX_VENC
#define MTK_OMX_VENC

#include "MtkOmxBase.h"
#include "properties.h"

#define ANDROID_ICS 1

#include "val_api_public.h"

//#include <window.h>

#include "MtkOmxMVAMgr.h"
#include "RoiNet.h"
#include "RoiInfoConfig.h"
#include "Profiler.h"

#include "val_types_public.h"
#include "venc_drv_if_public.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include "utils/KeyedVector.h"
#include "OMX_Video_Extensions.h"
#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>
#include "videodev2.h"
#include "v4l2-controls.h"

#define MFV_OUTPUT_BUFFER_NUM 8
#define DEC_TRYALLOCMEMCOUNT 10
#define H264_MAX_FRAME_NUM  0xFFFF

#include <ui/GraphicBuffer.h>//for handle output
#include <graphics_mtk_defs.h>//for ROME BGRX definition
#include "DpBlitStream.h"
#include <gralloc_mtk_defs.h>//for GRALLOC_USAGE_SECURE
#include <hardware/gralloc.h>
#include <ui/gralloc_extra.h>

#include <HardwareAPI.h> //for describeFlexibleYUVFormat
#include <queue>
#include <MtkOmxVencCmdQueue.h>
#include <MtkOmxVencBufQueue.h>

// Morris Yang 20110610 [
#define MTK_VENC_DEFAULT_INPUT_BUFFER_COUNT 12  // for non-camera encode
//#define MTK_VENC_DEFAULT_INPUT_BUFFER_COUNT 1   // for camera encode
// ]
#define MTK_VENC_DEFAULT_OUTPUT_BUFFER_COUNT 12
#define MTK_VENC_DEFAULT_CONVERT_BUFFER_COUNT 3

//#define MTK_VENC_TIMESTAMP_WINDOW_SIZE 4

#define UHD_AREA (3840*2160)
#define FHD_AREA (1920*1088)
#define HD_AREA (1280*736)

#define MTK_VENC_DEFAULT_INPUT_BUFFER_SIZE          38016
#define MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_AVC     1024*1024*3/2   //69*1024  //38581
#define MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_AVC_1080P (4*1024*1024)
#define MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_AVC_4K    (14*1024*1024)

#define MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_HEVC    2*1024*1024 //69*1024  //38581
#define MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_HEVC_4K 8*1024*1024
#define MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_HEIC    16*1024*1024*3/2
#define MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_MPEG4   1024*1024   //128*1024
#define MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_VP8     1024*1024   //128*1024
#define MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_VP8_1080P (2*1024*1024)

#define MTK_VENC_DEFAULT_OUTPUT_BITRATE    64000

#define MTK_OMXVENC_EXTENSION_INDEX_PARAM_SET_FORCE_I_FRAME "OMX.MTK.index.param.video.EncSetForceIframe"
#define MTK_OMXVENC_EXTENSION_INDEX_PARAM_SET_I_INTERVAL    "OMX.MTK.index.param.video.EncSetIFrameRate"
#define MTK_OMXVENC_EXTENSION_INDEX_PARAM_SET_VENC_SCENARIO "OMX.MTK.index.param.video.SetVencScenario"

//for skype
#define DRIVERVER_RESERVED  0x0000ULL
#define DRIVERVER_MAJOR     0x0001
#define DRIVERVER_MINOR     0x0000

#undef MTK_OMX_LOGD
#undef MTK_OMX_LOGE

#define MORE_DEBUG_LOG
#ifdef MORE_DEBUG_LOG
#define MTK_OMX_LOGD(fmt, arg...)       ALOGD("[0x%08x] " fmt, this, ##arg)
#define MTK_OMX_LOGW(fmt, arg...)       ALOGW("[0x%08x] " fmt, this, ##arg)
#define MTK_OMX_LOGE(fmt, arg...)       ALOGE("[0x%08x] " fmt, this, ##arg)
#define MTK_OMX_LOGD_ENG(fmt, arg...) \
do { \
    if (MtkOmxVenc::mEnableMoreLog >= 1) { \
        ALOGD("[0x%08x] " fmt, this, ##arg); \
    } \
} while(0)

#define MTK_OMX_LOGV_ENG(fmt, arg...) \
do { \
    if (MtkOmxVenc::mEnableMoreLog >= 2) { \
        ALOGD("[0x%08x] " fmt, this, ##arg); \
    } \
} while(0)

#define IN_FUNC() \
    MTK_OMX_LOGV_ENG("+ %s():%d\n", __func__, __LINE__)

#define OUT_FUNC() \
    MTK_OMX_LOGV_ENG("- %s():%d\n", __func__, __LINE__)

#define PROP() \
    MTK_OMX_LOGV_ENG(" --> %s : %d\n", __func__, __LINE__)

#else
#define MTK_OMX_LOGD(fmt, arg...)
#define MTK_OMX_LOGW(fmt, arg...)
#define MTK_OMX_LOGE(fmt, arg...)
#define MTK_OMX_LOGD_ENG(fmt, arg...)
#define MTK_OMX_LOGV_ENG(fmt, arg...)
#define IN_FUNC()
#define OUT_FUNC()
#define PROP()
#endif


#define XORSWAP(a, b) ((&(a) == &(b)) ? (a) : ((a)^=(b),(b)^=(a),(a)^=(b)))
#define SWAP_VALUE XORSWAP


#define OMX_TICKU_PER_SECOND 1000000LLU

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

//#define MTK_DUM_SEC_ENC 0
typedef enum MTK_VENC_CODEC_ID
{
    MTK_VENC_CODEC_ID_AVC,
    MTK_VENC_CODEC_ID_AVC_VGA,      //VCODEC_MULTI_THREAD  (77 org)
    MTK_VENC_CODEC_ID_MPEG4,
    MTK_VENC_CODEC_ID_MPEG4_1080P,  //VCODEC_MULTI_THREAD  (77 org)
    MTK_VENC_CODEC_ID_HEVC,
    MTK_VENC_CODEC_ID_VP8,
    MTK_VENC_CODEC_ID_H263_VT,
    MTK_VENC_CODEC_ID_H263,
    MTK_VENC_CODEC_ID_HEIF,
    MTK_VENC_CODEC_ID_INVALID = 0xFFFFFFFF,
} MTK_VENC_CODEC_ID;

typedef struct  MTK_VENC_PROFILE_LEVEL_ENTRY
{
    OMX_U32 profile;
    OMX_U32 level;
} MTK_VENC_PROFILE_LEVEL_ENTRY;

typedef struct {
    int width;
    int height;
    int buf_w;
    int buf_h;
    int format; // for input buf reference
    int codec; // for output buf reference
    int hdr;
    int profile;
    int level;
    int bitrate;
    int framerate;
    int gop;
    int prepend_hdr;
    int bframe_count;
} video_encode_param;

// for SECURE VIDEO PATH && TRUSTONIC TEE SUPPORT [
typedef struct _MtkH264Venc_TLC
{
    void *tlcLib;

    void *(*tlcHandleCreate)();
    void (*tlcHandleRelease)(void *);

    int (*tlcDumpSecMem)(void *, uint32_t, void *, uint32_t);
    int (*tlcFillSecMem)(void *, void *, uint32_t, uint32_t);
} MtkH264Venc_TLC;
// ]
#define SUPPORT_NATIVE_HANDLE 1
#define BS_SHIFT (0)
#define CHECK_PATTERN (0xB)
#define USE_WFD_RCCODE 1
#ifdef SUPPORT_NATIVE_HANDLE
#include <linux/ion.h>
#include <linux/ion_drv.h>
#include <ion/ion.h>
#include <linux/mtk_ion.h>
#include <ion.h>
#include <sys/mman.h>
#include "bandwidth_control.h"
#endif

class MtkOmxVenc : public MtkOmxBase
{
    public:
        MtkOmxVenc();
        ~MtkOmxVenc();

        virtual OMX_ERRORTYPE ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_STRING componentName);

        virtual OMX_ERRORTYPE  ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent);

        virtual OMX_ERRORTYPE  GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                                   OMX_IN OMX_STRING componentName,
                                                   OMX_OUT OMX_VERSIONTYPE *componentVersion,
                                                   OMX_OUT OMX_VERSIONTYPE *specVersion,
                                                   OMX_OUT OMX_UUIDTYPE *componentUUID);

        virtual OMX_ERRORTYPE  SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                           OMX_IN OMX_COMMANDTYPE Cmd,
                                           OMX_IN OMX_U32 nParam1,
                                           OMX_IN OMX_PTR pCmdData);

        virtual OMX_ERRORTYPE  GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN  OMX_INDEXTYPE nParamIndex,
                                            OMX_INOUT OMX_PTR ComponentParameterStructure);

        virtual OMX_ERRORTYPE  SetParameter(OMX_IN OMX_HANDLETYPE hComp,
                                            OMX_IN OMX_INDEXTYPE nParamIndex,
                                            OMX_IN OMX_PTR pCompParam);

        virtual OMX_ERRORTYPE  GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_INDEXTYPE nConfigIndex,
                                         OMX_INOUT OMX_PTR ComponentConfigStructure);

        virtual OMX_ERRORTYPE  SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_INDEXTYPE nConfigIndex,
                                         OMX_IN OMX_PTR ComponentConfigStructure);

        virtual OMX_ERRORTYPE GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                                OMX_IN OMX_STRING parameterName,
                                                OMX_OUT OMX_INDEXTYPE *pIndexType);

        virtual OMX_ERRORTYPE  GetState(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_INOUT OMX_STATETYPE *pState);

        virtual OMX_ERRORTYPE  UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                         OMX_IN OMX_U32 nPortIndex,
                                         OMX_IN OMX_PTR pAppPrivate,
                                         OMX_IN OMX_U32 nSizeBytes,
                                         OMX_IN OMX_U8 *pBuffer);


        virtual OMX_ERRORTYPE  AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                              OMX_INOUT OMX_BUFFERHEADERTYPE **pBuffHead,
                                              OMX_IN OMX_U32 nPortIndex,
                                              OMX_IN OMX_PTR pAppPrivate,
                                              OMX_IN OMX_U32 nSizeBytes);


        virtual OMX_ERRORTYPE  FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_U32 nPortIndex,
                                          OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead);


        virtual OMX_ERRORTYPE  EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                               OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead);


        virtual OMX_ERRORTYPE  FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                              OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead);

        virtual OMX_ERRORTYPE  SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                            OMX_IN OMX_PTR pAppDat);

        virtual OMX_ERRORTYPE  ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                                 OMX_OUT OMX_U8 *cRole,
                                                 OMX_IN OMX_U32 nIndex);

        friend void *MtkOmxVencThread(void *pData);
        friend void *MtkOmxVencEncodeThread(void *pData);

        VENC_DRV_VIDEO_FORMAT_T GetVencFormat(MTK_VENC_CODEC_ID codecId);

        void EncodeVideo(OMX_BUFFERHEADERTYPE *pInputBuf);
        inline uint32_t VENC_ROUND_N(uint32_t x, uint32_t n)
        {
            //n must be exponential of 2
            return ((x + (n - 1)) & (~(n - 1)));
        }
        inline uint32_t VENC_FLOOR_N(uint32_t x, uint32_t n)
        {
            //n must be exponential of 2
            return ((x / n) * n);
        }
        //#define VENC_ROUND_N(X, N)   (((X) + ((N)-1)) & (~((N)-1))) //only for N is exponential of 2
        //#define VENC_FLOOR_N(X, N)   ((int)(((X)/(N)))*(N))         //only for N is exponential of 2
        static OMX_U8 mEnableMoreLog;

    private:

        OMX_BOOL PortBuffersPopulated();
        OMX_BOOL FlushInputPort();
        OMX_BOOL FlushOutputPort();
        OMX_BOOL CheckBufferAvailability(); // check if we have at least one input buffer and one output buffer
        void QueueOutputBuffer(int index);  // queue output buffer to the tail of the fill buffer list
        void QueueInputBuffer(int index);    // queue input buffer to the head of the empty buffer list

        OMX_BOOL QueryDriverFormat(VENC_DRV_QUERY_VIDEO_FORMAT_T *pQinfoOut);

        OMX_ERRORTYPE HandleStateSet(OMX_U32 nNewState);
        OMX_ERRORTYPE HandlePortEnable(OMX_U32 nPortIndex);
        OMX_ERRORTYPE HandlePortDisable(OMX_U32 nPortIndex);
        OMX_ERRORTYPE HandlePortFlush(OMX_U32 nPortIndex);
        OMX_ERRORTYPE HandleMarkBuffer(OMX_U32 nParam1, OMX_PTR pCmdData);

        OMX_ERRORTYPE HandleEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_ERRORTYPE HandleFillThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_ERRORTYPE HandleEmptyBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr);
        OMX_ERRORTYPE HandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr);

        void ReturnPendingInputBuffers();
        void ReturnPendingOutputBuffers();

        OMX_VIDEO_PARAM_PORTFORMATTYPE mInputPortFormat;
        OMX_VIDEO_PARAM_PORTFORMATTYPE mOutputPortFormat;

        OMX_VIDEO_PARAM_AVCTYPE     mAvcType;
        OMX_VIDEO_PARAM_MPEG4TYPE   mMpeg4Type;
        OMX_VIDEO_PARAM_H263TYPE    mH263Type;
        OMX_VIDEO_PARAM_HEVCTYPE    mHevcType;
        OMX_VIDEO_PARAM_VP8TYPE     mVp8Type;

        OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE mErrorCorrectionType;
        OMX_VIDEO_PARAM_PROFILELEVELTYPE    mProfileLevelType;
        OMX_VIDEO_PARAM_QUANTIZATIONTYPE    mQuantizationType;
        OMX_VIDEO_PARAM_VBSMCTYPE           mVbsmcType;
        OMX_VIDEO_PARAM_MOTIONVECTORTYPE    mMvType;
        OMX_VIDEO_PARAM_INTRAREFRESHTYPE    mIntraRefreshType;
        OMX_VIDEO_PARAM_AVCSLICEFMO         mAvcSliceFMO;
        OMX_CONFIG_FRAMERATETYPE            mFrameRateType;
        OMX_CONFIG_ROTATIONTYPE             mRotationType;
        OMX_VIDEO_CONFIG_BITRATETYPE        mConfigBitrate;
        OMX_CONFIG_INTRAREFRESHVOPTYPE      mConfigIntraRefreshVopType;
        OMX_VIDEO_CONFIG_AVCINTRAPERIOD     mConfigAVCIntraPeriod;
        OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE mConfigAndroidIntraPeriod;
        OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE mLayerParams;
        OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE mLayerConfig;

        MtkOmxCmdQueue mOMXCmdQueue;
        pthread_mutex_t mCmdQLock;

        pthread_t mVencThread;
        pthread_t mVencEncodeThread;

        VAL_UINT32_T mVencThreadTid;
        VAL_UINT32_T mVencEncThreadTid;
        VAL_UINT32_T mVencCnvtThreadTid;

        unsigned int mPendingStatus;
        OMX_BOOL mEncodeStarted;
        OMX_BOOL mIsComponentAlive;

        pthread_mutex_t mEncodeLock;

        // for UseBuffer/AllocateBuffer
        sem_t mInPortAllocDoneSem;
        sem_t mOutPortAllocDoneSem;

        // for FreeBuffer
        sem_t mInPortFreeDoneSem;
        sem_t mOutPortFreeDoneSem;

        sem_t mEncodeSem;

        MTK_VENC_CODEC_ID mCodecId;

        VENC_DRV_PARAM_BS_BUF_T         mBitStreamBuf;
        VENC_DRV_PARAM_FRM_BUF_T        mFrameBuf;

        OMX_BOOL mEncoderInitCompleteFlag;

        VAL_UINT32_T mHeaderLen;
        VAL_UINT32_T mFrameCount;
        VAL_UINT32_T mBsCount;

        void DumpETBQ();
        void DumpFTBQ();

        MtkOmxBufQ  mEmptyThisBufQ;
        MtkOmxBufQ  mFillThisBufQ;

        VAL_UINT32_T mETBCount;
        VAL_UINT32_T mFTBCount;
        VAL_UINT32_T mEBDCount;
        VAL_UINT32_T mFBDCount;

        void EncodeFunc(OMX_BUFFERHEADERTYPE *pInputBuf);
        OMX_ERRORTYPE EncodeInit(OMX_BUFFERHEADERTYPE *pInputBuf);
        OMX_ERRORTYPE EncodeEos(OMX_BUFFERHEADERTYPE *pInputBuf);

        const char *codecName();

        OMX_BOOL InitEncParams();
        OMX_BOOL InitH263EncParams();
        OMX_BOOL InitMpeg4EncParams();
        OMX_BOOL InitAvcEncParams();
        OMX_BOOL InitHevcEncParams();
        OMX_BOOL InitHeifEncParams();
        OMX_BOOL InitVP8EncParams();

        VAL_BOOL_T QueryDriverEnc();
        VAL_BOOL_T EncSettingEnc();
        VAL_BOOL_T EncSettingEncCommon();

        VAL_BOOL_T QueryDriverH264Enc();
        VAL_BOOL_T EncSettingH264Enc();

        VAL_BOOL_T QueryDriverMPEG4Enc();
        VAL_BOOL_T EncSettingMPEG4Enc();

        VAL_BOOL_T QueryDriverHEVCEnc();
        VAL_BOOL_T EncSettingHEVCEnc();

        VAL_BOOL_T QueryDriverHEIFEnc();
        VAL_BOOL_T EncSettingHEIFEnc();

        VAL_BOOL_T QueryDriverVP8Enc();
        VAL_BOOL_T EncSettingVP8Enc();

        VAL_UINT32_T Omx2DriverH264ProfileMap(OMX_VIDEO_AVCPROFILETYPE eProfile);
        VAL_UINT32_T Omx2DriverH264LevelMap(OMX_VIDEO_AVCLEVELTYPE eLevel);
        VAL_UINT32_T Omx2DriverMPEG4ProfileMap(OMX_VIDEO_MPEG4PROFILETYPE eProfile);
        VAL_UINT32_T Omx2DriverMPEG4LevelMap(OMX_VIDEO_MPEG4LEVELTYPE eLevel);
        VAL_UINT32_T Omx2DriverH263ProfileMap(OMX_VIDEO_H263PROFILETYPE eProfile);
        VAL_UINT32_T Omx2DriverH263LevelMap(OMX_VIDEO_H263LEVELTYPE eLevel);
        VAL_UINT32_T Omx2DriverHEVCProfileMap(OMX_VIDEO_HEVCPROFILETYPE eProfile);
        VAL_UINT32_T Omx2DriverHEVCLevelMap(OMX_VIDEO_HEVCLEVELTYPE eLevel);

        OMX_VIDEO_AVCPROFILETYPE defaultAvcProfile();
        OMX_VIDEO_AVCLEVELTYPE defaultAvcLevel();

        OMX_VIDEO_HEVCPROFILETYPE defaultHevcProfile();
        OMX_VIDEO_HEVCLEVELTYPE defaultHevcLevel();
        VENC_DRV_PARAM_ENC_T        mEncDrvSetting;
        VENC_DRV_PARAM_ENC_EXTRA_T  mExtraEncDrvSetting;
        void EncSettingDrvResolution(void);

        // sub functions of encoding function
        bool setDrvParamBeforeHdr(void);
        bool preEncProcess(void);
        bool setDrvParamBeforeEnc(void);
        bool setDrvParamAfterEnc(void);

        OMX_BOOL InitVideoEncodeHW();
        OMX_BOOL DeInitVideoEncodeHW(bool closefd = true);
        VAL_HANDLE_T                    mDrvHandle;

        // utility
        void DrawYUVStripeLine(OMX_U8 *pYUV);
        OMX_U32 getInputBufferSizeByFormat(OMX_VIDEO_PORTDEFINITIONTYPE video);

        VAL_UINT32_T mVencAdjustPriority;
        inline int64_t getTickCountMs()
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return (int64_t)(tv.tv_sec * 1000LL + tv.tv_usec / 1000);
        }

        // for enc setting
        OMX_VIDEO_H264FPATYPE   m3DVideoRecordMode;// for MTK S3D Support
        OMX_BOOL                mForceIFrame;
        OMX_BOOL                mIsTimeLapseMode;
        OMX_BOOL                mIsWhiteboardEffectMode;
        OMX_BOOL                mBitRateUpdated;
        OMX_BOOL                mFrameRateUpdated;
        int                     mIInterval;
        OMX_BOOL                mSetIInterval;
        OMX_U32                 mSkipFrame;
        OMX_BOOL                mDrawBlack;//for Miracast test case SIGMA 5.1.11 workaround
        bool                    mIsLivePhoto;
        OMX_BOOL                mPrependSPSPPSToIDRFrames;
        OMX_BOOL                mEnableNonRefP;

        // for color convert
        OMX_BOOL  mStoreMetaDataInBuffers;
        OMX_U8   *mCnvtBuffer;    //don't need to free
        unsigned mCnvtBufferSize;

        static const int CONVERT_MAX_BUFFER = MTK_VENC_DEFAULT_CONVERT_BUFFER_COUNT;
        uint32_t CheckOpaqueFormat(OMX_BUFFERHEADERTYPE *pInputBuf);
        int     ColorConvert(OMX_U8 *aInputBuf, OMX_U32 aInputSize, OMX_U8 *aOutputBuf, OMX_U32 aOutputSize);
        DpColorFormat dpFormatMap(VENC_DRV_YUV_FORMAT_T format);

        OmxMVAManager   *mCnvtMVAMgr;
        DpBlitStream    mBlitStream;
        // for resize YUV to meet HW or SW codec constrain
        unsigned char *mTmpColorConvertBuf;
        int mTmpColorConvertBufSize;
        VENC_DRV_YUV_FORMAT_T CheckFormatToDrv();
        //this format is in system/core/include/system/graphics.h
        uint32_t mInputMetaDataFormat;
        OMX_U32 CheckGrallocWStride(OMX_BUFFERHEADERTYPE *pInputBuf);
        OMX_U32 mGrallocWStride;
        OMX_BOOL        NeedConversion();
        int colorConvert(OMX_U8 *aInputBuf, OMX_U32 aInputSize, OMX_U8 *aOutputBuf, OMX_U32 aOutputSize);
        void dumpCCInput(OMX_U8 *aInputBuf, OMX_U32 aInputSize);
        void dumpCCOutput(OMX_U8 *aOutputBuf, OMX_U32 aOutputSize);
        int drawRGBStripe(void *handle, int imgWidth, int imgHeight, int size);
        int setYUVMVAForCC(void *va, void *mva, int stride, int uvStride, int sliceHeight,
                           uint8_t *vaArray[], uint8_t *mvaArray[], unsigned int sizeArray[],
                           int format=0);
        //color convert case:
        //meta mode with format not YV12 (meta mode YUV should always fit SW/HW codec limitation?)
        //normal mode with format not YUV and/or YUV but not fit SW/HW codec constrain

        // for mva
        OMX_BOOL mIsMCIMode;
        OMX_BOOL mIsClientLocally;
        OmxMVAManager   *mInputMVAMgr;
        OmxMVAManager   *mOutputMVAMgr;
        OMX_BOOL GetMVAFrameBuffer(OMX_U8 *aInputBuf, OMX_U32 aInputSize);
        OMX_BOOL GetMVABitstreamBuffer(OMX_U8 *aOutputBuf, OMX_U32 aOutputSize);
        OMX_BOOL GetMVAFrameandBitstreamBuffer(
        OMX_U8 *aInputBuf, OMX_U32 aInputSize, OMX_U8 *aOutputBuf, OMX_U32 aOutputSize);

        OMX_BOOL DescribeFlexibleColorFormat(DescribeColorFormatParams *params);

        OMX_BOOL mInputAllocateBuffer;
        OMX_BOOL mOutputAllocateBuffer;

        // for debug dump
        bool mDrawStripe;
        bool mDumpInputFrame;
        bool mDumpOutputFrame;
        bool mRTDumpInputFrame;
        bool mDumpColorConvertFrame;
        int mDumpCCNum;
        bool mDumpCts; // for CTS debug
        enum dumpFlag
        {
            DUMP_SECURE_INPUT_Flag = 1,
            DUMP_SECURE_TMP_IN_Flag = (1 << 1),
            DUMP_SECURE_OUTPUT_Flag = (1 << 2),
            DUMP_CC_Flag = (1 << 3),
            DUMP_YV12_Flag = (1 << 4),
        };

        int mDumpFlag;
        int mYV12Switch;//for dump
        int mYV12State;//for dump

        //scale inut frame buffer to mScaledWidth mScaledHeight then encode
        OMX_BOOL mInputScalingMode;
        OMX_U32 mMaxScaledWide;
        OMX_U32 mMaxScaledNarrow;
        OMX_U32 mScaledWidth;
        OMX_U32 mScaledHeight;
        OMX_U32 mResChangeWidth;
        OMX_U32 mResChangeHeight;

        // for Watchdog
        OMX_BOOL mIsMtklog;
        OMX_BOOL mDumpDLBS;
        OMX_BOOL mSetQP;
        OMX_U32 mQP;

        int64_t m_start_tick;
        int64_t m_end_tick;

        int64_t m_trans_start_tick;
        int64_t m_trans_end_tick;

        OMX_BOOL mIsMultiSlice;

        bool mETBDebug;
        bool mIsCrossMount;

        OMX_BOOL mAVPFEnable;
        OMX_BOOL mForceFullIFrame;
        OMX_BOOL mForceFullIFramePrependHeader;

        bool supportNV12(void);
        void setBufferBlack(OMX_U8* buffer, int size);

        int                     mIDRInterval;
        OMX_BOOL                mSetIDRInterval;
        OMX_BOOL mbYUV420FlexibleMode;

        OMX_BOOL WaitFence(OMX_U8 *mBufHdr, OMX_BOOL mWaitFence);

        OMX_TICKS mLastTimeStamp;
        OMX_U32 mSlotBitCount;
        OMX_BOOL mPrependSPSPPSToIDRFramesNotify;
        OMX_U32 mIDRIntervalinSec;
        OMX_TICKS mLastIDRTimeStamp;

        bool allowEncodeVideo(int inputIdx);
        bool mIsWeChatRecording;

#ifdef V4L2
        int mV4L2fd;
        video_encode_param mEncParam;
        int mdevice_poll_interrupt_fd;

        void PrepareOutputBuffers();
        void PrepareOutputBuffer(OMX_BUFFERHEADERTYPE *pOutputBuf);

        OMX_U8 *aFakeEosBuf;
        bool trySendStopCommand();
        void SendStopCommand(bool);

        int device_open(const char* );
        void device_close(int fd);
        int profile_id_ioctl_set_ctrl();
        int level_id_ioctl_set_ctrl();
        int CheckFormatToV4L2();
        int queryCapability(int format, int profile, int level, int width, int height);

        int ioctl_set_crop(video_encode_param *param);
        int ioctl_set_param(video_encode_param *param);
        int ioctl_set_fmt(video_encode_param *param);
        int ioctl_set_fix_scale(video_encode_param *param);

        int ioctl_set_ctrl(video_encode_param *param);
        int ioctl_req_bufs(int port = 3);
        int ioctl_clear_req_bufs(int port = 3);
        int ioctl_query_in_dmabuf();
        int ioctl_query_out_dmabuf();
        int ioctl_q_in_buf(OMX_BUFFERHEADERTYPE *pInputBuf, int idx, int dmabuf_fd, bool mod=true);
        int ioctl_q_out_buf(OMX_BUFFERHEADERTYPE *pOutputBuf, int idx, int dmabuf_fd, bool mod=true);
        int ioctl_dq_in_buf(OMX_BUFFERHEADERTYPE **ppInputBuf, OMX_U8 **ppCvtBuf=0);
        int ioctl_dq_out_buf(OMX_BUFFERHEADERTYPE **ppOutputBuf);
        int ioctl_query_in_buf(int idx, struct v4l2_buffer* buffer);
        int ioctl_query_out_buf(int idx, struct v4l2_buffer* buffer);
        int ioctl_subscribe_event(int eventtype);
        int ioctl_dq_event(struct v4l2_event* event);
        bool InputErrorHandle(bool err, OMX_BUFFERHEADERTYPE* pInputBuf);
        bool OutputErrorHandle(bool err, OMX_BUFFERHEADERTYPE* pOutputBuf);
        void CommonErrorHandle(OMX_ERRORTYPE err, OMX_BUFFERHEADERTYPE* pBuffer=NULL);
        inline int IoctlPoll(int* event, bool* input, bool* output) { return IoctlPoll(-1, event, input, output); }
        int IoctlPoll(int timeout, int* event, bool* input, bool* output);
        void SetPollInterrupt();
        int GetPollInterrupt();

        int stop_cmd_sent;
        int ioctl_send_stop_cmd();

        bool stream_on_status[2];
        int ioctl_stream_on(int port=3);
        int ioctl_stream_off(int port=3);
        int ioctl_runtime_config(int change_to_mode, int change_to_value);
        int ioctl_runtime_query(int mode, int* value);

        int Omx2V4L2H264ProfileMap(OMX_VIDEO_AVCPROFILETYPE eProfile);
        int Omx2V4L2H264LevelMap(OMX_VIDEO_AVCLEVELTYPE eLevel);
        void GenerateDummyBufferH264Enc(OMX_BUFFERHEADERTYPE* pDummyOutputBufHdr);

        int Omx2V4L2Mpeg4ProfileMap(OMX_VIDEO_MPEG4PROFILETYPE eProfile);
        int Omx2V4L2Mpeg4LevelMap(OMX_VIDEO_MPEG4LEVELTYPE eLevel);

        int Omx2V4L2HevcProfileMap(OMX_VIDEO_HEVCPROFILETYPE eProfile);
        int Omx2V4L2HevcLevelMap(OMX_VIDEO_HEVCLEVELTYPE eLevel);
#endif

        //MtkVenc::Properties properties;
        const char* indexType(OMX_INDEXTYPE index);

        #include "MtkOmxVencDeque.h"
        #include "MtkOmxVencParam.h"
        #include "MtkOmxVencConfig.h"
        #include "MtkOmxVencBuffer.h"
        #include "MtkOmxVencSMVR.h"
        #include "MtkOmxVencVT.h"
        #include "MtkOmxVencWFD.h"
        #include "MtkOmxVencRoi.h"
        #include "MtkOmxVencHeif.h"
};

#endif
