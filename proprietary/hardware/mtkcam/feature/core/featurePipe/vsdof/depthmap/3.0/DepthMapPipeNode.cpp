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
#include <time.h>
#include <string.h>
// Android system/core header file
#include <fstream>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/TuningUtils/CommonRule.h>
// Module header file
#include <featurePipe/core/include/DebugUtil.h>
// Local header file
#include "DepthMapPipeNode.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"
#include "./bufferConfig/BaseBufferConfig.h"
// logging
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "DepthMapPipeNode"
#include <featurePipe/core/include/PipeLog.h>
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {
/*******************************************************************************
* NodeSignal Definition
********************************************************************************/

NodeSignal::NodeSignal()
: mSignal(0)
, mStatus(0)
{
}

NodeSignal::~NodeSignal()
{
}

MVOID NodeSignal::setSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    mSignal |= signal;
    mCondition.broadcast();
}

MVOID NodeSignal::clearSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    mSignal &= ~signal;
}

MBOOL NodeSignal::getSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    return (mSignal & signal);
}

MVOID NodeSignal::waitSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    while( !(mSignal & signal) )
    {
        mCondition.wait(mMutex);
    }
}

MVOID NodeSignal::setStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    mStatus |= status;
}

MVOID NodeSignal::clearStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    mStatus &= ~status;
}

MBOOL NodeSignal::getStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    return (mStatus & status);
}

/*******************************************************************************
* DepthMapDataHandler Definition
********************************************************************************/
DataIDToBIDMap DepthMapPipeNode::mDataIDToBIDMap = getDataIDToBIDMap();

const char* DepthMapDataHandler::ID2Name(DataID id)
{
#define MAKE_NAME_CASE(name) \
  case name: return #name;

  switch(id)
  {
    MAKE_NAME_CASE(ID_INVALID);
    MAKE_NAME_CASE(ROOT_ENQUE);
    MAKE_NAME_CASE(BAYER_ENQUE);

    MAKE_NAME_CASE(P2A_TO_N3D_FEFM_CCin);
    MAKE_NAME_CASE(P2A_TO_N3D_NOFEFM_RECT1);
    MAKE_NAME_CASE(P2A_TO_N3D_NOFEFM_RECT2);
    MAKE_NAME_CASE(P2A_TO_FD_IMG);
    MAKE_NAME_CASE(P2A_TO_OCC_MY_S);
    MAKE_NAME_CASE(P2A_TO_HWOCC_MY_S);

    MAKE_NAME_CASE(N3D_TO_DPE_MVSV_MASK);
    MAKE_NAME_CASE(N3D_TO_FD_EXTDATA_MASK);
    MAKE_NAME_CASE(DPE_TO_OCC_MVSV_DMP_CFM);
    MAKE_NAME_CASE(OCC_TO_WMF_OMYSN);
    MAKE_NAME_CASE(WMF_TO_GF_DMW_MY_S);
    MAKE_NAME_CASE(WMF_TO_GF_OND);
    MAKE_NAME_CASE(DPE_TO_HWOCC_MVSV_DMP);
    MAKE_NAME_CASE(HWOCC_TO_WMF_NOC);
    MAKE_NAME_CASE(GF_OUT_DEPTHMAP);
    // DepthMap output
    MAKE_NAME_CASE(P2A_OUT_MV_F);
    MAKE_NAME_CASE(P2A_OUT_FD);
    MAKE_NAME_CASE(P2A_OUT_MV_F_CAP);
    MAKE_NAME_CASE(P2A_OUT_YUV_DONE);
    MAKE_NAME_CASE(P2A_OUT_DMBG);
    MAKE_NAME_CASE(GF_OUT_DMBG);
    MAKE_NAME_CASE(GF_OUT_INTERNAL_DMBG);
    MAKE_NAME_CASE(DPE_OUT_DISPARITY);
    MAKE_NAME_CASE(FD_OUT_EXTRADATA);
    MAKE_NAME_CASE(N3D_OUT_JPS_WARPMTX);
    MAKE_NAME_CASE(DEPTHMAP_META_OUT);
    MAKE_NAME_CASE(GF_OUT_DEPTH_WRAPPER);
    MAKE_NAME_CASE(HWOCC_OUT_3RD_NOC);

    #ifdef GTEST
    MAKE_NAME_CASE(UT_OUT_FE);
    #endif
    MAKE_NAME_CASE(TO_DUMP_BUFFERS);
    MAKE_NAME_CASE(TO_DUMP_IMG3O);
    MAKE_NAME_CASE(TO_DUMP_RAWS);
    MAKE_NAME_CASE(ERROR_OCCUR_NOTIFY);
    MAKE_NAME_CASE(QUEUED_FLOW_DONE);
    MAKE_NAME_CASE(REQUEST_DEPTH_NOT_READY);

  };
  MY_LOGW(" unknown id:%d", id);
  return "UNKNOWN";
#undef MAKE_NAME_CASE
}

