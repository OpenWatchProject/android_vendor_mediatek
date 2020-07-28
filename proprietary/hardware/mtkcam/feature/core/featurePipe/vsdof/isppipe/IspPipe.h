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
 * INCORPORATED IN, OR SUPPLIESD WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
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
 * @file IspPipe.h
 * @brief Main class of ThirdParty IspPipe
 */
#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_H_

// Standard C header file

// Android system/core header file
#include <utils/Vector.h>
#include <utils/KeyedVector.h>

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/feature/stereo/pipe/IIspPipe.h>
// Module header file
#include <featurePipe/core/include/CamPipe.h>

// Local header file
#include "PipeBufferHandler.h"
#include "PipeBufferPoolMgr.h"
#include "IspPipeNode.h"
#include "IIspPipeFlowControler.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

class IIspPipeFlowControler;
/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class IspPipe
 * @brief ThirdParty Isp pipe
 */
class IspPipe
: public CamPipe<IspPipeNode>
, public IspPipeNode::Handler_T
, public IIspPipe
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    IspPipe(
        sp<IspPipeSetting> pSetting,
        sp<IspPipeOption> pOption,
        sp<IIspPipeFlowControler> pFlowControler);

    virtual ~IspPipe();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IIspPipe Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL init();
    MBOOL uninit();
    MVOID flush();
    MVOID sync();
    MBOOL enque(android::sp<IIspPipeRequest>& request);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CamPipe Protected Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual MBOOL onInit();
    virtual MVOID onUninit();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeDataHandler Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MBOOL onData(DataID id, const IspPipeRequestPtr& data);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IspPipe Private Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
     /**
     * @brief handle function for ERROR_OCCUR_NOTIFY.
     * @param [in] pRequest IspPipe request
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL onErrorNotify(const IspPipeRequestPtr& pRequest);
    /**
     * @brief handle function for data complete.
     * @param [in] pRequest IspPipe request
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL onDataSuccess(const IspPipeRequestPtr& pRequest);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IspPipe Private member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // mutex for request map
    android::Mutex mReqMapLock;
    // Request collections
    KeyedVector<MUINT32, IspPipeRequestPtr> mvRequestDataMap;
    // node map
    KeyedVector<IspPipeNodeID, IspPipeNode*> mNodeMap;
    // buffer pool mgr
    sp<PipeBufferPoolMgr> mpBufferPoolMgr = nullptr;
    // setting/config
    sp<IspPipeSetting> mpPipeSetting = nullptr;
    sp<IspPipeOption> mpPipeOption = nullptr;
    // Flow Controler
    sp<IIspPipeFlowControler> mpIspPipeFlowControler = nullptr;
    // node signal
    sp<NodeSignal> mpNodeSignal;
};



}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam


#endif
