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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <common/vsdof/hal/common/n3d_hal.h>
#include <featurePipe/core/include/WaitQueue.h>
#include <featurePipe/core/include/GraphicBufferPool.h>
// Local header file
#include "../DepthMapPipeNode.h"
#include "../DepthMapPipe_Common.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
using namespace StereoHAL;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

class N3DNode: public DepthMapPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
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
     * @brief cleanup function - release hal/resources
     */
    MVOID cleanUp();
    /**
     * @brief Perform N3D ALGO of each scenario
     */
    MBOOL performN3DALGO_VRPV(DepthMapRequestPtr& pRequest);
    MBOOL performN3DALGO_CAP(DepthMapRequestPtr& pRequest);
    /**
     * @brief Prepare N3D HAL input param for each scenario
     * @param [in] pRequest Current effect request
     * @param [out] rN3dParam N3D input params
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareN3DInputParam(
                        DepthMapRequestPtr& pRequest,
                        N3D_HAL_PARAM& rN3dParam);
    MBOOL prepareN3DInputParam_CAP(
                        DepthMapRequestPtr& pRequest,
                        N3D_HAL_PARAM_CAPTURE& rN3dParam);
    /**
     * @brief Prepare FE/FM buffers of N3D input param
     */
    MBOOL prepareFEFMData(
                    sp<BaseBufferHandler>& pBufferHandler,
                    HWFEFM_DATA& rFefmData);
    /**
     * @brief Prepare EIS config of N3D input param
     */
    MBOOL prepareEISData(
                    IMetadata*& pInAppMeta,
                    IMetadata*& pInHalMeta_Main1,
                    EIS_DATA& rEISData);

    /**
     * @brief Prepare N3D output param
     * @param [in] pBufferHandler buffer handler of current effect request
     * @param [out] rN3dParam N3D output params
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareN3DOutputParam(
                    sp<BaseBufferHandler>& pBufferHandler,
                    N3D_HAL_OUTPUT& rN3dParam);

    MBOOL prepareN3DOutputParam_CAP(
                    DepthMapRequestPtr& pRequest,
                    N3D_HAL_OUTPUT_CAPTURE& rN3dParam);
    /**
     * @brief Prepare the MV/SV/MASK buffer of output params
     */
    MBOOL prepareN3DOutputYUVMask(
                    sp<BaseBufferHandler>& pBufferHandler,
                    N3D_HAL_OUTPUT& rN3dParam);
    /**
     * @brief write N3D result into metadata
     */
    MBOOL writeN3DResultToMeta(
                    const N3D_HAL_OUTPUT&  n3dOutput,
                    DepthMapRequestPtr& pRequest);
    MBOOL writeN3DResultToMeta_CAP(
                    const N3D_HAL_OUTPUT_CAPTURE&  n3dOutput,
                    DepthMapRequestPtr& pRequest);
    /**
     * @brief write copy image buffer
     */
    MBOOL copyImageBuffer(
                    sp<IImageBuffer> srcImgBuf,
                    sp<IImageBuffer> dstImgBuf);
    /**
     * @brief extra data output handling
     */
    MBOOL outputExtraData(
                    char* sExtraData,
                    DepthMapRequestPtr& pRequest);

    /**
     * @brief debug functions
     */
    MVOID debugN3DParams(N3D_HAL_PARAM& param, N3D_HAL_OUTPUT& output);
    MVOID debugN3DParams_Cap(N3D_HAL_PARAM_CAPTURE& param, N3D_HAL_OUTPUT_CAPTURE& output);

private:
    WaitQueue<DepthMapRequestPtr> mJobQueue;
    // N3D hal
    N3D_HAL* mpN3DHAL_CAP = nullptr;
    N3D_HAL* mpN3DHAL_VRPV = nullptr;
};


}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam

#endif


