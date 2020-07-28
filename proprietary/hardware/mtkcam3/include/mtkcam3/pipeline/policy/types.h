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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_TYPES_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_TYPES_H_
//
#include <mtkcam3/pipeline/stream/IStreamBuffer.h>
#include <mtkcam3/pipeline/pipeline/IPipelineContext.h>
#include <mtkcam/def/common.h>  // for mtkcam3/feature/eis/EisInfo.h
#include <mtkcam/drv/def/ICam_type.h> // for isp quality enum
#include <mtkcam3/feature/eis/EisInfo.h>
#include <mtkcam/def/ImageBufferInfo.h>

//
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {

////////////////////////////////////////////////////////////////////////////////

/**
 *  Pipeline static information.
 *
 *  The following information is static and unchanged forever, regardless of
 *  any operation (e.g. open or configure).
 */
struct PipelineStaticInfo
{
    /**
     *  Logical device open id
     */
    int32_t                                     openId = -1;

    /**
     *  Physical sensor id (0, 1, 2)
     */
    std::vector<int32_t>                        sensorId;

    int32_t getIndexFromSensorId(int Id) const
    {
        for (int32_t i = 0; i < (int32_t)sensorId.size(); i++)
        {
            if(Id == sensorId[i])
                return i;
        }
        return -1;
    };

    /**
     *  Sensor raw type.
     *
     *  SENSOR_RAW_xxx in mtkcam/include/mtkcam/drv/IHalSensor.h
     */
    std::vector<uint32_t>                       sensorRawType;

    /**
     *
     */
    bool                                        isDualDevice = false;

    /**
     *  Type3 PD sensor without PD hardware (ISP3.0)
     */
    bool                                        isType3PDSensorWithoutPDE = false;

    /**
     *  is 4-Cell sensor
     */
    bool                                        is4CellSensor = false;

    /**
     *  is VHDR sensor
     */
    bool                                        isVhdrSensor = false;
    /**
     *  is P1 direct output FD YUV (ISP6.0)
     */
    bool                                        isP1DirectFDYUV = false;

    /**
     *  support AIAWB
     */
    bool                                        isAIAWBSupported = false;

    /**
     *  Must use P2 output process raw
     */
    bool                                        isNeedP2ProcessRaw = false;

    /**
     *  P1 direct output rectify YUV (ISP6.0)
     */
    bool                                        isP1DirectScaledYUV = false;
    /**
     *  is support burst capture or not
     */
    bool                                        isSupportBurstCap = false;

    /**
     *  isp tuning data stream size for raw
     */
    MSize                                       rawIspTuningDataStreamSize;

    /**
     *  isp tuning data stream size for yuv
     */
    MSize                                       yuvIspTuningDataStreamSize;

};


struct ParsedAppConfiguration;
struct ParsedAppImageStreamInfo;
struct ParsedMultiCamInfo;
struct ParsedSMVRBatchInfo;

/**
 *  Pipeline user configuration
 *
 *  The following information is given and set up at the configuration stage, and
 *  is never changed AFTER the configuration stage.
 */
struct PipelineUserConfiguration
{
    std::shared_ptr<ParsedAppConfiguration>     pParsedAppConfiguration;

    /**
     * Parsed App image stream info set
     *
     * It results from the raw data, i.e. vImageStreams.
     */
    std::shared_ptr<ParsedAppImageStreamInfo>   pParsedAppImageStreamInfo;

    /**************************************************************************
     * App image stream info set (raw data)
     **************************************************************************/

    /**
     * App image streams to configure.
     */
    std::unordered_map<StreamId_T, android::sp<IImageStreamInfo>>
                                                vImageStreams;

    /**
     * App meta streams to configure.
     */
    std::unordered_map<StreamId_T, android::sp<IMetaStreamInfo>>
                                                vMetaStreams;

    /**
     * App image streams min frame duration to configure.
     */
    std::unordered_map<StreamId_T, int64_t>     vMinFrameDuration;

    /**
     * App image streams stall frame duration to configure.
     */
    std::unordered_map<StreamId_T, int64_t>     vStallFrameDuration;

   /**
     * @param[in] physical camera id list
     */
    std::vector<int32_t>                        vPhysicCameras;

};

////////////////////////////////////////////////////////////////////////////////


/**
 * P1 DMA bitmask definitions
 *
 * Used in the following structures:
 *      IPipelineSettingPolicy.h: RequestResultParams::needP1Dma
 *      IIOMapPolicy.h: RequestInputParams::pRequest_NeedP1Dma
 *      IStreamInfoConfigurationPolicy.h: FunctionType_StreamInfoConfiguration_P1
 *
 */
enum : uint32_t
{
    P1_IMGO     = (0x01U << 0),
    P1_RRZO     = (0x01U << 1),
    P1_LCSO     = (0x01U << 2),
    P1_RSSO     = (0x01U << 3),
    // for ISP 60
    P1_FDYUV    = (0x01U << 4),
    P1_FULLYUV  = (0x01U << 5),
    P1_SCALEDYUV = (0x01U << 6),
    //
    P1_MASK     = 0x0F,
    P1_ISP6_0_MASK = 0xFF,
};

/**
 * P2 node bitmask definitions
 *
 * Used in the following structures:
 *      IIOMapPolicy.h: RequestInputParams::pRequest_NeedP1Dma
 *
 */
enum : uint32_t
{
    P2_STREAMING     = (0x01U << 0),
    P2_CAPTURE       = (0x01U << 1),
};


/**
 * Reconfig category enum definitions
 * For pipelineModelSession processReconfiguration use
 */
enum class ReCfgCtg: uint8_t
{
    NO          = 0,
    STREAMING,
    CAPTURE,
    NUM
};

/**
 *  Sensor Setting
 */
struct SensorSetting
{
    uint32_t                                    sensorMode = 0;
    uint32_t                                    sensorFps = 0;
    MSize                                       sensorSize;
};


static inline android::String8 toString(const SensorSetting& o __unused)
{
    android::String8 os = android::String8::format("{ .sensorMode=%d .sensorFps=%d .sensorSize=%dx%d }", o.sensorMode, o.sensorFps, o.sensorSize.w, o.sensorSize.h);
    return os;
};


/**
 *  DMA settings
 */
struct DmaSetting
{
    /**
     * Image format.
     */
    int32_t                                     format = 0;

    /**
     * Image resolution in pixel.
     */
    MSize                                       imageSize;

};


static inline android::String8 toString(const DmaSetting& o __unused)
{
    android::String8 os;
    os += "{";
    os += android::String8::format(" format:%#x", o.format);
    os += android::String8::format(" %dx%d", o.imageSize.w, o.imageSize.h);
    os += " }";
    return os;
};


/**
 *  Pass1-specific HW settings
 */
struct P1HwSetting
{
    /**
     * @param imgoAlloc
     *        It is used for allcating buffer. If we need to runtime change
     *        this dma setting, a worst setting (e.g. eImgFmt_BAYER10_UNPAK)
     *        should be indicated in allocating buffer stage.
     *
     * @param imgoDefaultRequest
     *        It is used for default (streaming) request.
     *        It is used for configuring P1Node.
     */
    DmaSetting                                  imgoAlloc;
    DmaSetting                                  imgoDefaultRequest;

    /**
     * @param rrzoDefaultRequest
     *        It is used for default (streaming) request.
     */
    DmaSetting                                  rrzoDefaultRequest;

    MSize                                       rssoSize;

    uint32_t                                    pixelMode = 0;
    bool                                        usingCamSV = false;
    // ISP6.0
    MSize                                       fdyuvSize;
    MSize                                       scaledYuvSize;
    bool                                        canSupportScaledYuv = false;

    // define in def/ICam_type.h
    E_CamIQLevel                                ispQuality = eCamIQ_MAX;
    E_INPUT                                     camTg = TG_CAM_MAX;
    CAM_RESCONFIG                               camResConfig;
};


static inline android::String8 toString(const P1HwSetting& o __unused)
{
    android::String8 os;
    os += "{";
    os += " .imgoAlloc=";
    os += toString(o.imgoAlloc);
    os += " .imgoDefaultRequest=";
    os += toString(o.imgoDefaultRequest);
    os += " .rrzoDefaultRequest=";
    os += toString(o.rrzoDefaultRequest);
    os += android::String8::format(" .rssoSize=%dx%d", o.rssoSize.w, o.rssoSize.h);
    os += android::String8::format(" .pixelMode=%u .usingCamSV=%d", o.pixelMode, o.usingCamSV);
    os += " }";
    return os;
};


/**
 *  Parsed metadata control request
 */
struct ParsedMetaControl
{
    bool                                        repeating = false;

    int32_t                                     control_aeTargetFpsRange[2]     = {0};//CONTROL_AE_TARGET_FPS_RANGE
    uint8_t                                     control_captureIntent           = static_cast< uint8_t>(-1L);//CONTROL_CAPTURE_INTENT
    uint8_t                                     control_enableZsl               = static_cast< uint8_t>(0);//CONTROL_ENABLE_ZSL
    uint8_t                                     control_mode                    = static_cast< uint8_t>(-1L);//CONTROL_MODE
    uint8_t                                     control_sceneMode               = static_cast< uint8_t>(-1L);//CONTROL_SCENE_MODE
    uint8_t                                     control_videoStabilizationMode  = static_cast< uint8_t>(-1L);//CONTROL_VIDEO_STABILIZATION_MODE
    uint8_t                                     edge_mode                       = static_cast< uint8_t>(-1L);//EDGE_MODE
    uint8_t                                     control_isp_tuning              = 0;//for mtk isp tuning
    MINT32                                      control_processRaw              = 0;
    MINT32                                      control_remosaicEn              = 0;//MTK_CONTROL_CAPTURE_REMOSAIC_EN
    MINT32                                      control_vsdofFeatureWarning     = -1;//MTK_VSDOF_FEATURE_CAPTURE_WARNING_MSG

};


static inline android::String8 toString(const ParsedMetaControl& o __unused)
{
    android::String8 os;
    os += "{";
    os += android::String8::format(" repeating:%d", o.repeating);
    os += android::String8::format(" control.aeTargetFpsRange:%d,%d", o.control_aeTargetFpsRange[0], o.control_aeTargetFpsRange[1]);
    os += android::String8::format(" control.captureIntent:%d", o.control_captureIntent);
    os += android::String8::format(" control.enableZsl:%d", o.control_enableZsl);
    os += android::String8::format(" control.processRawEn:%d", o.control_processRaw);
    if ( static_cast< uint8_t>(-1L) != o.control_mode ) {
        os += android::String8::format(" control.mode:%d", o.control_mode);
    }
    if ( static_cast< uint8_t>(-1L) != o.control_sceneMode ) {
        os += android::String8::format(" control.sceneMode:%d", o.control_sceneMode);
    }
    if ( static_cast< uint8_t>(-1L) != o.control_videoStabilizationMode ) {
        os += android::String8::format(" control.videoStabilizationMode:%d", o.control_videoStabilizationMode);
    }
    if ( static_cast< uint8_t>(-1L) != o.edge_mode ) {
        os += android::String8::format(" edge.mode:%d", o.edge_mode);
    }
    os += " }";
    return os;
};


/**
 *  Parsed App configuration
 */
struct ParsedAppConfiguration
{
    /**
     * The operation mode of pipeline.
     * The caller must promise its value.
     */
    uint32_t                                    operationMode = 0;

    /**
     * Session wide camera parameters.
     *
     * The session parameters contain the initial values of any request keys that were
     * made available via ANDROID_REQUEST_AVAILABLE_SESSION_KEYS. The Hal implementation
     * can advertise any settings that can potentially introduce unexpected delays when
     * their value changes during active process requests. Typical examples are
     * parameters that trigger time-consuming HW re-configurations or internal camera
     * pipeline updates. The field is optional, clients can choose to ignore it and avoid
     * including any initial settings. If parameters are present, then hal must examine
     * their values and configure the internal camera pipeline accordingly.
     */
    IMetadata                                   sessionParams;

    /**
     * operationMode = 1
     *
     * StreamConfigurationMode::CONSTRAINED_HIGH_SPEED_MODE = 1
     * Refer to https://developer.android.com/reference/android/hardware/camera2/params/SessionConfiguration#SESSION_HIGH_SPEED
     */
    bool                                        isConstrainedHighSpeedMode = false;

    /**
     * operationMode: "persist.vendor.mtkcam.operationMode.superNightMode"
     *
     * Super night mode.
     */
    bool                                        isSuperNightMode = false;

    /**
     * Check to see whether or not the camera is being used by a proprietary client.
     *
     * true: proprietary.
     * false: unknown. It could be a proprietary client or a 3rd-party client.
     */
    bool                                        isProprietaryClient = false;

    /**
     * initRequest: parsing sessionparam to get init request is enable or not
     *
     * if enable, default set 4 to improve start preview time
     */
    int                                         initRequest = 0;

    /**
     * Dual cam related info.
     */
    std::shared_ptr<ParsedMultiCamInfo>          pParsedMultiCamInfo;

    /**
     * SMVRBatch related info.
     */
    std::shared_ptr<ParsedSMVRBatchInfo>        pParsedSMVRBatchInfo;

    int                                         useP1DirectFDYUV = false;

    /**
     *  Type3 PD sensor without PD hardware (ISP3.0)
     */
    bool                                        isType3PDSensorWithoutPDE = false;

    bool                                        useP1DirectAppRaw = false;

    bool                                        hasTuningEnable = false;

    uint64_t                                    configTimestamp = 0;
};


/**
 *  Parsed App image stream info
 */
struct ParsedAppImageStreamInfo
{
    /**************************************************************************
     *  App image stream info set
     **************************************************************************/

    /**
     * Output streams for any processed (but not-stalling) formats
     *
     * Reference:
     * https://developer.android.com/reference/android/hardware/camera2/CameraCharacteristics.html#REQUEST_MAX_NUM_OUTPUT_PROC
     */
    std::unordered_map<StreamId_T, android::sp<IImageStreamInfo>>
                                                vAppImage_Output_Proc;

    /**
     * Input stream for yuv reprocessing
     */
    android::sp<IImageStreamInfo>               pAppImage_Input_Yuv;

    /**
     * Output stream for private reprocessing
     */
    android::sp<IImageStreamInfo>               pAppImage_Output_Priv;

    /**
     * Input stream for private reprocessing
     */
    android::sp<IImageStreamInfo>               pAppImage_Input_Priv;

    /**
     * Output stream for RAW16/DNG capture.
     * map : <sensorId, sp<streamInfo>>
     */
    std::unordered_map<int, android::sp<IImageStreamInfo>>
                                                vAppImage_Output_RAW16;

    /**
     * Input stream for RAW16 reprocessing.
     */
    android::sp<IImageStreamInfo>               pAppImage_Input_RAW16;

    /**
     * Output stream for JPEG capture.
     */
    android::sp<IImageStreamInfo>               pAppImage_Jpeg;

    /**
     * Output stream for physical yuv stream
     * map : <sensorId, sp<streamInfo>>
     */
    std::unordered_map<uint32_t, std::vector<android::sp<IImageStreamInfo>>>
                                                vAppImage_Output_Proc_Physical;

    /**
     * Output stream for physical RAW16/DNG capture.
     * map : <sensorId, sp<streamInfo>>
     */
    std::unordered_map<uint32_t, std::vector<android::sp<IImageStreamInfo>>>
                                                vAppImage_Output_RAW16_Physical;

    /**
     * Output stream for isp tuning data stream (RAW)
     */
    android::sp<IImageStreamInfo>               pAppImage_Output_IspTuningData_Raw;

    /**
     * Output stream for isp tuning data stream (YUV)
     */
    android::sp<IImageStreamInfo>               pAppImage_Output_IspTuningData_Yuv;

    /**
     * Output stream for physical isp meta stream (Raw)
     * map : <sensorId, sp<streamInfo>>
     */
    std::unordered_map<uint32_t, android::sp<IImageStreamInfo>>
                                                vAppImage_Output_IspTuningData_Raw_Physical;

    /**
     * Output stream for physical isp meta stream (Yuv)
     * map : <sensorId, sp<streamInfo>>
     */
    std::unordered_map<uint32_t, android::sp<IImageStreamInfo>>
                                                vAppImage_Output_IspTuningData_Yuv_Physical;

    /**************************************************************************
     *  Parsed info
     **************************************************************************/

    /**
     * One of consumer usages of App image streams contains BufferUsage::VIDEO_ENCODER.
     */
    bool                                        hasVideoConsumer = false;

    /**
     * 4K video recording
     */
    bool                                        hasVideo4K = false;

    /**
     * The image size of video recording, in pixels.
     */
    MSize                                       videoImageSize;

    /**
     * The image size of app yuv out, in pixels.
     */
    MSize                                       maxYuvSize;

    /**
     * The max. image size of App image streams, in pixels, regardless of stream formats.
     */
    MSize                                       maxImageSize;

    /**
     * Security Info
     */
    SecureInfo                                  secureInfo{.type=SecType::mem_normal,};

    /**
     * Store needs handling physical sensor id.
     */
    std::vector<uint32_t>                       outputPhysicalSensors;

    bool                                        hasRawOut = false;

};

enum class DualDevicePath{
    /* Using dual camera device, but it does not set feature mode in session parameter or
     * set camera id to specific stream.
     */
    Single,
    /* Using dual camera device, set physical camera id to specific stream.
     */
    MultiCamControl,
};

struct ParsedMultiCamInfo
{
    /**
     * Dual device pipeline path.
     */
    DualDevicePath                              mDualDevicePath = DualDevicePath::Single;

    /**
     * dual cam feature which get from session param.
     */
    int32_t                                     mDualFeatureMode = -1;

    /**
     * support dual cam pack image in hal.
     */
    int32_t                                     mStreamingFeatureMode = -1;

    /**
     * store capture feature mode. (like tk vsdof/ 3rd vsdof/ zoom,.etc)
     */
    int32_t                                     mCaptureFeatureMode = -1;


   /**
     * support dual cam pack image in hal.
     */
    bool                                        mSupportPackJpegImages = false;

    /**
     * support number of avtive camera in the same time.
     */
    uint32_t                                    mSupportPass1Number = 2;


    /**
     * support physical stream output
     */
    bool                                        mSupportPhysicalOutput = false;

};

struct ParsedSMVRBatchInfo
{
    // meta: MTK_SMVR_FEATURE_SMVR_MODE
    int32_t              maxFps = 0;    // = meta:idx=0
    int32_t              p2BatchNum = 1; // = min(meta:idx=1, p2IspBatchNum)
    int32_t              imgW = 0;       // = StreamConfiguration.streams[videoIdx].width
    int32_t              imgH = 0;       // = StreamConfiguration.streams[videoIdx].height
    int32_t              p1BatchNum = 1; // = maxFps / 30
    int32_t              opMode = 0;     // = StreamConfiguration.operationMode
    int32_t              logLevel = 0;   // from property
};

/**
 *  (Non Pass1-specific) Parsed stream info
 */
struct ParsedStreamInfo_NonP1
{
    /******************************************
     *  app meta stream info
     ******************************************/
    android::sp<IMetaStreamInfo>                pAppMeta_Control;
    android::sp<IMetaStreamInfo>                pAppMeta_DynamicP2StreamNode;
    // <sensor id, IMetaStreamInfo>
    std::unordered_map<uint32_t, android::sp<IMetaStreamInfo>>
                                                vAppMeta_DynamicP2StreamNode_Physical;
    android::sp<IMetaStreamInfo>                pAppMeta_DynamicP2CaptureNode;
    // <sensor id, IMetaStreamInfo>
    std::unordered_map<uint32_t, android::sp<IMetaStreamInfo>>
                                                vAppMeta_DynamicP2CaptureNode_Physical;
    android::sp<IMetaStreamInfo>                pAppMeta_DynamicFD;
    android::sp<IMetaStreamInfo>                pAppMeta_DynamicJpeg;
    android::sp<IMetaStreamInfo>                pAppMeta_DynamicRAW16;
    android::sp<IMetaStreamInfo>                pAppMeta_DynamicP1ISPPack;
    android::sp<IMetaStreamInfo>                pAppMeta_DynamicP2ISPPack;
    // <sensor id, IMetaStreamInfo>
    std::unordered_map<uint32_t, android::sp<IMetaStreamInfo>>
                                                vAppMeta_DynamicP1ISPPack_Physical;
    // <sensor id, IMetaStreamInfo>
    std::unordered_map<uint32_t, android::sp<IMetaStreamInfo>>
                                                vAppMeta_DynamicP2ISPPack_Physical;
    // <sensor id, IMetaStreamInfo>
    std::unordered_map<uint32_t, android::sp<IMetaStreamInfo>>
                                                vAppMeta_DynamicRAW16_Physical;


    /******************************************
     *  hal meta stream info
     ******************************************/
    android::sp<IMetaStreamInfo>                pHalMeta_DynamicP2StreamNode;
    android::sp<IMetaStreamInfo>                pHalMeta_DynamicP2CaptureNode;
    android::sp<IMetaStreamInfo>                pHalMeta_DynamicPDE;


    /******************************************
     *  hal image stream info
     ******************************************/

    /**
     *  Face detection.
     */
    android::sp<IImageStreamInfo>               pHalImage_FD_YUV = nullptr;

    /**
     *  The Jpeg orientation is passed to HAL at the request stage.
     *  Maybe we can create a stream set for every orientation at the configuration stage, but only
     *  one within that stream set can be passed to the configuration of pipeline context.
     */
    android::sp<IImageStreamInfo>               pHalImage_Jpeg_YUV;

    /**
     *  The thumbnail size is passed to HAL at the request stage.
     */
    android::sp<IImageStreamInfo>               pHalImage_Thumbnail_YUV;

    /**
     *  HAL-level Jpeg stream info.
     *
     *  HAL-level Jpeg is used for debug and tuning on YUV capture
     *  (while App-level Jpeg is not configured).
     */
    android::sp<IImageStreamInfo>               pHalImage_Jpeg;

    /**
     *  The jpeg yuv used to store in main jpeg.
     */
    android::sp<IImageStreamInfo>               pHalImage_Jpeg_Sub_YUV = nullptr;

    /**
     *  The depth yuv used to store in main jpeg.
     */
    android::sp<IImageStreamInfo>               pHalImage_Depth_YUV = nullptr;

};


/**
 *  (Pass1-specific) Parsed stream info
 */
struct ParsedStreamInfo_P1
{
    /******************************************
     *  app meta stream info
     ******************************************/
    /**
     *  Only one of P1Node can output this data.
     *  Why do we need more than one of this stream?
     */
    android::sp<IMetaStreamInfo>                pAppMeta_DynamicP1 = nullptr;


    /******************************************
     *  hal meta stream info
     ******************************************/
    android::sp<IMetaStreamInfo>                pHalMeta_Control = nullptr;
    android::sp<IMetaStreamInfo>                pHalMeta_DynamicP1 = nullptr;


    /******************************************
     *  hal image stream info
     ******************************************/
    android::sp<IImageStreamInfo>               pHalImage_P1_Imgo = nullptr;
    android::sp<IImageStreamInfo>               pHalImage_P1_Rrzo = nullptr;
    android::sp<IImageStreamInfo>               pHalImage_P1_Lcso = nullptr;
    android::sp<IImageStreamInfo>               pHalImage_P1_Rsso = nullptr;
    /******************************************
     *  hal image stream info for ISP6.0
     ******************************************/
    android::sp<IImageStreamInfo>               pHalImage_P1_FDYuv = nullptr;
    android::sp<IImageStreamInfo>               pHalImage_P1_ScaledYuv = nullptr;
};


/**
 *  Pipeline nodes need.
 *  true indicates its corresponding pipeline node is needed.
 */
struct PipelineNodesNeed
{
    /**
     * [Note]
     * The index is shared, for example:
     *      needP1Node[index]
     *      PipelineStaticInfo::sensorId[index]
     */
    std::vector<bool>                           needP1Node;

    bool                                        needP2StreamNode = false;
    bool                                        needP2CaptureNode = false;

    bool                                        needFDNode = false;
    bool                                        needJpegNode = false;
    bool                                        needRaw16Node = false;
    bool                                        needPDENode = false;

    bool                                        needP1RawIspTuningDataPackNode = false;
    bool                                        needP2YuvIspTuningDataPackNode = false;
    bool                                        needP2RawIspTuningDataPackNode = false;
};


static inline android::String8 toString(const PipelineNodesNeed& o __unused)
{
    android::String8 os;
    os += "{ ";
    for (size_t i = 0; i < o.needP1Node.size(); i++) {
        if ( o.needP1Node[i] )  { os += android::String8::format("P1Node[%zu] ", i); }
    }
    if ( o.needP2StreamNode )   { os += "P2StreamNode "; }
    if ( o.needP2CaptureNode )  { os += "P2CaptureNode "; }
    if ( o.needFDNode )         { os += "FDNode "; }
    if ( o.needJpegNode )       { os += "JpegNode "; }
    if ( o.needRaw16Node )      { os += "Raw16Node "; }
    if ( o.needPDENode )        { os += "PDENode "; }
    if ( o.needP1RawIspTuningDataPackNode )        { os += "P1RawIspTuningDataPackNode "; }
    if ( o.needP2YuvIspTuningDataPackNode )        { os += "P2YuvIspTuningDataPackNode "; }
    os += "}";
    return os;
};


/**
 *  Pipeline topology
 */
struct PipelineTopology
{
    using NodeSet       = NSCam::v3::pipeline::NSPipelineContext::NodeSet;
    using NodeEdgeSet   = NSCam::v3::pipeline::NSPipelineContext::NodeEdgeSet;

    /**
     * The root nodes of a pipeline.
     */
    NodeSet                                     roots;

    /**
     * The edges to connect pipeline nodes.
     */
    NodeEdgeSet                                 edges;

};


static inline android::String8 toString(const PipelineTopology& o __unused)
{
    android::String8 os;
    os += "{ ";
    os += ".root={";
    for( size_t i = 0; i < o.roots.size(); i++ ) {
        os += android::String8::format(" %#" PRIxPTR " ", o.roots[i]);
    }
    os += "}";
    os += ", .edges={";
    for( size_t i = 0; i < o.edges.size(); i++ ) {
        os += android::String8::format("(%#" PRIxPTR " -> %#" PRIxPTR ")",
            o.edges[i].src, o.edges[i].dst);
    }
    os += "}";
    return os;
};


/**
 * The buffer numbers of Hal P1 output streams (for feature settings)
 */
struct HalP1OutputBufferNum
{
    uint8_t       imgo = 0;
    uint8_t       rrzo = 0;
    uint8_t       lcso = 0;
    uint8_t       rsso = 0;      // reserver for streaming features
    uint8_t       scaledYuv = 0; // reserver for streaming features

};


static inline android::String8 toString(const HalP1OutputBufferNum& o __unused)
{
    android::String8 os;
    os += "{ ";
    os += android::String8::format(".imgo=%u ", o.imgo);
    os += android::String8::format(".rrzo=%u ", o.rrzo);
    os += android::String8::format(".lcso=%u ", o.lcso);
    os += android::String8::format(".rsso=%u ", o.rsso);
    os += android::String8::format(".scaledYuv=%u ", o.scaledYuv);
    os += "}";
    return os;
};


/**
 * Streaming feature settings
 */
struct StreamingFeatureSetting
{
    struct AppInfo
    {
        int32_t recordState = -1;
        uint32_t appMode = 0;
        uint32_t eisOn = 0;
    };

    AppInfo                                     mLastAppInfo;
    uint32_t                                    hdrHalMode = 0;
    uint32_t                                    hdrSensorMode = 0;
    uint32_t                                    nr3dMode = 0;
    uint32_t                                    fscMode = 0;
    bool                                        bNeedLMV = false;
    bool                                        bNeedRSS = false;
    bool                                        bIsEIS   = false;
    bool                                        bPreviewEIS = false;
    bool                                        bNeedLargeRsso = false;
    bool                                        bDisableInitRequest = false;
    NSCam::EIS::EisInfo                         eisInfo;
    uint32_t                                    eisExtraBufNum = 0;
    uint32_t                                    minRrzoEisW    = 0;
    uint32_t                                    groupSize      = 0;
    MSize                                       minRrzoSize = MSize(0, 0);
    // final rrzo = calculated rrz * targetRrzoRatio
    // This ratio contains fixed margin (ex EIS) + max dynamic margin (ex FOV)
    // Currently only support fixed margin
    float                                       targetRrzoRatio = 1.0f;
    float                                       rrzoHeightToWidth = 0;
    bool                                        bEnableTSQ     = false;
    uint32_t                                    BWCScenario = -1;
    uint32_t                                    BWCFeatureFlag = 0;
    uint32_t                                    dsdnHint = 0;
    //
    /**
     * The additional buffer numbers of Hal P1 output streams which may be kept
     * and used by streaming features during processing a streaming request.
     */
    HalP1OutputBufferNum                        additionalHalP1OutputBufferNum;
    // hint support feature for dedicated scenario for P2 node init
    int64_t                                     supportedScenarioFeatures = 0; /*eFeatureIndexMtk and eFeatureIndexCustomer*/
    // PQ/CZ/DRE/HFG
    bool                                        bSupportPQ     = false;
    bool                                        bSupportCZ     = false;
    bool                                        bSupportDRE     = false;
    bool                                        bSupportHFG     = false;
    // ISP6.0
    bool                                        bNeedP1FDYUV = false;
    bool                                        bNeedP1ScaledYUV = false;
};


static inline android::String8 toString(const StreamingFeatureSetting& o __unused)
{
    android::String8 os;
    os += "{ ";
    os += android::String8::format(".additionalHalP1OutputBufferNum=%s ", toString(o.additionalHalP1OutputBufferNum).c_str());
    os += android::String8::format(".supportedScenarioFeatures=%#" PRIx64 " ", o.supportedScenarioFeatures);
    os += "}";
    return os;
};


/**
 * capture feature settings
 */
struct CaptureFeatureSetting
{
    /**
     * The additional buffer numbers of Hal P1 output streams which may be kept
     * and used by capture features during processing a capture request.
     */
    HalP1OutputBufferNum                        additionalHalP1OutputBufferNum;

    /**
     * The max. buffer number of App RAW16 output image stream.
     *
     * App RAW16 output image stream could be used for RAW16 capture or reprocessing.
     *
     * This value is usually used for
     * ParsedAppImageStreamInfo::vAppImage_Output_RAW16::setMaxBufNum.
     */
    uint32_t                                    maxAppRaw16OutputBufferNum = 1;

    /**
     */
    uint32_t                                    maxAppJpegStreamNum     = 1;

    // hint support feature for dedicated scenario for P2 node init
    uint64_t                                    supportedScenarioFeatures = 0; /*eFeatureIndexMtk and eFeatureIndexCustomer*/

    uint32_t                                     pluginUniqueKey           = 0;

    /**
     *  indicate the dualcam feature mode
     *
     *  E_STEREO_FEATURE_xxx in mtkcam3/include/mtkcam3/feature/stereo/StereoCamEnum.h
     */
    uint64_t                                    dualFeatureMode = 0;
};


static inline android::String8 toString(const CaptureFeatureSetting& o __unused)
{
    android::String8 os;
    os += "{ ";
    os += android::String8::format(".additionalHalP1OutputBufferNum=%s ", toString(o.additionalHalP1OutputBufferNum).c_str());
    os += android::String8::format(".maxAppRaw16OutputBufferNum=%u ", o.maxAppRaw16OutputBufferNum);
    os += android::String8::format(".maxAppJpegStreamNum=%u ", o.maxAppJpegStreamNum);
    os += android::String8::format(".supportedScenarioFeatures=%#" PRIx64 " ", o.supportedScenarioFeatures);
    os += "}";
    return os;
};


/**
 * boost scenario control
 */
struct BoostControl
{
    int32_t                                    boostScenario = -1;
    uint32_t                                   featureFlag   = 0;
};

/**
 * ZSL policy
 */
enum eZslPolicy
{
    // bit 0~15:    preserved for image quality. select from metadata.
    // bitwise operation. the history buffer result must fullfile all requirements.
    eZslPolicy_None                 = 0x0,
    eZslPolicy_AfState              = 0x1 << 0,
    eZslPolicy_AeState              = 0x1 << 1,
    eZslPolicy_DualFrameSync        = 0x1 << 2,
    eZslPolicy_PD_ProcessedRaw      = 0x1 << 3,

    // bit 16~27:   preserved for zsl behavior.
    eZslPolicy_Behavior_MASK        = 0x0FFF0000,
    eZslPolicy_ContinuousFrame      = ( 0x1 << 0 ) << 16,
    eZslPolicy_ZeroShutterDelay     = ( 0x1 << 1 ) << 16,

    // bit 28: customize, user selects suitable buffers. cannot combine with other policy.
    eZslPolicy_UserSelect           = ( 0x1 << 0 ) << 28,
};

#define ZslBehaviorOf(PolicyType)   (PolicyType & eZslPolicy_Behavior_MASK)

struct ISelector
{
    typedef std::pair< std::unordered_map< StreamId_T, android::sp<IMetaStreamBuffer> >,
                       std::unordered_map< StreamId_T, android::sp<IImageStreamBuffer> > >
                                                            BufferMap_T;
    // Becareful! The order is <Meta, Image> ...

    struct SelectorInputParams
    {
        // request number from FW
        uint32_t                                            requestNo = 0;

        // key: history frame number
        // value: history buffer of metadata & image
        std::map< uint32_t, BufferMap_T >                   vHistoryData;
    };

    struct SelectorOutputParams
    {
        // one vector represents main frame, sub frame#1, sub frame#2, ... in-order.
        // ex.  History buffer are represented in Rx:[NE(Fy), SE(Fz)], ...
        //      x:request number; y:pipelineframe frame number of NE; z:pipelineframe frame number of SE
        //      R1[NE(F1),SE(F2)], R2[NE(F3),SE(F4)], R3[NE(F5),SE(F6)], R4[NE(F7),SE(F8)], ...
        //
        //      output#1:               NE,SE,NE,SE,NE,SE,LE
        //      vSelectedFrmNo_first   { 1, 1, 1, 1, 1, 1, 0 }
        //      vSelectedFrmNo_second  { 1, 2, 3, 4, 5, 6, 0 }
        //
        //      output#2:               SE,SE,SE,NE,NE,NE,LE
        //      vSelectedFrmNo_first   { 1, 1, 1, 1, 1, 1, 0 }
        //      vSelectedFrmNo_second  { 2, 4, 6, 1, 3, 5, 0 }

        // pair structure in this vector, for each pair:
        // first element represents this frame uses zsl buffers or not
        //      true:  using history buffer (ex. NE & SE)
        //      false: not using history buffer(ex. LE)
        // second element represents the history frame number if using zsl buffers.
        //      the order of history frame number MUST be determined by callee.
        std::vector< std::pair<bool, uint32_t> >            vSelectedFrmNo;
    };

                    ISelector() = default;
    virtual         ~ISelector() {}

    /**
     * select buffer by user.
     *
     * @param[in] rSelectInputParams: the capture request number and history data.
     *
     * @param[out] rSelectOutputParams: results indicate main frame, sub frame#1, sub frame#2, ...
     *                     The callee must promise its value.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual auto    select(
                        std::shared_ptr<SelectorInputParams> const& rSelectInputParams,    // in
                        std::shared_ptr<SelectorOutputParams>&      rSelectOutputParams   // out
                    ) -> int                                    = 0;

};

#define DEFAULT_ZSL_PENDING_FOR_3A_STABLE_TIMEOUT_DURATION (1000) // ms

struct ZslPolicyParams
{
    int32_t                                             mPolicy = eZslPolicy_None; /*eZslPolicy*/
    //
    int64_t                                             mTimestamp = -1;
    int64_t                                             mTimeouts = 2000;

    // user MUST implement ISelector::select() in eZslPolicy_UserSelect policy.
    // FIX ME: weak or strong pointer?
    std::weak_ptr<ISelector>                            mwpSelector = std::weak_ptr<ISelector>();
};


/**
 * Request-stage policy for multicam
 *  $ Need to flush specific NodeId.
 *  $ Need to config specific NodeId.
 */
struct MultiCamReqOutputParams
{
    /**
     * In reconfig stage, if size not equal to zero,
     * it means need flush specific sensor id.
     */
    std::vector<uint32_t>                       flushSensorIdList;
    /**
     * In reconfig stage, if size not equal to zero,
     * it means need config specific node id.
     */
    std::vector<uint32_t>                       configSensorIdList;
    /**
     * use to set sync2a in switch sensor scenario
     */
    std::vector<uint32_t>                       switchControl_Sync2ASensorList;
    /**
     * list needs mark error buffer sensor id list. (For phsyical stream)
     */
    std::vector<uint32_t>                       markErrorSensorList;
    // [sensor status]
    std::vector<uint32_t>                       goToStandbySensorList;
    std::vector<uint32_t>                       streamingSensorList;
    std::vector<uint32_t>                       standbySensorList;
    std::vector<uint32_t>                       resumeSensorList;
    // [crop]
    std::unordered_map<uint32_t, MRect>         tgCropRegionList;
    std::unordered_map<uint32_t, MRect>         tgCropRegionList_Cap;
    std::unordered_map<uint32_t, MRect>         sensorScalerCropRegionList;
    std::unordered_map<uint32_t, MRect>         sensorScalerCropRegionList_Cap;

    bool need3ASwitchDoneNotify = false;
    bool needSync2A = false;
    bool needSyncAf = false;
    bool needFramesync = false;
    bool needSynchelper_3AEnq = false;
    bool needSynchelper_Timestamp = false;

    // master id: decided by sensor control decision.
    uint32_t                                    masterId = (uint32_t)-1;
    // preview master id: current master id in streaming flow.
    uint32_t                                    prvMasterId = (uint32_t)-1;
    // for capture request
    std::vector<uint32_t>                       prvStreamingSensorList;
    // is logical raw capture
    bool                                        needLogicalRawOutput = false;
};

struct AdditionalRawInfo
{
    MINT32 format = -1;
    MINT32 stride = -1;
    MSize imgSize;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_TYPES_H_

