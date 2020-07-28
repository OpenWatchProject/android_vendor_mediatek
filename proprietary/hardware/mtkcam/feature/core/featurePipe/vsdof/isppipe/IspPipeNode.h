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
 * @file IspPipeNode.h
 * @brief Base class of the ThirdParty IspPipe's feature pipe node
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_H_

// Standard C header file

// Android system/core header file
#include <utils/Vector.h>
#include <utils/String8.h>
#include <utils/Condition.h>

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <featurePipe/core/include/CamThreadNode.h>

// Local header file
#include "IspPipe_Common.h"
#include "IspPipeRequest.h"
#include "IIspPipeFlow_Interface.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

/*******************************************************************************
* Struct Definition
********************************************************************************/
struct PipeNodeConfigs
{
    PipeNodeConfigs(
        sp<IspPipeSetting> pPipeSetting,
        sp<IspPipeOption> pPipeOption
    )
    : mpPipeSetting(pPipeSetting)
    , mpPipeOption(pPipeOption) {}

    sp<IspPipeSetting> mpPipeSetting;
    sp<IspPipeOption> mpPipeOption;
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
 * @class IspPipeDataHandler
 * @brief Data hanlder for isp pipe node
 */
class IspPipeDataHandler
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef IspPipeDataID DataID;
    IspPipeDataHandler();
    virtual ~IspPipeDataHandler();
public:
    // dump config struct
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
                const IspPipeRequestPtr &request,
                DumpConfig* config=NULL) { return MFALSE; }

    virtual MBOOL onData(DataID, const IspPipeRequestPtr&) { return MFALSE; }
    // ID to name
    static const char* ID2Name(DataID id);
};


/**
 * @class IspPipeNode
 * @brief nodes inside the ThirdParty isp pipe
 */

class IspPipeNode
: public IspPipeDataHandler
, public CamThreadNode<IspPipeDataHandler>
{
public:
    typedef CamGraph<IspPipeNode> Graph_T;
    typedef IspPipeDataHandler Handler_T;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    IspPipeNode(
            const char *name,
            IspPipeNodeID nodeId,
            const PipeNodeConfigs& config);

    IspPipeNode(
            const char *name,
            IspPipeNodeID nodeId,
            const PipeNodeConfigs& config,
            int policy,
            int priority);

    virtual ~IspPipeNode();

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
//  IspPipeDataHandler Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief set IDataIDToBIDMapProvider
     * @param [in] the new IDataIDToBIDMapProvider
     */
    MVOID setDataIDToBIDMapProvider(sp<IDataIDToBIDMapProvider> provider);

    /**
     * @brief set IIspPipeConverter
     * @param [in] the new IIspPipeConverter
     */
    MVOID setIspPipeConverter(sp<IIspPipeConverter> conveter);

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
                const IspPipeRequestPtr& request,
                DumpConfig* config=NULL);

    /**
     * @brief perform handleData and dumpping buffer
     */
    MBOOL handleDataAndDump(DataID id, const IspPipeRequestPtr& request);

    /**
     * @brief get node id of this node
     */
    IspPipeNodeID getNodeId() {return mNodeId;}

    // dump buffer
    virtual MBOOL onDump(
                DataID id,
                const IspPipeRequestPtr& request,
                DumpConfig* config=NULL);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeDataHandler Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    // By node define the
    virtual const char* onDumpBIDToName(IspPipeBufferID BID);
    // check the buffer of DataID will dump or not
    virtual MBOOL checkToDump(DataID id);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief set the pipe NodeSignal instance
     */
    MVOID setNodeSignal(sp<NodeSignal> pSignal) {mpNodeSignal = pSignal;}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    // node signal
    sp<NodeSignal> mpNodeSignal = nullptr;
    // node id
    IspPipeNodeID mNodeId;
    // Dump buffer index
    MUINT miDumpBufSize;
    MUINT miDumpStartIdx;
    // setting/option
    sp<IspPipeSetting> mpPipeSetting;
    sp<IspPipeOption> mpPipeOption;
    // Data ID to BID (used in Dump)
    sp<IDataIDToBIDMapProvider> mDataIDToBIDMapProvider;
    // covert id to string (used in Dump)
    sp<IIspPipeConverter> mIspPipeConverter;

};


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif