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

#define LOG_TAG "MtkCam/BufferCallbackHandler"
//
#include "MyUtils.h"
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/utils/std/Format.h>
//
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>

using namespace android;
using namespace android::MtkCamUtils;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::Utils;

using namespace NSCam::v3::Utils;
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

/******************************************************************************
 *
 ******************************************************************************/
#define BUFFERPOOL_NAME       ("Cam@v1BufferPool")
#define BUFFERPOOL_POLICY     (SCHED_OTHER)
#define BUFFERPOOL_PRIORITY   (0)


/******************************************************************************
 *
 ******************************************************************************/
BufferCallbackHandler::
BufferCallbackHandler(
    MINT32 rOpenId
)
    : mOpenId(rOpenId)
{}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
BufferCallbackHandler::
onBufferAcquired(
    MINT32           rRequestNo,
    StreamId_T       rStreamId
)
{
    MY_LOGD("+ [%d] rRequestNo:%d rStreamId:%d", mOpenId,rRequestNo,rStreamId);
    Mutex::Autolock _l(mLock);
    ssize_t index = mvOrder.indexOfKey(rRequestNo);
    if ( index < 0 ) {
        Vector<Buffer_T> v;
        v.push_back(Buffer_T{rRequestNo, rStreamId, false, false, NULL});
        mvOrder.add(rRequestNo, v);
    } else {
        mvOrder.editValueFor(rRequestNo).push_back(Buffer_T{rRequestNo, rStreamId, false, false, NULL});
    }
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
BufferCallbackHandler::
onBufferReturned(
    MINT32                         rRequestNo,
    StreamId_T                     rStreamId,
    bool                           bErrorBuffer,
    android::sp<IImageBufferHeap>& rpBuffer
)
{
    MY_LOGD("+ [%d] rRequestNo:%d rStreamId:%d bErrorBuffer:%d rpBuffer:%p", mOpenId, rRequestNo, rStreamId, bErrorBuffer, rpBuffer.get());
    Mutex::Autolock _l(mLock);
    bool bKeepBuf;
    MINT32 i, size;

    /* check RequestNO */
    if (mvOrder.indexOfKey(rRequestNo) < 0)
    {
        MY_LOGE("[%d] no find rRequestNo. Should not happen.", mOpenId);
        FUNC_END;
        return false;
    }

    /* Process the return buffer */
    size     = mvOrder.editValueFor(rRequestNo).size();
    bKeepBuf = (rStreamId == eSTREAMID_IMAGE_JPEG && size > 1); // must KEEP jpeg buffer until all other buffers are released
    sp<IImageCallback> pCb = mpCallback.promote();
    if (pCb == NULL)
    {
        MY_LOGE("[%d] streamID(%d) mpCallback is NULL! can't callback buffer!!", mOpenId, rStreamId);
        FUNC_END;
        return false;
    }
    //
    for (i = 0 ; i < size ; i++)
    {
        Buffer_T* buf = &mvOrder.editValueFor(rRequestNo).editItemAt(i);
        if(buf == 0)
        {
            MY_LOGE("[%d] buf is NULL. Should not happen.", mOpenId);
            FUNC_END;
            return false;
        }
        if (buf->streamId == rStreamId && !buf->isReturned)
        {
            buf->buffer = rpBuffer->createImageBuffer();
            buf->isError = bErrorBuffer;

            /* Keep buffer if it is Jpeg and some non-Jpeg buffers aren't received */
            if (bKeepBuf)
            {
                buf->isReturned = false;
            }
            else
            {
                /* callback current buffer */
                if(pCb != NULL)
                {
                    pCb->onResultReceived(buf->requestNo, buf->streamId, buf->isError, buf->buffer);
                    MY_LOGD("[%d] onResultReceived(%d,%#" PRIxPTR ",%d,%p)", mOpenId,
                        buf->requestNo, buf->streamId, buf->isError, buf->buffer.get());
                }
                else
                {
                    MY_LOGW("mpCallback.promote() == NULL, [%d] onResultReceived(%d,%#" PRIxPTR ",%d,%p)", mOpenId,
                        buf->requestNo, buf->streamId, buf->isError, buf->buffer.get());
                }
                buf->isReturned = true;
            }
            break;
        }
    }

    if (i == size)
    {
        MY_LOGE("[%d] Can't find streamID(%d). Should not happen.", mOpenId, rStreamId);
        FUNC_END;
        return false;
    }

    /* check Jpeg buffer, callback if the other buffer is for Jpeg */
    if (!bKeepBuf && size == 2)
    {
        for (i = 0 ; i < size ; i++)
        {
            Buffer_T* buf = &mvOrder.editValueFor(rRequestNo).editItemAt(i);
            if (!buf->isReturned && buf->buffer != NULL && buf->streamId == eSTREAMID_IMAGE_JPEG)
            {
                //callback Jpeg at last
                pCb->onResultReceived(buf->requestNo, buf->streamId, buf->isError, buf->buffer);
                MY_LOGD("[%d] onResultReceived(%d,%#" PRIxPTR ",%d,%p)", mOpenId,
                    buf->requestNo, buf->streamId, buf->isError, buf->buffer.get());
                buf->isReturned = true;
            }
        }
    }

    /* remove returned item for this RequestNo, remove this RequestNO from mvOrder if no item insides */
    Vector<Buffer_T>::iterator it;
    for (it = mvOrder.editValueFor(rRequestNo).begin() ; it != mvOrder.editValueFor(rRequestNo).end() ;)
    {
      if (it->isReturned)
        it = mvOrder.editValueFor(rRequestNo).erase(it);
      else
        it++;
    }
    if (mvOrder.editValueFor(rRequestNo).size() == 0) mvOrder.removeItem(rRequestNo);

    FUNC_END;
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
BufferCallbackHandler::
setBufferPool( android::sp<CallbackBufferPool> pPool )
{
    if(pPool==NULL)
    {
        MY_LOGE("[%d] Pool is NULL!", mOpenId);
        return;
    }
    pPool->setNotifier(this);
    android::sp<IImageStreamInfo> pStreamInfo = pPool->getStreamInfo();
    if(pStreamInfo==NULL)
    {
        MY_LOGE("[%d] Pool has no StreamInfo!", mOpenId);
        return;
    }
    mvBufferPool.add( pStreamInfo->getStreamId(), Info_T{pStreamInfo->getStreamId(), pPool} );
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
BufferCallbackHandler::
onLastStrongRef(const void* /*id*/)
{
    CAM_TRACE_NAME("BufferCallbackHandler:onLastStrongRef (flush pool)");
    FUNC_START;
    for(int i=0; i<mvBufferPool.size(); i++)
    {
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
        // for reusable,
        // buffer pool will free by owner or no owner to keep the pool sp
        MY_LOGW("for reusable, buffer pool for streamId(0x%#" PRIxPTR ") should free by owner",
                mvBufferPool.editValueAt(i).pool->getStreamInfo()->getStreamId());
#else
        mvBufferPool.editValueAt(i).pool->flush();
#endif
    }
    FUNC_END;
}
