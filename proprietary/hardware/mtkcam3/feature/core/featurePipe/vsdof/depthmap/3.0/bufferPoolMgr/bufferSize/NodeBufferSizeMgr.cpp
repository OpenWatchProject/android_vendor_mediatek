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
 * @file NodeBufferSizeMgr.cpp
 * @brief Buffer size provider for stereo features
*/

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
// Module header file
// Local header file
#include "NodeBufferSizeMgr.h"
#include "../../DepthMapPipe_Common.h"
// Logging header file
#define PIPE_CLASS_TAG "NodeBufferSizeMgr"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH);

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSCam::NSIoPipe;
using namespace StereoHAL;
/*******************************************************************************
* External Function
********************************************************************************/

/*******************************************************************************
* Enum Define
********************************************************************************/


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferSizeMgr::NodeP2ABufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NodeBufferSizeMgr::NodeP2ABufferSize::
NodeP2ABufferSize(
    ENUM_STEREO_SCENARIO scenario,
    sp<DepthMapPipeOption> pOption
)
: P2ABufferSize(scenario)
{
    MY_LOGD("+");
    StereoSizeProvider* pSizePrvder = StereoSizeProvider::getInstance();

    Pass2SizeInfo pass2SizeInfo;
    pSizePrvder->getPass2SizeInfo(PASS2A_P, scenario, pass2SizeInfo);
    mFEB_INPUT_SIZE_MAIN2 = pass2SizeInfo.areaWROT;
    mFEAO_AREA_MAIN2 = pass2SizeInfo.areaFEO;
    // frame 1
    pSizePrvder->getPass2SizeInfo(PASS2A, scenario, pass2SizeInfo);
    mFD_IMG_SIZE = pass2SizeInfo.areaIMG2O;
    mMAIN_IMAGE_SIZE = pass2SizeInfo.areaWDMA.size;
    mFEB_INPUT_SIZE_MAIN1 = pass2SizeInfo.areaWROT;
    // crop info
    if(pOption->mSensorType == v1::Stereo::BAYER_AND_BAYER && !pOption->mbEnableLCE)
        pSizePrvder->getPass2SizeInfo(PASS2A_CROP, scenario, pass2SizeInfo);
    else
        pSizePrvder->getPass2SizeInfo(PASS2A_B_CROP, scenario, pass2SizeInfo);
    mFD_IMG_CROP = pass2SizeInfo.areaIMG2O;
    mMAIN_IMAGE_CROP = pass2SizeInfo.areaWDMA;
    mFEB_INPUT_CROP_MAIN1 = pass2SizeInfo.areaWROT;
    // frame 2
    pSizePrvder->getPass2SizeInfo(PASS2A_P_2, scenario, pass2SizeInfo);
    mFEC_INPUT_SIZE_MAIN2 = pass2SizeInfo.areaIMG2O;
    mRECT_IN_SIZE_MAIN2 = pass2SizeInfo.areaWDMA.size;
    mFEBO_AREA_MAIN2 = pass2SizeInfo.areaFEO;
    mRECT_IN_CONTENT_SIZE_MAIN2 = pass2SizeInfo.areaWDMA.size - pass2SizeInfo.areaWDMA.padding;
    // frame 3
    pSizePrvder->getPass2SizeInfo(PASS2A_2, scenario, pass2SizeInfo);
    mFEC_INPUT_SIZE_MAIN1 = pass2SizeInfo.areaIMG2O;
    mRECT_IN_SIZE_MAIN1 = pass2SizeInfo.areaWDMA.size;
    mRECT_IN_CONTENT_SIZE_MAIN1 = pass2SizeInfo.areaWDMA.size - pass2SizeInfo.areaWDMA.padding;
    // frame 4
    pSizePrvder->getPass2SizeInfo(PASS2A_P_3, scenario, pass2SizeInfo);
    mFECO_AREA_MAIN2 = pass2SizeInfo.areaFEO;
    // cc_in
    mCCIN_SIZE_MAIN2 = pass2SizeInfo.areaIMG2O;
    // frame 5
    pSizePrvder->getPass2SizeInfo(PASS2A_3, scenario, pass2SizeInfo);
    mCCIN_SIZE_MAIN1 = pass2SizeInfo.areaIMG2O;
    //
    mMYS_SIZE = pSizePrvder->getBufferSize(E_MY_S, scenario);

    debug();
    MY_LOGD("-");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferSizeMgr::N3DBufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NodeBufferSizeMgr::NodeN3DBufferSize::
NodeN3DBufferSize(ENUM_STEREO_SCENARIO scenario)
: N3DBufferSize(scenario)
{
   MY_LOGD("+");
    // setup constants
    StereoSizeProvider * pSizeProvder = StereoSizeProvider::getInstance();
    mWARP_IMG_SIZE = pSizeProvder->getBufferSize(E_MV_Y, scenario);
    mWARP_MASK_SIZE = pSizeProvder->getBufferSize(E_MASK_M_Y, scenario);
    // LDC
    mLDC_SIZE = pSizeProvder->getBufferSize(E_LDC, scenario);
    debug();
    MY_LOGD("-");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferSizeMgr::NodeDPEBufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NodeBufferSizeMgr::NodeDPEBufferSize::
NodeDPEBufferSize(ENUM_STEREO_SCENARIO scenario)
: DPEBufferSize(scenario)
{
    MY_LOGD("+");
    // setup constans
    StereoSizeProvider* pSizePrvder = StereoSizeProvider::getInstance();
    mDMP_SIZE = pSizePrvder->getBufferSize(E_DMP_H, scenario);
    mCFM_SIZE = pSizePrvder->getBufferSize(E_CFM_H, scenario);
    mRESPO_SIZE = pSizePrvder->getBufferSize(E_RESPO, scenario);

    // original image width is the size before N3D in Cap
    Pass2SizeInfo pass2SizeInfo;
    pSizePrvder->getPass2SizeInfo(PASS2A_P_2, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    mTARGET_IMG_WIDTH = pass2SizeInfo.areaWDMA.size.w;
    mTARGET_IMG_HEIGHT = pass2SizeInfo.areaWDMA.size.h;

    debug();
    MY_LOGD("+");
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferSizeMgr::NodeOCCBufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NodeBufferSizeMgr::NodeOCCBufferSize::
NodeOCCBufferSize(ENUM_STEREO_SCENARIO scenario)
: OCCBufferSize(scenario)
{
    MY_LOGD("+");
    // setup constants
    StereoSizeProvider * pSizeProvder = StereoSizeProvider::getInstance();
    mDMH_SIZE = pSizeProvder->getBufferSize(E_DMH, scenario);
    mMYS_SIZE = pSizeProvder->getBufferSize(E_MY_S, scenario);
    mOCCNOC_SIZE = pSizeProvder->getBufferSize(E_NOC, scenario);
    debug();
    MY_LOGD("-");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferSizeMgr::NodeWMFBufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NodeBufferSizeMgr::NodeWMFBufferSize::
NodeWMFBufferSize(ENUM_STEREO_SCENARIO scenario)
: WMFBufferSize(scenario)
{
    MY_LOGD("+");
    // setup constants
    StereoSizeProvider * pSizeProvder = StereoSizeProvider::getInstance();
    mDMW_SIZE = pSizeProvder->getBufferSize(E_DMW, scenario);
    debug();
    MY_LOGD("-");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferSizeMgr::NodeGFBufferSize class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NodeBufferSizeMgr::NodeGFBufferSize::
NodeGFBufferSize(ENUM_STEREO_SCENARIO scenario)
: GFBufferSize(scenario)
{
    MY_LOGD("+");
    // setup constants
    StereoSizeProvider * pSizeProvder = StereoSizeProvider::getInstance();
    mDEPTHMAP_SIZE = pSizeProvder->getBufferSize(E_DEPTH_MAP, scenario);
    mDMBG_SIZE = pSizeProvder->getBufferSize(E_DMBG, scenario);
    debug();
    MY_LOGD("-");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferSizeMgr class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NodeBufferSizeMgr::
NodeBufferSizeMgr(sp<DepthMapPipeOption> pOption)
{
    ENUM_STEREO_SCENARIO scen[] = {eSTEREO_SCENARIO_RECORD, eSTEREO_SCENARIO_CAPTURE, eSTEREO_SCENARIO_PREVIEW};

    for(int index=0;index<3;++index)
    {
        mP2ASize.add(scen[index], NodeP2ABufferSize(scen[index], pOption));
        mN3DSize.add(scen[index], NodeN3DBufferSize(scen[index]));
        mDPESize.add(scen[index], NodeDPEBufferSize(scen[index]));
        mOCCSize.add(scen[index], NodeOCCBufferSize(scen[index]));
        mWMFSize.add(scen[index], NodeWMFBufferSize(scen[index]));
        mGFSize.add(scen[index], NodeGFBufferSize(scen[index]));
    }
}

NodeBufferSizeMgr::
~NodeBufferSizeMgr()
{
}

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam



