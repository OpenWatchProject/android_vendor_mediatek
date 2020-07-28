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
 * @file DPENode.h
 * @brief DPENode inside DepthMapPipe
*/

#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_DPE_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_DPE_NODE_H

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/def/dpecommon.h>
#include <mtkcam/drv/iopipe/PostProc/IDpeStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalDpePipe.h>
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

using namespace NSCam::NSIoPipe::NSDpe;
using namespace StereoHAL;
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
    MVOID setDMPBufferPool(sp<ImageBufferPool> pDMPBufPool);

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
     * @param [out] DVEConfig DPE enque params
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareDPEEnqueConfig_PVVR(
                        DepthMapRequestPtr pRequest,
                        std::vector<NSIoPipe::DVEConfig>& rDPEConfigVec);
    /**
     * @brief prepare DPE enque params
     * @param [in] pRequest effect request
     * @param [out] DVEConfig DPE enque params
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareDPEEnqueConfig_CAP(
                        DepthMapRequestPtr pRequest,
                        std::vector<NSIoPipe::DVEConfig>& rDPEConfigVec);
    /**
     * @brief build image buffer into the DPEBufInfo structure
     * @param [in] dmaPort DPE DMA port
     * @param [in] dmaPort DPE DMA port
     * @param [out] rBufInfo DPEBufInfo
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL setupDPEBufInfo(
                    NSIoPipe::DPEDMAPort dmaPort,
                    IImageBuffer* pImgBuf,
                    NSIoPipe::DPEBufInfo& rBufInfo);

    struct DPEInputData
    {
        IImageBuffer* pImgBuf_MV_Y = nullptr;
        IImageBuffer* pImgBuf_SV_Y = nullptr;
        IImageBuffer* pImgBuf_MASK_M = nullptr;
        IImageBuffer* pImgBuf_MASK_S = nullptr;
        IImageBuffer* pImgBuf_LastDMP_L = nullptr;
        IImageBuffer* pImgBuf_LastDMP_R = nullptr;
    };
    struct DPEOutputData
    {
        IImageBuffer* pImgBuf_DMP_L = nullptr;
        IImageBuffer* pImgBuf_DMP_R = nullptr;
        IImageBuffer* pImgBuf_CFM_L = nullptr;
        IImageBuffer* pImgBuf_CFM_R = nullptr;
        IImageBuffer* pImgBuf_RESPO_L = nullptr;
        IImageBuffer* pImgBuf_RESPO_R = nullptr;
    };
    /**
     * @brief config DVEConfig IO buffers
     * @param [in] input data
     * @param [in] output data
     * @param [out] rDVEConfig config to be configured
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL setupDPEConfigIO(
            DPEInputData input,
            DPEOutputData output,
            NSIoPipe::DVEConfig& rDPEConfig
            );

    /**
     * @brief DPEStream success callback function
     */
    static MVOID onDPEEnqueDone(DVEParams& rParams);
    /**
     * @brief DPE enqued-operation finished handle function
     * @param [in] rParams dequed DVEParam
     * @param [in] pEnqueCookie EnqueCookie instance
     */
    MVOID handleDPEEnqueDone(
                        DVEParams& rParams,
                        EnqueCookieContainer* pEnqueCookie);
    MVOID debugDPEConfig(NSIoPipe::DVEConfig& config);
    MVOID debugDPEIOData(DPEInputData input, DPEOutputData output);

private:
    // job queue
    WaitQueue<DepthMapRequestPtr> mJobQueue;
    MBOOL mbFirst;
    // Last frame DMP
    SmartImageBuffer mLastDMP_L = nullptr;
    SmartImageBuffer mLastDMP_R = nullptr;
    // DPE stream
    IDpeStream* mpDPEStream = nullptr;
    // DMP buffer pool - for default DMP
    sp<ImageBufferPool> mpDMPBufferPool;
};





}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam

#endif