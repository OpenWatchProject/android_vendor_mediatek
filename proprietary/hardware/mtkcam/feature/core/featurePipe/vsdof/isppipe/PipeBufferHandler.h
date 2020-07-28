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
 * @file PipeBufferHandler.h
 * @brief Buffer handler for ispPipe
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_ISP_PIPE_BUFFER_HANDLER_H_
#define _MTK_CAMERA_FEATURE_PIPE_ISP_PIPE_BUFFER_HANDLER_H_

// Standard C header file

// Android system/core header file
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/RWLock.h>
// mtkcam custom header file

// mtkcam global header file
#include <metadata/IMetadata.h>
#include <mtkcam/feature/stereo/pipe/IIspPipe.h>
// Module header file
// Local header file
#include "IspPipe_Common.h"
#include "PipeBufferPoolMgr.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

using android::sp;

/*******************************************************************************
* Global Define
********************************************************************************/

/*******************************************************************************
* Structure Define
********************************************************************************/

/*******************************************************************************
* Class Definition
********************************************************************************/
class PipeBufferPoolMgr;
class IspPipeRequest;
/**
 * @class PipeBufferHandler
 * @brief Buffer Handler inside IspPipe
 */

class PipeBufferHandler: public virtual android::VirtualLightRefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PipeBufferHandler(PipeBufferPoolMgr* pPoolMgr);
    virtual ~PipeBufferHandler();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferHandler Public Operations
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief request buffer from the buffer handler
     *        one buffer id can request one buffer only, requires one one mapping
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id used to request buffer.
     * @return
     * - requested buffer pointer
     */
    IImageBuffer* requestBuffer(
                                IspPipeNodeID nodeID,
                                IspPipeBufferID bufferID);
    /**
     * @brief request working buffer from the buffer handler
     *        no one-one mapping constraint
     * @param [in] bufferID buffer id used to request buffer.
     * @return
     * - requested buffer pointer
     */
    IImageBuffer* requestWorkingBuffer(IspPipeBufferID bufferID);

    MVOID* requestWorkingTuningBuf(IspPipeBufferID bufferID);

    /**
     * @brief request tuning buffer from the buffer handler
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id used to request buffer.
     * @return
     * - requested tuning buffer pointer
     */
    MVOID* requestTuningBuf(
                            IspPipeNodeID nodeID,
                            IspPipeBufferID bufferID);
    /**
     * @brief request metadata from the buffer handler
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id used to request buffer.
     * @return
     * - requested metadata buffer pointer
     */
    IMetadata* requestMetadata(
                            IspPipeNodeID nodeID,
                            IspPipeBufferID bufferID);
    /**
     * @brief Callback when the correnspoinding node finishes its job.
     * @param [in] nodeID Node ID that finishes its job
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL onProcessDone(IspPipeNodeID nodeID);

    /**
     * @brief configure the effectRequest
     * @param [in] pRequest EffectRequest pointer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL configRequest(IspPipeRequest* pRequest);

    /**
     * @brief config the buffer is for the node's output and do not release it.
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id used to config
     * @param [in] outNodeID output node id
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL configOutBuffer(
                            IspPipeNodeID srcNodeID,
                            IspPipeBufferID bufferID,
                            IspPipeNodeID outNodeID
                            );


    /**
     * @brief Manully specify the externel smart image buffer as output buffer
     *        from src node to out node
     * @param [in] srcNodeID src node id
     * @param [in] bufferID smImgBuf's buffer id
     * @param [in] outNodeID output node id
     * @param [in] smImgBuf the smart image buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL configExternalOutBuffer(
                        IspPipeNodeID srcNodeID,
                        IspPipeBufferID bufferID,
                        IspPipeNodeID outNodeID,
                        SmartPipeImgBuffer smImgBuf
                        );


    /**
     * @brief Manully set the output buffer ID and config the src buffer as output for outNode
     *        (Used in some situations that output buffer comes from different id )
     * @param [in] srcNodeID caller's node id
     * @param [in] srcBufferID buffer id used to retrieve from srcNode
     * @param [in] outNodeID buffer's output node id
     * @param [in] outBufferID buffer id to be used inside the output node id
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL configInOutBuffer(
                        IspPipeNodeID srcNodeID,
                        IspPipeBufferID srcBufferID,
                        IspPipeNodeID outNodeID,
                        IspPipeBufferID outBufferID
                        );

    /**
     * @brief Get the enqueued/active buffer of this operation
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id
     * @param [out] rpImgBuf enqued buffer
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates failure
     */
    MBOOL getEnqueBuffer(
                        IspPipeNodeID srcNodeID,
                        IspPipeBufferID bufferID,
                        IImageBuffer*& rpImgBuf);

    MBOOL getEnquedSmartBuffer(
                            IspPipeNodeID srcNodeID,
                            IspPipeBufferID bufferID,
                            SmartPipeImgBuffer& pSmImgBuf);
    MBOOL getEnquedSmartBuffer(
                            IspPipeNodeID srcNodeID,
                            IspPipeBufferID bufferID,
                            SmartGraphicBuffer& pSmGraBuf);
    MBOOL getEnquedSmartBuffer(
                            IspPipeNodeID srcNodeID,
                            IspPipeBufferID bufferID,
                            SmartTuningBuffer& pSmTuningBuf);

    /**
     * @brief get buffer pool mgr instance
     */
    sp<PipeBufferPoolMgr> getBufferPoolMgr() {return mpBufferPoolMgr;}
    /**
     * @brief flush the working buffer
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates failure
     */
    MBOOL flush();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferHandler Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief record the requested smart buffer into enque buffer map
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id
     * @param [in] smImgBuf smart buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL addEnquedBuffer(
                    IspPipeNodeID srcNodeID,
                    IspPipeBufferID bufferID,
                    SmartPipeImgBuffer smImgBuf
                    );

    MBOOL addEnquedBuffer(
                    IspPipeNodeID srcNodeID,
                    IspPipeBufferID bufferID,
                    SmartImageBuffer smImgBuf
                    );

    MBOOL addEnquedBuffer(
                    IspPipeNodeID srcNodeID,
                    IspPipeBufferID bufferID,
                    SmartGraphicBuffer smGraBuf
                    );

    MBOOL addEnquedBuffer(
                    IspPipeNodeID srcNodeID,
                    IspPipeBufferID bufferID,
                    SmartTuningBuffer smTuningBuf
                    );

    /**
     * @brief record the requested working smart buffer into buffer map
     * @param [in] smImgBuf smart buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL addEnquedWorkingBuffer(
                    SmartPipeImgBuffer smImgBuf
                    );

    MBOOL addEnquedWorkingBuffer(
                    SmartGraphicBuffer smGraBuf
                    );

    MBOOL addEnquedWorkingBuffer(
                    SmartTuningBuffer smTuningBuf
                    );

    /**
     * @brief record the enqued smart buffer into
     *        output buffer map of the output nodeID
     * @param [in] output node id
     * @param [in] bufferID buffer id
     * @param [in] smImgBuf smart buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL addOutputBuffer(
                    IspPipeNodeID outNodeID,
                    IspPipeBufferID bufferID,
                    SmartPipeImgBuffer smImgBuf
                    );

    MBOOL addOutputBuffer(
                    IspPipeNodeID outNodeID,
                    IspPipeBufferID bufferID,
                    SmartGraphicBuffer smGraBuf
                    );

    MBOOL addOutputBuffer(
                    IspPipeNodeID outNodeID,
                    IspPipeBufferID bufferID,
                    SmartTuningBuffer smTuningBuf
                    );
    /**
     * @brief init the internal enque buffer map
     */
    MVOID initEnqueBufferMap(MINT32 nodeCount);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferHandler Private Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    typedef KeyedVector<IspPipeBufferID, SmartPipeImgBuffer> BIDToSmartBufferMap;
    typedef KeyedVector<IspPipeBufferID, SmartImageBuffer> BIDToSmartIndividualBufferMap;
    typedef KeyedVector<IspPipeBufferID, SmartGraphicBuffer> BIDToGraphicBufferMap;
    typedef KeyedVector<IspPipeBufferID, SmartTuningBuffer> BIDToTuningBufferMap;
    // use to record the enqued buffers which are requested from buffer pool
    // Limit: one buffer id/one enque buffer mapping
    RWLock mEnqueRWLock[eBUFFER_SIZE];
    KeyedVector<IspPipeNodeID, BIDToSmartBufferMap> mEnqueBufferMap;
    KeyedVector<IspPipeNodeID, BIDToSmartIndividualBufferMap> mEnqueIndiviBufferMap;
    KeyedVector<IspPipeNodeID, BIDToGraphicBufferMap> mEnqueGraphicBufferMap;
    KeyedVector<IspPipeNodeID, BIDToTuningBufferMap> mEnqueTuningBufferMap;

    // working buffer map - no bid mapping
    RWLock mWorkingBufRWLock[eBUFFER_SIZE];
    Vector<SmartPipeImgBuffer> mvWorkingSmartImgBuffer;
    Vector<SmartGraphicBuffer> mvWorkingSmartGraImgBuffer;
    Vector<SmartTuningBuffer> mvWorkingSmartTuningBuffer;

    // IspPipeRequest
    IspPipeRequest* mpIspPipeRequest = nullptr;
    // isp pipe buffer mgr
    PipeBufferPoolMgr* mpBufferPoolMgr = nullptr;
};

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif