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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_PIPELINEMODELIMPL_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_PIPELINEMODELIMPL_H_
//
#include <mtkcam3/main/standalone_isp/IISPPipelineModel.h>
#include <mtkcam3/pipeline/pipeline/IPipelineBufferSetFrameControl.h>
#include <mtkcam3/pipeline/pipeline/IPipelineContext.h>
#include <mtkcam3/main/standalone_isp/IFeatureSettingPolicy_ISP.h>

#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <string>

#include <utils/Printer.h>
//

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace isppipeline {
namespace model {

using namespace NSCam::v3::pipeline::policy::featuresetting_isp;
/******************************************************************************
 *
 ******************************************************************************/
class PipelineModelISP
    : public IISPPipelineModel
    , public NSCam::v3::pipeline::IPipelineBufferSetFrameControl::IAppCallback
    , public NSCam::v3::pipeline::NSPipelineContext::DataCallbackBase
{
public:
    using IPipelineContext = NSCam::v3::pipeline::NSPipelineContext::IPipelineContext;
    using HalMetaStreamBuffer = NSCam::v3::Utils::HalMetaStreamBuffer;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Instantiation data (initialized at the creation stage).
    std::shared_ptr<android::Printer>       mErrorPrinter;
    //
    int32_t                                 mLogLevel = 0;

protected:  ////    Open data (initialized at the open stage).
    android::wp<IISPPipelineModelCallback>     mCallback;
    std::string                             mModelName;
    int32_t                                 mModel;
    int32_t                                 mCamId = -1;
    std::vector<int32_t>                    mSensorIds;

protected:  ////    Configuration data and members
    std::timed_mutex                        mLock;
    mutable android::RWLock         mRWLock_PipelineContext;
    android::sp<IPipelineContext>  mCurrentPipelineContext;
    std::shared_ptr<IFeatureSettingPolicy_ISP> mpFeaturePolicy = nullptr;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.

                    /**
                     * A structure for creation parameters.
                     */
                    struct  ModelCreationParams
                    {
                        /**
                         * @param printers with several levels for debugging.
                         */
                        std::shared_ptr<android::Printer>   errorPrinter;
                        int32_t                             id;
                        int32_t                             camId;
                        std::vector<int32_t>                sensorIds;
                    };

                    PipelineModelISP(ModelCreationParams const& creationParams);

public:
    // build pipelinecontext
    struct ConfigureInfo
    {
        android::sp<IMetaStreamInfo>                pAppMeta_Control = nullptr; // p2c/jpeg/fd input app meta, camera app control
        android::sp<IMetaStreamInfo>                pHalMeta_P2CIn = nullptr;   // P2c input hal meta, P1 of camera process produce
        android::sp<IMetaStreamInfo>                pAppMeta_P1Result = nullptr;// P2c input app meta, P1 of camera process produce
        android::sp<IMetaStreamInfo>                pAppMeta_P2COut = nullptr;  // p2c app output
        android::sp<IMetaStreamInfo>                pHalMeta_P2COut = nullptr;  // p2c hal output, jpeg input

        android::sp<IMetaStreamInfo>                pAppMeta_FDOut = nullptr;  // FD meta out
        android::sp<IMetaStreamInfo>                pAppMeta_JpegOut = nullptr; // jpeg meta out
        android::sp<IMetaStreamInfo>                pAppMeta_P2PackOut = nullptr; // pack meta out

        /******************************************
        *  image stream info
        ******************************************/
        android::sp<IImageStreamInfo>               pHalImage_Jpeg_YUV = nullptr;
        android::sp<IImageStreamInfo>               pHalImage_Thumbnail_YUV = nullptr;
        android::sp<IImageStreamInfo>               pHalImage_Jpeg_Sub_YUV = nullptr; // dual camera feature used
        android::sp<IImageStreamInfo>               pHalImage_Depth_YUV = nullptr; // dual camera feature used

        android::sp<IImageStreamInfo>               pISPImage_FD_YUV_In = nullptr;
        android::sp<IImageStreamInfo>               pISPImage_Imgo_Raw_In = nullptr; // raw16 format, p2c input image data
        android::sp<IImageStreamInfo>               pISPImage_Resized_In = nullptr; // p2c input image data
        android::sp<IImageStreamInfo>               pISPImage_Lcso_In = nullptr; // p2c input image data
        android::sp<IImageStreamInfo>               pISPImage_Imgo_Yuv_In = nullptr; // YV12/NV21 formant, p2c input image data
        // for output
        std::unordered_map<StreamId_T, android::sp<IImageStreamInfo>> pvISPImage_YuvOut;
        android::sp<IImageStreamInfo>               pISPImage_JpegOut = nullptr;
        std::unordered_map<StreamId_T, android::sp<IImageStreamInfo>> pvISPImage_RawOut; // unpack raw out, p2c output
        android::sp<IImageStreamInfo>               pISPImage_TuningOut = nullptr; // blob, for output tuning data

        MSize                                       maxImageSize;

        MBOOL                                       needP2C = false;
        MBOOL                                       needFD = false;
        MBOOL                                       needJpeg = false;
        MBOOL                                       needISPPack = false;
        MBOOL                                       AppYuvDirectJpeg = false;
        MBOOL                                       WithoutThumbnail = false;
        MBOOL                                       OnlyUseJpegNode = false;

        MINT32                                      rootNode = -1;
        ConfigurationOutputParams                   featureInfo;

    };
    struct ConfigureInfo                            mConfigureInfo;

public:     ////    Operations.

    /**
     * Dump debugging state.
     */
    virtual auto    dumpState(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                        ) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModel Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    virtual auto    open(
                        std::string const& Name,
                        android::wp<IISPPipelineModelCallback> const& callback
                    ) -> int override;
    virtual auto    close() -> void override;
    virtual auto    configure(
                        std::shared_ptr<UserConfigurationParams>const& params
                    ) -> int override;
    virtual auto    submitRequest(
                        std::vector<std::shared_ptr<UserRequestParams>>const& requests,
                        uint32_t& numRequestProcessed
                    ) -> int override;
    virtual auto    beginFlush() -> int override;
    virtual auto    endFlush() -> void override;
    virtual auto    queryFeature(IMetadata const* pInMeta, std::vector<std::shared_ptr<IMetadata>> &outSetting) -> void override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual MVOID   updateFrame(
                        MUINT32 const requestNo,
                        MINTPTR const userId,
                        Result const& result
                    ) override;
    virtual MVOID   onMetaCallback(
                        MUINT32              requestNo __unused,
                        Pipeline_NodeId_T    nodeId __unused,
                        StreamId_T           streamId __unused,
                        IMetadata const&     rMetaData __unused,
                        MBOOL                errorResult __unused
                    ) override;
protected:
    virtual auto
            parseConfigureParams(
                std::shared_ptr<UserConfigurationParams>const& params
            ) -> int;

    virtual auto
            buildISPStreams(
                std::shared_ptr<UserConfigurationParams>const& params,
                android::sp<IPipelineContext> pContext
            ) -> int;

    virtual auto
            buildISPNodes(
                android::sp<IPipelineContext> pContext
            ) -> int;

    virtual auto
            buildISPPipeline(
                android::sp<IPipelineContext> pContext
            ) -> int;
    virtual auto
            buildISPPipelineFrame(
                std::shared_ptr<UserRequestParams>const& request,
                android::sp<IPipelineContext> pContext,
                android::sp<IPipelineFrame>& pFrame,
                int32_t frameCount,
                int32_t frameIndex
            ) -> int;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_PIPELINEMODELIMPL_H_

