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
 * @file NodeBufferPoolMgr_VSDOF.h
 * @brief BufferPoolMgr for VSDOF
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_VSDOF_BUFFERPOOL_MGR_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_VSDOF_BUFFERPOOL_MGR_H_

// Standard C header file

// Android system/core header file
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
// Module header file

// Local header file
#include "BaseBufferPoolMgr.h"
#include "./bufferSize/BaseBufferSizeMgr.h"
#include "../DepthMapPipe_Common.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using android::sp;

/*******************************************************************************
* Struct Definition
********************************************************************************/

/*******************************************************************************
* Class Definition
********************************************************************************/

/**
 * @class NodeBufferPoolMgr_VSDOF
 * @brief Base class of BufferPoolMgr
 */
class NodeBufferPoolMgr_VSDOF : public BaseBufferPoolMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    NodeBufferPoolMgr_VSDOF(
                    PipeNodeBitSet& nodeBitSet,
                    sp<DepthMapPipeSetting> pPipeSetting,
                    sp<DepthMapPipeOption> pPipeOption);
    virtual ~NodeBufferPoolMgr_VSDOF();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferPoolMgr Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual SmartFatImageBuffer request(
                            DepthMapBufferID id,
                            BufferPoolScenario scen
                            );
    virtual SmartGraphicBuffer requestGB(
                            DepthMapBufferID id,
                            BufferPoolScenario scen
                            );

    virtual SmartTuningBuffer requestTB(
                            DepthMapBufferID id,
                            BufferPoolScenario scen
                            );

    virtual sp<BaseBufferHandler> createBufferPoolHandler();

    virtual MBOOL queryBufferType(
                            DepthMapBufferID bid,
                            BufferPoolScenario scen,
                            DepthBufferType& rOutBufType);

    virtual sp<FatImageBufferPool> getImageBufferPool(
                            DepthMapBufferID bufferID,
                            BufferPoolScenario scenario=eBUFFER_POOL_SCENARIO_PREVIEW);

    virtual sp<GraphicBufferPool> getGraphicImageBufferPool(
                            DepthMapBufferID bufferID,
                            BufferPoolScenario scenario=eBUFFER_POOL_SCENARIO_PREVIEW);

    virtual MBOOL getAllPoolImageBuffer(
                        DepthMapBufferID id,
                        BufferPoolScenario scen,
                        std::vector<IImageBuffer*>& rImgVec
                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferPoolMgr_VSDOF Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferPoolMgr_VSDOF Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief initialize all buffer pool
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL initializeBufferPool();

    /**
     * @brief initialize node's buffer pools
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL initP2ABufferPool();
    MBOOL initN3DWPEBufferPool();
    MBOOL initDVSBufferPool();
    MBOOL initDVPBufferPool();
    MBOOL initGFBufferPool();

    /**
     * @brief initialize FEO/FMO buffer pools
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL initFEFMBufferPool();

    /**
     * @brief build the mapping between buffer id and image buffer pool
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL buildImageBufferPoolMap();

    /**
     * @brief build the mapping between buffer id and its buffer type with specific scenario
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL buildBufScenarioToTypeMap();
    /**
     * @brief uninit function
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL uninit();
    /**
     * @brief init BT160 color space for graphic buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MVOID initBT160ColorSpaceForYUVGB(sp<GraphicBufferPool> pool);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferPoolMgr_VSDOF Public Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferPoolMgr_VSDOF Private Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // node bit set: indicate which buffer are active
    PipeNodeBitSet mNodeBitSet;
    // buffer pool mapping
    KeyedVector<DepthMapBufferID, sp<FatImageBufferPool> > mBIDtoImgBufPoolMap_Default;
    KeyedVector<DepthMapBufferID, ScenarioToImgBufPoolMap> mBIDtoImgBufPoolMap_Scenario;
    // graphic pool mapping
    KeyedVector<DepthMapBufferID, ScenarioToGraBufPoolMap> mBIDtoGraBufPoolMap_Scenario;
    // Use to define the buffer type of all buffers inside this bufferPoolMgr, which means working buffer.
    KeyedVector<DepthMapBufferID, BufScenarioToTypeMap> mBIDToScenarioTypeMap;
    // size mgr
    BaseBufferSizeMgr* mpBufferSizeMgr;
    // pipe option
    sp<DepthMapPipeOption> mpPipeOption;
    sp<DepthMapPipeSetting> mpPipeSetting;
    // requst bufffer map
    RequestBufferMap mRequestBufferIDMap;
    RequestBufferMap mRequestMetaIDMap;
    // ++++++++++++++++++++ Buffer Pools ++++++++++++++++++++++++++++++++//
    //----------------------P2A section--------------------------------//
    // Rectify_in Buffer
    sp<FatImageBufferPool> mpRectInBufPool_Main1_CAP;
    // Rect_in main2
    sp<FatImageBufferPool> mpRectInBufPool_Main2_VR;
    sp<FatImageBufferPool> mpRectInBufPool_Main2_PV;
    sp<FatImageBufferPool> mpRectInBufPool_Main2_CAP;
    // FEO buffer
    sp<FatImageBufferPool> mpFEOB_BufPool;
    sp<FatImageBufferPool> mpFEOC_BufPool;
    sp<FatImageBufferPool> mpFMOB_BufPool;
    sp<FatImageBufferPool> mpFMOC_BufPool;
    // FE Input Buffer
    sp<FatImageBufferPool> mpFEBInBufPool_Main1;
    sp<FatImageBufferPool> mpFEBInBufPool_Main2;
    sp<FatImageBufferPool> mpFECInBufPool_Main1;
    sp<FatImageBufferPool> mpFECInBufPool_Main2;
    // tuning buffer
    sp<TuningBufferPool> mpTuningBufferPool;
    sp<TuningBufferPool> mpPQTuningBufferPool;
    sp<TuningBufferPool> mpDpPQParamTuningBufferPool;
    #ifdef GTEST
    // For UT requirement - dump fe input buffer output at IMG3O
    sp<FatImageBufferPool> mFEHWInput_StageB_Main1;
    sp<FatImageBufferPool> mFEHWInput_StageB_Main2;
    sp<FatImageBufferPool> mFEHWInput_StageC_Main1;
    sp<FatImageBufferPool> mFEHWInput_StageC_Main2;
    #endif
    sp<FatImageBufferPool> mMYSImgBufPool;
    // IMG3O
    sp<FatImageBufferPool> mIMG3OmgBufPool;
    sp<FatImageBufferPool> mInternalFDImgBufPool;
    //----------------------N3D section--------------------------------//
    sp<GraphicBufferPool> mN3DWarpingMatrix_Main2_X;
    sp<GraphicBufferPool> mN3DWarpingMatrix_Main2_X_CAP;
    sp<GraphicBufferPool> mN3DWarpingMatrix_Main2_Y;
    sp<GraphicBufferPool> mN3DWarpingMatrix_Main2_Y_CAP;
    //----------------------WPE section--------------------------------//
    // WPE_IN_MASK
    sp<FatImageBufferPool> mDefaultMaskBufPool_Main2;
    // MV_Y
    sp<FatImageBufferPool> mWarpImgBufPool_Main1;
    sp<FatImageBufferPool> mWarpImgBufPool_Main1_CAP;
    // MASK_M
    sp<FatImageBufferPool> mWarpMaskBufPool_Main1;
    sp<FatImageBufferPool> mWarpMaskBufPool_Main1_CAP;
    // SV_Y
    sp<FatImageBufferPool> mWarpImgBufPool_Main2;
    sp<FatImageBufferPool> mWarpImgBufPool_Main2_CAP;
    // MASK_S
    sp<FatImageBufferPool> mWarpMaskBufPool_Main2;
    sp<FatImageBufferPool> mWarpMaskBufPool_Main2_CAP;
    //----------------------DPEsection--------------------------------//
    sp<FatImageBufferPool> mDVLRImgBufPool;
    sp<FatImageBufferPool> mCFMImgBufPool;
    sp<FatImageBufferPool> mNOCImgBufPool;
    sp<FatImageBufferPool> mASFCRMImgBufPool;
    sp<FatImageBufferPool> mASFRDImgBufPool;
    sp<FatImageBufferPool> mASFHFImgBufPool;
    sp<FatImageBufferPool> mDMWImgBufPool;
    //----------------------GF section--------------------------------//
    sp<FatImageBufferPool> mpInternalDMBGImgBufPool;


};


}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif
