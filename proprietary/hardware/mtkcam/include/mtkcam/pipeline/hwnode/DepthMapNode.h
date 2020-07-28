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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_DEPTHMAPNODE_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_DEPTHMAPNODE_
//
#include <mtkcam/pipeline/pipeline/IPipelineNode.h>
#include <mtkcam/feature/stereo/StereoCamEnum.h>
//


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

using namespace NSCam::v1::Stereo;
/******************************************************************************
 *
 ******************************************************************************/
class DepthMapNode
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

        /**
         * Feature mode
         */
        StereoFeatureMode          featureMode;
        /**
         * Sensor state
         */
        SeneorModuleType           moduleType;
    };

    struct  ConfigParams
    {
         /**
         * A pointer to a set of input app meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInAppMeta = NULL;

        /**
         * A pointer to a set of input hal meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInHalMeta = NULL;
        /**
         * A pointer to a set of input hal meta stream info.
         */
        android::sp<IMetaStreamInfo>  pInHalMeta_Main2 = NULL;

        /**
         * A pointer to a set of output app meta stream info.
         */
        android::sp<IMetaStreamInfo>  pOutAppMeta = NULL;

        /**
         * A pointer to a set of output hal meta stream info.
         */
        android::sp<IMetaStreamInfo>  pOutHalMeta = NULL;

        /**
         * A pointer to a full-size raw input image stream info of main 1.
         */
        android::sp<IImageStreamInfo>  pInFullRaw = NULL;

        /**
         * A pointer to a full-size raw input image stream info of main 11
         */
        android::sp<IImageStreamInfo>  pInFullRaw_Main2 = NULL;

        /**
         * A pointer to a resize raw input image stream info of main 1.
         */
        android::sp<IImageStreamInfo> pInResizedRaw = NULL;
        /**
         * A pointer to a resize raw input image stream info of main 2.
         */
        android::sp<IImageStreamInfo> pInResizedRaw_Main2 = NULL;
        /**
         * A pointer to a lcso input image stream info of main 1.
         */
        android::sp<IImageStreamInfo> pInLcso = NULL;
        /**
         * A pointer to a lcso input image stream info of main 2.
         */
        android::sp<IImageStreamInfo> pInLcso_Main2 = NULL;
        /**
         * A pointer to MainImage stream info.
         */
        android::sp<IImageStreamInfo> pHalImageMainImg = NULL;
        /**
         * A pointer to MainImage_Cap stream info.
         */
        android::sp<IImageStreamInfo> pHalImageMainImg_Cap = NULL;
        /**
         * A pointer to DMBG image stream info.
         */
        android::sp<IImageStreamInfo> pHalImageDMBG = NULL;
        /**
         * A pointer to JSPMain image stream info.
         */
        android::sp<IImageStreamInfo> pHalImageJPSMain1 = NULL;
        /**
         * A pointer to JPSMain2 image stream info.
         */
        android::sp<IImageStreamInfo> pHalImageJPSMain2 = NULL;
        /**
         * A pointer to disparity_left image stream info.
         */
        android::sp<IImageStreamInfo> pHalImageDisparityMap_Left = NULL;
        /**
         * A pointer to disparity_right image stream info.
         */
        android::sp<IImageStreamInfo> pHalImageDisparityMap_Right = NULL;
        /**
         * A pointer to WarpingMatrix stream info.
         */
        android::sp<IImageStreamInfo> pHalImageWarpingMatrix = NULL;
        /**
         * A pointer to SceneInfo stream info.
         */
        android::sp<IImageStreamInfo> pHalImageSceneInfo = NULL;
        /**
         * A pointer to FD image stream info.
         */
        android::sp<IImageStreamInfo> pHalImageFD = NULL;

        /**
         * A pointer to hal DepthMap image stream info.
         */
        android::sp<IImageStreamInfo> pHalImageDepthMap = NULL;

        /**
         * A pointer to ExtraData image stream info.
         */
        android::sp<IImageStreamInfo> pAppImageExtraData = NULL;
        /**
         * A pointer to LDC image stream info.
         */
        android::sp<IImageStreamInfo> pAppImageLDC = NULL;
        /**
         * A pointer to DepthMap image stream info.
         */
        android::sp<IImageStreamInfo> pAppImageDepthMap = NULL;
        /**
         * A pointer to hal depth wrapper image stream info.
         */
        android::sp<IImageStreamInfo> pHalImageDepthWrapper = NULL;
        /**
         * A pointer to N3DDebug image stream info.
         */
        android::sp<IImageStreamInfo> pAppImageN3DDebug = NULL;
        /**
         * A pointer to Postview image stream info.
         */
        android::sp<IImageStreamInfo> pAppImagePostview = NULL;
        /**
         * A pointer to tuning image stream info.
         */
        android::sp<IImageStreamInfo> pAppImageTuning_Main1 = NULL;

    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    static android::sp<DepthMapNode>   createInstance();

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
#endif

