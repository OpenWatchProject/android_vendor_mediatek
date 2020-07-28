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

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <featurePipe/core/include/DebugUtil.h>
// Local header file
#include "DepthMapPipeNode.h"
#include "./bufferPoolMgr/BaseBufferHandler.h"
#include "./bufferConfig/BaseBufferConfig.h"
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
MUINT32 DepthMapPipeNode::miTimestamp = 0;

const char* DepthMapDataHandler::ID2Name(DataID id)
{
#define MAKE_NAME_CASE(name) \
  case name: return #name;

  switch(id)
  {
    MAKE_NAME_CASE(ID_INVALID);
    MAKE_NAME_CASE(ROOT_ENQUE);

    MAKE_NAME_CASE(P2A_TO_N3D_FEFM_CCin);
    MAKE_NAME_CASE(P2A_TO_FD_IMG);
    MAKE_NAME_CASE(P2A_TO_OCC_MY_S);
    MAKE_NAME_CASE(P2A_TO_GF_DMW_MYS);

    MAKE_NAME_CASE(N3D_TO_DPE_MVSV_MASK);
    MAKE_NAME_CASE(N3D_TO_OCC_LDC);
    MAKE_NAME_CASE(N3D_TO_FD_EXTDATA_MASK);
    MAKE_NAME_CASE(DPE_TO_OCC_MVSV_DMP_CFM);
    MAKE_NAME_CASE(OCC_TO_WMF_OMYSN);
    MAKE_NAME_CASE(WMF_TO_GF_DMW_MY_S);
    MAKE_NAME_CASE(WMF_TO_GF_OND);

    // DepthMap output
    MAKE_NAME_CASE(P2A_OUT_MV_F);
    MAKE_NAME_CASE(P2A_OUT_FD);
    MAKE_NAME_CASE(P2A_OUT_MV_F_CAP);
    MAKE_NAME_CASE(P2A_OUT_YUV_DONE);
    MAKE_NAME_CASE(P2A_OUT_TUNING_BUF);
    MAKE_NAME_CASE(GF_OUT_DMBG);
    MAKE_NAME_CASE(DPE_OUT_DISPARITY);
    MAKE_NAME_CASE(FD_OUT_EXTRADATA);
    MAKE_NAME_CASE(N3D_OUT_JPS_WARPMTX);
    MAKE_NAME_CASE(DEPTHMAP_META_OUT);
    MAKE_NAME_CASE(GF_OUT_DEPTH_WRAPPER);

    #ifdef GTEST
    MAKE_NAME_CASE(UT_OUT_FE);
    #endif
    MAKE_NAME_CASE(TO_DUMP_BUFFERS);
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
    miDumpBufSize = ::property_get_int32("vendor.depthmap.pipe.dump.size", 0);
    miDumpStartIdx = ::property_get_int32("vendor.depthmap.pipe.dump.start", 0);

#ifdef GTEST
    MY_LOGD("mbDebugLog=%d mbProfileLog=%d, GTEST: 1 miDumpBufSize=%d miDumpStartIdx=%d",
            DepthPipeLoggingSetup::mbDebugLog, DepthPipeLoggingSetup::mbProfileLog);
#else
    MY_LOGD("mbDebugLog=%d mbProfileLog=%d, GTEST: 0",
            DepthPipeLoggingSetup::mbDebugLog, DepthPipeLoggingSetup::mbProfileLog);
#endif
}

DepthMapPipeNode::~DepthMapPipeNode()
{
}

MBOOL
DepthMapPipeNode::
onDump(
    DataID id,
    DepthMapRequestPtr &pRequest,
    DumpConfig* config
)
{
    if(!checkToDump(id))
    {
        VSDOF_LOGD("onDump reqID=%d dataid=%d(%s), checkDump failed!", pRequest->getRequestNo(), id, ID2Name(id));
        return MFALSE;
    }
    MY_LOGD("%s onDump reqID=%d dataid=%d(%s)", getName(), pRequest->getRequestNo(), id, ID2Name(id));

    char* fileName = (config != NULL) ? config->fileName : NULL;
    char* postfix = (config != NULL) ? config->postfix : NULL;
    MBOOL bStridePostfix = (config != NULL) ? config->bStridePostfix : MFALSE;

    MUINT iReqIdx = pRequest->getRequestNo();

#ifndef GTEST
    // check dump index
    if(iReqIdx < miDumpStartIdx || iReqIdx >= miDumpStartIdx + miDumpBufSize)
        return MTRUE;
#endif

    if(mDataIDToBIDMap.indexOfKey(id)<0)
    {
        MY_LOGD("%s onDump: reqID=%d, cannot find BID map of the data id:%d! Chk BaseBufferConfig.cpp",
                getName(), pRequest->getRequestNo(), id);
        return MFALSE;
    }

    const char *dir = (StereoSettingProvider::stereoProfile() == STEREO_SENSOR_PROFILE_FRONT_FRONT) ?
                        "Front" : "Rear";

    // generate file path
    char filepath[1024];
    if(pRequest->getRequestAttr().opState == eSTATE_NORMAL)
        snprintf(filepath, 1024, "/sdcard/vsdof/pv_vr/%s/%d/%s", dir, iReqIdx, getName());
    else
        snprintf(filepath, 1024, "/sdcard/vsdof/capture/%s/%d/%s", dir, iReqIdx, getName());

    // make path
    VSDOF_LOGD("makePath: %s", filepath);
    makePath(filepath, 0660);

    // get the buffer id array for dumping
    const Vector<DepthMapBufferID>& vDumpBufferID = mDataIDToBIDMap.valueFor(id);
     sp<BaseBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    char writepath[1024];
    char strideStr[100];

    VSDOF_LOGD("dataID:%d buffer id size=%d", id, vDumpBufferID.size());
    for(size_t i=0;i<vDumpBufferID.size();++i)
    {
        const DepthMapBufferID& oriBID = vDumpBufferID.itemAt(i);
        DepthMapBufferID BID = mpFlowOption->reMapBufferID(pRequest->getRequestAttr(), oriBID);

        VSDOF_LOGD("Dump -- index%d, buffer id=%d", i, BID);
        IImageBuffer* pImgBuf = nullptr;
        MBOOL bRet = pBufferHandler->getEnqueBuffer(this->getNodeId(), BID, pImgBuf);
        if(!bRet)
        {
            VSDOF_LOGD("Failed to get enqued buffer, id: %d", BID);
            continue;
        }
        // stride string
        if(bStridePostfix)
            snprintf(strideStr, 100, "_%d", pImgBuf->getBufStridesInBytes(0));
        else
            snprintf(strideStr, 100, "");

        const char* writeFileName = (fileName != NULL) ? fileName : onDumpBIDToName(BID);
        const char* postfixName = (postfix != NULL) ? postfix : "";

        if(strchr(postfixName, '.') != NULL)
            snprintf(writepath, 1024, "%s/%s_%dx%d%s%s", filepath, writeFileName,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, strideStr, postfixName);
        else
        {
            snprintf(writepath, 1024, "%s/%s_%dx%d%s%s.yuv", filepath, writeFileName,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, strideStr, postfixName);
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
        VSDOF_LOGD("dataID check failed!dataID: %d", id);
        return MFALSE;
    }

    return MTRUE;
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
        MAKE_NAME_CASE(BID_GF_INTERAL_DEPTHMAP);
        MAKE_NAME_CASE(BID_GF_OUT_DMBG);
        MAKE_NAME_CASE(BID_GF_OUT_DEPTH_WRAPPER);

        #ifdef GTEST
        // UT output
        MAKE_NAME_CASE(BID_FE2_HWIN_MAIN1);
        MAKE_NAME_CASE(BID_FE2_HWIN_MAIN2);
        MAKE_NAME_CASE(BID_FE3_HWIN_MAIN1);
        MAKE_NAME_CASE(BID_FE3_HWIN_MAIN2);
        #endif

        default:
            MY_LOGW("unknown BID:%d", BID);
            return "unknown";
    }

#undef MAKE_NAME_CASE
}


}; // namespace NSFeaturePipe_DepthMap
}; // namespace NSCamFeature
}; // namespace NSCam