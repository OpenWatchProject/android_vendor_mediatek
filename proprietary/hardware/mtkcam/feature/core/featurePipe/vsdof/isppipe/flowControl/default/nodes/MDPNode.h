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
 * @file MDPNode.h
 * @brief MDP node for isp pipe
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_MDP_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_MDP_H_

// Standard C header file

// Android system/core header file
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
// mtkcam custom header file

// mtkcam global header file
#include <DpAsyncBlitStream.h>
#include <DpBlitStream.h>
#include <DpDataType.h>
// Module header file
#include <featurePipe/core/include/WaitQueue.h>
// Local header file
#include "../DefaultIspPipeFlow_Common.h"
#include "../../../IspPipeNode.h"

//#define ENABLE_ASYNC
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
 * @class MDPNode
 * @brief Node class for MDP
 */
class MDPNode: public IspPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MDPNode(
            const char *name,
            IspPipeNodeID nodeID,
            const PipeNodeConfigs& config);
    virtual ~MDPNode();

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
//  MDPNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief generate thumbnail buffer
     * @param [in] pRequest request
     * @return
     * - MTRUE: success
     * - MFALSE: failure
     */
    MBOOL generateThumbnail(IspPipeRequestPtr pRequest);
    /**
     * @brief generate YUV buffer, source is PV00
     * @param [in] pRequest request
     * @return
     * - MTRUE: success
     * - MFALSE: failure
     */
    MBOOL generateYUVs(IspPipeRequestPtr pRequest);
    /**
     * @brief Map image format into dp color format
     * @param [in] format image format
     * @return
     * - Dp color format
     */
    DpColorFormat mapToDpColorFmt(MINT format);

    struct MDPTarget
    {
        IspPipeBufferID mBufID;
        IImageBuffer* mpImgBuf = nullptr;
        MINT32 miRotAngle = 0;
    };
    /**
     * @brief execute MDP(support 1 in 2 out)
     * @param [in] pInputImg input buffer
     * @param [out] pOutputImg output buffer
     * @param [out] pOutputImgSec output buffer
     * @return
     * - MTRUE: success
     * - MFALSE: failure
     */
    MBOOL executeMDPResize(
        IImageBuffer* pInputImg,
        MDPTarget* pTarget,
        MDPTarget* pTarget2 = nullptr);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MDPNode Private Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // request queue
    WaitQueue<IspPipeRequestPtr> mRequestQue;
    // MDP
    #ifdef ENABLE_ASYNC
    DpAsyncBlitStream* mpAsynBlitStream = nullptr;
    #else
    DpBlitStream* mpDpStream = nullptr;;
    #endif
};


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
#endif