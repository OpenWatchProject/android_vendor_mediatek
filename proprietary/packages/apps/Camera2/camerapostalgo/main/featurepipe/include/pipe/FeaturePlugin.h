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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_PLUGIN_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_PLUGIN_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtk/mtk_feature_type.h>
#include <plugin/PipelinePlugin.h>
// Local header file
#include <pipe/FeatureRequest.h>

//using namespace NSCam::NSPipelinePlugin;


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {


class PluginHelper
{
public:
    static NSCam::NSPipelinePlugin::BufferHandle::Ptr CreateBuffer(
           sp<FeatureNodeRequest> pNodeRequest, TypeID_T typeId, Direction dir);

    static NSCam::NSPipelinePlugin::BufferHandle::Ptr CreateBuffer(
           sp<FeatureNodeRequest> pNodeRequest, BufferID_T bufId);

    static NSCam::NSPipelinePlugin::MetadataHandle::Ptr CreateMetadata(
           sp<FeatureNodeRequest> pNodeRequest, MetadataID_T metaId);
};

/*******************************************************************************
* Class Define
*******************************************************************************/
/**
 * @brief implemented class of the NSCam::NSPipelinePlugin::BufferHandle
 */
class PluginBufferHandle final: public NSCam::NSPipelinePlugin::BufferHandle
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PluginBufferHandle(sp<FeatureNodeRequest> pNodeRequest, BufferID_T bufId);

    ~PluginBufferHandle();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NSCam::NSPipelinePlugin::BufferHandle Public Operators.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    IImageBuffer* acquire(MINT usage) override;

    MVOID release() override;

    MVOID dump(std::ostream& os) const override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginBufferHandle Private Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    wp<FeatureNodeRequest>   mpNodeRequest;
    IImageBuffer*                   mpImageBuffer;
    BufferID_T                      mBufferId;
};

/**
 * @brief implemented class of the NSCam::NSPipelinePlugin::MetadataHandle
 */
class PluginMetadataHandle : public NSCam::NSPipelinePlugin::MetadataHandle
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PluginMetadataHandle(sp<FeatureNodeRequest> pNodeRequest, MetadataID_T metaId);

    ~PluginMetadataHandle();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NSCam::NSPipelinePlugin::MetadataHandle Public Operators.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    IMetadata* acquire() override;

    MVOID release() override;

    MVOID dump(std::ostream& os) const override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginMetadataHandle Private Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    wp<FeatureNodeRequest>   mpNodeRequest;
    IMetadata*                      mpMetadata;
    MetadataID_T                    mMetaId;
};


} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com


#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_PLUGIN_H_
