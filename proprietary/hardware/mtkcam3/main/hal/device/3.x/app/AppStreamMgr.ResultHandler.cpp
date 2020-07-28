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

#include "AppStreamMgr.h"
#include "MyUtils.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;

#define ThisNamespace   AppStreamMgr::ResultHandler

/******************************************************************************
 *
 ******************************************************************************/
CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_DEVICE);
#define MY_DEBUG(level, fmt, arg...) \
    do { \
        CAM_ULOGM##level("[%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
        mCommonInfo->mDebugPrinter->printFormatLine(#level" [%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_WARN(level, fmt, arg...) \
    do { \
        CAM_ULOGM##level("[%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
        mCommonInfo->mWarningPrinter->printFormatLine(#level" [%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_ERROR_ULOG(level, fmt, arg...) \
    do { \
        CAM_ULOGM##level("[%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
        mCommonInfo->mErrorPrinter->printFormatLine(#level" [%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_ERROR(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
        mCommonInfo->mErrorPrinter->printFormatLine(#level" [%s::%s] " fmt, mInstanceName.c_str(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_LOGV(...)                MY_DEBUG(V, __VA_ARGS__)
#define MY_LOGD(...)                MY_DEBUG(D, __VA_ARGS__)
#define MY_LOGI(...)                MY_DEBUG(I, __VA_ARGS__)
#define MY_LOGW(...)                MY_WARN (W, __VA_ARGS__)
#define MY_LOGE(...)                MY_ERROR_ULOG(E, __VA_ARGS__)
#define MY_LOGA(...)                MY_ERROR(A, __VA_ARGS__)
#define MY_LOGF(...)                MY_ERROR(F, __VA_ARGS__)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
ResultHandler(std::shared_ptr<CommonInfo> pCommonInfo, android::sp<FrameHandler> pFrameHandler)
    : mInstanceName{std::to_string(pCommonInfo->mInstanceId) + "-ResultHandler"}
    , mCommonInfo(pCommonInfo)
    , mFrameHandler(pFrameHandler)
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
destroy() -> void
{
    mCommonInfo->mDebugPrinter->printLine("[destroy] join +");
    this->requestExit();
    this->join();
    mCommonInfo->mDebugPrinter->printLine("[destroy] join -");

    mFrameHandler = nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
auto
ThisNamespace::
readyToRun() -> status_t
{
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
requestExit() -> void
{
    MY_LOGD("+ %s", mInstanceName.c_str());
    //
    {
        Mutex::Autolock _l(mResultQueueLock);
        Thread::requestExit();
        mResultQueueCond.broadcast();
    }
    //
    MY_LOGD("- %s", mInstanceName.c_str());
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
threadLoop() -> bool
{
    ResultQueueT vResult;
    int err = dequeResult(vResult);
    if  ( OK == err && ! vResult.isEmpty() )
    {
        mFrameHandler->update(vResult);
    }
    //
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dequeResult(ResultQueueT& rvResult) -> int
{
    int err = OK;
    //
    Mutex::Autolock _l(mResultQueueLock);
    //
    while ( ! exitPending() && mResultQueue.isEmpty() )
    {
        err = mResultQueueCond.wait(mResultQueueLock);
        MY_LOGW_IF(
            OK != err,
            "exitPending:%d ResultQueue#:%zu err:%d(%s)",
            exitPending(), mResultQueue.size(), err, ::strerror(-err)
        );
    }
    //
    if  ( mResultQueue.isEmpty() )
    {
        MY_LOGD_IF(getLogLevel() >= 1, "empty queue");
        rvResult.clear();
        err = NOT_ENOUGH_DATA;
    }
    else
    {
        //  If the queue is not empty, deque all items from the queue.
        rvResult = mResultQueue;
        mResultQueue.clear();
        err = OK;
    }
    //
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
enqueResult(UpdateResultParams const& params) -> int
{
    auto const frameNo = params.frameNo;
    auto const userId = params.userId;
    auto const hasLastPartial = params.hasLastPartial;

    NSCam::Utils::CamProfile profile(__FUNCTION__, "AppStreamMgr::ResultHandler");
    Mutex::Autolock _l(mResultQueueLock);
    //
    if  ( exitPending() ) {
        MY_LOGW("Dead ResultQueue");
        return DEAD_OBJECT;
    }
    //
    ssize_t const index = mResultQueue.indexOfKey(frameNo);
    profile.print_overtime(1, "frameNo:%u userId:%#" PRIxPTR " ResultQueue#:%zu", frameNo, userId, mResultQueue.size());
    //
    if  ( 0 <= index ) {
        sp<ResultItem> pItem = mResultQueue.editValueAt(index);

        MY_LOGW(
            "frameNo:%u existed in mResultQueue[%zd] - hasLastPartial:%d->%d",
            frameNo, index, pItem->lastPartial, hasLastPartial
        );

        pItem->lastPartial = pItem->lastPartial || hasLastPartial;
        if( ! params.resultMeta.isEmpty() ){
            pItem->buffer.appendVector(params.resultMeta);
        }
        pItem->isRealTimePartial = pItem->isRealTimePartial || params.hasRealTimePartial;
    }
    else {
        sp<ResultItem> pItem = new ResultItem;
        pItem->frameNo     = frameNo;
        pItem->lastPartial = hasLastPartial;
        pItem->isRealTimePartial  = params.hasRealTimePartial;
        pItem->buffer      = params.resultMeta;
        if ( params.timestampStartOfFrame != 0 ){
            pItem->timestampStartOfFrame = params.timestampStartOfFrame;
        }
        //
        mResultQueue.add(frameNo, pItem);
    }
    //
    //Update the physical results
    if ( params.physicalResult.size() != 0){
        sp<ResultItem> pItem = mResultQueue.valueFor(frameNo);
        for ( size_t i = 0; i < params.physicalResult.size(); ++i ) {
            android::sp<PhysicalResult> physicRes = params.physicalResult[i];
            if ( physicRes != nullptr ){
                ssize_t const indexOfCamId = pItem->physicBuffer.indexOfKey(physicRes->physicalCameraId);
                if  ( 0 <= indexOfCamId ) {
                    //there's existing physical metadatas
                    android::Vector<android::sp<IMetaStreamBuffer>>& physicBuf = pItem->physicBuffer.editValueFor(physicRes->physicalCameraId);
                    physicBuf.add(physicRes->physicalResultMeta);
                }
                else{
                    android::Vector<android::sp<IMetaStreamBuffer>> physicBuf;
                    physicBuf.add(physicRes->physicalResultMeta);
                    pItem->physicBuffer.add(physicRes->physicalCameraId, physicBuf);
                }
            }
        }
    }
    //
    mResultQueueCond.broadcast();
    //
    profile.print_overtime(1, "- frameNo:%u userId:%#" PRIxPTR, frameNo, userId);
    return OK;
}

