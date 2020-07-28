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

#define LOG_TAG "MtkCam/ClientBufPo"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/fwk/MtkCameraParameters.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/IGraphicImageBufferHeap.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/middleware/v1/camutils/IBuffer.h>
#include <mtkcam/middleware/v1/camutils/IImgBufQueue.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
#include <mtkcam/middleware/v1/IShot.h>

#include "ClientBufferPool.h"

using namespace android;
using namespace android::MtkCamUtils;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::Utils;


using namespace NSCam::v3::Utils;
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
/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* const pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        CAM_LOGE("pMetadata == NULL");
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
#define GET_IIMGBUF_IMG_STRIDE_IN_BYTE(pBuf, plane) \
        (Format::queryPlaneCount(MtkCameraParameters::queryImageFormat(pBuf->getImgFormat().string())) >= (plane+1)) ? \
        ((pBuf->getImgWidthStride(plane)*Format::queryPlaneBitsPerPixel(MtkCameraParameters::queryImageFormat(pBuf->getImgFormat().string()),plane)))>>3 : 0

#define GET_IIMGBUF_IMG_STRIDE_IN_BYTE_FOR_DISPLAY_ROTATION(pBuf, plane)  \
    (Format::queryPlaneCount(MtkCameraParameters::queryImageFormat(pBuf->getImgFormat().string())) >= (plane+1)) ? \
    ((pBuf->getImgWidthStride(plane+3)*Format::queryPlaneBitsPerPixel(MtkCameraParameters::queryImageFormat(pBuf->getImgFormat().string()),plane)))>>3 : 0

/******************************************************************************
 *
 ******************************************************************************/
ClientBufferPool::
ClientBufferPool(
    MINT32  openId,
    MBOOL   bUseTransform)
    : mOpenId(openId)
    , mbUseTransform(bUseTransform)
    , mLastTimestamp(0)
    , mbEnableBuffer(MTRUE)
    , mbDumpNotReturnBuffer(MTRUE)
{
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.camera.log.clientBP", 0);
    }
    //
    mpRecordBufferSnapShot = RecordBufferSnapShot::createInstance(getOpenId());
    mspResourceContainer = IResourceContainer::getInstance(getOpenId());
}


/******************************************************************************
 *
 ******************************************************************************/
