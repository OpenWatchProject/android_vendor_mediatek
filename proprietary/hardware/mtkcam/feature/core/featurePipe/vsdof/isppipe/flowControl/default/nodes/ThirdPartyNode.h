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
 * @file ThirdPartyNode.h
 * @brief third-party node for isp pipe
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_TP_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISP_PIPE_NODE_TP_H_

// Standard C header file

// Android system/core header file
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
// mtkcam custom header file

// mtkcam global header file
// Module header file
#include <DpBlitStream.h>
#include <featurePipe/vsdof/util/vsdof_util.h>
#include <featurePipe/core/include/WaitQueue.h>
// Local header file
#include "../DefaultIspPipeFlow_Common.h"
#include "../../../IspPipeNode.h"


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

const int MAX_THIRDPARTY_FD_FACES_NUM = 15;

struct ThirdpartyInputFdInfo
{
    MPoint roiTopLeft      = MPoint(0, 0);        // FD ROI top left point
    MPoint roiBotRight     = MPoint(0, 0);        // FD ROI bottom right point
    MINT32 degreeRotInPlane = 0;                  // FD rotation degree, rotation in plane(RIP)
    MINT32 degreeRotOffPlane = 0;                 // FD rotation degree, rotation off plane(ROP)
};

struct ThirdpartyInputParameter
{
    MINT32 miFaceCount;
    ThirdpartyInputFdInfo mFdInfo[MAX_THIRDPARTY_FD_FACES_NUM];
    MUINT8 featureModeStatus;

    MSize  mPreviewSize;
    MPoint mAFTopLeft;
    MPoint mAFBottomRight;
    MINT32 mAFType;
    MFLOAT mAFDistance;
    MUINT8 mAFState;
    MINT32 mSensorOrientation;
    MINT32 mPreviewBokehLevel;
    MPoint mFocusCenter;
};

/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class ThirdPartyNode
 * @brief Node class for third-party SW
 */
class ThirdPartyNode: public IspPipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ThirdPartyNode(
            const char *name,
            IspPipeNodeID nodeID,
            const PipeNodeConfigs& config);
    virtual ~ThirdPartyNode();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL onData(DataID id, const IspPipeRequestPtr& request);
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadLoop();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MVOID onFlush();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief execute third-party SW
     * @param [in] pRequest Current request
     * @param [in] qTemplate QParam template
     * @param [out] rQParams QParams to be output
     * @return
     * - Tuning result
     */
    MBOOL executeThirdParty(
                    const IspPipeRequestPtr& pRequest
                    );
    /**
     * @brief execute third-party SW for preview request
     * @param [in] pRequest Current request
     * @param [in] inputParam Thridparty input parameters
     */
    MBOOL onHandlePreviewRequest(
                    const IspPipeRequestPtr& pRequest,
                    const ThirdpartyInputParameter& inputParam
                    );
    /**
     * @brief execute third-party SW for capture request
     */
    MBOOL onHandleCaptureRequest(
                    const IspPipeRequestPtr& pRequest
                    );
    /**
     * @brief prepare thirdparty input parameter
     * @param [in] pRequest Current request
     * @param [out] rInputParam input parameters
     */
    MBOOL prepareThirdpartyInputParameter(
                    const IspPipeRequestPtr& pRequest,
                    ThirdpartyInputParameter& rInputParam
                    );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyNode Private Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // request queue
    WaitQueue<IspPipeRequestPtr> mRequestQue;
    DpBlitStream* mpDpStream = nullptr;;
    // thirdparty
    MINT32 miThirdpartyEnable = 1;
    MINT64 miThirdpartyFdTolerence = 0;
};


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
#endif