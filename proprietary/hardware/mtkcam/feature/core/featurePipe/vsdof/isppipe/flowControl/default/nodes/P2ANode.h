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
 * @file P2ANode.h
 * @brief Pass2 node for isp pipe
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_P2A_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_P2A_H_

// Standard C header file

// Android system/core header file
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>
#include <utils/Vector.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <featurePipe/vsdof/util/QParamTemplate.h>
// Module header file
#include <featurePipe/core/include/WaitQueue.h>
// Local header file
#include "../DefaultIspPipeFlow_Common.h"
#include "../../../IspPipeNode.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

using android::KeyedVector;
using android::Vector;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NS3Av3;
using PortID = NSCam::NSIoPipe::PortID;

/*******************************************************************************
* Enum Definition
********************************************************************************/

typedef enum eIspP2Pass
{
    eP2_DPE_MAIN1,
    eP2_DPE_MAIN2,
    eP2_RESIZE,     //
    eP2_FULLSIZE
} IspP2Pass;
/*******************************************************************************
* Struct Definition
********************************************************************************/
/**
 * @class EnqueCookieContainer
 * @brief
 */
struct EnqueCookieContainer
{
public:
    EnqueCookieContainer(sp<IspPipeRequest> req, IspPipeNode* pNode)
    : mRequest(req), mpNode(pNode) {}
public:
    sp<IspPipeRequest> mRequest;
    IspPipeNode* mpNode;
};

/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class P2ANode
 * @brief Node class for Pass2 operation
 */

class P2ANode : public IspPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    P2ANode(
        const char *name,
        IspPipeNodeID nodeID,
        const PipeNodeConfigs& config);
    virtual ~P2ANode();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL onData(DataID id, const IspPipeRequestPtr& request);
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadLoop();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MVOID onFlush();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeNode Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  P2ANode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    /**
     * @brief NormalStream success callback function
     * @param [in] rParams dequeued QParams
     */
    static MVOID onP2Callback(QParams& rParams);
    MVOID handleP2Done(QParams& rParams, EnqueCookieContainer* pEnqueData);
    /**
     * @brief NormalStream failure callback function
     * @param [in] rParams dequeued QParams
     */
    static MVOID onP2FailedCallback(QParams& rParams);
    /**
     * @brief resource uninit/cleanup function
     */
    MVOID cleanUp();
    /**
     * @brief 3A isp tuning function
     * @param [in] pass P2 pass type
     * @param [in] pRequest Current request
     * @return
     * - Tuning result
     */
    TuningParam applyISPTuning(
                        IspP2Pass pass,
                        const IspPipeRequestPtr& pRequest);
    /**
     * @brief prepare the Qparam to enque
     * @param [in] pRequest Current request
     * @param [out] rQParams QParams to be output
     * @return
     * - MTRUE: success
     * - MFALSE: failure
     */
    MBOOL prepareEnqueQParam(
                    const IspPipeRequestPtr& pRequest,
                    QParams& rQParams);
    /**
     * @brief Build the Qparam to enque
     * @param [in] pRequest Current request
     * @param [out] rQParams QParams to be output
     * @return
     * - Tuning result
     */
    MBOOL buildEnqueQParam(
                    const IspPipeRequestPtr& pRequest,
                    QParams& rQParams);
    /**
     * @brief check the enque request is valid or not
     * @param [in] pRequest request
     * @return
     * - MTRUE: valid to enque
     * - MFALSE non-valid to enque
     */
    MBOOL checkEnqueValid(const IspPipeRequestPtr& pRequest);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeNode Private member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // request queue
    WaitQueue<IspPipeRequestPtr> mRequestQue;
    // P2 NormalStream
    INormalStream*  mpINormalStream = nullptr;
    // 3A hal
    IHal3A* mp3AHal_Main1 = nullptr;
    IHal3A* mp3AHal_Main2 = nullptr;
    // sensor index
    MUINT32 miSensorIdx_Main1;
    MUINT32 miSensorIdx_Main2;
};


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam


#endif