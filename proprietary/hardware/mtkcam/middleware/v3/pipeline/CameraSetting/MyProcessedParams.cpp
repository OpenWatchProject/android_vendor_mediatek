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

#define LOG_TAG "MtkCam/MyProcessedParams"
//
#include "PipelineDefaultImp.h"
#include "PipelineUtility.h"
//
//#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/utils/hw/IScenarioControl.h>
#include <mtkcam/utils/hw/HwTransform.h>
//
#include <cutils/compiler.h>
//
#include <camera_custom_eis.h>
#if MTKCAM_HAVE_ADV_SETTING
#include <mtkcam/feature/3dnr/3dnr_defs.h>
using namespace NSCam::NR3D;
#endif // MTKCAM_HAVE_ADV_SETTING

using namespace android;
using namespace NSCam;
using namespace NSCam::EIS;
using namespace NSCam::v3;
using namespace NSCam::Utils;
using namespace NSCam::v3::Utils;
//using namespace NSCam::v3::CameraSetting;

#define DEFAULT_LOG_EN (1)

class my_scoped_tracer
{
/******************************************************************************
 *
 ******************************************************************************/
public:
    my_scoped_tracer(MINT32 openId, const char* functionName, MINT32 log)
    : mOpenId(openId)
    , mFunctionName(functionName)
    , mLogEn(log)
    {
        MY_LOGI_IF(mLogEn, "id:%d[%s] +", mOpenId, mFunctionName);
    }
    ~my_scoped_tracer()
    {
        MY_LOGI_IF(mLogEn, "id:%d[%s] -", mOpenId, mFunctionName);
    }
private:
    const MINT32        mOpenId;
    const char* const   mFunctionName;
    const MINT32        mLogEn;
};
#define MY_SCOPED_TRACER(Log) my_scoped_tracer ___scoped_tracer(mOpenId, __FUNCTION__, Log);


/******************************************************************************
 *
 ******************************************************************************/
MyProcessedParams::
MyProcessedParams(MINT32 const openId)
    : mOpenId(openId)
    //
    , mConfigParams()
    , mSkipJpeg(MFALSE)
    //
    , mReprocParams()
    , mCHSvrParams()
    , mPipelineConfigParams()
    , mHwParams()
    //
{
    mPipelineConfigParams.mForceEnableIMGO = property_get_int32("debug.feature.forceEnableIMGO", 0);
    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    mSensorId = pHalDeviceList->getSensorId(openId);
    mLogEn = ::property_get_int32("debug.camera.log.camsetting", DEFAULT_LOG_EN);
    mIsDual = 0;
    MY_LOGD("OpenId : %d", openId);
    for (MINT32 i = 0; i < mSensorId.size(); i++)
    {
        std::shared_ptr<NSCamHW::HwInfoHelper> helper = std::make_shared<NSCamHW::HwInfoHelper>(mSensorId[i]);
        std::shared_ptr<HwParams> temp = std::make_shared<HwParams>();
        mHwInfoHelper.push_back(helper);
        mHwParams.push_back(temp);
        MY_LOGD("i : %d, mSensorId : %d", i, mSensorId[i]);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
MyProcessedParams::
querySensorStatics()
{
    MY_SCOPED_TRACER(mLogEn);
    for (MUINT32 idx = 0; idx < mHwInfoHelper.size(); idx++)
    {
        if ( ! mHwInfoHelper[idx]->updateInfos() )
        {
            MY_LOGE("cannot properly update infos");
                return DEAD_OBJECT;
        }
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MyRecordParams
MyProcessedParams::
genRecordParam(PipeConfigParams const& configParams)
{
    MY_SCOPED_TRACER(mLogEn);
    MyRecordParams recordParam{MFALSE, MFALSE, MSize()};
#define hasUsage(flag, usage) ((flag & usage) == usage)
    for (size_t i = 0; i < configParams.vImage_Yuv_NonStall.size(); i++) {
        if  ( hasUsage(
                    configParams.vImage_Yuv_NonStall[i]->getUsageForConsumer(),
                    GRALLOC_USAGE_HW_VIDEO_ENCODER
                    ) )
        {
            recordParam.hasVRConsumer = MTRUE;
            recordParam.videoSize = configParams.vImage_Yuv_NonStall[i]->getImgSize();
            recordParam.has4KVR = ( recordParam.videoSize.w*recordParam.videoSize.h > 8000000 )? MTRUE : MFALSE;
            break;
        }
    }
#undef hasUsage
    return recordParam;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MyProcessedParams::
preprocess()
{
    MY_SCOPED_TRACER(mLogEn);
    MyRecordParams recordParam = genRecordParam(mConfigParams);
    //
    
    MSize maxStreamSize;
    
    mPipelineConfigParams.mbUseP1Node.clear();
    mPipelineConfigParams.mbHasRsso.clear();
    mPipelineConfigParams.mbHasRaw.clear();
    mPipelineConfigParams.mbHasLcso.clear();
    {
        struct Log
        {
            static String8
                skippedStream(IImageStreamInfo* pStreamInfo)
                {
                    return String8::format(
                            "skipped stream - format:0x%x type:%x size:%dx%d",
                            pStreamInfo->getImgFormat(), pStreamInfo->getStreamType(),
                            pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h
                            );
                }

            static String8
                candidateStream(IImageStreamInfo* pStreamInfo)
                {
                    return String8::format(
                            "candidate stream - format:0x%x type:%x size:%dx%d",
                            pStreamInfo->getImgFormat(), pStreamInfo->getStreamType(),
                            pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h
                            );
                }
        };
        //
        if  ( IImageStreamInfo* pStreamInfo = mConfigParams.pImage_Raw.get() ) {
            if  ( pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_IN ) {
                MY_LOGD("%s", Log::skippedStream(pStreamInfo).string());
            }
            else {
                MY_LOGD("%s", Log::candidateStream(pStreamInfo).string());
                maxStreamSize = pStreamInfo->getImgSize();
            }
        }
        //
        if  ( IImageStreamInfo* pStreamInfo = mConfigParams.pImage_Jpeg_Stall.get() ) {
            MY_LOGD("%s", Log::candidateStream(pStreamInfo).string());
            MY_LOGD("jpeg size : %dx%d", pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h);
            if  ( maxStreamSize.size() <= pStreamInfo->getImgSize().size() ) {
                maxStreamSize = pStreamInfo->getImgSize();
            }
        }
        //
        for (size_t i = 0; i < mConfigParams.vImage_Yuv_NonStall.size(); i++) {
            if  ( IImageStreamInfo* pStreamInfo = mConfigParams.vImage_Yuv_NonStall[i].get()) {
                MY_LOGD("%s", Log::candidateStream(pStreamInfo).string());
                if  ( maxStreamSize.size() <= pStreamInfo->getImgSize().size()) {
                    maxStreamSize = pStreamInfo->getImgSize();
                }
            }
        }
    }
    //
    // update processed params
    mPipelineConfigParams.mbHasRecording = recordParam.hasVRConsumer;
    mPipelineConfigParams.mVideoSize     = (mPipelineConfigParams.mbHasRecording)? recordParam.videoSize : MSize();
    mPipelineConfigParams.mb4KRecording  = recordParam.has4KVR;
    mReprocParams.mbOpaqueReproc         = ( mConfigParams.pImage_Opaque_Out.get() ||
                       mConfigParams.pImage_Opaque_In.get() ) ? MTRUE : MFALSE;
    mReprocParams.mbYuvReproc            = mConfigParams.pImage_Yuv_In.get() ? MTRUE : MFALSE;
    mPipelineConfigParams.mbHasJpeg      = mConfigParams.pImage_Jpeg_Stall.get() ? MTRUE : MFALSE;
    mPipelineConfigParams.mMaxStreamSize = maxStreamSize;
    // constrained high speed video
    mPipelineConfigParams.mOperation_mode= mConfigParams.mOperation_mode;
    if ( mPipelineConfigParams.mOperation_mode )
    {
        android::sp<IMetadataProvider const>
            pMetadataProvider = NSMetadataProviderManager::valueFor(mOpenId);
        IMetadata::IEntry const& entry = pMetadataProvider->getMtkStaticCharacteristics()
                                         .entryFor(MTK_CONTROL_AVAILABLE_HIGH_SPEED_VIDEO_CONFIGURATIONS);
        if  ( entry.isEmpty() ) {
            MY_LOGW("no static MTK_CONTROL_AVAILABLE_HIGH_SPEED_VIDEO_CONFIGURATIONS");
            mCHSvrParams.mDefaultBusrstNum = 1;
        }
        else {
            // [width, height, fps_min, fps_max, batch_size]
            MBOOL hit = MFALSE;
            for ( size_t i=0; i<entry.count(); i+=5 )
            {
                MY_LOGI_IF( 1, "[width(%d), height(%d), fps_min(%d), fps_max(%d), batch_size(%d)]",
                            entry.itemAt(i  , Type2Type<MINT32>()), entry.itemAt(i+1, Type2Type<MINT32>()),
                            entry.itemAt(i+2, Type2Type<MINT32>()), entry.itemAt(i+3, Type2Type<MINT32>()),
                            entry.itemAt(i+4, Type2Type<MINT32>()) );
                if ( recordParam.videoSize.w == entry.itemAt(i, Type2Type<MINT32>()) &&
                     recordParam.videoSize.h == entry.itemAt(i+1, Type2Type<MINT32>()) )
                {
                    if ( mCHSvrParams.mResetAeTargetFps_Req )
                    {
                        mCHSvrParams.mAeTargetFpsMin   = mCHSvrParams.mAeTargetFpsMin_Req;
                        mCHSvrParams.mResetAeTargetFps_Req = MFALSE;
                    }
                    else
                        mCHSvrParams.mAeTargetFpsMin   = entry.itemAt(i+2, Type2Type<MINT32>());
                    mCHSvrParams.mAeTargetFpsMax   = entry.itemAt(i+3, Type2Type<MINT32>());
                    mCHSvrParams.mDefaultBusrstNum = entry.itemAt(i+4, Type2Type<MINT32>());
                    hit = MTRUE;
                    break;
                }
            }
            if ( !hit )
                MY_LOGW("no matching high speed profile(%dx%d)", recordParam.videoSize.w, recordParam.videoSize.h);
        }
    }
    MY_LOGD("max stream(%d, %d), jpeg(%d), hasRecording(%d), operation_mode(%d):burst(%d)",
             mPipelineConfigParams.mMaxStreamSize.w, mPipelineConfigParams.mMaxStreamSize.h, mPipelineConfigParams.mbHasJpeg,
             mPipelineConfigParams.mbHasRecording, mPipelineConfigParams.mOperation_mode,
             mCHSvrParams.mDefaultBusrstNum );
    //
    mPipelineConfigParams.mbUseP1Node.push_back(MTRUE);
    mPipelineConfigParams.mbUseP2Node    = MTRUE;
    mPipelineConfigParams.mbUseP2CapNode  = mPipelineConfigParams.mbHasJpeg;
    mPipelineConfigParams.mbUseFDNode    = (!mPipelineConfigParams.mOperation_mode)? MTRUE : MFALSE;
    mPipelineConfigParams.mbUseJpegNode  = mPipelineConfigParams.mbHasJpeg;
    //
    // set feature
    mPipelineConfigParams.mbIsEIS     = 0;//( mPipelineConfigParams.mbHasRecording && !mPipelineConfigParams.mOperation_mode )? MTRUE : MFALSE;
    mPipelineConfigParams.mEisInfo.mode   = mPipelineConfigParams.mbIsEIS ? getEisMode() : EIS_MODE_OFF;
    mPipelineConfigParams.mEisInfo.factor = mPipelineConfigParams.mbIsEIS ? getEisRatio100X() : 100;
    mPipelineConfigParams.mEisInfo.videoConfig = mPipelineConfigParams.mb4KRecording ? EISCustom::VIDEO_CFG_4K2K : EISCustom::VIDEO_CFG_FHD;
    if(EIS_MODE_IS_EIS_QUEUE_ENABLED(mPipelineConfigParams.mEisInfo.mode))
    {
        mPipelineConfigParams.mEisInfo.queueSize= EISCustom::getForwardFrames(mPipelineConfigParams.mEisInfo.videoConfig);
        mPipelineConfigParams.mEisInfo.startFrame = EISCustom::getForwardStartFrame();
    }
    // config p1 specific raw`
    mPipelineConfigParams.mbHasRsso.push_back(isNeedRsso(mPipelineConfigParams.mEisInfo.mode));
    mPipelineConfigParams.mbHasRaw.push_back(mConfigParams.pImage_Raw.get() ? MTRUE : MFALSE);
    mPipelineConfigParams.mbHasLcso.push_back(isNeedLcso());
    for (int i = 1; i < mSensorId.size(); i++)
    {
        mPipelineConfigParams.mbUseP1Node.push_back(MTRUE);
        mPipelineConfigParams.mbHasRsso.push_back(MFALSE);
        mPipelineConfigParams.mbHasRaw.push_back(MFALSE);
        mPipelineConfigParams.mbHasLcso.push_back(MFALSE);
    }
    mPipelineConfigParams.mbUseRaw16Node = mPipelineConfigParams.mbHasRaw[0]
                                        && (eImgFmt_RAW16 == mConfigParams.pImage_Raw->getImgFormat());
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MyProcessedParams::
decideSensor()
{
    MY_SCOPED_TRACER(mLogEn);
    struct sensormodeHelper
    {
        // use enum to select sensor mode if have preferred sensor mode.
        enum
        {
            eNORMAL_PREVIEW = 0,
            eNORMAL_VIDEO,
            eNORMAL_CAPTURE,
            eSLIM_VIDEO1,
            eSLIM_VIDEO2,
            eNUM_SENSOR_MODE,
        };
        //
                    sensormodeHelper( MyProcessedParams* rpParams, MUINT32 idx)
                        : sensorFps{0}
                        , sensorMode{0}
                        , selectIdx(-1)
                        , supportPrevMode(true)
                        , pParams(rpParams)
                        , sensorIdx(idx)
                    {
                        {
                            if ( pParams->mPipelineConfigParams.mVhdrMode != SENSOR_VHDR_MODE_NONE )
                            {
                                // Currently  only need to check sensor scenario preview
                                // If preview not support, directly use video
                                MUINT32 supportHDRMode = 0;
                                if (! rpParams->mHwInfoHelper[sensorIdx]->querySupportVHDRMode(SENSOR_SCENARIO_ID_NORMAL_PREVIEW, supportHDRMode)){
                                    MY_LOGE("helper.querySupportVHDRMode Failed!");
                                    return;
                                }
                                if(supportHDRMode != pParams->mPipelineConfigParams.mVhdrMode){
                                    MY_LOGD("sensor preview mode not support vhdr, change to use video mode if select preview mode");
                                    supportPrevMode = false;
                                }
                            }
                        }
                    #define addMode(idx, _scenarioId_, _key_)                                                   \
                        do {                                                                                    \
                            MBOOL bResult = MTRUE;                                                              \
                            bResult = rpParams->mHwInfoHelper[sensorIdx]->getSensorSize(_scenarioId_, sensorSize[idx]);     \
                            if (CC_UNLIKELY(bResult == MFALSE)) {                                               \
                                MY_LOGD("getSensorSize failed"); break;                                         \
                            }                                                                                   \
                            bResult = rpParams->mHwInfoHelper[sensorIdx]->getSensorFps (_scenarioId_, sensorFps[idx] );     \
                            if (CC_UNLIKELY(bResult == MFALSE)) {                                               \
                                MY_LOGD("getSensorFps failed"); break;                                          \
                            }                                                                                   \
                            sensorMode[idx] = _scenarioId_;                                             \
                            MY_LOGD("candidate mode %d, size(%d, %d)@%d", idx, sensorSize[idx].w,       \
                                    sensorSize[idx].h, sensorFps[idx]);                                 \
                        } while(0)
                        addMode(eNORMAL_PREVIEW, SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview);
                        addMode(eNORMAL_VIDEO  , SENSOR_SCENARIO_ID_NORMAL_VIDEO  , video);
                        addMode(eNORMAL_CAPTURE, SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture);
                        addMode(eSLIM_VIDEO1,    SENSOR_SCENARIO_ID_SLIM_VIDEO1, video1);
                        addMode(eSLIM_VIDEO2,    SENSOR_SCENARIO_ID_SLIM_VIDEO2, video2);
                    #undef addMode
                    //
                    #if 1 // preview/video mode, accept by FOV verification
                        NSCamHW::HwTransHelper helper(rpParams->mOpenId);
                        MBOOL acceptPrv = verifyFov(helper, eNORMAL_PREVIEW);
                        // force video mode for 4K recording
                        MBOOL acceptVdo = verifyFov(helper, eNORMAL_VIDEO) |
                                          rpParams->mPipelineConfigParams.mb4KRecording | rpParams->mPipelineConfigParams.mOperation_mode;
                        // MBOOL acceptVdo = verifyFov(helper, eNORMAL_VIDEO);
                    #define altMode(src, dst)                               \
                        do {                                                \
                            sensorSize[src] = sensorSize[dst];              \
                            sensorFps [src] = sensorFps [dst];              \
                            sensorMode[src] = sensorMode[dst];              \
                            MY_LOGD("alt candidate mode %d=>%d", src, dst); \
                        } while(0)
                        if (!acceptPrv) {
                            if (!acceptVdo) {
                                altMode(eNORMAL_VIDEO  , eNORMAL_CAPTURE);
                                altMode(eNORMAL_PREVIEW, eNORMAL_CAPTURE);
                            } else {
                                altMode(eNORMAL_PREVIEW, eNORMAL_VIDEO  );
                            }
                        } else if (!acceptVdo) {
                            altMode(eNORMAL_VIDEO,
                                (sensorFps[eNORMAL_CAPTURE] >= 30 ?
                                eNORMAL_CAPTURE : (supportPrevMode) ? eNORMAL_PREVIEW : eNORMAL_VIDEO));
                        }
                        // force skip video mode if no GRALLOC_USAGE_HW_VIDEO_ENCODER
                        if (!pParams->mPipelineConfigParams.mbHasRecording)
                        {
                            altMode(eNORMAL_VIDEO  , eNORMAL_CAPTURE);
                        }
                    #undef altMode
                    #endif
                    };
                    ~sensormodeHelper()
                    {
                        if( selectIdx != -1 ) {
                            pParams->mHwParams[sensorIdx]->mSensorMode = sensorMode[selectIdx];
                            pParams->mHwParams[sensorIdx]->mSensorSize = sensorSize[selectIdx];
                            pParams->mHwParams[sensorIdx]->mSensorFps  = sensorFps [selectIdx];
                            //
                            refineFps_MultiOpen(pParams->mHwParams[sensorIdx]->mSensorMode, pParams->mHwParams[sensorIdx]->mSensorFps);
                            //
                            MY_LOGD("select mode %d, size(%d, %d)@%d vhdr mode(%d)",
                                    pParams->mHwParams[sensorIdx]->mSensorMode,
                                    pParams->mHwParams[sensorIdx]->mSensorSize.w, pParams->mHwParams[sensorIdx]->mSensorSize.h,
                                    pParams->mHwParams[sensorIdx]->mSensorFps, pParams->mPipelineConfigParams.mVhdrMode
                                   );
                        } else {
                            MY_LOGW("sensor mode is not selected!");
                            for( int i = 0; i < eNUM_SENSOR_MODE; i++ ) {
                                MY_LOGD("mode %d, size(%d, %d)@%d",
                                        sensorMode[i],
                                        sensorSize[i].w, sensorSize[i].h,
                                        sensorFps[i]
                                       );
                            }
                        }
                    }
        MBOOL       verifyFov(NSCamHW::HwTransHelper helper, MUINT const mode)
                    {
                        #define FOV_DIFF_TOLERANCE (0.002)
                        float dX = 0.0f;
                        float dY = 0.0f;
                        return (helper.calculateFovDifference(
                            sensorMode[mode], &dX, &dY) &&
                            dX < FOV_DIFF_TOLERANCE && dY < FOV_DIFF_TOLERANCE)
                            ? MTRUE : MFALSE;
                    };
        MVOID       refineFps_MultiOpen(MUINT const /*mode*/, MUINT& /*fps*/)
                    {}
                    //
        MSize                       sensorSize[eNUM_SENSOR_MODE];
        MINT32                      sensorFps [eNUM_SENSOR_MODE];
        MUINT                       sensorMode[eNUM_SENSOR_MODE];
        //
        int                         selectIdx;
        bool                        supportPrevMode;
        MyProcessedParams* const    pParams;
        MUINT32                     sensorIdx;
    };
    //
    for (MUINT32 idx = 0; idx < mSensorId.size(); idx++)
    {
        struct sensormodeHelper aHelper(this, idx);
        // 1. Raw stream configured: find sensor mode with raw size.
        if  ( IImageStreamInfo* pStreamInfo = mConfigParams.pImage_Raw.get() ) {
            bool hit = false;
            for (int i = 0; i < sensormodeHelper::eNUM_SENSOR_MODE; i++) {
                if  (pStreamInfo->getImgSize() == aHelper.sensorSize[i]) {
                    aHelper.selectIdx = i;
                    hit = true;
                    break;
                }
            }
            if( !hit ) {
                MY_LOGE("Can't find sesnor size that equals to raw size");
                return UNKNOWN_ERROR;
            }
        }
        // 2. if has VR consumer: sensor video mode is preferred
        // if ( 1 ) {
        MY_LOGI_IF( mPipelineConfigParams.mOperation_mode, "mAeTargetFpsMin(%d) sensorMode(%d).fps(%d)",
                    mCHSvrParams.mAeTargetFpsMin, sensormodeHelper::eSLIM_VIDEO1, aHelper.sensorFps[sensormodeHelper::eSLIM_VIDEO1]);
        if ( mPipelineConfigParams.mOperation_mode==IPipelineModelMgr::OperationMode::C_HIGH_SPEED_VIDEO_MODE &&
            mPipelineConfigParams.mbHasRecording && mCHSvrParams.mAeTargetFpsMin==aHelper.sensorFps[sensormodeHelper::eSLIM_VIDEO1] ) {
            aHelper.selectIdx = sensormodeHelper::eSLIM_VIDEO1;
        }
        else if ( mPipelineConfigParams.mbHasRecording ) {
            aHelper.selectIdx = sensormodeHelper::eNORMAL_VIDEO;
        }
        else {
            //policy:
            //    find the smallest size that is "larger" than max of stream size
            //    (not the smallest difference)
            bool hit = false;
            for (int i = 0; i < sensormodeHelper::eNUM_SENSOR_MODE; i++) {
                if  ( mPipelineConfigParams.mMaxStreamSize.w <= aHelper.sensorSize[i].w &&
                      mPipelineConfigParams.mMaxStreamSize.h <= aHelper.sensorSize[i].h )
                {
                    aHelper.selectIdx = i;
                    hit = true;
                    break;
                }
            }
            if( !hit ) {
                // pick largest one
                MY_LOGW("select capture mode");
                aHelper.selectIdx = sensormodeHelper::eNORMAL_CAPTURE;
            }
        }
        //
        // Change sensor mode if preview mode is selected but not support
        if(aHelper.selectIdx == sensormodeHelper::eNORMAL_PREVIEW && ! aHelper.supportPrevMode)
            aHelper.selectIdx = sensormodeHelper::eNORMAL_VIDEO;
    }
    return OK;
};

/******************************************************************************
 *
 ******************************************************************************/
MERROR
MyProcessedParams::
decideP1()
{
    MY_SCOPED_TRACER(mLogEn);
    struct refine
    {
        static
            MVOID       not_larger_than(MSize& size, MSize const& limit) {
                if( size.w > limit.w ) size.w = limit.w;
                if( size.h > limit.h ) size.h = limit.h;
            }
        static
            MVOID       not_smaller_than(MSize& size, MSize const& limit) {
                if( size.w < limit.w ) size.w = limit.w;
                if( size.h < limit.h ) size.h = limit.h;
            }
        static
            MSize       align_2(MSize const& size) {
#define align2(x)  ((x+1) & (~1))
                return MSize(align2(size.w), align2(size.h));
#undef align2
            }
        static
            MSize       scale_roundup(MSize const& size, int mul, int div) {
                return MSize((size.w * mul + div - 1) / div, (size.h * mul + div - 1) / div);
            }
    };
    for (MUINT32 idx = 0; idx < mSensorId.size(); idx++)
    {
        //
        if ( ! mHwInfoHelper[idx]->queryPixelMode( mHwParams[idx]->mSensorMode, mHwParams[idx]->mSensorFps, mHwParams[idx]->mPixelMode ) )
        {
            MY_LOGE("idx : %d, queryPixelMode error", idx);
            return UNKNOWN_ERROR;
        }
        //
        MSize const sensorSize = mHwParams[idx]->mSensorSize;
        //
    #if 1
    #define MAX_PREVIEW_W           (2560)
    #else
    #define MAX_PREVIEW_W           (1920)
    #endif
    #define MIN_RRZO_RATIO_100X     (25)

    #define CHECK_TARGET_SIZE(_msg_, _size_) \
            MY_LOGD_IF(mLogEn, "%s: target size(%dx%d)", _msg_, _size_.w, _size_.h);

        // estimate preview yuv max size
        MSize const max_preview_size = refine::align_2(
                MSize(MAX_PREVIEW_W, MAX_PREVIEW_W * sensorSize.h / sensorSize.w));
        //
        MSize maxYuvStreamSize;
        MSize largeYuvStreamSize;
        for (size_t i = 0; i < mConfigParams.vImage_Yuv_NonStall.size(); i++ )
        {
            MSize const streamSize = mConfigParams.vImage_Yuv_NonStall[i]->getImgSize();
            // if stream's size is suitable to use rrzo
            if( streamSize.w <= max_preview_size.w && streamSize.h <= max_preview_size.h )
                refine::not_smaller_than(maxYuvStreamSize, streamSize);
            else
                refine::not_smaller_than(largeYuvStreamSize, streamSize);
        }
        MY_LOGD_IF( !!maxYuvStreamSize, "max yuv stream size(%dx%d)",
                    maxYuvStreamSize.w, maxYuvStreamSize.h);
        MY_LOGD_IF( !!largeYuvStreamSize, "large yuv stream size(%dx%d), burst capture",
                    largeYuvStreamSize.w, largeYuvStreamSize.h );

        // use resized raw if
        // 1. raw sensor
        // 2. some streams need this
        if( mHwInfoHelper[idx]->isRaw() )
        {
            //
            // currently, should always enable resized raw due to some reasons...
            //
            // initial value
            MSize target_rrzo_size = ( !mIsDual && mPipelineConfigParams.mb4KRecording )?
                                     largeYuvStreamSize : maxYuvStreamSize;
            CHECK_TARGET_SIZE("max yuv stream", target_rrzo_size);
            // apply limitations
            //  1. lower bounds
            {
                // get eis ownership and apply eis hw limitation
                if ( mPipelineConfigParams.mbIsEIS ) {
                    MUINT32 minRrzoEisW = getMinRrzoEisW();
                    MSize const min_rrzo_eis_size = refine::align_2(
                            MSize(minRrzoEisW, minRrzoEisW * sensorSize.h / sensorSize.w));
                    refine::not_smaller_than(target_rrzo_size, min_rrzo_eis_size);
                    target_rrzo_size = refine::align_2(
                            refine::scale_roundup(target_rrzo_size, mPipelineConfigParams.mEisInfo.factor, 100)
                            );
                   CHECK_TARGET_SIZE("eis lower bound limitation", target_rrzo_size);
                }
                MSize const min_rrzo_hw_size = refine::align_2(
                        MSize(sensorSize.w*MIN_RRZO_RATIO_100X/100, sensorSize.h*MIN_RRZO_RATIO_100X/100) );
                refine::not_smaller_than(target_rrzo_size, min_rrzo_hw_size);
                CHECK_TARGET_SIZE("rrz hw lower bound limitation", target_rrzo_size);
            }
            //  2. upper bounds
            {
                if ( !mPipelineConfigParams.mb4KRecording )
                {
                    refine::not_larger_than(target_rrzo_size, max_preview_size);
                    CHECK_TARGET_SIZE("preview upper bound limitation", target_rrzo_size);
                }
                refine::not_larger_than(target_rrzo_size, sensorSize);
                CHECK_TARGET_SIZE("sensor size upper bound limitation", target_rrzo_size);
            }
            // 3. dual mode prevent 2-pixel mode.
            if ( mHwParams[idx]->mPixelMode!=ONE_PIXEL_MODE && mIsDual )
            {
                MY_LOGD_IF( 1, "pixel mode(%d) sensor(%dx%d)",
                            mHwParams[idx]->mPixelMode, sensorSize.w, sensorSize.h);
    #define GET_MAX_RRZO_W(_x)      ( (_x)/2 - 32 )
                MSize const max_2pixel_bin_rrzo_size = refine::align_2(
                        MSize( GET_MAX_RRZO_W(sensorSize.w),
                               GET_MAX_RRZO_W(sensorSize.w)*sensorSize.h/sensorSize.w )
                        );
    #undef GET_MAX_RRZO_W
                refine::not_larger_than(target_rrzo_size, max_2pixel_bin_rrzo_size);
                CHECK_TARGET_SIZE("2-pixel bin upper bound limitation", target_rrzo_size);
            }
            MY_LOGD_IF(1, "rrzo size(%dx%d)", target_rrzo_size.w, target_rrzo_size.h);
            //
            mHwParams[idx]->mResizedrawSize = target_rrzo_size;
            // check hw limitation with pixel mode & stride
            if( ! mHwInfoHelper[idx]->getRrzoFmt(10, mHwParams[idx]->mResizedrawFormat) ||
                ! mHwInfoHelper[idx]->alignRrzoHwLimitation(target_rrzo_size, mHwParams[idx]->mSensorSize, mHwParams[idx]->mResizedrawSize) ||
                ! mHwInfoHelper[idx]->alignPass1HwLimitation( mHwParams[idx]->mPixelMode, mHwParams[idx]->mResizedrawFormat,
                                                        MFALSE, mHwParams[idx]->mResizedrawSize, mHwParams[idx]->mResizedrawStride ) )
            {
                MY_LOGE("wrong params about rrzo");
                return BAD_VALUE;
            }
            MY_LOGI_IF( 1, "rrzo size(%dx%d) stride %zu",
                        mHwParams[idx]->mResizedrawSize.w, mHwParams[idx]->mResizedrawSize.h, mHwParams[idx]->mResizedrawStride);
        }
    #undef CHECK_TARGET_SIZE
        //
        // use full raw, if
        // 1. jpeg stream (&& not met BW limit)
        // 2. raw stream
        // 3. opaque stream
        // 4. or stream's size is beyond rrzo's limit
        MBOOL useImgo =
            (mPipelineConfigParams.mbHasJpeg /*&& ! mPipelineConfigParams.mbHasRecording*/) ||
            mPipelineConfigParams.mbHasRaw[idx] ||
            mReprocParams.mbOpaqueReproc ||
            !!largeYuvStreamSize ||
            mPipelineConfigParams.mForceEnableIMGO;

        if( useImgo )
        {
            mHwParams[idx]->mFullrawSize = sensorSize;
            // check hw limitation with pixel mode & stride
            if( ! mHwInfoHelper[idx]->getImgoFmt(10, mHwParams[idx]->mFullrawFormat) ||
                ! mHwInfoHelper[idx]->alignPass1HwLimitation( mHwParams[idx]->mPixelMode, mHwParams[idx]->mFullrawFormat, MTRUE,
                                                        mHwParams[idx]->mFullrawSize, mHwParams[idx]->mFullrawStride ) )
            {
                MY_LOGE("wrong params about imgo");
                return BAD_VALUE;
            }
            MY_LOGI_IF( 1, "imgo size(%dx%d) stride %zu",
                        mHwParams[idx]->mFullrawSize.w, mHwParams[idx]->mFullrawSize.h, mHwParams[idx]->mFullrawStride);
        }
        else
        {
            mHwParams[idx]->mFullrawSize = MSize(0,0);
        }
    }
    return OK;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
MyProcessedParams::
isNeedLcso()
{
    MBOOL normalLTM = (MTKCAM_LTM_SUPPORT) != 0 ? MTRUE : MFALSE;

    MBOOL izHDR = ((mPipelineConfigParams.mVhdrMode == SENSOR_VHDR_MODE_IVHDR
            || mPipelineConfigParams.mVhdrMode == SENSOR_VHDR_MODE_ZVHDR));
    MBOOL mHDR = ((mPipelineConfigParams.mVhdrMode == SENSOR_VHDR_MODE_MVHDR));

    return (izHDR || normalLTM) && (!mHDR);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
MyProcessedParams::
isNeedRsso(MUINT32 eisMode) const
{
    MBOOL ret = MFALSE;

    if( mPipelineConfigParams.mbIsEIS && EIS_MODE_IS_EIS_30_ENABLED(eisMode) &&
        EIS_MODE_IS_EIS_IMAGE_ENABLED(eisMode) )
    {
        MY_LOGD("use RSSO for EIS");
        ret = MTRUE;
    }

    if( (ret != MTRUE))
    {
        MUINT32 nr3dMode;
        MY_LOGW(" need to do 3DNR!!!!");
        // turn on RSSO if 3DNR support RSC
        if( E3DNR_MODE_MASK_ENABLED(nr3dMode, (E3DNR_MODE_MASK_UI_SUPPORT | E3DNR_MODE_MASK_HAL_FORCE_SUPPORT)) &&
            E3DNR_MODE_MASK_ENABLED(nr3dMode, E3DNR_MODE_MASK_RSC_EN) )
        {
            MY_LOGD("use RSSO for 3DNR");
            ret = MTRUE;
        }
    }

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
#define MIN_RRZO_EIS_W          (1280)
MUINT32
MyProcessedParams::
getMinRrzoEisW()
{
    MUINT32 width = MIN_RRZO_EIS_W;
    return width;
}


/******************************************************************************
 *
 ******************************************************************************/
#define EIS_RATIO_100X          (120)  // src/dst = 1.2
MUINT32
MyProcessedParams::
getEisRatio100X() const
{
    MUINT32 factor = EISCustom::getEIS12Factor(), adbFactor = 0;
    factor = mPipelineConfigParams.mEisFactor;
    adbFactor = property_get_int32("debug.mtkcam.eis.factor", 0);
    MY_LOGI("EIS factor=%d, debug.mtkcam.eis.factor=%d", factor, adbFactor);
    if( adbFactor >= 100 )
    {
        factor = adbFactor;
    }
    return factor;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
MyProcessedParams::
getEisMode() const
{
    MUINT32 eisMode = EIS_MODE_OFF;

    if( property_get_int32("debug.mtkcam.eis.adv", 0) > 0 )
    {
        eisMode = (1<<EIS_MODE_EIS_30) | (1<<EIS_MODE_GYRO) | (1<<EIS_MODE_EIS_DEJELLO) | (1<<EIS_MODE_IMAGE) | (1<<EIS_MODE_EIS_QUEUE);
    }

    return eisMode;
}
