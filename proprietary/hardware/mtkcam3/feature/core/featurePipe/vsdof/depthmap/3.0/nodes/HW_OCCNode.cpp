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

// Local header file
#include "HW_OCCNode.h"
#include "../DepthMapPipe_Common.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"

// Logging header file
#define PIPE_CLASS_TAG "HW_OCCNode"
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <fefm_setting_provider.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH_HW_OCC);

using namespace NS3Av3;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using StereoHAL::StereoDistanceUtil;

#define OCC_USER_NAME "DEPTHPIP_HWOCC"

#define DISTANCE_LOG_PROPERTY "vendor.debug.STEREO.log.distance"

HW_OCCNode::
HW_OCCNode(
   const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
, mDistanceUtil(E_DIST_SRC_NOC)
{
    this->addWaitQueue(&mJobQueue);
    this->addWaitQueue(&mMYSReqIDQueue);
}

HW_OCCNode::
~HW_OCCNode()
{
    MY_LOGD("[Destructor]");
}

MVOID
HW_OCCNode::
cleanUp()
{
    VSDOF_LOGD("+");
    if(mpOweStream != nullptr)
    {
        mpOweStream->uninit();
        mpOweStream->destroyInstance(OCC_USER_NAME);
        mpOweStream = nullptr;
    }
    mJobQueue.clear();
    VSDOF_LOGD("-");
}

MBOOL
HW_OCCNode::
onInit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
HW_OCCNode::
onUninit()
{
    VSDOF_INIT_LOG("+");
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
HW_OCCNode::
onThreadStart()
{
    VSDOF_INIT_LOG("+");
    CAM_ULOGM_TAGLIFE("HW_OCCNode::onThreadStart");
    // init oweStream
    mpOweStream = IEgnStream<OCCConfig>::createInstance(OCC_USER_NAME);
    if(mpOweStream == nullptr)
    {
        MY_LOGE("OWE Stream create instance failed!");
        return MFALSE;
    }
    if(!mpOweStream->init())
    {
        MY_LOGE("OWE Stream init failed!");
        return MFALSE;
    }

    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
HW_OCCNode::
onThreadStop()
{
    CAM_ULOGM_TAGLIFE("HW_OCCNode::onThreadStop");
    VSDOF_INIT_LOG("+");
    cleanUp();
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
HW_OCCNode::
onData(DataID data, DepthMapRequestPtr& pRequest)
{
    MBOOL bRet = MTRUE;
    VSDOF_LOGD("+, DataID=%s reqId=%d", ID2Name(data), pRequest->getRequestNo());

    switch(data)
    {
        case P2A_TO_HWOCC_MY_S:
            mMYSReqIDQueue.enque(pRequest->getRequestNo());
            break;
        case DPE_TO_HWOCC_MVSV_DMP:
            mJobQueue.enque(pRequest);
            break;
        default:
            MY_LOGW("Unrecongnized DataID=%d", data);
            bRet = MFALSE;
            break;
    }

    TRACE_FUNC_EXIT();
    return bRet;
}

MBOOL
HW_OCCNode::
onThreadLoop()
{
    MUINT32 iLDCReadyReqID, iMYSReadyReqID;
    DepthMapRequestPtr pRequest;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    // get request
    if( !mJobQueue.deque(pRequest) )
    {
        MY_LOGE("mJobQueue.deque() failed");
        return MFALSE;
    }
    // get MYS request id
    if(!mMYSReqIDQueue.deque(iMYSReadyReqID) )
    {
        MY_LOGE("mMYSReqIDQueue.deque() failed");
        return MFALSE;
    }

    MUINT32 iReqNo = pRequest->getRequestNo();
    if(iReqNo != iMYSReadyReqID)
    {
        AEE_ASSERT("[HW_OCCNode]The deque request is not consistent. iReqNo=%d iMYSReadyReqID=%d",
                    iReqNo, iMYSReadyReqID);
        return MFALSE;
    }

    CAM_ULOGM_TAGLIFE("HW_OCCNode::onThreadLoop");
    // mark on-going-request start
    this->incExtThreadDependency();
    VSDOF_LOGD("HW OCC threadloop start, reqID=%d", iReqNo);

    EGNParams<OCCConfig> rOccParams;
    EnqueCookieContainer *pEnqueCookie = nullptr;
    // prepare in/out params
    MBOOL bRet = prepareEnqueParams(pRequest, rOccParams);
    if(!bRet)
    {
        MY_LOGE("reqID=%d, failed to prepare enque param!", pRequest->getRequestNo());
        goto lbExit;
    }

    rOccParams.mpfnCallback = onOWECallback;
    pEnqueCookie = new EnqueCookieContainer(pRequest, this);
    rOccParams.mpCookie = (void*) pEnqueCookie;

    VSDOF_PRFLOG("HW　OCC Enque start, reqID=%d", pRequest->getRequestNo());
    // timer
    pRequest->mTimer.startHWOCC();
    pRequest->mTimer.startHWOCCEnque();
    CAM_ULOGM_TAG_BEGIN("HW_OCCNode::Enque");
    bRet = mpOweStream->EGNenque(rOccParams);
    CAM_ULOGM_TAG_END();
    // timer
    pRequest->mTimer.stopHWOCCEnque();
    VSDOF_PRFTIME_LOG("HW OCC enque end, reqID=%d, exec-time=%d msec",
                pRequest->getRequestNo(), pRequest->mTimer.getElapsedHWOCCEnque());
    if(!bRet)
    {
        MY_LOGE("HW OCC enque failed");
        goto lbExit;
    }
    return MTRUE;

lbExit:
    delete pEnqueCookie;
    // launch onProcessDone
    pRequest->getBufferHandler()->onProcessDone(getNodeId());
    // mark on-going-request end
    this->decExtThreadDependency();
    return MFALSE;
}

MVOID
HW_OCCNode::
onOWECallback(
    EGNParams<OCCConfig>& rParams
)
{
    EnqueCookieContainer* pEnqueCookie = reinterpret_cast<EnqueCookieContainer*>(rParams.mpCookie);
    HW_OCCNode* pHW_OCCNode = reinterpret_cast<HW_OCCNode*>(pEnqueCookie->mpNode);
    pHW_OCCNode->handleOWEDone(rParams, pEnqueCookie);
}

MVOID
HW_OCCNode::
handleOWEDone(
    EGNParams<OCCConfig>& rParams,
    EnqueCookieContainer* pEnqueCookie
)
{
    CAM_ULOGM_TAGLIFE("HW_OCCNode::handleOWEDone");
    DepthMapRequestPtr pRequest = pEnqueCookie->mRequest;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    MUINT32 iReqNo =  pRequest->getRequestNo();
    IImageBuffer* pImgBuf_NOC = nullptr;
    DepthMapBufferID cfmID = (STEREO_SENSOR_REAR_MAIN_TOP ==
                                    StereoSettingProvider::getSensorRelativePosition())
                                    ? BID_DPE_OUT_CFM_L : BID_DPE_OUT_CFM_R;
    // check flush status
    if(mpNodeSignal->getStatus(NodeSignal::STATUS_IN_FLUSH))
        goto lbExit;
    pRequest->mTimer.stopHWOCC();
    VSDOF_PRFLOG("+, reqID=%d exec-time=%d ms", iReqNo, pRequest->mTimer.getElapsedHWOCC());
    if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_UNPROCESS_DEPTH)
    {
        DepthBufferInfo depthInfo;
        // unprocess depth means NOC buffer
        MBOOL bRet = pRequest->getBufferHandler()->getEnquedSmartBuffer(
                                    getNodeId(), BID_OCC_OUT_NOC, depthInfo.mpDepthBuffer);
        if(bRet)
        {
            depthInfo.mpDepthBuffer->mImageBuffer->syncCache(eCACHECTRL_INVALID);
            depthInfo.miReqIdx = pRequest->getRequestNo();

            // Get distance
            DepthMapBufferID inHalBID_Main1 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
            DepthMapBufferID inHalBID_Main2 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN2);
            IMetadata* pInHalMeta_Main1 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main1);
            IMetadata* pInHalMeta_Main2 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main2);

            int magicNumber[2] = {0, 0};
            if(!tryGetMetadata<MINT32>(pInHalMeta_Main1, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNumber[0])) {
                MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main1!");
            }
            if(!tryGetMetadata<MINT32>(pInHalMeta_Main2, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNumber[1])) {
                MY_LOGE("Cannot find MTK_P1NODE_PROCESSOR_MAGICNUM meta of Main2!");
            }
            VSDOF_PRFLOG("Magic Number: %d %d", magicNumber[0], magicNumber[1]);

            float focalLensFactor = 0.0f;
            DepthMapBufferID outAppBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_APP);
            IMetadata* pOutAppMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), outAppBID);
            if(!tryGetMetadata<MFLOAT>(pOutAppMeta, MTK_STEREO_FEATURE_RESULT_DISTANCE, focalLensFactor))
            {
                MY_LOGW("reqID=%d, Failed to get MTK_STEREO_FEATURE_RESULT_DISTANCE!", pRequest->getRequestNo());
            }
            else
            {
                VSDOF_PRFLOG("N3D FocalLensFactor %.2f", focalLensFactor);
                mDistance = mDistanceUtil.getDistance(depthInfo.mpDepthBuffer->mImageBuffer.get(),
                                                      focalLensFactor, magicNumber[0], magicNumber[1]);
                VSDOF_PRFLOG("Distance %.2f cm", mDistance);
            }
            depthInfo.mfDistance = mDistance;

            mpDepthStorage->setStoredData(depthInfo);
            this->handleDump(HWOCC_OUT_3RD_NOC, pRequest);
        }
        //
        handleData(QUEUED_FLOW_DONE, pRequest);
    }
    else
    {
        // invalidate buffer
        pBufferHandler->getEnqueBuffer(getNodeId(), BID_OCC_OUT_NOC, pImgBuf_NOC);
        pImgBuf_NOC->syncCache(eCACHECTRL_INVALID);
        // config output
        pBufferHandler->configOutBuffer(getNodeId(), BID_OCC_OUT_NOC, eDPETHMAP_PIPE_NODEID_WMF);
        pBufferHandler->configOutBuffer(getNodeId(), BID_P2A_OUT_MY_S, eDPETHMAP_PIPE_NODEID_WMF);
        // config CFM to GFNode
        pBufferHandler->configOutBuffer(getNodeId(), cfmID,  eDPETHMAP_PIPE_NODEID_GF);
        //
        handleDataAndDump(HWOCC_TO_WMF_NOC, pRequest);
    }
lbExit:
    // launch onProcessDone
    pBufferHandler->onProcessDone(getNodeId());
    delete pEnqueCookie;
    // mark on-going-request end
    this->decExtThreadDependency();
}

MBOOL
HW_OCCNode::
prepareEnqueParams(
    DepthMapRequestPtr& pRequest,
    EGNParams<OCCConfig>& rEnqParam
)
{
    OCCConfig occconfig;
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    // read default tuniung
    auto scenario = (pRequest->getRequestAttr().opState == eSTATE_CAPTURE) ? eSTEREO_SCENARIO_CAPTURE
                        : (pRequest->getRequestAttr().isEISOn) ? eSTEREO_SCENARIO_RECORD
                        : eSTEREO_SCENARIO_PREVIEW;
    StereoTuningProvider::getHWOCCTuningInfo(occconfig, scenario);

    //
    MBOOL bRet = MTRUE;
    IImageBuffer *pImgBuf_MV_Y = nullptr, *pImgBuf_SV_Y = nullptr;
    IImageBuffer *pImgBuf_DMP_L=nullptr,  *pImgBuf_DMP_R=nullptr, *pImgBuf_LDC = nullptr;
    // input buffers
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_MV_Y, pImgBuf_MV_Y);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_N3D_OUT_SV_Y, pImgBuf_SV_Y);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_DPE_OUT_DMP_L, pImgBuf_DMP_L);
    bRet &= pBufferHandler->getEnqueBuffer(getNodeId(), BID_DPE_OUT_DMP_R, pImgBuf_DMP_R);
    if(!bRet)
    {
        MY_LOGE("Cannot get enque buffers!");
        return MFALSE;
    }
    rEnqParam.mpEngineID = eOCC;
    // PXL means color image
    bRet = setupOWEBufInfo(DMA_OCC_MAJ_PXL, pImgBuf_MV_Y, occconfig.OCC_MAJ_PXL);
    bRet &= setupOWEBufInfo(DMA_OCC_REF_PXL, pImgBuf_SV_Y, occconfig.OCC_REF_PXL);
    // VEC neans disparity
    if(STEREO_SENSOR_REAR_MAIN_TOP == StereoSettingProvider::getSensorRelativePosition())
    {
        // Main1 locations: L
        bRet &= setupOWEBufInfo(DMA_OCC_MAJ_VEC, pImgBuf_DMP_L, occconfig.OCC_MAJ_VEC);
        bRet &= setupOWEBufInfo(DMA_OCC_REF_VEC, pImgBuf_DMP_R, occconfig.OCC_REF_VEC);
    }
    else
    {
        // Main1 locations: R
        bRet &= setupOWEBufInfo(DMA_OCC_MAJ_VEC, pImgBuf_DMP_R, occconfig.OCC_MAJ_VEC);
        bRet &= setupOWEBufInfo(DMA_OCC_REF_VEC, pImgBuf_DMP_L, occconfig.OCC_REF_VEC);
    }
    // decide the fmt
    if(pImgBuf_MV_Y->getImgFormat() == eImgFmt_YV12 || pImgBuf_MV_Y->getImgFormat() == eImgFmt_Y8)
    {
        occconfig.occ_imgi_maj_fmt = OWE_IMGI_Y_FMT;
        occconfig.occ_imgi_ref_fmt = OWE_IMGI_Y_FMT;
    }
    else if(pImgBuf_MV_Y->getImgFormat() == eImgFmt_YUY2)
    {
        occconfig.occ_imgi_maj_fmt = OWE_IMGI_YC_FMT;
        occconfig.occ_imgi_ref_fmt = OWE_IMGI_YC_FMT;
    }
    else
    {
        MY_LOGE("IMGI not support this format=%d", pImgBuf_MV_Y->getImgFormat());
        return MFALSE;
    }
    // output buffer
    IImageBuffer *pOutBuf_NOC = pBufferHandler->requestBuffer(getNodeId(), BID_OCC_OUT_NOC);
    bRet &= setupOWEBufInfo(DMA_OCC_WDMAO, pOutBuf_NOC, occconfig.OCC_WDMA);
    if(!bRet)
        return MFALSE;
    //
    rEnqParam.mEGNConfigVec.push_back(occconfig);

    debugOCCParams({pImgBuf_MV_Y, pImgBuf_SV_Y, pImgBuf_DMP_L,
                    pImgBuf_DMP_R, pOutBuf_NOC, occconfig});

    return MTRUE;
}

