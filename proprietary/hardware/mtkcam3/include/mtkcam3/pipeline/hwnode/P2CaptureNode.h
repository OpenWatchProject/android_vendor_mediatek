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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_HWNODE_P2_CAPTURE_NODE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_HWNODE_P2_CAPTURE_NODE_H_
//
#include <mtkcam3/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>
#include <mtkcam3/pipeline/stream/IStreamBuffer.h>
#include <mtkcam3/pipeline/utils/streambuf/IStreamBufferPool.h>
//
#include <mtkcam3/pipeline/hwnode/P2Common.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
class P2CaptureNode
    : virtual public IPipelineNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    typedef IPipelineNode::InitParams       InitParams;

    enum ePass2Type
    {
        PASS2_TIMESHARING,
        PASS2_CAPTURE,
        PASS2_TYPE_TOTAL
    };

    enum eCustomOption
    {
        CUSTOM_OPTION_NONE = 0,
        CUSTOM_OPTION_SENSOR_4CELL          = 1 << 0,
        CUSTOM_OPTION_CLEAR_ZOOM_SUPPORT    = 1 << 1,
        CUSTOM_OPTION_DRE_SUPPORT           = 1 << 2
    };


    struct  ConfigParams
    {
        ConfigParams()
            : uCustomOption(0)
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
        android::Vector<android::sp<IImageStreamInfo>>
                                      pvInFullRaw;

        /**
         * A pointer to a full-size raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInResizedRaw;

        /**
         * A pointer to input image stream info. (full-zsl input port)
         */
        android::Vector<android::sp<IImageStreamInfo>>
                                      pvInOpaque;

        /**
         * A pointer to YUV reprocessing image stream info.
         */
        android::sp<IImageStreamInfo> pInFullYuv;
        android::sp<IImageStreamInfo> pInFullYuv2;

        /**
         * A pointer to a lcso raw input image stream info.
         */
        android::sp<IImageStreamInfo> pInLcsoRaw;

        /**
         * A set of pointer to output image stream info.
         */
        android::Vector<android::sp<IImageStreamInfo>>
                                      pvOutImage;

        /**
         * A pointer to JPEG YUV image stream info.
         */
        android::sp<IImageStreamInfo> pOutJpegYuv;

        /**
         * A pointer to post view image stream info.
         */
        android::sp<IImageStreamInfo> pOutPostView;

        /**
         * A pointer to clean image stream info.
         */
        android::sp<IImageStreamInfo> pOutClean;

        /**
         * A pointer to clean image stream info.
         */
        android::sp<IImageStreamInfo> pOutBokeh;

        /**
         * A pointer to depth stream info.
         */
        android::sp<IImageStreamInfo> pOutDepth;

        /**
         * A pointer to thumbnail image stream info.
         */
        android::sp<IImageStreamInfo> pOutThumbnailYuv;

        struct PhysicalStream
        {
            /**
             * A pointer to a set of input app meta stream info.
             * It's for multi-cam's one physical stream use.
             */
            android::sp<IMetaStreamInfo>  pInAppPhysicalMeta;
            /**
             * A pointer to a set of output app meta stream info.
             * It's for multi-cam's one physical stream use.
             */
            android::sp<IMetaStreamInfo>  pOutAppPhysicalMeta;
            MUINT32  sensorId = -1;
        };

        android::Vector<struct PhysicalStream> vPhysicalStreamsInfo;

        struct P1SubStreams
        {
            android::sp<IMetaStreamInfo>  pInAppRetMetaSub = NULL;
            android::sp<IMetaStreamInfo>  pInHalMetaSub = NULL;
            android::sp<IImageStreamInfo> pInFullRawSub = NULL;
            android::sp<IImageStreamInfo> pInResizedRawSub = NULL;
            android::sp<IImageStreamInfo> pInLcsoRawSub = NULL;
            MUINT32  sensorId = -1;
        };

        android::Vector<struct P1SubStreams> vP1SubStreamsInfo;

        /**
         * A set of pointer to out raw image stream info.
         */
        android::Vector<android::sp<IImageStreamInfo>> pvOutRaw;

        /**
         * customize option
         */
        MUINT32 uCustomOption;

        /**
        * UsageHint for common HAL3 P2
        */
        P2Common::Capture::UsageHint mUsageHint;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    static android::sp<P2CaptureNode>   createInstance(ePass2Type type, P2Common::Capture::UsageHint const& usage);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual MERROR                      init(InitParams const& rParams) = 0;

    virtual MERROR                      config(ConfigParams const& rParams) = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_HWNODE_P2_CAPTURE_NODE_H_

