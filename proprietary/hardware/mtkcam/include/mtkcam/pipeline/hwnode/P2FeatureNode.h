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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2_FEATURE_NODE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2_FEATURE_NODE_H_
//
#include <mtkcam/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include <mtkcam/pipeline/utils/streambuf/IStreamBufferPool.h>
#include <mtkcam/utils/hw/IResourceConcurrency.h>
#include <mtkcam/feature/eis/EisInfo.h>
#include <mtkcam/utils/imgbuf/ISecureImageBufferHeap.h>
//

#define FEATURE_MODIFY

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
class P2FeatureNode
    : virtual public IPipelineNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.
    typedef android::Vector<android::sp<IMetaStreamInfo> >  MetaStreamInfoSetT;
    typedef android::Vector<android::sp<IImageStreamInfo> > ImageStreamInfoSetT;

    typedef IPipelineNode::InitParams       InitParams;

    enum ePass2Type
    {
        PASS2_STREAM,
        PASS2_TIMESHARING,
        PASS2_TYPE_TOTAL
    };
    enum eCustomOption
    {
        CUSTOM_OPTION_NONE = 0,
        CUSTOM_OPTION_SENSOR_4CELL          = 1 << 0,
        CUSTOM_OPTION_CLEAR_ZOOM_SUPPORT    = 1 << 1,
        CUSTOM_OPTION_DRE_SUPPORT           = 1 << 2
    };

#ifdef FEATURE_MODIFY
    enum eUsageMode
    {
        USAGE_DEFAULT,
        USAGE_PREVIEW,
        USAGE_RECORD,
        USAGE_CAPTURE,
        USAGE_TIMESHARE_CAPTURE,
    };

    struct UsageHint
    {
        eUsageMode mUsageMode;
        MSize mStreamingSize;
        MSize mPreviewSize;
        MSize mVideoSize;
        MINT32 m3DNRMode;// 3DNR flag
        MINT32 mFSCMode;
        MUINT32 mDualMode;
        NSCam::SecType mSecType;
        NSCam::EIS::EisInfo mEisInfo;
        android::sp<IResourceConcurrency> pResourceConcurrency;

        UsageHint()
            : mUsageMode(USAGE_DEFAULT)
            , mStreamingSize(0, 0)
            , mPreviewSize(0, 0)
            , mVideoSize(0, 0)
            , m3DNRMode(0)
            , mFSCMode(0)
            , mDualMode(0)
            , mSecType(NSCam::SecType::mem_normal)
        {
        }

        UsageHint(eUsageMode mode, const MSize &streamingSize = MSize(0,0), const MUINT32 eisMode = 0, const MUINT32 dualMode = 0)
            : mUsageMode(mode)
            , mStreamingSize(streamingSize)
            , mPreviewSize(0, 0)
            , mVideoSize(0, 0)
            , m3DNRMode(0)
            , mFSCMode(0)
            , mDualMode(dualMode)
            , mSecType(NSCam::SecType::mem_normal)
        {
            mEisInfo.mode = eisMode;
        }
    };

#endif // FEATURE_MODIFY

    struct  ConfigParams
    {
        ConfigParams()
            : burstNum(0)
            , customOption(0)
            {};
        /**
         * A pointer to a set of input app meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInAppMeta;

#ifdef FEATURE_MODIFY
        /**
         * A pointer to a set of input app result meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInAppRetMeta;
#endif // FEATURE_MODIFY
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
         * A pointer to a full-size raw input image stream info.
         */
        android::Vector<android::sp<IImageStreamInfo> > pvInFullRaw;

        /**
         * A pointer to a full-size raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInResizedRaw;

        /**
         * A pointer to a lcso raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInLcsoRaw;

        /**
         * A pointer to a rsso raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInRssoRaw;

        /**
         * A pointer to a set of output image stream info.
         */
        ImageStreamInfoSetT           vOutImage;

        /**
         * A pointer to FD image stream info.
         */
        android::sp<IImageStreamInfo> pOutFDImage;

        /**
         * The number of burst processing: default value is 0.
         */
        MUINT8 burstNum;

        /**
        * customize option
        */
        MUINT32 customOption;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    static android::sp<P2FeatureNode>      createInstance(ePass2Type type = PASS2_STREAM, const UsageHint usageHint = UsageHint());
    static android::sp<P2FeatureNode>      createHal1Instance(ePass2Type type = PASS2_STREAM, const UsageHint usageHint = UsageHint());


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.

    virtual MERROR                  init(InitParams const& rParams) = 0;

    virtual MERROR                  config(ConfigParams const& rParams) = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2_FEATURE_NODE_H_

