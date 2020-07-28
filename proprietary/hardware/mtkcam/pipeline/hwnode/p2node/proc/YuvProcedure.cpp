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
#define PROCESSOR_NAME_YUV  "YUV"
#define DEFINE_OPEN_ID      (muOpenId)

#define LOG_TAG "MtkCam/P2Node_YUV"
#include "YuvProcedure.h"
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
using namespace NSCam;
using namespace plugin;

/******************************************************************************
 *
 ******************************************************************************/
sp<Processor>
YuvProcedure::
createProcessor(CreateParams &params) {
    return new ProcessorBase<YuvProcedure>(
            params.uOpenId, params, PROCESSOR_NAME_YUV);
}

/******************************************************************************
 *
 ******************************************************************************/
YuvProcedure::
YuvProcedure(CreateParams const &params)
        : mbEnableLog(params.bEnableLog),
          muOpenId(params.uOpenId)
{
    muDumpBuffer = ::property_get_int32("debug.camera.dump.yuv", 0);
    mpFrameHandler = new FrameHandler<FrameInfo>(params.bEnableLog);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
YuvProcedure::
onYuvPullParams(
    sp<Request> pRequest,
    FrameParams &frameParams)
{
     if (! (pRequest->context.processorMask & MTK_P2_YUV_PROCESSOR) ) {
        MY_LOGE("cannot enter YuvProcedure");
        return UNKNOWN_ERROR;
    }
/***************************************
 [YUV,YUV]:
 in_app_meta
 in_hal_meta
 vWork_buffer / out_buffers
 ****************************************/
    FUNC_START;

    MBOOL bLastCall  = MTRUE;
    // wait imgo+rrzo request
    if( pRequest->context.processorMask&MTK_P2_PREVIEW_REQUEST
     && pRequest->context.processorMask&MTK_P2_CAPTURE_REQUEST )
    {
        bLastCall = pRequest->getFrameSubNo()/10 == pRequest->getFrameSubNo()%10-1 ? MTRUE : MFALSE;
    }
    FrameInfo frameInfo;
    // 1. prepare params
    {
        frameParams.lastCall   = bLastCall;
        frameInfo.uRequestNo  = pRequest->getRequestNo();
        frameInfo.uFrameNo    = pRequest->getFrameNo();
        frameInfo.userID      = pRequest->context.userId;
        frameInfo.pVendor     = pRequest->getVendorMgr();
        frameInfo.needDstBuffer = !(pRequest->context.processorMask & MTK_P2_MDP_PROCESSOR);
        if(frameInfo.pVendor.get() == NULL)
            return UNKNOWN_ERROR;
        //
        frameInfo.meta.setting.appMeta   = *pRequest->context.in_app_meta->getMetadata();
        frameInfo.meta.setting.halMeta   = *pRequest->context.in_hal_meta->getMetadata();

        if(pRequest->context.in_app_ret_meta.get())
        {
            frameInfo.meta.others.push_back(*pRequest->context.in_app_ret_meta->getMetadata());
            pRequest->context.in_app_ret_meta.clear();
        }
        else
            MY_LOGW("no p1 dynamic result meta.");
        //
        if(pRequest->context.out_hal_meta.get())
        {
            frameInfo.meta.others.push_back(*pRequest->context.out_hal_meta->getMetadata());
            frameInfo.outHal = pRequest->context.out_hal_meta;
        }

        if(pRequest->context.out_app_meta.get())
        {
            frameInfo.outApp = pRequest->context.out_app_meta;
            pRequest->context.out_app_meta.clear();
        }
    }
    // 3. src working buffer
    BufferItem item = reinterpret_cast< WorkingBufferHandle* >(pRequest->context.work_buffer.get())->getBufferItem();
    pRequest->context.work_buffer.clear();
    // workaound
    pRequest->context.in_mdp_buffer.clear();
    frameInfo.vInBuff.push_back(item);

    // 4. need to push dst buffer or not
    if(frameInfo.needDstBuffer && pRequest->context.out_buffers.size())
    {
        // out buffer
        vector<sp<BufferHandle>>::iterator iter = pRequest->context.out_buffers.begin();
        for (; iter != pRequest->context.out_buffers.end(); iter++)
        {
            sp<BufferHandle>& pOutBuffer = *iter;
            if (pOutBuffer.get() == NULL){
                MY_LOGW("no streamBuffer");
                continue;
            }
            //
            sp<IImageBufferHeap> pHeap = pOutBuffer->getImageBufferHeap();
            frameInfo.vBufMap[pHeap.get()] = pOutBuffer;
            BufferItem out;
            out.bufferStatus    = BUFFER_OUT | BUFFER_EMPTY;
            out.heap            = pHeap;
            out.streamInfo      = const_cast<IImageStreamInfo*>(pOutBuffer->getStreamBuffer()->getStreamInfo());
            frameInfo.vInBuff.push_back(out);
            pOutBuffer.clear();
            MY_LOGDO_IF(1, "out heap(%p) streamId:%#" PRIx64 ", status: 0x%x, img size: %dx%d",
                                out.heap.get(),
                                out.streamInfo->getStreamId(),
                                out.bufferStatus,
                                out.streamInfo->getImgSize().w,
                                out.streamInfo->getImgSize().h );
        }
    }
    // debug
    Vector<BufferItem>::iterator iter = frameInfo.vInBuff.begin();
    for (; iter != frameInfo.vInBuff.end(); iter++)
    {
        MY_LOGD("src buffer heap: %p",(*iter).heap.get());
    }

    Mutex::Autolock _l(mLock);
    mpFrameHandler->collect(frameInfo.uFrameNo, pRequest, frameInfo);
    MY_LOGDO("[%02d]Resized(%d) F/R Number: %d/%d, uId: %d, vSize(buf/otherMeta): %zu/%zu, needDstBuffer: %d, iso: %d, last: %d",
                   pRequest->getFrameSubNo(),
                   pRequest->context.resized,
                   frameInfo.uFrameNo,
                   frameInfo.uRequestNo,
                   frameInfo.userID,
                   frameInfo.vInBuff.size(), frameInfo.meta.others.size(),
                   frameInfo.needDstBuffer,
                   pRequest->context.iso,
                   frameParams.lastCall);
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
YuvProcedure::
onYuvExecute(
        sp<Request> const pRequest,
        FrameParams const &frameParams)
{
    CAM_TRACE_NAME("P2:onYuvExecute");
    FUNC_START;
    status_t ret = OK;

    Vector< BufferItem > inBuff;
    MetaItem   setting;

    if(!frameParams.lastCall){
        MY_LOGDO_IF(mbEnableLog, "do noting....");
    }
    else {
        mLock.lock();
        // collect params
        size_t size = mpFrameHandler->sizeOfRequest(pRequest->getFrameNo());
        FrameInfo& param = mpFrameHandler->getFrameParamAndEdit(pRequest->getFrameNo(), 0);
        inBuff  = param.vInBuff;
        param.vInBuff.clear();
        setting = param.meta;
        //
        for(size_t idx =1; idx < size; idx++)
        {
            FrameInfo& param1 = mpFrameHandler->getFrameParamAndEdit(pRequest->getFrameNo(), idx);
            inBuff.appendVector(param1.vInBuff);
            param1.vInBuff.clear();
            MY_LOGDO_IF(mbEnableLog, "collect %zu buffer", inBuff.size());
        }
        mLock.unlock();
        ret = param.pVendor->queue(
                            param.uFrameNo,
                            static_cast< IVendorManager::IDataCallback* >(this),    //wp<IDataCallback>    cb
                            inBuff,                         //Vector< BufferItem > srcBuffer
                            setting                         //MetaItem   setting
                        );
    }
    MY_LOGDO("Resized(%d) F/R Number: %d/%d, vSize(buf/otherMeta): %zu/%zu, last: %d",
                   pRequest->context.resized,
                   pRequest->getFrameNo(),
                   pRequest->getRequestNo(),
                   inBuff.size(), setting.others.size(),
                   frameParams.lastCall);
   FUNC_END;
   return ret;
 }


/******************************************************************************
 *
 ******************************************************************************/
MERROR
YuvProcedure::
onYuvFinish(
        FrameParams const &/*params*/,
        MBOOL const /*success*/)
{
    CAM_TRACE_NAME("P2:onYuvFinish");
    FUNC_START;
    if (muDumpBuffer) {
        /*sp<IImageBuffer> pImgBuf = (mpPostProcessing->profile().processingType == PostProcessing::eIN_OUT_PROCESSING)?
                                   params.pOutHandle->getBuffer():
                                   params.pInHandle->getBuffer();

        if (!NSCam::Utils::makePath(P2_DEBUG_DUMP_PATH, 0660))
            MY_LOGWO("makePath[%s] fails", P2_DEBUG_DUMP_PATH);

        // ouput
        char filename[256] = {0};
        sprintf(filename, P2_DEBUG_DUMP_PATH "/pg-%04d-out-%dx%d.yuv",
                params.uFrameNo,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);

        MY_LOGDO("[yuv][out] %d (%dx%d) fmt(0x%x)",
                params.uFrameNo,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
                pImgBuf->getImgFormat());

        pImgBuf->saveToFile(filename);*/
    }

    /*params.pInHandle->updateState(
            success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);

    if (mpPostProcessing->profile().processingType == PostProcessing::eIN_OUT_PROCESSING)
    {
        params.pOutHandle->updateState(
                success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);
    }*/
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
YuvProcedure::
onDataReceived(
    MBOOL const isLastCb,
    MUINT32 const requestNo,
    MetaSet       result,
    Vector<BufferItem>    buffers
 )
{
    FUNC_START;
    Mutex::Autolock _l(mLock);
    MY_LOGDO_IF(1, "lastCb(%d) R Number: %d, buffer size: %zu, result app/hal meta: %d/%d",
                    isLastCb, requestNo, buffers.size(), result.appMeta.count(), result.halMeta.count()
                    );
    //
    size_t size = mpFrameHandler->sizeOfRequest(requestNo);
    for(size_t idx =0; idx < size; idx++)
    {
        FrameInfo& param = mpFrameHandler->getFrameParamAndEdit(requestNo, idx);
        if(P2_DEBUG_LOG)
            param.dump();
        sp<Request> pRequest = mpFrameHandler->getRequest(requestNo, idx);
        // buffer
        for (size_t i = 0; i < buffers.size(); ++i)
        {
            BufferItem buffer   = buffers.itemAt(i);
            StreamId_T streamid = !buffer.streamInfo.get() ? 0 : buffer.streamInfo->getStreamId();
            MINT32     status   = buffer.bufferStatus;
            MY_LOGDO("streamId: %#" PRIx64 ", buffer heap: %p, needDstBuffer: %d, bufferStatus:0x%x",
                       streamid, buffer.heap.get(), param.needDstBuffer, buffer.bufferStatus);
            sp<BufferHandle> pBufHandle = NULL;
            if(streamid == IVENDOR_STREAMID_IMAGE_PIPE_WORKING)// get wb
            {
                pBufHandle = WorkingBufferHandle::create(
                                    "VendorWB:Yuv",
                                    buffer,
                                    pRequest->getVendorMgr(),
                                    param.meta,
                                    pRequest->context.userId
                                );

                if(pBufHandle.get())
                {
                    pRequest->context.in_mdp_buffer = pBufHandle;
                    pRequest->context.in_mdp_crop = MTRUE;
                }
                else
                    MY_LOGE("can't create buffer handle");
            }
            else // get dst buffer
            {
                sp<BufferHandle> pBufHandle = param.vBufMap[buffer.heap.get()];
                if(pBufHandle.get())
                {
                    pBufHandle->updateState(
                                status & BUFFER_WRITE_ERROR  ?
                                BufferHandle::STATE_WRITE_FAIL : BufferHandle::STATE_WRITE_OK
                            );
                    param.vBufMap.erase(buffer.heap.get());
                    pBufHandle = NULL;
                }
            }
        }
        //
        auto updateMetaAndRemoveMetaHandleIfNeeded = [&](IMetadata meta, sp<MetaHandle> pMetaHandle, bool remove) {
            MERROR err = OK;
            IMetadata *pMeta_Out = pMetaHandle.get()  ?
                                     pMetaHandle->getMetadata() : NULL;
            if(pMeta_Out == NULL)
                MY_LOGE("this request have no out meta");
            else
                for (size_t i = 0; i < meta.count(); i++)
                {
                    err = pMeta_Out->update(meta.entryAt(i).tag(), meta.entryAt(i));
                    if(err != OK)
                        break;
                }
            pMetaHandle->updateState(err==OK ?
                                     MetaHandle::STATE_WRITE_OK  :  MetaHandle::STATE_WRITE_FAIL);
            if(remove)
                pMetaHandle.clear();
        };

        if(!result.appMeta.isEmpty())
        {
            updateMetaAndRemoveMetaHandleIfNeeded(result.appMeta, param.outApp, true);
        }

        if(!result.halMeta.isEmpty())
        {
            updateMetaAndRemoveMetaHandleIfNeeded(result.halMeta, param.outHal, true);
        }

        if(!isLastCb && idx < size-1)
            continue;

        else if(!isLastCb)
            return OK;

        if(param.outApp.get())
            param.outApp.clear();

        if(param.outHal.get())
            param.outHal.clear();

        MY_LOGDO("R Number: %d subNo: %02d done.",requestNo, pRequest->getFrameSubNo());
        pRequest->responseDone(MTRUE);
    }
    mpFrameHandler->remove(requestNo);
    FUNC_END;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
YuvProcedure::
onYuvFlush()
{
    CAM_TRACE_NAME("P2:onYuvFlush");
    mpFrameHandler->dump();
    /*sp<Request> pRequest = NULL;
    size_t size = mpFrameHandler->size();
    MY_LOGE_IF(size,"vendor have %d frame(s) Not callback completely.", size);
    for(size_t i =0;i < size; i++)
    {
        {
            pRequest = mpFrameHandler->getRequestByIndex(i, 0);
            MY_LOGWO("remain F/R Number(%d/%d)...", pRequest->getFrameNo(),pRequest->getRequestNo());
            //pRequest->responseDone(MTRUE);
            //mpFrameHandler->remove(pRequest->getRequestNo());
        }
    }*/
    return;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
YuvProcedure::FrameInfo::
dump() const
{
    MY_LOGI("Dump FrameParams");
    MY_LOGI("\tvendorMgr: %p", pVendor.get());
    MY_LOGI("\tframeNo: %d, requestNo: %d", uFrameNo, uRequestNo);
    MY_LOGI("\tuserID: %d", userID);
    MY_LOGI("\tneedDstBuffer: %d", needDstBuffer);

    for(auto const &iter: vBufMap)
    {
        MY_LOGI("\t\t[%p] StreamId:%#" PRIx64 " count: %d", iter.first, iter.second->getStreamId(), iter.second->getStrongCount());
    }

    return;
}

