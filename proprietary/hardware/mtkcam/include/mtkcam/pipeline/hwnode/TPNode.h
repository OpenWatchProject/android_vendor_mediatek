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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_TPNode_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_TPNode_H_

#include <mtkcam/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include <mtkcam/pipeline/utils/streambuf/IStreamBufferPool.h>

#include <vector>

// ---------------------------------------------------------------------------

namespace NSCam {
namespace v3 {

class TPNode
    : public virtual IPipelineNode
{
public:
    typedef android::Vector<android::sp<IImageStreamInfo>> ImageStreamInfoSetT;

    typedef IPipelineNode::InitParams       InitParams;

    struct  ConfigParams
    {
        /**
         * A pointer to a set of input app meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInAppMeta;

        /**
         * A pointer to a set of input hal meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInHalMeta;

        /**
         * A pointer to a set of output app meta stream info.
         */
        android::sp<IMetaStreamInfo>  pOutAppMeta;

        /**
         * A pointer to a set of output hal meta stream info.
         */
        android::sp<IMetaStreamInfo>  pOutHalMeta;

        /**
         * A pointer to a full-size yuv input image stream info.
         */
        android::sp<IImageStreamInfo> pInFullYuv;

        /**
         * A pointer to a resized yuv input image stream info.
         */
        android::sp<IImageStreamInfo> pInResizedYuv;

        /**
         * A pointer to a binning yuv input image stream info.
         */
        android::sp<IImageStreamInfo> pInBinningYuv;

        /**
         * A pointer to a yuv jpeg output image stream info.
         */
        android::sp<IImageStreamInfo> pOutYuvJpeg;

        /**
         * A pointer to a yuv thumbnail output image stream info.
         */
        android::sp<IImageStreamInfo> pOutYuvThumbnail;

        /**
         * A pointer to a yuv 00 output image stream info.
         */
        android::sp<IImageStreamInfo> pOutYuv00;

        /**
         * A pointer to a yuv 01 output image stream info.
         */
        android::sp<IImageStreamInfo> pOutYuv01;

        /**
         * A pointer to a depth output image stream info.
         */
        android::sp<IImageStreamInfo> pOutDepth;

        /**
         * A pointer to a clean output image stream info.
         */
        android::sp<IImageStreamInfo> pOutClean;
    };

    // IPipelineNode Interface
    virtual MERROR                  init(InitParams const& rParams) = 0;
    virtual MERROR                  config(ConfigParams const& params) = 0;

    static android::sp<TPNode>      createInstance();
};

}; // namespace v3
}; // namespace NSCam

#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_TPNode_H_
