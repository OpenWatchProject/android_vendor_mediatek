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
 * @file NodeBufferSizeMgr.h
 * @brief buffer size providers for Stereo features, ex.VSDOF
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BUFFERSIZE_MGR_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BUFFERSIZE_MGR_H_

// Standard C header file

// Android system/core header file
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>
// Module header file
// Local header file
#include "BaseBufferSizeMgr.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace StereoHAL;
using namespace android;

/*******************************************************************************
* Struct Definition
********************************************************************************/

/*******************************************************************************
* Type Definition
********************************************************************************/
typedef BaseBufferSizeMgr::P2ABufferSize P2ABufferSize;
typedef BaseBufferSizeMgr::N3DBufferSize N3DBufferSize;
typedef BaseBufferSizeMgr::DPEBufferSize DPEBufferSize;
typedef BaseBufferSizeMgr::OCCBufferSize OCCBufferSize;
typedef BaseBufferSizeMgr::WMFBufferSize WMFBufferSize;
typedef BaseBufferSizeMgr::GFBufferSize GFBufferSize;
/*******************************************************************************
* Class Definition
********************************************************************************/

/**
 * @class NodeBufferSizeMgr
 * @brief control all the buffer size in VSDOF
 */
class NodeBufferSizeMgr : public BaseBufferSizeMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual ~NodeBufferSizeMgr();
    NodeBufferSizeMgr(sp<DepthMapPipeOption> pOption);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferSizeMgr Public Structure
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    struct NodeP2ABufferSize : public P2ABufferSize
    {
        NodeP2ABufferSize(){};
        NodeP2ABufferSize(
            ENUM_STEREO_SCENARIO scenario,
            sp<DepthMapPipeOption> pOption);
    };

    struct NodeN3DBufferSize : public N3DBufferSize
    {
        NodeN3DBufferSize(){};
        NodeN3DBufferSize(ENUM_STEREO_SCENARIO scenario);
    };

    struct NodeDPEBufferSize : public DPEBufferSize
    {
        NodeDPEBufferSize(){};
        NodeDPEBufferSize(ENUM_STEREO_SCENARIO scenario);
    };

    struct NodeOCCBufferSize : public OCCBufferSize
    {
        NodeOCCBufferSize(){};
        NodeOCCBufferSize(ENUM_STEREO_SCENARIO scenario);
    };

    struct NodeWMFBufferSize : public WMFBufferSize
    {
        NodeWMFBufferSize(){};
        NodeWMFBufferSize(ENUM_STEREO_SCENARIO scenario);
    };

    struct NodeGFBufferSize : public GFBufferSize
    {
        NodeGFBufferSize(){};
        NodeGFBufferSize(ENUM_STEREO_SCENARIO scenario);
    };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferSizeMgr Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual const P2ABufferSize& getP2A(ENUM_STEREO_SCENARIO scen) {return mP2ASize.valueFor(scen);}
    virtual const N3DBufferSize& getN3D(ENUM_STEREO_SCENARIO scen) {return mN3DSize.valueFor(scen);}
    virtual const DPEBufferSize& getDPE(ENUM_STEREO_SCENARIO scen) {return mDPESize.valueFor(scen);}
    virtual const OCCBufferSize& getOCC(ENUM_STEREO_SCENARIO scen) {return mOCCSize.valueFor(scen);}
    virtual const WMFBufferSize& getWMF(ENUM_STEREO_SCENARIO scen) {return mWMFSize.valueFor(scen);}
    virtual const GFBufferSize& getGF(ENUM_STEREO_SCENARIO scen) {return mGFSize.valueFor(scen);}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferSizeMgr Public Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    KeyedVector<ENUM_STEREO_SCENARIO, NodeP2ABufferSize> mP2ASize;
    KeyedVector<ENUM_STEREO_SCENARIO, NodeN3DBufferSize> mN3DSize;
    KeyedVector<ENUM_STEREO_SCENARIO, NodeDPEBufferSize> mDPESize;
    KeyedVector<ENUM_STEREO_SCENARIO, NodeOCCBufferSize> mOCCSize;
    KeyedVector<ENUM_STEREO_SCENARIO, NodeWMFBufferSize> mWMFSize;
    KeyedVector<ENUM_STEREO_SCENARIO, NodeGFBufferSize> mGFSize;
};

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif
