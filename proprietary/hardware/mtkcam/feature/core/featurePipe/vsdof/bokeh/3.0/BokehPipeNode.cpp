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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
//
#define PIPE_MODULE_TAG "BokehPipe"
#define PIPE_CLASS_TAG "BokehPipeNode"
//
#include <featurePipe/core/include/PipeLog.h>
#include "BokehPipeNode.h"
//
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
//
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NS3Av3;
//************************************************************************
//
//************************************************************************
const char*
BokehPipeDataHandler::
ID2Name(DataID id)
{
    switch(id)
    {
        case ID_INVALID:
            return "ID_INVALID";
        case ID_ROOT_ENQUE:
            return "ID_ROOT_ENQUE";
        case P2B_OUT:
            return "P2B_OUT";
        case VENDOR_TO_MDP:
            return "VENDOR_TO_MDP";
        case P2B_ERROR:
            return "P2B_ERROR";
    }
    return "UNKNOWN";
}
//************************************************************************
//
//************************************************************************
BokehPipeDataHandler::
~BokehPipeDataHandler()
{
}
//************************************************************************
//
//************************************************************************
BokehPipeNode::
BokehPipeNode(const char *name, Graph_T *graph)
  : CamThreadNode(name)
{
    miDumpBufSize = ::property_get_int32("vendor.dualfeature.dump.size", 0);
    miDumpStartIdx = ::property_get_int32("vendor.dualfeature.dump.start", 0);
    MY_LOGD("miDumpBufSize=%d miDumpStartIdx=%d", miDumpBufSize, miDumpStartIdx);
}
//************************************************************************
//
//************************************************************************
BokehPipeNode::
~BokehPipeNode()
{
}
//************************************************************************
//
//************************************************************************
void
BokehPipeNode::
enableDumpImage(MBOOL flag)
{
    mbDumpImageBuffer = flag;
}
//************************************************************************
//
//************************************************************************
MBOOL
BokehPipeNode::
onInit()
{
    miPipeLogEnable = StereoSettingProvider::isLogEnabled(PERPERTY_BOKEH_NODE_LOG);
    mbDumpImageBuffer = getPropValue();
    mbProfileLog = StereoSettingProvider::isProfileLogEnabled();
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MBOOL
BokehPipeNode::
createBufferPool(
    android::sp<ImageBufferPool> &pPool,
    MUINT32 width,
    MUINT32 height,
    NSCam::EImageFormat format,
    MUINT32 bufCount,
    const char* caller,
    MUINT32 bufUsage)
{
    FUNC_START;
    MBOOL ret = MFALSE;
    pPool = ImageBufferPool::create(caller, width, height, format, bufUsage);
    if(pPool == nullptr)
    {
        ret = MFALSE;
        MY_LOGE("Create [%s] failed.", caller);
        goto lbExit;
    }
    for(MUINT32 i=0;i<bufCount;++i)
    {
        if(!pPool->allocate())
        {
            ret = MFALSE;
            MY_LOGE("Allocate [%s] working buffer failed.", caller);
            goto lbExit;
        }
    }
    ret = MTRUE;
    FUNC_END;
lbExit:
    return ret;
}
//************************************************************************
//
//************************************************************************
MVOID
BokehPipeNode::
setImageBufferValue(SmartImageBuffer& buffer,
                MINT32 width,
                MINT32 height,
                MINT32 value)
{
    MUINT8* data = (MUINT8*)buffer->mImageBuffer->getBufVA(0);
    memset(data, value, sizeof(MUINT8) * width * height);
}
//************************************************************************
//
//************************************************************************
MBOOL
BokehPipeNode::
getInputBufferFromRequest(
    const EffectRequestPtr request,
    BokehEffectRequestBufferType type,
    sp<IImageBuffer>& frame
)
{
    ssize_t keyIndex = -1;
    sp<EffectFrameInfo> frameInfo = nullptr;
    keyIndex = request->vInputFrameInfo.indexOfKey(type);
    if(keyIndex>=0)
    {
        frameInfo = request->vInputFrameInfo.valueAt(keyIndex);
        frameInfo->getFrameBuffer(frame);
    }
    else
    {
        MY_LOGE("Get buffer fail. bufferType(%d)", type);
        return MFALSE;
    }
    //
    if(frame.get() == nullptr)
    {
        MY_LOGE("Buffer is invalid. bufferType(%d)", type);
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BokehPipeNode::
getOutputBufferFromRequest(
    const EffectRequestPtr request,
    BokehEffectRequestBufferType type,
    sp<IImageBuffer>& frame
)
{
    ssize_t keyIndex = -1;
    sp<EffectFrameInfo> frameInfo = nullptr;
    keyIndex = request->vOutputFrameInfo.indexOfKey(type);
    if(keyIndex>=0)
    {
        frameInfo = request->vOutputFrameInfo.valueAt(keyIndex);
        frameInfo->getFrameBuffer(frame);
    }
    else
    {
        MY_LOGE("Get buffer fail. bufferType(%d)", type);
        return MFALSE;
    }
    //
    if(frame.get() == nullptr)
    {
        MY_LOGE("Buffer is invalid. bufferType(%d)", type);
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BokehPipeNode::
getInputMetadataFromRequest(
    const EffectRequestPtr request,
    BokehEffectRequestBufferType type,
    IMetadata*& rpMetaBuf
)
{
    ssize_t keyIndex = -1;
    sp<EffectFrameInfo> frameInfo = nullptr;
    sp<EffectParameter> pEffParam = nullptr;
    keyIndex = request->vInputFrameInfo.indexOfKey(type);
    if(keyIndex>=0)
    {
        frameInfo = request->vInputFrameInfo.valueAt(keyIndex);
        pEffParam = frameInfo->getFrameParameter();
        rpMetaBuf = reinterpret_cast<IMetadata*>(pEffParam->getPtr(BOKEH_META_KEY_STRING));
    }
    else
    {
        MY_LOGE("Get input metadata fail. bufferType(%d)", type);
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BokehPipeNode::
getOutputMetadataFromRequest(
    const EffectRequestPtr request,
    BokehEffectRequestBufferType type,
    IMetadata*& rpMetaBuf
)
{
    ssize_t keyIndex = -1;
    sp<EffectFrameInfo> frameInfo = nullptr;
    sp<EffectParameter> pEffParam = nullptr;
    keyIndex = request->vOutputFrameInfo.indexOfKey(type);
    if(keyIndex>=0)
    {
        frameInfo = request->vOutputFrameInfo.valueAt(keyIndex);
        pEffParam = frameInfo->getFrameParameter();
        rpMetaBuf = reinterpret_cast<IMetadata*>(pEffParam->getPtr(BOKEH_META_KEY_STRING));
    }
    else
    {
        MY_LOGE("Get output metadata fail. bufferType(%d)", type);
        return MFALSE;
    }
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
TuningParam
BokehPipeNode::
applyISPTuning(
    const char* name,
    SmartTuningBuffer& targetTuningBuf,
    const ISPTuningConfig& ispConfig,
    MBOOL updateHalMeta
)
{
    MY_LOGD("+, [%s] reqID=%d bIsResized=%d", name, ispConfig.reqNo, ispConfig.bInputResizeRaw);

    NS3Av3::TuningParam tuningParam = {NULL, NULL};
    tuningParam.pRegBuf = reinterpret_cast<void*>(targetTuningBuf->mpVA);

    MetaSet_T inMetaSet;
    IMetadata* pMeta_InApp  = ispConfig.pInAppMeta;
    IMetadata* pMeta_InHal  = ispConfig.pInHalMeta;

    inMetaSet.appMeta = *pMeta_InApp;
    inMetaSet.halMeta = *pMeta_InHal;

    MUINT8 profile = -1;
    if(tryGetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, profile)){
        MY_LOGD("Profile:%d", profile);
    }else{
        MY_LOGW("Failed getting profile!");
    }

    // USE resize raw-->set PGN 0
    if(ispConfig.bInputResizeRaw)
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
    else
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);

#ifndef GTEST
    MetaSet_T resultMeta;
    ispConfig.p3AHAL->setIsp(0, inMetaSet, &tuningParam, &resultMeta);

    if(updateHalMeta){
        // write ISP resultMeta to input hal Meta
        (*pMeta_InHal) += resultMeta.halMeta;
    }
#else
    MY_LOGW("GTEST do nothing");
#endif

    MY_LOGD("-, [%s] reqID=%d", name, ispConfig.reqNo);
    return tuningParam;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BokehPipeNode::
checkReqIndexToDump(const EffectRequestPtr request)
{
    MBOOL needToDump = MTRUE;
#ifndef GTEST
    MUINT iReqIdx = request->getRequestNo();
    if(iReqIdx < miDumpStartIdx || iReqIdx >= miDumpStartIdx + miDumpBufSize)
    {
        needToDump = MFALSE;
    }
#endif
    MY_LOGD("iReqIdx:%d",iReqIdx);
    return needToDump;
}