DepthMapDataHandler::
DepthMapDataHandler()
{}

DepthMapDataHandler::
~DepthMapDataHandler()
{
}


DepthMapBufferID
DepthMapDataHandler::
mapQueuedBufferID(
    DepthMapRequestPtr pRequest,
    sp<DepthMapPipeOption> pOption,
    DepthMapBufferID bid)
{
    if(pRequest->isQueuedDepthRequest(pOption))
    {
        switch(bid)
        {
            case BID_META_IN_APP:
                return BID_META_IN_APP_QUEUED;
            case BID_META_IN_HAL_MAIN1:
                return BID_META_IN_HAL_MAIN1_QUEUED;
            case BID_META_IN_HAL_MAIN2:
                return BID_META_IN_HAL_MAIN2_QUEUED;
            case BID_META_OUT_APP:
                return BID_META_OUT_APP_QUEUED;
            case BID_META_OUT_HAL:
                return BID_META_OUT_HAL_QUEUED;
            case BID_META_IN_P1_RETURN:
                return BID_META_IN_P1_RETURN_QUEUED;
            default:
                break;
        }
    }
    return bid;
}
/*******************************************************************************
* DepthMapPipeNode Definition
********************************************************************************/
DepthMapPipeNode::
DepthMapPipeNode(
    const char *name,
    DepthMapPipeNodeID nodeID,
    PipeNodeConfigs config
)
: CamThreadNode(name)
, mpFlowOption(config.mpFlowOption)
, mpPipeOption(config.mpPipeOption)
, mNodeId(nodeID)
{
    miDumpBufSize = ::property_get_int32("vendor.dualfeature.dump.size", 0);
    miDumpStartIdx = ::property_get_int32("vendor.dualfeature.dump.start", 0);

    //3A operation may cost time, so we only extract by sensor id once
    if(miDumpBufSize > 0)
    {
        int main1Idx, main2Idx;
        StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
        extract_by_SensorOpenId(&mDumpHint_Main1, main1Idx);
        extract_by_SensorOpenId(&mDumpHint_Main2, main2Idx);
    }
    MY_LOGD("mbDebugLog=%d mbProfileLog=%d, miDumpBufSize=%d miDumpStartIdx=%d",
            DepthPipeLoggingSetup::mbDebugLog, DepthPipeLoggingSetup::mbProfileLog,
            miDumpBufSize, miDumpStartIdx);
}

DepthMapPipeNode::~DepthMapPipeNode()
{
}

