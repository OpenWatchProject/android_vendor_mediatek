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

#define LOG_TAG "mtkcam-InFlightRequest"
//
#include "InFlightRequest.h"
#include "MyUtils.h"
//
#include <thread>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define MY_LOGD1(...)               MY_LOGD_IF(1<=mLogLevel, __VA_ARGS__)

/******************************************************************************
 *
 ******************************************************************************/
InFlightRequest::
InFlightRequest()
    : IPipelineFrameListener()
{
    mLogLevel = property_get_int32("vendor.debug.camera.log", 0);
    if ( 0 == mLogLevel ) {
        mLogLevel = property_get_int32("vendor.debug.camera.log.inflightrequest", 0);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
freeIPipelineFrameHolder(
    std::vector<android::sp<IPipelineFrame>>&& holder
) -> void
{
    MY_LOGD("#holder=%zu", holder.size());
    holder.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
dumpState(android::Printer& printer, const std::vector<std::string>& options) const
{
    printer.printLine("*In-flight frames*");
    android::PrefixPrinter prefixPrinter(printer, " ");

    if (mLock.timedLock(100000000 /* 100ms */) == OK) {
        std::vector<sp<IPipelineFrame>> holder;
        for (auto const& v : mRequest) {
            auto pFrame = v.second.promote();
            if ( pFrame != nullptr ) {
                pFrame->dumpState(prefixPrinter, options);
                holder.push_back(pFrame);
            }
        }

        mLock.unlock();

        // Since IPipelineFrame::onLastStrongRef will perform callbacks,
        // it's better to call IPipelineFrame's decStrong in another thread
        // to avoid dead lock.
        if ( ! holder.empty() ) {
            std::thread t(freeIPipelineFrameHolder, std::move(holder));
            t.detach();
        }

    }
    else {
        prefixPrinter.printLine("timedLock");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
registerRequest(
    sp<IPipelineFrame> const&pFrame
)
{
    MY_LOGD1("+");
    Mutex::Autolock _l(mLock);
    //add request
    mRequest.push_back(std::make_pair(pFrame->getFrameNo(), pFrame));

    //add node
    Vector<IPipelineDAG::NodeObj_T>const& vNode = pFrame->getPipelineDAG().getToposort();
    for(size_t s = 0; s < vNode.size(); s++) {
        NodeId_T node = vNode[s].id;

        sp<IStreamInfoSet const> in, out;
        if(CC_UNLIKELY( OK != pFrame->queryIOStreamInfoSet(node, in, out))) {
            MY_LOGE("queryIOStreamInfoSet failed");
            break;
        }
        //
        if( out->getImageInfoNum() > 0 || out->getMetaInfoNum() > 0) {
            if (mRequestMap_image.indexOfKey(node) < 0) {
                RequestList frameL;
                frameL.push_back(pFrame->getFrameNo());
                mRequestMap_image.add(node, frameL);
            } else {
                mRequestMap_image.editValueFor(node).push_back(pFrame->getFrameNo());
            }

            if (mRequestMap_meta.indexOfKey(node) < 0) {
                RequestList frameL;
                frameL.push_back(pFrame->getFrameNo());
                mRequestMap_meta.add(node, frameL);
            } else {
                mRequestMap_meta.editValueFor(node).push_back(pFrame->getFrameNo());
            }
        }

    }
    // register
    pFrame->attachListener(this, NULL);
    MY_LOGD1("-");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
waitUntilDrained()
{
    CAM_TRACE_CALL();

    MY_LOGI_IF(1, "+");
    Mutex::Autolock _l(mLock);
    while(!mRequest.empty())
    {
        MY_LOGI_IF(1, "frameNo:%u in the front", (*(mRequest.begin())).first);
        mRequestCond.wait(mLock);
    }
    MY_LOGI_IF(1, "-");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
waitUntilNodeDrained(NodeId_T id)
{
    CAM_TRACE_CALL();

    waitUntilNodeMetaDrained(id);
    waitUntilNodeImageDrained(id);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
waitUntilNodeMetaDrained(NodeId_T id)
{
    Mutex::Autolock _l(mLock);
    do {
        ssize_t index = mRequestMap_meta.indexOfKey(id);
        if ( index < 0 || mRequestMap_meta.valueAt(index).empty() ) {
            break;
        }

        MY_LOGD_IF(1, "Node: %" PRIdPTR " has frameNo: %d in the front of meta list",
            id, *(mRequestMap_meta.valueAt(index).begin()));
        mRequestCond.wait(mLock);

    } while (1);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
waitUntilNodeImageDrained(NodeId_T id)
{
    Mutex::Autolock _l(mLock);
    do {
        ssize_t index = mRequestMap_image.indexOfKey(id);
        if ( index < 0 || mRequestMap_image.valueAt(index).empty() ) {
            break;
        }

        MY_LOGD_IF(1, "Node: %" PRIdPTR " has frameNo: %d in the front of image list",
           id, *(mRequestMap_image.valueAt(index).begin()));
        mRequestCond.wait(mLock);

    } while (1);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
abort()
{
    MY_LOGI("+");

    std::list<android::sp<IPipelineFrame>> abortFrames;
    std::string abortLog;
    size_t inFlightCount = 0;

    {
        android::Mutex::Autolock _l(mLock);

        // a reverse order
        for (auto it = mRequest.crbegin(); it != mRequest.crend(); it++)
        {
            inFlightCount++;

            auto pFrame = (*it).second.promote();
            if ( pFrame != nullptr ) {
                auto pPipelineBufferSetFrameControl = IPipelineBufferSetFrameControl::castFrom(pFrame.get());
                if ( pPipelineBufferSetFrameControl != nullptr ) {

                    pPipelineBufferSetFrameControl->abort();

                    abortFrames.push_front(pFrame);
                    abortLog += std::to_string(pFrame->getFrameNo()) + " ";
                }
            }
        }
    }

    abortFrames.clear();//free sp outside locking

    MY_LOGI("- frames:{ %s} being aborted (#%zu in-flight frames)", abortLog.c_str(), inFlightCount);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
onPipelineFrame(
    MUINT32 const frameNo,
    MUINT32 const message,
    MVOID*const /*pCookie*/
)
{
    MY_LOGD1("frameNo:%d message:%d", frameNo, message);

    if(message == eMSG_FRAME_RELEASED){
        bool hit = false;
        Mutex::Autolock _l(mLock);
        for(FrameListT::iterator it = mRequest.begin(); it != mRequest.end(); it++) {
            if((*it).first == frameNo) {
                hit = true;
                mRequest.erase(it);
                mRequestCond.broadcast();
                break;
            }
        }
        if (CC_UNLIKELY( ! hit )) {
            std::string list;
            for(FrameListT::iterator it = mRequest.begin(); it != mRequest.end(); it++) {
                list += std::to_string((*it).first);
                list += " ";
            }
            MY_LOGW("frameNo:%d is not in { %s }", frameNo, list.c_str());
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
onPipelineFrame(
    MUINT32 const frameNo,
    NodeId_T const nodeId,
    MUINT32 const message,
    MVOID*const /*pCookie*/
)
{
    MY_LOGD1("frameNo:%d nodeId:%#" PRIxPTR " message:%d", frameNo, nodeId, message);

    Mutex::Autolock _l(mLock);

    switch (message)
    {
    case eMSG_ALL_OUT_META_BUFFERS_RELEASED:{
        ssize_t index_meta  = mRequestMap_meta.indexOfKey(nodeId);
        if (CC_UNLIKELY( index_meta < 0 )) {
            MY_LOGE("eMSG_ALL_OUT_META_BUFFERS_RELEASED: "
                    "no nodeId:%#" PRIxPTR " in meta mapper", nodeId);
            break;
        }

        RequestList& list = mRequestMap_meta.editValueAt(index_meta);
        for(RequestList::iterator it = list.begin(); it != list.end(); it++) {
            if(*it == frameNo) {
                list.erase(it);
                break;
            }
        }
        mRequestCond.broadcast();
        }break;

    case eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED:{
        ssize_t index_image = mRequestMap_image.indexOfKey(nodeId);
        if (CC_UNLIKELY( index_image < 0 )) {
            MY_LOGE("eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED: "
                    "no nodeId:%#" PRIxPTR " in image mapper", nodeId);
            break;
        }

        RequestList& list = mRequestMap_image.editValueAt(index_image);
        for(RequestList::iterator it = list.begin(); it != list.end(); it++) {
            if(*it == frameNo) {
                list.erase(it);
                break;
            }
        }
        mRequestCond.broadcast();
        }break;

    default:
        break;
    }

}

