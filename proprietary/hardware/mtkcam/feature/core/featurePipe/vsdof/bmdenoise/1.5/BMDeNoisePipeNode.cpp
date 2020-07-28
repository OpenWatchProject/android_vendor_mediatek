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


// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

// Module header file

// Local header file
#include "BMDeNoisePipeNode.h"

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSIoPipe::NSSImager;
using namespace NS3Av3;
/*******************************************************************************
* Global Define
********************************************************************************/
#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "BaseNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

/*******************************************************************************
* External Function
********************************************************************************/



/*******************************************************************************
* Enum Define
********************************************************************************/




/*******************************************************************************
* Structure Define
********************************************************************************/






//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BMDeNoisePipeNode::
BMDeNoisePipeNode(const char *name, Graph_T *graph)
  : CamThreadNode(name)
{
    miDumpStartIdx = ::property_get_int32("vendor.bmdenoise.pipe.dump.start", 0);
    miDumpBufSize = ::property_get_int32("vendor.bmdenoise.pipe.dump.size", 0);
    miTuningDump = ::property_get_int32("vendor.bmdenoise.tuning.dump", 0);

    MY_LOGD("miDumpStartIdx:%d, miDumpBufSize:%d, miTuningDump:%d",
        miDumpStartIdx, miDumpBufSize, miTuningDump
    );

    #ifdef BIT_TRUE
    MY_LOGD("bit-true mode");
    #endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BMDeNoisePipeDataHandler Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char*
BMDeNoisePipeDataHandler::
ID2Name(DataID id)
{
    return "UNKNOWN";
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BMDeNoisePipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
BMDeNoisePipeNode::
enableDumpImage(MBOOL flag)
{
    mbDumpImageBuffer = flag;
}
MBOOL
BMDeNoisePipeNode::
onInit()
{
    mbDebugLog = StereoSettingProvider::isLogEnabled(PERPERTY_BMDENOISE_NODE_LOG);
    mbDumpImageBuffer = getPropValue();
    mbProfileLog = StereoSettingProvider::isProfileLogEnabled();

    MY_LOGD("%d,%d,%d",
        mbDebugLog,
        mbDumpImageBuffer,
        mbProfileLog
    );

    if(StereoSettingProvider::getStereoSensorIndex(mSensorIdx_Main1, mSensorIdx_Main2) != MTRUE){
        MY_LOGE("Can't get sensor id from StereoSettingProvider!");
        return MFALSE;
    }

    if(!prepareModuleSettings()){
        MY_LOGE("prepareModuleSettings Error! Please check the error msg above!");
    }

    return MTRUE;
}

MBOOL
BMDeNoisePipeNode::
handleDump(IImageBuffer* pBuf, BMDeNoiseBufferID BID, MUINT32 iReqIdx)
{
    MY_LOGD("BID:%d +", BID);

    if(!shouldDumpForTuning(BID)){
        return MFALSE;
    }

    if(pBuf == nullptr){
        MY_LOGE("pBuf is nullptr!");
        return MFALSE;
    }

    char filepath[1024];
    snprintf(filepath, 1024, "/sdcard/bmdenoise/%d/%s", iReqIdx, getName());

    // make path
    MY_LOGD("makePath: %s", filepath);
    makePath(filepath, 0660);

    const char* fileName = getBIDName(BID);
    const char* filePostfix = getBIDPostfix(BID);

    char writepath[1024];
    snprintf(writepath,
        1024, "%s/%s_%dx%d_%d.%s",
        filepath, fileName,
        pBuf->getImgSize().w, pBuf->getImgSize().h, pBuf->getBufStridesInBytes(0),
        filePostfix
    );

    pBuf->saveToFile(writepath);

    MY_LOGD("BID:%d -", BID);
    return MTRUE;
}

MBOOL
BMDeNoisePipeNode::
handleDumpWithExtName(IImageBuffer* pBuf, BMDeNoiseBufferID BID, MUINT32 iReqIdx, string& extName)
{
    MY_LOGD("BID:%d +", BID);

    if(!shouldDumpForTuning(BID)){
        return MFALSE;
    }

    if(pBuf == nullptr){
        MY_LOGE("pBuf is nullptr!");
        return MFALSE;
    }

    char filepath[1024];
    snprintf(filepath, 1024, "/sdcard/bmdenoise/%d/%s", iReqIdx, getName());

    // make path
    MY_LOGD("makePath: %s", filepath);
    makePath(filepath, 0660);

    const char* fileName = getBIDName(BID);
    const char* filePostfix = getBIDPostfix(BID);

    char writepath[1024];
    snprintf(writepath,
        1024, "%s/%s_%s.%s",
        filepath, fileName, extName.c_str(),
        filePostfix
    );

    pBuf->saveToFile(writepath);

    MY_LOGD("BID:%d -", BID);
    return MTRUE;
}

MBOOL
BMDeNoisePipeNode::
onDump(DataID id, ImgInfoMapPtr &data, const char* fileName, const char* postfix)
{
    MY_LOGD("dataID:%d +", id);

    sp<PipeRequest> pPipeReq = data->getRequestPtr();
    MUINT iReqIdx = pPipeReq->getRequestNo();

    if(!shouldDumpRequest(pPipeReq) || !data->shouldDump()){
        return MFALSE;
    }

    // IImageBuffer
    {
        IImageBufferSet set = data->getIImageBufferSet();
        for(auto const &e : set) {
            const BMDeNoiseBufferID& BID = e.first;
            const sp<IImageBuffer>& spBuf = e.second;

            string extName = "";
            if(data->getExtBufName(BID, extName)){
                handleDumpWithExtName(spBuf.get(), BID, iReqIdx, extName);
            }else{
                handleDump(spBuf.get(), BID, iReqIdx);
            }
        }
    }

    // SmartImageBuffer
    {
        SmartImageBufferSet set = data->getSmartBufferSet();
        for(size_t i=0 ; i<set.size() ; ++i){
            const BMDeNoiseBufferID& BID = set.keyAt(i);
            const SmartImageBuffer spBuf = set.valueAt(i);
            string extName = "";
            if(data->getExtBufName(BID, extName)){
                handleDumpWithExtName(spBuf->mImageBuffer.get(), BID, iReqIdx, extName);
            }else{
                handleDump(spBuf->mImageBuffer.get(), BID, iReqIdx);
            }
        }
    }

    // GraphicBuffer
    {
        GraphicImageBufferSet set = data->getGraphicBufferSet();
        for(size_t i=0 ; i<set.size() ; ++i){
            const BMDeNoiseBufferID& BID = set.keyAt(i);
            const SmartGraphicBuffer spBuf = set.valueAt(i);
            string extName = "";
            if(data->getExtBufName(BID, extName)){
                handleDumpWithExtName(spBuf->mImageBuffer.get(), BID, iReqIdx, extName);
            }else{
                handleDump(spBuf->mImageBuffer.get(), BID, iReqIdx);
            }
        }
    }

    // MfllBuffer
    {
        MfllImageBufferSet set = data->getMfllBufferSet();
        for(auto const &e : set) {
            const BMDeNoiseBufferID& BID = e.first;
            const sp<mfll::IMfllImageBuffer>& spBuf = e.second;

            IImageBuffer* pBuf = static_cast<IImageBuffer*>(spBuf->getImageBuffer());
            string extName = "";
            if(data->getExtBufName(BID, extName)){
                handleDumpWithExtName(pBuf, BID, iReqIdx, extName);
            }else{
                handleDump(pBuf, BID, iReqIdx);
            }
        }
    }

    MY_LOGD("dataID:%d -", id);
    return MTRUE;
}

MBOOL
BMDeNoisePipeNode::
handleData(DataID id, PipeRequestPtr pReq)
{
    if(pReq != nullptr){
        VSDOF_LOGD("handleData(request): %d", pReq->getRequestNo());
    }
    return CamThreadNode<BMDeNoisePipeDataHandler>::handleData(id, pReq);
}

MBOOL
BMDeNoisePipeNode::
handleData(DataID id, EffectRequestPtr pReq)
{
    if(pReq != nullptr){
        VSDOF_LOGD("handleData(request): %d", pReq->getRequestNo());
    }
    return CamThreadNode<BMDeNoisePipeDataHandler>::handleData(id, pReq);
}

MBOOL
BMDeNoisePipeNode::
handleData(DataID id, ImgInfoMapPtr data)
{
    if(data != nullptr){
        VSDOF_LOGD("handleData(imgInfo): regId:%d", data->getRequestPtr()->getRequestNo());
    }
    this->onDump(id, data);
    return CamThreadNode<BMDeNoisePipeDataHandler>::handleData(id, data);
}

MBOOL
BMDeNoisePipeNode::
handleData(DataID id, MINT32 data)
{
    VSDOF_LOGD("handleData(MINT32): %d", data);
    return CamThreadNode<BMDeNoisePipeDataHandler>::handleData(id, data);
}

MBOOL
BMDeNoisePipeNode::
handleData(DataID id, Vector<MINT32>& data)
{
    VSDOF_LOGD("handleData(Vector<MINT32>): data.size:%d", data.size());
    return CamThreadNode<BMDeNoisePipeDataHandler>::handleData(id, data);
}

MBOOL
BMDeNoisePipeNode::
handleData(DataID id, Vector<BM_BSS_RESULT>& data)
{
    VSDOF_LOGD("handleData(Vector<BM_BSS_RESULT>): data.size:%d", data.size());
    return CamThreadNode<BMDeNoisePipeDataHandler>::handleData(id, data);
}

MVOID
BMDeNoisePipeNode::
dumpRequestData(BMDeNoiseBufferID BID, PipeRequestPtr pReq)
{
    VSDOF_LOGD("%d", BID);
    if(shouldDumpRequest(pReq)){
        if( pReq->getImageBuffer(BID) != nullptr){
            handleDump(pReq->getImageBuffer(BID).get(), BID, pReq->getRequestNo());
        }else{
            MY_LOGE("no data(%d)(%s) in req!", BID, getBIDName(BID));
        }
    }
}

MVOID
BMDeNoisePipeNode::
setP2Operator(sp<P2Operator> spP2Op)
{
    VSDOF_LOGD("setP2Operator: %p", spP2Op.get());
    mwpP2Operator = spP2Op;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BMDeNoisePipeNode Protect Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char*
BMDeNoisePipeNode::
getBIDName(BMDeNoiseBufferID BID)
{
    #define MAKE_NAME_CASE(name) \
        case name: return #name;

    switch(BID)
    {
        MAKE_NAME_CASE(BID_INPUT_FSRAW_1);
        MAKE_NAME_CASE(BID_INPUT_FSRAW_2);
        MAKE_NAME_CASE(BID_INPUT_RSRAW_1);
        MAKE_NAME_CASE(BID_INPUT_RSRAW_2);
        MAKE_NAME_CASE(BID_PRE_PROCESS_OUT_W_1);
        MAKE_NAME_CASE(BID_PRE_PROCESS_OUT_W_2);
        MAKE_NAME_CASE(BID_PRE_PROCESS_OUT_MFBO_FINAL_1);
        MAKE_NAME_CASE(BID_PRE_PROCESS_OUT_MFBO_FINAL_2);
        MAKE_NAME_CASE(BID_DENOISE_IN_MFBO_FINAL_1);
        MAKE_NAME_CASE(BID_DENOISE_IN_MFBO_FINAL_2);
        MAKE_NAME_CASE(BID_DENOISE_IN_W_1);
        MAKE_NAME_CASE(BID_DENOISE_IN_DISPARITY_MAP_1);
        MAKE_NAME_CASE(BID_DENOISE_HAL_OUT);
        MAKE_NAME_CASE(BID_DENOISE_HAL_OUT_ROT_BACK);
        MAKE_NAME_CASE(BID_DENOISE_FINAL_RESULT);
        MAKE_NAME_CASE(BID_THUMB_YUV);
        MAKE_NAME_CASE(BID_JPEG_YUV);
        MAKE_NAME_CASE(BID_DENOISE_AND_SWNR_OUT);
        MAKE_NAME_CASE(BID_DENOISE_AND_MNR_OUT);
        MAKE_NAME_CASE(BID_DENOISE_3RD_PARTY_OUT);
        MAKE_NAME_CASE(BID_PRE_PROCESS_OUT_1);
        MAKE_NAME_CASE(BID_PRE_PROCESS_OUT_2);
        MAKE_NAME_CASE(BID_PRE_PROCESS_OUT_RRZO);
        MAKE_NAME_CASE(BID_IMG3O);
        MAKE_NAME_CASE(BID_SWNR_IN_YV12);
        MAKE_NAME_CASE(BID_SWNR_OUT_YV12);
        MAKE_NAME_CASE(BID_PRE_PROCESS_OUT_YUV);
        MAKE_NAME_CASE(BID_P2AFM_OUT_FE1BO);
        MAKE_NAME_CASE(BID_P2AFM_OUT_FE2BO);
        MAKE_NAME_CASE(BID_P2AFM_OUT_FE1CO);
        MAKE_NAME_CASE(BID_P2AFM_OUT_FE2CO);
        MAKE_NAME_CASE(BID_P2AFM_OUT_FMBO_LR);
        MAKE_NAME_CASE(BID_P2AFM_OUT_FMBO_RL);
        MAKE_NAME_CASE(BID_P2AFM_OUT_FMCO_LR);
        MAKE_NAME_CASE(BID_P2AFM_OUT_FMCO_RL);
        MAKE_NAME_CASE(BID_P2AFM_OUT_CC_IN1);
        MAKE_NAME_CASE(BID_P2AFM_OUT_CC_IN2);
        MAKE_NAME_CASE(BID_P2AFM_OUT_RECT_IN1_CAP);
        MAKE_NAME_CASE(BID_N3D_OUT_MV_Y);
        MAKE_NAME_CASE(BID_N3D_OUT_SV_Y);
        MAKE_NAME_CASE(BID_N3D_OUT_MASK_M);
        MAKE_NAME_CASE(BID_N3D_OUT_MASK_S);
        MAKE_NAME_CASE(BID_N3D_OUT_WARPING_MATRIX);
        MAKE_NAME_CASE(BID_DPE_OUT_DMP_L);
        MAKE_NAME_CASE(BID_DPE_OUT_DMP_R);
        MAKE_NAME_CASE(BID_P2AFM_OUT_RECT_IN2_CAP);
        MAKE_NAME_CASE(BID_POST_PROCESS_TEMP_YV12);
        MAKE_NAME_CASE(BID_POST_PROCESS_TEMP_I422);
        MAKE_NAME_CASE(BID_POSTVIEW);
        MAKE_NAME_CASE(BID_PRE_PROCESS_OUT_W_1_IMG3O);
        MAKE_NAME_CASE(BID_DENOISE_HAL_OUT_ROT_BACK_IMG3O);
        MAKE_NAME_CASE(BID_PRE_PROCESS_OUT_YUV_IMG3O);
        MAKE_NAME_CASE(BID_BAYER_PREPROCESS_YUV);
        MAKE_NAME_CASE(BID_MONO_PREPROCESS_YUV);
        MAKE_NAME_CASE(BID_MONO_PREPROCESS_SMALL_YUV);
        MAKE_NAME_CASE(BID_WARP_OUT);
        MAKE_NAME_CASE(BID_FUSION_OUT);
        MAKE_NAME_CASE(BID_MFNR_OUT);
        MAKE_NAME_CASE(BID_MFNR_OUT_FINAL);
        MAKE_NAME_CASE(BID_LCSO_1_BUF);
        MAKE_NAME_CASE(BID_LCSO_2_BUF);
        MAKE_NAME_CASE(BID_LCSO_ROT_BUF);
    }
    MY_LOGW("unknown BID:%d", BID);

    return "unknown";
    #undef MAKE_NAME_CASE
}

const char*
BMDeNoisePipeNode::
getBIDPostfix(BMDeNoiseBufferID BID)
{
    switch(BID){
        case BID_INPUT_FSRAW_1:
        case BID_INPUT_FSRAW_2:
        case BID_INPUT_RSRAW_1:
        case BID_INPUT_RSRAW_2:
        case BID_PRE_PROCESS_OUT_MFBO_FINAL_1:
        case BID_PRE_PROCESS_OUT_MFBO_FINAL_2:
        case BID_DENOISE_HAL_OUT:
            return "raw";
        default:
            return "yuv";
    }
}

const char*
BMDeNoisePipeNode::
getProcessName(BMDeNoiseProcessId PID)
{
    #define MAKE_NAME_CASE(name) \
        case name: return #name;

    switch(PID)
    {
        MAKE_NAME_CASE(UNKNOWN);
        MAKE_NAME_CASE(BAYER_TO_YUV);
        MAKE_NAME_CASE(BAYER_COLOR_EFFECT);
        MAKE_NAME_CASE(MONO_TO_YUV);
        MAKE_NAME_CASE(BAYER_PREPROCESS);
        MAKE_NAME_CASE(MONO_PREPROCESS);
        MAKE_NAME_CASE(P2AFM);
        MAKE_NAME_CASE(DPE);
        MAKE_NAME_CASE(DO_GGM);
        MAKE_NAME_CASE(YUV_MERGE);
        MAKE_NAME_CASE(FINAL_PROCESS);
    }
    MY_LOGW("unknown PID:%d", PID);

    return "unknown";
    #undef MAKE_NAME_CASE
}

MBOOL
BMDeNoisePipeNode::
formatConverter(
        IImageBuffer *imgSrc,
        IImageBuffer *imgDst)
{
    MY_LOGD("%s +", __FUNCTION__);
    MBOOL ret = MFALSE;

    ISImager *pISImager = NULL;
    pISImager = ISImager::createInstance(imgSrc);
    if (!pISImager) {
        MY_LOGE("ISImager::createInstance() failed!!!");
        goto lbExit;
    }

    if (!pISImager->setTargetImgBuffer(imgDst)) {
        MY_LOGE("setTargetImgBuffer failed!!!");
        goto lbExit;
    }

    if (!pISImager->execute()) {
        MY_LOGE("execute failed!!!");
        goto lbExit;
    }

    ret = MTRUE;

lbExit:
    if (pISImager) {
        pISImager->destroyInstance();
    }

    MY_LOGD("%s -", __FUNCTION__);
    return ret;
}

MVOID
BMDeNoisePipeNode::
MET_START(const char* tag_name)
{
#if MET_USER_EVENT_SUPPORT
    met_tag_start(0, tag_name);
#endif
}

MVOID
BMDeNoisePipeNode::
MET_END(const char* tag_name)
{
#if MET_USER_EVENT_SUPPORT
    met_tag_end(0, tag_name);
#endif
}

MVOID
BMDeNoisePipeNode::
initBufferPool()
{
    MY_LOGD("do nothing");
}

MBOOL
BMDeNoisePipeNode::
doBufferPoolAllocation(MUINT32 count)
{
    MY_LOGD("do nothing");
    return MFALSE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMDeNoisePipeNode::
prepareModuleSettings()
{
    MY_LOGD("+");

    // module rotation
    ENUM_ROTATION eRot = StereoSettingProvider::getModuleRotation();
    mModuleTrans = eRotationToeTransform(eRot);
    MY_LOGD("mModuleTrans=%d", mModuleTrans);

    MY_LOGD("-");
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MINT32
BMDeNoisePipeNode::
eRotationToeTransform(ENUM_ROTATION rotation)
{
    MINT32 ret = -1;
    switch(rotation)
    {
        case eRotate_0:
            ret = 0;
            break;
        case eRotate_90:
            ret = eTransform_ROT_90;
            break;
        case eRotate_180:
            ret = eTransform_ROT_180;
            break;
        case eRotate_270:
            ret = eTransform_ROT_270;
            break;
        default:
            MY_LOGE("unknow format = %d", rotation);
            break;
    }
    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
ENUM_ROTATION
BMDeNoisePipeNode::
eTransformToRotation(MINT32 eTrans)
{
    ENUM_ROTATION ret = eRotate_0;
    switch(eTrans)
    {
        case 0:
            ret = eRotate_0;
            break;
        case eTransform_ROT_90:
            ret = eRotate_90;
            break;
        case eTransform_ROT_180:
            ret = eRotate_180;
            break;
        case eTransform_ROT_270:
            ret = eRotate_270;
            break;
        default:
            MY_LOGE("unknow format = %d", eTrans);
            break;
    }
    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
MINT32
BMDeNoisePipeNode::
eTransformToDegree(MINT32 eTrans)
{
    MINT32 ret = 0;
    switch(eTrans)
    {
        case 0:
            ret = 0;
            break;
        case eTransform_ROT_90:
            ret = 90;
            break;
        case eTransform_ROT_180:
            ret = 180;
            break;
        case eTransform_ROT_270:
            ret = 270;
            break;
        default:
            MY_LOGE("Not support eTransform=%d", eTrans);
    }
    MY_LOGD("trans:%d -> degree:%d", eTrans, ret);
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT32
BMDeNoisePipeNode::
DegreeToeTransform(MINT32 degree)
{
    MINT32 ret = 0;
    switch(degree)
    {
        case 0:
            ret = 0;
            break;
        case 90:
            ret = eTransform_ROT_90;
            break;
        case 180:
            ret = eTransform_ROT_180;
            break;
        case 270:
            ret = eTransform_ROT_270;
            break;
        default:
            MY_LOGE("Not support degree=%d", degree);
    }
    MY_LOGD("degree:%d -> trans:%d", degree, ret);
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
TuningParam
BMDeNoisePipeNode::
applyISPTuning(
    const char* name,
    SmartTuningBuffer& targetTuningBuf,
    const ISPTuningConfig& ispConfig,
    MBOOL useDefault,
    MBOOL updateHalMeta
)
{
    MY_LOGD("+, [%s] reqID=%d bIsResized=%d", name, ispConfig.reqNo, ispConfig.bInputResizeRaw);

    NS3Av3::TuningParam tuningParam = {NULL, NULL};
    tuningParam.pRegBuf = reinterpret_cast<void*>(targetTuningBuf->mpVA);

    // LCEI
    if(ispConfig.pLcsBuf != nullptr){
        MY_LOGD("have LCSO Buf");
        tuningParam.pLcsBuf = reinterpret_cast<void*>(ispConfig.pLcsBuf);
    }

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
MINT32
BMDeNoisePipeNode::
getJpegRotation(IMetadata* pMeta)
{
    MINT32 jpegRotationApp = 0;
    if(!tryGetMetadata<MINT32>(pMeta, MTK_JPEG_ORIENTATION, jpegRotationApp))
    {
        MY_LOGE("Get jpegRotationApp failed!");
    }
    MINT32 rotDegreeJpeg = jpegRotationApp;
    if(rotDegreeJpeg < 0){
        rotDegreeJpeg = rotDegreeJpeg + 360;
    }
    MY_LOGD("jpegRotationApp:%d, rotDegreeJpeg:%d", jpegRotationApp, rotDegreeJpeg);
    return DegreeToeTransform(rotDegreeJpeg);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMDeNoisePipeNode::
checkToDump(DataID id)
{
    MINT32 iNodeDump =  getPropValue();
    if(getPropValue() == 0)
    {
        VSDOF_LOGD("node check failed!node: %s dataID: %d", getName(), id);
        return MFALSE;
    }
    if(getPropValue(id) == 0)
    {
        VSDOF_LOGD("dataID check failed!dataID: %d", id);
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMDeNoisePipeNode::
shouldDumpRequest(PipeRequestPtr pReq)
{
    MUINT iReqIdx = pReq->getRequestNo();
    if(iReqIdx < miDumpStartIdx || iReqIdx >= miDumpStartIdx + miDumpBufSize){
        return MFALSE;
    }else{
        return MTRUE;
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMDeNoisePipeNode::
shouldDumpForTuning(BMDeNoiseBufferID BID)
{
    if(miTuningDump != 1){
        return MTRUE;
    }

    switch(BID){
        case BID_INPUT_FSRAW_1:
        case BID_INPUT_FSRAW_2:
        case BID_INPUT_RSRAW_1:
        case BID_INPUT_RSRAW_2:
        case BID_LCSO_1_BUF:
        case BID_LCSO_2_BUF:
            return MTRUE;
        default:
            return MFALSE;
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMDeNoisePipeNode::
separateFloat(float src, MINT32& integer, MINT32& fraction)
{
    integer = (int)src;
    float decpart = src - integer;
    fraction = 100000000*(src - integer);
    MY_LOGD("src = %f, integer = %d, decpart = %f, fraction:%d", src, integer, decpart, fraction);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMDeNoisePipeNode::
systemCall(const char* cmd)
{
    MY_LOGD("%s", cmd);
    system(cmd);
}
/*******************************************************************************
 *
 ********************************************************************************/
sp<ImageBufferHeap>
BMDeNoisePipeNode::
createImageBufferHeapInFormat(sp<IImageBuffer> pImgBuf, EImageFormat eFmt, MSize ROI)
{
    IImageBufferHeap* heap = pImgBuf->getImageBufferHeap();

    using namespace Utils::Format;

    if(heap->getPlaneCount() != 1 || queryPlaneCount(eFmt) != 1){
        MY_LOGE("only support 1 plane transfom src:%d, target:%d", heap->getPlaneCount(), queryPlaneCount(eFmt));
        return nullptr;
    }

    MY_LOGD("targetBPP:%d, source BPP:%d",
        queryImageBitsPerPixel(eFmt),
        queryImageBitsPerPixel(heap->getImgFormat())
    );

    if(queryImageBitsPerPixel(eFmt) % 8 != 0){
        MY_LOGE("BPP for 0x%x not a exact multiple of 8!!", eFmt);
        return nullptr;
    }

    MY_LOGD("format trans before %p, 0x%x, %dx%d_%d",
        heap,
        heap->getImgFormat(),
        heap->getImgSize().w, heap->getImgSize().h, heap->getBufStridesInBytes(0));

    // Target Settings
    MUINT32 bufStridesInBytes[3] = {
        (MUINT32)ROI.w*queryImageBitsPerPixel(eFmt)/8,
        0,
        0
    };

    MSize targetSize(ROI);
    MINT32 bufBoundaryInBytes[3] = {0,0,0};
    MY_LOGD("create new heap with size:%dx%d, bufStridesInBytes [%d,%d,%d]",
        targetSize.w, targetSize.h,
        bufStridesInBytes[0],
        bufStridesInBytes[1],
        bufStridesInBytes[2]
    );

    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(
        eFmt,
        targetSize,
        bufStridesInBytes,
        bufBoundaryInBytes,
        1
    );
    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
        pImgBuf->getFD(0),
        (MUINTPTR)pImgBuf->getBufVA(0),
        0,
        0,
        0
    );
    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(
        LOG_TAG,
        imgParam,
        portBufInfo
    );
    if(pHeap == nullptr){
        MY_LOGE("creating pHeap heap in %d failed!", eFmt);
    }

    MY_LOGD("format trans after %p, 0x%x, %dx%d_%d",
        pHeap.get(),
        pHeap->getImgFormat(),
        pHeap->getImgSize().w, pHeap->getImgSize().h, pHeap->getBufStridesInBytes(0));

    return pHeap;
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMDeNoisePipeNode::
updateSrcCropByOutputRatio(MRect& rSrcCrop, sp<IImageBuffer> src, sp<IImageBuffer> dst)
{
    MSize srcCrop_origin = rSrcCrop.s;
    MSize dstSize = dst->getImgSize();
    MSize srcSize = src->getImgSize();
    MY_LOGD("before rSrcCropStartPt:(%d, %d) rSrcCropSize:(%d,%d) src:(%dx%d)",
        rSrcCrop.p.x, rSrcCrop.p.y, rSrcCrop.s.w, rSrcCrop.s.h, srcSize.w, srcSize.h
    );
    if(srcCrop_origin.w > srcSize.w){
        MY_LOGW("crop width exceed src width, changed to source width");
        srcCrop_origin.w = srcSize.w;
    }
    if(srcCrop_origin.h > srcSize.h){
        MY_LOGW("crop height exceed src height, changed to source height");
        srcCrop_origin.h = srcSize.h;
    }
    // calculate the required image hight according to image ratio
    #define APPLY_2_ALIGN(x)  ((int(x)>>1)<<1)
    const double OUTPUT_RATIO = ((double)dstSize.w) / ((double)dstSize.h);
    rSrcCrop.s = srcCrop_origin;
    rSrcCrop.s.h = APPLY_2_ALIGN(rSrcCrop.s.w / OUTPUT_RATIO);
    if(rSrcCrop.s.h > srcCrop_origin.h) {
        rSrcCrop.s.h = APPLY_2_ALIGN(srcCrop_origin.h);
        rSrcCrop.s.w = APPLY_2_ALIGN(rSrcCrop.s.h * OUTPUT_RATIO);
    } else {
        rSrcCrop.s.w = APPLY_2_ALIGN(srcCrop_origin.w);
    }
    rSrcCrop.p.x = (srcCrop_origin.w - rSrcCrop.s.w)/2;
    rSrcCrop.p.y = (srcCrop_origin.h - rSrcCrop.s.h)/2;
    MY_LOGD("after rSrcCropStartPt:(%d, %d) rSrcCropSize:(%d,%d)",
            rSrcCrop.p.x, rSrcCrop.p.y, rSrcCrop.s.w, rSrcCrop.s.h);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMDeNoisePipeNode::
setRawDumpExtNames(ImgInfoMapPtr imgInfoMap, const BMDeNoiseBufferID& bid)
{
    // pass1 raw is dump in unpak format
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(!shouldDumpRequest(imgInfoMap->getRequestPtr())){
        return;
    }

    // width
    string extBufName = to_string(imgInfoMap->getIImageBuffer(bid)->getImgSize().w) + "x";

    // height
    extBufName = extBufName + to_string(imgInfoMap->getIImageBuffer(bid)->getImgSize().h) + "_";

    // Strides
    extBufName = extBufName + to_string(imgInfoMap->getIImageBuffer(bid)->getBufStridesInBytes(0)) + "_";

    // bayer order
    MINT32 bayerOrder = -1;
    if(bid == BID_INPUT_FSRAW_1 || bid == BID_INPUT_FSRAW_2){
        bayerOrder = mBayerOrder_main1;
    }else if(bid == BID_INPUT_RSRAW_1 || bid == BID_INPUT_RSRAW_2){
        bayerOrder = mBayerOrder_main2;
    }else{
        MY_LOGE("un-supported BID!");
    }
    extBufName = extBufName + to_string(bayerOrder);

    MY_LOGD("extBufName:%s", extBufName.c_str());
    imgInfoMap->addExtBufName(bid, extBufName);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMDeNoisePipeNode::
fillInPQParam(PQParam* const pPQParam, MUINT32 processId)
{
    auto fiilPQ = [&](DpPqParam* rISPParam){
        rISPParam->enable           = false;
        rISPParam->scenario         = MEDIA_ISP_CAPTURE;

        rISPParam->u.isp.iso        = 0; // dummy value because PQ should be disabled
        rISPParam->u.isp.lensId     = BMDN_FEAUTRE_UNIQUE_ID;
        rISPParam->u.isp.timestamp  = processId;
        rISPParam->u.isp.frameNo    = processId;
        rISPParam->u.isp.requestNo  = processId;

        rISPParam->u.isp.clearZoomParam.captureShot = CAPTURE_SINGLE;
    };

    // WROT
    DpPqParam*  ISPParam_WROT = new DpPqParam();
    fiilPQ(ISPParam_WROT);
    pPQParam->WROTPQParam = static_cast<void*>(ISPParam_WROT);

    // WDMA
    DpPqParam*  ISPParam_WDMA = new DpPqParam();
    fiilPQ(ISPParam_WDMA);
    pPQParam->WDMAPQParam = static_cast<void*>(ISPParam_WDMA);

    MY_LOGD("PQ params: lensId(%d) captureShot(%d) timestamp(%d) frameNo(%d) requestNo(%d) enable(%d) scenario(%d) iso(%d)",
        ISPParam_WROT->u.isp.lensId,
        ISPParam_WROT->u.isp.clearZoomParam.captureShot,
        ISPParam_WROT->u.isp.timestamp,
        ISPParam_WROT->u.isp.frameNo,
        ISPParam_WROT->u.isp.requestNo,
        ISPParam_WROT->enable,
        ISPParam_WROT->scenario,
        ISPParam_WROT->u.isp.iso
    );
}

/*******************************************************************************
 *
 ********************************************************************************/
MRect
BMDeNoisePipeNode::
getFovRatioCrop(const MSize srcImgSize, const MSize dstImgSize, MBOOL isCropMain)
{
    FOVCropCalculator fOVCropCalculator(isCropMain);
    fOVCropCalculator.setRatio(dstImgSize);

    MRect ret = fOVCropCalculator.getResultRegion(srcImgSize);
    MY_LOGD("srcImgSize: (%d, %d), dstImgSize: (%d, %d), fovRatioCrop: (%d, %d, %d, %d)",
        srcImgSize.w, srcImgSize.h, dstImgSize.w, dstImgSize.h, ret.p.x, ret.p.y, ret.s.w, ret.s.h);

    return ret;
}