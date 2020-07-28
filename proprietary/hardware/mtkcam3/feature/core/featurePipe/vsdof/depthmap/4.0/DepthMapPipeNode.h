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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
 * @file DeptMapPipeNode.h
 * @brief Base class of the DepthMapPipe's feature pipe node
 */
#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_NODE_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_NODE_H_

// Standard C header file

// Android system/core header file
#include <utils/Vector.h>
#include <utils/String8.h>
#include <utils/Condition.h>

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>

// Module header file
#include <featurePipe/core/include/CamNodeULog.h>
#include <featurePipe/core/include/CamThreadNode.h>

// Local header file
#include "DepthMapPipe_Common.h"
#include "DepthMapEffectRequest.h"
#include "./flowOption/DepthMapFlowOption.h"
#include "./bufferConfig/BaseBufferConfig.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

/*******************************************************************************
* Struct Definition
********************************************************************************/

struct PipeNodeConfigs
{
    PipeNodeConfigs(
        sp<DepthMapFlowOption> pFlowOption,
        sp<DepthMapPipeSetting> pSetting,
        sp<DepthMapPipeOption> pPipeOption
    )
    : mpFlowOption(pFlowOption)
    , mpSetting(pSetting)
    , mpPipeOption(pPipeOption) {}

    sp<DepthMapFlowOption> mpFlowOption;
    sp<DepthMapPipeSetting> mpSetting;
    sp<DepthMapPipeOption> mpPipeOption;
};

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class NodeSignal
 * @brief Use the to notify the status
 **/
class NodeSignal : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    NodeSignal();
    virtual ~NodeSignal();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeSignal Public Function/Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    enum Signal
    {
    };

    enum Status
    {
        STATUS_IN_FLUSH = 0x01 << 0,
        STATUS_IN_ERROR = 0x01 << 1,
    };
    MVOID setSignal(Signal signal);
    MVOID clearSignal(Signal signal);
    MBOOL getSignal(Signal signal);
    MVOID waitSignal(Signal signal);

    MVOID setStatus(Status status);
    MVOID clearStatus(Status status);
    MBOOL getStatus(Status status);
private:
    android::Mutex mMutex;
    android::Condition mCondition;
    MUINT32 mSignal;
    MUINT32 mStatus;
};

/**
 * @class DepthMapDataHandler
 * @brief Data hanlder for depthmap pipe node
 */
class DepthMapDataHandler : virtual public NSFeaturePipe::CamNodeULogHandler
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef DepthMapDataID DataID;
    DepthMapDataHandler();
    virtual ~DepthMapDataHandler();
public:
    virtual MBOOL onData(DataID id, DepthMapRequestPtr &data) { return MFALSE; }
    // dump buffer
    virtual MBOOL onDump(
                DataID id,
                DepthMapRequestPtr &request,
                const char* fileName=NULL,
                const char* postfix=NULL) { return MFALSE; }
    // ID to name
    static const char* ID2Name(DataID id);
    /*
     * @brief Map buffer id into another if needed during QUEUED_DEPTH flow
     * @param [in] pRequest depth effect request
     * @param [in] bid buffer ID
     * @return
     * - Remapped buffer id
     */
    DepthMapBufferID mapQueuedBufferID(
                                DepthMapRequestPtr pRequest,
                                sp<DepthMapPipeOption> pOption,
                                DepthMapBufferID bid);
};

/**
 * @class DepthMapPipeNode
 * @brief nodes inside the depthmap pipe
 */
class DepthMapPipeNode : public DepthMapDataHandler, public CamThreadNode<DepthMapDataHandler>
{
public:
    typedef CamGraph<DepthMapPipeNode> Graph_T;
    typedef DepthMapDataHandler Handler_T;

public:
    DepthMapPipeNode(
                const char *name,
                DepthMapPipeNodeID nodeID,
                PipeNodeConfigs config);

    virtual ~DepthMapPipeNode();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CamThreadNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL onInit()           { return MTRUE; }
    virtual MBOOL onUninit()         { return MTRUE; }
    virtual MBOOL onThreadStart()    { return MTRUE; }
    virtual MBOOL onThreadStop()     { return MTRUE; }
    virtual MBOOL onThreadLoop() = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapDataHandler Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    struct DumpConfig
    {
        DumpConfig() {}
        DumpConfig(char* fname, char* post, MBOOL stride)
        : fileName(fname), postfix(post), bStridePostfix(stride) {}

        char* fileName=NULL;
        char* postfix=NULL;
        MBOOL bStridePostfix = MFALSE;
    };
    // dump buffer
    virtual MBOOL onDump(
                DataID id,
                DepthMapRequestPtr &request,
                DumpConfig* config=NULL);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief handle data for DepthMapNode
     * @param [in] id Data id
     * @param [in] pReq request to pass
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL handleData(DataID id, DepthMapRequestPtr pReq);

    /**
     * @brief perform dump operation
     * @param [in] id Data id
     * @param [in] request request to dump
     * @param [in] config dump config
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL handleDump(
                DataID id,
                DepthMapRequestPtr &request,
                DumpConfig* config=NULL);

    /**
     * @brief perform handleData and dumpping buffer
     */
    MBOOL handleDataAndDump(DataID id, DepthMapRequestPtr &request);

    /**
     * @brief get node id of this node
     */
    DepthMapPipeNodeID getNodeId() {return mNodeId;}
    // used for UT
    template <typename Type3>
    MVOID handleUTData(DataID id, Type3& buffer)
    {
      VSDOF_MDEPTH_LOGD("handleUTData aa");
      this->onData(id, buffer);
    }
    /**
     * @brief set the pipe NodeSignal instance
     */
    MVOID setNodeSignal(sp<NodeSignal> pSignal) {mpNodeSignal = pSignal;}
    /**
     * @brief set the pipe DepthInfoStorage instance
     */
    MVOID setDepthStorage(sp<DepthInfoStorage> pStorage) {mpDepthStorage = pStorage;}
    /**
     * @brief set the buffer pool mgr instance
     */
    MVOID setBufferPoolMgr(sp<BaseBufferPoolMgr> pMgr) {mpBuffPoolMgr = pMgr;}
    /**
     * @brief dump the main1/main2 mapping
     */
    MVOID dumpSensorMapping(DepthMapRequestPtr &request);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    // check the buffer of DataID will dump or not
    virtual MBOOL checkToDump(DataID id, DepthMapRequestPtr pRequest);
    virtual MBOOL checkDumpIndex(MUINT iReqIdx);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Public Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static const char* onDumpBIDToName(DepthMapBufferID BID);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    static DataIDToBIDMap mDataIDToBIDMap;
    // Dump buffer index
    MUINT miDumpBufSize;
    MUINT miDumpStartIdx;
    MBOOL mbDumpCapture;
    // flow option instance
    sp<DepthMapFlowOption> mpFlowOption = nullptr;
    // node signal
    sp<NodeSignal> mpNodeSignal = nullptr;
    // pipe option
    sp<DepthMapPipeOption> mpPipeOption = nullptr;
    // data storage
    sp<DepthInfoStorage> mpDepthStorage = nullptr;
    // node id
    DepthMapPipeNodeID mNodeId;
    // buffer pool mgr
    sp<BaseBufferPoolMgr> mpBuffPoolMgr = nullptr;
    // Dump hint, for generating dump path
    TuningUtils::FILE_DUMP_NAMING_HINT mDumpHint_Main1;
    TuningUtils::FILE_DUMP_NAMING_HINT mDumpHint_Main2;
private:
};

}; // namespace NSFeaturePipe_DepthMap
}; // namespace NSCamFeature
}; // namespace NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_NODE_H_
