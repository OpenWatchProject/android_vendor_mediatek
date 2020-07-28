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
 * @file FDNode.h
 * @brief Face detection node inside the DepthMapPipe
*/
#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_FD_NODE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_FD_NODE_H

// Standard C header file

// Android system/core header file
#include <utils/Mutex.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam3/feature/FaceDetection/fd_hal_base.h>
// Module header file
#include <vsdof/hal/json_util.h>
#include <featurePipe/core/include/WaitQueue.h>
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

class StereoFDNode: public DepthMapPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    StereoFDNode(
        const char *name,
        DepthMapPipeNodeID nodeID,
        PipeNodeConfigs config);
    virtual ~StereoFDNode();

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
//  StereoFDNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief Get the rotated FD result from the FDHal output result
     * @param [in] inputFaceInfo input FD FaceInfo result
     * @param [in] iRotation current rotation
     * @param [out] rRotatedFDResult rotated FD result
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MVOID calcRotatedFDResult(
                        MtkCameraFace inputFaceInfo,
                        MUINT32 iRotation,
                        FD_DATA_STEREO_T &rRotatedFDResult);

    /**
     * @brief Merge the json data of FD and N3D node
     * @param [in] pEffReqPtr request
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL mergeExtraData(DepthMapRequestPtr& pEffReqPtr);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StereoFDNode Private Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    WaitQueue<DepthMapRequestPtr> mJobQueue;
    halFDBase* mpFDHalObj = nullptr;
    MUINT8* mpFDWorkingBuffer = nullptr;
    MUINT8* mpDDPBuffer = nullptr;
    MUINT8* mpExtractYBuffer = nullptr;
    MtkCameraFaceMetadata* mpDetectedFaces = nullptr;
    JSON_Util* mpJsonUtil = nullptr;

    KeyedVector<MUINT32, String8> mvFDJsonMap;
    KeyedVector<MUINT32, String8> mvN3DJsonMap;
    android::Mutex mJsonMutex;

    MSize mFD_IMG_SIZE;

};

}; //NSFeaturePipe_DepthMap
}; //NSCamFeature
}; //NSCam

#endif
