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

#define LOG_TAG "MtkCam/PipelineMgr"
//
#include "MyUtils.h"
//
#include "IPipelineModel.h"
#include "PipelineModelFactory.h"
#include "PipelineModelMgr.h"
#include <mtkcam/feature/hdrDetection/Defs.h>
//
#include <sys/prctl.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


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

template<class T>
inline
status_t
waitInitialize( T& vFuture )
{
    NSCam::MERROR err = OK;
    for( auto &fut : vFuture ) {
        NSCam::MERROR result = fut.get();
        if( result != OK ) {
            err = result;
        }
    }
    //
    vFuture.clear();
    //
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineModelMgr>
IPipelineModelMgr::
create(
    android::wp<IPipelineModelMgr::IAppCallback>const& pAppCallback
)
{
    return new PipelineModelMgr(pAppCallback);
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineModelMgr::
PipelineModelMgr(
    android::wp<IPipelineModelMgr::IAppCallback>const& pAppCallback
)
    : mpAppCallback(pAppCallback)
    , mCurrentOperationMode(0)
    , mvFutures()
{
    FUNC_START;
    //
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
PipelineModelMgr::
~PipelineModelMgr()
{
    FUNC_START;
    //
    MERROR err = waitInitialize( mvFutures );
    if (OK != err) {
        MY_LOGE("destroy fail.");
    }
    //

    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
char const*
PipelineModelMgr::
getName()
const
{
    sp<IPipelineModel> pPipeline = getPipeline();
    if( pPipeline.get() )
        return pPipeline->getName();
    return LOG_TAG;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
PipelineModelMgr::
getOpenId()
const
{
    sp<IPipelineModel> pPipeline = getPipeline();
    if( pPipeline.get() )
        return pPipeline->getOpenId();
    return -1;
}

/******************************************************************************
*
******************************************************************************/
MERROR
PipelineModelMgr::
beginFlush()
{
    FUNC_START;
    //
    MERROR err = waitInitialize( mvFutures );
    if (OK != err) {
        MY_LOGE("initialize fail.");
        return -ENODEV;
    }
    //
    sp<IPipelineModel> pPipeline = getPipeline();
    if( pPipeline.get() )
        err = pPipeline->beginFlush();
    //
    FUNC_END;
    return err;
}

/******************************************************************************
*
******************************************************************************/
MVOID
PipelineModelMgr::
endFlush()
{
    FUNC_START;
    //
    sp<IPipelineModel> pPipeline = getPipeline();
    if( pPipeline.get() )
        pPipeline->endFlush();
    //
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineModelMgr::
configurePipeline(
    ConfigureParam const& rParams
)
{
    FUNC_START;

    CAM_TRACE_CALL();
    //
    // update config params
    mConfigParams.openId = rParams.openId;
    mConfigParams.pAppCallback = mpAppCallback;
    mConfigParams.operation_mode = rParams.operation_mode;
    //
#define _CLONE_(src, dst) \
            do  { \
                dst.setCapacity(src.size()); \
                dst.clear(); \
                for (size_t i = 0; i < src.size(); i++) { \
                    dst.add(src.keyAt(i), src.valueAt(i)); \
                } \
            } while (0) \

        _CLONE_(rParams.vImageStreams      , mConfigParams.vImageStreams);
        _CLONE_(rParams.vMetaStreams       , mConfigParams.vMetaStreams);
        _CLONE_(rParams.vMinFrameDuration  , mConfigParams.vMinFrameDuration);
        _CLONE_(rParams.vStallFrameDuration, mConfigParams.vStallFrameDuration);
#undef  _CLONE_
    //
    // determine pipelinemodel : map operation mode to pipeline scene
    mConfigParams.pipelineScene = evalPipelineScene(rParams.operation_mode);
    //
    mpCurrentPipeline = mConfigParams.pPipelineModel = NULL;
    MERROR err = ConfigurePipeline()(mConfigParams);
    if ( OK != err ) {
        //dump();
        //dumpConfigure();
        MY_LOGE("configure pipeline %d", err);
        return err;
    }
    //
    {
        Mutex::Autolock _l(mPipelineLock);
        mpCurrentPipeline = mConfigParams.pPipelineModel;
    }
    //
    FUNC_END;
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineModelMgr::
submitRequest(IPipelineModelMgr::AppRequest& request)
{
    CAM_TRACE_CALL();
    //
    MERROR err = OK;
    //
    sp<IPipelineModel> pPipeline = getPipeline();
    if( ! pPipeline.get() ) {
        MY_LOGE("pipeline does not exist");
        return INVALID_OPERATION;
    }
    //
    MINT pipelineScene;
    MBOOL bReconfig = isReconfigPipeline(mConfigParams, request, pipelineScene);
    if( bReconfig )
    {
        MY_LOGD("switch pipeline scene %d -> %d", mConfigParams.pipelineScene, pipelineScene);
        //
        pPipeline->endRequesting();
        // Currently, hold sp of previous pipeline for callback
        // and release ref when in-flight request drained
#if 1
        {
            mvFutures.push_back(
                std::async(std::launch::async,
                    [ this ]() {
                        MY_LOGD("Previous pipeline(%s:%p) to wait drained",
                                mConfigParams.pPipelineModel->getName(), mConfigParams.pPipelineModel.get());
                        MERROR err = OK;
                        sp<IPipelineModel> pPipeline = mConfigParams.pPipelineModel;
                        if ( ! pPipeline.get() )
                            err = -ENODEV;
                        else {
                            pPipeline->waitDrained();
                        }
                        return err;
                    }
                 )
            );
        }
#else
        pPipeline->waitDrained();
#endif
        //
        {
            Mutex::Autolock _l(mPipelineLock);
            mpCurrentPipeline = pPipeline = NULL;
        }
        //
        mConfigParams.pipelineScene = pipelineScene;
        MERROR err = ConfigurePipeline()(mConfigParams);
        if ( OK != err ) {
            //dump();
            //dumpConfigure();
            MY_LOGE("configure pipeline %d", err);
            return err;
        }
        //
        {
            Mutex::Autolock _l(mPipelineLock);
            mpCurrentPipeline = mConfigParams.pPipelineModel;
        }
        //
        pPipeline = getPipeline();
    }
    //
    IPipelineModel::AppRequest appRequestParams;
#define _CLONE_(src, dst) \
            do  { \
                dst.setCapacity(src.size()); \
                dst.clear(); \
                for (size_t i = 0; i < src.size(); i++) { \
                    dst.add(src.keyAt(i), src.valueAt(i)); \
                } \
            } while (0) \

    appRequestParams.requestNo = request.requestNo;
    _CLONE_(request.vIImageBuffers,  appRequestParams.vIImageBuffers);
    _CLONE_(request.vOImageBuffers, appRequestParams.vOImageBuffers);
    _CLONE_(request.vIMetaBuffers,   appRequestParams.vIMetaBuffers);

#undef  _CLONE_
    //
    err = pPipeline->submitRequest(appRequestParams);
    if  ( OK != err ) {
        //dump();
        //dumpConfigure();
    }
    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
PipelineModelMgr::
evalPipelineScene(MINT const /*op_mode*/) const
{
#if 0
    switch(op_mode)
    {
        case 0:
            return ePIPELINEMODEL_DEFAULT;
        default:
            break;
    }
#endif
    return ePIPELINEMODEL_DEFAULT;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineModelMgr::
isReconfigPipeline(
    ConfigurePipeline::Params const& curConfig,
    IPipelineModelMgr::AppRequest const& request,
    MINT& pipelineScene
)
{
    CAM_TRACE_CALL();
    //
    pipelineScene = curConfig.pipelineScene;
    //
    sp<IMetaStreamBuffer> pStreamBuffer = request.vIMetaBuffers[0];
    IMetadata* pMetadata = pStreamBuffer->tryReadLock(LOG_TAG);
    IMetadata::IEntry const eSceneMode = pMetadata->entryFor(MTK_CONTROL_SCENE_MODE);
    IMetadata::IEntry const eCaptureIntent = pMetadata->entryFor(MTK_CONTROL_CAPTURE_INTENT);
    IMetadata::IEntry const eHDRMode = pMetadata->entryFor(MTK_HDR_FEATURE_HDR_MODE);
    pStreamBuffer->unlock(LOG_TAG, pMetadata);

    HDRMode hdrMode = eHDRMode.isEmpty() ? HDRMode::OFF : static_cast<HDRMode>(eHDRMode.itemAt(0, Type2Type<MINT32>()));

    /*android::sp<IMetadataConverter> metadataConverter = IMetadataConverter::createInstance(IDefaultMetadataTagSet::singleton()->getTagSet());
    metadataConverter->dumpAll(*pMetadata, request.requestNo);*/

    if ( ! eSceneMode.isEmpty() &&
           eSceneMode.itemAt(0, Type2Type<MUINT8>()) == MTK_CONTROL_SCENE_MODE_HDR &&
         ! eCaptureIntent.isEmpty() &&
           eCaptureIntent.itemAt(0, Type2Type<MUINT8>()) == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE &&
           hdrMode != HDRMode::VIDEO_ON &&
           hdrMode != HDRMode::VIDEO_AUTO)
    {
        // hdr pipeline
        MY_LOGD("this request is hdr scene mode w/ capture");
#if MTKCAM_HAVE_IVENDOR_SUPPORT
        pipelineScene = ePIPELINEMODEL_DEFAULT;
#else
        pipelineScene = ePIPELINEMODEL_HDR;
#endif


    }
    else
    {
        // default pipeline
        pipelineScene = ePIPELINEMODEL_DEFAULT;
    }
    //
    MY_LOGD_IF( 0, "pipeline scene(%d); current scene(%d)", pipelineScene, curConfig.pipelineScene);
    return (pipelineScene != curConfig.pipelineScene);
}
