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
 * @file P2ABayerNode.h
 * @brief Extra pass2A path run for bayer data
*/

#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_P2ABAYER_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_P2ABAYER_NODE_H

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <DpIspStream.h>
#include <DpDataType.h>
// Module header file
#include <featurePipe/core/include/WaitQueue.h>

// Local header file
#include "../DepthMapPipeNode.h"
#include "../DepthMapPipe_Common.h"
#include "../DepthMapPipeUtils.h"
#include "../DataStorage.h"
#include "../DataSequential.h"
#include "NR3DCommon.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NS3Av3;
using namespace StereoHAL;

/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class P2ABayerNode
 * @brief Node class for P2A Bayer pass
 */
class P2ABayerNode
: public DepthMapPipeNode
, public NR3DCommon
, public DataSequential<DepthMapRequestPtr>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    P2ABayerNode(
        const char *name,
        DepthMapPipeNodeID nodeID,
        PipeNodeConfigs config);

    virtual ~P2ABayerNode();

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
//  P2ABayerNode Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief handle the preview frame wit no-need FEFM
     * @param [in] pRequest Current effect request
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL handleNeedFEFMFrame(
                    sp<DepthMapEffectRequest> pRequest);
    MBOOL handleFrames(
                    sp<DepthMapEffectRequest> pRequest);
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
     * @brief NormalStream failure callback function
     * @param [in] rParams dequeued QParams
     */
    static MVOID onP2FailedCallback(QParams& rParams);
    /**
     * @brief resource uninit/cleanup function
     */
    MVOID cleanUp();
     /**
     * @brief Perform 3A Isp tuning and generate the Stereo3ATuningRes
     * @param [in] rEffectReqPtr Current effect request
     * @param [out] rOutTuningRes Output tungin result
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL perform3AIspTuning(
                        DepthMapRequestPtr& rEffectReqPtr,
                        Stereo3ATuningRes& rOutTuningRes);

    /**
     * @brief 3A isp tuning function
     * @param [in] rEffectReqPtr Current effect request
     * @return
     * - Tuning result
     */
    AAATuningResult applyISPTuning(
                        DepthMapRequestPtr& rEffectReqPtr);
    /**
     * @brief Map image format into dp color format
     * @param [in] format image format
     * @return
     * - Dp color format
     */
    DpColorFormat mapToDpColorFmt(MINT format);
    /**
     * @brief handle the flow type related tasks
     * @param [in] pRequest Current effect request
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL onHandleFlowTypeP2Done(
                    sp<DepthMapEffectRequest> pRequest);
    /**
     * @brief handle the on-going request data finish
     * @param [in] iReqID finished request id
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL onHandleOnGoingReqReady(MUINT32 iReqID);

    /**
     * @brief copy the src buffer into the request
     * @param [in] pSrcBuffer src buffer
     * @param [in] pRequest the buffer inside the request will be replaced.
     * @param [in] bufferID buffer id to be copied
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL _copyBufferIntoRequest(
                    IImageBuffer* pSrcBuffer,
                    sp<DepthMapEffectRequest> pRequest,
                    DepthMapBufferID bufferID);
public:
    // request queue
    WaitQueue<DepthMapRequestPtr> mRequestQue;
    // P2 NormalStream
    INormalStream*  mpINormalStream = NULL;
    // 3A hal
    IHal3A* mp3AHal_Main1 = NULL;
    // sensor index
    MUINT32 miSensorIdx_Main1;
    MUINT32 miSensorIdx_Main2;
    // MDP stream
    DpIspStream* mpDpIspStream = nullptr;;

};

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam


#endif //_MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_P2ABAYER_NODE_H
