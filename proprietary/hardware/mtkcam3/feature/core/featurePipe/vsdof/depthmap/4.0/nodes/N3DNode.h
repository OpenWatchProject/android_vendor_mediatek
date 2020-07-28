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
 * @file N3DNode.h
 * @brief N3DNode inside the DepthMapPipe
*/
#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_N3D_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_N3D_NODE_H

// Standard C header file
#include <queue>
// Android system/core header file
#include <utils/Mutex.h>
#include <utils/KeyedVector.h>
// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <n3d_hal.h>
#include <featurePipe/core/include/WaitQueue.h>
#include <featurePipe/core/include/GraphicBufferPool.h>
// Local header file
#include "../DepthMapPipeNode.h"
#include "../DepthMapPipe_Common.h"
#include "../DataSequential.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
using namespace StereoHAL;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

enum DEQUE_STATUS
{
    DEQUE_FAILED,   // error
    DEQUE_WAITING,  //waiting for some request
    DEQUE_SUCCESS
};

class N3DNode
: public DepthMapPipeNode
, public DataSequential<DepthMapRequestPtr>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    N3DNode() = delete;
    N3DNode(
        const char *name,
        DepthMapPipeNodeID nodeID,
        PipeNodeConfigs config);

    virtual ~N3DNode();

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
//  N3DNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief get the N3D preview operation phase
     */
    N3D_PVPHASE_ENUM getPreviewPhase(DepthMapRequestPtr pRequest);
    /**
     * @brief cleanup function - release hal/resources
     */
    MVOID cleanUp();
    /**
     * @brief Perform N3D ALGO of each scenario
     */
    MBOOL performN3DALGO_VRPV(DepthMapRequestPtr& pRequest);
    MBOOL performN3DALGO_CAP(DepthMapRequestPtr& pRequest);

    /**
     * @brief Prepare FE/FM buffers of N3D input param
     */
    MBOOL prepareFEFMData(
                    sp<BaseBufferHandler>& pBufferHandler,
                    HWFEFM_DATA& rFefmData);

    MBOOL prepareN3DInputMeta(
                    DepthMapRequestPtr& pRequest,
                    N3D_HAL_PARAM_WPE& rN3dParam);
    /**
     * @brief Prepare the MV/SV/MASK buffer of output params
     */

    /**
     * @brief write N3D result into metadata
     */
    MBOOL writeN3DResultToMeta(
                    const N3D_HAL_OUTPUT_WPE&  n3dOutput,
                    DepthMapRequestPtr& pRequest);
    /**
     * @brief write copy image buffer
     */
    MBOOL copyImageBuffer(
                    sp<IImageBuffer> srcImgBuf,
                    sp<IImageBuffer> dstImgBuf);

    /**
     * @brief debug functions
     */
    MVOID debugN3DParams(N3D_HAL_OUTPUT_WPE& param);

private:
    PriorityWaitQueue<DepthMapRequestPtr, DepthRequestOrderConverter> mPriorityQueue;
    KeyedVector<MUINT32, MBOOL> mvToDoLearningReqIDs;
    // N3D hal
    N3D_HAL* mpN3DHAL_CAP = nullptr;
    N3D_HAL* mpN3DHAL_VRPV = nullptr;
    //
    MUINT32 miMaxWorkingSize = 0;
};


}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam

#endif