ClientBufferPool::
~ClientBufferPool()
{
    if(mpRecordBufferSnapShot != NULL)
    {
        mpRecordBufferSnapShot->destroyInstance();
        mpRecordBufferSnapShot = NULL;
    }
    mspResourceContainer = NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ClientBufferPool::
acquireFromPool(
    char const*           szCallerName,
    MINT32                rRequestNo,
    sp<IImageBufferHeap>& rpBuffer,
    MUINT32&              rTransform
)
{
    MY_LOGD2("%s", szCallerName);

    Mutex::Autolock _l(mLock);

    if(!mbEnableBuffer)
    {
        return NO_MEMORY;
    }

    return getBufferFromClient_l(
            szCallerName,
            rRequestNo,
            rpBuffer,
            rTransform
            );

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ClientBufferPool::
tryGetClient()
{
    //FUNC_START;
    MERROR ret = UNKNOWN_ERROR;
    for ( size_t i = 0; i < mvClientMap.size(); ++i ) {
        switch ( mvClientMap.keyAt(i) )
        {
            case IImgBufProvider::eID_DISPLAY: {
                MY_LOGD2("Try get display client");
                mvClientMap.replaceValueFor(
                            IImgBufProvider::eID_DISPLAY,
                            mpSourceManager->getDisplayPvdr()
                            );
                break;
            }
            case IImgBufProvider::eID_PRV_CB: {
                MY_LOGD2("Try get eID_PRV_CB");
                mvClientMap.replaceValueFor(
                            IImgBufProvider::eID_PRV_CB,
                            mpSourceManager->getPrvCBPvdr()
                            );
                break;
            }
            case IImgBufProvider::eID_REC_CB: {
                MY_LOGD2("Try get eID_REC_CB");
                mvClientMap.replaceValueFor(
                            IImgBufProvider::eID_REC_CB,
                            mpSourceManager->getRecCBPvdr()
                            );
                break;
            }
            case IImgBufProvider::eID_GENERIC: {
                MY_LOGD2("Try get eID_GENERIC");
                mvClientMap.replaceValueFor(
                            IImgBufProvider::eID_GENERIC,
                            mpSourceManager->getGenericBufPvdr()
                            );
                break;
            }
            case IImgBufProvider::eID_FD: {
                MY_LOGD2("Try get eID_FD");
                mvClientMap.replaceValueFor(
                            IImgBufProvider::eID_FD,
                            mpSourceManager->getFDBufPvdr()
                            );
                break;
            }
            case IImgBufProvider::eID_OT: {
                MY_LOGD2("Try get eID_OT");
                mvClientMap.replaceValueFor(
                            IImgBufProvider::eID_OT,
                            mpSourceManager->getOTBufPvdr()
                            );
                break;
            }
            default: {
                MY_LOGE("Unsupport client:%d", mvClientMap.keyAt(i));
                return ret;
            }
        }
        if ( mvClientMap.valueAt(i) != 0 ) ret = OK;
    }

    //FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ClientBufferPool::
getBufferFromClient_l(
    char const*           /*szCallerName*/,
    MINT32                rRequestNo,
    sp<IImageBufferHeap>& rpBuffer,
    MUINT32&              rTransform
)
{
    //FUNC_START;

    if ( OK != tryGetClient() ) return UNKNOWN_ERROR;

    ImgBufQueNode node;
    sp<IImgBufProvider> pCurrentClient;
    for ( size_t i = 0; i < mvClientMap.size(); ++i ) {
        sp<IImgBufProvider> pClient = mvClientMap.valueAt(i);
        if ( pClient != 0 ) {
            if( pClient->dequeProvider(node) ) {
                pCurrentClient = pClient;
                break;
            }
        }
    }

    if ( pCurrentClient == 0 ) {
        MY_LOGD2("Cannot dequeProvider.");
        return NO_MEMORY;
    }

    if( pCurrentClient->getProviderId() == IImgBufProvider::eID_REC_CB )
    {
        if( pCurrentClient->isProviderRunning() == false )
        {
            MY_LOGD2("Rec provider has been stopped");
            return NO_MEMORY;
        }
        else
        {
            mbDumpNotReturnBuffer = MTRUE;
        }
    }

    MINT imgWidth = node.getImgBuf()->getImgWidth();
    MINT imgHeight = node.getImgBuf()->getImgHeight();
    size_t bufStridesInBytes[] = {0,0,0};
    bufStridesInBytes[0] =  GET_IIMGBUF_IMG_STRIDE_IN_BYTE(node.getImgBuf(), 0);
    bufStridesInBytes[1] =  GET_IIMGBUF_IMG_STRIDE_IN_BYTE(node.getImgBuf(), 1);
    bufStridesInBytes[2] =  GET_IIMGBUF_IMG_STRIDE_IN_BYTE(node.getImgBuf(), 2);

    if(mbUseTransform)
    {
        rTransform = node.getRotation();
    }

    size_t bufBoundaryInBytes[] = {0,0,0};
    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(
                                                    MtkCameraParameters::queryImageFormat(node.getImgBuf()->getImgFormat().string()),
                                                    MSize(
                                                        imgWidth,
                                                        imgHeight
                                                    ),
                                                    bufStridesInBytes,
                                                    bufBoundaryInBytes,
                                                    Format::queryPlaneCount(MtkCameraParameters::queryImageFormat(node.getImgBuf()->getImgFormat().string()))
                                                );
    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
                                    node.getImgBuf()->getIonFd(),
                                    (MUINTPTR)node.getImgBuf()->getVirAddr(),
                                    0,
                                    node.getImgBuf()->getBufSecu(),
                                    node.getImgBuf()->getBufCohe()
                                );
    //
    sp<GraphicBuffer> gb = node.getImgBuf()->getGraphicBuffer();
    if( gb == NULL )
    {
        rpBuffer = ImageBufferHeap::create(
                                        LOG_TAG,
                                        imgParam,
                                        portBufInfo,
                                        mLogLevel >= 2 //mbEenableIImageBufferLog
                                    );
    }
    else
    {
        rpBuffer = IGraphicImageBufferHeap::create(LOG_TAG, gb.get());
    }

    if(rpBuffer == 0) {
        MY_LOGE("rpBuffer is NULL");
        return NO_MEMORY;
    }

    mDequeOrderMap.push_back(rRequestNo);
    mClientBufferMap.add(rRequestNo, Buffer_T{rRequestNo, false, false, 0, node, pCurrentClient, NULL});
    //
    MBOOL perFrameLog = MFALSE;
    if( pCurrentClient->getProviderId() == IImgBufProvider::eID_DISPLAY)
    {
        perFrameLog = MTRUE;
    }
    else
    {
        perFrameLog = (mLogLevel>=1) ? MTRUE : MFALSE;
    }
    MY_LOGD_IF(perFrameLog, "ReqNo(%d),Name(%s),Buf(%p),VA(%p),Fmt(%s),R(%d)",
            rRequestNo,
            pCurrentClient->getProviderName(),
            rpBuffer.get(),
            node.getImgBuf()->getVirAddr(),
            node.getImgBuf()->getImgFormat().string(),
            node.getRotation());
    CAM_TRACE_FMT_BEGIN("CBPGetFromClient No%d",rRequestNo);
    CAM_TRACE_FMT_END();
    //FUNC_END;
#undef GET_IIMGBUF_IMG_STRIDE_IN_BYTE
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ClientBufferPool::
releaseToPool(
    char const*          szCallerName,
    MINT32               rRequestNo,
    sp<IImageBufferHeap> rpBuffer,
    MUINT64              rTimeStamp,
    bool                 rErrorResult
)
{
    MY_LOGD2("%s", szCallerName);

    Mutex::Autolock _l(mLock);
    return returnBufferToClient_l(
            szCallerName,
            rRequestNo,
            rpBuffer,
            rTimeStamp,
            rErrorResult
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ClientBufferPool::
returnBufferToClient_l(
    char const*          szCallerName,
    MINT32               rRequestNo,
    sp<IImageBufferHeap> rpBuffer,
    MUINT64              rTimeStamp,
    bool                 rErrorResult
)
{
    //FUNC_START;
    MUINT64 timestampDiff;
    ssize_t index = mClientBufferMap.indexOfKey(rRequestNo);
    if ( index < 0 ) {
        MY_LOGE("[%s][%d] rpBuffer:%p not found.", szCallerName, rRequestNo, rpBuffer.get());
        return UNKNOWN_ERROR;
    } else {
        MY_LOGD2("requestNo %d",rRequestNo);
        Buffer_T* buf  = &mClientBufferMap.editValueFor(rRequestNo);
        buf->isReturn  = true;
        buf->error     = rErrorResult;
        buf->timestamp = rTimeStamp;
        buf->heap      = rpBuffer;
    }
    size_t i = 0;
    MBOOL bRecBufReturn = MFALSE;
    while ( i < mDequeOrderMap.size() ) {
        index = mClientBufferMap.indexOfKey(mDequeOrderMap[i]);
        if ( index < 0 ) {
            MY_LOGE("[%s]buffer map not found, i(%d/%d), requestNo(%d)",
                    szCallerName, i, mDequeOrderMap.size(), mDequeOrderMap[i]);
            for( size_t j = 0 ; j < mDequeOrderMap.size() ; j++ )
            {
                MY_LOGE("mDequeOrderMap(%d), requestNo(%d)", j, mDequeOrderMap[j]);
            }
            for( size_t j = 0 ; j < mClientBufferMap.size() ; j++ )
            {
                MY_LOGE("mClientBufferMap(%d), requestNo(%d)", j, mClientBufferMap.keyAt(j));
            }
            return UNKNOWN_ERROR;
        }
        Buffer_T* buf = &mClientBufferMap.editValueFor(mDequeOrderMap[i]);
        if ( buf->isReturn ) {
            if ( buf->error ||
                 buf->timestamp == 0) {
                const_cast<ImgBufQueNode*>(&(buf->node))->setStatus(ImgBufQueNode::eSTATUS_CANCEL);
            } else {
                const_cast<ImgBufQueNode*>(&(buf->node))->setStatus(ImgBufQueNode::eSTATUS_DONE);
                const_cast<ImgBufQueNode*>(&(buf->node))->getImgBuf()->setTimestamp(buf->timestamp);  // time stamp

                if(!mbEnableBuffer)
                    const_cast<ImgBufQueNode*>(&(buf->node))->setStatus(ImgBufQueNode::eSTATUS_CANCEL);
            }
            //
            MBOOL perFrameLog = MFALSE;
            if( buf->client->getProviderId() == IImgBufProvider::eID_DISPLAY)
            {
                perFrameLog = MTRUE;
                //MY_LOGD("timestamp %lld, mLastTimestamp %lld", buf->timestamp, mLastTimestamp);
                timestampDiff = buf->timestamp - mLastTimestamp;
                mLastTimestamp = buf->timestamp;
            }
            else
            if( buf->client->getProviderId() == IImgBufProvider::eID_REC_CB &&
                buf->client->isProviderRunning() == false)
            {
                perFrameLog = MTRUE;
            }
            else
            {
                perFrameLog = (mLogLevel>=1) ? MTRUE : MFALSE;
            }
            MY_LOGD_IF(perFrameLog, "ReqNo(%d),Name(%s),Buf(%p),VA(%p),Fmt(%s),R(%d),TS(%d.%06d)",
                    buf->requestNo,
                    szCallerName,
                    rpBuffer.get(),
                    buf->node.getImgBuf()->getVirAddr(),
                    buf->node.getImgBuf()->getImgFormat().string(),
                    buf->node.getRotation(),
                    (MUINT32)((buf->timestamp/1000)/1000000),
                    (MUINT32)((buf->timestamp/1000)%1000000));
            MY_LOGD2("Err(%d),MS(%d)",
                    buf->error,
                    mClientBufferMap.size());
            //
            if( buf->client->getProviderId() == IImgBufProvider::eID_REC_CB &&
                mpRecordBufferSnapShot != NULL)
            {
                mpRecordBufferSnapShot->postBuffer(rpBuffer);
            }
            //
            if( buf->client->getProviderId() == IImgBufProvider::eID_DISPLAY &&
                mspResourceContainer != NULL)
            {
                //MY_LOGD("timestampDiff %lld ms", timestampDiff);
                sp<IFrameInfo> spFrameInfo = mspResourceContainer->queryLatestFrameInfo();
                if(spFrameInfo != NULL)
                {
                    IMetadata halMeta;
                    if(spFrameInfo->getFrameMetadata(rRequestNo, eSTREAMID_META_HAL_DYNAMIC_P1, halMeta))
                    {
                        MBOOL skipFrame = MFALSE;
                        tryGetMetadata<MINT32>(&halMeta, MTK_3A_SKIP_BAD_FRAME, skipFrame);
                        if(skipFrame)
                        {
                            MY_LOGW("skip frame rRequestNo %d",rRequestNo);
                            const_cast<ImgBufQueNode*>(&(buf->node))->setStatus(ImgBufQueNode::eSTATUS_CANCEL);
                        }
                    }
                }
                else
                {
                    if(timestampDiff >= 30000000)
                    {
                        MY_LOGW("failed to queryLatestFrameInfo");
                    }
                }
            }
            //
            if( buf->client->getProviderId() == IImgBufProvider::eID_REC_CB &&
                buf->client->isProviderRunning() == false)
            {
                bRecBufReturn = MTRUE;
                MY_LOGD2("Rec provider has been stopped");
            }
            else
            {
                buf->client->enqueProvider(buf->node);
            }
            mClientBufferMap.removeItem(mDequeOrderMap[i]);
            mDequeOrderMap.removeAt(i);
        } else {
            MY_LOGD2("i %d, requestNo %d, isReturn %d",
                    i,
                    buf->requestNo,
                    buf->isReturn);
            break;
        }
    }
    //
    MINT recBufCnt = 0;
    for( i= 0 ; i < mDequeOrderMap.size() ; i++ )
    {
        Buffer_T* buf = &mClientBufferMap.editValueFor(mDequeOrderMap[i]);
        if( buf->client->getProviderId() == IImgBufProvider::eID_REC_CB &&
            buf->client->isProviderRunning() == false)
        {
            recBufCnt++;
            MY_LOGD_IF(mbDumpNotReturnBuffer, "(%02d/%02d) Not return RecBuf:ReqNo(%d),Name(%s),Buf(%p),VA(%p),Fmt(%s)",
                    i+1,
                    mDequeOrderMap.size(),
                    buf->requestNo,
                    szCallerName,
                    rpBuffer.get(),
                    buf->node.getImgBuf()->getVirAddr(),
                    buf->node.getImgBuf()->getImgFormat().string());
        }
    }
    //
    if(bRecBufReturn)
    {
        mbDumpNotReturnBuffer = MFALSE;
    }
    //
    if(recBufCnt)
    {
        MY_LOGI("Not return RecBuf num %d", recBufCnt);
    }
    else
    if(bRecBufReturn)
    {
        MY_LOGI("Lat return RecBuf after stop recording");
    }
    //
    CAM_TRACE_FMT_BEGIN("CBPReleaseToClient No%d",rRequestNo);
    CAM_TRACE_FMT_END();
    //FUNC_END;

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
char const*
ClientBufferPool::
poolName() const
{
    return "Client";
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ClientBufferPool::
dumpPool() const
{
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ClientBufferPool::
setCamClient(
    char const*                /*szCallerName*/,
    sp<ImgBufProvidersManager> pSource,
    MINT32                     rMode
)
{
    FUNC_START;

    mpSourceManager = pSource;
    mvClientMap.add(rMode, NULL);

    MY_LOGD("mpSourceManager:%p ClientMode:%d", mpSourceManager.get(), rMode);
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ClientBufferPool::
setCamClient(
    MINT32                     rMode
)
{
    FUNC_START;
    if ( mpSourceManager == 0 ) {
        MY_LOGE("Client manager not set.");
        return BAD_VALUE;
    }

    mvClientMap.add(rMode, NULL);

    MY_LOGD("ClientMode:%d", rMode);
    FUNC_END;
    return OK;
}

MBOOL
ClientBufferPool::
enableBuffer(
    MBOOL                      enable
)
{
    Mutex::Autolock _l(mLock);

    MY_LOGD("IsEnable(%d)", enable);
    mbEnableBuffer = enable;
    return OK;
}
