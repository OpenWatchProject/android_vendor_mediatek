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
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_MFCNODE_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_MFCNODE_H_

#include <mtkcam/pipeline/pipeline/IPipelineNode.h> // IPipelineNode
#include <mtkcam/pipeline/stream/IStreamInfo.h> // IImageStreamInfo

using namespace android; // android::Vector, android::List, android::sp

namespace NSCam {
namespace v3 {

/**
 *  Multi-Frame Capture Node (MFCNode) is a super class for HDRNode and MfllNode.
 *  This class only provides typedefs, interfaces must be implemented information
 *  for derived classes (HDRNode, MfllNode). Notice that, you CAN NOT create a
 *  MFCNode instance, instead of, you may need to create derived classes directly.
 *
 *  e.g.:
 *      for HDR, invoke HDRNode::createInstance();
 *      for MFNR, invoke MfllNode::createInstance();
 *
 *  There's a common part implementation for HDR/MFNR usage, called MFCNodeImp,
 *  hence lots of common parts can be re-used for multi-frame capture node. Derived
 *  class implementations should inherit MFCNodeImp rather than MFCNode because
 *  it's supposed to be with this benifit of common parts implementation.
 *
 *  Implementation of MFCNode inherits BaseNode and the input/output of this node
 *  is:
 *      [Input]
 *          Meta/Request
 *
 *      [Output]
 *          Meta/Result
 *          Image/Yuv
 */
class MFCNode : public virtual IPipelineNode
{
/* typedefs & enumerations */
public:
    /* Typedefs long symbol to the shorter and meanful one */
    typedef List< sp<IPipelineFrame> >      IPipelineFrameList_t;
    typedef Vector< sp<IPipelineFrame> >    IPipelineFrameVector_t;
    typedef Vector< sp<IImageStreamInfo> >  ImageStreamInfoSetT;
    typedef IPipelineNode::InitParams       InitParams;

    /* Configurations of the node */
    typedef struct _ConfigParams {
        android::sp<IMetaStreamInfo>  pInAppMeta;   // A pointer to a set of input app meta stream info.
        android::sp<IMetaStreamInfo>  pInHalMeta;   // A pointer to a set of input hal meta stream info.
        android::sp<IMetaStreamInfo>  pOutAppMeta;  // A pointer to a set of output app meta stream info.
        android::sp<IMetaStreamInfo>  pOutHalMeta;  // A pointer to a set of output hal meta stream info.
        ImageStreamInfoSetT           vInFullRaw;   // A pointer to a full-size raw input image stream info.
        ImageStreamInfoSetT           vInLcsoRaw;   // A pointer to a lcso input buffer stream info.
        android::sp<IImageStreamInfo> pInResizedRaw;// A pointer to a resized raw input image stream info.
        android::sp<IImageStreamInfo> vOutYuvJpeg;  // A pointer to a yuv jpeg output image stream info.
        android::sp<IImageStreamInfo> vOutYuvThumbnail; // A pointer to a yuv thumbnail output image stream info.
        ImageStreamInfoSetT           vOutImage;    // A pointer to a set of output image stream info.
    } ConfigParams;

    typedef struct _MfcAttribute {
        int is_time_sharing;   // To tell node if node runs as time sharing mode (ZSD)
        _MfcAttribute()
        {
            is_time_sharing = 0;
        };
    } MfcAttribute;

public:
    /**
     *  A dummpy createInstance function for avoiding compiling error
     */
    static android::sp<MFCNode> createInstance(const MfcAttribute &attr = MfcAttribute());

public:
    /**
     *  Implement this method for PipelineContext to configure node
     *
     *  @param rParams      - MFCNode::ConfigParams to configure MFCNodes.
     *  @return             - Returns OK if works well.
     *  @notice             - This method is not inherited from IPipelineNode.
     */
    virtual MERROR config(const ConfigParams& rParams) = 0;

}; /* class MFCNode */
}; /* namespace v3 */
}; /* namespace NSCam */

#endif /* _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_MFCNODE_H_ */

