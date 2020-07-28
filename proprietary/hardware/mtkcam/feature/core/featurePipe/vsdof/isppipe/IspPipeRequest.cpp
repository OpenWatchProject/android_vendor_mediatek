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

/**
 * @file ISPPipeRequest.cpp
 * @brief Request inside the ThirdParty ISPPipe
 */

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "IspPipeRequest.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

const char* fnStateToName(PipeResultState state)
{
#define MAKE_NAME_CASE(name) \
case name: return #name;
    switch(state)
    {
        MAKE_NAME_CASE(eRESULT_SUCCESS);
        MAKE_NAME_CASE(eRESULT_FAILURE);
        MAKE_NAME_CASE(eRESULT_FLUSH);
        default:
            return "unknown";
    }
#undef MAKE_NAME_CASE
}

#define ISPPIPE_META_KEY_STRING "isppipe_meta"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

sp<IIspPipeRequest>
IIspPipeRequest::
createInstance(
    MUINT32 reqID,
    PFN_PIPE_FINISH_CALLBACK_T callback,
    MVOID* tag
)
{
    return new IspPipeRequest(reqID, callback, tag);
}

IIspPipeRequest::
IIspPipeRequest(
    MUINT32 reqID,
    PFN_CALLBACK_T callback,
    MVOID* tag
)
: EffectRequest(reqID, callback, tag)
{

}

IspPipeRequest::
IspPipeRequest(
    MUINT32 reqID,
    PFN_PIPE_FINISH_CALLBACK_T callback,
    MVOID* tag
)
: IIspPipeRequest(reqID, onRequestFinishCallback, tag)
, mpfFinishCallback(callback)
{

}

MVOID
IspPipeRequest::
onRequestFinishCallback(
    MVOID* tag,
    String8 status,
    sp<EffectRequest>& request
)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IIspPipeRequest Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspPipeRequest::
pushRequestImageBuffer(
    const BufferSetting& setting,
    sp<IImageBuffer>& pImgBuf
)
{
    sp<EffectFrameInfo> pEffectFrame = new EffectFrameInfo(getRequestNo(), setting.bufferID);
    pEffectFrame->setFrameBuffer(pImgBuf);

    if(setting.ioType == eBUFFER_IOTYPE_INPUT)
        this->vInputFrameInfo.add(setting.bufferID, pEffectFrame);
    else
        this->vOutputFrameInfo.add(setting.bufferID, pEffectFrame);

    // add io map
    mBufBIDToIOType.add(setting.bufferID, setting.ioType);
    return MTRUE;
}

MBOOL
IspPipeRequest::
pushRequestMetadata(
    const BufferSetting& setting,
    IMetadata* pMetaBuf
)
{
    sp<EffectFrameInfo> pEffectFrame = new EffectFrameInfo(getRequestNo(), setting.bufferID);
    sp<EffectParameter> pEffParam = new EffectParameter();

    pEffParam->setPtr(ISPPIPE_META_KEY_STRING, (void*)pMetaBuf);
    pEffectFrame->setFrameParameter(pEffParam);

    if(setting.ioType == eBUFFER_IOTYPE_INPUT)
        this->vInputFrameInfo.add(setting.bufferID, pEffectFrame);
    else
        this->vOutputFrameInfo.add(setting.bufferID, pEffectFrame);

    // add io map
    mMetaBIDToIOType.add(setting.bufferID, setting.ioType);
    return MTRUE;
}

MBOOL
IspPipeRequest::
getRequestImageBuffer(
    const BufferSetting& setting,
    IImageBuffer*& rpImgBuf
)
{
    auto& vFrameInfo = (setting.ioType == eBUFFER_IOTYPE_INPUT) ? vInputFrameInfo : vOutputFrameInfo;
    if(vFrameInfo.indexOfKey(setting.bufferID)<0)
        return MFALSE;

    sp<EffectFrameInfo> pEffectFrame = vFrameInfo.valueFor(setting.bufferID);
    sp<IImageBuffer> pImgBuf;

    pEffectFrame->getFrameBuffer(pImgBuf);
    rpImgBuf = pImgBuf.get();
    return MTRUE;
}


MBOOL
IspPipeRequest::
getRequestMetadata(
    const BufferSetting& setting,
    IMetadata*& rpMetaBuf
)
{
    auto& vFrameInfo = (setting.ioType == eBUFFER_IOTYPE_INPUT) ? vInputFrameInfo : vOutputFrameInfo;
    if(vFrameInfo.indexOfKey(setting.bufferID)<0)
        return MFALSE;

    sp<EffectFrameInfo> pEffectFrame = vFrameInfo.valueFor(setting.bufferID);
    sp<EffectParameter> pEffParam = pEffectFrame->getFrameParameter();
    rpMetaBuf = reinterpret_cast<IMetadata*>(pEffParam->getPtr(ISPPIPE_META_KEY_STRING));
    return MTRUE;
}

MBOOL
IspPipeRequest::
popRequestImageBuffer(
    const BufferSetting& setting
)
{
    auto& vFrameInfo = (setting.ioType == eBUFFER_IOTYPE_INPUT) ? vInputFrameInfo : vOutputFrameInfo;
    ssize_t index = vFrameInfo.indexOfKey(setting.bufferID);
    if(index<0)
        return MFALSE;
    vFrameInfo.removeItemsAt(index);
    return MTRUE;
}

MBOOL
IspPipeRequest::
popRequestMetadata(
    const BufferSetting& setting
)
{
    auto& vFrameInfo = (setting.ioType == eBUFFER_IOTYPE_INPUT) ? vInputFrameInfo : vOutputFrameInfo;
    ssize_t index = vFrameInfo.indexOfKey(setting.bufferID);
    if(index<0)
        return MFALSE;
    vFrameInfo.removeItemsAt(index);
    return MTRUE;
}

MBOOL
IspPipeRequest::
startTimer()
{
    this->mTimer.start();
    return MTRUE;
}

MBOOL
IspPipeRequest::
stopTimer()
{
    this->mTimer.stop();
    return MTRUE;
}

MUINT32
IspPipeRequest::
getElapsedTime()
{
    return this->mTimer.getElapsed();
}

size_t
IspPipeRequest::
getOutputSize()
{
    return this->vOutputFrameInfo.size();
}

MVOID
IspPipeRequest::
setStopWatchCollection(sp<IStopWatchCollection> stopWatchCollection)
{
    mStopWatchCollection = stopWatchCollection;
}

sp<IStopWatchCollection>
IspPipeRequest::
getStopWatchCollection()
{
    if(mStopWatchCollection == nullptr)
    {
        MY_LOGW("mStopWatchCollection is nullptr");
    }
    return mStopWatchCollection;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeRequest Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspPipeRequest::
launchFinishCallback(PipeResultState state)
{
    sp<IIspPipeRequest> pReq = (IIspPipeRequest*) this;
    // launch callback
    this->mpfFinishCallback(this->mpTag, state, pReq);
}

MBOOL
IspPipeRequest::
checkAllOutputReady()
{
    size_t outBufSize = this->vOutputFrameInfo.size();
    // make sure all output frame are ready
    for(size_t index=0;index<outBufSize;++index)
    {
        if(!this->vOutputFrameInfo[index]->isFrameBufferReady())
        {
            PIPE_LOGD("reqID=%d Data not ready!! buffer key=%d",
                this->getRequestNo(), (IspPipeBufferID)this->vOutputFrameInfo.keyAt(index));
            return MFALSE;
        }
    }
    PIPE_LOGD("reqID=%d Data all ready!!", getRequestNo());
    return MTRUE;
}

MBOOL
IspPipeRequest::
setOutputBufferReady(const IspPipeBufferID& bufferID)
{
    ssize_t index = this->vOutputFrameInfo.indexOfKey(bufferID);
    if(index >= 0)
    {
        sp<EffectFrameInfo> pFrame = this->vOutputFrameInfo.valueAt(index);
        pFrame->setFrameReady(true);
        return MTRUE;
    }
    return MFALSE;
}

MBOOL
IspPipeRequest::
getOutputBufferReady(const IspPipeBufferID& bufferID)
{
    ssize_t index = this->vOutputFrameInfo.indexOfKey(bufferID);
    if(index >= 0)
    {
        sp<EffectFrameInfo> pFrame = this->vOutputFrameInfo.valueAt(index);
        return pFrame->isFrameBufferReady();
    }
    return MFALSE;
}

MBOOL
IspPipeRequest::
init(sp<PipeBufferHandler> pHandler, sp<IspPipeOption> pPipeOption)
{
    // set buffer handler
    mpBufferHandler = pHandler;
    //set request attributes
    MBOOL bRet = parseReqAttrs(pPipeOption);
    // config request to the handler
    bRet &= pHandler->configRequest(this);
    return bRet;
}

MBOOL
IspPipeRequest::
parseReqAttrs(sp<IspPipeOption> pPipeOption)
{
    // decide EIS option
    if(!pPipeOption->mbEnableEIS1d2)
        mReqAttrs.eisOption = PIPE_EIS_OFF;
    else
    {
        IMetadata* pInAppMeta = nullptr;
        MBOOL bRet = this->getRequestMetadata({.bufferID=BID_META_IN_APP,
                                            .ioType=eBUFFER_IOTYPE_INPUT}, pInAppMeta);
        if(bRet || isEISOn(pInAppMeta))
            mReqAttrs.eisOption = PIPE_EIS_ON;
        else
            mReqAttrs.eisOption = PIPE_EIS_OFF;
    }
    // check request type
    if(this->isRequestBuffer(BID_PIPE_OUT_BOKEH_YUV) ||
        this->isRequestBuffer(BID_PIPE_OUT_DEPTH))
    {
        // TODO:
        // review whether this change has side effect or not
        mReqAttrs.reqType = PIPE_REQ_CAPTURE;
    }
    else if(this->isRequestBuffer(BID_PIPE_OUT_PV_YUV0) ||
            this->isRequestBuffer(BID_PIPE_OUT_PV_YUV1))
    {
        mReqAttrs.reqType = PIPE_REQ_PREVIEW;
    }
    else if(this->isRequestBuffer(BID_PIPE_OUT_PV_FD))
    {
        mReqAttrs.reqType = PIPE_REQ_FD;
    }
    else
    {
        mReqAttrs.reqType = PIPE_REQ_META;
    }

    PIPE_LOGD("reqID=%d  reqType=%d", getRequestNo(), mReqAttrs.reqType);

    return MTRUE;
}

MBOOL
IspPipeRequest::
isRequestBuffer(
    IspPipeBufferID bufferID
)
{
    if(mBufBIDToIOType.indexOfKey(bufferID) >= 0 ||
        mMetaBIDToIOType.indexOfKey(bufferID) >= 0)
        return MTRUE;
    else
        return MFALSE;
}

MBOOL
IspPipeRequest::
getBufferIOType(
    IspPipeBufferID bufferID,
    BufferIOType& rIOType
)
{
    ssize_t index = mBufBIDToIOType.indexOfKey(bufferID);
    if(index >= 0)
    {
        rIOType = mBufBIDToIOType.valueAt(index);
        return MTRUE;
    }
    else if((index = mMetaBIDToIOType.indexOfKey(bufferID)) >=0)
    {
        rIOType = mMetaBIDToIOType.valueAt(index);
        return MTRUE;
    }
    else
        return MFALSE;
}

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
