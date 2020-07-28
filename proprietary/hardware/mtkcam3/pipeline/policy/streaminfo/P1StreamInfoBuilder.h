/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM3_PIPELINE_POLICY_STREAMINFO_P1STREAMINFOBUILDER_H_
#define _MTK_HARDWARE_MTKCAM3_PIPELINE_POLICY_STREAMINFO_P1STREAMINFOBUILDER_H_
//
#include <mtkcam3/pipeline/policy/types.h>
#include <mtkcam3/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline::policy {


/**
 * pass1 meta stream info builder.
 */
class P1MetaStreamInfoBuilder : public NSCam::v3::Utils::MetaStreamInfoBuilder
{
protected:
    size_t          mIndex = 0;

public:
                    P1MetaStreamInfoBuilder(
                        size_t index
                    )
                        : MetaStreamInfoBuilder()
                        , mIndex(index)
                    {
                    }

public:
    virtual auto    setP1AppMetaDynamic_Default() -> P1MetaStreamInfoBuilder&;
    virtual auto    setP1HalMetaDynamic_Default() -> P1MetaStreamInfoBuilder&;
    virtual auto    setP1HalMetaControl_Default() -> P1MetaStreamInfoBuilder&;
};


/**
 * pass1 image stream info builder.
 */
class P1ImageStreamInfoBuilder : public NSCam::v3::Utils::ImageStreamInfoBuilder
{
protected:
    static const bool                   kIsLowRam;
    size_t                              mIndex = 0;
    std::shared_ptr<IHwInfoHelper>      mHwInfoHelper = nullptr;
    CaptureFeatureSetting const*        mpCaptureFeatureSetting = nullptr;
    StreamingFeatureSetting const*      mpStreamingFeatureSetting = nullptr;
    PipelineStaticInfo const*           mpPipelineStaticInfo = nullptr;
    PipelineUserConfiguration const*    mpPipelineUserConfiguration = nullptr;

public:
                    struct CtorParams
                    {
                        size_t                          index = 0;
                        std::shared_ptr<IHwInfoHelper>  pHwInfoHelper = nullptr;
                        CaptureFeatureSetting const*    pCaptureFeatureSetting = nullptr;
                        StreamingFeatureSetting const*  pStreamingFeatureSetting = nullptr;
                        PipelineStaticInfo const*       pPipelineStaticInfo = nullptr;
                        PipelineUserConfiguration const*pPipelineUserConfiguration = nullptr;
                    };
                    P1ImageStreamInfoBuilder(CtorParams const& arg)
                        : ImageStreamInfoBuilder()
                        , mIndex(arg.index)
                        , mHwInfoHelper(arg.pHwInfoHelper)
                        , mpCaptureFeatureSetting(arg.pCaptureFeatureSetting)
                        , mpStreamingFeatureSetting(arg.pStreamingFeatureSetting)
                        , mpPipelineStaticInfo(arg.pPipelineStaticInfo)
                        , mpPipelineUserConfiguration(arg.pPipelineUserConfiguration)
                    {
                    }

public:

    static  auto    queryMinBufNum(char const* name) -> int32_t;
    static  auto    queryBaseBufNum(char const* name) -> int32_t;

public:

    virtual auto    setP1Imgo_Default(
                        P1HwSetting const& rP1HwSetting
                    ) -> P1ImageStreamInfoBuilder&;

    virtual auto    setP1Imgo_Default(
                        size_t maxBufNum,
                        P1HwSetting const& rP1HwSetting
                    ) -> P1ImageStreamInfoBuilder&;

    virtual auto    setP1Rrzo_Default(
                        P1HwSetting const& rP1HwSetting
                    ) -> P1ImageStreamInfoBuilder&;

    virtual auto    setP1Rrzo_Default(
                        size_t maxBufNum,
                        P1HwSetting const& rP1HwSetting
                    ) -> P1ImageStreamInfoBuilder&;

    virtual auto    setP1Rsso_Default(
                        P1HwSetting const& rP1HwSetting
                    ) -> P1ImageStreamInfoBuilder&;

    virtual auto    setP1Rsso_Default(
                        size_t maxBufNum,
                        P1HwSetting const& rP1HwSetting
                    ) -> P1ImageStreamInfoBuilder&;

    virtual auto    setP1FDYuv_Default(
                        P1HwSetting const& rP1HwSetting
                    ) -> P1ImageStreamInfoBuilder&;

    virtual auto    setP1ScaledYuv_Default(
                        P1HwSetting const& rP1HwSetting
                    ) -> P1ImageStreamInfoBuilder&;

protected:
    virtual auto    toBufPlanes(
                        IImageStreamInfo::BufPlanes_t& bufPlanes,
                        size_t stride,
                        MINT imgFormat,
                        MSize const& imgSize
                    ) -> bool;

};


/**
 * pass1 image stream info builder.
 */
class P1STTImageStreamInfoBuilder : public P1ImageStreamInfoBuilder
{
public:
                    P1STTImageStreamInfoBuilder(CtorParams const& arg)
                        : P1ImageStreamInfoBuilder(arg)
                    {
                    }

public:
    virtual auto    setMaxBufNum_Default() -> void;
    virtual auto    setP1Stt_Default(MINT32 sensorId) -> void;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3::pipeline::policy
#endif  //_MTK_HARDWARE_MTKCAM3_PIPELINE_POLICY_STREAMINFO_P1STREAMINFOBUILDER_H_

