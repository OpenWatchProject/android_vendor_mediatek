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
 * @file IspPipeControlerBase.h
 * @brief define the basic operator/attributes for IspPipeControler
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISPPIPE_FLOW_CONTROLER_BASE_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISPPIPE_FLOW_CONTROLER_BASE_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "../IIspPipeFlowControler.h"
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
 * @class IspPipeControlerBase
 * @brief base class for IspPipe flow control
 */

class IspPipeControlerBase : public IIspPipeFlowControler
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IIspPipeFlowControler interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    const std::string& getName() const final override { return mName; }

    const IspPipeSetting& getSetting() const final override { return mSetting; }

    const IspPipeOption& getOption() const final override { return mOption; }

    sp<IDataIDToBIDMapProvider> getDataIDToBIDMapProvider() const override
    {
        return onGetDataIDToBIDMapProvider();
    }

    sp<IIspPipeConverter> getIspPipeConverter() const override
    {
        return onGetIspPipeConverter();
    }

    MVOID BuildPipeBufferPool(PipeBufferPoolMgr& mgr) const override
    {
        onBuildPipeBufferPool(mgr);
    }

    MVOID BuildPipeNodes(IspPipe& pipe, KeyedVector<IspPipeNodeID, IspPipeNode*>& nodeMap) const override
    {
        onBuildPipeNodes(pipe, nodeMap);
    }

    MBOOL HandlePipeOnData(IspPipeDataID id, const IspPipeRequestPtr& pRequest) override
    {
        return onHandlePipeOnData(id, pRequest);
    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeControlerBase Protected Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    IspPipeControlerBase(const std::string name, const IspPipeSetting& setting, const IspPipeOption& option)
    : mName(name)
    , mSetting(setting)
    , mOption(option)
    {

    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeControlerBase non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    virtual sp<IDataIDToBIDMapProvider> onGetDataIDToBIDMapProvider() const  = 0;

    virtual sp<IIspPipeConverter> onGetIspPipeConverter() const = 0;

    virtual MVOID onBuildPipeBufferPool(PipeBufferPoolMgr& mgr) const = 0;

    virtual MVOID onBuildPipeNodes(IspPipe& pipe, KeyedVector<IspPipeNodeID, IspPipeNode*>& nodeMap) const = 0;

    virtual MBOOL onHandlePipeOnData(IspPipeDataID id, const IspPipeRequestPtr& pRequest) = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeControlerBase Private Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    const std::string       mName;
    const IspPipeSetting    mSetting;
    const IspPipeOption     mOption;
};



}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam


#endif