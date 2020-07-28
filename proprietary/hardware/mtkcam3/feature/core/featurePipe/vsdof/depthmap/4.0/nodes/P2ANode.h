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
 * @brief P2A node
*/

#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_P2A_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_P2A_NODE_H

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <utils/Mutex.h>
#include <utils/Vector.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <mtkcam3/feature/3dnr/3dnr_defs.h>
#include <common/3dnr/3dnr_hal_base.h>
// Module header file
#include <featurePipe/core/include/WaitQueue.h>
#include <featurePipe/core/include/GraphicBufferPool.h>
#include <featurePipe/core/include/TuningBufferPool.h>

// Local header file
#include "../DepthMapPipeNode.h"
#include "../DepthMapPipe_Common.h"
#include "../DepthMapPipeUtils.h"
#include "../flowOption/P2AFlowOption.h"
#include "../DataStorage.h"
#include "../DataSequential.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using android::KeyedVector;
using android::Vector;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NS3Av3;
using namespace StereoHAL;

/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class P2ANode
 * @brief Node class for P2A
 */
class P2ANode
: public DepthMapPipeNode
, public DataSequential<DepthMapRequestPtr>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    P2ANode() = delete;
    P2ANode(
        const char *name,
        DepthMapPipeNodeID nodeID,
        PipeNodeConfigs config);
    virtual ~P2ANode();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL onData(DataID id, DepthMapRequestPtr &request);
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadLoop();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MVOID onFlush();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief NormalStream success callback function
     * @param [in] rParams dequeued QParams
     */
    static MVOID onP2Callback(QParams& rParams);
    MVOID handleP2Done(QParams& rParams, EnqueCookieContainer* pEnqueData);
    /**
     * @brief configure in/out and pass to next nodes
     */
    MBOOL configureToNext(DepthMapRequestPtr pRequest);
    /**
     * @brief resource uninit/cleanup function
     */
    MVOID cleanUp();
    /**
     * @brief 3A isp tuning function
     * @param [in] rEffectReqPtr Current effect request
     * @param [in] bIsMain1Path Main1 Path or not
     * @return
     * - Tuning result
     */
    AAATuningResult applyISPTuning(
                        DepthMapRequestPtr& rEffectReqPtr,
                        StereoP2Path p2aPath);

    /**
     * @brief Perform 3A Isp tuning and generate the Stereo3ATuningRes
     * @param [in] rEffectReqPtr Current effect request
     * @param [out] rOutTuningRes Output tungin result
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL perform3AIspTuning(DepthMapRequestPtr& rEffectReqPtr, Stereo3ATuningRes& rOutTuningRes);

    /**
     * @brief handle the on-going request data finish
     * @param [in] iReqID finished request id
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL onHandleOnGoingReqReady(MUINT32 iReqID);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  P2ANode Private members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // request queue
    WaitQueue<DepthMapRequestPtr> mRequestQue;
    // P2 NormalStream
    INormalStream*  mpINormalStream = NULL;
    // ISP hal
    IHalISP* mpIspHal_Main1 = NULL;
    IHalISP* mpIspHal_Main2 = NULL;
    // sensor index
    MUINT32 miSensorIdx_Main1;
    MUINT32 miSensorIdx_Main2;
};

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam


#endif //_MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_P2A_NODE_H
