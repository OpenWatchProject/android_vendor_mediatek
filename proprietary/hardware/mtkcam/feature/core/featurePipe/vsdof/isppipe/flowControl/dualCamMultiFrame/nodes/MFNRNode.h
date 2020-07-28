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
 * @file MFNRNode.h
 * @brief third-party node for isp pipe
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_MFNR_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_MFNR_H_

// Standard C header file
#include <memory>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
// MFNR Core library
#include <mtkcam/feature/mfnr/MfllTypes.h>
#include <mtkcam/feature/mfnr/IMfllNvram.h>
#include <mtkcam/feature/mfnr/IMfllStrategy.h>
#include <mtkcam/feature/mfnr/IMfllCore.h>
#include <mtkcam/feature/mfnr/IMfllEventListener.h>

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
 * @class MFNRNode
 * @brief MFNRNode Node for IspPipe DCMF flow
 */
class MFNRWrap;
class MFNRNode: public IspPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MFNRNode Public Type Alias.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    using RequestCollection = std::deque<IspPipeRequestPtr>;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MFNRNode Private Type Alias.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    using MFNRWrapPtr = std::unique_ptr<MFNRWrap>;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MFNRNode(
            const char *name,
            IspPipeNodeID nodeId,
            const PipeNodeConfigs& config);
    MFNRNode(
            const char *name,
            IspPipeNodeID nodeId,
            const PipeNodeConfigs& config,
            int policy,
            int priority);
    virtual ~MFNRNode();

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
//  MFNRNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief check whether MFNR is enabled
     * @param [in] pRequest Current request
     * @return
     * - true, false
     */
    MBOOL isEnableMFNR(const IspPipeRequestPtr& pRequest);
    /**
     * @brief execute MFNR
     * @param [in] pRequest Current request
     * @return
     * - execute result
     */
    MBOOL executeMFNR(const IspPipeRequestPtr& pRequest);
    /**
     * @brief execute by-pass
     * @param [in] pRequest Current request
     * @return
     * - execute result
     */
    MBOOL handleByPass(const IspPipeRequestPtr& pRequest);
    /**
     * @brief execute swnr
     * @param [in] pRequest Current request
     * @return
     * - execute result
     */
    MBOOL executeSWNR(const IspPipeRequestPtr& pRequest);
    /**
     * @brief get open id from hal metadata
     * @param [in] pRequest Current request
     * @return
     * - execute result
     */
    MINT32 getOpenIdFromMeta(const IspPipeRequestPtr& pRequest);
    /**
     * @brief get iso from app metadata
     * @param [in] pRequest Current request
     * @return
     * - execute result
     */
    MINT32 getIsoFromMeta(const IspPipeRequestPtr& pRequest);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MFNRNode Private Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // request queue
    WaitQueue<IspPipeRequestPtr> mRequestQue;
    // on processing request, for FIFO dispatch request
    RequestCollection            mProcessingRequests;

    MINT32                       miSensorIdx_Main1 = -1;
    MINT32                       miSensorIdx_Main2 = -1;

    MFNRWrapPtr                  mWrapPtr;

    MINT32                       mSwnrMode;
};


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
#endif