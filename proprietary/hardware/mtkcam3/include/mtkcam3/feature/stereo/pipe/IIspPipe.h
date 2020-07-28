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

#ifndef _MTK_CAMERA_FEATURE_PIPE_INTERFACE_THIRD_PARTY_ISP_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_INTERFACE_THIRD_PARTY_ISP_PIPE_H_

#include <utils/RefBase.h>
#include <mtkcam3/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/def/common.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

/*******************************************************************************
* Enum Definition
********************************************************************************/
typedef MINT32 IspPipeBufferID;
/**
 * @brief buffer id inside the Zoom ISP Pipe
 */
typedef enum ISPBufferDataTypes_Public{
    PUBLIC_PBID_START = 0,
    //
    PBID_IN_FULL_RAW_MAIN1,
    PBID_IN_FULL_RAW_MAIN2,
    PBID_IN_RESIZE_RAW_MAIN1,
    PBID_IN_RESIZE_RAW_MAIN2,
    PBID_IN_LCSO_MAIN1,
    PBID_IN_LCSO_MAIN2,
    PBID_IN_APP_META,
    PBID_IN_HAL_META_MAIN1,
    PBID_IN_HAL_META_MAIN2,
    PBID_IN_RESIZE_YUV_MAIN1, //10
    PBID_IN_RESIZE_YUV_MAIN2,
    PBID_IN_FULLSIZE_YUV_MAIN1,
    PBID_IN_FULLSIZE_YUV_MAIN2,
    PBID_IN_BINING_YUV_MAIN1,
    PBID_IN_BINING_YUV_MAIN2,
    // output meta
    PBID_OUT_APP_META,
    PBID_OUT_HAL_META,
    //
    PBID_OUT_PV_YUV0,
    PBID_OUT_PV_YUV1,
    PBID_OUT_PV_FD,
    PBID_OUT_BOKEH_YUV,
    PBID_OUT_BOKEH_THUMBNAIL_YUV,
    PBID_OUT_THIRDPARTY_DEPTH,
    PBID_OUT_CLEAN_YUV,
    // end
    PUBLIC_PBID_END = 100
} PublicIspPipeBufferID;
/**
  * @brief type for IspPipe flow control
 */
enum class IspPipeFlowControlType
{
    Default,
    DCMF        // Mtlti-Frame Third Party
};
/*******************************************************************************
* Struct Definition
********************************************************************************/
/**
 * @brief IspPipe functional option.
 */

struct IspPipeOption : public android::LightRefBase<IspPipeOption>
{
    IspPipeOption() {}
    IspPipeOption(const IspPipeOption& option)
    : mbEnableEIS1d2(option.mbEnableEIS1d2)
    , mbEnableLCE(option.mbEnableLCE) {}
    //
    MBOOL mbEnableEIS1d2 = MFALSE; // enable EIS 1.2 or not
    MBOOL mbEnableLCE = MFALSE;
};

/**
 * @brief IspPipe config setting
 */

struct IspPipeSetting : public android::LightRefBase<IspPipeSetting>
{
public:
    IspPipeSetting() {}
    IspPipeSetting(const IspPipeSetting& setting)
    : IspPipeSetting(setting.miSensorIdx_Main1, setting.miSensorIdx_Main2,
                     setting.mszIMGO_Main1, setting.mszIMGO_Main2,
                     setting.mszRRZO_Main1, setting.mszRRZO_Main2,
                     setting.mszPreviewYUV) {}
    IspPipeSetting(
        MUINT32 senIdx_main1,
        MUINT32 senIdx_main2,
        MSize szIMGO_Main1,
        MSize szIMGO_Main2,
        MSize szRRZO_Main1,
        MSize szRRZO_Main2,
        MSize szPreviewSize
        );

public:
    MUINT32 miSensorIdx_Main1 = -1;
    MUINT32 miSensorIdx_Main2 = -1;
    MSize mszIMGO_Main1;
    MSize mszIMGO_Main2;
    MSize mszRRZO_Main1;
    MSize mszRRZO_Main2;
    MSize mszPreviewYUV;
    MSize mszFDImg;
};

class IIspPipeRequest;
/******************************************************************************
 * The Interface of DualCam Zoom ISP Pipe
 ******************************************************************************/
class IIspPipe
{
public:
    /**
     * @brief Zoom ISPPipe creation
     * @param [in] setting pipe setting
     * @param [in] option pipe option
     * @param [in] flow control type
     * @return
     * - MTRUE indicates success
     * - MFALSE indicates failure
     */
    static IIspPipe* createInstance(
                            IspPipeSetting& setting,
                            IspPipeOption& option,
                            IspPipeFlowControlType type = IspPipeFlowControlType::Default);

    MBOOL destroyInstance();
    // destructor
    virtual ~IIspPipe() {};
public:
    /**
     * @brief perform initial operation
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL init() = 0;
    /**
     * @brief perform uninitial operation
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL uninit() = 0;
    /**
     * @brief perform enque operation
     * @param [in] request to-be-performed EffectRequest
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL enque(android::sp<IIspPipeRequest>& request) = 0;
    /**
     * @brief Flush all the operations inside the feature pipe
     */
    virtual MVOID flush() = 0;
    /**
     * @brief Sync all the operations inside the feature pipe
     */
    virtual MVOID sync() = 0;
};

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif