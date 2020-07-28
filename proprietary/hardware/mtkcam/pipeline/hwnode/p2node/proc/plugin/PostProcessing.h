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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_POSTPROCESSING_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_POSTPROCESSING_H_

#include "../../P2Common.h"

namespace NSCam {
namespace v3 {

class PostProcessing {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  3rd Party Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    enum ProcessingType
    {
        eIN_PLACE_PROCESSING,
        eIN_OUT_PROCESSING
    };

    enum ImageFormat
    {
        eIMG_FMT_UNDEFINED = -1,
        eIMG_FMT_DEFAULT = 0,                       /* with the same format of source image */
        eIMG_FMT_YUY2 = eImgFmt_YUY2,               /*!< 422 format, 1 plane (YUYV) */
        eIMG_FMT_NV16 = eImgFmt_NV16,               /*!< 422 format, 2 plane (Y),(UV) */
        eIMG_FMT_NV21 = eImgFmt_NV21,               /*!< 420 format, 2 plane (Y),(VU) */
        eIMG_FMT_YV12 = eImgFmt_YV12,               /*!< 420 format, 3 plane (Y),(V),(U) */
        eIMG_FMT_YV16 = eImgFmt_YV16,               /*!< 422 format, 3 plane (Y),(V),(U) */
        eIMG_FMT_I420 = eImgFmt_I420,               /*!< 420 format, 3 plane (Y),(U),(V) */
        eIMG_FMT_I422 = eImgFmt_I422,               /*!< 422 format, 3 plane (Y),(U),(V) */

        eIMG_FMT_BAYER8 = eImgFmt_BAYER8,           /*!< Bayer format, 8-bit */
        eIMG_FMT_BAYER10 = eImgFmt_BAYER10,         /*!< Bayer format, 10-bit */
        eIMG_FMT_BAYER12 = eImgFmt_BAYER12,         /*!< Bayer format, 12-bit */
        eIMG_FMT_BAYER14 = eImgFmt_BAYER14,         /*!< Bayer format, 14-bit */
        eIMG_FMT_BAYER10_UNPAK = eImgFmt_BAYER10_UNPAK,   /*!< Bayer unpack format, 10-bit */
        eIMG_FMT_FG_BAYER8 = eImgFmt_FG_BAYER8,     /*!< Full-G (8-bit) */
        eIMG_FMT_FG_BAYER10 = eImgFmt_FG_BAYER10,   /*!< Full-G (10-bit) */
        eIMG_FMT_FG_BAYER12 = eImgFmt_FG_BAYER12,   /*!< Full-G (12-bit) */
        eIMG_FMT_FG_BAYER14 = eImgFmt_FG_BAYER14    /*!< Full-G (14-bit) */
    };

    struct ProcessingProfile
    {
        ProcessingType processingType;
        ImageFormat inImageFormat;
        ImageFormat outImageFormat;

        ProcessingProfile(
                ProcessingType type,
                ImageFormat inFomat,
                ImageFormat outFomat = eIMG_FMT_UNDEFINED)
                : processingType(type)
                , inImageFormat(inFomat)
                , outImageFormat(outFomat)
        {
            ASSERT_TRUE(processingType != eIN_PLACE_PROCESSING || inFomat != eIMG_FMT_UNDEFINED,
                        "should define the format of input image while in-place processing");
            ASSERT_TRUE(processingType != eIN_PLACE_PROCESSING || outFomat == eIMG_FMT_UNDEFINED,
                        "should not define the format of output image while in-place processing");
            ASSERT_TRUE(processingType != eIN_OUT_PROCESSING || inFomat != eIMG_FMT_UNDEFINED,
                        "should define the format of input image while in-out processing");
            ASSERT_TRUE(processingType != eIN_OUT_PROCESSING || outFomat != eIMG_FMT_UNDEFINED,
                        "should define the format of output image while in-out processing");
        }
    };

    struct PreConditionParams
    {
        MBOOL bResized;
        IMetadata *pInAppMeta;
        IMetadata *pInHalMeta;

        PreConditionParams()
                :bResized(MTRUE)
                ,pInAppMeta(NULL)
                , pInHalMeta(NULL)
        { }
    };

    struct ProcessingParams
    {
        IMetadata *pInAppMeta;
        IMetadata *pInHalMeta;
        NSCam::IImageBuffer *pInBuffer;
        // Should be null if processing type is eIN_PLACE_PROCESSING
        NSCam::IImageBuffer *pOutBuffer;
        MINT32 iOpenId;

        ProcessingParams()
                : pInAppMeta(NULL)
                , pInHalMeta(NULL)
                , pInBuffer(NULL)
                , pOutBuffer(NULL)
                , iOpenId(-1)
        { }
    };

    PostProcessing(MUINT32 const openId)
            : muOpenId(openId) { };

    virtual ~PostProcessing() { };

    virtual ProcessingProfile& profile() = 0;

    virtual MBOOL doOrNot(PreConditionParams const& param) = 0;

    virtual MBOOL process(ProcessingParams const& param) = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Procedure Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MUINT32 const muOpenId;

};

}
}

#endif