MVOID
DepthMapPipeNode::
dumpSensorMapping(DepthMapRequestPtr &pRequest)
{
    if(checkToDump() && pRequest->getRequestAttr().opState == eSTATE_NORMAL)
    {
        const size_t PATH_SIZE = 1024;
        char filepath[PATH_SIZE];
        char writepath[PATH_SIZE];
        sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
        DepthMapBufferID inHalBID_Main1 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
        IMetadata* pInHalMeta_Main1 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main1);
        extract(&mDumpHint_Main1, pInHalMeta_Main1);
        //
        DepthMapBufferID inHalBID_Main2 = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN2);
        IMetadata* pInHalMeta_Main2 = pBufferHandler->requestMetadata(getNodeId(), inHalBID_Main2);
        extract(&mDumpHint_Main2, pInHalMeta_Main2);

        TuningUtils::MakePrefix(filepath, PATH_SIZE, mDumpHint_Main1.UniqueKey, mDumpHint_Main1.FrameNo, mDumpHint_Main1.RequestNo);
        // write file path
        snprintf(writepath, PATH_SIZE, "%s-%s_%d_%s_%d", filepath, SENSOR_DEV_TO_STRING(mDumpHint_Main1.SensorDev),
                    mDumpHint_Main1.FrameNo, SENSOR_DEV_TO_STRING(mDumpHint_Main2.SensorDev), mDumpHint_Main2.FrameNo);
        std::fstream fs;
        fs.open (writepath, std::fstream::out);
        fs.close();
    }
}

MBOOL
DepthMapPipeNode::
onDump(
    DataID id,
    DepthMapRequestPtr &pRequest,
    DumpConfig* config
)
{
#ifdef GTEST_PROFILE
    return MTRUE;
#endif
#ifndef GTEST
    if(!checkToDump(id) || pRequest->getRequestAttr().opState == eSTATE_STANDALONE)
    {
        VSDOF_LOGD("onDump reqID=%d dataid=%d(%s), checkDump failed! standalone=%d",
                    pRequest->getRequestNo(), id, ID2Name(id),
                    pRequest->getRequestAttr().opState == eSTATE_STANDALONE);
        return MFALSE;
    }
#endif
    MY_LOGD("%s onDump reqID=%d dataid=%d(%s)", getName(), pRequest->getRequestNo(), id, ID2Name(id));

    char* fileName = (config != NULL) ? config->fileName : NULL;
    char* postfix = (config != NULL) ? config->postfix : NULL;
    MBOOL bStridePostfix = (config != NULL) ? config->bStridePostfix : MFALSE;
    MUINT iReqIdx = pRequest->getRequestNo();

#ifndef GTEST
    // check dump index
    if(!checkDumpIndex(iReqIdx))
    {
        MY_LOGD("%s onDump reqID=%d dataid=%d(%s) request not in range!",
                getName(), iReqIdx, id, ID2Name(id));
        return MTRUE;
    }
#endif

    if(mDataIDToBIDMap.indexOfKey(id)<0)
    {
        MY_LOGD("%s onDump: reqID=%d, cannot find BID map of the data id:%d! Chk BaseBufferConfig.cpp",
                getName(), pRequest->getRequestNo(), id);
        return MFALSE;
    }

    const size_t PATH_SIZE = 1024;
    char filepath[PATH_SIZE];
    //Get metadata
    sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    DepthMapBufferID inHalBID = mapQueuedBufferID(pRequest, mpPipeOption, BID_META_IN_HAL_MAIN1);
    IMetadata* pInHalMeta = pBufferHandler->requestMetadata(getNodeId(), inHalBID);
    extract(&mDumpHint_Main1, pInHalMeta);

    // get the buffer id array for dumping
    const Vector<DepthMapBufferID>& vDumpBufferID = mDataIDToBIDMap.valueFor(id);
    char writepath[PATH_SIZE];
    char filename[PATH_SIZE];
    char strideStr[100];

    VSDOF_LOGD("dataID:%d buffer id size=%zu", id, vDumpBufferID.size());
    for(size_t i=0;i<vDumpBufferID.size();++i)
    {
        const DepthMapBufferID& oriBID = vDumpBufferID.itemAt(i);
        DepthMapBufferID BID = mpFlowOption->reMapBufferID(pRequest->getRequestAttr(), oriBID);

        VSDOF_LOGD("Dump -- index%zu, buffer id=%d", i, BID);
        IImageBuffer* pImgBuf = nullptr;
        MBOOL bRet = pBufferHandler->getEnqueBuffer(this->getNodeId(), BID, pImgBuf);
        if(!bRet)
        {
            VSDOF_LOGD("Failed to get enqued buffer, id: %d", BID);
            continue;
        }

        // stride string
        if(bStridePostfix) {
            snprintf(filename, PATH_SIZE, "%s_%d_reqID_%d", (fileName != NULL) ? fileName : onDumpBIDToName(BID),
                                                         pImgBuf->getBufStridesInBytes(0), pRequest->getRequestNo());
        } else {
            snprintf(filename, PATH_SIZE, "%s_reqID_%d", (fileName != NULL) ? fileName : onDumpBIDToName(BID), pRequest->getRequestNo());
        }

        extract(&mDumpHint_Main1, pImgBuf);
        switch(BID) {
            case BID_P2A_IN_FSRAW1:
            case BID_P2A_IN_FSRAW2:
                genFileName_RAW(writepath, PATH_SIZE, &mDumpHint_Main1, TuningUtils::RAW_PORT_IMGO, filename);
            break;
            case BID_P2A_IN_RSRAW1:
            case BID_P2A_IN_RSRAW2:
                genFileName_RAW(writepath, PATH_SIZE, &mDumpHint_Main1, TuningUtils::RAW_PORT_RRZO, filename);
            break;
            case BID_P2A_OUT_RECT_IN1:
            case BID_P2A_OUT_RECT_IN2:
            {
                ENUM_STEREO_SCENARIO eScenario = (pRequest->getRequestAttr().opState == eSTATE_CAPTURE)
                                                 ? eSTEREO_SCENARIO_CAPTURE
                                                 : eSTEREO_SCENARIO_PREVIEW;
                Pass2SizeInfo pass2info;
                ENUM_PASS2_ROUND p2Round = (BID_P2A_OUT_RECT_IN1 == BID) ? PASS2A_2 : PASS2A_P_2;
                StereoSizeProvider::getInstance()->getPass2SizeInfo(p2Round, eScenario, pass2info);
                mDumpHint_Main1.ImgWidth  = pass2info.areaWDMA.size.w;
                mDumpHint_Main1.ImgHeight = pass2info.areaWDMA.size.h;
                genFileName_YUV(writepath, PATH_SIZE, &mDumpHint_Main1, TuningUtils::YUV_PORT_UNDEFINED, filename);
            }
            break;
            case BID_P2A_IN_LCSO1:
            case BID_P2A_IN_LCSO2:
                genFileName_LCSO(writepath, PATH_SIZE, &mDumpHint_Main1, filename);
            break;
            default:
                genFileName_YUV(writepath, PATH_SIZE, &mDumpHint_Main1, TuningUtils::YUV_PORT_UNDEFINED, filename);
            break;
        }

        VSDOF_LOGD("saveToFile: %s", writepath);
        pImgBuf->saveToFile(writepath);
    }

    return MTRUE;
}

