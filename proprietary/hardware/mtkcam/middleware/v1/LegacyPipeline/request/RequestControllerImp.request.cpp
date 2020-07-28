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

#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>

//
using namespace android;
#include <mtkcam/middleware/v1/LegacyPipeline/request/RequestSettingBuilder.h>
#include "RequestControllerImp.h"
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/utils/hw/HwInfoHelper.h>
using namespace NSCamHW;

using namespace NSCam;
using namespace NSCam::v1::NSLegacyPipeline;


#if (MTKCAM_HAVE_MTKSTEREO_SUPPORT == 1)
#define DUAL_CAM (1)
#else
#define DUAL_CAM (0)
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
#define FUNC_START                  MY_LOGD1("+")
#define FUNC_END                    MY_LOGD1("-")
//
typedef NSCam::v3::Utils::HalMetaStreamBuffer       HalMetaStreamBuffer;
typedef NSCam::v3::Utils::HalImageStreamBuffer      HalImageStreamBuffer;
typedef HalMetaStreamBuffer::Allocator              HalMetaStreamBufferAllocatorT;

/******************************************************************************
 *
 ******************************************************************************/
RequestControllerImp::RequestThread::
RequestThread(
    MINT32  openId,
    wp< RequestControllerImp > parent
)
    : mpRequestControllerImp(parent)
    , mpPipeline(NULL)
    , mOpenId(openId)
    , mExtRequest(false)
    , mIsPausingPipeline(false)
    , mbForGetRequestDonePauseCase(false)

{
    mLogLevel = property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = property_get_int32("debug.camera.log.requestCtrl", 0);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
bool
RequestControllerImp::RequestThread::
threadLoop()
{
    if ( !exitPending() ) {

        Mutex::Autolock _l(mLock);

        IMetadata settingAppMeta;
        IMetadata settingHalMeta;
        //
        sp< ILegacyPipeline > pPipeline = mpPipeline.promote();
        if ( pPipeline == NULL) {
            MY_LOGE("LegacyPipeline promote fail.");
            return false;
        }

        MINT32 requestNumber;
        Vector< BufferSet > vDstStreams;
        if ( waitForNextRequest(
                    settingAppMeta,
                    settingHalMeta,
                    requestNumber,
                    vDstStreams
            ) != OK ) {
            MY_LOGE("waitForNextRequest fail.");
            return false;
        }

        {
            bool bIsPausingPipeline;
            Vector< BufferSet > vDstStreamsForPausePipeline;
            {
                Mutex::Autolock _l(mDesStreamSeForPausetLock);
                bIsPausingPipeline = mIsPausingPipeline;
                vDstStreamsForPausePipeline = mvDstStreamsForPausePipeline;
            }
            MERROR err = OK;
            if( bIsPausingPipeline )
            {
                if( vDstStreams.empty() )
                {
                    if ( vDstStreamsForPausePipeline.empty() ) {
                             Mutex::Autolock _l(mDesStreamSeForPausetLock);
                             if(mvDstStreamsForPausePipeline.empty())
                             {
                                 MY_LOGD("No Dst Stream, ready to wait resume pipeline");
                                 MY_LOGD("mbForGetRequestDonePauseCase = %d, mIsPausingPipeline = %d", mbForGetRequestDonePauseCase, mIsPausingPipeline);
                                 if(!mbForGetRequestDonePauseCase && mIsPausingPipeline)
                                     mCondDesStreamSetForPause.wait(mDesStreamSeForPausetLock);
                             }
                             else
                             {
                                 MY_LOGD("In Pause Pipeline, mvDstStreamsForPausePipeline just change to be not empty");
                                 err = pPipeline->submitRequest(
                                                 requestNumber,
                                                 settingAppMeta,
                                                 settingHalMeta,
                                                 mvDstStreamsForPausePipeline
                                            );
                             }
                     } else {
                         MY_LOGD("In Pause Pipeline, use default dst stream");
                         err = pPipeline->submitRequest(
                                         requestNumber,
                                         settingAppMeta,
                                         settingHalMeta,
                                         vDstStreamsForPausePipeline
                                    );
                     }
                }
                else
                {
                    MY_LOGD("In Pause Pipeline, use external submit dst stream providered");
                    err = pPipeline->submitRequest(
                                    requestNumber,
                                    settingAppMeta,
                                    settingHalMeta,
                                    vDstStreams
                               );
                }
            }
            else
            {
#if DUAL_CAM
                IMetadata::IEntry const& entry = settingHalMeta.entryFor(MTK_DUALZOOM_DROP_REQ);
                int DropReq = MTK_DUALZOOM_DROP_NONE;
                if(!entry.isEmpty()) {
                    DropReq = entry.itemAt(0, Type2Type<MINT32>());
                    MY_LOGD("WillDBG3 req thread DropReq : %d", DropReq);
                }
                if(DropReq == MTK_DUALZOOM_DROP_DIRECTLY)
                {
                    usleep(30*1000);
                }
                else if (DropReq == MTK_DUALZOOM_DROP_NEED_P1)
                {
                    // Need to run P1 before drop frame
                    Vector< BufferSet > vStreams;
                    vStreams.push_back(
                        BufferSet {
                            .streamId       = eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
                            .criticalBuffer = false
                        }
                    );
                    err = pPipeline->submitRequest(
                                        requestNumber,
                                        settingAppMeta,
                                        settingHalMeta,
                                        vStreams
                              );
                }
                else
#endif // DUAL_CAM
                {
                    if ( vDstStreams.empty() ) {
                       err = pPipeline->submitSetting(
                                        requestNumber,
                                        settingAppMeta,
                                        settingHalMeta
                                   );
                    } else {
                        err = pPipeline->submitRequest(
                                        requestNumber,
                                        settingAppMeta,
                                        settingHalMeta,
                                        vDstStreams
                                   );
                    }
                }
            }
            //
            MY_LOGE_IF(err != OK, "request %d submit fail", requestNumber);
        }

        {
            Mutex::Autolock _l(mDesStreamSeForPausetLock);
            mbForGetRequestDonePauseCase = false;
        }

        return true;
    }

    MY_LOGD("RequestThread exit.");
    return false;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::RequestThread::
start(
    wp< ILegacyPipeline >& rpPipeline,
    wp< IRequestUpdater >  rpRequestUpdater,
    MINT32               aStartRequestNumber,
    MINT32               aEndRequestNumber
)
{
    FUNC_START;

    Mutex::Autolock _l(mLock);

    mRequestNumberMin = aStartRequestNumber;
    mRequestNumberMax = aEndRequestNumber;
    mRequestNumber    = aStartRequestNumber;
    mpPipeline        = rpPipeline;
    mpRequestUpdater  = rpRequestUpdater;

    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::RequestThread::
waitForNextRequest(
    IMetadata&               aSettingAppMeta,
    IMetadata&               aSettingHalMeta,
    MINT32&                  aRequestNumber,
    Vector<BufferSet>&       rvDstStreams
)
{
    CAM_TRACE_NAME("RC:waitForNextRequest");
    //FUNC_START;
    //
    Mutex::Autolock _l(mRequestLock);
    if ( mRequestNumber > mRequestNumberMax ) mRequestNumber = mRequestNumberMin;

    sp< RequestControllerImp > pRequestControllerImp = mpRequestControllerImp.promote();
    if ( pRequestControllerImp == NULL) {
        MY_LOGE("RequestController promote fail.");
        return BAD_VALUE;
    }
    //
    aRequestNumber = mRequestNumber;
    if ( !mRequestQueue.empty() ) {
        MY_LOGD("External request:%d.", aRequestNumber);
        rvDstStreams = mRequestQueue[0].dstStreams;

        if ( mRequestQueue[0].setting.appSetting.count() == 0 ) {
            // user does not provide setting, use default setting
            MY_LOGD("External request:%d. user does not provide setting, use default setting", aRequestNumber);
            mRequestQueue.removeAt(0);
        } else {
            aSettingAppMeta = mRequestQueue[0].setting.appSetting;
            aSettingHalMeta = mRequestQueue[0].setting.halSetting;
            //
            IMetadata::IEntry entry(MTK_HAL_REQUEST_REPEAT);
            entry.push_back(0, Type2Type< MUINT8 >());
            aSettingHalMeta.update(entry.tag(), entry);
            //
            mRequestQueue.removeAt(0);
            mRequestNumber++;
            mExtRequest = true;
            return OK;
        }
    }

    sp< RequestSettingBuilder > pBuilder = pRequestControllerImp->getRequestSettingBuilder();
    if ( pBuilder == NULL ) {
        MY_LOGE("RequestSettingBuilder is NULL.");
        return BAD_VALUE;
    }
    pBuilder->getRequest(mRequestNumber++, aSettingAppMeta, aSettingHalMeta);

    sp< IRequestUpdater > pRequestUpdater = mpRequestUpdater.promote();
    if ( pRequestUpdater == NULL) {
        MY_LOGE("RequestUpdater promote fail.");
        return BAD_VALUE;
    }

    // request after external request should be set as non-repeat
    if ( mExtRequest ){
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REPEAT);
        entry.push_back(false, Type2Type< MUINT8 >());
        aSettingHalMeta.update(entry.tag(), entry);
        //
        mExtRequest = false;
    }

    MERROR err = pRequestUpdater->updateRequestSetting(&aSettingAppMeta, &aSettingHalMeta);
    if(err != OK)
    {
        MY_LOGD("updateRequestSetting Fail");
        return err;
    }
    err = pRequestUpdater->runRequestFlow(&aSettingAppMeta, &aSettingHalMeta);
    if(err != OK)
    {
        MY_LOGD("runRequestFlow Fail");
        return err;
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::RequestThread::
submitRequest(
    Vector< SettingSet >      vSettings,
    Vector< BufferList >      vDstStreams,
    Vector< MINT32 >&         rvRequestNo
)
{
    FUNC_START;

    Mutex::Autolock _l(mRequestLock);
    MINT32 requestNo;

    requestNo = mRequestNumber + mRequestQueue.size();

    for ( size_t i = 0; i < vSettings.size(); ++ i )
    {
        mRequestQueue.push_back( RequestItem{vSettings[i], vDstStreams[i]} );

        if (requestNo > mRequestNumberMax) requestNo = mRequestNumberMin;
        rvRequestNo.push_back(requestNo++);
    }

    // if pipeline pause, wake thread up
    {
        Mutex::Autolock _l(mDesStreamSeForPausetLock);
        mbForGetRequestDonePauseCase = true;
        mCondDesStreamSetForPause.signal();
    }
    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::RequestThread::
getRequestNo(
    MINT32 &requestNo
)
{
    FUNC_START;

    Mutex::Autolock _l(mRequestLock);

    requestNo = mRequestNumber + mRequestQueue.size();
    if (requestNo > mRequestNumberMax) {
        MY_LOGD("current requestNo:%d, mRequestNumberMax:%d, mRequestNumberMin:%d",
                requestNo, mRequestNumberMax, mRequestNumberMin);
        requestNo = mRequestNumberMin;
    }

    MY_LOGD("current requestNo: %d", requestNo);

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
RequestControllerImp::RequestThread::
onLastStrongRef(const void* /*id*/)
{
    FUNC_START;

    //requestExit();

    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
void
RequestControllerImp::RequestThread::
requestExit()
{
    FUNC_START;

    mCondDesStreamSetForPause.signal();
    //let deque thread back
    Thread::requestExit();
    join();

    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
RequestControllerImp::RequestThread::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)REQUESTCONTROLLER_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, REQUESTCONTROLLER_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, REQUESTCONTROLLER_PRIORITY);
    //
    ::sched_getparam(0, &sched_p);

    MY_LOGD(
        "tid(%d) policy(%d) priority(%d)"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority
    );

    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::RequestThread::
pausePipeline(Vector< BufferSet > &vDstStreams)
{
    FUNC_START;
    Mutex::Autolock _l(mDesStreamSeForPausetLock);
    mvDstStreamsForPausePipeline = vDstStreams;
    mIsPausingPipeline = true;
    FUNC_END;
    return OK;

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestControllerImp::RequestThread::
resumePipeline()
{
    FUNC_START;
    Mutex::Autolock _l(mDesStreamSeForPausetLock);
    mIsPausingPipeline = false;
    mCondDesStreamSetForPause.signal();
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
RequestControllerImp::RequestThreadHighSpeedVideo::
RequestThreadHighSpeedVideo(
    MINT32  openId,
    wp< RequestControllerImp > parent,
    MUINT loopCnt,
    LegacyPipelineBuilder::ConfigParams *pPipelineConfig
)
    : RequestThread(openId, parent)
    , mRequestLoopCnt(loopCnt)
    , mpPipelineConfig(pPipelineConfig)
{
}

/******************************************************************************
 *
 ******************************************************************************/
bool
RequestControllerImp::RequestThreadHighSpeedVideo::
threadLoop()
{
    if ( !exitPending() ) {

        Mutex::Autolock _l(mLock);
        //
        sp< ILegacyPipeline > pPipeline = mpPipeline.promote();
        if ( pPipeline == NULL) {
            MY_LOGE("LegacyPipeline promote fail.");
            return false;
        }

        MINT32 requestNumber;
        Vector< BufferSet > vDstStreams;
        MERROR err = OK;

        for(MINT32 i = 0; i < mRequestLoopCnt; i++)
        {
            IMetadata settingAppMeta;
            IMetadata settingHalMeta;

            if ( waitForNextRequest(
                        settingAppMeta,
                        settingHalMeta,
                        requestNumber,
                        vDstStreams) != OK )
            {
                MY_LOGE("waitForNextRequest fail.");
                return false;
            }

            bool useLCSO = false;
            if(mpPipelineConfig)
            {
                useLCSO = mpPipelineConfig->enableLCS;
            }

            MY_LOGD("frame %d useLCSO(%d)", i,useLCSO);
            vDstStreams.clear();
            HwInfoHelper helper(mOpenId);
            if( ! helper.updateInfos() ) {
                MY_LOGE("cannot properly update infos");
            }
            vDstStreams.push_back(
                BufferSet{
                    .streamId       = helper.hasRrzo() ? eSTREAMID_IMAGE_PIPE_RAW_RESIZER : eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                    .criticalBuffer = false
                }
            );

            if(i == 0)
            {
                vDstStreams.push_back(
                    BufferSet{
                        .streamId       = eSTREAMID_IMAGE_PIPE_YUV_00,
                        .criticalBuffer = false
                    }
                );
            }

            vDstStreams.push_back(
                BufferSet{
                    .streamId       = eSTREAMID_IMAGE_PIPE_YUV_01,
                    .criticalBuffer = false
                }
            );

            if(useLCSO)
            {
                vDstStreams.push_back(
                    BufferSet{
                        .streamId       = eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                        .criticalBuffer = false
                    }
                );
            }


            err = pPipeline->submitRequest(
                            requestNumber,
                            settingAppMeta,
                            settingHalMeta,
                            vDstStreams);
            MY_LOGE_IF(err != OK, "request %d submit fail", requestNumber);
        }

        return true;
    }

    MY_LOGD("RequestThread exit.");
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestControllerImp::RequestThreadStereo::
RequestThreadStereo(
    MINT32  openId,
    wp< RequestControllerImp > parent
)
    : RequestThread(openId, parent)
{
    MY_LOGD("Create stereo request thread");
}

/******************************************************************************
 *
 ******************************************************************************/
bool
RequestControllerImp::RequestThreadStereo::
threadLoop()
{
    if ( !exitPending() ) {
        Mutex::Autolock _l(mLock);

        IMetadata settingAppMeta;
        IMetadata settingHalMeta;
        //
        sp< ILegacyPipeline > pPipeline = mpPipeline.promote();
        if ( pPipeline == NULL) {
            MY_LOGE("LegacyPipeline promote fail.");
            return false;
        }

        MINT32 requestNumber = 0;
        {
            MERROR err = OK;
            MBOOL lIsPaugingPipline = MFALSE;
            {
                Mutex::Autolock _l(mDesStreamSeForPausetLock);
                lIsPaugingPipline = mIsPausingPipeline;
                if(lIsPaugingPipline)
                {
                    if(mvDstStreamsForPausePipeline.empty())
                    {
                        mCondDesStreamSetForPause.wait(mDesStreamSeForPausetLock);
                    }
                    else
                    {
                         err = pPipeline->submitRequest(
                                         requestNumber,
                                         settingAppMeta,
                                         settingHalMeta,
                                         mvDstStreamsForPausePipeline
                                    );
                    }
                }
            }
            if(!lIsPaugingPipline)
            {
                Vector< BufferSet > vDstStreams;
                if ( waitForNextRequest(
                            settingAppMeta,
                            settingHalMeta,
                            requestNumber,
                            vDstStreams
                    ) != OK ) {
                    MY_LOGE("waitForNextRequest fail.");
                    return false;
                }
                if ( vDstStreams.empty() ) {
                    err = pPipeline->submitSetting(
                                    requestNumber,
                                    settingAppMeta,
                                    settingHalMeta
                               );
                } else {
                    err = pPipeline->submitRequest(
                                    requestNumber,
                                    settingAppMeta,
                                    settingHalMeta,
                                    vDstStreams
                               );
                }
            }
            MY_LOGE_IF(err != OK, "request %d submit fail", requestNumber);

        }
        return true;
    }

    MY_LOGD("RequestThread exit.");
    return false;
}
