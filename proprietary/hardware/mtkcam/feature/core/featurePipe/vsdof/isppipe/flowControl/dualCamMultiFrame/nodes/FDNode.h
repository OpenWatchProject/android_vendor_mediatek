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
 * @file FDNode.h
 * @brief FD node for isp pipe
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_FD_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_FD_H_

// Standard C header file
#include <memory>
#include <vector>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <DpBlitStream.h>
#include <DpDataType.h>
#include <mtkcam/feature/FaceDetection/fd_hal_base.h>
// Module header file

// Local header file
#include "../DCMFIspPipeFlow_Common.h"
#include "../../../IspPipeNode.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {


/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class FDNode
 * @brief FD Node for IspPipe DCMF flow
 */
class FDNode: public IspPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    FDNode(
            const char *name,
            IspPipeNodeID nodeId,
            const PipeNodeConfigs& config);
    FDNode(
            const char *name,
            IspPipeNodeID nodeId,
            const PipeNodeConfigs& config,
            int policy,
            int priority);
    virtual ~FDNode();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL onData(DataID id, const IspPipeRequestPtr& request) override;
    MBOOL onInit() override;
    MBOOL onUninit() override;
    MBOOL onThreadLoop() override;
    MBOOL onThreadStart() override;
    MBOOL onThreadStop() override;
    MVOID onFlush() override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FDNode Private Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief handle the output frame arrive
     * @param [in] pRequest the output grame
     * @return
     * - MTRUE: success
     * - MFALSE: failure
     */
    MBOOL handleOutputFrame(IspPipeRequestPtr& pRequest);
    /**
     * @brief generate the FD YUV for algo input
     * @param [in] pRequest the output grame
     * @return
     * - MTRUE: success
     * - MFALSE: failure
     */
    MBOOL generateFDYUV(IspPipeRequestPtr& pRequest);
    /**
     * @brief run the face detection algo
     * @param [in] pRequest the output grame
     * @return
     * - MTRUE: success
     * - MFALSE: failure
     */
    MBOOL runFaceDetection(IspPipeRequestPtr& pRequest);
    /**
     * @brief tranform the FD algo result into capture image domain (left-top is origin)
     * @param [in] domainSize new domain size
     * @return
     * - MTRUE: success
     * - MFALSE: failure
     */
    MBOOL tranformFaceDomain(const MSize& domainSize);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FDNode Private Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // request queue
    WaitQueue<IspPipeRequestPtr> mRequestQue;
    halFDBase* mpFDHalObj = nullptr;
    MUINT8* mpFDWorkingBuffer = nullptr;
    MUINT8* mpDDPBuffer = nullptr;
    MUINT8* mpExtractYBuffer = nullptr;
    MtkCameraFaceMetadata* mpDetectedFaces = nullptr;
    // MDP
    DpBlitStream* mpDpStream = nullptr;;
    //
    MSize mszFDImg;
    // store the latest FD result
    android::Mutex mLock;
    MBOOL mbFDResultReady = MFALSE;
    FDResultInfo mLatestFDResult;
};

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif