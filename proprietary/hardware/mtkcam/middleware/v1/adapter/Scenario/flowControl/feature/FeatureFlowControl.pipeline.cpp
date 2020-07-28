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

#define LOG_TAG "MtkCam/FeatureFlowControl"
//
#include "MyUtils.h"
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>

#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include "FeatureFlowControl.h"
#include <buffer/ClientBufferPool.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>


// TODO: FIXME
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/Selector.h>

#ifdef FEATURE_MODIFY
#include <mtkcam/feature/eis/eis_ext.h>
#include <camera_custom_eis.h>
#include <camera_custom_3dnr.h>
#include <camera_custom_fsc.h>
#include <mtkcam/feature/fsc/fsc_defs.h>
#include <mtkcam/aaa/IHal3A.h>
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
// for dual zoom
#include <mtkcam/feature/DualCam/FOVHal.h>
#include <camera_custom_dualzoom.h>
#endif
#endif // FEATURE_MODIFY

#include <math.h>
#include <camera_custom_isp_limitation.h>

#include <mtkcam/utils/hw/CamManager.h>

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1) || (MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1) || (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
// DualCam support
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/DualCam/utils/DualCameraUtility.FeatureFlowControl.PropDef.h>
#include <mtkcam/feature/DualCam/utils/DualCameraHWHelper.h>
#define DUALCAM 1
#else
#define DUALCAM 0
#endif
using namespace NSCam::Utils;

using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android;
using namespace NSCam::v3;
using namespace NSCamHW;
using namespace NSCam::EIS;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)
//
#define FUNC_START                  MY_LOGD1("+")
#define FUNC_END                    MY_LOGD1("-")
//
#define TEST_RAW_ALLOCATE (0)
#define SUPPORT_VSS (1)
//
/******************************************************************************
 *
 ******************************************************************************/
//

MERROR
FeatureFlowControl::
decideSensorModeByVHDR(
    HwInfoHelper&   helper,
    MUINT&          sensorMode,
    MUINT32 const   vhdrMode
)
{
    MUINT32 supportHDRMode = 0;
    char forceSensorMode[PROPERTY_VALUE_MAX];
    property_get("vendor.debug.force.vhdr.sensormode", forceSensorMode, "0");
    switch( forceSensorMode[0] )
    {
        case '0':
            break;
        case 'P':
        case 'p':
            sensorMode=SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
            MY_LOGD("set sensor mode to NORMAL_PREVIEW(%d)",SENSOR_SCENARIO_ID_NORMAL_PREVIEW);
            return OK;
        case 'V':
        case 'v':
            sensorMode=SENSOR_SCENARIO_ID_NORMAL_VIDEO;
            MY_LOGD("set sensor mode to NORMAL_VIDEO(%d)",SENSOR_SCENARIO_ID_NORMAL_VIDEO);
            return OK;
        case 'C':
        case 'c':
            sensorMode=SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
            MY_LOGD("set sensor mode to NORMAL_CAPTURE(%d)",SENSOR_SCENARIO_ID_NORMAL_CAPTURE);
            return OK;
        default:
            MY_LOGW("unknown force sensor mode(%s), not used", forceSensorMode);
            MY_LOGW("usage : setprop debug.force.vhdr.sensormode P/V/C");
            break;
    }

    // 1. Current sensor mode is VHDR support, use it.
    if (! helper.querySupportVHDRMode(sensorMode, supportHDRMode))
        return BAD_VALUE;
    if(vhdrMode == supportHDRMode)
        return OK;
    MY_LOGI("VHDR Mode(%d) is not support in current sensor mode(%d), try others sensor mode.", vhdrMode, sensorMode);

    // 2. Check sensor mode in order: preview -> video -> capture
    //    Find acceptable sensor mode for this vhdrMode
    MUINT origSensorMode = sensorMode;

#define CHECK_SENSOR_MODE_VHDR_SUPPORT(senMode) \
do { \
    sensorMode = senMode;\
    if ( ! helper.querySupportVHDRMode(sensorMode, supportHDRMode) )\
    {\
        sensorMode = origSensorMode;\
        return BAD_VALUE;\
    }\
    if (vhdrMode == supportHDRMode)\
    {\
        return OK;\
    }\
} while (0)
    CHECK_SENSOR_MODE_VHDR_SUPPORT(SENSOR_SCENARIO_ID_NORMAL_PREVIEW);
    CHECK_SENSOR_MODE_VHDR_SUPPORT(SENSOR_SCENARIO_ID_NORMAL_VIDEO);
    CHECK_SENSOR_MODE_VHDR_SUPPORT(SENSOR_SCENARIO_ID_NORMAL_CAPTURE);
#undef CHECK_SENSOR_MODE_VHDR_SUPPORT

    // 3.  PREVIEW & VIDEO & CAPTURE mode are all not acceptable
    MY_LOGE("VHDR not support preview & video & capture mode.");
    sensorMode = origSensorMode;
    return BAD_VALUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
decideRrzoImage(
    HwInfoHelper& helper,
    MUINT32 const bitDepth,
    MSize         referenceSize,
    MUINT const   usage,
    MINT32 const  minBuffer,
    MINT32 const  maxBuffer,
    sp<IImageStreamInfo>& rpImageStreamInfo,
    MBOOL         useUFO
)
{
    MSize autualSize;
    size_t stride;
    MINT format;
    if( ! helper.getRrzoFmt(bitDepth, format, useUFO) ||
        ! helper.alignRrzoHwLimitation(referenceSize, mSensorParam.size, autualSize) ||
        ! helper.alignPass1HwLimitation(mSensorParam.pixelMode, format, false, autualSize, stride) )
    {
        MY_LOGE("wrong params about rrzo");
        return BAD_VALUE;
    }
    //
    mRRZOFmt = format;
    //
    MY_LOGI("rrzo num:%d-%d bitDepth:%d format:%d referenceSize:%dx%d, actual size:%dx%d, stride:%zu",
                minBuffer,
                maxBuffer,
                bitDepth,
                format,
                referenceSize.w, referenceSize.h,
                autualSize.w, autualSize.h,
                stride
            );
    rpImageStreamInfo =
        createRawImageStreamInfo(
            "Hal:Image:Resiedraw",
            eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
            eSTREAMTYPE_IMAGE_INOUT,
            maxBuffer, minBuffer,
            usage, format, autualSize, stride
            );

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
decideImgoImage(
    HwInfoHelper& helper,
    MUINT32 const bitDepth,
    MSize         referenceSize,
    MUINT  const  usage,
    MINT32 const  minBuffer,
    MINT32 const  maxBuffer,
    sp<IImageStreamInfo>& rpImageStreamInfo,
    MBOOL         useUFO
)
{
    MSize autualSize = referenceSize;
    size_t stride;
    MINT format;
    if( ! helper.getImgoFmt(bitDepth, format, useUFO) ||
        ! helper.alignPass1HwLimitation(mSensorParam.pixelMode, format, true, autualSize, stride) )
    {
        MY_LOGE("wrong params about imgo");
        return BAD_VALUE;
    }
    //
    mIMGOFmt = format;
    //
    MY_LOGD("imgo num:%d-%d bitDepth:%d format:%d referenceSize:%dx%d, actual size:%dx%d, stride:%zu",
                minBuffer,
                maxBuffer,
                bitDepth,
                format,
                referenceSize.w, referenceSize.h,
                autualSize.w, autualSize.h,
                stride
            );
    rpImageStreamInfo =
        createRawImageStreamInfo(
            "Hal:Image:Fullraw",
            eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
            eSTREAMTYPE_IMAGE_INOUT,
            maxBuffer, minBuffer,
            usage, format, autualSize, stride
            );

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
decideRssoImage(
    MINT32 minBuffer,
    MINT32 maxBuffer,
    sp<IImageStreamInfo>& rpImageStreamInfo
)
{
    MUINT usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
    MINT format = eImgFmt_STA_BYTE;
    MSize size(288, 512);
#ifdef FEATURE_MODIFY
    if (mLPBConfigParams.enableFSC)//add extra 10% margin for FSC crop
        size.w = FSC_MAX_RSSO_WIDTH;
#endif
    size_t stride = size.w;
    //
    MY_LOGI("rsso num:%d-%d format:%d size:%dx%d, stride:%zu",
            minBuffer, maxBuffer, format, size.w, size.h, stride);

    rpImageStreamInfo =
        createRawImageStreamInfo(
            "Hal:Image:RSSO",
            eSTREAMID_IMAGE_PIPE_RAW_RSSO,
            eSTREAMTYPE_IMAGE_INOUT,
            maxBuffer, minBuffer,
            usage, format, size, stride
            );

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
setCamClient(
    const char* name,
    StreamId streamId,
    Vector<PipelineImageParam>& vImageParam,
    Vector<MUINT32> clientMode,
    MUINT usage,
    MBOOL useTransform
)
{
    MSize const& size = MSize(-1,-1);
    MINT const format = eImgFmt_YUY2;
    size_t const stride = 1280;
    sp<IImageStreamInfo> pImage_Yuv =
        createImageStreamInfo(
            name,
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            5, 1,
            usage, format, size, 0
            );
    sp<ClientBufferPool> pClient = new ClientBufferPool(getOpenId(), useTransform);
    pClient->setCamClient(
                        name,
                        mpImgBufProvidersMgr,
                        clientMode[0]
                    );
    for ( size_t i = 1; i < clientMode.size(); ++i ) {
        pClient->setCamClient( clientMode[i] );
    }

    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
    pFactory->setImageStreamInfo(pImage_Yuv);
    pFactory->setUsersPool(pClient);

    vImageParam.push_back(
        PipelineImageParam{
            pImage_Yuv,
            pFactory->create(),
            0
        }
    );

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
addYUVHalPool(
    const char* name,
    StreamId streamId,
    size_t maxBufNum,
    size_t minInitBufNum,
    MUINT usage,
    MINT const format,
    MSize const& size,
    Vector<PipelineImageParam>& vImageParam
)
{
    sp<IImageStreamInfo> pImage_Yuv =
        createImageStreamInfo(
            name,
            streamId,
            eSTREAMTYPE_IMAGE_INOUT,
            maxBufNum, minInitBufNum,
            usage, format, size, 0
            );
    vImageParam.push_back(
        PipelineImageParam{
            pImage_Yuv,
            nullptr,
            0
        }
    );
    MY_LOGD("N[%s] S[%llx] s[%dx%d]", name, streamId, size.w, size.h);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
constructNormalPreviewPipeline()
{
    FUNC_START;

    CAM_TRACE_NAME("DFC:constructFeaturePreviewPipeline");

    mLPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_Feature_Preview;

    HwInfoHelper helper(mOpenId);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
#if DUALCAM
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        if (DUALZOOM_FOV_MAX_FPS == 30)
        {
            mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        }
        else if (DUALZOOM_FOV_MAX_FPS == 24)
        {
            mSensorParam.mode = SENSOR_SCENARIO_ID_CUSTOM1;
        }
        else
        {
            MY_LOGW("Dualzoom not support fps : %d", DUALZOOM_FOV_MAX_FPS);
            mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        }
    }
    else
#endif // (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#if ((MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1))
    if(StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE|NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF)
       || StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
    {
        mSensorParam.mode = getSensorMode(PipelineMode_PREVIEW);
    }
    else
#endif // ((MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1))
    {
        mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    }
#endif // DUALCAM
    mConstructVideoPipe = MFALSE;

#ifdef FEATURE_MODIFY
    MUINT32 vhdrMode = mpParamsManagerV3->getParamsMgr()->getVHdr();
    if(vhdrMode != SENSOR_VHDR_MODE_NONE
        && decideSensorModeByVHDR(helper, mSensorParam.mode, vhdrMode) != OK) {
        MY_LOGE("cannot get vhdr supported sensor mode.");
        return BAD_VALUE;
    }
#endif // FEATURE_MODIFY

    if( ! helper.getSensorSize( mSensorParam.mode, mSensorParam.size) ||
        ! helper.getSensorFps( (MUINT32)mSensorParam.mode, (MINT32&)mSensorParam.fps) ||
        ! helper.queryPixelMode( mSensorParam.mode, mSensorParam.fps, mSensorParam.pixelMode)
      ) {
        MY_LOGE("cannot get params about sensor");
        return BAD_VALUE;
    }

    if (helper.getDualPDAFSupported(mSensorParam.mode))
    {
        mLPBConfigParams.enableDualPD = MTRUE;
    }
    //
    if (helper.getPDAFSupported(mSensorParam.mode) && !CamManager::getInstance()->isMultiDevice())
    {
        mLPBConfigParams.disableFrontalBinning = MTRUE;
    }
    //
    if( mbNeedInitRequest && mInitRequest > 0 )
    {
        mLPBConfigParams.initRequest = mInitRequest;
    }
    //
#if DUALCAM
    if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        mLPBConfigParams.enableUNI = DualCameraUtility::getProperty<DUALZOOM_FORCE_DISABLE_UNIT>() ? MFALSE : MTRUE;
        mLPBConfigParams.dualcamMode = Stereo::E_DUALCAM_FEATURE_ZOOM;

        MBOOL isEnableFrontalBinning =  (DualCameraHWHelper::getImgQualitySupportedByPlatform() || DualCameraUtility::getProperty<DUALZOOM_FORCE_ENABLE_FRONRALBIN>());
        mLPBConfigParams.disableFrontalBinning = isEnableFrontalBinning ? MFALSE : MTRUE;
    }
    else if(StereoSettingProvider::isDualCamMode())
    {
        // may denosie or vsdof
        mLPBConfigParams.enableUNI = MFALSE;
        mLPBConfigParams.dualcamMode = StereoSettingProvider::getStereoFeatureMode();
        mLPBConfigParams.disableFrontalBinning = MFALSE;
    }
#endif // DUALCAM
    //
    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if( helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit) )
    {
        using namespace NSCam::NSIoPipe::NSCamIOPipe;
        if(pipeBit & CAM_Pipeline_14BITS)
        {
            mLPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else
        {
            mLPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }
    //

#ifdef FEATURE_MODIFY
    // Sensor
    mSensorParam.vhdrMode = mpParamsManagerV3->getParamsMgr()->getVHdr();
    MY_LOGD("VHDR sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d vhdrMode:%d",
        mSensorParam.mode,
        mSensorParam.rawType,
        mSensorParam.size.w, mSensorParam.size.h,
        mSensorParam.fps,
        mSensorParam.pixelMode,
        mSensorParam.vhdrMode);
#else
    // Sensor
    MY_LOGD("sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d",
        mSensorParam.mode,
        mSensorParam.rawType,
        mSensorParam.size.w, mSensorParam.size.h,
        mSensorParam.fps,
        mSensorParam.pixelMode);
#endif // FEATURE_MODIFY
    //
    MUINT32 const bitDepth = getPreviewRawBitDepth(helper);
    MSize previewsize;
    MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
    mpParamsManagerV3->getParamsMgr()->getPreviewSize(&previewsize.w, &previewsize.h);
#if DUALCAM
    if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        MSize previewsize_orig = previewsize;
        decideRrzoForDualZoom(mOpenId, previewsize, 1, 0);

        MINT main2;
        mpParamsManagerV3->getParamsMgr()->getDualZoomInfo(&main2, NULL);
        if (mOpenId != main2) {
            MSize previewsize2 = previewsize;
            decideRrzoForDualZoom(main2, previewsize2, 1, 0);
            MY_LOGD("normalpreview wanted(%dx%d) 1(%dx%d) 2(%dx%d)",
                previewsize_orig.w, previewsize_orig.h,
                previewsize.w, previewsize.h,
                previewsize2.w, previewsize2.h
                );
            previewsize2.w = (previewsize2.w > previewsize.w)? previewsize2.w : previewsize.w;
            previewsize2.h = (previewsize2.h > previewsize.h)? previewsize2.h : previewsize.h;

            MSize raw = previewsize2;
            decideRrzoForStreaming(raw, mSensorParam.mode);
            mLPBConfigParams.streamingSize = raw;
            MY_LOGD("constructNormalPreviewPipeline (%dx%d)->(%dx%d)",
                previewsize2.w, previewsize2.h, raw.w, raw.h);
        }
    }
#endif // DUALCAM
#ifdef FEATURE_MODIFY
    mLPBConfigParams.enableRSS = this->useRSSO(mEisInfo.mode);
    mLPBConfigParams.enableFSC = MFALSE;
#endif // FEATURE_MODIFY

    mpParamsManagerV3->getParamsMgr()->getPreviewSize(&mLPBConfigParams.previewSize.w, &mLPBConfigParams.previewSize.h);
    mpParamsManagerV3->getParamsMgr()->getVideoSize(&mLPBConfigParams.videoSize.w, &mLPBConfigParams.videoSize.h);

    sp<LegacyPipelineBuilder> pBuilder =
        LegacyPipelineBuilder::createInstance(
                                    mOpenId,
                                    "FeaturePreview",
                                    mLPBConfigParams);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return BAD_VALUE;
    }
    //
    pBuilder->setSrc(mSensorParam);
    //
    MUINT featureCFG = 0;
    if (mLPBConfigParams.enableDualPD)
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_DUAL_PD);
    }
    if (mSensorParam.vhdrMode != SENSOR_VHDR_MODE_NONE)
    {
        if(mSensorParam.vhdrMode == SENSOR_VHDR_MODE_IVHDR)
            FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_IVHDR);
        if(mSensorParam.vhdrMode == SENSOR_VHDR_MODE_MVHDR)
            FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_MVHDR);
        if(mSensorParam.vhdrMode == SENSOR_VHDR_MODE_ZVHDR)
            FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_ZVHDR);
    }
#if DUALCAM
    if(StereoSettingProvider::getStereoFeatureMode() == Stereo::E_STEREO_FEATURE_DENOISE ||
       StereoSettingProvider::getStereoFeatureMode() == Stereo::E_DUALCAM_FEATURE_ZOOM)
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG, IScenarioControl::FEATURE_DUALZOOM_PREVIEW);
    }
    else if(StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE|NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF)
       || StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG, IScenarioControl::FEATURE_VSDOF_PREVIEW);
    }
#endif // DUALCAM
    //
    sp<IScenarioControl> pScenarioCtrl = enterScenarioControl(getScenario(), mSensorParam.size, mSensorParam.fps, featureCFG);
    if( pScenarioCtrl.get() == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return BAD_VALUE;
    }
    pBuilder->setScenarioControl(pScenarioCtrl);
    //
    sp<IImageStreamInfo> pImage_FullRaw;
    sp<PairMetadata>  pFullPair;
    sp<BufferPoolImp> pFullRawPool;
    sp<IImageStreamInfo> pImage_ResizedRaw;
    sp<PairMetadata>  pResizedPair;
    sp<BufferPoolImp> pResizedRawPool;
    // Image
    {
        CAM_TRACE_NAME("DFC:SetImageDst");
        Vector<PipelineImageParam> vImageParam;
        // RAW (RRZO)
        {
            if ( OK != decideRrzoImage(
                            helper, bitDepth,
                            previewsize, usage,
                            2, 8,
                            pImage_ResizedRaw, MFALSE
                        ))
            {
                MY_LOGE("No rrzo image");
                return BAD_VALUE;
            }

            if(mNeedDumpRRZO)
            {
                MY_LOGD("Open RRZO for dump mNeedDumpRRZO(%d)",mNeedDumpRRZO);
                pResizedPair = PairMetadata::createInstance(pImage_ResizedRaw->getStreamName());
                pResizedRawPool = new BufferPoolImp(pImage_ResizedRaw);
                //
                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                pFactory->setImageStreamInfo(pImage_ResizedRaw);
                pFactory->setPairRule(pResizedPair, 1);
                pFactory->setUsersPool(pResizedRawPool);
                sp<StreamBufferProvider> pProducer = pFactory->create();
                //
                mpResourceContainer->setConsumer( pImage_ResizedRaw->getStreamId(), pProducer );
                //
                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_ResizedRaw,
                        pProducer,
                        0
                    }
                );
            }
            else
            {
                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_ResizedRaw,
                        NULL,
                        0
                    }
                );
            }
            // ===== For Dump IMGO raw  ============
            if(mForceEnableIMGO || mNeedDumpIMGO){

                MY_LOGD("Open IMGO for dump mForceEnableIMGO(%d) mNeedDumpIMGO(%d)",mForceEnableIMGO,mNeedDumpIMGO);

                if ( OK != decideImgoImage(
                            helper, bitDepth,
                            mSensorParam.size, usage,
                            4, 8,
                            pImage_FullRaw, MTRUE
                            ))
                {
                    MY_LOGE("No imgo image");
                    return BAD_VALUE;
                }

                if(mNeedDumpIMGO)
                {
                    pFullPair = PairMetadata::createInstance(pImage_FullRaw->getStreamName());
                    pFullRawPool = new BufferPoolImp(pImage_FullRaw);
                    //
                    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                    pFactory->setImageStreamInfo(pImage_FullRaw);
                    pFactory->setPairRule(pFullPair, 1);
                    pFactory->setUsersPool(pFullRawPool);
                    sp<StreamBufferProvider> pProducer = pFactory->create();
                    //
                    mpResourceContainer->setConsumer( pImage_FullRaw->getStreamId(), pProducer );
                    //
                    vImageParam.push_back(
                            PipelineImageParam{
                            pImage_FullRaw,
                            pProducer,
                            0
                            }
                            );
                }
                else
                {
                    vImageParam.push_back(
                            PipelineImageParam{
                            pImage_FullRaw,
                            NULL,
                            0
                            }
                            );
                }
            }
        }
        // ===============================
        // RAW (LCSO with pure pool)
        if (mLPBConfigParams.enableLCS)
        {
            MUINT32 const bitDepth = 10; // no use
            MSize anySize; // no use
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
            sp<IImageStreamInfo> pImage_Raw;

            if ( OK != decideLcsoImage(
                            helper, bitDepth,
                            anySize, usage,
                            2, 8,
                            pImage_Raw
                        ))
            {
                MY_LOGE("No lcso image");
                return BAD_VALUE;
            }

            vImageParam.push_back(
                PipelineImageParam{
                    pImage_Raw,
                    NULL,
                    0
                }
            );
        }
#ifdef FEATURE_MODIFY
        // RSSO RAW
        if (mLPBConfigParams.enableRSS)
        {
            int mrrzoSize_Max = 8 , mrrzoSize_Min = 2;
            sp<IImageStreamInfo> pImage_RssoRaw;

            if( OK != decideRssoImage(mrrzoSize_Min+1,
                                      mrrzoSize_Max+1,
                                      pImage_RssoRaw) )
            {
                MY_LOGE("No rsso image");
                return BAD_VALUE;
            }

            vImageParam.push_back(
                PipelineImageParam{
                    pImage_RssoRaw,
                    NULL,
                    0
                }
            );
        }
#endif // FEATURE_MODIFY
        // YUV preview -> display client
        {
            Vector<MUINT32> clientMode;
            clientMode.push_back(IImgBufProvider::eID_DISPLAY);

            setCamClient(
                "Hal:Image:yuvDisp",
                eSTREAMID_IMAGE_PIPE_YUV_00,
                vImageParam,
                clientMode,
                GRALLOC_USAGE_HW_COMPOSER,
                MTRUE
            );
        }
        // YUV preview callback -> preview callback client
        {
            Vector<MUINT32> clientMode;
            clientMode.push_back(IImgBufProvider::eID_PRV_CB);
            clientMode.push_back(IImgBufProvider::eID_GENERIC);

            setCamClient(
                "Hal:Image:yuvPrvCB",
                eSTREAMID_IMAGE_PIPE_YUV_01,
                vImageParam,
                clientMode
            );
        }
        // YUV FD & OT
        {
            Vector<MUINT32> clientMode;
            clientMode.push_back(IImgBufProvider::eID_FD);
            clientMode.push_back(IImgBufProvider::eID_OT);

            setCamClient(
                "Hal:Image:yuvFD",
                eSTREAMID_IMAGE_YUV_FD,
                vImageParam,
                clientMode
            );
        }

        pBuilder->setDst(vImageParam);
    }

#ifdef FEATURE_MODIFY
    // --- Update Hal3A Config param
    prepare3AInitParam(pImage_ResizedRaw->getImgSize(), mLPBConfigParams.hal3AParams);
    pBuilder->updateHal3AConfig( mLPBConfigParams.hal3AParams );
#endif

    mpPipeline = pBuilder->create();

    if ( mpPipeline == 0) {
        MY_LOGE("Fail to create Legacy Pipeline.");
        return BAD_VALUE;
    }
    //
    sp<ResultProcessor> pProcessor = mpPipeline->getResultProcessor().promote();
    sp<FrameInfo> pFrameInfo = new FrameInfo();
    mpResourceContainer->setLatestFrameInfo(pFrameInfo);
    pProcessor->registerListener(
                    eSTREAMID_META_APP_DYNAMIC_P1,
                    pFrameInfo
                    );
    pProcessor->registerListener(
                    eSTREAMID_META_HAL_DYNAMIC_P1,
                    pFrameInfo
                    );
#if DUALCAM
    pProcessor->registerListener(
                    eSTREAMID_META_APP_DYNAMIC_P2,
                    pFrameInfo
                    );
#endif // DualCam
    //
    if ( mNeedDumpIMGO )
    {
        pFullRawPool->allocateBuffer(
        pImage_FullRaw->getStreamName(),
        pImage_FullRaw->getMaxBufNum(),
        pImage_FullRaw->getMinInitBufNum()
                );

        sp<StreamBufferProvider> pTempConsumer =
            mpResourceContainer->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );

        if ( pTempConsumer != 0 ) {
            sp<ResultProcessor> pProcessor = mpPipeline->getResultProcessor().promote();
            pProcessor->registerListener(
                            eSTREAMID_META_HAL_DYNAMIC_P2,
                            pFullPair
                            );
            // Need set Selector
            sp<DumpBufferSelector> pSelector = new DumpBufferSelector();
            pSelector->setDumpConfig( mOpenId );
            pTempConsumer->setSelector(pSelector);
            //
        }
    }
    //
    if ( mNeedDumpRRZO )
    {
        pResizedRawPool->allocateBuffer(
        pImage_ResizedRaw->getStreamName(),
        pImage_ResizedRaw->getMaxBufNum(),
        pImage_ResizedRaw->getMinInitBufNum()
                );

        sp<StreamBufferProvider> pTempConsumer =
            mpResourceContainer->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_RESIZER );

        if ( pTempConsumer != 0 ) {
            sp<ResultProcessor> pProcessor = mpPipeline->getResultProcessor().promote();
            pProcessor->registerListener(
                            eSTREAMID_META_HAL_DYNAMIC_P2,
                            pResizedPair
                            );
            // Need set Selector
            sp<DumpBufferSelector> pSelector = new DumpBufferSelector();
            pSelector->setDumpConfig( mOpenId );
            pTempConsumer->setSelector(pSelector);
            //
        }
    }
    //
    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
constructRecordingPipeline()
{
    FUNC_START;
    MBOOL bHighSpeedMode = MFALSE;
    MUINT previewMaxFps = 0;
    HwInfoHelper helper(mOpenId);

    #ifdef FEATURE_MODIFY
    MBOOL bAdvEISEnabled = MFALSE;
    #endif // FEATURE_MODIFY

    CAM_TRACE_NAME("DFC:constructRecordingPipeline");
    //
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if(mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) > HighSpeedVideoFpsBase)
    {
        selectHighSpeedSensorScen(
            mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE),
            mSensorParam.mode);
    }
    else
    {
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
        if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
        {
            if (DUALZOOM_FOV_MAX_FPS == 30)
            {
                mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
            }
            else if (DUALZOOM_FOV_MAX_FPS == 24)
            {
                mSensorParam.mode = SENSOR_SCENARIO_ID_CUSTOM1;
            }
            else
            {
                MY_LOGW("Dualzoom not support fps : %d", DUALZOOM_FOV_MAX_FPS);
                mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
            }
        }
        else
#endif // (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#if ((MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1))
    if(StereoSettingProvider::getStereoFeatureMode() == (Stereo::E_STEREO_FEATURE_CAPTURE|Stereo::E_STEREO_FEATURE_VSDOF)
       || StereoSettingProvider::getStereoFeatureMode() == (Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
    {
        mSensorParam.mode = getSensorMode(PipelineMode_RECORDING);
    }
    else
#endif // ((MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1))
        {
            mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
        }
    }
    mConstructVideoPipe = MTRUE;
    //
    if (helper.getDualPDAFSupported(mSensorParam.mode))
    {
        mLPBConfigParams.enableDualPD = MTRUE;
    }
    //
    if( mbNeedInitRequest && mInitRequest > 0 )
    {
        mLPBConfigParams.initRequest = mInitRequest;
    }
    //

#if DUALCAM
    if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        mLPBConfigParams.enableUNI = DualCameraUtility::getProperty<DUALZOOM_FORCE_DISABLE_UNIT>() ? MFALSE : MTRUE;
        mLPBConfigParams.dualcamMode = Stereo::E_DUALCAM_FEATURE_ZOOM;

        MBOOL isEnableFrontalBinning =  (DualCameraHWHelper::getImgQualitySupportedByPlatform() || DualCameraUtility::getProperty<DUALZOOM_FORCE_ENABLE_FRONRALBIN>());
        mLPBConfigParams.disableFrontalBinning = isEnableFrontalBinning ? MFALSE : MTRUE;
    }
    else if(StereoSettingProvider::isDualCamMode())
    {
        // may denosie or vsdof
        mLPBConfigParams.enableUNI = MFALSE;
        mLPBConfigParams.dualcamMode = StereoSettingProvider::getStereoFeatureMode();
        mLPBConfigParams.disableFrontalBinning = MFALSE;
    }
#endif // DUALCAM
    //
    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if( helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit) )
    {
        using namespace NSCam::NSIoPipe::NSCamIOPipe;
        if(pipeBit & CAM_Pipeline_14BITS)
        {
            mLPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else
        {
            mLPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }
    //
    mLPBConfigParams.receiveMode = P1Node::REV_MODE_CONSERVATIVE;
    //
    if(!helper.getSensorFps((MUINT32)mSensorParam.mode, (MINT32&)mSensorParam.fps))
    {
        MY_LOGE("cannot get params about sensor");
        return BAD_VALUE;
    }
    //
    previewMaxFps =     ((MUINT)mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= mSensorParam.fps) ?
                        mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) : mSensorParam.fps;
    mpParamsManagerV3->setPreviewMaxFps(previewMaxFps);
    if(previewMaxFps > HighSpeedVideoFpsBase)
    {
        bHighSpeedMode = MTRUE;
    }
    //
    MSize recordsize = MSize(0,0);
    if( decideSensorModeAndRrzo(mOpenId, recordsize, mSensorParam.mode, bHighSpeedMode) != OK )
    {
        return BAD_VALUE;
    }
    //
    if (helper.getPDAFSupported(mSensorParam.mode) && !CamManager::getInstance()->isMultiDevice())
    {
        mLPBConfigParams.disableFrontalBinning = MTRUE;
    }
    //
#if DUALCAM
    if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        MINT main2;
        mpParamsManagerV3->getParamsMgr()->getDualZoomInfo(&main2, NULL);
        if (mOpenId != main2) {
            MSize recordsize2 = MSize(0, 0);
            MUINT mode;
            decideSensorModeAndRrzo(main2, recordsize2, mode, bHighSpeedMode);
            MY_LOGD("record 1(%dx%d) 2(%dx%d)",
                recordsize.w, recordsize.h,
                recordsize2.w, recordsize2.h
                );
            recordsize2.w = (recordsize2.w > recordsize.w)? recordsize2.w : recordsize.w;
            recordsize2.h = (recordsize2.h > recordsize.h)? recordsize2.h : recordsize.h;
            MSize raw = recordsize2;
            decideRrzoForStreaming(raw, mSensorParam.mode);
            mLPBConfigParams.streamingSize = raw;
            MY_LOGD("constructRecordingPipeline: (%dx%d)->(%dx%d)",
                recordsize2.w, recordsize2.h, raw.w, raw.h);
        }
    }
#endif

#ifdef FEATURE_MODIFY
    MUINT32 vhdrMode = mpParamsManagerV3->getParamsMgr()->getVHdr();
    if(vhdrMode != SENSOR_VHDR_MODE_NONE
        && decideSensorModeByVHDR(helper, mSensorParam.mode, vhdrMode) != OK) {
        MY_LOGE("cannot get vhdr supported sensor mode.");
        return BAD_VALUE;
    }
