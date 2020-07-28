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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_H_
//
#include <mtkcam/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include <mtkcam/pipeline/utils/streambuf/IStreamBufferPool.h>
//
#include <mtkcam/pipeline/extension/IVendorManager.h>
#include <mtkcam/pipeline/hwnode/P2Common.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
class P2Node
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
        CUSTOM_OPTION_SENSOR_4CELL       = 1 << 0,
        CUSTOM_OPTION_CLEAR_ZOOM_SUPPORT = 1 << 1,
        CUSTOM_OPTION_DRE_SUPPORT        = 1 << 2,
        CUSTOM_OPTION_DOWNSCALE_DN       = 1 << 3
    };

    struct  ConfigParams
    {
        ConfigParams()
            : pInAppRetMeta(NULL)
            , pOutCaptureImage(NULL)
            , burstNum(0)
            , pVendor(NULL)
            , uUserId(0)
            , customOption(0)
        {};

        /**
         * A pointer to a set of input app meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInAppMeta;

        /**
         * A pointer to a set of input app result meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInAppRetMeta;

        /**
         * A pointer to a set of input hal meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInHalMeta;

        /**
         * A pointer to a set of input app result meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInAppRetMeta_Sub;

        /**
         * A pointer to a set of input hal meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInHalMeta_Sub;

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
        android::Vector<android::sp<IImageStreamInfo>> pvInFullRaw;

        /**
         * A pointer to a full-size raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInResizedRaw;

        /**
         * A pointer to a lcs raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInLcsoRaw;

        /**
         * A pointer to a rss raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInRssoRaw;

        /**
         * A pointer to a full-size raw input image stream info.
         */
        android::Vector<android::sp<IImageStreamInfo>> pvInFullRaw_Sub;

        /**
         * A pointer to a full-size raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInResizedRaw_Sub;

        /**
         * A pointer to a lcs raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInLcsoRaw_Sub;

        /**
         * A pointer to a rss raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInRssoRaw_Sub;

        /**
         * A pointer to a set of output image stream info.
         */
        ImageStreamInfoSetT           vOutImage;

        /**
         * A pointer to FD image stream info.
         */
        android::sp<IImageStreamInfo> pOutFDImage;

        /**
         * A pointer to YUV reprocessing image stream info.
         */
        android::sp<IImageStreamInfo> pInYuvImage;

        /**
         * A pointer to Capture image stream info.
         */
        android::sp<IImageStreamInfo> pOutCaptureImage;

        /**
         * The number of burst processing: default value is 0.
         */
        MUINT8 burstNum;

        /**
         * A pointer to input image stream info. (full-zsl input port)
         */
        android::Vector<android::sp<IImageStreamInfo>> pvInOpaque;

        /**
         * A pointer to input image stream info. (full-zsl input port)
         */
        android::Vector<android::sp<IImageStreamInfo>> pvInOpaque_Sub;

        /**
        * A pointer to IVendorManager
        */
        android::sp<NSCam::plugin::IVendorManager> pVendor;
        /**
        * user's id
        */
        MUINT64 uUserId;

        /**
        * customize option
        */
        MUINT32 customOption;

        /**
        * UsageHint for common HAL3 P2
        */
        P2Common::UsageHint mUsageHint;

    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    static android::sp<P2Node>      createInstance(ePass2Type type = PASS2_STREAM);
    static android::sp<P2Node>      createInstance(ePass2Type type, P2Common::UsageHint usage);

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
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_H_

