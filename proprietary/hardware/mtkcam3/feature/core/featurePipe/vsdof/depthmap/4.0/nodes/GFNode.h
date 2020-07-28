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
#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_GF_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_GF_NODE_H
//
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
//
#include <gf_hal.h>
//
#include "../DepthMapPipeNode.h"
#include "../DepthMapPipe_Common.h"
//
using namespace android;
using namespace StereoHAL;
//
class DpBlitStream;
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe_DepthMap{

class GFNode : public DepthMapPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //****************************************************************
    // Does not support default construct
    //****************************************************************
    GFNode() = delete;

    GFNode(
        const char *name,
        DepthMapPipeNodeID nodeID,
        PipeNodeConfigs config);
    virtual ~GFNode();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();
    virtual MBOOL onData(DataID id, DepthMapRequestPtr& pRequest);
    virtual MVOID onFlush();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapPipeNode Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  GFNode Private Definition.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  GFNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MVOID cleanUp();
    //
    MBOOL executeAlgo(
            DepthMapRequestPtr pRequest);
    //
    MBOOL runNormalPass(
            DepthMapRequestPtr pRequest);
    //
    MBOOL requireAlgoDataFromRequest(
            DepthMapRequestPtr pRequest,
            GF_HAL_IN_DATA& inData,
            GF_HAL_OUT_DATA& outData,
            bool bIsCapture = false);

    MBOOL requireInputMetaFromRequest(
            DepthMapRequestPtr pRequest,
            GF_HAL_IN_DATA& inData);
    //
    MVOID debugGFParams(
            const GF_HAL_IN_DATA& inData,
            const GF_HAL_OUT_DATA& outData);
//************************************************************************
// Private member
//************************************************************************
private:
    WaitQueue<DepthMapRequestPtr>           mJobQueue;
    GF_HAL* mpGf_Hal = nullptr; // GF HAL for PV/VR
    //
    MBOOL mbIsWaitDepth = MFALSE;
};

};
};
};
#endif // _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_GF_NODE_H