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
 * @file IIspPipeFlowControler.h
 * @brief flow control interface for IspPipe
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISPPIPE_FLOW_CONTROLER_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISPPIPE_FLOW_CONTROLER_H_

// Standard C header file
#include <string>
// Android system/core header file
#include <utils/RefBase.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/feature/stereo/pipe/IIspPipe.h>
// Module header file

// Local header file
#include "IspPipe.h"
#include "IspPipeRequest.h"
#include "PipeBufferPoolMgr.h"
#include "IIspPipeFlow_Interface.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class IIspPipeFlowControler
 * @brief flow control interface for IspPipe
 */
class IspPipe;
class IIspPipeFlowControler : public RefBase
{
public:
    /**
     * @brief create the desire isp flow control type
     * @param [in] type: flow controler type
     * @param [in] setting: isp pipe settings
     * @param [in] option: isp pipe options
     * @return
     * - nullptr indicates failure
     * - otherwise indicates success
     */
    static sp<IIspPipeFlowControler> createInstance(
                                            IspPipeFlowControlType type,
                                            const IspPipeSetting& setting,
                                            const IspPipeOption& option);
    virtual ~IIspPipeFlowControler() {};

public:
    /**
     * @brief get flow control name
     */
    virtual const std::string& getName() const = 0;
    /**
     * @brief get isp pipe setting
     */
    virtual const IspPipeSetting& getSetting() const = 0;
    /**
     * @brief get isp pipe functional options
     */
    virtual const IspPipeOption& getOption() const = 0;
    /**
     * @brief get IDataIDToBIDMapProvider
     */
    virtual sp<IDataIDToBIDMapProvider> getDataIDToBIDMapProvider() const = 0;
    /**
     * @brief get IIspPipeConverter
     */
    virtual sp<IIspPipeConverter> getIspPipeConverter() const = 0;
    /**
     * @brief buid the pipe buffer pools for argument mgr
     * @param [in] mgr : the pipe buffer pool mgr that need to build pools inside
     */
    virtual MVOID BuildPipeBufferPool(PipeBufferPoolMgr& mgr) const = 0;
    /**
     * @brief buid the pipe node and the related connections for argument pipe
     * @param [in] pipe : the isp pipe that need to build nodes and set conections inside
     * @param [in] nodeMap : nodes created in this operator will add in this argument
     */
    virtual MVOID BuildPipeNodes(IspPipe& pipe, KeyedVector<IspPipeNodeID, IspPipeNode*>& nodeMap) const = 0;
    /**
     * @brief Handle data when that come from other node
     * @param [in] id : data id
     * @param [in] pRequest : request from other node
     */
    // TODO: the IspPipeDataID need to dependence on IspPipe::DataID
    virtual MBOOL HandlePipeOnData(IspPipeDataID id, const IspPipeRequestPtr& pRequest) = 0;
};



}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam


#endif