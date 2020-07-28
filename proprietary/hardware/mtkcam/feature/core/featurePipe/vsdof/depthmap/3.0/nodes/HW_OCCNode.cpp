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

using namespace NS3Av3;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

#define OCC_USER_NAME "DEPTHPIP_HWOCC"

#define DISTANCE_LOG_PROPERTY "vendor.debug.STEREO.log.distance"

HW_OCCNode::
HW_OCCNode(
   const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: DepthMapPipeNode(name, nodeID, config)
, AF_ROTATE_DOMAIN(StereoSizeProvider::getInstance()->getBufferSize(E_DMG))
, DEPTHMAP_SIZE(StereoSizeProvider::getInstance()->getBufferSize(E_DEPTH_MAP))
, mDistnaceLogEnabled(property_get_int32(DISTANCE_LOG_PROPERTY, 0))
, mLogger("StereoDistance", DISTANCE_LOG_PROPERTY)
{
    mLogger.setSingleLineMode(false);
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

    //Get AF table
    int32_t main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    IHal3A *pHal3A = MAKE_Hal3A(main1Idx, LOG_TAG);
    if(NULL == pHal3A) {
        MY_LOGE("Cannot get 3A HAL");
    } else {
        pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetAFDAFTable, (MUINTPTR)&mpAFTable, 0);
        if(NULL == mpAFTable) {
            MY_LOGE("Cannot get AF table");
        }

        pHal3A->destroyInstance(LOG_TAG);
    }

    __initAFWinTransform();

    NSCam::NSIoPipe::OCCConfig config;
    StereoTuningProvider::getHWOCCTuningInfo(config, eSTEREO_SCENARIO_PREVIEW);
    INVALID_DEPTH_VALUE = config.occ_invalid_value;

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
    CAM_TRACE_NAME("HW_OCCNode::onThreadStart");
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
    CAM_TRACE_NAME("HW_OCCNode::onThreadStop");
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

    CAM_TRACE_NAME("HW_OCCNode::onThreadLoop");
    // mark on-going-request start
    this->incExtThreadDependency();
    VSDOF_LOGD("HW OCC threadloop start, reqID=%d", iReqNo);

    EGNParams<OCCConfig> rOccParams;
    EnqueCookieContainer *pEnqueCookie;
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
    CAM_TRACE_BEGIN("HW_OCCNode::Enque");
    bRet = mpOweStream->EGNenque(rOccParams);
    CAM_TRACE_END();
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
    CAM_TRACE_NAME("HW_OCCNode::handleOWEDone");
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

            if(FEFMSettingProvider::getInstance()->needToRunFEFM(magicNumber[0], magicNumber[1], false))
            {
                float focalLensFactor = 0.0f;
                const int AF_INDEX = magicNumber[0] % DAF_TBL_QLEN;
                DepthMapBufferID outAppBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_OUT_APP);
                IMetadata* pOutAppMeta = pRequest->getBufferHandler()->requestMetadata(getNodeId(), outAppBID);
                if(!tryGetMetadata<MFLOAT>(pOutAppMeta, MTK_STEREO_FEATURE_RESULT_DISTANCE, focalLensFactor))
                {
                    MY_LOGW("reqID=%d, Failed to get MTK_STEREO_FEATURE_RESULT_DISTANCE!", pRequest->getRequestNo());
                }
                else
                {
                    VSDOF_PRFLOG("N3D FocalLensFactor %.2f", focalLensFactor);
                    mDistance = __getDistance(depthInfo.mpDepthBuffer->mImageBuffer.get(), __getAFRect(AF_INDEX), focalLensFactor);
                    VSDOF_PRFLOG("Distance %.2f cm, DAC %d(Macro: %d, Inf: %d)", mDistance, mDacCurrent, mDacMacro, mDacInf);
                }
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
            MY_LOGD("imageBuff plane count=%d", imageBuf->getPlaneCount());\
            MY_LOGD("imageBuff format=%x", imageBuf->getImgFormat());\
            MY_LOGD("imageBuff getImgBitsPerPixel=%d", imageBuf->getImgBitsPerPixel());\
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
        MY_LOGD("u4BufVA=%x", buffer.u4BufVA);\
        MY_LOGD("u4BufPA=%x", buffer.u4BufPA);\
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

//For distance calculation
void
HW_OCCNode::__initAFWinTransform()
{
    MINT32 err = 0;
    int main1SensorIndex, main2SensorIndex;
    StereoSettingProvider::getStereoSensorIndex(main1SensorIndex, main2SensorIndex);

    int main1SensorDevIndex, main2SensorDevIndex;
    StereoSettingProvider::getStereoSensorDevIndex(main1SensorDevIndex, main2SensorDevIndex);

    IHalSensorList* sensorList = MAKE_HalSensorList();
    IHalSensor* pIHalSensor = NULL;

    if(NULL == sensorList) {
        MY_LOGE("Cannot get sensor list");
    } else {
        MUINT32 junkStride;
        //========= Get main1 size =========
        IHalSensor* pIHalSensor = sensorList->createSensor(LOG_TAG, main1SensorIndex);
        if(NULL == pIHalSensor) {
            MY_LOGE("Cannot get hal sensor of main1");
            err = 1;
        } else {
            SensorCropWinInfo sensorCropInfoZSD;
            const int STEREO_FEATURE_MODE = NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP;
            const int STEREO_MODULE_TYPE = (StereoSettingProvider::isBayerPlusMono()) ? NSCam::v1::Stereo::BAYER_AND_MONO : NSCam::v1::Stereo::BAYER_AND_BAYER;
            MUINT sensorScenarioMain1, sensorScenarioMain2;

            ::memset(&sensorCropInfoZSD, 0, sizeof(SensorCropWinInfo));
            StereoSettingProvider::getSensorScenario(STEREO_FEATURE_MODE,
                                                     STEREO_MODULE_TYPE,
                                                     PipelineMode_ZSD,
                                                     sensorScenarioMain1,
                                                     sensorScenarioMain2);
            err = pIHalSensor->sendCommand(main1SensorDevIndex, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                           (MUINTPTR)&sensorScenarioMain1, (MUINTPTR)&sensorCropInfoZSD, 0);
            if(err) {
                MY_LOGE("Cannot get sensor crop info for preview/record");
            } else {
                switch(StereoSettingProvider::imageRatio()) {
                    case eRatio_4_3:
                    default:
                        {
                            mAFOffsetX = sensorCropInfoZSD.x0_offset;
                            mAFOffsetY = sensorCropInfoZSD.y0_offset;
                            mAFScaleW  = (float)AF_ROTATE_DOMAIN.w / sensorCropInfoZSD.scale_w;
                            mAFScaleH  = (float)AF_ROTATE_DOMAIN.h / sensorCropInfoZSD.scale_h;
                        }
                        break;
                    case eRatio_16_9:
                        {
                            //4:3->16:9
                            mAFOffsetX = sensorCropInfoZSD.x0_offset;
                            mAFOffsetY = sensorCropInfoZSD.y0_offset + sensorCropInfoZSD.scale_h/8;
                            mAFScaleW  = (float)AF_ROTATE_DOMAIN.w / sensorCropInfoZSD.scale_w;
                            mAFScaleH  = (float)AF_ROTATE_DOMAIN.h / (sensorCropInfoZSD.scale_h * 3 / 4);
                        }
                        break;
                }
            }

            pIHalSensor->destroyInstance(LOG_TAG);

            VSDOF_LOGD("AF Transform: offset(%d, %d), scale(%f, %f)",
                       mAFOffsetX, mAFOffsetY, mAFScaleW, mAFScaleH);
        }
    }
}

MPoint
HW_OCCNode::__AFToDepthmapPoint(const MPoint &ptAF)
{
    return MPoint( (ptAF.x - mAFOffsetX) * mAFScaleW ,
                   (ptAF.y - mAFOffsetY) * mAFScaleH );
}

MRect
HW_OCCNode::__getAFRect(const int AF_INDEX)
{
    DAF_VEC_STRUCT *afVec = &mpAFTable->daf_vec[AF_INDEX];
    MPoint topLeft     = __AFToDepthmapPoint(MPoint(afVec->af_win_start_x, afVec->af_win_start_y));
    mLogger.FastLogD("Top Left:     (%d, %d) -> (%d, %d)",
                     afVec->af_win_start_x, afVec->af_win_start_y, topLeft.x, topLeft.y);
    MPoint bottomRight = __AFToDepthmapPoint(MPoint(afVec->af_win_end_x, afVec->af_win_end_y));
    mLogger.FastLogD("Bottom Right: (%d, %d) -> (%d, %d)",
                     afVec->af_win_end_x, afVec->af_win_end_y, bottomRight.x, bottomRight.y);
    MRect focusRect = StereoHAL::rotateRect( MRect(topLeft, MSize(bottomRight.x - topLeft.x + 1, bottomRight.y - topLeft.y + 1)),
                                             AF_ROTATE_DOMAIN,
                                             StereoSettingProvider::getModuleRotation() );
    mLogger.FastLogD("Rotated focus rect: (%d, %d), %dx%d (domain size %dx%d)",
                     focusRect.p.x, focusRect.p.y, focusRect.s.w, focusRect.s.h, AF_ROTATE_DOMAIN.w, AF_ROTATE_DOMAIN.h);

    //To depthmap domain
    int depthmapWidthFactor = 2;//DEPTHMAP_SIZE.w/AF_ROTATE_DOMAIN.h;
    focusRect.p.x *= depthmapWidthFactor;
    focusRect.s.w *= depthmapWidthFactor;

    //Handle out-of-range, center point will not change, but size will
    if(focusRect.p.x < 0) {
        focusRect.s.w -= -focusRect.p.x * 2;
        focusRect.p.x = 0;
    }

    if(focusRect.p.y < 0) {
        focusRect.s.h -= -focusRect.p.y * 2;
        focusRect.p.y = 0;
    }

    int offset = focusRect.p.x + focusRect.s.w - DEPTHMAP_SIZE.w;
    if(offset > 0) {
        focusRect.s.w -= offset * 2;
        focusRect.p.x += offset;
    }

    offset = focusRect.p.y + focusRect.s.h - DEPTHMAP_SIZE.h;
    if(offset > 0) {
        focusRect.s.h -= offset * 2;
        focusRect.p.y += offset;
    }

    return focusRect;
}

MFLOAT
HW_OCCNode::__getDistance(IImageBuffer *depthmap, MRect afROI, MFLOAT focalLensFactor)
{
    mLogger.FastLogD("=====================================================\n"
                     "  AF ROI: (%d, %d) size %dx%d, depthmap size %dx%d\n"
                     "================= Content of AF ROI =================",
                     afROI.p.x, afROI.p.y, afROI.s.w, afROI.s.h,
                     depthmap->getImgSize().w, depthmap->getImgSize().h);
    ::memset(mROIStatistics, 0, sizeof(mROIStatistics));
    const size_t STRIDE = depthmap->getImgSize().w;
    MUINT8 *posHead = (MUINT8*)depthmap->getBufVA(0) + STRIDE * afROI.p.y + afROI.p.x;
    MUINT8 *posRun = posHead;

    char debugLine[afROI.s.w+1];
    debugLine[afROI.s.w] = 0;
    int debugValue = 0;
    //Decide depth, find index of max count of near depth
    float distance      = 10000.0f;    //means infinite
    int statisticsStart = 128;
    int statisticsEnd   = ROI_STATISTICS_SIZE - 1;
    int depthIndex      = 0;
    if(0 == INVALID_DEPTH_VALUE) {
        statisticsStart = 129;
        statisticsEnd   = ROI_STATISTICS_SIZE;
    }
    int minIndex        = statisticsStart - 128;

    //Depthmap statistics
    for(int row = 0; row < afROI.s.h; row++) {
        for(int col = 0; col < afROI.s.w; col++, posRun++) {
            mROIStatistics[*posRun]++;

            if(mDistnaceLogEnabled) {
                if(*posRun==INVALID_DEPTH_VALUE) {
                    //Hole
                    debugLine[col] = ' ';
                } else {
                    if(*posRun <= statisticsStart) {
                        //Invalid depth
                        debugLine[col] = '-';
                    } else {
                        //Valid depth to 0~9
                        debugValue = (*posRun-statisticsStart)/13;
                        debugLine[col] = debugValue+48;
                    }
                }
            }
        }

        if(mDistnaceLogEnabled) {
            mLogger.FastLogD("%s", debugLine);
        }

        posHead += STRIDE;
        posRun = posHead;
    }

    int MAX_COUNT = 0;
    int targetDepth = statisticsStart;
    for(depthIndex = statisticsStart+1; depthIndex < statisticsEnd-1; ++depthIndex) {
        if(mROIStatistics[depthIndex] > MAX_COUNT) {
            MAX_COUNT = mROIStatistics[depthIndex];
            targetDepth = depthIndex;
        }
    }

    if(targetDepth > statisticsStart) {
        distance = focalLensFactor/(targetDepth - statisticsStart);
    }

    if(mDistnaceLogEnabled) {
        mLogger.FastLogD("============= Depth Histogram in AF ROI =============");
        const int TOTAL_MAX_COUNT = *std::max_element(mROIStatistics+statisticsStart-128, mROIStatistics+statisticsEnd);
        int COUNT_PER_SYMBOL = TOTAL_MAX_COUNT/40;    //40 '*'
        for(depthIndex = statisticsEnd-1; depthIndex >= minIndex; --depthIndex) {
            if(statisticsStart == depthIndex &&
               MAX_COUNT > 0)
            {
                mLogger.FastLogD("-----------------------------------------------------");
            }

            if(mROIStatistics[depthIndex] > 0)
            {
                if(targetDepth != depthIndex ||
                   depthIndex <= statisticsStart)
                {
                    mLogger.FastLogD("%4d|%-40.*s (%5d)",
                                     depthIndex,
                                     mROIStatistics[depthIndex]/COUNT_PER_SYMBOL, "****************************************",
                                     mROIStatistics[depthIndex]);
                } else {
                    mLogger.FastLogD("%4d|%-40.*s (%5d) <-- Target depth",
                                     depthIndex,
                                     mROIStatistics[depthIndex]/COUNT_PER_SYMBOL, "****************************************",
                                     mROIStatistics[depthIndex]);
                }

            }
        }

        mLogger.FastLogD("=====================================================");
        if(MAX_COUNT > 0) {
            mLogger.FastLogD("   Depth %d, FB %.2f, Distance %.2f\n", targetDepth, focalLensFactor, distance);
        } else {
            mLogger.FastLogD("   Depth --, FB %.2f, Distance %.2f\n", focalLensFactor, distance);
        }
        mLogger.FastLogD("=====================================================");
        mLogger.print();
    }

    return distance;
}

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam
