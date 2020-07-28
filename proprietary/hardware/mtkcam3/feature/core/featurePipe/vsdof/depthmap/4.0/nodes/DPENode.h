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
 * @file DPENode.h
 * @brief DPENode inside DepthMapPipe
*/

#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_DPE_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_DPE_NODE_H

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/iopipe/PostProc/IHalEgnPipe.h>
#include <mtkcam/drv/iopipe/PostProc/IEgnStream.h>
#include <mtkcam/drv/def/dpecommon_v20.h>
// Module header file
#include <featurePipe/core/include/WaitQueue.h>
// Local header file
#include "../DepthMapPipeNode.h"
#include "../DepthMapPipe_Common.h"
#include "../DepthMapPipeUtils.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace StereoHAL;
using namespace NSCam::NSIoPipe::NSEgn;
using NSCam::NSIoPipe::DPEConfig;

/**
 * @class DPENode
 * @brief DPE feature pipe node
 */
class DPENode: public DepthMapPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DPENode() = delete;

    DPENode(
        const char *name,
        DepthMapPipeNodeID nodeID,
        PipeNodeConfigs config);

    virtual ~DPENode();
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
//  DPENode Public Operations
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DPENode Private Operations
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief cleanup function - release hal/resources
     */
    MVOID cleanUp();
    /**
     * @brief prepare DPE enque params
     * @param [in] pRequest effect request
     * @param [out] rEnqParam DPE enque params
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareDPEEnqueConfig_PVVR(
                        DepthMapRequestPtr pRequest,
                        EGNParams<DPEConfig>& rEnqParam);

    /**
     * @brief DPEStream success callback function
     */
    static MVOID onDPEEnqueDone(EGNParams<DPEConfig>& rParams);
    /**
     * @brief DPE enqued-operation finished handle function
     * @param [in] rParams dequed DVEParam
     * @param [in] pEnqueCookie EnqueCookie instance
     */
    MVOID handleDPEEnqueDone(
                        EGNParams<DPEConfig>& rParams,
                        EnqueCookieContainer* pEnqueCookie);
    MVOID debugDPEConfig(DPEConfig& config);

private:
    // job queue
    WaitQueue<DepthMapRequestPtr> mJobQueue;
    WaitQueue<DepthMapRequestPtr> mJobQueue_MYS;
    MBOOL mbFirst = MFALSE;
    // Last frame DMP
    SmartFatImageBuffer mLastDV_LR = nullptr;
    // DPE stream
    IEgnStream<DPEConfig>* mpDPEStream = nullptr;
    //
    MBOOL mbIsWaitDepth = MFALSE;
};

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam

#endif