#endif // FEATURE_MODIFY

    //
    if( ! helper.getSensorSize( mSensorParam.mode, mSensorParam.size) ||
        ! helper.getSensorFps( (MUINT32)mSensorParam.mode, (MINT32&)mSensorParam.fps) ||
        ! helper.queryPixelMode( mSensorParam.mode, mSensorParam.fps, mSensorParam.pixelMode)
      ) {
        MY_LOGE("cannot get params about sensor");
        return BAD_VALUE;
    }
    //
    if(bHighSpeedMode)
    {
        mLPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_HighSpeedVideo;
        mLPBConfigParams.enableEIS = MFALSE;
        mpRequestThreadLoopCnt = previewMaxFps/HighSpeedVideoFpsBase;
    }
    else
    {
        mLPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_Feature_VideoRecord;

        #ifdef FEATURE_MODIFY
        if( mpParamsManagerV3->getParamsMgr()->getVideoStabilization() )
        {
            MINT32 videoWidth = 0, videoHeight = 0;

            mpParamsManagerV3->getParamsMgr()->getVideoSize(&videoWidth,&videoHeight);

            // Set EIS configurations for external uses
            if( EIS_MODE_IS_EIS_22_ENABLED(mEisInfo.mode) ||
                EIS_MODE_IS_EIS_25_ENABLED(mEisInfo.mode) ||
                EIS_MODE_IS_EIS_30_ENABLED(mEisInfo.mode) )
            {
                bAdvEISEnabled = MTRUE;
            }
            MY_LOGD("start recording WxH (%dx%d), eisMode 0x%x",videoWidth,videoHeight,mEisInfo.mode);
        }
        else
        {
            //DoNothing
            mEisInfo.mode = EIS_MODE_OFF;
        }

        #endif // FEATURE_MODIFY
    }

    // Determine enable RSSO by EIS mode
    mLPBConfigParams.enableRSS = this->useRSSO(mEisInfo.mode);

    mpParamsManagerV3->getParamsMgr()->getPreviewSize(&mLPBConfigParams.previewSize.w, &mLPBConfigParams.previewSize.h);
    mpParamsManagerV3->getParamsMgr()->getVideoSize(&mLPBConfigParams.videoSize.w, &mLPBConfigParams.videoSize.h);

    #ifdef FEATURE_MODIFY
    MBOOL enableFSC = this->useFSC();
    if (enableFSC)
    {
        bool isEisOn = mpParamsManagerV3->getParamsMgr()->getVideoStabilization();
        if (isEisOn && !EIS_MODE_IS_EIS_30_ENABLED(mEisInfo.mode))
        {
            MY_LOGD("disable FSC due to EIS combination eisMode(0x%x)", mEisInfo.mode);
            enableFSC = MFALSE;
        }
    }
    mLPBConfigParams.enableFSC = enableFSC;
    #endif
    sp<LegacyPipelineBuilder> pBuilder =
        LegacyPipelineBuilder::createInstance(
                                    mOpenId,
                                    "FeatureRecord",
                                    mLPBConfigParams);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return BAD_VALUE;
    }

    #ifdef FEATURE_MODIFY
    // Sensor
    mSensorParam.vhdrMode = mpParamsManagerV3->getParamsMgr()->getVHdr();
    MY_LOGD("sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d vhdrMode:%d",
        mSensorParam.mode,
        mSensorParam.rawType,
        mSensorParam.size.w, mSensorParam.size.h,
        mSensorParam.fps,
        mSensorParam.pixelMode,
        mSensorParam.vhdrMode);
    #else
    // Sensor
    MY_LOGD("sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d",
        mSensorParam.mode,
        mSensorParam.rawType,
        mSensorParam.size.w, mSensorParam.size.h,
        mSensorParam.fps,
        mSensorParam.pixelMode);
    #endif // FEATURE_MODIFY

    pBuilder->setSrc(mSensorParam);
    //

    #ifdef FEATURE_MODIFY
    MUINT featureCFG = 0;
    if( bAdvEISEnabled )
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG, IScenarioControl::FEATURE_ADV_EIS);
        if( mb4K2KVideoRecord )
        {
            FEATURE_CFG_ENABLE_MASK(featureCFG, IScenarioControl::FEATURE_ADV_EIS_4K);
        }
    }
    if (mLPBConfigParams.enableDualPD)
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_DUAL_PD);
    }
    if (mSensorParam.vhdrMode != SENSOR_VHDR_MODE_NONE)
    {
        if(mSensorParam.vhdrMode == SENSOR_VHDR_MODE_IVHDR)
            FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_IVHDR);
        if(mSensorParam.vhdrMode == SENSOR_VHDR_MODE_MVHDR)
            FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_MVHDR);
        if(mSensorParam.vhdrMode == SENSOR_VHDR_MODE_ZVHDR)
            FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_ZVHDR);
    }
#if DUALCAM
    if(StereoSettingProvider::getStereoFeatureMode() == Stereo::E_STEREO_FEATURE_DENOISE ||
       StereoSettingProvider::getStereoFeatureMode() == Stereo::E_DUALCAM_FEATURE_ZOOM)
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG, IScenarioControl::FEATURE_DUALZOOM_RECORD);
    }
    else if(StereoSettingProvider::getStereoFeatureMode() == (Stereo::E_STEREO_FEATURE_CAPTURE|Stereo::E_STEREO_FEATURE_VSDOF)
       || StereoSettingProvider::getStereoFeatureMode() == (Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG, IScenarioControl::FEATURE_VSDOF_RECORD);
    }
#endif // DUALCAM
    //
    MSize videoSize = MSize(0,0);
    MINT32 camMode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE); //if camMode = 0 => 3rd AP
    mpParamsManagerV3->getParamsMgr()->getVideoSize(&videoSize.w,&videoSize.h);
    //
    sp<IScenarioControl> pScenarioCtrl = enterScenarioControl(getScenario(), mSensorParam.size, mSensorParam.fps, featureCFG, videoSize, camMode);
    #endif // FEATURE_MODIFY

    if( pScenarioCtrl.get() == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return BAD_VALUE;
    }
    pBuilder->setScenarioControl(pScenarioCtrl);

    // Image
    //
    sp<IImageStreamInfo> pImage_FullRaw;
    sp<PairMetadata>  pFullPair;
    sp<BufferPoolImp> pFullRawPool;
    sp<IImageStreamInfo> pImage_ResizedRaw;
    sp<PairMetadata>  pResizedPair;
    sp<BufferPoolImp> pResizedRawPool;
    sp<IImageStreamInfo> pImage_LcsoRaw;
    sp<IImageStreamInfo> pImage_RssoRaw;

    sp<VssSelector> pVssSelector = new VssSelector();
//        sp<PairMetadata> pPair;
    int mrrzoSize_Max = 0 , mrrzoSize_Min = 0;
    {
        CAM_TRACE_NAME("DFC:SetImageDst");
        Vector<PipelineImageParam> vImageParam;
        // RAW
        {
            MUINT32 const bitDepth = getPreviewRawBitDepth(helper);
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
//                sp<IImageStreamInfo> pImage_Raw;



            if(bHighSpeedMode)
            {
                // TODO: Check how to decide RRZO buffer count for high speed mode
                mrrzoSize_Max = mpRequestThreadLoopCnt * 8; // 6 for p1, 2 for p2 depth
                mrrzoSize_Min = mrrzoSize_Max/2;
            }
            else
            {
                mrrzoSize_Max = 8;
                mrrzoSize_Min = 4;
            }
            // for debug use for modify RRZO buffer count
            {
                char rrzoSize[PROPERTY_VALUE_MAX] = {'\0'};
                int tempMrrzoSize;
                ::property_get("vendor.debug.camera.rrzosize", rrzoSize, "-1");
                tempMrrzoSize = ::atoi(rrzoSize);
                if (tempMrrzoSize != -1)
                {
                    mrrzoSize_Max = tempMrrzoSize;
                    // incase of set RRZO count as 0
                    if (mrrzoSize_Max == 0)
                    {
                        mrrzoSize_Max = 8;
                    }
                    mrrzoSize_Min = mrrzoSize_Max/2;
                }
            }
            MY_LOGI("RRZO Count %d/%d",mrrzoSize_Min, mrrzoSize_Max);
            if(mNeedDumpRRZO)
            {
                if ( OK != decideRrzoImage(
                                helper, bitDepth,
                                recordsize, usage,
                                mrrzoSize_Min, mrrzoSize_Max,
                                pImage_ResizedRaw, MTRUE
                            ))
                {
                    MY_LOGE("No rrzo image");
                    return BAD_VALUE;
                }
            }
            else
            {
                if ( OK != decideRrzoImage( helper, bitDepth, recordsize, usage, mrrzoSize_Min, mrrzoSize_Max,
                                pImage_ResizedRaw, MTRUE
                            ))
                {
                    MY_LOGE("No rrzo image");
                    return BAD_VALUE;
                }
            }
            //
            if(mNeedDumpRRZO)
            {
                MY_LOGD("Open RRZO for dump mNeedDumpRRZO(%d)",mNeedDumpRRZO);
                pResizedPair = PairMetadata::createInstance(pImage_ResizedRaw->getStreamName());
                pResizedRawPool = new BufferPoolImp(pImage_ResizedRaw);
                //
                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                pFactory->setImageStreamInfo(pImage_ResizedRaw);
                pFactory->setPairRule(pResizedPair, 1);
                pFactory->setUsersPool(pResizedRawPool);
                sp<StreamBufferProvider> pProducer = pFactory->create();
                //
                mpResourceContainer->setConsumer( pImage_ResizedRaw->getStreamId(), pProducer );
                //
                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_ResizedRaw,
                        pProducer,
                        0
                    }
                );
            }
            else
            {
                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_ResizedRaw,
                        NULL,
                        0
                    }
                );
            }

            if(!bHighSpeedMode)
            {
                if(mNeedDumpIMGO)
                {
                    MY_LOGD("Open IMGO for dump mNeedDumpIMGO(%d)",mNeedDumpIMGO);
                    // for Debug Dump
                    if ( OK != decideImgoImage(
                                    helper, bitDepth,
                                    mSensorParam.size, usage,
                                    4, 10,
                                    pImage_FullRaw, MTRUE
                                ))
                    {
                        MY_LOGE("No imgo image");
                        return BAD_VALUE;
                    }
                }
                else
                {
                    // for VSS
                    if ( OK != decideImgoImage(
                                    helper, bitDepth,
                                    mSensorParam.size, usage,
                                    1, 1,
                                    pImage_FullRaw, MTRUE
                                ))
                    {
                        MY_LOGE("No imgo image");
                        return BAD_VALUE;
                    }
                }
                pFullPair = PairMetadata::createInstance(pImage_FullRaw->getStreamName());

                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                pFactory->setImageStreamInfo(pImage_FullRaw);
                if (mNeedDumpIMGO)
                {
                    pFullRawPool = new BufferPoolImp(pImage_FullRaw);
                    pFactory->setPairRule(pFullPair, 3);
                    pFactory->setUsersPool(pFullRawPool);
                    pVssSelector->setDumpConfig(mOpenId);
                }
                else
                {
                    pFactory->setPairRule(pFullPair, 2);
                }
                sp<StreamBufferProvider> pProducer = pFactory->create();
                pProducer->setSelector(pVssSelector);
                mpResourceContainer->setConsumer(pImage_FullRaw->getStreamId(),pProducer);

                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_FullRaw,
                        pProducer,
                        0
                    }
                );
            }

            // RAW (LCSO with provider)
            if (mLPBConfigParams.enableLCS)
            {
                MUINT32 const bitDepth = 10; // no use
                MSize anySize; // no use
                MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here

                if ( OK != decideLcsoImage(
                            helper, bitDepth,
                            anySize, usage,
                            mrrzoSize_Min, mrrzoSize_Max + 3, // in VR, LCSO buffer need more than RRZO, +3 is the tunning result for 4K VSS case
                            pImage_LcsoRaw
                        ))
                {
                    MY_LOGE("No lcso image");
                    return BAD_VALUE;
                }

                if(!bHighSpeedMode)
                {
                    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                    pFactory->setImageStreamInfo(pImage_LcsoRaw);
                    // Need to set the same PairMetadata, and the PairMetadata need to wait 2 buffer
                    pFactory->setPairRule(pFullPair, 2);
                    sp<StreamBufferProvider> pProducer = pFactory->create();
                    // Need set Selector
                    pProducer->setSelector(pVssSelector);

                    mpResourceContainer->setConsumer(pImage_LcsoRaw->getStreamId(),pProducer);

                    vImageParam.push_back(
                        PipelineImageParam{
                            pImage_LcsoRaw,
                            pProducer,
                            0
                        }
                    );
                }
                else
                {
                    vImageParam.push_back(
                        PipelineImageParam{
                            pImage_LcsoRaw,
                            NULL,
                            0
                        }
                    );
                }
            }

            if( !bHighSpeedMode && mLPBConfigParams.enableRSS )
            {
                if( OK != decideRssoImage(mrrzoSize_Min+1,
                                          mrrzoSize_Max+1,
                                          pImage_RssoRaw) )
                {
                    MY_LOGE("No rsso image");
                    return BAD_VALUE;
                }

                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_RssoRaw,
                        NULL,
                        0
                    }
                );
            }
        }
        // YUV preview -> display client
        {
            Vector<MUINT32> clientMode;
            clientMode.push_back(IImgBufProvider::eID_DISPLAY);

            setCamClient(
                "Hal:Image:yuvDisp",
                eSTREAMID_IMAGE_PIPE_YUV_00,
                vImageParam,
                clientMode,
                GRALLOC_USAGE_HW_COMPOSER,
                MTRUE
            );
        }
        // YUV record
        {
            Vector<MUINT32> clientMode;
            clientMode.push_back(IImgBufProvider::eID_REC_CB);
            clientMode.push_back(IImgBufProvider::eID_PRV_CB);

            setCamClient(
                "Hal:Image:yuvRecord",
                eSTREAMID_IMAGE_PIPE_YUV_01,
                vImageParam,
                clientMode,
                GRALLOC_USAGE_HW_VIDEO_ENCODER
            );
        }
        // YUV FD & OT
        {
            Vector<MUINT32> clientMode;
            clientMode.push_back(IImgBufProvider::eID_FD);
            clientMode.push_back(IImgBufProvider::eID_OT);

            setCamClient(
                "Hal:Image:yuvFD",
                eSTREAMID_IMAGE_YUV_FD,
                vImageParam,
                clientMode
            );
        }

        pBuilder->setDst(vImageParam);
    }

#ifdef FEATURE_MODIFY
    // --- Update Hal3A Config param
    prepare3AInitParam(pImage_ResizedRaw->getImgSize(), mLPBConfigParams.hal3AParams);
    pBuilder->updateHal3AConfig( mLPBConfigParams.hal3AParams );
#endif

    mpPipeline = pBuilder->create();

    if ( mpPipeline == 0) {
        MY_LOGE("Fail to create Legacy Pipeline.");
        return BAD_VALUE;
    }

    if(!bHighSpeedMode)
    {
        sp<StreamBufferProvider> pTempConsumer = mpResourceContainer->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
        MY_LOGD("provider(%p)", pTempConsumer.get());
        //
        //
        if ( mNeedDumpIMGO )
        {
            pFullRawPool->allocateBuffer(
            pImage_FullRaw->getStreamName(),
            pImage_FullRaw->getMaxBufNum(),
            pImage_FullRaw->getMinInitBufNum()
                    );

            sp<ResultProcessor> pProcessor = mpPipeline->getResultProcessor().promote();
            pProcessor->registerListener(
                            eSTREAMID_META_HAL_DYNAMIC_P2,
                            pFullPair
                            );
        }
        //
        sp<ResultProcessor> pProcessor = mpPipeline->getResultProcessor().promote();
        pProcessor->registerListener(
                        eSTREAMID_META_APP_DYNAMIC_P1,
                        pFullPair
                        );
        pProcessor->registerListener(
                        eSTREAMID_META_HAL_DYNAMIC_P1,
                        pFullPair
                        );
        //
        sp<FrameInfo> pFrameInfo = new FrameInfo();
        mpResourceContainer->setLatestFrameInfo(pFrameInfo);
        pProcessor->registerListener(
                        eSTREAMID_META_APP_DYNAMIC_P1,
                        pFrameInfo
                        );
        pProcessor->registerListener(
                        eSTREAMID_META_HAL_DYNAMIC_P1,
                        pFrameInfo
                        );
#if DUALCAM
    pProcessor->registerListener(
                    eSTREAMID_META_APP_DYNAMIC_P2,
                    pFrameInfo
                    );
#endif // DualCam
    }
    //
    if ( mNeedDumpRRZO )
    {
        pResizedRawPool->allocateBuffer(
        pImage_ResizedRaw->getStreamName(),
        pImage_ResizedRaw->getMaxBufNum(),
        pImage_ResizedRaw->getMinInitBufNum()
                );

        sp<StreamBufferProvider> pTempConsumer =
            mpResourceContainer->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_RESIZER );

        if ( pTempConsumer != 0 ) {
            sp<ResultProcessor> pProcessor = mpPipeline->getResultProcessor().promote();
            pProcessor->registerListener(
                            eSTREAMID_META_HAL_DYNAMIC_P2,
                            pResizedPair
                            );
            // Need set Selector
            sp<DumpBufferSelector> pSelector = new DumpBufferSelector();
            pSelector->setDumpConfig( mOpenId );
            pTempConsumer->setSelector(pSelector);
            //
        }
    }

    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
constructZsdPreviewPipeline()
{
    FUNC_START;

    CAM_TRACE_NAME("DFC:constructZsdPreviewPipeline");

    mLPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_Feature_ZsdPreview;

    HwInfoHelper helper(mOpenId);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }

    mSensorParam.rawType = 1;

#if DUALCAM
    if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        mLPBConfigParams.enableUNI = DualCameraUtility::getProperty<DUALZOOM_FORCE_DISABLE_UNIT>() ? MFALSE : MTRUE;
        mLPBConfigParams.dualcamMode = Stereo::E_DUALCAM_FEATURE_ZOOM;

        MBOOL isEnableFrontalBinning =  (DualCameraHWHelper::getImgQualitySupportedByPlatform() || DualCameraUtility::getProperty<DUALZOOM_FORCE_ENABLE_FRONRALBIN>());
        mLPBConfigParams.disableFrontalBinning = isEnableFrontalBinning ? MFALSE : MTRUE;
    }
    else if(StereoSettingProvider::isDualCamMode())
    {
        // may denosie or vsdof
        mLPBConfigParams.enableUNI = MFALSE;
        mLPBConfigParams.dualcamMode = StereoSettingProvider::getStereoFeatureMode();
        mLPBConfigParams.disableFrontalBinning = MFALSE;
    }
#endif // DUALCAM

    mConstructVideoPipe = MFALSE;
    //
    if( mbNeedInitRequest && mInitRequest > 0 )
    {
        mLPBConfigParams.initRequest = mInitRequest;
    }
    //
    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if( helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit) )
    {
        using namespace NSCam::NSIoPipe::NSCamIOPipe;
        if(pipeBit & CAM_Pipeline_14BITS)
        {
            mLPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else
        {
            mLPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        if (DUALZOOM_FOV_MAX_FPS == 30)
        {
            mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        }
        else if (DUALZOOM_FOV_MAX_FPS == 24)
        {
            mSensorParam.mode = SENSOR_SCENARIO_ID_CUSTOM1;
        }
        else
        {
            MY_LOGW("Dualzoom not support fps : %d", DUALZOOM_FOV_MAX_FPS);
            mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        }
    }
    else
#endif // (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#if ((MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1))
    if(StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE|NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF)
       || StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
    {
        // need use capture sensor for zsd mode.
        mSensorParam.mode = getSensorMode(PipelineMode_ZSD);
    }
    else
#endif // ((MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1))
    {
        if (helper.get4CellSensorSupported())
        {
            mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        }
        else if(mbSwitchModeEnable &&
                mSwitchModeStatus != eSwitchMode_Undefined &&
                isHdrUiOn())
        {
            if(mSwitchModeStatus == eSwitchMode_HighLightMode)
            {
                mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
            }
            else if(mSwitchModeStatus == eSwitchMode_LowLightLvMode)
            {
                mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
            }
            else
            {
                mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
            }
        }
        else
        {
            mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        }
    }

#ifdef FEATURE_MODIFY
        if(getNowSensorModeStatusForSwitchFlow() == eSensorModeStatus_3HDR)  // now: iso < 2800 : 3hdr
        {
            String8 strStaticVhdrMode = mpParamsManagerV3->getParamsMgr()->getStaticVhdrMode();
            if(!strcmp(strStaticVhdrMode, MtkCameraParameters:: VIDEO_HDR_MODE_MVHDR))
            {
                mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_VIDEO_HDR_MODE, MtkCameraParameters::VIDEO_HDR_MODE_MVHDR);
                MY_LOGD("KEY_VIDEO_HDR_MODE = VIDEO_HDR_MODE_MVHDR");
            }
        }
        else if(getNowSensorModeStatusForSwitchFlow() == eSensorModeStatus_Binning)  // now: iso > 2800 : binning
        {
            String8 strStaticVhdrMode = mpParamsManagerV3->getParamsMgr()->getStaticVhdrMode();
            if(!strcmp(strStaticVhdrMode, MtkCameraParameters:: VIDEO_HDR_MODE_MVHDR))
            {
                mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_VIDEO_HDR_MODE, MtkCameraParameters::VIDEO_HDR_MODE_NONE);
                MY_LOGD("KEY_VIDEO_HDR_MODE = VIDEO_HDR_MODE_NONE");
            }
        }
        else
        {
            MY_LOGD("eSensorModeStatus_Disable, no need switch mode");
        }
        //
        if(mpHal3a)
        {
            mpHal3a->send3ACtrl(E3ACtrl_ResetGetISOThresStatus, 0, 0);
            MY_LOGD("E3ACtrl_ResetGetISOThresStatus");
        }
        //

        MUINT32 vhdrMode = mpParamsManagerV3->getParamsMgr()->getVHdr();
        if(vhdrMode != SENSOR_VHDR_MODE_NONE)
        {
            uint32_t supportVHDRMode = SENSOR_VHDR_MODE_NONE;
            if (! helper.querySupportVHDRMode(mSensorParam.mode, supportVHDRMode))
            {
                MY_LOGE("SensorMode(%d) querySupportVHDRMode fail", mSensorParam.mode);
                return BAD_VALUE;
            }
            if (vhdrMode != supportVHDRMode)
            {
                MY_LOGE("SensorMode(%d) cannot support VHDR(%d)", mSensorParam.mode, vhdrMode);
                return BAD_VALUE;
            }
        }
#endif // FEATURE_MODIFY

    if( ! helper.getSensorSize( mSensorParam.mode, mSensorParam.size) ||
        ! helper.getSensorFps( (MUINT32)mSensorParam.mode, (MINT32&)mSensorParam.fps) ||
        ! helper.queryPixelMode( mSensorParam.mode, mSensorParam.fps, mSensorParam.pixelMode)
      ) {
        MY_LOGE("cannot get params about sensor");
        return BAD_VALUE;
    }
    //
    if (helper.getPDAFSupported(mSensorParam.mode) && !CamManager::getInstance()->isMultiDevice())
    {
        mLPBConfigParams.disableFrontalBinning = MTRUE;
    }
    //
    if (helper.getDualPDAFSupported(mSensorParam.mode))
    {
        mLPBConfigParams.enableDualPD = MTRUE;
    }
    //
    MUINT32 const bitDepth = getPreviewRawBitDepth(helper);
    MSize previewsize;
    MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
    mpParamsManagerV3->getParamsMgr()->getPreviewSize(&previewsize.w, &previewsize.h);
#if DUALCAM
    if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        MSize previewsize_orig = previewsize;
        decideRrzoForDualZoom(mOpenId, previewsize, 1, 0);
        // for dual zoom featurepipe
        MINT main2;
        mpParamsManagerV3->getParamsMgr()->getDualZoomInfo(&main2, NULL);
        if (mOpenId != main2) {
            MSize previewsize2 = previewsize;
            decideRrzoForDualZoom(main2, previewsize2, 1, 0);
            MY_LOGD("zsdpreview wanted(%dx%d) 1(%dx%d) 2(%dx%d)",
                previewsize_orig.w, previewsize_orig.h,
                previewsize.w, previewsize.h,
                previewsize2.w, previewsize2.h
                );
            previewsize2.w = (previewsize2.w > previewsize.w)? previewsize2.w : previewsize.w;
            previewsize2.h = (previewsize2.h > previewsize.h)? previewsize2.h : previewsize.h;

            MSize raw = previewsize2;
            decideRrzoForStreaming(raw, mSensorParam.mode);
            mLPBConfigParams.streamingSize = raw;
            MY_LOGD("constructZsdPreviewPipeline (%dx%d)->(%dx%d)",
                previewsize2.w, previewsize2.h, raw.w, raw.h);
        }
    }
#endif // DUALCAM
#ifdef FEATURE_MODIFY
    mLPBConfigParams.enableRSS = this->useRSSO(mEisInfo.mode);
    mLPBConfigParams.enableFSC = MFALSE;
#endif // FEATURE_MODIFY
    //
    mLPBConfigParams.p1ConfigConcurrency = mP1NodeConcurrency;
    mLPBConfigParams.p2ConfigConcurrency = mP2NodeConcurrency;

    mpParamsManagerV3->getParamsMgr()->getPreviewSize(&mLPBConfigParams.previewSize.w, &mLPBConfigParams.previewSize.h);
    mpParamsManagerV3->getParamsMgr()->getVideoSize(&mLPBConfigParams.videoSize.w, &mLPBConfigParams.videoSize.h);

    //
    sp<LegacyPipelineBuilder> pBuilder =
        LegacyPipelineBuilder::createInstance(
                                    mOpenId,
                                    "ZSD",
                                    mLPBConfigParams);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return BAD_VALUE;
    }
    //
#ifdef FEATURE_MODIFY
    // Sensor
    mSensorParam.vhdrMode = mpParamsManagerV3->getParamsMgr()->getVHdr();
    MY_LOGD("VHDR sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d vhdrMode:%d",
        mSensorParam.mode,
        mSensorParam.rawType,
        mSensorParam.size.w, mSensorParam.size.h,
        mSensorParam.fps,
        mSensorParam.pixelMode,
        mSensorParam.vhdrMode);
#else
    // Sensor
    MY_LOGD("sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d",
        mSensorParam.mode,
        mSensorParam.rawType,
        mSensorParam.size.w, mSensorParam.size.h,
        mSensorParam.fps,
        mSensorParam.pixelMode);
#endif
    //
    MUINT featureCFG = 0;
    if (mLPBConfigParams.enableDualPD)
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_DUAL_PD);
    }
    if (mSensorParam.vhdrMode != SENSOR_VHDR_MODE_NONE)
    {
        if(mSensorParam.vhdrMode == SENSOR_VHDR_MODE_IVHDR)
            FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_IVHDR);
        if(mSensorParam.vhdrMode == SENSOR_VHDR_MODE_MVHDR)
            FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_MVHDR);
        if(mSensorParam.vhdrMode == SENSOR_VHDR_MODE_ZVHDR)
            FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_ZVHDR);
    }
#if DUALCAM
    if(StereoSettingProvider::getStereoFeatureMode() == Stereo::E_STEREO_FEATURE_DENOISE ||
       StereoSettingProvider::getStereoFeatureMode() == Stereo::E_DUALCAM_FEATURE_ZOOM)
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG, IScenarioControl::FEATURE_DUALZOOM_PREVIEW);
    }
    else if(StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE|NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF)
           || StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG, IScenarioControl::FEATURE_VSDOF_PREVIEW);
    }
#endif // DUALCAM
    //
    pBuilder->setSrc(mSensorParam);
    //

    sp<IScenarioControl> pScenarioCtrl = enterScenarioControl(getScenario(), mSensorParam.size, mSensorParam.fps, featureCFG);
    if( pScenarioCtrl.get() == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return BAD_VALUE;
    }
    pBuilder->setScenarioControl(pScenarioCtrl);


    // Image
    sp<PairMetadata>  pPair;
    sp<ZsdSelector> pSelector = new ZsdSelector();
    sp<BufferPoolImp> pFullRawPool;
    sp<IImageStreamInfo> pImage_Raw;
    sp<IImageStreamInfo> pImage_ResizedRaw;
    sp<PairMetadata>  pResizedPair;
    sp<BufferPoolImp> pResizedRawPool;
    const int ZSD_PREVIEW_MAX_IMGO_BUFFFER_NUM = 12;
    {
        Vector<PipelineImageParam> vImageParam;
        // RAW
        {
            CAM_TRACE_NAME("allocate RAW");

  #if 1
            //IMGO RAW
            if ( OK != decideImgoImage(
                            helper, bitDepth,
                            mSensorParam.size, usage,
                            0, ZSD_PREVIEW_MAX_IMGO_BUFFFER_NUM,
                            pImage_Raw, MTRUE
                        ))
            {
                MY_LOGE("No imgo image");
                return BAD_VALUE;
            }
            pPair = PairMetadata::createInstance(pImage_Raw->getStreamName());
            pFullRawPool = new BufferPoolImp(pImage_Raw);

            sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
            pFactory->setImageStreamInfo(pImage_Raw);
            pFactory->setPairRule(pPair, 2);
            pFactory->setUsersPool(pFullRawPool);
            sp<StreamBufferProvider> pProducer = pFactory->create();
            // Need set Selector
            pProducer->setSelector(pSelector);
            if(mNeedDumpIMGO)
            {
                pSelector->setDumpConfig( mOpenId );
            }
            //
            mpResourceContainer->setConsumer( pImage_Raw->getStreamId(), pProducer);

            vImageParam.push_back(
                PipelineImageParam{
                    pImage_Raw,
                    pProducer,
                    0
                }
            );
#endif

            int rrzo_max = 9; // default value, for general single cam used.
            int lcso_max = ZSD_PREVIEW_MAX_IMGO_BUFFFER_NUM;

#if MTKCAM_HAVE_MFB_SUPPORT
            // 6 frames for mfnr raw doamin bss cache frames.
            rrzo_max = rrzo_max + 6;
            MY_LOGD("rrzo_max(%d) updated for mfnr feature", rrzo_max);
            pSelector->sendCommand(ISelector::eCmd_setAllBuffersKept, 0, 0, 0);
#else
            rrzo_max += 3;
#endif

#if DUALCAM
            MUINT32 stereoMode = StereoSettingProvider::getStereoFeatureMode();
            if ( stereoMode == Stereo::E_STEREO_FEATURE_DENOISE)
            {
                // default dual cam rrzo size, 6 for pipeline p1+p2 and 6 for mfhr shot2shot
                rrzo_max = 12;
                MY_LOGD("rrzo_max(%d) updated by denoise mode", rrzo_max);
                pSelector->sendCommand(ISelector::eCmd_setAllBuffersKept, 0, 0, 0);
            }
            else if ( stereoMode == (NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE | NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF)
                      || stereoMode == NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP)
            {
                // for bokeh path, it nees 16 for rrzo
                rrzo_max = 16;
                MY_LOGD("rrzo_max(%d) updated by vsdof mode", rrzo_max);
                pSelector->sendCommand(ISelector::eCmd_setAllBuffersKept, 0, 0, 0);
            }
            else
            {
                // no special setting
            }
#endif
            //RRZO RAW
            if ( OK != decideRrzoImage(
                            helper, bitDepth,
                            previewsize, usage,
                            1, rrzo_max, //at least one rrzo buffer
                            pImage_ResizedRaw, MFALSE
                        ))
            {
                MY_LOGE("No rrzo image");
                return BAD_VALUE;
            }
            //
            if(mNeedDumpRRZO)
            {
                MY_LOGD("Open RRZO for dump mNeedDumpRRZO(%d)",mNeedDumpRRZO);
                pResizedPair = PairMetadata::createInstance(pImage_ResizedRaw->getStreamName());
                pResizedRawPool = new BufferPoolImp(pImage_ResizedRaw);
                //
                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                pFactory->setImageStreamInfo(pImage_ResizedRaw);
                pFactory->setPairRule(pResizedPair, 2);
                pFactory->setUsersPool(pResizedRawPool);
                sp<StreamBufferProvider> pProducer = pFactory->create();
                //
                mpResourceContainer->setConsumer( pImage_ResizedRaw->getStreamId(), pProducer );
                //
                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_ResizedRaw,
                        pProducer,
                        0
                    }
                );
            }
            else
            {
                pResizedRawPool = new BufferPoolImp(pImage_ResizedRaw);

                pResizedRawPool->allocateBuffer(
                    pImage_ResizedRaw->getStreamName(),
                    pImage_ResizedRaw->getMaxBufNum(),
                    pImage_ResizedRaw->getMinInitBufNum()
                );

                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                pFactory->setImageStreamInfo(pImage_ResizedRaw);
                pFactory->setPairRule(pPair, 2);
                pFactory->setUsersPool(pResizedRawPool);
                //
                sp<StreamBufferProvider> pProducer = pFactory->create();
                //
                pProducer->setSelector(pSelector);
                if(mNeedDumpIMGO)
                {
                    pSelector->setDumpConfig( mOpenId );
                }
                //
                mpResourceContainer->setConsumer( pImage_ResizedRaw->getStreamId(), pProducer );
                //
                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_ResizedRaw,
                        pProducer,
                        0
                    }
                );
            }
            // RAW (LCSO with provider)
            if (mLPBConfigParams.enableLCS)
            {
                MUINT32 const bitDepth = 10; // no use
                MSize anySize; // no use
                MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
                sp<IImageStreamInfo> pImage_Lcso;

                if ( OK != decideLcsoImage(
                            helper, bitDepth,
                            anySize, usage,
                            4, lcso_max + 3, // +3 for record
                            pImage_Lcso
                        ))
                {
                    MY_LOGE("No lcso image");
                    return BAD_VALUE;
                }

                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                pFactory->setImageStreamInfo(pImage_Lcso);
                // Need to set the same PairMetadata, and the PairMetadata need to wait 2 buffer
                pFactory->setPairRule(pPair, 2);
                sp<StreamBufferProvider> pProducer = pFactory->create();
                // Need set Selector
                pProducer->setSelector(pSelector);

                mpResourceContainer->setConsumer(pImage_Lcso->getStreamId(),pProducer);

                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_Lcso,
                        pProducer,
                        0
                    }
                );
            }
#if DUALCAM
    if ( stereoMode == (NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE | NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF)
              || stereoMode == NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP)
    {
        // cache imgo & rrzo fmt
        DualCameraHWHelper::setRawFormat(getOpenId(), mIMGOFmt, mRRZOFmt);
    }
#endif
#ifdef FEATURE_MODIFY
            // RSSO RAW
            if (mLPBConfigParams.enableRSS)
            {
                int mrrzoSize_Max = rrzo_max , mrrzoSize_Min = 0;
                sp<IImageStreamInfo> pImage_RssoRaw;

                if( OK != decideRssoImage(mrrzoSize_Min+1,
                                          mrrzoSize_Max+1,
                                          pImage_RssoRaw) )
                {
                    MY_LOGE("No rsso image");
                    return BAD_VALUE;
                }

                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_RssoRaw,
                        NULL,
                        0
                    }
                );
            }
#endif // FEATURE_MODIFY
        }
        auto setOuputProvider = [&vImageParam, this]()
        {
        // YUV preview -> display client
        {
            Vector<MUINT32> clientMode;
            clientMode.push_back(IImgBufProvider::eID_DISPLAY);

            setCamClient(
                "Hal:Image:yuvDisp",
                eSTREAMID_IMAGE_PIPE_YUV_00,
                vImageParam,
                clientMode,
                GRALLOC_USAGE_HW_COMPOSER,
                MTRUE
            );
        }
        // YUV preview callback -> preview callback client
        {
            Vector<MUINT32> clientMode;
            clientMode.push_back(IImgBufProvider::eID_PRV_CB);
            clientMode.push_back(IImgBufProvider::eID_GENERIC);

            setCamClient(
                "Hal:Image:yuvPrvCB",
                eSTREAMID_IMAGE_PIPE_YUV_01,
                vImageParam,
                clientMode
            );
        }
        // YUV FD & OT
        {
            Vector<MUINT32> clientMode;
            clientMode.push_back(IImgBufProvider::eID_FD);
            clientMode.push_back(IImgBufProvider::eID_OT);

            setCamClient(
                "Hal:Image:yuvFD",
                eSTREAMID_IMAGE_YUV_FD,
                vImageParam,
                clientMode
            );
        }
        };
#if DUALCAM
        if(StereoSettingProvider::getStereoFeatureMode() == (Stereo::E_STEREO_FEATURE_CAPTURE|Stereo::E_STEREO_FEATURE_VSDOF)
        || StereoSettingProvider::getStereoFeatureMode() == (Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
        {
            // for main2 no need to set output buffer
            if(isMain1Pipeline())
            {
                setOuputProvider();
            }
        }
        else
        {
            setOuputProvider();
        }
#else
        setOuputProvider();
#endif
#if DUALCAM
        if(StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE|NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF)
           || StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
        {
            if(isMain1Pipeline())
            {
                MY_LOGD("allocate working buffer for bokeh");
                {
                    Pass2SizeInfo pass2SizeInfo;
                    StereoSizeProvider::getInstance()->getPass2SizeInfo(
                                                            PASS2A,
                                                            StereoHAL::eSTEREO_SCENARIO_PREVIEW,
                                                            pass2SizeInfo);
                    addYUVHalPool(
                        "Hal:Image:MainImage",
                        eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV,
                        10, 1,
                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE,
                        eImgFmt_YV12,
                        pass2SizeInfo.areaWDMA.size,
                        vImageParam
                    );
                }
                {
                    ENUM_BUFFER_NAME bufferType;
                    StreamId_T streamid;
                    if(StereoSettingProvider::getStereoFeatureMode() == NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP)
                    {
                        bufferType = E_DEPTH_MAP;
                        streamid = eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV;
                    }
                    else
                    {
                        bufferType = E_DMBG;
                        streamid = eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DMBGYUV;
                    }
                    StereoArea imgSize =
                                StereoSizeProvider::getInstance()->getBufferSize(
                                                            bufferType,
                                                            StereoHAL::eSTEREO_SCENARIO_PREVIEW);
                    addYUVHalPool(
                        "Hal:Image:Y8_01",
                        streamid,
                        10, 1,
                        eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN,
                        eImgFmt_STA_BYTE,
                        imgSize.size,
                        vImageParam
                    );
                }
            }
        }
#endif

        pBuilder->setDst(vImageParam);
    }

#ifdef FEATURE_MODIFY
    // --- Update Hal3A Config param
    prepare3AInitParam(pImage_ResizedRaw->getImgSize(), mLPBConfigParams.hal3AParams);
    pBuilder->updateHal3AConfig( mLPBConfigParams.hal3AParams );
#endif

    sp< ILegacyPipeline > pPipeline = pBuilder->create();
    if ( pPipeline == 0) {
        if(mbConstruct2ndPipeline)
        {
            mpSecondPipeline = NULL;
            MY_LOGE("Fail to create Legacy Pipeline. (mpSecondPipeline)");
        }
        else
        {
            mpPipeline = NULL;
            MY_LOGE("Fail to create Legacy Pipeline.");
        }
        return BAD_VALUE;
    }

    //
    pFullRawPool->allocateBuffer(
            pImage_Raw->getStreamName(),
            pImage_Raw->getMaxBufNum(),
            pImage_Raw->getMinInitBufNum()
            );

#if 1
    sp<StreamBufferProvider> pTempConsumer =
        mpResourceContainer->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );

    if ( pTempConsumer != 0 ) {
        CAM_TRACE_NAME("DFC:SetSelector");
        sp<ResultProcessor> pProcessor = pPipeline->getResultProcessor().promote();
        pProcessor->registerListener(
                        eSTREAMID_META_APP_DYNAMIC_P1,
                        pPair
                        );
        pProcessor->registerListener(
                        eSTREAMID_META_HAL_DYNAMIC_P1,
                        pPair
                        );
        //
        sp<FrameInfo> pFrameInfo = new FrameInfo();
        mpResourceContainer->setLatestFrameInfo(pFrameInfo);
        pProcessor->registerListener(
                        eSTREAMID_META_APP_DYNAMIC_P1,
                        pFrameInfo
                        );
        pProcessor->registerListener(
                        eSTREAMID_META_HAL_DYNAMIC_P1,
                        pFrameInfo
                        );

#if DUALCAM
        pProcessor->registerListener(
                        eSTREAMID_META_APP_DYNAMIC_P2,
                        pFrameInfo
                        );
#endif // DualCam

    }
#endif

    if ( mNeedDumpRRZO )
    {
        pResizedRawPool->allocateBuffer(
        pImage_ResizedRaw->getStreamName(),
        pImage_ResizedRaw->getMaxBufNum(),
        pImage_ResizedRaw->getMinInitBufNum()
                );

        sp<StreamBufferProvider> pTempConsumer =
            mpResourceContainer->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_RESIZER );

        if ( pTempConsumer != 0 ) {
            sp<ResultProcessor> pProcessor = pPipeline->getResultProcessor().promote();
            pProcessor->registerListener(
                            eSTREAMID_META_HAL_DYNAMIC_P2,
                            pResizedPair
                            );
            pProcessor->registerListener(
                            eSTREAMID_META_APP_DYNAMIC_P1,
                            pResizedPair
                            );
            // Need set Selector
            sp<DumpBufferSelector> pSelector = new DumpBufferSelector();
            pSelector->setDumpConfig( mOpenId );
            pTempConsumer->setSelector(pSelector);
            //
        }
    }
    //
    if(mbConstruct2ndPipeline)
    {
        mpSecondPipeline = pPipeline;
        MY_LOGD("create mpSecondPipeline");
    }
    else
    {
        mpPipeline = pPipeline;
        MY_LOGD("create mpPipeline");
    }
    //
    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
FeatureFlowControl::
getScenario() const
{
    switch(mLPBConfigParams.mode)
    {
        case LegacyPipelineMode_T::PipelineMode_Preview:
        case LegacyPipelineMode_T::PipelineMode_Feature_Preview:
            return IScenarioControl::Scenario_NormalPreivew;
        case LegacyPipelineMode_T::PipelineMode_ZsdPreview:
        case LegacyPipelineMode_T::PipelineMode_Feature_ZsdPreview:
            return IScenarioControl::Scenario_ZsdPreview;
        case LegacyPipelineMode_T::PipelineMode_VideoRecord:
        case LegacyPipelineMode_T::PipelineMode_Feature_VideoRecord:
            return IScenarioControl::Scenario_VideoRecord;
        case LegacyPipelineMode_T::PipelineMode_HighSpeedVideo:
        case LegacyPipelineMode_T::PipelineMode_Feature_HighSpeedVideo:
            return IScenarioControl::Scenario_HighSpeedVideo;
        default:
            MY_LOGW("no mapped scenario for mode %d", mLPBConfigParams.mode);
            break;
    }
    return IScenarioControl::Scenario_None;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeatureFlowControl::
useRSSO(MUINT32 eisMode) const
{
#if 1
    MINT32 rssoff = ::property_get_int32("vendor.debug.camera.rssoff", 0);
    if (rssoff > 0) {
        MY_LOGI("Debug: useRSSO = MFALSE");
        return MFALSE;
    }
#endif
    MBOOL ret = MFALSE;
    if( EIS_MODE_IS_EIS_30_ENABLED(eisMode) && EIS_MODE_IS_EIS_IMAGE_ENABLED(eisMode) )
    {
        MY_LOGD("use RSSO for EIS");
        ret = MTRUE;
    }

#ifdef FEATURE_MODIFY
    if( !ret && ::strcmp(mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_3DNR_MODE), "on") == 0 )
    {
        MUINT32 nr3d_mask = NR3DCustom::USAGE_MASK_NONE;

        if( !mpDeviceHelper->isFirstUsingDevice() )
        {
            nr3d_mask |= NR3DCustom::USAGE_MASK_MULTIUSER;
        }
#if DUALCAM
        if( StereoSettingProvider::isDualCamMode() )
        {
            nr3d_mask |= NR3DCustom::USAGE_MASK_DUAL_ZOOM;
        }
#endif // DUALCAM
        if( mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_HighSpeedVideo )
        {
            nr3d_mask |= NR3DCustom::USAGE_MASK_HIGHSPEED;
        }

        // turn on RSSO if NR3D support RSC
        if( NR3DCustom::isEnabledRSC(nr3d_mask) )
        {
            MY_LOGD("use RSSO for 3DNR");
            ret = MTRUE;
        }
    }
#endif // FEATURE_MODIFY

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeatureFlowControl::
useFSC() const
{
    MBOOL ret = MFALSE;

    if( FSCCustom::isSupportFSC() )
    {
        MUINT32 fsc_mask = FSCCustom::USAGE_MASK_NONE;
        MBOOL support_AF = MFALSE;

        if( !mpDeviceHelper->isFirstUsingDevice() )
        {
            fsc_mask |= FSCCustom::USAGE_MASK_MULTIUSER;
        }
#if DUALCAM
        if( StereoSettingProvider::isDualCamMode() )
        {
            fsc_mask |= FSCCustom::USAGE_MASK_DUAL_ZOOM;
        } else
#endif // DUALCAM
        {
            NS3Av3::IHal3A* pHal3A = MAKE_Hal3A(getOpenId(), LOG_TAG);
            NS3Av3::FeatureParam_T r3ASupportedParam;
            if(pHal3A && pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetSupportedInfo, reinterpret_cast<MINTPTR>(&r3ASupportedParam), 0))
            {
                support_AF = (r3ASupportedParam.u4MaxFocusAreaNum > 0);
            }
            else {
                MY_LOGW("Cannot query AF ability from 3A");
            }
        }

        if( mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_HighSpeedVideo )
        {
            fsc_mask |= FSCCustom::USAGE_MASK_HIGHSPEED;
        }


        if( FSCCustom::isEnabledFSC(fsc_mask) && support_AF)
        {
            ret = MTRUE;
        }
        MY_LOGD("support FSC(%d) mask(%d) support_AF(%d) EISmode(%d)", ret, fsc_mask, support_AF, mEisInfo.mode);
    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FeatureFlowControl::
setEISParameter()
{
    if( mpParamsManagerV3->getParamsMgr()->getVideoStabilization() )
    {
        if( EIS_MODE_IS_EIS_QUEUE_ENABLED(mEisInfo.mode) )
        {
            MUINT32 eis_supFrms = mEisInfo.queueSize;

            mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_EIS25_MODE, 1);
            mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_EIS_SUPPORTED_FRAMES, eis_supFrms);
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
decideSensorModeAndRrzo(
    MINT    openId,
    MSize&  recordsize,
    MUINT&  sensorMode,
    MBOOL   bHighSpeedMode
)
{
    MSize paramSize, wantedSize, sensorSize;
    MSize previewModeSize = MSize(0,0);
    HwInfoHelper helper(openId);
    int main2 = -1;
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT==1)
    int FOVMargin = DUALZOOM_FOV_MARGIN; // 6 percent
    FOVHal::Configs Config;
#else
    int FOVMargin = 0;
#endif
    bool isEisOn = mpParamsManagerV3->getParamsMgr()->getVideoStabilization();

    //
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    mpParamsManagerV3->getParamsMgr()->getVideoSize(&paramSize.w, &paramSize.h);
    if( paramSize.w*paramSize.h > IMG_1080P_SIZE )
    {
        mb4K2KVideoRecord = MTRUE;
    }
    else
    {
        mb4K2KVideoRecord = MFALSE;
    }
    //

    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT==1)
    if (isEisOn)
    {
        FOVMargin = DUALZOOM_FOV_MARGIN_COMBINE_EIS;
    }
    if (DUALZOOM_FOV_APPLIED_CAM != openId)
    {
        MY_LOGD("no need FOV margin");
        FOVMargin = 0;
    }
    #endif

    mOrignalRRZOSize = wantedSize = paramSize;

    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT==1)
    if (DUALZOOM_FOV_APPLIED_CAM == openId &&
        Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        Config.CamID = openId;
        Config.Is4K2K = mb4K2KVideoRecord;
        Config.EisFactor = 0;
        Config.MainStreamingSize = paramSize;

        FOVHal::getInstance()->setPipelineConfig(Config);
    }
    #endif

    if(!bHighSpeedMode)
    {
        if( isEisOn )
        {
#ifdef FEATURE_MODIFY

            /* Determine EIS version
             */
            MUINT32 eis_mask = EISCustom::USAGE_MASK_NONE;

            // Check video type
            if( mpParamsManagerV3->getParamsMgr()->getVHdr() != SENSOR_VHDR_MODE_NONE )
            {
                eis_mask |= EISCustom::USAGE_MASK_VHDR;
            }
            if( (paramSize.w*paramSize.h) >= (VR_UHD_W*VR_UHD_H) )
            {
                eis_mask |= EISCustom::USAGE_MASK_4K2K;
            }
            if( !mpDeviceHelper->isFirstUsingDevice() )
            {
                eis_mask |= EISCustom::USAGE_MASK_MULTIUSER;
            }
#if DUALCAM
            if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
            {
                eis_mask |= EISCustom::USAGE_MASK_DUAL_ZOOM;
            }
#endif // DUALCAM
            mEisInfo.mode = EISCustom::getEISMode(eis_mask);
            mLPBConfigParams.eisMode = mEisInfo.mode;
#endif // FEATURE_MODIFY
            if( this->useFSC() && EIS_MODE_IS_EIS_30_ENABLED(mEisInfo.mode) )
            {
                eis_mask |= EISCustom::USAGE_MASK_FSC;
            }
            MUINT32 srcType = mb4K2KVideoRecord ?
                              EISCustom::VIDEO_CFG_4K2K : EISCustom::VIDEO_CFG_FHD;
            MUINT32 eis_factor = EIS_MODE_IS_EIS_12_ENABLED(mEisInfo.mode) ?
                                 EISCustom::getEIS12Factor() : EISCustom::getEISFactor(srcType, eis_mask);

            mEisInfo.factor = eis_factor;
            mEisInfo.videoConfig = srcType;
            mEisInfo.queueSize = EISCustom::getForwardFrames(srcType);
            mEisInfo.startFrame = EISCustom::getForwardStartFrame();

            mLPBConfigParams.packedEisInfo = mEisInfo.toPackedData();
#if DUALCAM
            if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
            {
                eis_factor += FOVMargin;
                #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT==1)
                if (FOVMargin != 0)
                {
                    Config.EisFactor = eis_factor;
                    FOVHal::getInstance()->setPipelineConfig(Config);
                }
                #endif
            }
#endif // DUALCAM

            if( wantedSize.w < 1920 )
            {
                wantedSize.w = 1920;
            }
            if( wantedSize.h < 1080 )
            {
                wantedSize.h = 1080;
            }
            if( EISCustom::isEnabledLosslessMode() )
            {
                wantedSize.w *= eis_factor/100.0f;//for EIS
                wantedSize.h *= eis_factor/100.0f;//for EIS
            }
        }
#if DUALCAM
        else if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
        {
            wantedSize.w *= (100 + FOVMargin)/100.0f;
            wantedSize.h *= (100 + FOVMargin)/100.0f;
        }
#endif // DUALCAM

// dual zoom set 24fps
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    if(Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        if (DUALZOOM_FOV_MAX_FPS == 30)
        {
            sensorMode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
        }
        else if (DUALZOOM_FOV_MAX_FPS == 24)
        {
            sensorMode = SENSOR_SCENARIO_ID_CUSTOM1;
        }
        else
        {
            MY_LOGW("Dualzoom not support fps : %d", DUALZOOM_FOV_MAX_FPS);
            sensorMode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
        }
    }
    else
#endif // (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#if ((MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1))
    if(StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE|NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF)
       || StereoSettingProvider::getStereoFeatureMode() == (NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
    {
        mSensorParam.mode = getSensorMode(PipelineMode_PREVIEW);
    }
    else
#endif // ((MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1))
    {
        sensorMode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
    }
#if DUALCAM
        if( helper.getSensorSize( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, previewModeSize) &&
            wantedSize.w <= previewModeSize.w &&
            wantedSize.h <= previewModeSize.h &&
            Stereo::E_DUALCAM_FEATURE_ZOOM != StereoSettingProvider::getStereoFeatureMode())
        {
            sensorMode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        }
#else
        if( helper.getSensorSize( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, previewModeSize) &&
            wantedSize.w <= previewModeSize.w &&
            wantedSize.h <= previewModeSize.h )
        {
            sensorMode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        }
#endif
    }
    //
    if( !helper.getSensorSize( sensorMode, sensorSize) )
    {
        MY_LOGE("cannot get params about sensor");
        return BAD_VALUE;
    }
    MY_LOGI("wanted(%dx%d), previewMode(%dx%d), mode(%d), sensorSize(%dx%d)",
            wantedSize.w,
            wantedSize.h,
            previewModeSize.w,
            previewModeSize.h,
            sensorMode,
            sensorSize.w,
            sensorSize.h);
    //
    recordsize.w = wantedSize.w;
    recordsize.h = wantedSize.h;
    if( wantedSize.w < 1920 )
    {
        recordsize.w = 1920;
    }
    if( wantedSize.h < 1080 )
    {
        recordsize.h = 1080;
    }
    MY_LOGI("record(%dx%d)",
                recordsize.w, recordsize.h);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
decideRrzoForDualZoom(
    MINT    openId,
    MSize&  rawSize,
    MBOOL   bDualZoomMode,
    MBOOL   bIsVideoMode
)
{
    (void)openId;
    (void)rawSize;
    (void)bDualZoomMode;
    (void)bIsVideoMode;
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    MSize paramSize, wantedSize, sensorSize;
    HwInfoHelper helper(openId);
    FOVHal::Configs Config;
    //
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }

    if (DUALZOOM_FOV_APPLIED_CAM != openId)
    {
        MY_LOGD("Main1 does not apply FOV, no need FOV margin, but config sensor ratio");
        sensorSize = mSensorParam.size;
        rawSize.h = mSensorParam.size.h * rawSize.w / mSensorParam.size.w;
        return OK;
    }
    {
        Config.CamID = openId;
        Config.Is4K2K = mb4K2KVideoRecord;
        Config.EisFactor = 0;
        Config.MainStreamingSize = rawSize;

        FOVHal::getInstance()->setPipelineConfig(Config);
    }


    if(mb4K2KVideoRecord)
        return OK;
    //
    paramSize = rawSize;
    if (paramSize.w == 0 || paramSize.h == 0)
    {
        if (!bIsVideoMode)
            mpParamsManagerV3->getParamsMgr()->getPreviewSize(&paramSize.w, &paramSize.h);
        else
            mpParamsManagerV3->getParamsMgr()->getVideoSize(&paramSize.w, &paramSize.h);
    }
    //
    sensorSize = mSensorParam.size;
    paramSize.h = mSensorParam.size.h * paramSize.w / mSensorParam.size.w;
    wantedSize = paramSize;

    Config.MainStreamingSize = wantedSize;
    FOVHal::getInstance()->setPipelineConfig(Config);

    rawSize.w = wantedSize.w * (100 + DUALZOOM_FOV_MARGIN)/100;
    rawSize.h = wantedSize.h * (100 + DUALZOOM_FOV_MARGIN)/100;

    MY_LOGI("rawsize(%dx%d), sensorSize(%dx%d)",
            rawSize.w,
            rawSize.h,
            sensorSize.w,
            sensorSize.h);
#endif
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
decideRrzoForStreaming(
    MSize&  rawSize,
    MINT    mode
)
{
    (void)rawSize;
    (void)mode;
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    int main2 = -1;
    mpParamsManagerV3->getParamsMgr()->getDualZoomInfo(&main2, NULL);
    if (main2 == mOpenId)
    {
        MY_LOGD("Only check in main1 flow");
        return OK;
    }

    // Idea
    // 1. check sensor size of main1 and main2
    // if main2 > main1, expand the buffer according to the ratio of main2/main1

    HwInfoHelper helper1(mOpenId);
    if(! helper1.updateInfos() ) {
       MY_LOGE("cannot properly update main1 infos");
       return BAD_VALUE;
    }

    HwInfoHelper helper2(main2);
    if (! helper2.updateInfos() ) {
       MY_LOGE("cannot properly update main2 infos");
       return BAD_VALUE;
    }

    MSize sensorSize1, sensorSize2, maxSensorSize;
    helper1.getSensorSize(mode, sensorSize1);
    helper2.getSensorSize(mode, sensorSize2);
    maxSensorSize = sensorSize1;
    if (sensorSize2.w > sensorSize1.w) {
        maxSensorSize.w = sensorSize2.w;
    }
    if (sensorSize2.h > sensorSize1.h) {
        maxSensorSize.h = sensorSize2.h;
    }

    // 2. compute the actual image size for new main1 buffer
    MUINT32 const bitDepth = getPreviewRawBitDepth(helper1);
    MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
    MINT format;
    size_t stride;
    MSize actualSize;
    if( ! helper1.getRrzoFmt(bitDepth, format) ||
        ! helper1.alignRrzoHwLimitation(rawSize, maxSensorSize, actualSize) ||
        ! helper1.alignPass1HwLimitation(mSensorParam.pixelMode, format, false, actualSize, stride) )
    {
        MY_LOGE("wrong params about rrzo for streaming");
        return BAD_VALUE;
    }
    if (rawSize.size() <= actualSize.size()) {
        rawSize = actualSize;
    } else {
        MY_LOGD("actualsize(%dx%d) too small, keep original size(%dx%d)",
            actualSize.w, actualSize.h, rawSize.w, rawSize.h);
    }
#endif
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeatureFlowControl::
isMain1Pipeline()
{
#if DUALCAM
    MINT32 main1Id = -1, main2Id = -1;
    MBOOL isMain1 = MFALSE;
    {
        // get Main1 & Main2 id
        if(!StereoSettingProvider::getStereoSensorIndex(main1Id, main2Id))
        {
            MY_LOGE("fail to get sensor id");
            return MFALSE;
        }
        (main1Id == mOpenId) ? isMain1 = MTRUE : isMain1 = MFALSE;
    }
    return isMain1;
#else
    return MTRUE;
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32
FeatureFlowControl::
getSensorMode(
    MUINT32 pipelineMode
)
{
#if DUALCAM
    MBOOL bMain1 = isMain1Pipeline();
    MUINT32 sensormode_main1, sensormode_main2;
    StereoSettingProvider::getSensorScenario(
                                             StereoSettingProvider::getStereoFeatureMode(),
                                             StereoSettingProvider::getStereoModuleType(),
                                             pipelineMode,
                                             sensormode_main1,
                                             sensormode_main2);
    StereoSettingProvider::updateSensorScenario(sensormode_main1,
                                                sensormode_main2);
    if(bMain1)
    {
        MY_LOGD("[%d] sensor mode(%d)", getOpenId(), sensormode_main1);
        return sensormode_main1;
    }
    else
    {
        MY_LOGD("[%d] sensor mode(%d)", getOpenId(), sensormode_main2);
        return sensormode_main2;
    }
#else
    MY_LOGD("[%d] sensor mode(%d)", getOpenId(), SENSOR_SCENARIO_ID_NORMAL_PREVIEW);
    return SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeatureFlowControl::
prepare3AInitParam(const MSize &streamSize, LegacyPipelineBuilder::Hal3AParams &hal3AParam)
{
    // HDR Mode
    HDRMode kHDRMode = mpParamsManagerV3->getParamsMgr()->getHDRMode();
    hal3AParam.setHdrMode(static_cast<MUINT8>(kHDRMode));

    // ISP Profile
    MUINT8 profile = 0;
    if(getIspProfile(streamSize, kHDRMode, profile) && profile != 0)
    {
        hal3AParam.setIspProfile(profile);
    }

    return MTRUE;
}
