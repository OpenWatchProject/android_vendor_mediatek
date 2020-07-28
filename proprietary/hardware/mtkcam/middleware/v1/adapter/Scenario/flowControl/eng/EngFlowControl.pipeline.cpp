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

#define LOG_TAG "MtkCam/EngFlowControl"
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
#include "EngFlowControl.h"
#include <buffer/ClientBufferPool.h>

#include <math.h>
#include <camera_custom_isp_limitation.h>
#include <camera_custom_eis.h>
#include <mtkcam/feature/eis/EisInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>

using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android;
using namespace NSCam::v3;
using namespace NSCamHW;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

#define TEST_RAW_ALLOCATE (0)

/******************************************************************************
 *
 ******************************************************************************/
MERROR
EngFlowControl::
decideRrzoImage(
    HwInfoHelper& helper,
    MUINT32 const bitDepth,
    MSize         referenceSize,
    MUINT const   usage,
    MINT32 const  minBuffer,
    MINT32 const  maxBuffer,
    sp<IImageStreamInfo>& rpImageStreamInfo
)
{
    MSize autualSize;
    size_t stride;
    MINT format;
    if( ! helper.getRrzoFmt(bitDepth, format, MFALSE) ||
        ! helper.alignRrzoHwLimitation(referenceSize, mSensorParam.size, autualSize) ||
        ! helper.alignPass1HwLimitation(mSensorParam.pixelMode, format, false, autualSize, stride) )
    {
        MY_LOGE("wrong params about rrzo");
        return BAD_VALUE;
    }
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
EngFlowControl::
decideImgoImage(
    HwInfoHelper& helper,
    MUINT32 const bitDepth,
    MSize         referenceSize,
    MUINT  const  usage,
    MINT32 const  minBuffer,
    MINT32 const  maxBuffer,
    sp<IImageStreamInfo>& rpImageStreamInfo
)
{
    MSize autualSize = referenceSize;
    size_t stride;
    MINT format;
    if( ! helper.getImgoFmt(bitDepth, format, MFALSE) ||
        ! helper.alignPass1HwLimitation(mSensorParam.pixelMode, format, true, autualSize, stride) )
    {
        MY_LOGE("wrong params about imgo");
        return BAD_VALUE;
    }
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
EngFlowControl::
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
EngFlowControl::
constructNormalPreviewPipeline()
{
    FUNC_START;
    //LegacyPipelineBuilder::ConfigParams mLPBConfigParams;
    //
    MUINT32 u4VHDmode = SENSOR_VHDR_MODE_NONE;
    u4VHDmode = mpParamsManagerV3->getParamsMgr()->getVHdr();

    MBOOL u4EISEnabled = mpParamsManagerV3->getParamsMgr()->getVideoStabilization();
    //
    sp<LegacyPipelineBuilder> pBuilder = NULL;

    mLPBConfigParams.enableEIS = u4EISEnabled;
    MINT32 lcsOpen = mEnLtm;
    mLPBConfigParams.enableLCS = (lcsOpen > 0) || (u4VHDmode == SENSOR_VHDR_MODE_ZVHDR)
                            || (u4VHDmode == SENSOR_VHDR_MODE_IVHDR); // LCS always true

    //
        { // start of Engineer Mode
            String8 ms8SaveMode = mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_RAW_SAVE_MODE);

            const char *strSaveMode = ms8SaveMode.string();
            mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;

            switch (strSaveMode[0])
            {
                case '1': // 1: "Preview Mode",
                    // mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                    MY_LOGD("SENSOR_SCENARIO_ID_NORMAL_PREVIEW");
                    break;
                case '2': // 2: "Capture Mode",
                    // mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                    mSensorParam.mode = (((u4VHDmode!= SENSOR_VHDR_MODE_NONE) && (u4VHDmode!=SENSOR_VHDR_MODE_MVHDR))?
                    SENSOR_SCENARIO_ID_NORMAL_CAPTURE:mSensorParam.mode);
                    MY_LOGD("SENSOR_SCENARIO_ID_NORMAL_PREVIEW");
                    break;
                case '4': // 4: "Video Preview Mode"
                    mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
                    MY_LOGD("SENSOR_SCENARIO_ID_NORMAL_VIDEO");
                    break;
                case '5': // 5: "Slim Video 1"
                    mSensorParam.mode = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
                    MY_LOGD("SENSOR_SCENARIO_ID_SLIM_VIDEO1");
                    break;
                case '6': // 6: "Slim Video 2"
                    mSensorParam.mode = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
                    MY_LOGD("SENSOR_SCENARIO_ID_SLIM_VIDEO2");
                    break;
                case '0': // 0: do not save
                case '3': // 3: "JPEG Only"
                default:
                    mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                    MY_LOGD("SENSOR_SCENARIO_ID_NORMAL_PREVIEW");
                    break;
            }
    }// end of Engineer Mode setting
    //
    HwInfoHelper helper(mOpenId);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if (helper.getDualPDAFSupported(mSensorParam.mode))
    {
        mLPBConfigParams.enableDualPD = MTRUE;
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
    MUINT32 eisMode = EIS_MODE_OFF;
    if( mLPBConfigParams.enableEIS && u4VHDmode == SENSOR_VHDR_MODE_NONE )
    {
        EIS_MODE_ENABLE_CALIBRATION(eisMode);
    }

    MSize videoSize;
    mpParamsManagerV3->getParamsMgr()->getVideoSize(&videoSize.w, &videoSize.h);
    NSCam::EIS::EisInfo eisInfo;
    eisInfo.videoConfig = ( videoSize.w < 3840 && videoSize.h < 2160 ) ?
                          EISCustom::VIDEO_CFG_FHD : EISCustom::VIDEO_CFG_4K2K;
    eisInfo.queueSize = 0;
    eisInfo.startFrame = 1;
    eisInfo.factor = 100;
    eisInfo.mode = eisMode;

    mLPBConfigParams.eisMode = eisMode;
    mLPBConfigParams.packedEisInfo = eisInfo.toPackedData();

    MY_LOGD("enableEIS: %d, eisMode: %d, packedEisInfo: %" PRIi64 "",
            mLPBConfigParams.enableEIS, mLPBConfigParams.eisMode, mLPBConfigParams.packedEisInfo);
    //
    if(u4VHDmode!= SENSOR_VHDR_MODE_NONE)
    {
        mLPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_Feature_Preview;
        pBuilder = LegacyPipelineBuilder::createInstance(
                                                        mOpenId,
                                                        "FeaturePreview",
                                                        mLPBConfigParams);
    }
    else
    {
        mLPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_Preview;
        pBuilder = LegacyPipelineBuilder::createInstance(
                                                        mOpenId,
                                                        "NormalPreview",
                                                        mLPBConfigParams);
    }
    //
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return BAD_VALUE;
    }
    //
    //mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    if( ! helper.getSensorSize( mSensorParam.mode, mSensorParam.size) ||
        ! helper.getSensorFps( (MUINT32)mSensorParam.mode, (MINT32&)mSensorParam.fps) ||
        ! helper.queryPixelMode( mSensorParam.mode, mSensorParam.fps, mSensorParam.pixelMode)
      ) {
        MY_LOGE("cannot get params about sensor");
        return BAD_VALUE;
    }
    //
    if(u4VHDmode!= SENSOR_VHDR_MODE_NONE)
    {
        // Sensor
        mSensorParam.vhdrMode = mpParamsManagerV3->getParamsMgr()->getVHdr();
        MY_LOGD("sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d vhdrMode:%d",
            mSensorParam.mode,
            mSensorParam.rawType,
            mSensorParam.size.w, mSensorParam.size.h,
            mSensorParam.fps,
            mSensorParam.pixelMode,
            mSensorParam.vhdrMode);
    }
    else
    {
        // Sensor
        MY_LOGD("sensor mode:%d, rawType:%d, size:%dx%d, fps:%d pixel:%d",
            mSensorParam.mode,
            mSensorParam.rawType,
            mSensorParam.size.w, mSensorParam.size.h,
            mSensorParam.fps,
            mSensorParam.pixelMode);
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
    sp<IScenarioControl> pScenarioCtrl = enterScenarioControl(IScenarioControl::Scenario_NormalPreivew, mSensorParam.size, mSensorParam.fps, featureCFG);
    if( pScenarioCtrl.get() == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return BAD_VALUE;
    }
    pBuilder->setScenarioControl(pScenarioCtrl);

    //
    int camera_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
    int rawoutport = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_RAW_OUTPUT_PORT);
    MY_LOGD("camera_mode( %d ), rawoutport( %d )", camera_mode, rawoutport);
    // Image
    sp<PairMetadata>  pPair;
    sp<BufferPoolImp> pFullRawPool;
    sp<IImageStreamInfo> pImage_Raw;
    {
        Vector<PipelineImageParam> vImageParam;
        // RAW
        {

            MUINT32 const bitDepth = getPreviewRawBitDepth(helper);
            MSize previewsize;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
            mpParamsManagerV3->getParamsMgr()->getPreviewSize(&previewsize.w, &previewsize.h);

            if( !helper.hasRrzo() )
            {
                if ( OK != decideImgoImage(
                                helper, bitDepth,
                                mSensorParam.size, usage,
                                2, 8,
                                pImage_Raw
                            ))
                {
                    MY_LOGE("No imgo image");
                    return BAD_VALUE;
                }
                //dump IMGO
                if(camera_mode != 0 && rawoutport == 0)
                {
                    MY_LOGD("camera_mode != 0 && rawoutport == 0");
                    pPair = PairMetadata::createInstance(pImage_Raw->getStreamName());
                    pFullRawPool = new BufferPoolImp(pImage_Raw);
                    //
                    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                    pFactory->setImageStreamInfo(pImage_Raw);
                    pFactory->setPairRule(pPair, 1);
                    pFactory->setUsersPool(pFullRawPool);
                    sp<StreamBufferProvider> pProducer = pFactory->create();

                    mpResourceContainer->setConsumer( pImage_Raw->getStreamId(), pProducer);

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
            else
            {
                if ( OK != decideRrzoImage(
                                helper, bitDepth,
                                previewsize, usage,
                                2, 8,
                                pImage_Raw
                            ))
                {
                    MY_LOGE("No rrzo image");
                    return BAD_VALUE;
                }

                //dump RRZO
                if(camera_mode != 0 && rawoutport == 1)
                {
                    MY_LOGD("camera_mode != 0 && rawoutport == 1");
                    pPair = PairMetadata::createInstance(pImage_Raw->getStreamName());
                    pFullRawPool = new BufferPoolImp(pImage_Raw);
                    //
                    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                    pFactory->setImageStreamInfo(pImage_Raw);
                    pFactory->setPairRule(pPair, 1);
                    pFactory->setUsersPool(pFullRawPool);
                    sp<StreamBufferProvider> pProducer = pFactory->create();

                    mpResourceContainer->setConsumer( pImage_Raw->getStreamId(), pProducer);

                    vImageParam.push_back(
                        PipelineImageParam{
                            pImage_Raw,
                            pProducer,
                            0
                        }
                    );
                }
                else //RRZO preview
                {
                    vImageParam.push_back(
                        PipelineImageParam{
                            pImage_Raw,
                            NULL,
                            0
                        }
                    );
                }
                //dump IMGO
                {
                    //int camera_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
                    if (camera_mode != 0 && rawoutport == 0)
                    {
                        MY_LOGD("camera_mode != 0 && rawoutport == 0");
                        if ( OK != decideImgoImage(
                                        helper, bitDepth,
                                        mSensorParam.size, usage,
                                        0, 9,
                                        pImage_Raw
                                    ))
                        {
                            MY_LOGE("No imgo image");
                            return BAD_VALUE;
                        }

                        pPair = PairMetadata::createInstance(pImage_Raw->getStreamName());
                        pFullRawPool = new BufferPoolImp(pImage_Raw);
                        //
                        sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                        pFactory->setImageStreamInfo(pImage_Raw);
                        pFactory->setPairRule(pPair, 1);
                        pFactory->setUsersPool(pFullRawPool);
                        sp<StreamBufferProvider> pProducer = pFactory->create();

                        mpResourceContainer->setConsumer( pImage_Raw->getStreamId(), pProducer);

                        vImageParam.push_back(
                            PipelineImageParam{
                                pImage_Raw,
                                pProducer,
                                0
                            }
                        );
                    }
                }
            }
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

    mpPipeline = pBuilder->create();

    if ( mpPipeline == 0) {
        MY_LOGE("Fail to create Legacy Pipeline.");
        return BAD_VALUE;
    }
    //
    {
        //int camera_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE);

        if (camera_mode != 0)
        {
            MY_LOGD("eng video preview flow");
            //
            pFullRawPool->allocateBuffer(
            pImage_Raw->getStreamName(),
            pImage_Raw->getMaxBufNum(),
            pImage_Raw->getMinInitBufNum()
                    );
            //
            sp<StreamBufferProvider> pTempConsumer =
                mpResourceContainer->queryConsumer( (rawoutport == 0) ? eSTREAMID_IMAGE_PIPE_RAW_OPAQUE : eSTREAMID_IMAGE_PIPE_RAW_RESIZER );

            if ( pTempConsumer != 0 ) {
                CAM_TRACE_NAME("DFC:SetSelector");
                sp<ResultProcessor> pProcessor = mpPipeline->getResultProcessor().promote();
                pProcessor->registerListener(
                                eSTREAMID_META_HAL_DYNAMIC_P2,
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
                //
                mpSelector = new NSCam::v1::NSLegacyPipeline::EngSelector();
                mpSelector->setConfig(mpParamsManagerV3, mOpenId, mpCamMsgCbInfo);
                pTempConsumer->setSelector(mpSelector);
            }
        }
        else
        {
            MY_LOGD("eng normal preview flow");
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
        }
    }

    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
EngFlowControl::
constructRecordingPipeline()
{
    FUNC_START;
    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    LPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_VideoRecord;
    LPBConfigParams.enableEIS = MFALSE;
    LPBConfigParams.enableLCS = MFALSE;

    HwInfoHelper helper(mOpenId);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    mSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
    if (helper.getDualPDAFSupported(mSensorParam.mode))
    {
        LPBConfigParams.enableDualPD = MTRUE;
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
    sp<LegacyPipelineBuilder> pBuilder =
        LegacyPipelineBuilder::createInstance(
                                    mOpenId,
                                    "NormalRecord",
                                    LPBConfigParams);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return BAD_VALUE;
    }
    //
    if( ! helper.getSensorSize( mSensorParam.mode, mSensorParam.size) ||
        ! helper.getSensorFps( (MUINT32)mSensorParam.mode, (MINT32&)mSensorParam.fps) ||
        ! helper.queryPixelMode( mSensorParam.mode, mSensorParam.fps, mSensorParam.pixelMode)
      ) {
        MY_LOGE("cannot get params about sensor");
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
    //
    sp<IScenarioControl> pScenarioCtrl = enterScenarioControl(IScenarioControl::Scenario_VideoRecord, mSensorParam.size, mSensorParam.fps, featureCFG);
    if( pScenarioCtrl.get() == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return BAD_VALUE;
    }
    pBuilder->setScenarioControl(pScenarioCtrl);

    // Image
    {
        Vector<PipelineImageParam> vImageParam;
        // RAW
        {
            MUINT32 const bitDepth = getPreviewRawBitDepth(helper);
            MSize recordsize;
            MSize size;
            MINT format;
            size_t stride;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here

            recordsize = calculateRrzoImage();

            sp<IImageStreamInfo> pImage_Raw;

            if( !helper.hasRrzo() )
            {
                if ( OK != decideImgoImage(
                                helper, bitDepth,
                                mSensorParam.size, usage,
                                4, 8,
                                pImage_Raw
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
                                recordsize, usage,
                                4, 8,
                                pImage_Raw
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

            setCamClient(
                "Hal:Image:yuvRecord",
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

    mpPipeline = pBuilder->create();

    if ( mpPipeline == 0) {
        MY_LOGE("Fail to create Legacy Pipeline.");
        return BAD_VALUE;
    }

    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
EngFlowControl::
getScenario() const
{
    if( mSensorParam.mode == SENSOR_SCENARIO_ID_NORMAL_VIDEO )
    {
        return IScenarioControl::Scenario_VideoRecord;
    }
    else
    {
        return IScenarioControl::Scenario_NormalPreivew;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MSize
EngFlowControl::
calculateRrzoImage()
{
    MSize paramSize, recordsize;
    mpParamsManagerV3->getParamsMgr()->getVideoSize(&paramSize.w, &paramSize.h);

    //decide default size
    if( 32*mSensorParam.size.h/mSensorParam.size.w >= 21 )
    {
        recordsize = MSize(1440,1080);
    }
    else
    {
        recordsize = MSize(1920,1080);
    }
    //
    if( paramSize.w > recordsize.w )
    {
        recordsize.w = paramSize.w;
    }
    if( paramSize.h > recordsize.h )
    {
        recordsize.h = paramSize.h;
    }
    recordsize.w *= 1.2;//for EIS
    recordsize.h *= 1.2;//for EIS
    if( paramSize.w < 3840 && paramSize.h < 2160 )
    {
        return recordsize;//not 4K2K VR
    }

    //check hw and bandwidth limitation
    MINT32 bufferSizeLimitation, bufferWidthLimitation;
    getVrRrzoLimitation(bufferSizeLimitation, bufferWidthLimitation, MTRUE);
    MY_LOGD("size lim(%d), width lim(%d)", bufferSizeLimitation, bufferWidthLimitation);
    if( mSensorParam.size.h*mSensorParam.size.w <= bufferSizeLimitation &&
        mSensorParam.size.w <= bufferWidthLimitation )
    {
        return recordsize;
    }
    else
    {
        //check hw limitation
        if( mSensorParam.size.w > bufferWidthLimitation )
        {
            recordsize = MSize(bufferWidthLimitation, bufferWidthLimitation*mSensorParam.size.h/mSensorParam.size.w );
        }
        else
        {
            recordsize = MSize(mSensorParam.size.w, mSensorParam.size.h );
        }
    }
    if( recordsize.w*recordsize.h <= bufferSizeLimitation )
    {
        return recordsize;
    }
    else
    {
#define sizeCoef (0.99f)
        MINT32 height = floor(sqrt((float)bufferSizeLimitation*sizeCoef/mSensorParam.size.w*mSensorParam.size.h));
        recordsize = MSize(height*mSensorParam.size.w/mSensorParam.size.h, height);
        MY_LOGD("size lim(%d), coef(%f), sensor(%dx%d), record(%dx%d)",
                bufferSizeLimitation, sizeCoef, mSensorParam.size.w, mSensorParam.size.h,
                recordsize.w, recordsize.h);
#undef sizeCoef
    }

    return recordsize;
}
