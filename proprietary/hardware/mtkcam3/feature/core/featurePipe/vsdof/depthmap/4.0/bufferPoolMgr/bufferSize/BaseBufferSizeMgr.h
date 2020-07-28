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
 * @file BaseBufferSizeMgr.h
 * @brief definition of all the buffer size for Stereo features
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BASEBUFFERSIZE_MGR_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BASEBUFFERSIZE_MGR_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/StereoArea.h>
// Module header file
// Local header file

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using android::sp;
using namespace StereoHAL;

/*******************************************************************************
* Global Definition
********************************************************************************/
#define DEBUG_MSIZE(sizeCons) \
    MY_LOGD("size: " #sizeCons " : %dx%d\n", sizeCons.w, sizeCons.h);

#define DEBUG_AREA(areaCons) \
    MY_LOGD("Area: " #areaCons " : size=%dx%d padd=%dx%d startPt=(%d, %d)\n", areaCons.size.w, areaCons.size.h, areaCons.padding.w, areaCons.padding.h, areaCons.startPt.x, areaCons.startPt.y);
/*******************************************************************************
* Type Definition
********************************************************************************/

/*******************************************************************************
* Class Definition
********************************************************************************/

/**
 * @class BaseBufferSizeMgr
 * @brief base class for buffer size mgr
 */
class BaseBufferSizeMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    BaseBufferSizeMgr() {};
    virtual ~BaseBufferSizeMgr() {};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferSizeMgr Public Structure
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    struct BaseBufferSize
    {
        BaseBufferSize()
        : mScenario(eSTEREO_SCENARIO_RECORD) {}

        BaseBufferSize(ENUM_STEREO_SCENARIO scenario)
        : mScenario(scenario) {}
        // scenario
        ENUM_STEREO_SCENARIO  mScenario;
    };

    struct P2ABufferSize : public BaseBufferSize
    {
        P2ABufferSize() {}
        P2ABufferSize(ENUM_STEREO_SCENARIO scenario)
        : BaseBufferSize(scenario) {};
        //
        MVOID debug() const;
        // size
        MSize mFD_IMG_SIZE;
        MSize mFEB_INPUT_SIZE_MAIN1;
        MSize mFEB_INPUT_SIZE_MAIN2;
        MSize mFEC_INPUT_SIZE_MAIN1;
        MSize mFEC_INPUT_SIZE_MAIN2;
        MSize mRECT_IN_SIZE_MAIN1;
        MSize mRECT_IN_SIZE_MAIN2;
        MSize mRECT_IN_CONTENT_SIZE_MAIN1;
        MSize mRECT_IN_CONTENT_SIZE_MAIN2;
        MSize mMAIN_IMAGE_SIZE;
        // crop info
        StereoArea mMAIN_IMAGE_CROP;
        StereoArea mFEB_INPUT_CROP_MAIN1;
        StereoArea mFD_IMG_CROP;
        // area
        StereoArea mFEAO_AREA_MAIN2;
        StereoArea mFEBO_AREA_MAIN2;
        StereoArea mFECO_AREA_MAIN2;
        // MYS
        MSize mMYS_SIZE;
    };

    struct N3DBufferSize : public BaseBufferSize
    {
        N3DBufferSize() {}
        N3DBufferSize(ENUM_STEREO_SCENARIO scenario)
        : BaseBufferSize(scenario) {};
        //
        MVOID debug() const;
        // buffer size const
        MSize mWARP_IN_MASK_MAIN2;
        MSize mWARP_IMG_SIZE;
        MSize mWARP_MASK_SIZE;
        MSize mWARP_MAP_SIZE_MAIN2;
    };

    struct DPEBufferSize : public BaseBufferSize
    {
        DPEBufferSize() {}
        DPEBufferSize(ENUM_STEREO_SCENARIO scenario)
        : BaseBufferSize(scenario) {};
        //
        MVOID debug() const;
        // size
        MSize mDV_LR_SIZE;
        MSize mCFM_SIZE;
        MSize mNOC_SIZE;
        MSize mASF_CRM_SIZE;
        MSize mASF_HF_SIZE;
        MSize mASF_RD_SIZE;
        MSize mDMW_SIZE;
    };

    struct GFBufferSize : public BaseBufferSize
    {
        GFBufferSize() {}
        GFBufferSize(ENUM_STEREO_SCENARIO scenario)
        : BaseBufferSize(scenario) {};
        //
        MVOID debug() const;
        //
        MSize mDEPTHMAP_SIZE;
        MSize mDMBG_SIZE;
    };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferSizeMgr Public Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual const P2ABufferSize& getP2A(ENUM_STEREO_SCENARIO scen) = 0;
    virtual const N3DBufferSize& getN3D(ENUM_STEREO_SCENARIO scen) = 0;
    virtual const DPEBufferSize& getDPE(ENUM_STEREO_SCENARIO scen) = 0;
    virtual const GFBufferSize& getGF(ENUM_STEREO_SCENARIO scen) = 0;

};

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif
