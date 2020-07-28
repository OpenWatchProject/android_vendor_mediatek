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

#define LOG_TAG "MtkCam/DefaultFlowControl"
//
#include "MyUtils.h"
//
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>

#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include "DefaultFlowControl.h"
#include <buffer/ClientBufferPool.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>

#warning "FIXME"
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/Selector.h>

#include <math.h>
#include <camera_custom_isp_limitation.h>
#include <camera_custom_eis.h>
#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/utils/hw/CamManager.h>
#include <mtkcam/feature/eis/EisInfo.h>
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
MERROR
DefaultFlowControl::
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
    MY_LOGI("rrzo num:%d-%d bitDepth:%d format:%d referenceSize:%dx%d, actual size:%dx%d, stride:%d",
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
DefaultFlowControl::
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
    MY_LOGD("imgo num:%d-%d bitDepth:%d format:%d referenceSize:%dx%d, actual size:%dx%d, stride:%d",
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
DefaultFlowControl::
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


    mvwpClientBufferPool.push_back(pClient);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultFlowControl::
constructNormalPreviewPipeline()
{
    FUNC_START;

    CAM_TRACE_NAME("DFC:constructNormalPreviewPipeline");

    mvwpClientBufferPool.clear();

    mLPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_Preview;

    HwInfoHelper helper(mOpenId);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    if( ! helper.getSensorSize( mSensorParam.mode, mSensorParam.size) ||
        ! helper.getSensorFps( (MUINT32)mSensorParam.mode, (MINT32&)mSensorParam.fps) ||
        ! helper.queryPixelMode( mSensorParam.mode, mSensorParam.fps, mSensorParam.pixelMode)
      ) {
        MY_LOGE("cannot get params about sensor");
        return BAD_VALUE;
    }
    //
    if (helper.getDualPDAFSupported(mSensorParam.mode))
    {
        mLPBConfigParams.enableDualPD = MTRUE;
    }
    //
    {
        MSize previewsize;
        mpParamsManagerV3->getParamsMgr()->getPreviewSize(&previewsize.w, &previewsize.h);
        if ((helper.getPDAFSupported(mSensorParam.mode) && !CamManager::getInstance()->isMultiDevice())
            || isSpecialSensorNeedDisableFrontalbinning(mSensorParam.mode, previewsize.w))
        {
            mLPBConfigParams.disableFrontalBinning = MTRUE;
        }
    }
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
     //
    // Sensor
    MY_LOGD("sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d",
        mSensorParam.mode,
        mSensorParam.rawType,
        mSensorParam.size.w, mSensorParam.size.h,
        mSensorParam.fps,
        mSensorParam.pixelMode);
    //
    sp<LegacyPipelineBuilder> pBuilder =
        LegacyPipelineBuilder::createInstance(
                                    mOpenId,
                                    "NormalPreview",
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
    //
    sp<IScenarioControl> pScenarioCtrl = enterScenarioControl(getScenario(), mSensorParam.size, mSensorParam.fps, featureCFG);
    if( pScenarioCtrl.get() == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return BAD_VALUE;
    }
    pBuilder->setScenarioControl(pScenarioCtrl);

    // Image
    {
        CAM_TRACE_NAME("DFC:SetImageDst");
        Vector<PipelineImageParam> vImageParam;
        // RAW (RRZO)
        {
            MUINT32 const bitDepth = getPreviewRawBitDepth(helper);
            MSize previewsize;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
            mpParamsManagerV3->getParamsMgr()->getPreviewSize(&previewsize.w, &previewsize.h);
            sp<IImageStreamInfo> pImage_Raw;

            if( !helper.hasRrzo() )
            {
                if ( OK != decideImgoImage(
                                helper, bitDepth,
                                mSensorParam.size, usage,
                                2, 8,
                                pImage_Raw, MTRUE
                            ))
                {
                    MY_LOGE("No imgo image");
                    return BAD_VALUE;
                }
            }
            else
            {
                if ( OK != decideRrzoImage(
                                helper, bitDepth,
                                previewsize, usage,
                                2, 8,
                                pImage_Raw, MFALSE
                            ))
                {
                    MY_LOGE("No rrzo image");
                    return BAD_VALUE;
                }
            }

            vImageParam.push_back(
                PipelineImageParam{
                    pImage_Raw,
                    NULL,
                    0
                }
            );
        }
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
                clientMode,
                0,
                MTRUE
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
    //


    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultFlowControl::
constructRecordingPipeline()
{
    FUNC_START;
    MBOOL bHighSpeedMode = MFALSE;
    MUINT previewMaxFps = 0;
    HwInfoHelper helper(mOpenId);

    mvwpClientBufferPool.clear();

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
        mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
    }
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
    previewMaxFps =     (mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= mSensorParam.fps) ?
                        mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) : mSensorParam.fps;
    mpParamsManagerV3->setPreviewMaxFps(previewMaxFps);
    if(previewMaxFps > HighSpeedVideoFpsBase)
    {
        bHighSpeedMode = MTRUE;
    }
    //
    MSize recordsize = MSize(0,0);
    if( decideSensorModeAndRrzo(recordsize, mSensorParam.mode, bHighSpeedMode) != OK )
    {
        return BAD_VALUE;
    }
    //
    if (helper.getPDAFSupported(mSensorParam.mode) && !CamManager::getInstance()->isMultiDevice())
    {
        mLPBConfigParams.disableFrontalBinning = MTRUE;
    }
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
        mLPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_VideoRecord;
    }

    sp<LegacyPipelineBuilder> pBuilder =
        LegacyPipelineBuilder::createInstance(
                                    mOpenId,
                                    "NormalRecord",
                                    mLPBConfigParams);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return BAD_VALUE;
    }
    // Sensor
    MY_LOGD("sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d",
        mSensorParam.mode,
        mSensorParam.rawType,
        mSensorParam.size.w, mSensorParam.size.h,
        mSensorParam.fps,
        mSensorParam.pixelMode);
    pBuilder->setSrc(mSensorParam);
    //
    MUINT featureCFG = 0;
    if (mLPBConfigParams.enableDualPD)
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_DUAL_PD);
    }
    //
    MSize videoSize = MSize(0,0);
    mpParamsManagerV3->getParamsMgr()->getVideoSize(&videoSize.w,&videoSize.h);
    MINT32 camMode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE); //if camMode = 0 => 3rd AP
    //
    sp<IScenarioControl> pScenarioCtrl = enterScenarioControl(getScenario(), mSensorParam.size, mSensorParam.fps, featureCFG, videoSize, camMode);
    if( pScenarioCtrl.get() == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return BAD_VALUE;
    }
    pBuilder->setScenarioControl(pScenarioCtrl);

    // Image
    sp<PairMetadata> pPair;
    sp<VssSelector> pSelector = new VssSelector();
    int mrrzoSize_Max = 0, mrrzoSize_Min = 0;
    {
        CAM_TRACE_NAME("DFC:SetImageDst");
        Vector<PipelineImageParam> vImageParam;
        // RAW
        {
            MUINT32 const bitDepth = getPreviewRawBitDepth(helper);
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
            sp<IImageStreamInfo> pImage_Raw;

            if(bHighSpeedMode)
            {
                #warning "TODO: Check how to decide RRZO buffer count for high speed mode"
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
            //
            if( !helper.hasRrzo() )
            {
                if( bHighSpeedMode )
                {
                    if ( OK != decideImgoImage(
                                    helper, bitDepth,
                                    mSensorParam.size, usage,
                                    mrrzoSize_Min, mrrzoSize_Max,
                                    pImage_Raw, MTRUE
                                ))
                    {
                        MY_LOGE("No imgo image");
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
                else
                {
                    // for VR+VSS
                    if ( OK != decideImgoImage(
                                    helper, bitDepth,
                                    mSensorParam.size, usage,
                                    mrrzoSize_Min, mrrzoSize_Max,
                                    pImage_Raw, MTRUE
                                ))
                    {
                        MY_LOGE("No imgo image");
                        return BAD_VALUE;
                    }
                    pPair = PairMetadata::createInstance(pImage_Raw->getStreamName());

                    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                    pFactory->setImageStreamInfo(pImage_Raw);
                    pFactory->setPairRule(pPair, 2);
                    sp<StreamBufferProvider> pProducer = pFactory->create();
                    // Need set Selector
                    pProducer->setSelector(pSelector);

                    mpResourceContainer->setConsumer(pImage_Raw->getStreamId(),pProducer);

                    vImageParam.push_back(
                        PipelineImageParam{
                            pImage_Raw,
                            pProducer,
                            0
                        }
                    );
                }
            }
            else
            {
                if ( OK != decideRrzoImage(
                                helper, bitDepth,
                                recordsize, usage,
                                mrrzoSize_Min, mrrzoSize_Max,
                                pImage_Raw, MFALSE
                            ))
                {
                    MY_LOGE("No rrzo image");
                    return BAD_VALUE;
                }

                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_Raw,
                        NULL,
                        0
                    }
                );

                if(!bHighSpeedMode)
                {
                    // for VSS
                    if ( OK != decideImgoImage(
                                    helper, bitDepth,
                                    mSensorParam.size, usage,
                                    1, 1,
                                    pImage_Raw, MTRUE
                                ))
                    {
                        MY_LOGE("No imgo image");
                        return BAD_VALUE;
                    }
                    pPair = PairMetadata::createInstance(pImage_Raw->getStreamName());

                    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                    pFactory->setImageStreamInfo(pImage_Raw);
                    pFactory->setPairRule(pPair, 2);
                    sp<StreamBufferProvider> pProducer = pFactory->create();
                    // Need set Selector
                    pProducer->setSelector(pSelector);

                    mpResourceContainer->setConsumer(pImage_Raw->getStreamId(),pProducer);

                    vImageParam.push_back(
                        PipelineImageParam{
                            pImage_Raw,
                            pProducer,
                            0
                        }
                    );
                }
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
                            mrrzoSize_Min, mrrzoSize_Max + 3,   // in VR, LCSO buffer need more than RRZO, +3 is the tunning result for 4K VSS case
                            pImage_Raw
                        ))
                {
                    MY_LOGE("No lcso image");
                    return BAD_VALUE;
                }

                if(!bHighSpeedMode)
                {
                    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                    pFactory->setImageStreamInfo(pImage_Raw);
                    // Need to set the same PairMetadata, and the PairMetadata need to wait 2 buffer
                    pFactory->setPairRule(pPair, 2);
                    sp<StreamBufferProvider> pProducer = pFactory->create();
                    // Need set Selector
                    pProducer->setSelector(pSelector);

                    mpResourceContainer->setConsumer(pImage_Raw->getStreamId(),pProducer);

                    vImageParam.push_back(
                        PipelineImageParam{
                            pImage_Raw,
                            pProducer,
                            0
                        }
                    );
                }
                else
                {
                    vImageParam.push_back(
                        PipelineImageParam{
                            pImage_Raw,
                            NULL,
                            0
                        }
                    );
                }
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

    mpPipeline = pBuilder->create();

    if ( mpPipeline == 0) {
        MY_LOGE("Fail to create Legacy Pipeline.");
        return BAD_VALUE;
    }

    if(!bHighSpeedMode)
    {
        sp<StreamBufferProvider> pTempConsumer = mpResourceContainer->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );

        MY_LOGD("provider(%x)", pTempConsumer.get());

        if ( pTempConsumer != 0 ) {
            MY_LOGD("provider(%x)", pTempConsumer.get());
            sp<ResultProcessor> pProcessor = mpPipeline->getResultProcessor().promote();
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
        }
        else
        {
            MY_LOGE("provider is NULL");
        }
    }

    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultFlowControl::
constructZsdPreviewPipeline()
{
    FUNC_START;

    CAM_TRACE_NAME("DFC:constructZsdPreviewPipeline");
    mvwpClientBufferPool.clear();

    mLPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_ZsdPreview;

    HwInfoHelper helper(mOpenId);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }

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
    //
    mLPBConfigParams.p1ConfigConcurrency = mP1NodeConcurrency;
    mLPBConfigParams.p2ConfigConcurrency = mP2NodeConcurrency;
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
    if (helper.get4CellSensorSupported())
    {
        mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    }
    else if(mbSwitchModeEnable &&
            mSwitchModeStatus != eSwitchMode_Undefined)
    {
        if(mSwitchModeStatus == eSwitchMode_HighLightMode)
        {
            mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
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
    // Sensor
    MY_LOGD("sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d",
        mSensorParam.mode,
        mSensorParam.rawType,
        mSensorParam.size.w, mSensorParam.size.h,
        mSensorParam.fps,
        mSensorParam.pixelMode);
    pBuilder->setSrc(mSensorParam);
    //
    MUINT featureCFG = 0;
    if (mLPBConfigParams.enableDualPD)
    {
        FEATURE_CFG_ENABLE_MASK(featureCFG,IScenarioControl::FEATURE_DUAL_PD);
    }
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
    const int ZSD_PREVIEW_MAX_IMGO_BUFFFER_NUM = 12;
    const int ZSD_PREVIEW_MAX_RRZO_BUFFER_NUM  = 6;
    {
        Vector<PipelineImageParam> vImageParam;
        // RAW
        {
            CAM_TRACE_NAME("allocate RAW");
            MUINT32 const bitDepth = getPreviewRawBitDepth(helper);
            MSize previewsize;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
            mpParamsManagerV3->getParamsMgr()->getPreviewSize(&previewsize.w, &previewsize.h);

#if 1
            // Full RAW (IMGO)
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

            mpResourceContainer->setConsumer( pImage_Raw->getStreamId(), pProducer);

            vImageParam.push_back(
                PipelineImageParam{
                    pImage_Raw,
                    pProducer,
                    0
                }
            );
#endif

            // RRZO
            if( helper.hasRrzo() )
            {
                sp<IImageStreamInfo> pImage_Rrzo;

#if (MTKCAM_HAVE_MFB_SUPPORT == 0)
                int __rrzoNum = ZSD_PREVIEW_MAX_RRZO_BUFFER_NUM + 6;
#else
                int __rrzoNum = ZSD_PREVIEW_MAX_RRZO_BUFFER_NUM + 6; // 6 frame for MFNR ZSD cache buffer.
                pSelector->sendCommand(ISelector::eCmd_setAllBuffersKept, 0, 0, 0);
                MY_LOGD("rrzo_max(%d) updated for mfnr feature", __rrzoNum);
#endif

                if ( OK != decideRrzoImage(
                                helper, bitDepth,
                                previewsize, usage,
                                0, __rrzoNum,
                                pImage_Rrzo, MFALSE
                            ))
                {
                    MY_LOGE("No rrzo image");
                    return BAD_VALUE;
                }

                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                pFactory->setImageStreamInfo(pImage_Rrzo);
                // Need to set the same PairMetadata, and the PairMetadata need to wait 2 buffer
                pFactory->setPairRule(pPair, 2);
                sp<StreamBufferProvider> pProducer = pFactory->create();
                // Need set Selector
                pProducer->setSelector(pSelector);

                mpResourceContainer->setConsumer(pImage_Rrzo->getStreamId(), pProducer);

                vImageParam.push_back(
                    PipelineImageParam{
                        pImage_Rrzo,
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
                            4, ZSD_PREVIEW_MAX_IMGO_BUFFFER_NUM+3,
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
    //
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
    }
#endif
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
DefaultFlowControl::
getScenario() const
{
    switch(mLPBConfigParams.mode)
    {
        case LegacyPipelineMode_T::PipelineMode_Preview:
            return IScenarioControl::Scenario_NormalPreivew;
        case LegacyPipelineMode_T::PipelineMode_ZsdPreview:
            return IScenarioControl::Scenario_ZsdPreview;
        case LegacyPipelineMode_T::PipelineMode_VideoRecord:
            return IScenarioControl::Scenario_VideoRecord;
        case LegacyPipelineMode_T::PipelineMode_HighSpeedVideo:
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
MERROR
DefaultFlowControl::
decideSensorModeAndRrzo(
    MSize&  recordsize,
    MUINT&  sensorMode,
    MBOOL   bHighSpeedMode
)
{
    MSize paramSize, wantedSize, sensorSize;
    MSize previewModeSize = MSize(0,0);
    HwInfoHelper helper(mOpenId);
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
    wantedSize = paramSize;

    if(!bHighSpeedMode)
    {
        if( isEisOn )
        {
            EisInfo eisInfo;
            EIS_MODE_ENABLE_EIS_12(eisInfo.mode);
            eisInfo.videoConfig = mb4K2KVideoRecord ?
                                  EISCustom::VIDEO_CFG_4K2K : EISCustom::VIDEO_CFG_FHD;
            eisInfo.factor = EISCustom::getEIS12Factor();

            mLPBConfigParams.packedEisInfo = eisInfo.toPackedData();

            wantedSize.w *= 1.2;//for EIS
            wantedSize.h *= 1.2;//for EIS
        }
        //
        sensorMode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
        if( helper.getSensorSize( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, previewModeSize) &&
            wantedSize.w <= previewModeSize.w &&
            wantedSize.h <= previewModeSize.h )
        {
            sensorMode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        }
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
*
********************************************************************************/
MBOOL
DefaultFlowControl::
isSpecialSensorNeedDisableFrontalbinning(MUINT32 const sensorMode, int width)
{
    #if (PLATFORM_IS_MT6765 == 1)//platform is MT6762/MT6765
    {
        SensorStaticInfo sensorStaticInfo;
        ::memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));

        IHalSensorList* pSensorList = MAKE_HalSensorList();
        if( ! pSensorList ) {
            MY_LOGE("cannot get sensorlist");
            return MFALSE;
        }
        MUINT32 sensorDev = pSensorList->querySensorDevIdx(mOpenId);
        pSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

        if (sensorStaticInfo.sensorDevID == 0x885A)//0x885A is sensor OV8856_SENSOR_ID
        {
            if (sensorMode == SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
            {
                if ((width *2 < sensorStaticInfo.previewWidth) && (sensorStaticInfo.previewWidth < 1920))
                {
                    return MTRUE;
                }
            }
            else if (sensorMode == SENSOR_SCENARIO_ID_NORMAL_VIDEO)
            {
                if ((width *2 < sensorStaticInfo.videoWidth) && (sensorStaticInfo.videoWidth < 1920))
                {
                    return MTRUE;
                }
            }
        }
    }
    #endif

    return MFALSE;
}
