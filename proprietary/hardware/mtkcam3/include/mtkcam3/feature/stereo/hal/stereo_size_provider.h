/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _STEREO_SIZE_PROVIDER_H_
#define _STEREO_SIZE_PROVIDER_H_

#include <mtkcam/drv/def/ispio_port_index.h>
#include "stereo_common.h"
#include "StereoArea.h"
#include "pass2_size_data.h"
#include "stereo_setting_provider.h"

// Threashold of baseline to apply double width to some sizes
#define DOUBLE_WIDTH_THRESHOLD (2.0f)

enum ENUM_BUFFER_NAME
{
    //P2A output for N3D
    E_RECT_IN_M,
    E_RECT_IN_S,
    E_MASK_IN_M,
    E_MASK_IN_S,
    //P2A output for DL-Depth
    E_DLD_P2A_M,
    E_DLD_P2A_S,

    //N3D Output
    E_MV_Y,
    E_MASK_M_Y,
    E_SV_Y,
    E_MASK_S_Y,
    E_LDC,

    E_WARP_MAP_M,
    E_WARP_MAP_S,

    //N3D before MDP for capture
    E_MV_Y_LARGE,
    E_MASK_M_Y_LARGE,
    E_SV_Y_LARGE,
    E_MASK_S_Y_LARGE,

    //DPE Output
    E_DMP_H,
    E_CFM_H,
    E_RESPO,

    //DPE2
    E_DV_LR,
    E_ASF_CRM,
    E_ASF_RD,
    E_ASF_HF,
    E_CFM_M,

    //DL Depth
    E_DL_DEPTHMAP,

    //OCC Output
    E_MY_S,
    E_DMH,
    E_OCC,
    E_NOC,

    //WMF Output
    E_DMW,
    E_DEPTH_MAP,

    //Video AIDepth output
    E_VAIDEPTH_DEPTHMAP,

    //GF Output
    E_DMBG,
    E_DMG,
    E_INK,

    //GF input sizes is from output size, so put them into the same group
    E_GF_IN_IMG_2X,
    E_GF_IN_IMG_4X,

    //Bokeh Output
    E_BOKEH_WROT,
    E_BOKEH_WDMA, //Clean image
    E_BOKEH_3DNR,
    E_BOKEH_PACKED_BUFFER,

    // Put this at the end
    STEREO_BUFFER_NAME_COUNT
};

extern const char *STEREO_BUFFER_NAMES[];

using namespace NSImageio::NSIspio;
using namespace StereoHAL;
using namespace std;    //TODO: remove in the future

class StereoSizeProvider
{
public:
    /**
     * \brief Get instance of size provider
     * \detail Size provider is implemented as singleton
     *
     * \return Instance of size provider
     */
    static StereoSizeProvider *getInstance();

    /**
     * \brief Reset size provider
     * \details This call will release cache of sizes, should be called when any one these changes:
     *          1. Open ID
     *          2. Image ratio
     *          3. Feature Mode
     *          4. Scenario(Preview, record)
     *          5. Base size
     */
    virtual void reset() = 0;

    /**
     * \brief Get pass1 related sizes
     *
     * \param sensor Sensor to get size
     * \param format Format of output image
     * \param port Output port of pass1
     * \param scenario The scenario to get image
     * \param tgCropRect Output TG crop rectangle
     * \param outSize Output size
     * \param strideInBytes Output stride
     * \return true if success
     */
    virtual bool getPass1Size( StereoHAL::ENUM_STEREO_SENSOR sensor,
                               NSCam::EImageFormat format,
                               NSImageio::NSIspio::EPortIndex port,
                               StereoHAL::ENUM_STEREO_SCENARIO scenario,
                               NSCam::MRect &tgCropRect,
                               NSCam::MSize &outSize,
                               MUINT32 &strideInBytes,
                               ENUM_STEREO_RATIO requiredRatio = eRatio_Unknown
                             ) = 0;

    /**
     * \brief Get pass1 related sizes by sensor ID
     *
     * \param SENSOR_INDEX Physical sensor index
     * \param format Format of output image
     * \param port Output port of pass1
     * \param scenario The scenario to get image
     * \param tgCropRect Output TG crop rectangle
     * \param outSize Output size
     * \param strideInBytes Output stride
     * \return true if success
     */
    virtual bool getPass1Size( const int32_t SENSOR_INDEX,
                               NSCam::EImageFormat format,
                               NSImageio::NSIspio::EPortIndex port,
                               StereoHAL::ENUM_STEREO_SCENARIO scenario,
                               NSCam::MRect &tgCropRect,
                               NSCam::MSize &outSize,
                               MUINT32 &strideInBytes,
                               ENUM_STEREO_RATIO requiredRatio = eRatio_Unknown
                             ) = 0;

    /**
     * \brief Get pass1 active array cropping rectangle
     * \details AP is working on the coodination of active array
     *
     * \param sensor Sensor to active array
     * \param cropRect Output active array cropping rectangle
     * \param isCropNeeded set to false if you want full size
     *
     * \return true if suceess
     */
    virtual bool getPass1ActiveArrayCrop(StereoHAL::ENUM_STEREO_SENSOR sensor, NSCam::MRect &cropRect, bool isCropNeeded=true) = 0;

    /**
     * \brief Get Pass2-A related size
     * \details The size will be affected by module orientation and image ratio
     *
     * \param round The round of Pass2-A
     * \param eScenario Stereo scenario
     * \param pass2SizeInfo Output size info
     * \return true if success
     */
    virtual bool getPass2SizeInfo(StereoHAL::ENUM_PASS2_ROUND round, StereoHAL::ENUM_STEREO_SCENARIO eScenario, StereoHAL::Pass2SizeInfo &pass2SizeInfo) const = 0;

    //For rests, will rotate according to module orientation and ratio inside
    /**
     * \brief Get buffer size besides Pass2-A,
     * \details The size will be affected by module orientation and image ratio
     *
     * \param eName Buffer name unum
     * \param eScenario Stereo scenario
     * \param capOrientation Capture orientation
     *
     * \return The area of each buffer
     */
    virtual StereoHAL::StereoArea getBufferSize(ENUM_BUFFER_NAME eName,
                                                StereoHAL::ENUM_STEREO_SCENARIO eScenario = StereoHAL::eSTEREO_SCENARIO_UNKNOWN,
                                                ENUM_ROTATION capOrientation = eRotate_0) = 0;

    /**
     * \brief Set capture image size
     * \details Capture size may change in runtime, this API can set the capture size set from AP
     *
     * \param w Width
     * \param h Height
     */
    virtual void setCaptureImageSize(int w, int h) = 0;

    /**
     * \brief Get capture image size
     * \details Capture size may change in runtime, this API can get the capture size set in AP
     * \return Size of captured image
     */
    virtual NSCam::MSize captureImageSize() = 0;

    /**
     * \brief Get custom IMGO and RRZO YUV sizes
     *
     * \param sensor Sensor to get size
     * \param port Output port of pass1
     * \param outSize Output size
     * \return true if success
     */
    virtual bool getcustomYUVSize( StereoHAL::ENUM_STEREO_SENSOR sensor,
                                   NSImageio::NSIspio::EPortIndex port,
                                   NSCam::MSize &outSize
                                 ) = 0;

    /**
     * \brief Get depthmap size for 3rd party, user must set logical device ID first
     *
     * \param ratio Image ratio of the image
     * \return Size of depthmap for 3rd party
     */
    virtual NSCam::MSize thirdPartyDepthmapSize(ENUM_STEREO_RATIO ratio, StereoHAL::ENUM_STEREO_SCENARIO senario=StereoHAL::eSTEREO_SCENARIO_PREVIEW) const = 0;

    /**
     * \brief Set current preview size, must set before start preview
     *
     * \param size Preview size
     */
    virtual void setPreviewSize(NSCam::MSize size) = 0;

    /**
     * \brief Get current preview size
     * \return Preview size
     */
    virtual NSCam::MSize getPreviewSize() const = 0;

    /**
     * \brief Get crop area for full sized YUV(from IMGO) and target size for P2
     * \details The full size of crop area is the size of sensor output
     *
     * \param SENSOR_INDEX Sensor index to query
     * \param cropRect Output crop rect
     * \param targetSize Output target size
     * \return true if succeed
     */
    virtual bool getDualcamP2IMGOYuvCropResizeInfo(const int SENSOR_INDEX, NSCam::MRect &cropRect, NSCam::MSize &targetSize) = 0;

    /**
     * \brief Set IMGO YUV Size
     * \details Set from StereoSettingProvider
     *
     * \param sensorIndex Sensor index
     * \param size Size to set
     */
    virtual void setIMGOYUVSize(int32_t sensorIndex, NSCam::MSize &size) = 0;

    /**
     * \brief Set RRZO YUV Size
     * \details Set from StereoSettingProvider
     *
     * \param sensorIndex Sensor index
     * \param size Size to set
     */
    virtual void setRRZOYUVSize(int32_t sensorIndex, NSCam::MSize &size) = 0;

    /**
     * \brief Set customized base size
     * \details Set from StereoSettingProvider
     *
     * \param ratio Image ratio to set
     * \param sizeConfig Size to config for the ratio
     */
    virtual void setCustomizedSize(ENUM_STEREO_RATIO ratio, StereoArea &sizeConfig) = 0;

    /**
     * \brief Get customized base size of the ratio
     * \details Must set before use
     *
     * \param ratio Ratio to query
     * \param size Base size
     *
     * \return False if the size of the ratio was not set, true if succeed
     */
    virtual bool getCustomziedSize(ENUM_STEREO_RATIO ratio, StereoArea &size) = 0;

protected:
    StereoSizeProvider() {}
    virtual ~StereoSizeProvider() {}

    friend class StereoSettingProvider;
    static void destroyInstance();
};

#endif
