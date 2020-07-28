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
* @file OCCNode.h
* @brief OCCNode inside DepthMapPipe
*/
#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATUREPIPE_OCC_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATUREPIPE_OCC_NODE_H

// Standard C header file
#include <queue>
// Android system/core header file
#include <utils/Mutex.h>
// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <stereo_distance_util.h>
#include <common/vsdof/hal/common/occ_hal.h>
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
using std::queue;
using android::Mutex;
using namespace StereoHAL;
/**
 * @class OCCNode
 * @brief OCC featurePipe node
 */
class OCCNode: public DepthMapPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    OCCNode(
        const char *name,
        DepthMapPipeNodeID nodeID,
        PipeNodeConfigs config);
    virtual ~OCCNode();
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
//  DepthMapPipeNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief cleanup function - release hal/resources
     */
    MVOID cleanUp();
    /**
     * @brief Perform OCC ALGO of each scenario
     * @param [in] pRequest Current effect request
     * @param [out] rOCCParams N3D input params
     * @param [out] rOutParams N3D output params
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareOCCParams(
                    DepthMapRequestPtr& pRequest,
                    OCC_HAL_PARAMS& rOCCParams,
                    OCC_HAL_OUTPUT& rOutParams);

    struct DebugBufParam
    {
        IImageBuffer *imgBuf_MV_Y = nullptr;
        IImageBuffer *imgBuf_SV_Y = nullptr;
        IImageBuffer *imgBuf_DMP_L = nullptr;
        IImageBuffer *imgBuf_DMP_R = nullptr;
        IImageBuffer *imageMain1Bayer = nullptr;
        IImageBuffer *occMap = nullptr;
        IImageBuffer *nocMap = nullptr;
        IImageBuffer *depthMap = nullptr;
    };
    /**
     * @brief debug OCC in/out buffers
     */
    MVOID debugOCCParams(DebugBufParam param);
    /**
     * @brief add a new depth info to storage.
     */
    MBOOL addDepthInfoToStorage(DepthMapRequestPtr pRequest);
    /**
     * @brief update distance into metadata
     */
    MBOOL udpateDistanceIntoMeta(DepthMapRequestPtr pRequest);
private:
    // Job queue
    WaitQueue<DepthMapRequestPtr> mJobQueue;
    // Record the request id of whose LDC buffer is read
    WaitQueue<MUINT32> mLDCReqIDQueue;
    // Record the request id of whose MY_S bayer buffer is read (only used in Bayer+Mono sensor)
    WaitQueue<MUINT32> mMYSReqIDQueue;
    // OCC HAL
    OCC_HAL *mpOCCHAL = NULL;
    // utils for disance
    StereoDistanceUtil mDistanceUtil;
};

} //NSFeaturePipe_DepthMap
} //NSCamFeature
} //NSCam

#endif
