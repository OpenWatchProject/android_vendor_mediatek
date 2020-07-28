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
#define PROCESSOR_NAME_RAW  "RAW"
#define DEFINE_OPEN_ID      (muOpenId)

#define LOG_TAG "MtkCam/P2Node_RAW"

#include "RawProcedure.h"
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

using namespace NSCam;
using namespace plugin;


/******************************************************************************
 *
 ******************************************************************************/
sp<Processor>
RawProcedure::
createProcessor(
    CreateParams &params
)
{
    return new ProcessorBase<RawProcedure>(
            params.uOpenId, params, PROCESSOR_NAME_RAW);
}
/******************************************************************************
 *
 ******************************************************************************/
RawProcedure::
RawProcedure(CreateParams const &params)
    : mbEnableLog(params.bEnableLog)
    , muOpenId(params.uOpenId)
{
    muDumpBuffer = ::property_get_int32("debug.camera.dump.raw", 0);
    mpFrameHandler = new FrameHandler<FrameInfo>(params.bEnableLog);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RawProcedure::
onRawPullParams(
    sp<Request> pRequest,
    FrameParams &frameParams
)
{
     if (! (pRequest->context.processorMask & MTK_P2_RAW_PROCESSOR) ) {
        MY_LOGE("cannot enter RawProcedure");
        return UNKNOWN_ERROR;
    }
 /***************************************
 [RAW,RAW]:
 in_app_meta
 in_hal_meta
 in_buffer

 [RAW,YUV]: need to compose a pipelineframe
 in_app_meta
 in_hal_meta
 in_buffer(s)
 out_buffers
 ****************************************/
    FUNC_START;

    MBOOL bLastCall  = MTRUE;
    if(pRequest->context.processorMask&MTK_P2_PREVIEW_REQUEST
              && pRequest->context.processorMask&MTK_P2_CAPTURE_REQUEST)
    {
        bLastCall = pRequest->getFrameSubNo()/10 == pRequest->getFrameSubNo()%10-1 ? MTRUE : MFALSE;
    }
    FrameInfo frameInfo;
    // 1. prepare params
    {
        frameParams.lastCall    = bLastCall;
        frameInfo.uRequestNo  = pRequest->getRequestNo();
        frameInfo.uFrameNo    = pRequest->getFrameNo();
        frameInfo.userID      = pRequest->context.userId;
        frameInfo.pVendor     = pRequest->getVendorMgr();
        frameInfo.passOtherProcessors = pRequest->context.processorMask & MTK_P2_ISP_PROCESSOR;
        if(frameInfo.pVendor.get() == NULL)
            return UNKNOWN_ERROR;
        //
        frameInfo.meta.setting.appMeta  = *pRequest->context.in_app_meta->getMetadata();
        frameInfo.meta.setting.halMeta  = *pRequest->context.in_hal_meta->getMetadata();
        //
        if(pRequest->context.in_app_ret_meta.get())
        {
            frameInfo.meta.others.push_back(*pRequest->context.in_app_ret_meta->getMetadata());
        }
        else
            MY_LOGW("no p1 dynamic result meta.");

        if(pRequest->context.out_hal_meta.get())
        {
            frameInfo.outHal = pRequest->context.out_hal_meta;
        }

        if(pRequest->context.out_app_meta.get())
        {
            frameInfo.outApp = pRequest->context.out_app_meta;
        }
    }
    //
    vector<sp<BufferHandle>>::iterator iter = pRequest->context.in_buffer.begin();
    for (; iter != pRequest->context.in_buffer.end(); iter++)
    {
        sp<BufferHandle>& pInBuffer = (*iter);
        if (pInBuffer.get() == NULL){
            MY_LOGW("not streamBuffer");
            continue;
        }
        sp<IImageBufferHeap> pHeap = pInBuffer->getImageBufferHeap();
        frameInfo.vBufMap[pHeap.get()] = pInBuffer;
        BufferItem in;
        in.bufferStatus    = BufferStatus_T::BUFFER_IN;
        in.heap            = pHeap;
        in.streamInfo      = const_cast<IImageStreamInfo*>(pInBuffer->getStreamBuffer()->getStreamInfo());
        frameInfo.vInBuff.push_back(in);


        MY_LOGDO_IF(1,"[D]get src buffer, streamId: %#" PRIx64 ",img size: %dx%d, buffer heap: %p, status 0x%x",
                      in.streamInfo->getStreamId(),
                      in.streamInfo->getImgSize().w,
                      in.streamInfo->getImgSize().h,
                      in.heap.get(),
                      in.bufferStatus);
    }

    //pass no processors after this processor
    if(!frameInfo.passOtherProcessors)
    {
        // in buffer
        pRequest->context.in_buffer.clear();
        // lcso
        if(pRequest->context.in_lcso_buffer.get())
        {
            sp<BufferHandle>& pOutBuffer = pRequest->context.in_lcso_buffer;
            android::sp<IImageBufferHeap> pHeap = pRequest->context.in_lcso_buffer->getImageBufferHeap();
            frameInfo.vBufMap[pHeap.get()] = pOutBuffer;
            BufferItem out;
            out.bufferStatus    = BUFFER_IN | BUFFER_EMPTY;
            out.heap            = pHeap;
            out.streamInfo      = const_cast<IImageStreamInfo*>(pRequest->context.in_lcso_buffer->getStreamBuffer()->getStreamInfo());
            frameInfo.vInBuff.push_back(out);
        }

        // out buffer
        vector<sp<BufferHandle>>::iterator iter = pRequest->context.out_buffers.begin();
        for (; iter != pRequest->context.out_buffers.end(); iter++)
        {
            sp<BufferHandle>& pOutBuffer = (*iter);
            if (pOutBuffer.get() == NULL){
                MY_LOGW("not streamBuffer");
                continue;
            }
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
                                out.streamInfo->getImgSize().h);
        }
        pRequest->context.out_app_meta.clear();
        pRequest->context.out_hal_meta.clear();
        pRequest->context.in_app_ret_meta.clear();
        if (frameInfo.vInBuff.size() == 0) {
            return UNKNOWN_ERROR;
        }
    }
    Mutex::Autolock _l(mLock);
    mpFrameHandler->collect(frameInfo.uFrameNo, pRequest, frameInfo);

    MY_LOGDO("[%02d]Resized(%d) F/R Number: %d/%d, uId: %d, vSize(buf/meta): %zu/%zu, passOtherProcessors: %d, last: %d",
                   pRequest->getFrameSubNo(),
                   pRequest->context.resized,
                   frameInfo.uFrameNo,
                   frameInfo.uRequestNo,
                   frameInfo.userID,
                   frameInfo.vInBuff.size(), frameInfo.meta.others.size(),
                   frameInfo.passOtherProcessors,
                   frameParams.lastCall);
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RawProcedure::
onRawExecute(
    sp<Request> const pRequest,
    FrameParams const &frameParams
)
{
    CAM_TRACE_NAME("P2:onRawExecute");
    FUNC_START;
    status_t ret = OK;

    Vector< BufferItem > inBuff;
    MetaItem   setting;

    if(!frameParams.lastCall){
        MY_LOGDO_IF(mbEnableLog, "do noting....");
    }
    else{
        mLock.lock();
        // collect params
        size_t size = mpFrameHandler->sizeOfRequest(pRequest->getFrameNo());
        FrameInfo& param = mpFrameHandler->getFrameParamAndEdit(pRequest->getFrameNo(), 0);
        inBuff = param.vInBuff;
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
        //
        MY_LOGDO_IF(mbEnableLog, "cd->%p", static_cast< IVendorManager::IDataCallback* >(this));
        ret = param.pVendor->queue(
                            param.uFrameNo,
                            static_cast< IVendorManager::IDataCallback* >(this),
                            inBuff,
                            setting
                        );
        if(ret!=OK)
            mpFrameHandler->remove(param.uFrameNo);
    }

    FUNC_END;
    return ret==OK ? OK : UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RawProcedure::
onRawFinish(
    FrameParams const &/*params*/,
    MBOOL const /*success*/)
{
    CAM_TRACE_NAME("P2:onRawFinish");
    FUNC_START;

    if (muDumpBuffer) {
    }

    // [TODO] check if need release buffer or not
    //
    //params.pInBufferHandle->updateState(
    //        success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);
    //for (size_t i = 0; i < params.pOutBufferHandle.size(); i++)
    //    if (params.pOutBufferHandle[i].mHandle.get())
    //        params.pOutBufferHandle[i].mHandle->updateState(
    //                success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL
    //        );
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
RawProcedure::
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
                    isLastCb, requestNo, buffers.size(), result.appMeta.count(), result.halMeta.count());
    size_t size = mpFrameHandler->sizeOfRequest(requestNo);

    for(size_t idx =0; idx < size; idx++)
    {
        FrameInfo& param = mpFrameHandler->getFrameParamAndEdit(requestNo, idx);

        // buffer
        for (size_t i = 0; i < buffers.size(); ++i)
        {
            BufferItem buffer   = buffers.itemAt(i);
            sp<BufferHandle> pBufHandle = param.vBufMap[buffer.heap.get()];
            if(pBufHandle.get())
            {
                MUINT32    streamid = !buffer.streamInfo.get() ? 0 : buffer.streamInfo->getStreamId();
                MINT32     status   = buffer.bufferStatus ;
                MY_LOGDO("streamId: %d, buffer heap: %p, status: 0x%x", streamid, buffer.heap.get(),status);
                //
                pBufHandle->updateState(
                            status & BUFFER_WRITE_ERROR  ?
                            BufferHandle::STATE_WRITE_FAIL : BufferHandle::STATE_WRITE_OK
                        );

                param.vBufMap.erase(buffer.heap.get());
                pBufHandle = NULL;
            }
        }

        auto updateMetaAndRemoveMetaHandleIfNeeded = [&](IMetadata meta, sp<MetaHandle> pMetaHandle, bool remove) {
            MERROR err = OK;
            IMetadata *pMeta_Out = pMetaHandle.get()  ?
                                     pMetaHandle->getMetadata() : NULL;
            if(pMeta_Out == NULL)
                MY_LOGE("this request have no out meta");
            else {
                for (size_t i = 0; i < meta.count(); i++)
                {
                    err = pMeta_Out->update(meta.entryAt(i).tag(), meta.entryAt(i));
                    if(err != OK)
                        break;
                }
            }
            pMetaHandle->updateState(err==OK ?
                                     MetaHandle::STATE_WRITE_OK  :  MetaHandle::STATE_WRITE_FAIL);
            if(remove)
                pMetaHandle.clear();
        };

        if(!result.appMeta.isEmpty())
            updateMetaAndRemoveMetaHandleIfNeeded(result.appMeta, param.outApp, !param.passOtherProcessors);

        if(!result.halMeta.isEmpty())
            updateMetaAndRemoveMetaHandleIfNeeded(result.halMeta, param.outHal, !param.passOtherProcessors);

        if(!isLastCb && idx < size-1)
            continue;

        else if(!isLastCb)
            return OK;

        if(param.outApp.get())
            param.outApp.clear();

        if(param.outHal.get())
            param.outHal.clear();

        sp<Request> pRequest = mpFrameHandler->getRequest(requestNo, idx);
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
RawProcedure::
onRawFlush()
{
    CAM_TRACE_NAME("P2:onRawFlush");
    mpFrameHandler->dump();
    /*sp<Request> pRequest = NULL;
    size_t size = mpFrameHandler->size();
    MY_LOGE_IF(size,"vendor have %d frame(s) Not callback completely.", size);
    for(size_t i =0;i < size; i++)
    {
        for(size_t idx =0;idx < mpFrameHandler->sizeOfRequestByIndex(i); idx++)
        {
            pRequest = mpFrameHandler->getRequestByIndex(i, idx);
            MY_LOGWO("remain F/R Number(%d/%d)...", pRequest->getFrameNo(),pRequest->getRequestNo());
        }
    }*/
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
RawProcedure::FrameInfo::
dump()
{
    MY_LOGI("Dump FrameParams");
    MY_LOGI("\tvendorMgr: %p", pVendor.get());
    MY_LOGI("\tframeNo: %d, requestNo: %d", uFrameNo, uRequestNo);
    MY_LOGI("\tuserID: %d", userID);
    MY_LOGI("\tneedDstBuffer: %d", passOtherProcessors);
    MY_LOGI("\tvBufMap %zu:", vBufMap.size());

    for(auto const &iter: vBufMap)
    {
        MY_LOGI("\t\t[%p] StreamId:%#" PRIx64 " ", iter.first, iter.second->getStreamId());
    }

    return;
}

