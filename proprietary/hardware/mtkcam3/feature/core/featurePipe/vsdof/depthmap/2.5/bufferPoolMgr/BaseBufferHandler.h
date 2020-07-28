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
 * @file BaseBufferPoolMgr.h
 * @brief base classes of BufferPoolMgr and BufferHanlder
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BUFFERPOOL_HANDLER_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BUFFERPOOL_HANDLER_H_

// Standard C header file

// Android system/core header file
#include <utils/RefBase.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>

// Module header file
// Local header file
#include "../DepthMapPipe_Common.h"
#include "BaseBufferPoolMgr.h"
#include "BufferPool.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

/*******************************************************************************
* Enum Definition
********************************************************************************/

/*******************************************************************************
* Class Definition
********************************************************************************/
class DepthMapEffectRequest;
class EffectRequestAttrs;
/**
 * @class BaseBufferHandler
 * @brief Base class of BufferHanlder
 */
class BaseBufferHandler : public virtual android::VirtualLightRefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    BaseBufferHandler(sp<BaseBufferPoolMgr> pPoopMgr)
                        : mpBufferPoolMgr(pPoopMgr)
                        {}
    virtual ~BaseBufferHandler() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferPoolMgr Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief request buffer from the buffer handler.
     *        the requested buffer can be acquired by using getEnqueBuffer api.
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id used to request buffer.
     * @return
     * - requested buffer pointer
     */
    virtual IImageBuffer* requestBuffer(
                                DepthMapPipeNodeID,
                                DepthMapBufferID) {return NULL;}

    /**
     * @brief request metadata from the buffer handler
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id used to request buffer.
     * @return
     * - requested metadata pointer
     */
    virtual IMetadata* requestMetadata(
                                DepthMapPipeNodeID,
                                DepthMapBufferID) {return NULL;}

     /**
     * @brief request tuning buffer from the buffer handler
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id used to request buffer.
     * @return
     * - requested tuning buffer pointer
     */
    virtual MVOID* requestTuningBuf(
                            DepthMapPipeNodeID,
                            DepthMapBufferID
                            ) {return NULL;}
    /**
     * @brief request working buffer from the buffer handler
     *        working buffer cannot be used as output to next node
     *        and cannot be acquired from handler after request
     * @param [in] bufferID buffer id used to request buffer.
     * @return
     * - requested buffer pointer
     */
    virtual IImageBuffer* requestWorkingBuffer(
                                DepthMapPipeNodeID,
                                DepthMapBufferID) {return NULL;}

     /**
     * @brief request working tuning buffer from the buffer handler
     *        working buffer cannot be used as output to next node.
     *        and cannot be acquired from handler after request
     * @param [in] bufferID buffer id used to request buffer.
     * @return
     * - requested tuning buffer pointer
     */
    virtual MVOID* requestWorkingTuningBuf(
                                DepthMapPipeNodeID,
                                DepthMapBufferID
                                ) {return NULL;}
    /**
     * @brief Callback when the correnspoinding node finishes its job.
     * @param [in] nodeID Node ID that finishes its job
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL onProcessDone(DepthMapPipeNodeID) {return NULL;}

    /**
     * @brief configure the effectRequest and generate its attributes
     * @param [in] pDepthReq EffectRequest pointer
     * @param [out] pOutAttr output effect attribute
     * @return
     * - effect attributes
     */
    virtual MBOOL configRequest(DepthMapEffectRequest*) {return MFALSE;}

    /**
     * @brief config the buffer is for the node's output and do not release it.
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id used to config
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL configOutBuffer(
                            DepthMapPipeNodeID,
                            DepthMapBufferID,
                            DepthMapPipeNodeID
                            ) {return MFALSE;}


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
    virtual MBOOL configExternalOutBuffer(
                        DepthMapPipeNodeID srcNodeID,
                        DepthMapBufferID bufferID,
                        DepthMapPipeNodeID outNodeID,
                        SmartImageBuffer smImgBuf
                        ) {return MFALSE;}


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
    virtual MBOOL configInOutBuffer(
                        DepthMapPipeNodeID srcNodeID,
                        DepthMapBufferID srcBufferID,
                        DepthMapPipeNodeID outNodeID,
                        DepthMapBufferID outBufferID
                        ) {return MFALSE;}

    /**
     * @brief Get the enqueued/active buffer of this operation
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id
     * @param [out] pImgBuf enqued buffer
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates failure
     */
    virtual MBOOL getEnqueBuffer(
                            DepthMapPipeNodeID,
                            DepthMapBufferID,
                            IImageBuffer*&) {return MFALSE;}

    /**
     * @brief Get the SmartBuffer format of enqued buffer
     * @param [in] srcNodeID caller's node id
     * @param [in] bufferID buffer id
     * @param [out] psmImgBuf enqued smart image buffer
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates failure
     */
    virtual MBOOL getEnquedSmartBuffer(
                            DepthMapPipeNodeID,
                            DepthMapBufferID,
                            SmartImageBuffer&) {return MFALSE;}
    virtual MBOOL getEnquedSmartBuffer(
                            DepthMapPipeNodeID,
                            DepthMapBufferID,
                            SmartGraphicBuffer&) {return MFALSE;}
    virtual MBOOL getEnquedSmartBuffer(
                            DepthMapPipeNodeID,
                            DepthMapBufferID,
                            SmartTuningBuffer&) {return MFALSE;}

    /**
     * @brief get buffer pool mgr instance
     */
    sp<BaseBufferPoolMgr> getBufferPoolMgr() {return mpBufferPoolMgr;}
    /**
     * @brief flush the working buffer
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates failure
     */
    virtual MBOOL flush() {return MFALSE;}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferPoolMgr Protected Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    sp<BaseBufferPoolMgr> mpBufferPoolMgr;

};

typedef sp<BaseBufferHandler> BufferPoolHandlerPtr;



}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif