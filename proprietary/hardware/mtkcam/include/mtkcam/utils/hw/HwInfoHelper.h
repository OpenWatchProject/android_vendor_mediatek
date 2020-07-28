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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_HWINFOHELPER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_HWINFOHELPER_H_
//
#include <memory>
#include <vector>

#include <mtkcam/def/common.h>
#include <mtkcam/def/ImageBufferInfo.h>
#include <mtkcam/drv/def/ICam_type.h>
#define DISPLAY_WIDTH 2560;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCamHW {

class HwInfoHelper
{
public:
    enum e4CellSensorPattern
    {
        e4CellSensorPattern_Unknown = 0,
        e4CellSensorPattern_Unpacked,
        e4CellSensorPattern_Packed,
    };

protected:  ////                Implementor.
                                class Implementor;
                                class DualImplementor;
    Implementor*                mpImp;

public:
                                HwInfoHelper(MINT32 const openId);
                                ~HwInfoHelper();
public:
    MBOOL                       updateInfos();
    MBOOL                       isRaw() const;
    MBOOL                       isYuv() const;
    MBOOL                       hasRrzo() const { return !isYuv(); }
    MBOOL                       getSensorSize(MUINT32 const sensorMode, NSCam::MSize& size) const;
    MBOOL                       getSensorOutputSize(MUINT32 const sensorMode, NSCam::MSize& size) const;
    MBOOL                       getSensorFps(MUINT32 const sensorMode, MINT32& fps) const;
    MBOOL                       getSensorVBTime(MUINT32 sensorMode, MUINT32& VBTime) const;
    MBOOL                       getSensorPDSize(MUINT32 sensorMode, MUINT32& PDSize) const;
    MBOOL                       getImgoFmt(MUINT32 const bitDepth, MINT& fmt, MBOOL forceUFO = MFALSE, MBOOL needUnpakFmt = MFALSE, MBOOL isSecure = MFALSE) const;
    MBOOL                       getRrzoFmt(MUINT32 const bitDepth, MINT& fmt, MBOOL forceUFO = MFALSE, MBOOL isSecure = MFALSE) const;
    MBOOL                       getLpModeSupportBitDepthFormat(MINT& fmt, MUINT32& pipeBit) const;
    MBOOL                       getRecommendRawBitDepth(MINT32& bitDepth) const;
    MBOOL                       getSensorPowerOnPredictionResult(MBOOL& isPowerOnSuccess)const;
    //
    MBOOL                       queryPixelMode(
                                    MUINT32 const sensorMode,
                                    MINT32 const fps,
                                    MUINT32& pixelMode
                                ) const;
    MBOOL                       alignPass1HwLimitation(
                                    MUINT32 const pixelMode /*not needed*/,
                                    MINT const imgFormat,
                                    MBOOL isImgo,
                                    NSCam::MSize& size,
                                    size_t& stride
                                ) const;
    MBOOL                       alignPass1HwLimitation(
                                    MINT const imgFormat,
                                    MBOOL isImgo,
                                    NSCam::MSize& size,
                                    size_t& stride
                                ) const;
    MBOOL                       alignRrzoHwLimitation(
                                    NSCam::MSize const targetSize,
                                    NSCam::MSize const sensorSize,
                                    NSCam::MSize& result,
                                    MUINT32 const recommendResizeRatio = 0 // multiplied by 100
                                ) const;

    MBOOL                       querySupportVHDRMode(
                                    MUINT32 const sensorMode,
                                    MUINT32& vhdrMode
                                ) const;
    MBOOL                       quertMaxRrzoWidth(MINT32 &maxWidth) const;
    MBOOL                       getPDAFSupported(MUINT32 const sensorMode) const;
    MBOOL                       isType3PDSensorWithoutPDE(MUINT32 const sensorMode, MBOOL const checkSensorOnly = MFALSE) const;
    MBOOL                       getDualPDAFSupported(MUINT32 const sensorMode) const;
    MBOOL                       queryUFOStride(MINT const imgFormat, NSCam::MSize const imgSize, size_t stride[3]) const;
    MBOOL                       getShutterDelayFrameCount(MINT32& shutterDelayCnt) const;
    MBOOL                       get4CellSensorSupported() const;
    // To query the 4 cell sensor pattern. If the sensor is not a 4 cell sensor,
    // this method returns e4CellSensorPattern_Unknown.
    //  @return                 The pattern that this sensor is using.
    //  @note                   If the sensor is not 4 cell sensor, this method
    //                          returns e4CellSensorPattern_Unknown.
    e4CellSensorPattern         get4CellSensorPattern() const;
    MBOOL                       getSensorRawFmtType(MUINT32 &u4RawFmtType) const;
    MBOOL                       shrinkCropRegion(NSCam::MSize const sensorSize, NSCam::MRect& cropRegion, MINT32 shrinkPx = 2) const;

    MBOOL                       querySupportResizeRatio(MUINT32& rPrecentage) const;
    MBOOL                       querySupportBurstNum(MUINT32& rBitField) const;
    MBOOL                       querySupportRawPattern(MUINT32& rBitField) const;

