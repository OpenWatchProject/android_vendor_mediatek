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
 * @file DepthMapFlowOption.h
 * @brief Flow option template for DepthMapNode
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_FLOWOPTION_DEPTHMAPNODE_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_FLOWOPTION_DEPTHMAPNODE_H_

// Standard C header file

// Android system/core header file
#include <utils/KeyedVector.h>
// mtkcam custom header file
// mtkcam global header file
#include <mtkcam/aaa/IHal3A.h>
#include <isp_tuning.h>
// Module header file

// Local header file
#include "P2AFlowOption.h"
#include "../DepthMapPipe_Common.h"
#include "../DataStorage.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using NSIspTuning::EIspProfile_T;
/*******************************************************************************
* Forward declaration
********************************************************************************/
class DepthMapEffectRequest;
class DepthMapPipe;
class DepthMapPipeNode;

/*******************************************************************************
* Enum Define
********************************************************************************/
typedef enum eFlowOptionID
{
    DEPTHMAP_FLOWOPTION_VSDOF,
    DEPTHMAP_FLOWOPTION_BMVSDOF
} FlowOptionID;

/*******************************************************************************
* Structure Define
********************************************************************************/
struct FlowOptionConfig
{
    // capture FD is enable or not
    MBOOL mbCaptureFDEnable;
    // FD buffer size
    MSize mFDSize;
};
/*******************************************************************************
* Class Definition
********************************************************************************/

/**
 * @class DepthMapFlowOption
 * @brief Flow option template class for P2A Node
 */
class DepthMapFlowOption: public android::VirtualLightRefBase
                        , public P2AFlowOption
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief init this flow option
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL init() = 0;
    /**
     * @brief query the effect request attributes from the effect request
     * @param [in] pRequest effect request
     * @param [out] rReqAttrs request attributes output
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL queryReqAttrs(
                        sp<DepthMapEffectRequest> pRequest,
                        EffectRequestAttrs& rReqAttrs) = 0;
    /**
     * @brief query the node is enable or not of this flow option
     * @param [out] nodeBitSet bitset to indicate each node is enable or not
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL queryPipeNodeBitSet(PipeNodeBitSet& nodeBitSet) = 0;

    /**
     * @brief build and connect the node graph of this flow option
     * @param [in] pPipe feature pipe pointer
     * @param [in] nodeMap Active node map of this flow option
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL buildPipeGraph(
                        DepthMapPipe* pPipe,
                        const DepthMapPipeNodeMap& nodeMap) = 0;

    /**
     * @brief check the DataID is connected or not in this option
     * @param [in] dataID the node that performed handleData
     * @return
     * - MTRUE indicates connected.
     * - MFALSE indicates disconnected.
     */
    virtual MBOOL checkConnected(
                        DepthMapDataID dataID
                        ) = 0;

    /**
     * @brief get the remapped buffer for special use
     * @param [in] reqAttr request attributes
     * @param [in] bufferID bufferID to be remapped
     * @return
     * - remapped buffer id
     */
    virtual DepthMapBufferID reMapBufferID(
                        const EffectRequestAttrs& reqAttr,
                        DepthMapBufferID bufferID
                        ) = 0;

    /**
     * @brief Manually control the 3A tuning
     * @param [in] pRequest current request
     * @param [in] path  main1 or main2 path
     * @param [out] MetaSet
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates failure
     */
    virtual MBOOL config3ATuningMeta(
        sp<DepthMapEffectRequest> pRequest,
        StereoP2Path path,
        MetaSet_T& rMetaSet
        ) = 0;

    /**
     * @brief Manually control the 3A tuning
     * @param [in] pRequest current request
     * @param [in] path  main1 or main2 path
     * @param [out] MetaSet
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates failure
     */
    virtual MBOOL config3ATuningMeta_Bayer(
        sp<DepthMapEffectRequest> pRequest,
        MetaSet_T& rMetaSet
        ) = 0;

    /**
     * @brief get the isp profile of bayer run
     * @param [in] pRequest current request
     * @return
     * - ISP Profile
     */
    virtual EIspProfile_T getIspProfile_Bayer(sp<DepthMapEffectRequest> pRequest) = 0;

    /**
     * @brief get the static flow option config of this FlowOption
     * @param [in] pRequest current request
     * @param [out] pConfig  pointer to config
     * @return
     * - MTRUE indicates yes
     * - MFALSE indicates no
     */
    virtual const FlowOptionConfig& getOptionConfig() = 0;
    /**
     * @brief get the ID of this flow option
     * @return
     * - flow option id
     */
    virtual const FlowOptionID getFlowOptionID() = 0;
    /**
     * @brief check the condition to bypass the P2A node
     * @param [in] pRequest current request
     * @return
     * - MTRUE indicates yes
     * - MFALSE indicates no
     */
    virtual MBOOL needBypassP2A(sp<DepthMapEffectRequest> pRequest) = 0;

    /**
     * @brief get the 3dnr VIPI buffer of current run
     */
    virtual IImageBuffer* get3DNRVIPIBuffer() = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapFlowOption Protected Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    // data storage
    sp<DepthInfoStorage> mpDepthStorage = nullptr;

};

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif
