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
#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_V3_PIPELINE_MODEL_TYPEDEF_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_V3_PIPELINE_MODEL_TYPEDEF_H_

#include <mtkcam/pipeline/hwnode/NodeId.h>
#include <mtkcam/pipeline/hwnode/StreamId.h>
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/feature/eis/EisInfo.h>
#include <mtkcam/pipeline/pipeline/PipelineContext.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <vector>
#include "IPipelineModel.h"

using namespace android;
using namespace NSCam::v3::NSPipelineContext;
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

    struct Duration
    {
        MINT64 minDuration;
        MINT64 stallDuration;
        //
        void set(MINT64 min, MINT64 stall) {
            minDuration = min;
            stallDuration = stall;
        };
    };

    struct  PipeConfigParams
    {
        android::sp<IMetaStreamInfo>  pMeta_Control;
        android::sp<IImageStreamInfo> pImage_Yuv_In;
        struct Duration               mImage_Yuv_In_Duration;
        android::sp<IImageStreamInfo> pImage_Opaque_In;
        struct Duration               mImage_Opaque_In_Duration;

        android::sp<IImageStreamInfo> pImage_Opaque_Out;
        struct Duration               mImage_Opaque_Out_Duration;

        android::sp<IImageStreamInfo> pImage_Raw;
        struct Duration               mImage_Raw_Duration;

        android::sp<IImageStreamInfo> pImage_Jpeg_Stall;
        struct Duration               mImage_Jpeg_Duration;

        android::Vector <android::sp<IImageStreamInfo>>
                                      vImage_Yuv_NonStall;
        android::Vector <Duration>    vImage_Yuv_Duration;

        MUINT32                       mOperation_mode;
    };
    
    struct MyConfigParams
    {
        PipeConfigParams            configParams;
        //
        // internal setting
        MBOOL                       skipJpeg;
    };
    struct MyRecordParams
    {
        MBOOL                       hasVRConsumer           = false;
        MBOOL                       has4KVR                 = false;
        MSize                       videoSize               = MSize(-1,-1);
    };
    struct ReprocParams
    {
        MBOOL                       mbOpaqueReproc          = false;
        MBOOL                       mbYuvReproc             = false;
    };
    struct CHSvrParams
    {
        // constrained high speed video related
        MINT32                      mDefaultBusrstNum       = 1;
        MINT32                      mAeTargetFpsMin         = -1;
        MINT32                      mAeTargetFpsMax         = -1;
        MINT32                      mAeTargetFpsMin_Req     = -1;
        MBOOL                       mResetAeTargetFps_Req   = false;
    };
    enum
    {
        CONFIG_USE_MAIN1_NODE       = 0x01 << 0,
        CONFIG_USE_MAIN2_NODE       = 0x01 << 1,
        CONFIG_USE_STREAM_NODE      = 0x01 << 2,
        CONFIG_USE_CAPTURE_NODE     = 0x01 << 3,
        CONFIG_USE_FD_NODE          = 0x01 << 4,
        CONFIG_USE_JPEG_NODE        = 0x01 << 5,
        CONFIG_USE_RAW16_NODE       = 0x01 << 6
    };
    
    enum
    {
        CONFIG_HAS_RAW_STREAM       = 0x01 << 0,
        CONFIG_HAS_JPEG_STREAM      = 0x01 << 1,
        CONFIG_HAS_LCSO_STREAM      = 0x01 << 2,
    };
    struct PipelineConfigParams
    {
        // constrained high speed video related
        MUINT32                     mOperation_mode         = 0;
        // Stream infos
        MSize                       mVideoSize              = MSize(-1,-1);
        MBOOL                       mbHasRecording          = false;
        MBOOL                       mb4KRecording           = false;
        std::vector<MBOOL>          mbHasRaw;
        MBOOL                       mbHasJpeg               = false;
        MSize                       mMaxStreamSize          = MSize(-1,-1);
        std::vector<MBOOL>          mbHasLcso;
        std::vector<MBOOL>          mbHasRsso;
        // node related
        std::vector<MBOOL>          mbUseP1Node;
        MBOOL                       mbUseP2Node             = false;
        MBOOL                       mbUseP2CapNode          = false;
        MBOOL                       mbUseFDNode             = false;
        MBOOL                       mbUseJpegNode           = false;
        MBOOL                       mbUseRaw16Node          = false;
        // feature related
        MUINT32                     mVhdrMode;
        MUINT32                     mHdrMode;
        MINT32                      mForceEnableIMGO;
        
        MBOOL                       mNeedLMV;
        MUINT32                     mNr3dMode;
        MBOOL                       mbIsEIS;
        NSCam::EIS::EisInfo         mEisInfo;
        MINT32                      mEisFactor;
        MUINT32                     mEisMode;
        MUINT32                     mEisExtraBufNum;
    };
    struct PipelineStreamSet
    {
        // meta: hal
        std::vector<android::sp<IMetaStreamInfo>>    mpHalMeta_Control;
        std::vector<android::sp<IMetaStreamInfo>>    mpHalMeta_DynamicP1;
        android::sp<IMetaStreamInfo>    mpHalMeta_DynamicP2;
        android::sp<IMetaStreamInfo>    mpHalMeta_DynamicP2Capture;
        // meta: app
        std::vector<android::sp<IMetaStreamInfo>>    mpAppMeta_DynamicP1;
        android::sp<IMetaStreamInfo>    mpAppMeta_DynamicP2;
        android::sp<IMetaStreamInfo>    mpAppMeta_DynamicP2Capture;
        android::sp<IMetaStreamInfo>    mpAppMeta_DynamicFD;
        android::sp<IMetaStreamInfo>    mpAppMeta_DynamicJpeg;
        android::sp<IMetaStreamInfo>    mpAppMeta_Control;
        // image: hal
        std::vector<android::sp<IImageStreamInfo>>   mpHalImage_P1_Raw;
        std::vector<android::sp<IImageStreamInfo>>   mpHalImage_P1_ResizerRaw;
        std::vector<android::sp<IImageStreamInfo>>   mpHalImage_P1_Lcso;
        std::vector<android::sp<IImageStreamInfo>>   mpHalImage_P1_Rsso;
        android::sp<IImageStreamInfo>   mpHalImage_FD_YUV;
        android::sp<IImageStreamInfo>   mpHalImage_Jpeg_YUV;
        android::sp<IImageStreamInfo>   mpHalImage_Thumbnail_YUV;

        // image: app
        android::sp<IImageStreamInfo>   mpAppImage_Yuv_In;
        android::sp<IImageStreamInfo>   mpAppImage_Opaque_In;
        android::sp<IImageStreamInfo>   mpAppImage_Opaque_Out;
        android::KeyedVector <
                StreamId_T, android::sp<IImageStreamInfo>
                        >               mvAppYuvImage;
        android::sp<IImageStreamInfo>   mpAppImage_Jpeg;
        android::sp<IImageStreamInfo>   mpAppImage_RAW16;

        //// raw/yuv stream mapping
        StreamSet                       mvYuvStreams_Fullraw;
        StreamSet                       mvYuvStreams_Resizedraw;

        android::KeyedVector < StreamId_T, MINT64 >
                                        mvStreamDurations;
    };
    struct HwParams
    {
        // sensor
        MBOOL                       mSkipSensorDecide       = false;
        MUINT                       mSensorMode;
        MSize                       mSensorSize             = MSize(-1,-1);;
        MUINT                       mSensorFps;
        // pass1
        MBOOL                       mSkipP1Decide           = false;
        MUINT32                     mPixelMode;
        MINT                        mFullrawFormat;
        MSize                       mFullrawSize;
        size_t                      mFullrawStride;
        MINT                        mResizedrawFormat;
        MSize                       mResizedrawSize;
        size_t                      mResizedrawStride;
    };
    //
    class MyProcessedParams
    {
    public:
        MINT32 const                mOpenId;
        std::vector<MINT32>         mSensorId;
        MINT32                      mLogEn;
        /*  input params  */
        PipeConfigParams            mConfigParams;
        // internal setting
        MBOOL                       mSkipJpeg;
        MBOOL                       mIsFirstId;
        MBOOL                       mIsDual;
        MBOOL                       mSupportJpegRot;
        //
        /*  processed params  */
        // opaque/yuv reprocessing
        ReprocParams                mReprocParams;
        //
        // constrained high speed video related
        CHSvrParams                 mCHSvrParams;
        //
        // Stream / Node related
        PipelineConfigParams        mPipelineConfigParams;
        //
        // streaminfo set
        PipelineStreamSet           mStreamSet;
        //
        // sensor / pass1 hw related
        std::vector<std::shared_ptr<HwParams>>                    mHwParams;
        //
        // pipeline setting related
        //PipelineSettingParams         mPipelineSettingParams;

        //
        std::vector<std::shared_ptr<NSCamHW::HwInfoHelper>>       mHwInfoHelper;

    public:
                                    MyProcessedParams(MINT32 const openId);
        virtual                     ~MyProcessedParams() {}
        MERROR                      update(
                                        PipeConfigParams const& rParams,
                                        MBOOL skipJpeg
                                        )
                                    {
                                        // update config params
                                        mConfigParams = rParams;
                                        mSkipJpeg     = skipJpeg; //TODO: use this?
                                        //
                                        return OK;
                                    }
        //
        MyRecordParams       genRecordParam( PipeConfigParams const& configParams );
        //
        MBOOL                       isSupportAdvP2() const;
        //
        virtual MERROR              querySensorStatics();
        virtual MERROR              preprocess();
        virtual MERROR              decideSensor();
        virtual MERROR              decideP1();
    protected:
        
        virtual MBOOL               isNeedLcso();
        virtual MBOOL               isNeedRsso(MUINT32 eisMode) const;
        virtual MUINT32             getMinRrzoEisW();
        virtual MUINT32             getEisRatio100X() const;
        virtual MUINT32             getEisMode() const;
    };

    struct parsedAppRequest
    {
        // original AppRequest
        IPipelineModel::AppRequest* pRequest;
        MUINT32                     requestNo;
        
        // in
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vIImageInfos_Raw;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vIImageInfos_Opaque;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vIImageInfos_Yuv;
        // out
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Raw;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Opaque;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Yuv;
        KeyedVector< StreamId_T, sp<IImageStreamInfo> >
                                    vOImageInfos_Jpeg;
        //
                                    parsedAppRequest()
                                    {}
                                    ~parsedAppRequest() {}
    };

    struct evaluateRequestResult
    {
        struct reprocessInfo
        {
            bool useYuvIn     = false;
            bool useOpaqueIn  = false;
            bool useOpaqueOut = false;
        };
        
        enum ePath
        {
            eP1R
        };
        struct hwnodeInfo
        {
            bool                                    needFDNode = false;
            bool                                    needJpegNode = false;
            bool                                    needRaw16Node = false;
            bool                                    needStreamNode = false;
            bool                                    needCaptureNode = false;
            bool                                    onlyCaptureNode = false;
            std::vector<bool>                       vhasLcso;
            std::vector<bool>                       vhasRsso;
        };
        
        enum eP1RawType
        {
            eP1RawType_Main1_Rrzo   = 0x01 << 0,
            eP1RawType_Main1_Imgo   = 0x01 << 1,
            eP1RawType_Main1_Lcso   = 0x01 << 2,
            eP1RawType_Main1_Rsso   = 0x01 << 3,
            eP1RawType_Main2_Rrzo   = 0x01 << 4,
            eP1RawType_Main2_Imgo   = 0x01 << 5,
            eP1RawType_Main2_Lcso   = 0x01 << 6,
            eP1RawType_Main2_Rsso   = 0x01 << 7,
            //
            eP1RawType_Main1_AllOuput = 0x0F,
            eP1RawType_Main2_AllOuput = 0xF0
        };
        
        struct SettingResult
        {
            MINT32     RequestCount   = 1;
            IOMapSet   P2IO;
            IMetadata* AddtionalApp;
            IMetadata* AddtionalHal;
            int        P1RawMap = 0;
        };

        DefaultKeyedVector<
            StreamId_T,
            sp<IImageStreamInfo>
                >                               vUpdatedImageInfos;
        //
        NodeSet                                 roots;
        NodeEdgeSet                             edges;
        bool                                    isTSflow = false;
        bool                                    forceFullRaw = false;
        bool                                    isZsdCap = false;
        hwnodeInfo                              nodeInfos;
        reprocessInfo                           reprocInfos;
        
        std::vector<struct SettingResult>       vPrvSettingResult;
        std::vector<struct SettingResult>       vCapSettingResult;
        sp<IImageStreamInfo>                    pHalImage_Jpeg_YUV;
        sp<IImageStreamInfo>                    pHalImage_Thumbnail_YUV;
        //
        DefaultKeyedVector<NodeId_T, IOMapSet>  nodeIOMapImage;
        DefaultKeyedVector<NodeId_T, IOMapSet>  nodeIOMapMeta;
        //
        DefaultKeyedVector<StreamId_T, sp<IImageStreamBuffer> >
                                                vAppImageBuffers;
        DefaultKeyedVector<StreamId_T, sp<NSCam::v3::Utils::HalImageStreamBuffer> >
                                                vHalImageBuffers;
        DefaultKeyedVector<StreamId_T, sp<IMetaStreamBuffer> >
                                                vAppMetaBuffers;
        DefaultKeyedVector<StreamId_T, sp<NSCam::v3::Utils::HalMetaStreamBuffer> >
                                                vHalMetaBuffers;

        MBOOL                                   hasEncOut = MFALSE;
        MBOOL                                   isRepeating = MFALSE;
        //
        bool                                    isOpaqueReprocOut()
                                                { return reprocInfos.useOpaqueOut; }
        bool                                    isOpaqueReprocIn()
                                                { return reprocInfos.useOpaqueIn; }
        bool                                    isYuvReprocIn()
                                                { return reprocInfos.useYuvIn; }
    };

    struct evaluateSubRequestResult
    {
        MUINT32 subRequetNumber;
        //
        Vector<evaluateRequestResult> subRequestList;
        //
        evaluateSubRequestResult()
            : subRequetNumber(0)
        {}
    };
}
}
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_V3_PIPELINE_MODEL_TYPEDEF_H_