    static MBOOL                getDynamicTwinSupported();
    // NOTE: Due to twin driver is not fully supported under secure flow currently,
    //       this API helps us to check the image sensor mode capability under secure flow.
    static MBOOL                getSecureSensorModeSupported(MUINT32 sensorMode);

    static MUINT32              getCameraSensorPowerOnCount();
    static MBOOL                getPorcessRawInfo(MINT& fmt);
    static MBOOL                getLMVSupported(const MUINT32 &idx, const MUINT32 &rawType);
};

};  //namespace NSCamHW


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

class IHwInfoHelper
{
public:     ////    Interfaces.
    virtual         ~IHwInfoHelper() = default;

    virtual auto    getDefaultBufPlanes_Pass1(
                        BufPlanes& bufPlanes,
                        int format,
                        MSize const& imgSize,
                        size_t stride
                    ) const -> bool                                         = 0;

    virtual auto    getDefaultBufPlanes_Imgo(
                        BufPlanes& bufPlanes,
                        int format,
                        MSize const& imgSize
                    ) const -> bool                                         = 0;

    virtual auto    getDefaultBufPlanes_Rrzo(
                        BufPlanes& bufPlanes,
                        int format,
                        MSize const& imgSize
                    ) const -> bool                                         = 0;



    struct QueryRawTypeSupportParams
    {
        /*********************************************************************
         * Output Parameters
         *********************************************************************/

        /**
         * true indicates post-processing raw is supported (by Pass2)
         * That's Pass2 has the capability (e.g. lsc engine) to post-process raw.
         */
        bool                            isPostProcRawSupport = true;

        /**
         * true indicates processed raw from imgo is supported (by Pass1)
         */
        bool                            isProcImgoSupport = true;

        /*********************************************************************
         * Input Parameters
         *********************************************************************/
        struct CameraInputParams
        {
            /**
             *  Physical sensor id (0, 1, 2)
             */
            int32_t                     sensorId = 0;

            /**
             *  Sensor mode
             */
            uint32_t                    sensorMode = 0;

            /**
             *  minimum processing fps
             */
            uint32_t                    minProcessingFps = 30;

            /**
             * RRZO Image resolution in pixel.
             */
            MSize                       rrzoImgSize;

        };
        /**
         * Input Parameters.
         */
        std::vector<CameraInputParams>  in;

    };
    virtual auto    queryRawTypeSupport(
                    QueryRawTypeSupportParams& params
                    ) const -> void                                         = 0;

    /* use to query p1 driver capability */
    struct QueryP1DrvIspParams
    {
        struct P1DrvIspOutputParams
        {
            E_CamIQLevel eIqLevel = eCamIQ_MAX;
            CAM_RESCONFIG eResConfig;
        };
                /*********************************************************************
         * Input Parameters
         *********************************************************************/
        struct CameraInputParams
        {
            /**
             *  Physical sensor id (0, 1, 2)
             */
            int32_t                     sensorId = 0;

            /**
             *  Sensor mode
             */
            uint32_t                    sensorMode = 0;

            /**
             *  minimum processing fps
             */
            uint32_t                    minProcessingFps = 30;

            /**
             * RRZO Image resolution in pixel.
             */
            MSize                       rrzoImgSize;

        };

        /**
         * Input Parameters.
         */
        std::vector<CameraInputParams>  in;
        /**
         * Output Parameters.
         */
        std::vector<P1DrvIspOutputParams>  out;
    };
    virtual auto    queryP1DrvIspCapability(
                    QueryP1DrvIspParams& params
                    ) const -> void                                         = 0;


};


class IHwInfoHelperManager
{
public:     ////    Interfaces.
    virtual         ~IHwInfoHelperManager() = default;

    /**
     * Get the singleton instance of IHwInfoHelperManager.
     */
    static  auto    get() -> IHwInfoHelperManager*;

    /**
     * Get the instance of IHwInfoHelper for a specific sensor id.
     *
     * @param[in] Physical sensor id (0, 1, 2)
     */
    virtual auto    getHwInfoHelper(
                        int32_t const sensorId
                    ) const -> std::shared_ptr<IHwInfoHelper>               = 0;

    /**
     * Jpeg HW has an alignment constraint of yuv input.
     *
     * @param[out] bufPlanes
     *
     * @param[in] format: the image format.
     *
     * @param[in] imgSize: the image size, in pixels.
     */
    virtual auto    getDefaultBufPlanes_JpegYuv(
                        BufPlanes& bufPlanes,
                        int format,
                        MSize const& imgSize
                    ) const -> bool                                         = 0;

    /**
     * Obtain the default buffer planes of pass1.
     *
     * @param[out] bufPlanes
     *
     * @param[in] format: the image format.
     *
     * @param[in] imgSize: the image size, in pixels.
     *
     * @param[in] stride: the stride, in bytes.
     */
    virtual auto    getDefaultBufPlanes_Pass1(
                        BufPlanes& bufPlanes,
                        int format,
                        MSize const& imgSize,
                        size_t stride
                    ) const -> bool                                         = 0;

};


};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_HWINFOHELPER_H_

