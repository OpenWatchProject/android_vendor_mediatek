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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_FUSION_NODE_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_FUSION_NODE_H_

// Standard C header file
#include <map>
#include <vector>
#include <mutex>
#include <future>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
// Local header file
#include "CaptureFeatureNode.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

/*******************************************************************************
* Class Define
********************************************************************************/

/**
 * @brief FusionNode Node of the CaptureFeaturePipe
 */
class FusionNode final: public CaptureFeatureNode,
                        public NSPipelinePlugin::FusionPlugin::RequestCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CaptureFeatureNode Public Type Alias.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    using Plugin            = NSPipelinePlugin::FusionPlugin;
    using PluginPtr         = Plugin::Ptr;
    using PluginRequestPtr  = Plugin::Request::Ptr;
    using PipeRequestPtr    = RequestPtr;
    using BufferHandlePtr   = NSPipelinePlugin::BufferHandle::Ptr;
    using MetadataHandlePtr = NSPipelinePlugin::MetadataHandle::Ptr;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    FusionNode(NodeID_T nodeId, const char* name, MINT32 policy = SCHED_NORMAL, MINT32 priority = DEFAULT_CAMTHREAD_PRIORITY);

    ~FusionNode() override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FusionPlugin::RequestCallback Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    void onAborted(PluginRequestPtr pluginRequestPtr) override;

    void onCompleted(PluginRequestPtr pluginRequestPtr, MERROR status) override;

    void onNextCapture(PluginRequestPtr pluginRequestPtr) override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CaptureFeatureDataHandler Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL onData(DataID dataId, const RequestPtr& pRequest) override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CaptureFeatureNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MERROR evaluate(NodeID_T nodeId, CaptureFeatureInferenceData& inferenceData) override;

    MVOID dispatch(const RequestPtr& pRequest, NodeID_T nodeId = NULL_NODE) override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CaptureFeatureNode Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
     MBOOL onInit() override;
     MBOOL onUninit() override;
     MBOOL onThreadStart() override;
     MBOOL onThreadStop() override;
     MBOOL onThreadLoop() override;
     MVOID onFlush() override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CaptureFeatureNode Private Type Alias.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    using Property           = Plugin::Property;
    using Selection          = Plugin::Selection;
    using SelectionPtr       = Plugin::Selection::Ptr;
    using RequestCallback    = Plugin::RequestCallback;
    using RequestCallbackPtr = RequestCallback::Ptr;
    using ProviderPtr        = Plugin::IProvider::Ptr;
    using InterfacePtr       = Plugin::IInterface::Ptr;
    //
    using InitJob            = std::future<MVOID>;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FusionNode Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MVOID process(ProviderPtr& providerPtr, RequestPtr& pipeRequestPtr);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CaptureFeatureNode Private Inner Class Define.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief struct to store the requests
     */
    struct RequestPair
    {
        RequestPtr          mPipeRequest;
        PluginRequestPtr    mPluginRequest;
    };
    /**
     * @brief struct to store the provider info
     */
    struct ProviderInfo
    {
        ProviderPtr     mProviderPtr;
        SelectionPtr    mSelectionPtr;
        InitJob         mInitJob;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CaptureFeatureNode Private Type Alias.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    using FeaturePairItem = std::pair<FeatureID_T, ProviderInfo>;
    using ProviderInfoTable = std::map<FeatureID_T, ProviderInfo>;
    using RequestPairItem = std::pair<MVOID*, RequestPair>;
    using RequestPairTable = std::map<MVOID*, RequestPair>;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CaptureFeatureNode Private Member Data.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    PluginPtr               mPluginPtr;
    ProviderInfoTable       mProviderInfoTable;
    RequestPairTable        mRequestPairTable;

    RequestCallbackPtr      mRequestCallbackPtr;

    std::mutex              mRequestPairLock;

    WaitQueue<RequestPtr>   mRequests;
};

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_FUSION_NODE_H_
