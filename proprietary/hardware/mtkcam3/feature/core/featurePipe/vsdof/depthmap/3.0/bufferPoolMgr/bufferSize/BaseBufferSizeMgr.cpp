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
 * @file BaseBufferSizeMgr.cpp
 * @brief base class for buffer size mgr
*/

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "BaseBufferSizeMgr.h"
// Logging header file
#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "BaseBufferSizeMgr"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH);

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

/*******************************************************************************
* Global Define
********************************************************************************/

/*******************************************************************************
* External Function
********************************************************************************/

/*******************************************************************************
* Enum Define
********************************************************************************/


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferSizeMgr::P2ABufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
BaseBufferSizeMgr::P2ABufferSize::
debug() const
{
    MY_LOGD("P2A debug size: scenario=%d, ", mScenario);

    DEBUG_MSIZE(mFD_IMG_SIZE);
    DEBUG_MSIZE(mFEB_INPUT_SIZE_MAIN1);
    DEBUG_MSIZE(mFEB_INPUT_SIZE_MAIN2);
    DEBUG_MSIZE(mFEC_INPUT_SIZE_MAIN1);
    DEBUG_MSIZE(mFEC_INPUT_SIZE_MAIN2);
    DEBUG_MSIZE(mRECT_IN_SIZE_MAIN1);
    DEBUG_MSIZE(mRECT_IN_SIZE_MAIN2);
    DEBUG_MSIZE(mRECT_IN_CONTENT_SIZE_MAIN1);
    DEBUG_MSIZE(mRECT_IN_CONTENT_SIZE_MAIN2);
    DEBUG_MSIZE(mMAIN_IMAGE_SIZE);
    DEBUG_MSIZE(mCCIN_SIZE_MAIN1);
    DEBUG_MSIZE(mCCIN_SIZE_MAIN2);
    DEBUG_MSIZE(mMYS_SIZE);

    DEBUG_AREA(mFEAO_AREA_MAIN2);
    DEBUG_AREA(mFEBO_AREA_MAIN2);
    DEBUG_AREA(mFECO_AREA_MAIN2);
    DEBUG_AREA(mFD_IMG_CROP);
    DEBUG_AREA(mMAIN_IMAGE_CROP);
    DEBUG_AREA(mFEB_INPUT_CROP_MAIN1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferSizeMgr::N3DBufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
BaseBufferSizeMgr::N3DBufferSize::
debug() const
{
    MY_LOGD("N3D debug size: scenario=%d, ", mScenario);

    DEBUG_MSIZE(mWARP_IMG_SIZE);
    DEBUG_MSIZE(mWARP_MASK_SIZE);
    DEBUG_MSIZE(mLDC_SIZE);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferSizeMgr::DPEBufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
BaseBufferSizeMgr::DPEBufferSize::
debug() const
{
    MY_LOGD("DPE debug size: scenario=%d, ", mScenario);

    DEBUG_MSIZE(mDMP_SIZE);
    DEBUG_MSIZE(mCFM_SIZE);
    DEBUG_MSIZE(mRESPO_SIZE);

    MY_LOGD("mTARGET_IMG_WIDTH=%d, mTARGET_IMG_HEIGHT=%d",
            mTARGET_IMG_WIDTH, mTARGET_IMG_HEIGHT);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferSizeMgr::OCCBufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
BaseBufferSizeMgr::OCCBufferSize::
debug() const
{
    MY_LOGD("OCC debug size: scenario=%d, ", mScenario);
    DEBUG_MSIZE(mDMH_SIZE);
    DEBUG_MSIZE(mMYS_SIZE);
    DEBUG_MSIZE(mOCCNOC_SIZE);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferSizeMgr::WMFBufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
BaseBufferSizeMgr::WMFBufferSize::
debug() const
{
    MY_LOGD("WMFBufferSize size======>\n");
    DEBUG_MSIZE(mDMW_SIZE);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferSizeMgr::GFBufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
BaseBufferSizeMgr::GFBufferSize::
debug() const
{
    MY_LOGD("GFBufferSize size======>\n");
    DEBUG_MSIZE(mDEPTHMAP_SIZE);
    DEBUG_MSIZE(mDMBG_SIZE);
}

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam



