/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/MShot"
//
#include <sys/stat.h>
#include <mtkcam/utils/std/Log.h>
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __func__, ##arg)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
//
#include <mtkcam/def/common.h>
#include <mtkcam/middleware/v1/common.h>
#include <mtkcam/utils/std/common.h>
using namespace android;
//
#include <mtkcam/middleware/v1/camshot/_callbacks.h>
#include <mtkcam/middleware/v1/camshot/_params.h>
//
#include <mtkcam/utils/metadata/IMetadata.h>
using namespace NSCam;

#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/Selector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/NormalCShotSelector.h>

using namespace NSCam::v1::NSLegacyPipeline;
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>

#include <mtkcam/utils/hw/HwInfoHelper.h>
using namespace NSCamHW;

#include <mtkcam/utils/hw/IScenarioControl.h>

#include "../inc/CamShotImp.h"
#include "../inc/MultiShot.h"
#include <mtkcam/middleware/v1/camshot/CamShotUtils.h>
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
using namespace NSCam::v1;
//
#include <mtkcam/utils/hw/CamManager.h>
using namespace NSCam::Utils;
//for LCSO
#include <mtkcam/aaa/IIspMgr.h>

#include <cutils/properties.h>
#define DUMP_KEY  "debug.multishot.dump"
#define DUMP_PATH "/sdcard/multishot"

//default use memory optimization or not
#define MSHOT_MEM_OPT (1)

//default use p2node optimization or not
#define MSHOT_P2_OPT (1)

#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {

/*******************************************************************************
*
********************************************************************************/
MultiShot::
MultiShot(
    EShotMode const eShotMode,
    char const*const szCamShotName,
    EPipelineMode const ePipelineMode
)
    : CamShotImp(eShotMode, szCamShotName, ePipelineMode)
    , mShotCount(0)
    , mCurCount(0)
    //
    , mSensorSize(MSize(0,0))
    , mSensorFps(0)
    , mPixelMode(0)
    //
    , mpInfo_FullRaw(NULL)
    , mpInfo_LcsoRaw(NULL)
    //
    , mpPreviewPipeline(NULL)
    , mpCallbackHandler(NULL)
    , mpImgoPool(NULL)
    , mpLcsoPool(NULL)
    , mpPreviewThread(NULL)
    //
    , mRequestNumber(1001)
    , mRequestNumberMin(1001)
    , mRequestNumberMax(2000)
    //
    , mbNeedPreview(MFALSE)
    , mbTorchFlash(MFALSE)
    , mbToStop(MFALSE)
    , mbOnStop(MFALSE)
    , mbDoShutterCb(MTRUE)
    , mFinishedData(0x0)
    , mCurSubmitCount(0)
    //
    , muNRType(ECamShot_NRTYPE_NONE)
    , mbEnGPU(ECamShot_RWB_NONE)
    //
    , mbMemOpt((MSHOT_MEM_OPT > 0) ? MTRUE : MFALSE)
    , mbP2Opt((MSHOT_P2_OPT > 0) ? MTRUE : MFALSE)
    , mTargetDurationMs(50)
    //
    , muCaptureFps(0)
    , muPreviewFps(0)
    //
    , mPrvPipeLCS(LCSO_RUNTIME_DECIDE)
    , mCapPipeLCS(LCSO_RUNTIME_DECIDE)
    , mbPrvEnableLCS(MFALSE)
    , mbCapEnableLCS(MFALSE)
    , mpDispatcher(NULL)
    , mpInfo_YuvThumbnail(NULL)
{
    MINT32 mkdirRet = 0;
    mDumpFlag = property_get_int32(DUMP_KEY, 0);
    if( mDumpFlag ) {
        mkdirRet = mkdir(DUMP_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
    }
    //
    MINT32 cshotCapFps = property_get_int32("debug.cshot.cap.fps", -1);
    if( cshotCapFps > 0 )
    {
        muCaptureFps = cshotCapFps;
    }
    //
    MINT32 prvPipeLCS = property_get_int32("debug.cshot.prv.lcs", -1);
    if( prvPipeLCS >= 0 )
    {
        mPrvPipeLCS = prvPipeLCS;
    }
    MINT32 capPipeLCS = property_get_int32("debug.cshot.cap.lcs", -1);
    if( capPipeLCS >= 0 )
    {
        mCapPipeLCS = capPipeLCS;
    }
    //
    MINT32 memOpt = property_get_int32("debug.cshot.opt.mem", -1);
    if( memOpt >= 0 )
    {
        mbMemOpt = (memOpt > 0) ? MTRUE : MFALSE;
    }
    MINT32 p2Opt = property_get_int32("debug.cshot.opt.p2", -1);
    if( p2Opt >= 0 )
    {
        mbP2Opt = (p2Opt > 0) ? MTRUE : MFALSE;
    }
    MINT32 duration = property_get_int32("debug.cshot.target.duration", -1);
    if( duration >= 0 )
    {
        mTargetDurationMs = duration;
    }
    MY_LOGI("mPrvPipeLCS(%d), mCapPipeLCS(%d), muCaptureFps(%d), mbMemOpt(%d), mbP2Opt(%d), mTargetDurationMs(%" PRId64 ")"
            ", dump flag(0x%x), mkdirRet(%d)"
            , mPrvPipeLCS, mCapPipeLCS, muCaptureFps, mbMemOpt, mbP2Opt, mTargetDurationMs, mDumpFlag, mkdirRet);
}


/*******************************************************************************
*
********************************************************************************/
MultiShot::
~MultiShot()
{
    FUNC_START;
#if 0
    if( mpCapturePipeline.get() )
    {
        mpCapturePipeline->flush();
    }

    if( mpPreviewPipeline.get() )
    {
        mpPreviewPipeline->flush();
    }
#endif
    FUNC_END;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
init()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    FUNC_END;

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
uninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    FUNC_END;

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

/******************************************************************************
 * get newest requests (at most maxCount requests) from selector
 ******************************************************************************/
MBOOL
MultiShot::
getRequest(sp<NSCam::v1::ISelector> spSelector, Vector<Request>& vRequest, MINT32 maxCount)
{
    //check inputs
    if( spSelector == NULL )
    {
        MY_LOGW("selector is NULL");
        return MFALSE;
    }
    //
    if( spSelector->getSelectorType() != ISelector::SelectorType_ZsdSelector )
    {
        MY_LOGW("selector type (%d) is not supported", spSelector->getSelectorType());
        return MFALSE;
    }
    //
    if( maxCount <= 0 )
    {
        MY_LOGW("maxCount (%d) is not supported", maxCount);
        return MFALSE;
    }
    //
    MY_LOGD("+ already has (%d) request, maxCount (%d)", vRequest.size(), maxCount);
    //
    //return no need requests to selector
    if( vRequest.size() > maxCount )
    {
        Vector<ISelector::BufferItemSet> bufferSet;
        while( vRequest.size() > maxCount )
        {
            bufferSet = vRequest[0].bufferSet;
            for(size_t i = 0; i < bufferSet.size() ; i++)
            {
                spSelector->returnBuffer(bufferSet.editItemAt(i));
            }
            vRequest.removeAt(0);
        }
    }
    //
    //get all requests in selector and return old requests
    {
        Vector<MINT32> vRequestNo;
        Vector< Vector<ISelector::MetaItemSet> > vResultMeta;
        Vector< Vector<ISelector::BufferItemSet> > vBuffers;
        MINT32 timeoutMs = 0;
        //
        while( 1 )
        {
            vRequestNo.clear();
            vResultMeta.clear();
            vBuffers.clear();
            vRequestNo.resize(1);
            if( static_cast<ZsdSelector*>(spSelector.get())->getContinuousResults(vRequestNo, vResultMeta, vBuffers, timeoutMs) == OK )
            {
                vRequest.push_back(Request(vRequestNo[0], vResultMeta[0], vBuffers[0]));
                if( vRequest.size() > maxCount )
                {
                    Vector<ISelector::BufferItemSet> bufferSet;
                    bufferSet = vRequest[0].bufferSet;
                    for(size_t i = 0; i < bufferSet.size() ; i++)
                    {
                        spSelector->returnBuffer(bufferSet.editItemAt(i));
                    }
                    vRequest.removeAt(0);
                }
            }
            else
            {
                break;
            }
        }
    }
    //
    MY_LOGD("- result (%d) requests, maxCount (%d)", vRequest.size(), maxCount);
    //
    return MTRUE;
}

/*******************************************************************************
* (1) select first and second capture frame from vCandidateRequest to vTodoRequest,
*     second capture frame should be the newest one,
*     first capture frame should be a rollback frame,
*     the frame duration of first and second frame should be as close as possible to mTargetDurationMs
* (2) if cannot find suitable request for first capture frame,
*     vTodoRequest may have only one request(should be the newest one)
********************************************************************************/
MVOID
MultiShot::
selectTodoRequest(Vector<Request>& vCandidateRequest, Vector<Request>& vTodoRequest)
{
    //check input
    if( vCandidateRequest.size() == 0 )
    {
        MY_LOGW("candidate request is empty");
        return;
    }
    //
    if( vTodoRequest.size() > 0 )
    {
        MY_LOGW("todo request size (%d) is not empty, clear requests", vTodoRequest.size());
        vTodoRequest.clear();
    }
    //
    if( mTargetDurationMs > 200 || mTargetDurationMs < 33 )
    {
        MY_LOGW("mTargetDurationMs(%" PRId64 ") seems wrong...", mTargetDurationMs);
    }
    //
    MY_LOGD("+ candidate (%d), mTargetDurationMs (%" PRId64 ")", vCandidateRequest.size(), mTargetDurationMs);
    //
    //if only one candidate
    if( vCandidateRequest.size() == 1 )
    {
        vTodoRequest.push_back(vCandidateRequest[0]);
        vCandidateRequest.removeAt(0);
        MY_LOGD("- result todo size (%d)", vTodoRequest.size());
        return;
    }
    //
    //get all timestamp
    KeyedVector<MINT64,MINT32> vTimeIndex;
    vTimeIndex.clear();
    MINT32 i = 0, j = 0;
    MINT64 time = 0;
    for( i = 0 ; i < vCandidateRequest.size() ; i++ )
    {
        time = 0;
        for( j = 0 ; j < vCandidateRequest[i].meta.size() ; j++ )
        {
            ISelector::MetaItemSet metaSet = vCandidateRequest[i].meta[j];
            tryGetMetadata<MINT64>(&(metaSet.meta), MTK_SENSOR_TIMESTAMP, time);
            if( time > 0 )
            {
                vTimeIndex.add(time, i);
                break;
            }
        }
    }
    if( vTimeIndex.size() == 0 )
    {
        vTodoRequest.push_back(vCandidateRequest[vCandidateRequest.size()-1]);
        vCandidateRequest.removeAt(vCandidateRequest.size()-1);
        MY_LOGW("- all timestamps are 0, use last request");
        return;
    }
    //
    //get newest request
    MINT64 newestTime = vTimeIndex.keyAt(0);
    MINT32 newestIndex = vTimeIndex.valueAt(0);
    for( i = 1 ; i < vTimeIndex.size() ; i++ )
    {
        if( vTimeIndex.keyAt(i) > newestTime )
        {
            newestTime = vTimeIndex.keyAt(i);
            newestIndex = vTimeIndex.valueAt(i);
        }
    }

    if (__builtin_expect( newestIndex < 0, false )) {
        MY_LOGE("get newest request but the index < 0");
    }
    else {
        MY_LOGD("get newest request, time(%" PRId64 "), index(%d), requestNo(%d)"
                , newestTime, newestIndex, vCandidateRequest[newestIndex].requestNo);
    }
    //
    //select best rollback request
    MINT64 rollbackTime = newestTime, candidateTime = 0, timeDiff = 0, rollbackDiff = 0;
    MINT32 rollbackIndex = newestIndex;
    /*******************************************************************************
    *                                <magic strategy>
    *  too old  | a little old |   perfect    | a little new |      too new
    *  not use  | 2nd priority | 1st priority | 3rd priority | should not exist
    *-----------+--------------+--------------+--------------+------------------>time
    *     (target-16ms)     (target)    (target+33ms)     (newest)
    * ps : target = newest - mTargetDurationMs
    ********************************************************************************/
    for( i = 0 ; i < vTimeIndex.size() ; i++ )
    {
        //it's newest request, skip
        if( vTimeIndex.valueAt(i) == newestIndex )
        {
            continue;
        }
        //
        timeDiff = newestTime - vTimeIndex.keyAt(i);
        rollbackDiff = newestTime - rollbackTime;
        MY_LOGD("times/diff : newest(%" PRId64 "), rollback(%" PRId64 "/%" PRId64 "), candidate(%" PRId64 "/%" PRId64 ")"
                , newestTime, rollbackTime, rollbackDiff, vTimeIndex.keyAt(i), timeDiff);
        //too new(should not exist) or too old(not use)
        if( timeDiff <= 0 || timeDiff > (mTargetDurationMs+16)*1000*1000 )
        {
            continue;
        }
        //perfect, 1st priority
        if( timeDiff >= (mTargetDurationMs-33)*1000*1000 && timeDiff <= mTargetDurationMs*1000*1000 )
        {
            rollbackTime = vTimeIndex.keyAt(i);
            rollbackIndex = vTimeIndex.valueAt(i);
            break;
        }
        //a little old, 2nd priority
        if( timeDiff > mTargetDurationMs*1000*1000 && timeDiff <= (mTargetDurationMs+16)*1000*1000 )
        {
            rollbackTime = vTimeIndex.keyAt(i);
            rollbackIndex = vTimeIndex.valueAt(i);
            continue;
        }
        //a little new, 3rd priority
        if( timeDiff < (mTargetDurationMs-33)*1000*1000 )
        {
            //rollback is 2nd priority
            if( rollbackDiff > mTargetDurationMs*1000*1000 && rollbackDiff <= (mTargetDurationMs+16)*1000*1000 )
            {
                continue;
            }
            //rollback is 3rd priority, selector older one
            if( rollbackDiff > 0 && rollbackDiff < (mTargetDurationMs-33)*1000*1000 )
            {
                if( timeDiff > rollbackDiff )
                {
                    rollbackTime = vTimeIndex.keyAt(i);
                    rollbackIndex = vTimeIndex.valueAt(i);
                }
                continue;
            }
            //rollback is newest
            rollbackTime = vTimeIndex.keyAt(i);
            rollbackIndex = vTimeIndex.valueAt(i);
            continue;
        }
    }
    //
    //move from vCandidateRequest to vTodoRequest
    MY_LOGD("newest : time(%" PRId64 "), index(%d); rollback : time(%" PRId64 "), index(%d); target duration(%" PRId64 ")"
            , newestTime, newestIndex, rollbackTime, rollbackIndex, mTargetDurationMs);
    if( newestIndex == rollbackIndex )
    {
        vTodoRequest.push_back(vCandidateRequest[newestIndex]);
        vCandidateRequest.removeAt(newestIndex);
    }
    else
    {
        if (__builtin_expect( rollbackIndex < 0 || newestIndex < 0, false ))
        {
            MY_LOGE("rollbackIndex(%d) < 0 or newestIndex(%d) < 0",
                    rollbackIndex, newestIndex);
            *(volatile uint32_t*)(0x00000000) = 0xdeadfeed; // cause NE
            return;
        }
        else
        {
            vTodoRequest.push_back(vCandidateRequest[rollbackIndex]);
            vTodoRequest.push_back(vCandidateRequest[newestIndex]);
        }
        //
        if( newestIndex > rollbackIndex )
        {
            vCandidateRequest.removeAt(newestIndex);
            vCandidateRequest.removeAt(rollbackIndex);
        }
        else
        {
            vCandidateRequest.removeAt(rollbackIndex);
            vCandidateRequest.removeAt(newestIndex);
        }
    }
    MY_LOGD("- result todo size (%d)", vTodoRequest.size());
}

/*******************************************************************************
*
********************************************************************************/
MVOID
MultiShot::
clearRequest(sp<NSCam::v1::ISelector> spSelector, Vector<Request>& vRequest)
{
    //check inputs
    if( spSelector == NULL )
    {
        MY_LOGW("selector is NULL");
        return;
    }
    //
    //return buffer to selector
    if( vRequest.size() > 0 )
    {
        Vector<Request>::iterator iter = vRequest.begin();
        for( ; iter != vRequest.end() ; iter++ )
        {
            for(size_t i = 0; i < iter->bufferSet.size() ; i++)
            {
                spSelector->returnBuffer(iter->bufferSet.editItemAt(i));
            }
        }
    }
    vRequest.clear();
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
start(SensorParam const & rSensorParam, MUINT32 count)
{
    FUNC_START;
    MBOOL ret = MTRUE;
    sp<CamManager::UsingDeviceHelper> spDeviceHelper = NULL;

    mSensorParam = rSensorParam;
    dumpSensorParam(mSensorParam);
    mShotCount = count;
    //update jpeg quality
    {
        MINT32 useQuality = mJpegParam.u4Quality;
        MINT32 propQuality = property_get_int32("debug.cshot.quality", -1);
        if( propQuality >= 0 )
        {
            useQuality = propQuality;
        }
        MY_LOGD("update jpeg quality to %d", useQuality);
        updateEntry<MUINT8>(&mShotParam.appSetting , MTK_JPEG_QUALITY , useQuality);
    }
    //
    if ( ! isDataMsgEnabled(ECamShot_DATA_MSG_ALL) && ! isNotifyMsgEnabled(ECamShot_DATA_MSG_ALL) )
    {
        MY_LOGE("no data/msg enabled");
        return MFALSE;
    }

    MY_LOGD("start multishot, shot count:%d, mbNeedPreview:%d", mShotCount, mbNeedPreview);
    if( mbNeedPreview )
    {
        //normal continuous shot, create preview pipeline
        spDeviceHelper = new CamManager::UsingDeviceHelper(mSensorParam.u4OpenID);
        if( OK != createPreviewPipeline() ) {
            MY_LOGE("createPreviewPipeline failed");
            return MFALSE;
        }
        spDeviceHelper->configDone();

        MINT32 aStartRequestNumber = 0;
        MINT32 aEndRequestNumber = 1000;

        IMetadata appMeta = mPreviewAppMetadata;
        IMetadata halMeta = mPreviewHalMetadata;
        {
            IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
            entry.push_back(mPreviewSensorParam.size, Type2Type< MSize >());
            halMeta.update(entry.tag(), entry);
        }

        mpPreviewThread = new MShotPreviewThread();
        mpPreviewThread->start(
                            mpPreviewPipeline,
                            aStartRequestNumber,
                            aEndRequestNumber,
                            appMeta,
                            halMeta
                        );

        if( mpPreviewThread->run("MShot") != OK ) {
            MY_LOGE("preview thread init fail.");
            return MFALSE;
        }
    }
    else
    {
        sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(mSensorParam.u4OpenID);
        if( pScenarioCtrl.get() == NULL )
        {
            MY_LOGE("get Scenario Control fail");
        }
        else
        {
            pScenarioCtrl->enterScenario(IScenarioControl::Scenario_ContinuousShot);
        }
    }

    //capture pipeline
    {
        //for p2 optimize flow
        Vector<Request> vCandidateRequest;//reqeusts get from selector, candidates of capture frame
        Vector<Request> vTodoRequest;//requests that will be used as capture frame
        vCandidateRequest.clear();
        vTodoRequest.clear();

        //ZSD continuous shot, get provider selector from consumer container
        sp<IResourceContainer> spContainer = IResourceContainer::getInstance(mSensorParam.u4OpenID);
        if( spContainer.get() == NULL )
        {
            MY_LOGE("spContainer.get() == NULL");
            return MFALSE;
        }

        sp<StreamBufferProvider> spProvider = spContainer->queryConsumer(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        if( spProvider.get() == NULL )
        {
            MY_LOGE("spProvider.get() == NULL");
            return MFALSE;
        }

        mpSelector = spProvider->querySelector();
        if ( mpSelector == 0 ) {
            MY_LOGE("get selector failed");
            return MFALSE;
        }
        mpSelector->sendCommand(ISelector::eCmd_setNoWaitAfDone, 0, 0, 0);
        mpSelector->sendCommand(ISelector::eCmd_setUseNewestBuffer, 1, 0, 0);
        MY_LOGD("set selector no need wait AF done, use newest buffer (+)");
        //
        if( mbP2Opt )
        {
            MINT32 maxBufCount = 4;
            getRequest(mpSelector, vCandidateRequest, maxBufCount);
        }
        mpSelector->flush();
        //
        if ( ! createCaptureStreams(spProvider.get()) ) {
            MY_LOGE("createCaptureStreams failed");
            return MFALSE;
        }
        //
        if ( ! createCapturePipeline(spProvider.get()) ) {
            MY_LOGE("createCapturePipeline failed");
            return MFALSE;
        }
        //
        mRequestNumber = mRequestNumberMin;
        MINT32 AeDropCount = 0;
        MINT32 AeTimeOutCount = 40;//avoid AE forever searching
        if( mbNeedPreview && mbTorchFlash )
        {
            AeDropCount = 4;//normal CShot with flash
        }
        while( !mbOnStop )
        {
            timeval startCapture;
            ::gettimeofday(&startCapture, NULL);
            IMetadata appCaptureSetting = mShotParam.appSetting;
            IMetadata halCaptureSetting = mShotParam.halSetting;

            //get meta, buffer from selector or vTodoRequest
            MINT32 rRequestNo = -1;
            Vector<ISelector::MetaItemSet> rvResultMeta;
            IMetadata selectorAppMetadata;
            Vector<ISelector::BufferItemSet> bufferSet;
            if( vTodoRequest.size() == 0 && vCandidateRequest.size() > 0 )
            {
                MINT32 maxBufCount = 5;
                getRequest(mpSelector, vCandidateRequest, maxBufCount);
                selectTodoRequest(vCandidateRequest, vTodoRequest);
                clearRequest(mpSelector, vCandidateRequest);//return no need buffer(in vCandidateRequest)
            }
            if( vTodoRequest.size() > 0 )
            {
                rRequestNo = vTodoRequest[0].requestNo;
                rvResultMeta = vTodoRequest[0].meta;
                bufferSet = vTodoRequest[0].bufferSet;
                vTodoRequest.removeAt(0);
                MY_LOGI("use first todo request, requestNo(%d), remain todo size(%d)", rRequestNo, vTodoRequest.size());
            }
            else
            {
                status_t err = mpSelector->getResult(rRequestNo, rvResultMeta, bufferSet);
                if( err )
                {
                    MY_LOGE("get selector result fail, %s", strerror(-err));
                    break;
                }
                mpSelector->flush();//prevent from get older buffer next time
            }

            //check buffer: make sure we have all needed buffers(IMGO/LCSO), and return all unneeded buffers(RRZO/LCSO/...)
            MBOOL hasLcso = MFALSE, hasImgo = MFALSE;
            for( size_t i = 0 ; i < bufferSet.size() ; i++ )
            {
                if( bufferSet[i].heap.get() == NULL )
                {
                    MY_LOGE("get NULL buffer, (i,id) = (%d,%#" PRIxPTR ")", i, bufferSet[i].streamInfo->getStreamId());
                    break;
                }
                int64_t streamId = bufferSet[i].streamInfo->getStreamId();
                switch( streamId )
                {
                    case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                    {
                        hasImgo = MTRUE;
                        break;
                    }
                    case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                    {
                        hasLcso = MTRUE;
                        if( mbCapEnableLCS == MFALSE )
                        {
                            mpSelector->returnBuffer(bufferSet.editItemAt(i));
                            bufferSet.removeAt(i);
                            i--;
                        }
                        break;
                    }
                    default:
                    {
                        mpSelector->returnBuffer(bufferSet.editItemAt(i));
                        bufferSet.removeAt(i);
                        i--;
                        break;
                    }
                }
            }
            if( hasImgo == MFALSE || (mbCapEnableLCS == MTRUE && hasLcso == MFALSE) )
            {
                MY_LOGE("need IMGO/LCSO = 1/%d, but found IMGO/LCSO = %d/%d", mbCapEnableLCS, hasImgo, hasLcso);
                for(size_t i = 0; i < bufferSet.size() ; i++)
                {
                    mpSelector->returnBuffer(bufferSet.editItemAt(i));
                }
                bufferSet.clear();
                break;
            }

            //check meta
            if(rvResultMeta.size()!=2)
            {
                MY_LOGW("ZsdSelect getResult rvResultMeta != 2");
                for(size_t i = 0; i < bufferSet.size() ; i++)
                {
                    mpSelector->returnBuffer(bufferSet.editItemAt(i));
                }
                bufferSet.clear();
                continue;
            }
            //
            MBOOL bAppSetting = MFALSE, bHalSetting = MFALSE;
            for(MUINT32 i=0; i<rvResultMeta.size(); i++)
            {
                if(rvResultMeta.editItemAt(i).id == eSTREAMID_META_APP_DYNAMIC_P1)
                {
                    selectorAppMetadata = rvResultMeta.editItemAt(i).meta;
                    bAppSetting = MTRUE;
                }
                else if(rvResultMeta.editItemAt(i).id == eSTREAMID_META_HAL_DYNAMIC_P1)
                {
                    halCaptureSetting = rvResultMeta.editItemAt(i).meta;
                    bHalSetting = MTRUE;
                }
            }
            //
            if( !bHalSetting ||
                !bAppSetting)
            {
                MY_LOGW("can't find App(%d)/Hal(%d) Setting from Select getResult rvResultMeta",
                        bAppSetting,
                        bHalSetting);
                for(size_t i = 0; i < bufferSet.size() ; i++)
                {
                    mpSelector->returnBuffer(bufferSet.editItemAt(i));
                }
                bufferSet.clear();
                continue;
            }
            //
            MUINT8 AeState = 0;
            tryGetMetadata< MUINT8 >(&selectorAppMetadata, MTK_CONTROL_AE_STATE, AeState);
            MY_LOGD("reqNo(%d), AeState(%d), AeDropCount(%d), AeTimeOutCount(%d)",
                    rRequestNo, AeState, AeDropCount, AeTimeOutCount);
            if( AeTimeOutCount > 0 &&
                (AeDropCount > 0 ||
                 (mRequestNumberMin == mRequestNumber &&
                  MTK_CONTROL_AE_STATE_CONVERGED != AeState &&
                  MTK_CONTROL_AE_STATE_FLASH_REQUIRED != AeState &&
                  MTK_CONTROL_AE_STATE_LOCKED != AeState) ) )
            {
                MY_LOGD("first frame need AE converge, drop this frame");
                for(size_t i = 0; i < bufferSet.size() ; i++)
                {
                    mpSelector->returnBuffer(bufferSet.editItemAt(i));
                }
                bufferSet.clear();
                if( AeDropCount > 0 )
                {
                    AeDropCount--;
                }
                if( AeTimeOutCount > 0 )
                {
                    AeTimeOutCount--;
                }
                continue;
            }
            AeTimeOutCount = 0;
            //
            sp<TimestampProcessor> pTimestampProcessor = mpCapturePipeline->getTimestampProcessor().promote();
            pTimestampProcessor->onResultReceived(
                                    mRequestNumber,
                                    eSTREAMID_META_APP_DYNAMIC_P1,
                                    MFALSE,
                                    selectorAppMetadata);
            //
            {
                IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
                entry.push_back(mSensorSize, Type2Type< MSize >());
                halCaptureSetting.update(entry.tag(), entry);
            }
            {
                IMetadata::IEntry entry(MTK_ISP_P2_TUNING_UPDATE_MODE);
                entry.push_back(5, Type2Type< MUINT8 >());
                halCaptureSetting.update(entry.tag(), entry);
            }

            //push buffer to virtual buffer pool
            for(size_t i = 0; i < bufferSet.size() ; i++)
            {
                if( bufferSet[i].streamInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE )
                {
                    mpImgoPool->onReceiveBufferHeap(bufferSet[i].heap);
                }
                if( mbCapEnableLCS == MTRUE && bufferSet[i].streamInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO )
                {
                    mpLcsoPool->onReceiveBufferHeap(bufferSet[i].heap);
                }
            }
            //
            {
                Mutex::Autolock _l(mLock);
                mCurSubmitCount++;
                MY_LOGD("CurSubmitCount %d", mCurSubmitCount);
            }
            if( OK != mpCapturePipeline->submitSetting(
                                            mRequestNumber,
                                            appCaptureSetting,
                                            halCaptureSetting))
            {
                Mutex::Autolock _l(mLock);
                mCurSubmitCount--;
                MY_LOGW("capture pipeline submitRequest failed, CurSubmitCount %d", mCurSubmitCount);
                continue;
            }
            //
            if( mbP2Opt && (mRequestNumber == mRequestNumberMin) )
            {
                MY_LOGI("queue 2 requests at initial");
            }
            else
            {
                {
                    MY_LOGV("wait mNextFrameCond +");
                    Mutex::Autolock _l(mLock);
                    if (mCurSubmitCount > 0)
                    {
                        mNextFrameCond.wait(mLock);
                    }
                    else
                    {
                        MY_LOGW("mCurSubmitCount = %d, don't wait for mNextFrameCond !!", mCurSubmitCount);
                    }
                    MY_LOGV("wait mNextFrameCond -");
                }

                if( muCaptureFps > 0 )
                {
                    // fps control wait here
                    timeval tv;
                    ::gettimeofday(&tv, NULL);
                    MINT32 uCaptureInterval = 1000000 / muCaptureFps;
                    MINT32 usDiff = (tv.tv_sec - startCapture.tv_sec)*1000000 + tv.tv_usec - startCapture.tv_usec;
                    MINT32 usWait = uCaptureInterval - usDiff;
                    MY_LOGD("muCaptureFps:%d, usWait:%d", muCaptureFps, usWait);
                    if( usWait > 0 )
                    {
                        ::usleep(usWait);
                    }
                }
            }

            mRequestNumber++;
            if( mRequestNumber > mRequestNumberMax )
            {
                mRequestNumber = mRequestNumberMin;
            }
        }

        MY_LOGD("vCandidateRequest size(%d), vTodoRequest size(%d)", vCandidateRequest.size(), vTodoRequest.size());
        clearRequest(mpSelector, vCandidateRequest);
        clearRequest(mpSelector, vTodoRequest);
    }

    //stop
    if( mpCapturePipeline.get() )
    {
        mpCapturePipeline->flush();
        mpCapturePipeline->waitUntilDrained();
    }
    if( mpCallbackHandler.get() )
    {
        mpCallbackHandler = NULL;
    }
    if( mpCapturePipeline.get() )
    {
        mpCapturePipeline = NULL;
    }
    if( mpDispatcher.get() )
    {
        mpDispatcher = NULL;
    }
    if( mpImgoPool.get() )
    {
        mpImgoPool = NULL;
    }
    if( mpLcsoPool.get() )
    {
        mpLcsoPool = NULL;
    }
    //
    if( mpSelector.get() )
    {
        mpSelector->sendCommand(ISelector::eCmd_setNeedWaitAfDone, 0, 0, 0);
        mpSelector->sendCommand(ISelector::eCmd_setUseNewestBuffer, 0, 0, 0);
        MY_LOGD("set selector no need wait AF done, use newest buffer (-)");
        //
        mpSelector = NULL;
    }
    //
    if( mpPreviewThread.get() )
    {
        mpPreviewThread->stop();
        mpPreviewThread = NULL;
    }
    if( mpPreviewPipeline.get() )
    {
        mpPreviewPipeline->flush();
        mpPreviewPipeline->waitUntilDrained();
        mpPreviewPipeline = NULL;
        spDeviceHelper = NULL;
    }
    //
    FUNC_END;
    //
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
startAsync(SensorParam const & /*rSensorParam*/)
{
    FUNC_START;
    //
    MY_LOGE("not supported yet");
    //
    FUNC_END;
    //
    return MFALSE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
stop()
{
    FUNC_START;

    Mutex::Autolock _l(mLock);
    mbToStop = MTRUE;

    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
setShotParam(ShotParam const & rParam)
{
    FUNC_START;
    mShotParam = rParam;
    //
    dumpShotParam(mShotParam);

    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
setJpegParam(JpegParam const & rParam)
{
    FUNC_START;
    mJpegParam = rParam;
    //
    dumpJpegParam(mJpegParam);

    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
registerImageBuffer(ECamShotImgBufType const /*eBufType*/, IImageBuffer const * /*pImgBuffer*/)
{
    FUNC_START;
#if 0
    if(pImgBuffer==NULL)
    {
        MY_LOGE("register NULL buffer!");
        return MFALSE;
    }
    IImageBuffer* pBuf = const_cast<IImageBuffer*>(pImgBuffer);
    if(mvRegBuf.indexOfKey(eBufType) >= 0)
    {
        mvRegBuf.replaceValueFor(eBufType, pBuf);
    }
    else
    {
        mvRegBuf.add(eBufType, pBuf);
    }
    //
#endif
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 /*arg2*/, MINT32 /*arg3*/, MVOID* arg4)
{
    switch( cmd )
    {
        case ECamShot_CMD_SET_NRTYPE:
            muNRType = arg1;
            MY_LOGD("NR type 0x%x", muNRType);
            break;
        case ECamShot_CMD_SET_RWB_PROC_TYPE:
            mbEnGPU = arg1; //enable GPU process
            MY_LOGD("GPU process 0x%x", mbEnGPU);
            break;
        case ECamShot_CMD_SET_CSHOT_SPEED:
            if( arg1 > 0 )
            {
                Mutex::Autolock _l(mLock);
                muCaptureFps = arg1;
                MY_LOGI("set capture speed %d", muCaptureFps);
            }
            else
            {
                MY_LOGW("set invalid shot speed %d", arg1);
                return MFALSE;
            }
            break;
        case ECamShot_CMD_SET_PRV_META:
            mPreviewAppMetadata = (*reinterpret_cast<PreviewMetadata const*>(arg4)).mPreviewAppSetting;
            mPreviewHalMetadata = (*reinterpret_cast<PreviewMetadata const*>(arg4)).mPreviewHalSetting;
            mbNeedPreview = MTRUE;
            {
                MUINT8 flashMode = 0, controlAeMode = 0;
                tryGetMetadata< MUINT8 >(&mPreviewAppMetadata, MTK_FLASH_MODE, flashMode);
                tryGetMetadata< MUINT8 >(&mPreviewAppMetadata, MTK_CONTROL_AE_MODE, controlAeMode);
                if( flashMode == MTK_FLASH_MODE_TORCH && controlAeMode == MTK_CONTROL_AE_MODE_ON )
                {
                    mbTorchFlash = MTRUE;
                }
                else
                {
                    mbTorchFlash = MFALSE;
                }
                MY_LOGI("flashMode(%d) controlAeMode(%d) mbTorchFlash(%d)", flashMode, controlAeMode, mbTorchFlash);
            }
            break;
        default:
            MY_LOGE("not supported cmd 0x%x", cmd);
            return MFALSE;
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
MultiShot::
getRotation() const
{
    return mShotParam.u4PictureTransform;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
MultiShot::
getCapBufCount(StreamId_T streamId, MINT32 type)
{
    MINT32 ret = 0;
    switch( streamId )
    {
        case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
            {
                if( mbMemOpt )
                {
                    ret = (type == eMAX) ? 2 : 1;
                }
                else
                {
                    ret = 3; //(type == eMAX) ? 3 : 3;
                }
                if( mbP2Opt )
                {
                    ret = ret + 1;
                }
            }
            break;
        case eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL:
            {
                if( mbMemOpt )
                {
                    ret = (type == eMAX) ? 2 : 1;
                }
                else
                {
                    ret = 3; // (type == eMAX) ? 3 : 3;
                }
                if( mbP2Opt )
                {
                    ret = ret + 1;
                }
            }
            break;
        case eSTREAMID_IMAGE_JPEG:
            {
                if( mbMemOpt )
                {
                    ret = 1; // (type == eMAX) ? 1 : 1;
                }
                else
                {
                    ret = 2; // (type == eMAX) ? 2 : 2;
                }
            }
            break;
        default:
            MY_LOGW("unsupport stream(%#" PRIxPTR "), type(%d), ret(%d)", streamId, type, ret);
            return ret;
    }
    MY_LOGD("stream(%#" PRIxPTR "), type(%d), mem opt(%d), p2 opt(%d), ret(%d)", streamId, type, mbMemOpt, mbP2Opt, ret);
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
createCaptureStreams(StreamBufferProvider* pProvider)
{
    FUNC_START;
    if( pProvider == nullptr )
    {
        MY_LOGE("provider is NULL");
        return MFALSE;
    }

    MSize const previewsize   = MSize(mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight);
    MINT const previewfmt     = mShotParam.ePostViewFmt;
    MINT const yuvfmt         = mShotParam.ePictureFmt;
    MSize const jpegsize      = (getRotation() & eTransform_ROT_90) ?
        MSize(mShotParam.u4PictureHeight, mShotParam.u4PictureWidth):
        MSize(mShotParam.u4PictureWidth, mShotParam.u4PictureHeight);
    MSize const thumbnailsize = MSize(mJpegParam.u4ThumbWidth, mJpegParam.u4ThumbHeight);

    //get sensor size
    {
        MUINT32 const openId     = mSensorParam.u4OpenID;
        MUINT32 const sensorMode = mSensorParam.u4Scenario;
        HwInfoHelper helper(openId);
        if( ! helper.updateInfos() ) {
            MY_LOGE("cannot properly update infos");
            return MFALSE;
        }
        if( ! helper.getSensorSize( sensorMode, mSensorSize) ||
            ! helper.getSensorFps( sensorMode, mSensorFps) ||
            ! helper.queryPixelMode( sensorMode, mSensorFps, mPixelMode)
        ) {
            MY_LOGE("cannot get params about sensor");
            return MFALSE;
        }
    }
    //
    Vector<sp<IImageStreamInfo>> rawInputInfos;
    pProvider->querySelector()->queryCollectImageStreamInfo( rawInputInfos );
    for(size_t i = 0 ; i < rawInputInfos.size() ; i++)
    {
        if( rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE )
        {
            mpInfo_FullRaw = rawInputInfos[i];
        }
        if( rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO )
        {
            mpInfo_LcsoRaw = rawInputInfos[i];
        }
        MY_LOGD("zsd raw(%d) stream (%#" PRIx64 ")(%s) size(%dx%d), fmt 0x%x",
            i,
            rawInputInfos[i]->getStreamId(),
            rawInputInfos[i]->getStreamName(),
            rawInputInfos[i]->getImgSize().w,
            rawInputInfos[i]->getImgSize().h,
            rawInputInfos[i]->getImgFormat()
           );
    }
    //
    //decide use lcso or not
    switch( mCapPipeLCS )
    {
        case LCSO_FORCE_OFF:
            mbCapEnableLCS = MFALSE;
            break;
        case LCSO_FORCE_ON:
            mbCapEnableLCS = MTRUE;
            break;
        case LCSO_RUNTIME_DECIDE:
            mbCapEnableLCS = (rawInputInfos.size() == 1) ? MFALSE : MTRUE;
            break;
        case LCSO_GET_PROPERTY:
            mbCapEnableLCS = (property_get_int32("debug.open.lcs", 0) > 0);
            break;
        default:
            MY_LOGW("un-support preview lcso option (%d)", mCapPipeLCS);
            break;
    }
    if( rawInputInfos.size() == 1 && mbCapEnableLCS == MTRUE )
    {
        MY_LOGE("want to enable lcso but selector only has one buffer");
    }
    if( rawInputInfos.size() == 2 && mbCapEnableLCS == MFALSE )
    {
        MY_LOGI("selector has two buffers but disable lcso");
    }

    // Main Yuv
    if( isDataMsgEnabled(ECamShot_DATA_MSG_YUV|ECamShot_DATA_MSG_JPEG) )
    {
        MSize size        = jpegsize;
        MINT format       = yuvfmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = getRotation();
        sp<IImageStreamInfo>
            pStreamInfo = NSCam::v1::NSLegacyPipeline::createImageStreamInfo(
                    "MultiShot:Yuv",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    getCapBufCount(eSTREAMID_IMAGE_PIPE_YUV_JPEG, eMAX),
                    getCapBufCount(eSTREAMID_IMAGE_PIPE_YUV_JPEG, eMIN),
                    usage, format, size, transform, MSize(16,16)
                    );
        if( pStreamInfo == nullptr ) {
            return MFALSE;
        }
        mpInfo_Yuv = pStreamInfo;
    }
    // Thumbnail Yuv
    if( isDataMsgEnabled(ECamShot_DATA_MSG_JPEG) && thumbnailsize.w > 0 && thumbnailsize.h > 0 )
    {
        MSize size        = thumbnailsize;
        MINT format       = eImgFmt_YUY2;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "MultiShot:ThumbnailYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                    eSTREAMTYPE_IMAGE_INOUT,
                    getCapBufCount(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, eMAX),
                    getCapBufCount(eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, eMIN),
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return MFALSE;
        }
        mpInfo_YuvThumbnail = pStreamInfo;
    }
    // Jpeg
    if( isDataMsgEnabled(ECamShot_DATA_MSG_JPEG) )
    {
        MSize size        = jpegsize;
        MINT format       = eImgFmt_BLOB;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "MultiShot:Jpeg",
                    eSTREAMID_IMAGE_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    getCapBufCount(eSTREAMID_IMAGE_JPEG, eMAX),
                    getCapBufCount(eSTREAMID_IMAGE_JPEG, eMIN),
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return MFALSE;
        }
        mpInfo_Jpeg = pStreamInfo;
    }
    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiShot::
createCapturePipeline(StreamBufferProvider* pProvider)
{
    FUNC_START;
    if( pProvider == nullptr )
    {
        MY_LOGE("provider is NULL");
        return MFALSE;
    }

    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    LPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_MShotCapture;
    LPBConfigParams.enableEIS = MFALSE;
    LPBConfigParams.enableLCS = mbCapEnableLCS;
    //
    HwInfoHelper helper(mSensorParam.u4OpenID);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if (helper.getDualPDAFSupported(mSensorParam.u4Scenario))
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
            LPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else
        {
            LPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }
    //
    sp<LegacyPipelineBuilder> pBuilder =
        LegacyPipelineBuilder::createInstance(
                                    mSensorParam.u4OpenID,
                                    "MultiShot",
                                    LPBConfigParams);
    CHECK_OBJECT(pBuilder);

    mpDispatcher = new MShotDispatcher(this);
    pBuilder->setDispatcher(mpDispatcher);

    mpCallbackHandler = new BufferCallbackHandler(mSensorParam.u4OpenID);
    mpCallbackHandler->setImageCallback(this);

    sp<StreamBufferProviderFactory> pFactory =
                StreamBufferProviderFactory::createInstance();
    {
        Vector<PipelineImageParam> imgSrcParams;
        //IMGO
        pFactory->setImageStreamInfo(mpInfo_FullRaw);
        mpImgoPool = new VirtualBufferPool(this, mpInfo_FullRaw->getStreamId());
        pFactory->setUsersPool(mpImgoPool);
        PipelineImageParam imgParam = {
            .pInfo     = mpInfo_FullRaw,
            .pProvider = pFactory->create(),
            .usage     = 0
        };
        imgSrcParams.push_back(imgParam);
        //LCSO
        if( mbCapEnableLCS )
        {
            pFactory->setImageStreamInfo(mpInfo_LcsoRaw);
            mpLcsoPool = new VirtualBufferPool(this, mpInfo_LcsoRaw->getStreamId());
            pFactory->setUsersPool(mpLcsoPool);
            PipelineImageParam imgParam = {
                .pInfo     = mpInfo_LcsoRaw,
                .pProvider = pFactory->create(),
                .usage     = 0
            };
            imgSrcParams.push_back(imgParam);
        }
        //
        if( OK != pBuilder->setSrc(imgSrcParams) ) {
            MY_LOGE("setSrc failed");
            return MFALSE;
        }
    }
    {
        Vector<PipelineImageParam> vImageParam;
        //
        if( mpInfo_Yuv.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_Yuv;
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = nullptr,
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( mpInfo_YuvThumbnail.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_YuvThumbnail;
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = nullptr,
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( mpInfo_Jpeg.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_Jpeg;
            //
            /*mpCallbackHandler->setImageStreamInfo(eSTREAMID_IMAGE_JPEG, pStreamInfo,
                (mvRegBuf.indexOfKey(ECamShot_DATA_MSG_JPEG)>=0 ? mvRegBuf.editValueFor(ECamShot_DATA_MSG_JPEG) : NULL ));*/
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            if(mvRegBuf.indexOfKey(ECamShot_DATA_MSG_JPEG)>=0)
            {
                pPool->addBuffer(mvRegBuf.editValueFor(ECamShot_DATA_MSG_JPEG));
            }
            else
            {
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            mpCallbackHandler->setBufferPool(pPool);
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( OK != pBuilder->setDst(vImageParam) ) {
            MY_LOGE("setDst failed");
            return MFALSE;
        }
    }
    //
    mpCapturePipeline = pBuilder->create();
    CHECK_OBJECT(mpCapturePipeline);
    //
    {
        sp<ResultProcessor> pResultProcessor = mpCapturePipeline->getResultProcessor().promote();
        CHECK_OBJECT(pResultProcessor);
        pResultProcessor->registerListener( mRequestNumberMin, mRequestNumberMax, true, this);
    }
    //
    FUNC_END;
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MultiShot::
createPreviewPipeline()
{
    FUNC_START;
    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    LPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_NormalMShotPreview;
    LPBConfigParams.enableEIS = MFALSE;
    //decide LCSO on/off
    switch( mPrvPipeLCS )
    {
        case LCSO_FORCE_OFF:
            mbPrvEnableLCS = MFALSE;
            break;
        case LCSO_FORCE_ON:
            mbPrvEnableLCS = MTRUE;
            break;
        case LCSO_RUNTIME_DECIDE:
            mbPrvEnableLCS = mShotParam.bOpenLCS;
            break;
        case LCSO_GET_PROPERTY:
            {
                MINT32 lcsOpen = property_get_int32("debug.open.lcs", 0);
                mbPrvEnableLCS = (lcsOpen > 0);
            }
            break;
        default:
            MY_LOGW("un-support preview lcso option (%d)", mPrvPipeLCS);
            break;
    }
    LPBConfigParams.enableLCS = mbPrvEnableLCS;
    MY_LOGD("LCSO : mshot config(%d), param(%d), prv enable(%d), p1 config(%d)"
            , mPrvPipeLCS, mShotParam.bOpenLCS , mbPrvEnableLCS, LPBConfigParams.enableLCS);
    //
    HwInfoHelper helper(mSensorParam.u4OpenID);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if (helper.getDualPDAFSupported(mSensorParam.u4Scenario))
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
            LPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else
        {
            LPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }
    //
    sp<LegacyPipelineBuilder> pBuilder =
        LegacyPipelineBuilder::createInstance(
                                    mSensorParam.u4OpenID,
                                    "MShot",
                                    LPBConfigParams);
    if ( pBuilder == 0 ) {
        MY_LOGE("Cannot create LegacyPipelineBuilder.");
        return BAD_VALUE;
    }
    //
    mPreviewSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;

    if( ! helper.getSensorSize( mPreviewSensorParam.mode, mPreviewSensorParam.size) ||
        ! helper.getSensorFps( (MUINT32)mPreviewSensorParam.mode, (MINT32&)mPreviewSensorParam.fps) ||
        ! helper.queryPixelMode( mPreviewSensorParam.mode, mPreviewSensorParam.fps, mPreviewSensorParam.pixelMode)
      ) {
        MY_LOGE("cannot get params about sensor");
        return BAD_VALUE;
    }
    // Sensor
    MY_LOGD("sensor mode:%d size:%dx%d, fps:%d pixel:%d",
        mPreviewSensorParam.mode,
        mPreviewSensorParam.size.w, mPreviewSensorParam.size.h,
        mPreviewSensorParam.fps,
        mPreviewSensorParam.pixelMode);
    pBuilder->setSrc(mPreviewSensorParam);
    //
    sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(mSensorParam.u4OpenID);
    if( pScenarioCtrl.get() == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return BAD_VALUE;
    }
    IScenarioControl::ControlParam param;
    param.scenario = IScenarioControl::Scenario_ContinuousShot;
    param.sensorSize = mPreviewSensorParam.size;
    param.sensorFps  = mPreviewSensorParam.fps;
    if(LPBConfigParams.enableDualPD)
        FEATURE_CFG_ENABLE_MASK(param.featureFlag,IScenarioControl::FEATURE_DUAL_PD);

    pScenarioCtrl->enterScenario(param);
    pBuilder->setScenarioControl(pScenarioCtrl);

    // Image
    sp<IResourceContainer> spContainer = IResourceContainer::getInstance(mSensorParam.u4OpenID);
    if( spContainer.get() == NULL )
    {
        MY_LOGE("spContainer.get() == NULL");
        return UNKNOWN_ERROR;
    }

    sp<PairMetadata> pPair;
    sp<NormalCShotSelector> pSelector = new NormalCShotSelector();
    {
        Vector<PipelineImageParam> vImageParam;
        // RAW
        {
            MINT32 rawBitDepth;
            helper.getRecommendRawBitDepth(rawBitDepth);
            MUINT32 const bitDepth = (MUINT32)rawBitDepth;

            MSize const previewsize   = MSize(mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight);
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
            sp<IImageStreamInfo> pImage_Raw;

            if( helper.hasRrzo() )
            {
                if ( OK != decideRrzoImage(
                                helper, bitDepth,
                                previewsize, usage,
                                4, 6,
                                pImage_Raw,
                                MFALSE
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
            }

            if ( OK != decideImgoImage(
                            helper, bitDepth,
                            mPreviewSensorParam.size, usage,
                            4, 6,
                            pImage_Raw,
                            MTRUE
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
            pProducer->setSelector(pSelector);

            spContainer->setConsumer(pImage_Raw->getStreamId(), pProducer);

            vImageParam.push_back(
                PipelineImageParam{
                    pImage_Raw,
                    pProducer,
                    0
                }
            );

            // RAW (LCSO with provider)
            if (LPBConfigParams.enableLCS)
            {
                MUINT32 const bitDepth = 10; // no use
                MSize anySize; // no use
                MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
                sp<IImageStreamInfo> pImage_Lcso;

                if ( OK != decideLcsoImage(
                            helper, bitDepth,
                            anySize, usage,
                            4, 12,
                            pImage_Lcso
                        ))
                {
                    MY_LOGE("No lcso image");
                    return BAD_VALUE;
                }

                sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                pFactory->setImageStreamInfo(pImage_Lcso);
                pFactory->setPairRule(pPair, 2);
                sp<StreamBufferProvider> pProducer = pFactory->create();
                pProducer->setSelector(pSelector);

                spContainer->setConsumer(pImage_Lcso->getStreamId(),pProducer);

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
            MSize const& size = MSize(-1,-1);
            MINT const format = eImgFmt_YUY2;
            size_t const stride = 1280;
            MUINT const usage = 0;
            sp<IImageStreamInfo> pImage_Yuv =
                createImageStreamInfo(
                    "Hal:Image:yuvDisp",
                    eSTREAMID_IMAGE_PIPE_YUV_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    5, 1,
                    usage, format, size, 0
                );

            sp<IResourceContainer> spContainer = IResourceContainer::getInstance(mSensorParam.u4OpenID);
            if( spContainer.get() == NULL )
            {
                MY_LOGE("spContainer.get() == NULL");
                return UNKNOWN_ERROR;
            }
            sp<StreamBufferProvider> spProvider = spContainer->queryConsumer(eSTREAMID_IMAGE_PIPE_YUV_00);
            if( spProvider.get() == NULL )
            {
                MY_LOGE("spProvider.get() == NULL");
                return UNKNOWN_ERROR;
            }
            spProvider->setImageStreamInfo(pImage_Yuv);

            vImageParam.push_back(
                PipelineImageParam{
                    pImage_Yuv,
                    spProvider,
                    0
                }
            );
        }

        pBuilder->setDst(vImageParam);
    }

    mpPreviewPipeline = pBuilder->create();

    if ( mpPreviewPipeline == 0) {
        MY_LOGE("Fail to create preview pipeline.");
        return BAD_VALUE;
    }

    sp<StreamBufferProvider> pTempConsumer =
        spContainer->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );

    if ( pTempConsumer != 0 ) {
        sp<ResultProcessor> pProcessor = mpPreviewPipeline->getResultProcessor().promote();
        pProcessor->registerListener(
                        eSTREAMID_META_APP_DYNAMIC_P1,
                        pPair
                        );
        pProcessor->registerListener(
                        eSTREAMID_META_HAL_DYNAMIC_P1,
                        pPair
                        );
    }

    FUNC_END;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
MultiShot::
decideRrzoImage(
    HwInfoHelper& helper,
    MUINT32 const bitDepth,
    MSize referenceSize,
    MUINT const usage,
    MINT32 const minBuffer,
    MINT32 const maxBuffer,
    sp<IImageStreamInfo>& rpImageStreamInfo,
    MBOOL useUFO
)
{
    MSize autualSize;
    size_t stride;
    MINT format;
    MBOOL useUFOfmt = MTRUE;
    if (getShotMode() == eShotMode_EngShot || useUFO == MFALSE)
    {
        useUFOfmt = MFALSE;
    }
    if( ! helper.getRrzoFmt(bitDepth, format, useUFOfmt) ||
        ! helper.alignRrzoHwLimitation(referenceSize, mPreviewSensorParam.size, autualSize) ||
        ! helper.alignPass1HwLimitation(mPreviewSensorParam.pixelMode, format, false, autualSize, stride) )
    {
        MY_LOGE("wrong params about rrzo");
        return BAD_VALUE;
    }
    //
    MY_LOGI("rrzo bitDepth:%d format:%d referenceSize:%dx%d, actual size:%dx%d, stride:%d",
                bitDepth, format,
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
MultiShot::
decideImgoImage(
    HwInfoHelper& helper,
    MUINT32 const bitDepth,
    MSize referenceSize,
    MUINT const usage,
    MINT32 const minBuffer,
    MINT32 const maxBuffer,
    sp<IImageStreamInfo>& rpImageStreamInfo,
    MBOOL useUFO
)
{
    MSize autualSize = referenceSize;
    size_t stride;
    MINT format;
    MBOOL useUFOfmt = MTRUE;
    if (getShotMode() == eShotMode_EngShot || useUFO == MFALSE)
    {
        useUFOfmt = MFALSE;
    }
    if( ! helper.getImgoFmt(bitDepth, format, useUFOfmt) ||
        ! helper.alignPass1HwLimitation(mPreviewSensorParam.pixelMode, format, true, autualSize, stride) )
    {
        MY_LOGE("wrong params about imgo");
        return BAD_VALUE;
    }
    //
    MY_LOGI("imgo bitDepth:%d format:%d referenceSize:%dx%d, actual size:%dx%d, stride:%d",
                bitDepth, format,
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
MultiShot::
decideLcsoImage(
    HwInfoHelper& /*helper*/,
    MUINT32 const bitDepth,
    MSize         referenceSize,
    MUINT const   usage,
    MINT32 const  minBuffer,
    MINT32 const  maxBuffer,
    sp<IImageStreamInfo>& rpImageStreamInfo
)
{
    NS3Av3::LCSO_Param lcsoParam;
    if ( auto pIspMgr = MAKE_IspMgr() ) {
        pIspMgr->queryLCSOParams(lcsoParam);
    }
    //
    MY_LOGI("lcso num:%d-%d bitDepth:%d format:%d referenceSize:%dx%d, actual size:%dx%d, stride:%d",
                minBuffer,
                maxBuffer,
                bitDepth,
                lcsoParam.format,
                referenceSize.w, referenceSize.h,
                lcsoParam.size.w, lcsoParam.size.h,
                lcsoParam.stride
            );
    rpImageStreamInfo =
        createRawImageStreamInfo(
            "Hal:Image:LCSraw",
            eSTREAMID_IMAGE_PIPE_RAW_LCSO,
            eSTREAMTYPE_IMAGE_INOUT,
            maxBuffer, minBuffer,
            usage, lcsoParam.format, lcsoParam.size, lcsoParam.stride
            );

    return OK;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
MultiShot::
updateFinishDataMsg(MUINT32 datamsg)
{
    MY_LOGV("update set(0x%x): finished(0x%x), data(0x%x)",
            mi4DataMsgSet, mFinishedData, datamsg);
    mFinishedData |= datamsg;
    //check if all valid data is processed
    if( mFinishedData == (mi4DataMsgSet & ECamShot_DATA_MSG_ALL) )
    {
        MY_LOGD("shot count %d done, datamsg(0x%x), mFinishedData(0x%x)", mCurCount, datamsg, mFinishedData);
        mFinishedData = 0x0;
        mCurCount++;

        if( (mCurCount == mShotCount) || mbToStop )
        {
            mbDoShutterCb = MFALSE;

            handleNotifyCallback(ECamShot_NOTIFY_MSG_SHOTS_END, 0, 0);

            MY_LOGI("shot done, on stop");
            mbOnStop = MTRUE;
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID
MultiShot::
onResultReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const errorResult,
    IMetadata       const /*result*/
)
{
    Mutex::Autolock _l(mLock);
    MY_LOGV("requestNo %d, error %d, stream %#" PRIx64, requestNo, errorResult, streamId);
    //
    if( mbDoShutterCb )
    {
        mbDoShutterCb = MFALSE;
        handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0);
    }
    return;
}

/*******************************************************************************
*
********************************************************************************/
MERROR
MultiShot::
onResultReceived(
    MUINT32     const           RequestNo,
    StreamId_T  const           streamId,
    MBOOL       const           errorBuffer,
    android::sp<IImageBuffer>&  pBuffer
)
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("requestNo %d, streamId %#" PRIx64 ", buffer %p, error %d", RequestNo, streamId, pBuffer.get(), errorBuffer);

    if( mbOnStop )
    {
        return OK;
    }

    if( errorBuffer )
    {
        MY_LOGW("error buffer, skip callback");
        return OK;
    }

    MUINT32 dataMsg = ECamShot_DATA_MSG_NONE;
    switch( streamId )
    {
        case eSTREAMID_IMAGE_JPEG:
            dataMsg = ECamShot_DATA_MSG_JPEG;
            break;
        default:
            MY_LOGW("streamId not supported yet: 0x%x", streamId);
            return UNKNOWN_ERROR;
    }

    updateFinishDataMsg(dataMsg);

    if( pBuffer != 0 )
    {
        // debug
        if( mDumpFlag & dataMsg )
        {
            String8 filename = String8::format("%s/singleshot_%dx%d",
                    DUMP_PATH, pBuffer->getImgSize().w, pBuffer->getImgSize().h);

            if(dataMsg == ECamShot_DATA_MSG_JPEG)
                filename += String8(".jpeg");

            pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
            pBuffer->saveToFile(filename);
            pBuffer->unlockBuf(LOG_TAG);
        }
        handleDataCallback(dataMsg, 0, 0, pBuffer.get());
    }
    if( mbOnStop )
    {
        MY_LOGD("mbOnStop is true, send mNextFrameCond.signal()");
        mNextFrameCond.signal();
    }
    return OK;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
MultiShot::
onFrameNotify(
    MINT32 const /*frameNo*/,
    MINT32 const nodeId
)
{
    if( nodeId == eNODEID_P2Node )
    {
        Mutex::Autolock _l(mLock);
        mCurSubmitCount--;
        MY_LOGD("p2 notify, send mNextFrameCond.signal(), CurSubmitCount %d", mCurSubmitCount);
        mNextFrameCond.signal();
    }
    return;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
MultiShot::
onReturnBufferHeap(
    StreamId_T                          id,
    android::sp<IImageBufferHeap>       spBufferHeap
)
{
    if( mpSelector != NULL )
    {
        ISelector::BufferItemSet set{id, spBufferHeap, NULL};
        mpSelector->returnBuffer(set);
    }
    else
    {
        MY_LOGE("mpSelector already release, cannot return buffer heap");
    }
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

