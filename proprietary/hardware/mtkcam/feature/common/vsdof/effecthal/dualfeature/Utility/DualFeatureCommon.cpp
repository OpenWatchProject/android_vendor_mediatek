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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define LOG_TAG "MtkCam/DualFeatureCommon"
//
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
#include <utils/String8.h>
// for metadata
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/utils/std/Misc.h>
//
#include <mtkcam/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam/feature/DualCam/DualFeatureCommon.h>
//
#include <list>
//
#include <chrono>
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::DualFeature;

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
/******************************************************************************
 *
 ******************************************************************************/
process_frame::
process_frame(
    sp<IPipelineFrame> const& pFrame,
    MINT32 openId,
    MUINT32 iRequestId,
    MUINT32 iNodeId,
    MINT32 logLevel
) :
    mpFrame(pFrame),
    mOpenId(openId),
    mRequestId(iRequestId),
    mNodeId(iNodeId),
    mLogLevel(logLevel)
{
    start = std::chrono::system_clock::now();
}
/******************************************************************************
 *
 ******************************************************************************/
process_frame::
~process_frame()
{
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
process_frame::
unlockAllStream(
    const char* nodeName
)
{
    // unlock in metadata
    {
        if(mvInMetaBufferContainer.size() !=
           mvInMetaStreamBuffer.size())
        {
            MY_LOGE("mvInMetaBufferContainer.size() != mvInMetaStreamBuffer.size()");
            return;
        }
        IMetadata* metaBuffer = nullptr;
        sp<IMetaStreamBuffer> metaStreamBuffer = nullptr;
        for(ssize_t i=0;i<mvInMetaBufferContainer.size();++i)
        {
            metaBuffer = mvInMetaBufferContainer.editValueAt(i);
            metaStreamBuffer = mvInMetaStreamBuffer.editValueAt(i);
            if(metaBuffer != nullptr && metaStreamBuffer != nullptr)
            {
                metaStreamBuffer->unlock(nodeName, metaBuffer);
            }
        }
    }
    // unlock out metadata
    {
        if(mvOutMetaBufferContainer.size() !=
           mvOutMetaStreamBuffer.size())
        {
            MY_LOGE("mvOutMetaBufferContainer.size() != mvOutMetaStreamBuffer.size()");
            return;
        }
        IMetadata* metaBuffer = nullptr;
        sp<IMetaStreamBuffer> metaStreamBuffer = nullptr;
        for(ssize_t i=0;i<mvOutMetaBufferContainer.size();++i)
        {
            metaBuffer = mvOutMetaBufferContainer.editValueAt(i);
            metaStreamBuffer = mvOutMetaStreamBuffer.editValueAt(i);
            if(metaBuffer != nullptr && metaStreamBuffer != nullptr)
            {
                metaStreamBuffer->unlock(nodeName, metaBuffer);
            }
        }
    }
    // unlock in image stream
    {
        if(mvInImgBufContainer.size() !=
           mvInImgStreamBuffer.size())
        {
            MY_LOGE("mvInImgBufContainer.size() != mvInImgStreamBuffer.size()");
            return;
        }
        sp<IImageBuffer> imgBuffer;
        sp<IImageStreamBuffer> imgStreamBuffer;
        for(ssize_t i=0; i<mvInImgBufContainer.size();++i)
        {
            imgBuffer = mvInImgBufContainer.editValueAt(i);
            imgStreamBuffer = mvInImgStreamBuffer.editValueAt(i);
            if(imgBuffer != nullptr && imgStreamBuffer != nullptr)
            {
                imgBuffer->unlockBuf(nodeName);
                imgStreamBuffer->unlock(nodeName, imgBuffer->getImageBufferHeap());
            }
        }
    }
    // unlock out image stream
    {
        if(mvOutImgBufContainer.size() !=
           mvOutImgStreamBuffer.size())
        {
            MY_LOGE("mvOutImgBufContainer.size() != mvOutImgStreamBuffer.size()");
            return;
        }
        sp<IImageBuffer> imgBuffer;
        sp<IImageStreamBuffer> imgStreamBuffer;
        for(ssize_t i=0; i<mvOutImgBufContainer.size();++i)
        {
            imgBuffer = mvOutImgBufContainer.editValueAt(i);
            imgStreamBuffer = mvOutImgStreamBuffer.editValueAt(i);
            if(imgBuffer != nullptr && imgStreamBuffer != nullptr)
            {
                imgBuffer->unlockBuf(nodeName);
                imgStreamBuffer->unlock(nodeName, imgBuffer->getImageBufferHeap());
            }
        }
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
process_frame::
markStreamStatus(
    MBOOL vaild
)
{
    MUINT32 mask = 0;
    if(vaild)
    {
        mask = STREAM_BUFFER_STATUS::WRITE_OK;
    }
    else
    {
        mask = STREAM_BUFFER_STATUS::WRITE_ERROR;
    }
    IStreamBufferSet& rStreamBufferSet = mpFrame->getStreamBufferSet();
    // mark stream status
#define MARK_STREAM_STATUS(STREAM_BUFFER, NEED_MARK_STREAM)\
do{\
    for(ssize_t i=0;i<STREAM_BUFFER.size();++i)\
    {\
        if(NEED_MARK_STREAM)\
        {\
            STREAM_BUFFER.valueAt(i)->markStatus(mask);\
        }\
        rStreamBufferSet.markUserStatus(\
                        STREAM_BUFFER.keyAt(i),\
                        mNodeId,\
                        IUsersManager::UserStatus::USED |\
                        IUsersManager::UserStatus::RELEASE);\
    }\
}while(0);
    // metadata
    MARK_STREAM_STATUS(mvInMetaStreamBuffer, MFALSE);
    MARK_STREAM_STATUS(mvOutMetaStreamBuffer, MTRUE);
    // image
    MARK_STREAM_STATUS(mvInImgStreamBuffer, MFALSE);
    MARK_STREAM_STATUS(mvOutImgStreamBuffer, MTRUE);
#undef MARK_STREAM_STATUS
    rStreamBufferSet.applyRelease(mNodeId);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
process_frame::
uninit(
    MBOOL vaild,
    const char* nodeName)
{
    // unlock all stream, include metadata and image stream
    unlockAllStream(nodeName);
    // mark stream status
    markStreamStatus(vaild);
    // relase stream
    mvInMetaBufferContainer.clear();
    mvInMetaStreamBuffer.clear();
    mvOutMetaBufferContainer.clear();
    mvOutMetaStreamBuffer.clear();
    mvInImgBufContainer.clear();
    mvInImgStreamBuffer.clear();
    mvOutImgBufContainer.clear();
    mvOutImgStreamBuffer.clear();
    if(mLogLevel > 0)
    {
        auto toString = [&vaild]()
        {
            if(vaild)
                return "SUCCESS";
            else
                return "FAIL";
        };
        MINT32 reqID = mpFrame->getRequestNo();
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        MY_LOGW("[%d] VSDOF_Profile: processing time(%lf ms) reqID=%d status(%s)",
                                    mOpenId,
                                    elapsed_seconds.count() *1000,
                                    reqID,
                                    toString());
    }
    mpFrame = nullptr;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
process_frame::
dumpInfo()
{
    if((mvInMetaBufferContainer.size() != mvInMetaStreamBuffer.size()) ||
       (mvOutMetaBufferContainer.size() != mvOutMetaStreamBuffer.size()) ||
       (mvInImgBufContainer.size() != mvInImgStreamBuffer.size()) ||
       (mvOutImgBufContainer.size() != mvOutImgStreamBuffer.size()))
    {
        MY_LOGE("Eorror! IM[%d] IMS[%d] OM[%d] OMS[%d] II[%d] IIS[%d] OI[%d] OIS[%d]",
                                            mvInMetaBufferContainer.size(),
                                            mvInMetaStreamBuffer.size(),
                                            mvOutMetaBufferContainer.size(),
                                            mvOutMetaStreamBuffer.size(),
                                            mvInImgBufContainer.size(),
                                            mvInImgStreamBuffer.size(),
                                            mvOutImgBufContainer.size(),
                                            mvOutImgStreamBuffer.size());
        return;
    }
    String8 value = String8("");
    value += String8::format("O[%d] R[%d] F[%d]\n", mOpenId, mpFrame->getRequestNo(), mpFrame->getFrameNo());
    value += String8::format("===== metadata =====\n");
    for(size_t i=0;i<mvInMetaBufferContainer.size();++i)
    {
        value += String8::format("I[%#" PRIx64 "] P[%x] PS[%x]\n",
                                    mvInMetaBufferContainer.keyAt(i),
                                    mvInMetaBufferContainer.valueAt(i),
                                    mvInMetaStreamBuffer.valueAt(i).get());
    }
    for(size_t i=0;i<mvOutMetaBufferContainer.size();++i)
    {
        value += String8::format("I[%#" PRIx64 "] P[%x] PS[%x]\n",
                                    mvOutMetaBufferContainer.keyAt(i),
                                    mvOutMetaBufferContainer.valueAt(i),
                                    mvOutMetaStreamBuffer.valueAt(i).get());
    }
    value += String8::format("===== image =====\n");
    for(size_t i=0;i<mvInImgBufContainer.size();++i)
    {
        value += String8::format("I[%#" PRIx64 "] P[%x] PS[%x]\n",
                                    mvInImgBufContainer.keyAt(i),
                                    mvInImgBufContainer.valueAt(i).get(),
                                    mvInImgStreamBuffer.valueAt(i).get());
    }
    for(size_t i=0;i<mvOutImgBufContainer.size();++i)
    {
        value += String8::format("I[%#" PRIx64 "] P[%x] PS[%x]\n",
                                    mvOutImgBufContainer.keyAt(i),
                                    mvOutImgBufContainer.valueAt(i).get(),
                                    mvOutImgStreamBuffer.valueAt(i).get());
    }
    MY_LOGI("%s", value.string());
}
/******************************************************************************
 *
 ******************************************************************************/
android::sp<NSCam::v3::IPipelineFrame>
process_frame::
getFrame()
{
    return mpFrame;
}