MVOID
HW_OCCNode::
debugOCCParams(DebugBufParam param)
{
    if(!DepthPipeLoggingSetup::mbDebugLog)
        return;
    #define OUTPUT_IMG_BUFF(imageBuf)\
        if(imageBuf!=NULL)\
        {\
            MY_LOGD("=======================:" # imageBuf);\
            MY_LOGD("imageBuff size=%dx%d", imageBuf->getImgSize().w, imageBuf->getImgSize().h);\
            MY_LOGD("imageBuff plane count=%zu", imageBuf->getPlaneCount());\
            MY_LOGD("imageBuff format=%x", imageBuf->getImgFormat());\
            MY_LOGD("imageBuff getImgBitsPerPixel=%zu", imageBuf->getImgBitsPerPixel());\
            MY_LOGD("=======================");\
        }\
        else\
            MY_LOGD("=======================:" # imageBuf " is NULL!!!!");
    #define OUTPUT_CONFIG_INT32(source, key)\
        MY_LOGD("occConfig." #key "=%d", source.key);
    #define OUTPUT_ENGBUF(buffer)\
        MY_LOGD("=======buffer: " #buffer);\
        MY_LOGD("dmaport=%d", buffer.dmaport);\
        MY_LOGD("memID=%d", buffer.memID);\
        MY_LOGD("u4BufVA=%" PRIxPTR "", buffer.u4BufVA);\
        MY_LOGD("u4BufPA=%" PRIxPTR "", buffer.u4BufPA);\
        MY_LOGD("u4BufSize=%d", buffer.u4BufSize);\
        MY_LOGD("u4Stride=%d", buffer.u4Stride);

    MY_LOGD("Input::");
    OUTPUT_IMG_BUFF(param.imgBuf_MV_Y);
    OUTPUT_IMG_BUFF(param.imgBuf_SV_Y);
    OUTPUT_IMG_BUFF(param.imgBuf_DMP_L);
    OUTPUT_IMG_BUFF(param.imgBuf_DMP_R);
    MY_LOGD("Output::");
    OUTPUT_IMG_BUFF(param.nocMap);
    MY_LOGD("OCC config:");
    OUTPUT_CONFIG_INT32(param.occConfig, occ_scan_r2l);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_horz_ds4);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_vert_ds4);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_h_skip_mode);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_imgi_maj_fmt);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_imgi_ref_fmt);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_hsize);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_vsize);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_v_crop_s);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_v_crop_e);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_h_crop_s);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_h_crop_e);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_th_luma);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_th_h);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_th_v);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_vec_shift);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_vec_offset);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_invalid_value);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_owc_th);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_owc_en);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_depth_clip_en);
    OUTPUT_CONFIG_INT32(param.occConfig, occ_spare);
    MY_LOGD("Buffer:");
    OUTPUT_ENGBUF(param.occConfig.OCC_REF_VEC);
    OUTPUT_ENGBUF(param.occConfig.OCC_REF_PXL);
    OUTPUT_ENGBUF(param.occConfig.OCC_MAJ_VEC);
    OUTPUT_ENGBUF(param.occConfig.OCC_MAJ_PXL);
    OUTPUT_ENGBUF(param.occConfig.OCC_WDMA);

    #undef OUTPUT_IMG_BUFF
    #undef OUTPUT_CONFIG_INT32
    #undef OUTPUT_ENGBUF
}

MVOID
HW_OCCNode::
onFlush()
{
    MY_LOGD("+ extDep=%d", this->getExtThreadDependency());
    DepthMapRequestPtr pRequest;
    while( mJobQueue.deque(pRequest) )
    {
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        pBufferHandler->onProcessDone(getNodeId());
    }
    DepthMapPipeNode::onFlush();
    MY_LOGD("-");
}

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam
