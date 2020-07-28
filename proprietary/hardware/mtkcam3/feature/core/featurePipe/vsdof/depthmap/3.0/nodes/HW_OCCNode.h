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
* @file HW_OCCNode.h
* @brief HW_OCCNode inside DepthMapPipe
*/
#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATUREPIPE_HWOCC_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATUREPIPE_HWOCC_NODE_H

// Standard C header file
#include <queue>
// Android system/core header file
#include <utils/Mutex.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/def/owecommon.h>
#include <mtkcam/drv/iopipe/PostProc/IOweStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalEgnPipe.h>
#include <mtkcam/drv/iopipe/PostProc/IEgnStream.h>
#include <stereo_tuning_provider.h>
// Module header file
#include <occ_hal.h>
#include <stereo_distance_util.h>
#include <featurePipe/core/include/WaitQueue.h>
// Local header file
#include "../DepthMapPipeNode.h"
#include "../DepthMapPipe_Common.h"
#include <mtkcam/feature/stereo/hal/FastLogger.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {
using std::queue;
using android::Mutex;
using namespace NSCam::NSIoPipe::NSEgn;
using namespace StereoHAL;
/**
 * @class OCC HW Node
 * @brief OCC HW featurePipe node
 */
class HW_OCCNode: public DepthMapPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    HW_OCCNode(
        const char *name,
        DepthMapPipeNodeID nodeID,
        PipeNodeConfigs config);
    virtual ~HW_OCCNode();
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
    /**
     * @brief OWE driver callback function
     */
    static MVOID onOWECallback(EGNParams<OCCConfig>& rParams);
    MVOID handleOWEDone(EGNParams<OCCConfig>& rParams,
                        EnqueCookieContainer* pEnqueCookie);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief cleanup function - release hal/resources
     */
    MVOID cleanUp();
    /**
     * @brief Perform OCC enque param
     * @param [in] pRequest Current effect request
     * @param [out] rEnqParam OWE OCC enque param
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareEnqueParams(
                    DepthMapRequestPtr& pRequest,
                    EGNParams<OCCConfig>& rEnqParam);

    struct DebugBufParam
    {
        IImageBuffer *imgBuf_MV_Y = nullptr;
        IImageBuffer *imgBuf_SV_Y = nullptr;
        IImageBuffer *imgBuf_DMP_L = nullptr;
        IImageBuffer *imgBuf_DMP_R = nullptr;
        IImageBuffer *nocMap = nullptr;
        const OCCConfig& occConfig;
    };
    /**
     * @brief debug OCC in/out buffers
     */
    MVOID debugOCCParams(DebugBufParam param);
private:
    // Job queue
    WaitQueue<DepthMapRequestPtr> mJobQueue;
    WaitQueue<MUINT32> mMYSReqIDQueue;
    // OWE stream
    IEgnStream<OCCConfig>* mpOweStream = nullptr;
    // utils for disance
    StereoDistanceUtil mDistanceUtil;
    float              mDistance = 0.0f;
};

} //NSFeaturePipe_DepthMap
} //NSCamFeature
} //NSCam

#endif
