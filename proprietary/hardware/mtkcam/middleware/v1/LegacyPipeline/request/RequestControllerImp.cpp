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

#define LOG_TAG "MtkCam/RequestControllerImp"
//
#include "MyUtils.h"
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>

#include <mtkcam/middleware/v1/camutils/CamInfo.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/request/IRequestController.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/request/RequestSettingBuilder.h>
#include "RequestControllerImp.h"
#include <mtkcam/middleware/v1/LegacyPipeline/processor/StreamingProcessor.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>

using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android;
using namespace NSCam::v3;

#if MTKCAM_HAVE_AEE_FEATURE
#include <aee.h>
#define AEE_ASSERT(String) \
    do { \
    aee_system_exception( \
    LOG_TAG, \
    NULL, \
    DB_OPT_DEFAULT, \
    String); \
} while(0)
#endif

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
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
/******************************************************************************
 *
 ******************************************************************************/
sp< IRequestController >
IRequestController::
createInstance(
    char                       const *name,
    MINT32                     const aCameraId,
    sp<INotifyCallback>        const &rpCamMsgCbInfo,
    sp< IParamsManagerV3 >     const &rpParamsManagerV3
)
{
    return new RequestControllerImp(
                    name,
                    aCameraId,
                    rpCamMsgCbInfo,
                    rpParamsManagerV3
               );
}

/******************************************************************************
 *
 ******************************************************************************/
RequestControllerImp::
RequestControllerImp(
    char                       const *name,
    MINT32                     const aCameraId,
    sp<INotifyCallback>        const &rpCamMsgCbInfo,
    sp< IParamsManagerV3 >     const &rpParamsManagerV3
)
    : mName(name)
    , mCameraId(aCameraId)
    , mTemplateType(1)
    , mpCamMsgCbInfo(rpCamMsgCbInfo)
    , mpParamsManagerV3(rpParamsManagerV3)
    , mIsPausingPipeline(false)
    , mpPipelineConfig(NULL)
{
    mbDummySubmit.store(false);
    mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.camera.log.requestCtrl", 0);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
char const*
RequestControllerImp::
getName() const
{
    return mName;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
startPipeline(
    MINT32               aStartRequestNumber,
    MINT32               aEndRequestNumber,
    wp<ILegacyPipeline>  apPipeline,
    wp<IRequestUpdater>  apRequestUpdater,
    MINT32               aPipelineMode,
    MINT32               aLoopCnt,
    LegacyPipelineBuilder::ConfigParams *pPipelineConfig
)
{
    //FUNC_START;
    MY_LOGD("startPipeline(%d, %d) +", aStartRequestNumber, aEndRequestNumber);
    //
    sp<ILegacyPipeline> pPipeline = apPipeline.promote();
    if ( pPipeline == 0 ) {
        MY_LOGE("invalid LegacyPipeline.");
        return BAD_VALUE;
    }
    //
    mpPipelineConfig = pPipelineConfig;
    //
    if(mpRequestThread != 0)
    {
        mpRequestThread->requestExit();
        mpRequestThread = NULL;
    }
    if ( mpStreamingProcessor != 0 ) mpStreamingProcessor = NULL;
    if ( mpRequestSettingBuilder != 0 ) mpRequestSettingBuilder = NULL;
    //
    if ( mpRequestSettingBuilder == NULL ) {
        MY_LOGD("Create RequestSettingBuilder.");
        mpRequestSettingBuilder = RequestSettingBuilder::createInstance(
                                            mCameraId,
                                            mpParamsManagerV3
                                        );
    }
    if ( mpCamMsgCbInfo != 0 ) {
        MY_LOGD("Create StreamingProcessor.");
        mpStreamingProcessor = StreamingProcessor::createInstance(
                                                getOpenId(),
                                                mpCamMsgCbInfo,
                                                mpRequestSettingBuilder,
                                                mpParamsManagerV3,
                                                aStartRequestNumber,
                                                aEndRequestNumber
                                            );
    } else {
        MY_LOGW("Streaming processor does not create.");
    }
    //
    sp<ResultProcessor> pProcessor = pPipeline->getResultProcessor().promote();
    if ( pProcessor != 0 ) {
        pProcessor->registerListener(
                    aStartRequestNumber,
                    aEndRequestNumber,
                    true,
                    mpStreamingProcessor
                );
        pProcessor->registerListener(
                    mpStreamingProcessor
                );
    } else {
        MY_LOGE("Cannot get result processor.");
    }

    sp< IRequestUpdater > pRequestUpdater = apRequestUpdater.promote();
    if ( pRequestUpdater == NULL) {
        MY_LOGE("RequestUpdater promote fail.");
        return BAD_VALUE;
    }

    IMetadata setting;
    mpRequestSettingBuilder->createDefaultRequest(mTemplateType, setting);
    pRequestUpdater->updateParameters(&setting);
    mpRequestSettingBuilder->setStreamingRequest(setting);

    mpPipeline = apPipeline;
    //
    MY_LOGD("aPipelineMode(%d)",aPipelineMode);
    switch(aPipelineMode)
    {
        case LegacyPipelineMode_T::PipelineMode_HighSpeedVideo:
        {
            mpRequestThread = new RequestThreadHighSpeedVideo(getOpenId(), this, aLoopCnt, mpPipelineConfig);
            mpStreamingProcessor->sendCommand(StreamingProcessor::eSP_extCmd_burst_num, aLoopCnt, 0);
            break;
        }
        case LegacyPipelineMode_T::PipelineMode_Feature_StereoZSD:
        {
            mpRequestThread = new RequestThreadStereo(getOpenId(), this);
            break;
        }
        default:
        {
            mpRequestThread = new RequestThread(getOpenId(), this);
            break;
        }
    }
    //
    if(mbDummySubmit.load())
    {
        Vector< BufferList >  vDstStreams;
        for (size_t i = 0; i < mvSettings.size(); ++i) {
            vDstStreams.push_back(mDstStreams);
        }
        submitRequest( mvSettings, vDstStreams, mvRequestNo );

        /* Notice, we have to update aStartRequestNumber to the last value of
         *  mvRequestNo or UB will happened because we guarantee the request numbers
         *  are unique.
         */
        if (__builtin_expect( mvRequestNo.size() > 0, true )) {
            MY_LOGD("update aStartRequestNumber to the last value(%d) + 1 of mvRequestNo", mvRequestNo.top());
            aStartRequestNumber += mvRequestNo.top() + 1 ;
        }
        else {
            MY_LOGD("no need to update aStartRequestNumber since mvRequestNo size is 0");
        }
    }

    if (mpParamsManagerV3->getCancelAF())
    {
        mpParamsManagerV3->setCancelAF(MFALSE); // reset
        mpRequestSettingBuilder->triggerCancelAutofocus();
        mpStreamingProcessor->sendCommand(StreamingProcessor::eSP_extCmd_resume_focus, 0, 0);
    }
    //
    mpRequestThread->start(
                        apPipeline,
                        apRequestUpdater,
                        aStartRequestNumber,
                        aEndRequestNumber
                    );
    //
    if( mpRequestThread->run(REQUESTCONTROLLER_NAME) != OK ) {
        MY_LOGE("Thread init fail.");
        return BAD_VALUE;
    }
    //
    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
stopPipeline()
{
    FUNC_START;
    if(mIsPausingPipeline)
    {
        MY_LOGD("Pipeline is pausing, need to resume pipeline first");
        resumePipeline();
    }
    //
    if(mpRequestThread != NULL)
    {
        mpRequestThread->requestExit();
        mpRequestThread = NULL;
    }
    mpRequestSettingBuilder = NULL;
    mpStreamingProcessor = NULL;
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
pausePipeline(Vector< BufferSet > &vDstStreams)
{
    FUNC_START;
    MERROR ret = OK;
    if( mpRequestThread != NULL )
    {
        MY_LOGD("pausePipeline when mpRequestThread exist");
        ret = mpRequestThread->pausePipeline(vDstStreams);
        mIsPausingPipeline = true;
    }
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
resumePipeline()
{
    FUNC_START;
    MERROR ret = OK;
    if(mbDummySubmit.load())
    {
        int forceEraseDummyMagic = property_get_int32("debug.forceset.erase.dummy.number", -1);

        Vector< BufferList >  vDstStreams;
        for (size_t i = 0; i < mvSettings.size(); ++i) {
            uint32_t vhdrMode = mpParamsManagerV3->getParamsMgr()->getVHdr();
            int eraseDummyMagic = (forceEraseDummyMagic == -1) ? 0 : (forceEraseDummyMagic == -99) ? i : forceEraseDummyMagic;
            if(vhdrMode == SENSOR_VHDR_MODE_MVHDR && i == eraseDummyMagic )
            {
                    BufferList list = mDstStreams;
                    for (auto itr = list.begin(); itr != list.end();)
                    {
                            if(itr->streamId == eSTREAMID_IMAGE_PIPE_YUV_00 || itr->streamId == eSTREAMID_IMAGE_PIPE_YUV_01 )
                                itr = list.erase(itr);
                            else
                                ++itr;
                    }
                    vDstStreams.push_back(list);
                    MY_LOGD("Resume dummy buffer erase number: %d", i);
            }
            else
                vDstStreams.push_back(mDstStreams);
        }
        submitRequest( mvSettings, vDstStreams, mvRequestNo );
        MY_LOGD("submit dummy request(%d):[%d]-[%d]", mvRequestNo.size(), mvRequestNo[0], mvRequestNo[mvRequestNo.size() - 1]);
    }
    //
    if( mpRequestThread != NULL )
    {
        MY_LOGD("resumePipeline when mpRequestThread exist");
        ret = mpRequestThread->resumePipeline();
        mIsPausingPipeline = false;
    }
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
autoFocus()
{
    int waitCnt = 30;
    while(mbDummySubmit.load()==true && waitCnt>0)
    {
        waitCnt--;
        MY_LOGW("sleep for waiting mbDummySubmit change to true : %d",waitCnt);
        usleep(100*1000); //100ms
    }

    if(mpStreamingProcessor != 0){
        return mpStreamingProcessor->startAutoFocus();
    }

    MY_LOGE("Streaming processor does not set.");
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
cancelAutoFocus()
{
    if ( mpStreamingProcessor != 0 ) {
        return mpStreamingProcessor->cancelAutoFocus();
    }

    MY_LOGE("Streaming processor does not set.");
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
precapture(int& flashRequired, nsecs_t tTimeout)
{
    if ( mpStreamingProcessor != 0 ) {
        return mpStreamingProcessor->preCapture(flashRequired, tTimeout);
    }

    MY_LOGE("Streaming processor does not set.");
    return UNKNOWN_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
setParameters(
    android::wp< IRequestUpdater > pUpdater
)
{
    FUNC_START;

    sp< IRequestUpdater > pRequestUpdater = pUpdater.promote();
    if ( pRequestUpdater == NULL) {
        MY_LOGE("RequestUpdater promote fail.");
        return BAD_VALUE;
    }

    sp< RequestSettingBuilder > pRequestSettingBuilder = mpRequestSettingBuilder;
    if( pRequestSettingBuilder != NULL )
    {
        IMetadata setting;
        pRequestSettingBuilder->createDefaultRequest(mTemplateType, setting);
        pRequestUpdater->updateParameters(&setting);
        pRequestSettingBuilder->setStreamingRequest(setting);
    }

    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
sendCommand(
    int32_t cmd,
    int32_t arg1,
    int32_t arg2
)
{
    MY_LOGD("cmd(0x%08X),arg1(0x%08X),arg2(0x%08X)", cmd, arg1, arg2);
    if ( mpStreamingProcessor == 0 )
    {
        MY_LOGW("Streaming processor does not exist.");
        return INVALID_OPERATION;
    }
    //
    switch (cmd)
    {
        case CAMERA_CMD_START_SMOOTH_ZOOM:
        {
            MY_LOGD("CAMERA_CMD_START_SMOOTH_ZOOM");
            return mpStreamingProcessor->startSmoothZoom(arg1);
        } break;
        case CAMERA_CMD_STOP_SMOOTH_ZOOM:
        {
            MY_LOGD("CAMERA_CMD_STOP_SMOOTH_ZOOM");
            return mpStreamingProcessor->stopSmoothZoom();
        } break;
        case CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG:
        {
            MY_LOGD("CAMERA_CMD_ENABLE_FOCUS_MOVE_MSG");
            return OK;
        } break;
        default:
        {
            //MY_LOGW("unsupported cmd(0x%08X),arg1(0x%08X),arg2(0x%08X)",cmd,arg1,arg2);
            break;
        }
    };

    return mpStreamingProcessor->sendCommand(cmd, arg1, arg2);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
setRequestType( int type )
{
    mTemplateType = type;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
RequestControllerImp::
onLastStrongRef( const void* /*id*/)
{
    FUNC_START;

    mpParamsManagerV3       = NULL;
    mpCamMsgCbInfo          = NULL;

    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
RequestControllerImp::
dump()
{
    MY_LOGI("name:%s cameraId:%d template:%d logLevel:%d"
        , mName, mCameraId, mTemplateType, mLogLevel);
    //
    String8 str = String8::format("pause:%d", mIsPausingPipeline);
    if ( !mvDstStreamsForPausePipeline.empty() ) {
        str += String8::format(" DstStreams(%zd): ", mvDstStreamsForPausePipeline.size());
        for (size_t i = 0; i < mvDstStreamsForPausePipeline.size(); ++i)
            str += String8::format("%#" PRIxPTR ", ", mvDstStreamsForPausePipeline[i]);
    }
    MY_LOGI("%s", str.string());
    //
    //if ( mpRequestThread != nullptr ) mpRequestThread->dump();
    if ( mpStreamingProcessor != nullptr ) mpStreamingProcessor->dump();
    // mpRequestSettingBuilder
    // mpCamMsgCbInfo
    // mpParamsManagerV3
}

/******************************************************************************
 *
 ******************************************************************************/
sp< RequestSettingBuilder >
RequestControllerImp::
getRequestSettingBuilder()
{
    return mpRequestSettingBuilder;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
submitRequest(
    Vector< SettingSet > rvSettings,
    Vector< BufferList > rvDstStreams,
    Vector< MINT32 >&    rvRequestNo
)
{
    // check app request setting
    {
        bool haveError = false;
        for ( size_t i = 0; i < rvSettings.size(); ++i ) {
            if ( rvSettings[i].appSetting.count() == 0 ) continue;
            IMetadata::IEntry entry = rvSettings[i].appSetting.entryFor(MTK_CONTROL_CAPTURE_INTENT);
            if( entry.isEmpty() ) {
                MY_LOGE(" Request does not contain capture intent. Should use template to over write.");
                haveError = true;
                break;
            }
        }
        //
        // dump
        if ( mLogLevel >= 2 ) {
            sp<IMetadataConverter> pMetadataConverter = IMetadataConverter::createInstance(IDefaultMetadataTagSet::singleton()->getTagSet());
            for ( size_t i = 0; i < rvSettings.size(); ++i ) {
                MY_LOGD("Setting(%zu / %zu)", i, rvSettings.size());
                pMetadataConverter->dumpAll(rvSettings[i].appSetting);
            }
        }
        //
        if ( rvSettings.size() != rvDstStreams.size() ) {
            //haveError = true;
            MY_LOGW("Each setting should have a set of ouput streams.");
        }
        //
#if MTKCAM_HAVE_AEE_FEATURE
        if ( haveError ) {
            sp<IMetadataConverter> pMetadataConverter = IMetadataConverter::createInstance(IDefaultMetadataTagSet::singleton()->getTagSet());
            for ( size_t i = 0; i < rvSettings.size(); ++i ) {
                MY_LOGW("Setting(%zu / %zu)", i, rvSettings.size());
                pMetadataConverter->dumpAll(rvSettings[i].appSetting);
            }
            for ( size_t i = 0; i < rvDstStreams.size(); ++i ) {
                MY_LOGW("Output stream(%zu / %zu)", i, rvDstStreams.size());
                for ( size_t j = 0; i < rvDstStreams[i].size(); ++j ) {
                    MY_LOGW("stream id: %#" PRIx64 ", critical:%d", rvDstStreams[i][j].streamId, rvDstStreams[i][j].criticalBuffer);
                }
            }
            AEE_ASSERT(String8::format("Wrong setting or output string.").string());
            return UNKNOWN_ERROR;
        }
#endif
    }
    //
    status_t ret = UNKNOWN_ERROR;
    if( mpRequestThread != NULL && mbDummySubmit.load()!= true)
    {
        MY_LOGD("submitRequest when mpRequestThread exist");
        ret = mpRequestThread->submitRequest(
                                rvSettings,
                                rvDstStreams,
                                rvRequestNo
                            );
    }
    else
    {
        MY_LOGD("submitRequest when mpRequestThread doesn't work or mbDummySubmit is true");
        mbDummySubmit.store(false);

        sp<ILegacyPipeline> pPipeline = mpPipeline.promote();
        if( pPipeline == NULL )
        {
            MY_LOGE("Pipeline doesn't exist, can't submitRequest!");
            ret = UNKNOWN_ERROR;
        }
        else
        {
            if(rvSettings.size() == 0)
            {
                MY_LOGE("rvSettings.size == 0");
                ret = BAD_VALUE;
            }
            else
            {
                int n = rvSettings.size();
                for(int i=0; i<n; i++)
                {
                    MY_LOGD("submitRequest (%d)",i);
                    ret = pPipeline->submitRequest(
                                    i,
                                    rvSettings.editItemAt(i).appSetting,
                                    rvSettings.editItemAt(i).halSetting,
                                    rvDstStreams.editItemAt(i)
                                    );
                    if(ret != OK)
                    {
                        MY_LOGE("submitRequest faile in request(%d)",i);
                        break;
                    }
                    rvRequestNo.push_back(i);
                }
            }
        }
    }
    return ret;
}

MVOID
RequestControllerImp::
setDummpyRequest(
    Vector< SettingSet > vSettings,
    BufferList           pDstStreams,
    Vector< MINT32 >     vRequestNo
)
{
    mbDummySubmit.store(true);
    mvSettings = vSettings;
    mDstStreams = pDstStreams;
    mvRequestNo = vRequestNo;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::
getRequestNo(
    MINT32 &requestNo
)
{
    status_t ret = UNKNOWN_ERROR;
    if(mpRequestThread != NULL) {
        MY_LOGD("submitRequest when mpRequestThread exist");
        ret = mpRequestThread->getRequestNo(requestNo);
    }
    else {
        MY_LOGD("get request frame number failed, mpRequestThread is not exist");
        requestNo = 0;
    }
    return ret;
}
