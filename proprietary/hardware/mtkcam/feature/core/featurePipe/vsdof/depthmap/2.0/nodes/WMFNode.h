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
 * @file WMFNode.h
 * @brief WMFNode inside the DepthMapPipe
*/
#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_WMF_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_WMF_NODE_H


// Standard C header file
#include <vector>
// Android system/core header file
#include <utils/KeyedVector.h>
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

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSCam::NSIoPipe::NSDpe;
using std::vector;
using namespace StereoHAL;

class WMFNode: public DepthMapPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    WMFNode(
        const char *name,
        DepthMapPipeNodeID nodeID,
        PipeNodeConfigs config);
    virtual ~WMFNode();

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
    // need manual onFlush to clear internal data
    virtual MVOID onFlush();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  WMFNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief Prepare WMF enque config
     * @param [in] pRequest Current effect request
     * @param [in] enqueConfig enque buffer id config
     * @param [out] rWMFConfig WMF enque params
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareWMFEnqueConfig(
                            DepthMapPipeOpState state,
                            sp<BaseBufferHandler> pBufferHandler,
                            NSIoPipe::WMFEConfig &rWMFConfig);
    /**
     * @brief perform WMF operation
     */
    MBOOL performWMFOperation(DepthMapRequestPtr& pRequest);
    /**
     * @brief WMF success callback function
     */
    static MVOID onWMFEnqueDone_FirstPass(WMFEParams& rParams);
     /**
     * @brief WMF enqued-operation finished handle function : first pass
     * @param [in] rParams dequed WMFEParams
     * @param [in] pEnqueCookie EnqueCookie instance
     */
    MVOID handleWMFEnqueDone_FirstPass(
                                WMFEParams& rParams,
                                EnqueCookieContainer* pEnqueCookie);

    /**
     * @brief build image buffer into the DPEBufInfo struction
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
                    NSIoPipe::DPEBufInfo& rBufInfo,
                    MUINT8 iPlaneIndex=0);

    /**
     * @brief prepare WMFE Control
     * @param [in] pSrcImgBuf source image input
     * @param [in] pDepthMaskBuf Depth and Mask buffer
     * @param [in] pOutputBuf source image input
     * @param [out] rWMFCtrl WMF control
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareWMFECtrl(
                IImageBuffer* pSrcImgBuf,
                IImageBuffer* pDepthMaskBuf,
                IImageBuffer* pOutputBuf,
                IImageBuffer* pTbliBuf,
                WMFECtrl& rWMFCtrl);

    /**
     * @brief cleanup function - release hal/resources
     */
    MVOID cleanUp();
    /**
     * @brief debug input params
     */
    MVOID debugWMFParam(NSIoPipe::WMFEConfig &config);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  WMFNode Private Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    WaitQueue<DepthMapRequestPtr> mJobQueue;
    // DPE stream
    IDpeStream* mpDPEStream = nullptr;
    // TBLI buffer pool
    android::sp<ImageBufferPool> mpTbliImgBufPool;
    // tuning parameter: WMFECtrl
    vector<NSIoPipe::WMFECtrl> mvWMFECtrl;
    // tuning buffer
    vector<SmartImageBuffer> mvTbliImgBuf;
    // default use the Y channel data only
    const ENUM_WMF_CHANNEL mWMFInputChannel = E_WMF_Y_CHANNEL;
};


}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam

#endif