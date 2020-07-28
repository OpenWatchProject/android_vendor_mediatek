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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_DUALTHIRDPARTY_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_DUALTHIRDPARTY_
//
#include <mtkcam/pipeline/pipeline/IPipelineNode.h>
//


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
/******************************************************************************
 * Dual3rdPartyNode is used to produce third party dual cam flow.
 ******************************************************************************/
class Dual3rdPartyNode
    : virtual public IPipelineNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    typedef IPipelineNode::InitParams       InitParams_Base;

    struct  InitParams
    {
        /**
         * An index to indicate which camera device to open.
         */
        MINT32                      openId;
        MINT32                      openId_main2;

        /**
         * A unique value for the node id.
         */
        NodeId_T                    nodeId;

        /**
         * A pointer to a null-terminated string for the node name.
         */
        char const*                 nodeName;
    };

    struct  ConfigParams
    {
        /**
         * A pointer to a set of input app meta stream info.
         */
        android::sp<IMetaStreamInfo> pInAppMeta;

        /**
         * A pointer to a set of output app meta stream info.
         */
        android::sp<IMetaStreamInfo> pOutAppMeta;

        /**
         * A pointer to a set of input hal meta stream info.
         */
        android::sp<IMetaStreamInfo> pInHalMeta;

        /**
         * A pointer to a set of input hal meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInHalMeta_Main2 = NULL;
        /**
         * A pointer to a set of output hal meta stream info.
         */
        android::sp<IMetaStreamInfo> pOutHalMeta;

        /**
         * A pointer to a full-size raw input image stream info of main 1.
         */
        android::sp<IImageStreamInfo>  pInFullRaw;

        /**
         * A pointer to a full-size raw input image stream info of main 2
         */
        android::sp<IImageStreamInfo>  pInFullRaw_Main2;

        /**
         * A pointer to a resize raw input image stream info of main 1.
         */
        android::sp<IImageStreamInfo> pInResizedRaw;

        /**
         * A pointer to a resize raw input image stream info of main 2
         */
        android::sp<IImageStreamInfo> pInResizedRaw_Main2;

        /**
         * A pointer to a resize raw input image stream info of main 2
         */
        android::sp<IImageStreamInfo> pInLCSO;

        /**
         * A pointer to a resize raw input image stream info of main 2
         */
        android::sp<IImageStreamInfo> pInLCSO_Main2;

        /**
         * A pointer to a resize YUV input image stream info of main 1
         */
        android::sp<IImageStreamInfo> pInResize_YUV;
        /**
         * A pointer to a resize YUV input image stream info of main 2
         */
        android::sp<IImageStreamInfo> pInResize_YUV_Main2;
        /**
         * A pointer to a fullsize YUV input image stream info of main 1
         */
        android::sp<IImageStreamInfo> pInFullSize_YUV;
        /**
         * A pointer to a fullsize YUV input image stream info of main 2
         */
        android::sp<IImageStreamInfo> pInFullSize_YUV_Main2;
        /**
         * A pointer to MainImage stream info.
         */
        android::sp<IImageStreamInfo> pOutAppPrvImg;
        /**
         * A ImageStream for preview callback
         */
        android::sp<IImageStreamInfo> pOutAppPrvCB;
        /**
         * A ImageStream for preview FD
         */
        android::sp<IImageStreamInfo> pOutAppPrvFD;
        /**
         * A pointer to pOutCapBokehResult stream info.
         */
        android::sp<IImageStreamInfo> pOutHalCapBokehResult;

        /**
         * A pointer to pOutCapDepthResult stream info.
         */
        android::sp<IImageStreamInfo> pOutAppCapDepthResult;

        /**
         * A pointer to Thumbnail_Cap stream info.
         */
        android::sp<IImageStreamInfo> pOutHalThumbnailImg_Cap;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    static android::sp<Dual3rdPartyNode>   createInstance();

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
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_DUALTHIRDPARTY_

