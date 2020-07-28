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
 * @file NodeBufferHandler.h
 * @brief VSDOF buffer handler
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_VSDOF_BUFFERHANDLER_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_VSDOF_BUFFERHANDLER_H_

// Standard C header file

// Android system/core header file
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/RWLock.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>

// Module header file

// Local header file
#include "BaseBufferHandler.h"
#include "BaseBufferPoolMgr.h"
#include "../DepthMapEffectRequest.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using android::sp;
using android::RWLock;

/*******************************************************************************
* Global Define
********************************************************************************/

/*******************************************************************************
* Structure Define
********************************************************************************/

/*******************************************************************************
* Class Definition
********************************************************************************/
//class NodeBufferPoolMgr_VSDOF;
/**
 * @class NodeBufferHandler
 * @brief Buffer Handler inside DepthMapPipe
 */

class NodeBufferHandler : public BaseBufferHandler

{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    NodeBufferHandler(sp<BaseBufferPoolMgr> pPoopMgr);
    virtual ~NodeBufferHandler();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferPoolMgr Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual IImageBuffer* requestBuffer(
                                DepthMapPipeNodeID srcNodeID,
                                DepthMapBufferID bufferID
                                );

    virtual IMetadata* requestMetadata(
                                DepthMapPipeNodeID srcNodeID,
                                DepthMapBufferID bufferID);

    virtual MVOID* requestTuningBuf(
                                DepthMapPipeNodeID srcNodeID,
                                DepthMapBufferID bufferID
                                );

    virtual IImageBuffer* requestWorkingBuffer(
                                DepthMapPipeNodeID srcNodeID,
                                DepthMapBufferID bufferID
                                );

    virtual MVOID* requestWorkingTuningBuf(
                                DepthMapPipeNodeID srcNodeID,
                                DepthMapBufferID bufferID
                                );

    virtual MBOOL onProcessDone(DepthMapPipeNodeID nodeID);

    virtual MBOOL configRequest(DepthMapEffectRequest* pDepthReq);

    virtual MBOOL configOutBuffer(
                            DepthMapPipeNodeID srcNodeID,
                            DepthMapBufferID bufferID,
                            DepthMapPipeNodeID outNodeID
                            );
    virtual MBOOL configExternalOutBuffer(
                        DepthMapPipeNodeID srcNodeID,
                        DepthMapBufferID bufferID,
                        DepthMapPipeNodeID outNodeID,
                        SmartFatImageBuffer smImgBuf
                        );

    virtual MBOOL configExternalOutBuffer(
                        DepthMapPipeNodeID srcNodeID,
                        DepthMapBufferID bufferID,
                        DepthMapPipeNodeID outNodeID,
                        SmartGraphicBuffer smGraImgBuf
                        );

    virtual MBOOL configInOutBuffer(
                        DepthMapPipeNodeID srcNodeID,
                        DepthMapBufferID srcBufferID,
                        DepthMapPipeNodeID outNodeID,
                        DepthMapBufferID outBufferID
                        );

    virtual MBOOL getEnqueBuffer(
                            DepthMapPipeNodeID srcNodeID,
                            DepthMapBufferID bufferID,
                            IImageBuffer*& pImgBuf
                            );

    virtual MBOOL getEnquedSmartBuffer(
                            DepthMapPipeNodeID srcNodeID,
                            DepthMapBufferID bufferID,
                            SmartFatImageBuffer& pSmImgBuf);
    virtual MBOOL getEnquedSmartBuffer(
                            DepthMapPipeNodeID srcNodeID,
                            DepthMapBufferID bufferID,
                            SmartGraphicBuffer& pSmGraBuf);
    virtual MBOOL getEnquedSmartBuffer(
                            DepthMapPipeNodeID srcNodeID,
                            DepthMapBufferID bufferID,
                            SmartTuningBuffer& pSmTuningBuf);

    virtual MBOOL flush();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferHandler Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferHandler Private Operations.
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
                    DepthMapPipeNodeID srcNodeID,
                    DepthMapBufferID bufferID,
                    SmartFatImageBuffer smImgBuf
                    );

    MBOOL addEnquedBuffer(
                    DepthMapPipeNodeID srcNodeID,
                    DepthMapBufferID bufferID,
                    SmartGraphicBuffer smGraBuf
                    );

    MBOOL addEnquedBuffer(
                    DepthMapPipeNodeID srcNodeID,
                    DepthMapBufferID bufferID,
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
                    DepthMapPipeNodeID srcNodeID,
                    SmartFatImageBuffer smImgBuf
                    );
    MBOOL addEnquedWorkingBuffer(
                    DepthMapPipeNodeID srcNodeID,
                    SmartGraphicBuffer smGraBuf
                    );
    MBOOL addEnquedWorkingBuffer(
                    DepthMapPipeNodeID srcNodeID,
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
                    DepthMapPipeNodeID outNodeID,
                    DepthMapBufferID bufferID,
                    SmartFatImageBuffer smImgBuf
                    );

    MBOOL addOutputBuffer(
                    DepthMapPipeNodeID outNodeID,
                    DepthMapBufferID bufferID,
                    SmartGraphicBuffer smGraBuf
                    );

    MBOOL addOutputBuffer(
                    DepthMapPipeNodeID outNodeID,
                    DepthMapBufferID bufferID,
                    SmartTuningBuffer smTuningBuf
                    );
    /**
     * @brief init the internal enque buffer map
     */
    MVOID initEnqueBufferMap();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferHandler Private Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    typedef KeyedVector<DepthMapBufferID, SmartFatImageBuffer> BIDToSmartBufferMap;
    typedef KeyedVector<DepthMapBufferID, SmartGraphicBuffer> BIDToGraphicBufferMap;
    typedef KeyedVector<DepthMapBufferID, SmartTuningBuffer> BIDToTuningBufferMap;
    // use to record the working buffers which are requested from buffer pool
    RWLock mEnqueRWLock[eBUFFER_SIZE];
    KeyedVector<DepthMapPipeNodeID, BIDToSmartBufferMap> mEnqueBufferMap;
    KeyedVector<DepthMapPipeNodeID, BIDToGraphicBufferMap> mEnqueGraphicBufferMap;
    KeyedVector<DepthMapPipeNodeID, BIDToTuningBufferMap> mEnqueTuningBufferMap;

    // working buffer map
    RWLock mWorkingBufRWLock[eBUFFER_SIZE];
    typedef Vector<SmartFatImageBuffer> SmartImgBufferVector;
    typedef Vector<SmartGraphicBuffer> SmartGraImgBufferVector;
    typedef Vector<SmartTuningBuffer> SmartTuningBufferVector;

    KeyedVector<DepthMapPipeNodeID, SmartImgBufferVector> mWorkingImageBufferMap;
    KeyedVector<DepthMapPipeNodeID, SmartGraImgBufferVector> mWorkingGraphicBufferMap;
    KeyedVector<DepthMapPipeNodeID, SmartTuningBufferVector> mWorkingTuningBufferMap;


    // DepthMapEffectRequest
    DepthMapEffectRequest *mpDepthMapEffectReq = NULL;
    // Request Attrs
    EffectRequestAttrs mReqAttr;
};



}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam


#endif
