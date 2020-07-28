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
 * @file IDualFeatureRequest.cpp
 * @brief Base interface class for feature requests
 */
#define LOG_TAG "DualFeature/IDualFeatureRequest"
// Standard C header file
// Android system/core header file
#include <utils/RWLock.h>
// mtkcam custom header file
// mtkcam global header file
#include <mtkcam/utils/std/Log.h>
// Module header file
// Local header file
#include "DualFeatureRequest.h"



#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature{
namespace NSDualFeature {
using namespace android;

IDualFeatureRequest::
IDualFeatureRequest(
    MUINT32 _reqNo,
    PFN_IREQ_FINISH_CALLBACK_T _cb,
    MVOID* _tag
)
: EffectRequest(_reqNo, onRequestFinishCallback, _tag)
, mpfnCallback(_cb)
{
}

MBOOL
IDualFeatureRequest::
pushRequestImageBuffer(
    const NodeBufferSetting& setting,
    sp<IImageBuffer>& pImgBuf
)
{
    RWLock::AutoWLock _l(mFrameLock[setting.ioType]);
    if(this->vInputFrameInfo.indexOfKey(setting.bufferID) >= 0 ||
        this->vOutputFrameInfo.indexOfKey(setting.bufferID) >= 0)
    {
        MY_LOGE("Already exit the same bufferID imagebuffer frameInfo, bufferID=%d", setting.bufferID);
        return MFALSE;
    }
    sp<EffectFrameInfo> pEffectFrame = new EffectFrameInfo(getRequestNo(), setting.bufferID);
    pEffectFrame->setFrameBuffer(pImgBuf);
    if(setting.ioType == eBUFFER_IOTYPE_INPUT)
        this->vInputFrameInfo.add(setting.bufferID, pEffectFrame);
    else
        this->vOutputFrameInfo.add(setting.bufferID, pEffectFrame);

    mBufBIDToIOType.add(setting.bufferID, setting.ioType);
    return MTRUE;
}

MBOOL
IDualFeatureRequest::
pushRequestMetadata(
    const NodeBufferSetting& setting,
    IMetadata* pMetaBuf
)
{
    RWLock::AutoWLock _l(mFrameLock[setting.ioType]);
    if(this->vInputFrameInfo.indexOfKey(setting.bufferID) >= 0 ||
        this->vOutputFrameInfo.indexOfKey(setting.bufferID) >= 0)
    {
        MY_LOGE("Already exit the same bufferID metadata frameInfo, bufferID=%d", setting.bufferID);
        return MFALSE;
    }
    sp<EffectFrameInfo> pEffectFrame = new EffectFrameInfo(getRequestNo(), setting.bufferID);
    sp<EffectParameter> pEffParam = new EffectParameter();
    pEffParam->setPtr(DUAL_REQ_META_KEY, (void*)pMetaBuf);
    pEffectFrame->setFrameParameter(pEffParam);
    //
    if(setting.ioType == eBUFFER_IOTYPE_INPUT)
        this->vInputFrameInfo.add(setting.bufferID, pEffectFrame);
    else
        this->vOutputFrameInfo.add(setting.bufferID, pEffectFrame);
    mMetaBIDToIOType.add(setting.bufferID, setting.ioType);
    return MTRUE;
}

MBOOL
IDualFeatureRequest::
getRequestImageBuffer(
    const NodeBufferSetting& setting,
    IImageBuffer*& rpImgBuf
)
{

    RWLock::AutoRLock _l(mFrameLock[setting.ioType]);
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
IDualFeatureRequest::
getRequestMetadata(
    const NodeBufferSetting& setting,
    IMetadata*& rpMetaBuf
)
{
    RWLock::AutoRLock _l(mFrameLock[setting.ioType]);
    auto& vFrameInfo = (setting.ioType == eBUFFER_IOTYPE_INPUT) ? vInputFrameInfo : vOutputFrameInfo;
    if(vFrameInfo.indexOfKey(setting.bufferID)<0)
        return MFALSE;

    sp<EffectFrameInfo> pEffectFrame = vFrameInfo.valueFor(setting.bufferID);
    sp<EffectParameter> pEffParam = pEffectFrame->getFrameParameter();
    rpMetaBuf = reinterpret_cast<IMetadata*>(pEffParam->getPtr(DUAL_REQ_META_KEY));
    return MTRUE;
}

MBOOL
IDualFeatureRequest::
popRequestImageBuffer(
    const NodeBufferSetting& setting
)
{
    RWLock::AutoWLock _l(mFrameLock[setting.ioType]);
    auto& vFrameInfo = (setting.ioType == eBUFFER_IOTYPE_INPUT) ? vInputFrameInfo : vOutputFrameInfo;
    ssize_t index = vFrameInfo.indexOfKey(setting.bufferID);
    if(index<0)
        return MFALSE;
    vFrameInfo.removeItemsAt(index);
    return MTRUE;
}

MBOOL
IDualFeatureRequest::
popRequestMetadata(
    const NodeBufferSetting& setting
)
{
    RWLock::AutoWLock _l(mFrameLock[setting.ioType]);
    auto& vFrameInfo = (setting.ioType == eBUFFER_IOTYPE_INPUT) ? vInputFrameInfo : vOutputFrameInfo;
    ssize_t index = vFrameInfo.indexOfKey(setting.bufferID);
    if(index<0)
        return MFALSE;
    vFrameInfo.removeItemsAt(index);
    return MTRUE;
}

MBOOL
IDualFeatureRequest::
startTimer()
{
    mStart = system_clock::now();
    return MTRUE;
}

MBOOL
IDualFeatureRequest::
stopTimer()
{
    mEnd = system_clock::now();
    return MFALSE;
}

MUINT32
IDualFeatureRequest::
getElapsedTime()
{
    std::chrono::duration<double> elap = mEnd - mStart;
    return elap.count()*1000;
}

MVOID
IDualFeatureRequest::
launchFinishCallback(ResultState state)
{
    sp<IDualFeatureRequest> pReq = this;
    // launch callback
    this->mpfnCallback(this->mpTag, state, pReq);
}


MBOOL
IDualFeatureRequest::
isRequestBuffer(
    RequestBufferID bufferID
)
{
    if(mBufBIDToIOType.indexOfKey(bufferID) >= 0 ||
        mMetaBIDToIOType.indexOfKey(bufferID) >= 0)
        return MTRUE;
    else
        return MFALSE;
}

MBOOL
IDualFeatureRequest::
getBufferIOType(
    RequestBufferID bufferID,
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

MBOOL
IDualFeatureRequest::
getBufferType(
    RequestBufferID bufferID,
    BufferType& rBufType
)
{
    ssize_t index = mBufBIDToIOType.indexOfKey(bufferID);
    if(mBufBIDToIOType.indexOfKey(bufferID) >= 0)
    {
        rBufType = eBUFFER_TYPE_IMAGE;
        return MTRUE;
    }
    else if(mMetaBIDToIOType.indexOfKey(bufferID) >= 0)
    {
        rBufType = eBUFFER_TYPE_META;
        return MTRUE;
    }

    return MFALSE;
}

MBOOL
IDualFeatureRequest::
copyTo(
    IDualFeatureRequest* pRequest,
    PFN_ID_MAPPER_T keyMapper
)
{
    MBOOL bRet = MTRUE;
    for(ssize_t index=0;index<this->vInputFrameInfo.size();++index)
    {
        RequestBufferID bufferID_req = (RequestBufferID)this->vInputFrameInfo.keyAt(index);
        MINT32 mappedBufID = keyMapper(bufferID_req);
        if(mappedBufID < 0)
            continue;
        sp<EffectFrameInfo> pFrame = new EffectFrameInfo(*this->vInputFrameInfo.valueAt(index).get());
        pFrame->setFrameNo(mappedBufID);
        pRequest->vInputFrameInfo.add(mappedBufID, pFrame);
        //
        if(mBufBIDToIOType.indexOfKey(bufferID_req) >= 0)
            pRequest->mBufBIDToIOType.add(mappedBufID, eBUFFER_IOTYPE_INPUT);
        else
            pRequest->mMetaBIDToIOType.add(mappedBufID, eBUFFER_IOTYPE_INPUT);


    }

    for(ssize_t index=0;index<this->vOutputFrameInfo.size();++index)
    {
        RequestBufferID bufferID_req = (RequestBufferID)this->vOutputFrameInfo.keyAt(index);
        MINT32 mappedBufID = keyMapper(bufferID_req);
        if(mappedBufID < 0)
            continue;
        sp<EffectFrameInfo> pFrame = new EffectFrameInfo(*this->vOutputFrameInfo.valueAt(index).get());
        pFrame->setFrameNo(mappedBufID);
        pRequest->vOutputFrameInfo.add(mappedBufID, pFrame);
        //
        if(mBufBIDToIOType.indexOfKey(bufferID_req) >= 0)
            pRequest->mBufBIDToIOType.add(mappedBufID, eBUFFER_IOTYPE_OUTPUT);
        else
            pRequest->mMetaBIDToIOType.add(mappedBufID, eBUFFER_IOTYPE_OUTPUT);
    }

    return MTRUE;
}

MVOID
IDualFeatureRequest::
showContent()
{
    MY_LOGD("reqID=%d input/output frame size=%d/%d", this->getRequestNo(),
            this->vInputFrameInfo.size(), this->vOutputFrameInfo.size());
    for(ssize_t index=0;index<this->vInputFrameInfo.size();++index)
    {
        auto key = this->vInputFrameInfo.keyAt(index);
        MY_LOGD("reqID=%d index=%d input bufferID=%d", this->getRequestNo(), index, key);
    }
    for(ssize_t index=0;index<this->vOutputFrameInfo.size();++index)
    {
        auto key = this->vOutputFrameInfo.keyAt(index);
        MY_LOGD("reqID=%d index=%d output bufferID=%d", this->getRequestNo(), index, key);
    }
}

};
};
};
