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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_BOKEHPIPENODE_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_BOKEHPIPENODE_
//
#include <mtkcam/pipeline/pipeline/IPipelineNode.h>
//
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
/******************************************************************************
 *
 ******************************************************************************/
class BokehNode
    : virtual public IPipelineNode
{
public:
enum Mode
{
    ALL_HW,
    HW_BOKEH_NODE,
    SW_BOKEH_NODE,
    VENDOR_BOKEH_NODE
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    typedef IPipelineNode::InitParams       InitParams;

    struct  ConfigParams
    {
        /**
         * A pointer to a set of input app meta stream info.
         */
        android::sp<IMetaStreamInfo> pInAppMeta;

        /**
         * A pointer to a set of input hal meta stream info from DepthMapNode
         */
        android::sp<IMetaStreamInfo> pInHalMeta;

        /**
         * A pointer to a set of output app meta stream info.
         */
        android::sp<IMetaStreamInfo> pOutAppMeta;

        /**
         * A pointer to a set of output hal meta stream info.
         */
        android::sp<IMetaStreamInfo> pOutHalMeta;

        /**
         * A pointer to a YV12 input image stream info.
         * This image is processed by DepthMapNode.
         * Image size:
         *      Preview -> (1920, 1080)
         *      Record  -> (1920, 1080)
         */
        android::sp<IImageStreamInfo> pInHalImageMainImage;

        /**
         * A pointer to a YV12 input image stream info.
         * This image is processed by DepthMapNode.
         * Image size:
         *      Capture -> (3072, 1728)
         */
        android::sp<IImageStreamInfo> pInHalImageMainImage_Capture;

        /**
         * A pointer to a Y8 input image stream info.
         * This image is processed by DepthMapNode.
         * Image size:
         */
        android::sp<IImageStreamInfo> pInHalDepthWrapper;

        /**
         * A pointer to a Y8 input image stream info.
         * This image is processed by DepthMapNode.
         * Image size:
         */
        android::sp<IImageStreamInfo> pInHalExtraData;

        /**
         * A pointer to a YV12 input image stream info.
         * This image is processed by DepthMapNode and is output by OCCL. alg..
         * The width and the height of My_S is fixed and image is (240, 135).
         */
        //android::sp<IImageStreamInfo> pInHalImageMYS;

        /**
         * A pointer to a Y image stream info.
         * This image is processed by DepthMapNode and is output by GF.
         * The width and the height of DMG is fixed and image is (240, 135).
         */
        android::sp<IImageStreamInfo> pInHalImageDMBG;

        /**
         * A pointer to a Y image stream info.
         * This image is processed by DepthMapNode.
         */
        android::sp<IImageStreamInfo> pInHalImageDepth;

        /**
         * A pointer to a Y image stream info.
         * This image is tuning buffer.
         */
        android::sp<IImageStreamInfo> pInHalImageP2Tuning;

        /**
         * A pointer to a YV12 image stream info.
         * This image is output by P2B which is contain Bokeh and Gaussian effect.
         * The output size is depend on display size.
         */
        android::sp<IImageStreamInfo> pOutAppImagePostView;

        /**
         * A pointer to a YV12 image stream info.
         * This image is output by P2B and is just output in Capture scenario.
         * The output size is (3072, 1728).
         */
        android::sp<IImageStreamInfo> pOutHalImageCleanImage;

        /**
         * A pointer to a YV12 image stream info.
         * This image is output by P2B which is contain Bokeh and Gaussian effect.
         * The output size is (1920, 1080).
         */
        android::sp<IImageStreamInfo> pOutAppImageRecord;

        /**
         * A pointer to a YV12 image stream info.
         * This image is output by P2B which is contain Bokeh and Gaussian effect.
         * The output size is (1920, 1080).
         */
        android::sp<IImageStreamInfo> pOutAppImagePreview;

        /**
         * A pointer to a YV12 image stream info.
         * This image is output by P2B which is contain Bokeh and Gaussian effect.
         * The output size is (1920, 1080).
         */
        android::sp<IImageStreamInfo> pOutHalImageBokehResult;

        /**
         * A pointer to a YV12 image stream info.
         * This image is thnumbnail.
         * It is output by P2B which is contain Bokeh and Gaussian effect.
         * The output size is set by AP.
         */
        android::sp<IImageStreamInfo> pOutHalImageBokehThumbnail;

        /**
         * A ImageStream for preview callback
         */
        android::sp<IImageStreamInfo> pOutAppPrvCB;

        /**
         * A pointer to a Y8 input image stream info.
         * Image size:
         */
        android::sp<IImageStreamInfo> pOutAppDepthMapWrapper;

        /**
         * A pointer to a Y8 input image stream info.
         * Image size:
         */
        android::sp<IImageStreamInfo> pOutAppExtraData;

        /**
         * bokeh mode
         */
        Mode bokehMode;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    static android::sp<BokehNode>   createInstance();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.

    virtual MERROR                  init(InitParams const& rParams)         = 0;

    virtual MERROR                  config(ConfigParams const& rParams)     = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_BOKEHPIPENODE_