MBOOL
DepthMapPipeNode::
handleData(DataID id, DepthMapRequestPtr pReq)
{
#ifdef GTEST_PARTIAL
    return CamThreadNode<DepthMapDataHandler>::handleData(id, pReq);
#else
    MBOOL bConnect = mpFlowOption->checkConnected(id);
    if(bConnect)
    {
        CamThreadNode<DepthMapDataHandler>::handleData(id, pReq);
        return MTRUE;
    }
    return MFALSE;
#endif
}


MBOOL
DepthMapPipeNode::
handleDump(
    DataID id,
    DepthMapRequestPtr &request,
    DumpConfig* config
)
{
    return this->onDump(id, request, config);
}

MBOOL
DepthMapPipeNode::
handleDataAndDump(DataID id, DepthMapRequestPtr &request)
{
    // dump first and then handle data
    MBOOL bRet = this->onDump(id, request);
    bRet &= this->handleData(id, request);
    return bRet;
}

MBOOL DepthMapPipeNode::checkToDump(DataID id)
{
#ifdef GTEST
    return MTRUE;
#endif
    MINT32 iNodeDump =  getPropValue();

    if(getPropValue() == 0)
    {
        VSDOF_LOGD("node check failed!node: %s dataID: %d", getName(), id);
        return MFALSE;
    }

    if(getPropValue(id) == 0)
    {
        VSDOF_LOGD("dataID check failed!dataID: %d  %s", id, ID2Name(id));
        return MFALSE;
    }

    return MTRUE;
}

MBOOL DepthMapPipeNode::checkToDump()
{
#ifdef GTEST
    return MTRUE;
#endif
    MINT32 iNodeDump =  getPropValue();

    if(getPropValue() == 0)
    {
        VSDOF_LOGD("node check failed!node: %s ", getName());
        return MFALSE;
    }
    return MTRUE;
}

MBOOL
DepthMapPipeNode::checkDumpIndex(MUINT iReqIdx)
{
    return (iReqIdx >= miDumpStartIdx && iReqIdx < miDumpStartIdx + miDumpBufSize);
}

const char*
DepthMapPipeNode::
onDumpBIDToName(DepthMapBufferID BID)
{
#define MAKE_NAME_CASE(name) \
    case name: return #name;
    switch(BID)
    {
        MAKE_NAME_CASE(BID_P2A_IN_FSRAW1);
        MAKE_NAME_CASE(BID_P2A_IN_FSRAW2);
        MAKE_NAME_CASE(BID_P2A_IN_RSRAW1);
        MAKE_NAME_CASE(BID_P2A_IN_RSRAW2);
        MAKE_NAME_CASE(BID_P2A_IN_LCSO1);
        MAKE_NAME_CASE(BID_P2A_IN_LCSO2);
        // internal P2A buffers
        MAKE_NAME_CASE(BID_P2A_FE1B_INPUT);
        MAKE_NAME_CASE(BID_P2A_FE2B_INPUT);
        MAKE_NAME_CASE(BID_P2A_FE1C_INPUT);
        MAKE_NAME_CASE(BID_P2A_FE2C_INPUT);
        // P2A output
        MAKE_NAME_CASE(BID_P2A_OUT_FDIMG);
        MAKE_NAME_CASE(BID_P2A_OUT_FE1AO);
        MAKE_NAME_CASE(BID_P2A_OUT_FE2AO);
        MAKE_NAME_CASE(BID_P2A_OUT_FE1BO);
        MAKE_NAME_CASE(BID_P2A_OUT_FE2BO);
        MAKE_NAME_CASE(BID_P2A_OUT_FE1CO);
        MAKE_NAME_CASE(BID_P2A_OUT_FE2CO);
        MAKE_NAME_CASE(BID_P2A_OUT_RECT_IN1);
        MAKE_NAME_CASE(BID_P2A_OUT_RECT_IN2);
        MAKE_NAME_CASE(BID_P2A_OUT_MV_F);
        MAKE_NAME_CASE(BID_P2A_OUT_MV_F_CAP);
        MAKE_NAME_CASE(BID_P2A_OUT_CC_IN1);
        MAKE_NAME_CASE(BID_P2A_OUT_CC_IN2);
        MAKE_NAME_CASE(BID_P2A_OUT_FMAO_LR);
        MAKE_NAME_CASE(BID_P2A_OUT_FMAO_RL);
        MAKE_NAME_CASE(BID_P2A_OUT_FMBO_LR);
        MAKE_NAME_CASE(BID_P2A_OUT_FMBO_RL);
        MAKE_NAME_CASE(BID_P2A_OUT_FMCO_LR);
        MAKE_NAME_CASE(BID_P2A_OUT_FMCO_RL);
        MAKE_NAME_CASE(BID_P2A_OUT_MY_S);
        MAKE_NAME_CASE(BID_P2A_OUT_POSTVIEW);
        MAKE_NAME_CASE(BID_P2A_OUT_INTERNAL_FD);
        MAKE_NAME_CASE(BID_P2A_INTERNAL_IMG3O);
        MAKE_NAME_CASE(BID_P2A_TUNING);
        // N3D output
        MAKE_NAME_CASE(BID_N3D_OUT_MV_Y);
        MAKE_NAME_CASE(BID_N3D_OUT_MASK_M);
        MAKE_NAME_CASE(BID_N3D_OUT_SV_Y);
        MAKE_NAME_CASE(BID_N3D_OUT_MASK_S);
        MAKE_NAME_CASE(BID_N3D_OUT_LDC);
        MAKE_NAME_CASE(BID_N3D_OUT_JPS_MAIN1);
        MAKE_NAME_CASE(BID_N3D_OUT_JPS_MAIN2);
        MAKE_NAME_CASE(BID_N3D_OUT_WARPING_MATRIX);
        // DPE output
        MAKE_NAME_CASE(BID_DPE_OUT_CFM_L);
        MAKE_NAME_CASE(BID_DPE_OUT_CFM_R);
        MAKE_NAME_CASE(BID_DPE_OUT_DMP_L);
        MAKE_NAME_CASE(BID_DPE_OUT_DMP_R);
        MAKE_NAME_CASE(BID_DPE_OUT_RESPO_L);
        MAKE_NAME_CASE(BID_DPE_OUT_RESPO_R);
        // Last DMP
        MAKE_NAME_CASE(BID_DPE_INTERNAL_LAST_DMP);
        // OCC output
        MAKE_NAME_CASE(BID_OCC_OUT_OCC);
        MAKE_NAME_CASE(BID_OCC_OUT_NOC);
        MAKE_NAME_CASE(BID_OCC_OUT_DS_MVY);
        // WMF
        MAKE_NAME_CASE(BID_WMF_OUT_DMW);
        MAKE_NAME_CASE(BID_WMF_DMW_INTERNAL);
        MAKE_NAME_CASE(BID_WMF_HOLEFILL_INTERNAL);
        // FD
        MAKE_NAME_CASE(BID_FD_OUT_EXTRADATA);
        // GF
        MAKE_NAME_CASE(BID_GF_INTERNAL_DMBG);
        MAKE_NAME_CASE(BID_GF_OUT_DMBG);
        MAKE_NAME_CASE(BID_GF_OUT_DEPTH_WRAPPER);
        MAKE_NAME_CASE(BID_GF_INTERNAL_DEPTHMAP);
        MAKE_NAME_CASE(BID_GF_OUT_DEPTH);

        #ifdef GTEST
        // UT output
        MAKE_NAME_CASE(BID_FE2_HWIN_MAIN1);
        MAKE_NAME_CASE(BID_FE2_HWIN_MAIN2);
        MAKE_NAME_CASE(BID_FE3_HWIN_MAIN1);
        MAKE_NAME_CASE(BID_FE3_HWIN_MAIN2);
        #endif

        MAKE_NAME_CASE(BID_META_IN_APP);
        MAKE_NAME_CASE(BID_META_IN_HAL_MAIN1);
        MAKE_NAME_CASE(BID_META_IN_HAL_MAIN2);
        MAKE_NAME_CASE(BID_META_OUT_APP);
        MAKE_NAME_CASE(BID_META_OUT_HAL);
        MAKE_NAME_CASE(BID_META_IN_APP_QUEUED);
        MAKE_NAME_CASE(BID_META_IN_HAL_MAIN1_QUEUED);
        MAKE_NAME_CASE(BID_META_IN_HAL_MAIN2_QUEUED);
        MAKE_NAME_CASE(BID_META_OUT_APP_QUEUED);
        MAKE_NAME_CASE(BID_META_OUT_HAL_QUEUED);
        MAKE_NAME_CASE(BID_PQ_PARAM);
        MAKE_NAME_CASE(BID_DP_PQ_PARAM);

        default:
            MY_LOGW("unknown BID:%d", BID);
            return "unknown";
    }

#undef MAKE_NAME_CASE
}


}; // namespace NSFeaturePipe_DepthMap
}; // namespace NSCamFeature
}; // namespace NSCam
