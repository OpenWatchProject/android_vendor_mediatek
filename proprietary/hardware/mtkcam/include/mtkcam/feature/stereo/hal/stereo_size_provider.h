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

#include <map>
#include <cutils/atomic.h>
#include <utils/Mutex.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wnull-conversion"
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wunused-value"
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#pragma GCC diagnostic pop
#include "stereo_common.h"
#include "StereoArea.h"
#include "pass2_size_data.h"
#include "stereo_setting_provider.h"

#define STEREO_SIZE_PROVIDER_DEBUG

#ifdef STEREO_SIZE_PROVIDER_DEBUG    // Enable debug log.

#undef __func__
#define __func__ __FUNCTION__

#define SIZE_PROVIDER_LOGD(fmt, arg...)    ALOGD("[%s]" fmt, __func__, ##arg)
#define SIZE_PROVIDER_LOGI(fmt, arg...)    ALOGI("[%s]" fmt, __func__, ##arg)
#define SIZE_PROVIDER_LOGW(fmt, arg...)    ALOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define SIZE_PROVIDER_LOGE(fmt, arg...)    ALOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else   // Disable debug log.
#define SIZE_PROVIDER_LOGD(a,...)
#define SIZE_PROVIDER_LOGI(a,...)
#define SIZE_PROVIDER_LOGW(a,...)
#define SIZE_PROVIDER_LOGE(a,...)
#endif  // STEREO_SIZE_PROVIDER_DEBUG

// Threashold of baseline to apply double width to some sizes
#define DOUBLE_WIDTH_THRESHOLD (2.0f)

enum ENUM_BUFFER_NAME
{
    //P2A output for N3D
    E_RECT_IN_M,
    E_RECT_IN_S,
    E_MASK_IN_M,
    E_MASK_IN_S,

    //N3D Output
    E_MV_Y,
    E_MASK_M_Y,
    E_SV_Y,
    E_MASK_S_Y,
    E_LDC,

    E_SV_CC_Y,
    E_PRE_MASK_S_Y,

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

    //OCC Output
    E_MY_S,
    E_DMH,
    E_OCC,
    E_NOC,

    //WMF Output
    E_DMW,
    E_DEPTH_MAP,

    //GF Output
    E_DMG,
    E_DMBG,
    E_INK,

    //GF input
    E_GF_IN_IMG_2X,
    E_GF_IN_IMG_4X,

    //Bokeh Output
    E_BOKEH_WROT, //VSDOF image
    E_BOKEH_WDMA, //Clean image
    E_BOKEH_3DNR,
    E_BOKEH_PACKED_BUFFER,

    //BMDeNoise
    E_BM_PREPROCESS_FULLRAW_CROP_1,
    E_BM_PREPROCESS_FULLRAW_CROP_2,
    E_BM_PREPROCESS_OUT_YUV,
    E_BM_PREPROCESS_OUT_YUV_IMG3O,
    E_BM_PREPROCESS_W_1,
    E_BM_PREPROCESS_OUT_W_1_IMG3O,
    E_BM_PREPROCESS_W_2,
    E_BM_PREPROCESS_OUT_MFBO,
    E_BM_PREPROCESS_MFBO_1,
    E_BM_PREPROCESS_MFBO_2,
    E_BM_PREPROCESS_MFBO_FINAL_1,
    E_BM_PREPROCESS_MFBO_FINAL_2,
    E_BM_DENOISE_HAL_OUT,
    E_BM_DENOISE_HAL_OUT_ROT_BACK,
    E_BM_DENOISE_HAL_OUT_ROT_BACK_IMG3O,
    E_BM_DENOISE_FINAL_RESULT,
    E_BM_DENOISE_SWNR_IN_YV12,
    E_BM_PREPROCESS_SMALL_YUV,
    E_BM_PREPROCESS_FUSION_OUT
};

//using namespace NSImageio;
using namespace NSImageio::NSIspio;
using namespace StereoHAL;

enum {
    STEREO_AREA_WO_PADDING = false,
    STEREO_AREA_W_PADDING  = true,
    STEREO_AREA_WO_ROTATE  = false,
    STEREO_AREA_W_ROTATE   = true,
};

struct StereoSizeConfig
{
    bool hasPadding = false;
    bool needRotate = false;

    StereoSizeConfig(bool hasPad=false, bool rotate=false)
    {
        hasPadding  = hasPad;
        needRotate  = rotate;
    }
};

class StereoSize
{
public:
    //Base size will be IP-based
    static StereoArea getStereoArea1x(const StereoSizeConfig &config);

    static StereoArea getStereoArea2x(const StereoSizeConfig &config)
    {
        return getStereoArea1x(config) * 2;
    }

    static StereoArea getStereoArea4x(const StereoSizeConfig &config)
    {
        return getStereoArea1x(config) * 4;
    }

    static StereoArea getStereoArea8x(const StereoSizeConfig &config)
    {
        return getStereoArea1x(config) * 8;
    }
};

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
    bool getPass1Size( ENUM_STEREO_SENSOR sensor,
                       NSCam::EImageFormat format,
                       EPortIndex port,
                       ENUM_STEREO_SCENARIO scenario,
                       NSCam::MRect &tgCropRect,
                       NSCam::MSize &outSize,
                       MUINT32 &strideInBytes,
                       ENUM_STEREO_RATIO requiredRatio = eRatio_Unknown
                       );

    /**
     * \brief Get pass1 active array cropping rectangle
     * \details AP is working on the coodination of active array
     *
     * \param sensor Sensor to active array
     * \param cropRect Output active array cropping rectangle
     *
     * \return true if suceess
     */
    bool getPass1ActiveArrayCrop(ENUM_STEREO_SENSOR sensor, NSCam::MRect &cropRect);

    /**
     * \brief Get Pass2-A related size
     * \details The size will be affected by module orientation and image ratio
     *
     * \param round The round of Pass2-A
     * \param eScenario Stereo scenario
     * \param pass2SizeInfo Output size info
     * \return true if success
     */
    bool getPass2SizeInfo(ENUM_PASS2_ROUND round, ENUM_STEREO_SCENARIO eScenario, Pass2SizeInfo &pass2SizeInfo) const;

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
    StereoArea getBufferSize(ENUM_BUFFER_NAME eName,
                             ENUM_STEREO_SCENARIO eScenario = eSTEREO_SCENARIO_UNKNOWN,
                             ENUM_ROTATION capOrientation = eRotate_0);

    /**
     * \brief Get side-by-side(SBS) image(a.k.a. JPS) size
     * \details JPS size = ( single image size + padding ) * 2
     * \return SBS image size
     */
    NSCam::MSize getSBSImageSize();    //For capture only

    /**
     * \brief Set capture image size
     * \details Capture size may change in runtime, this API can set the capture size set from AP
     *
     * \param w Width
     * \param h Height
     */
    void setCaptureImageSize(int w, int h);

    /**
     * \brief Get capture image size
     * \details Capture size may change in runtime, this API can get the capture size set in AP
     * \return Size of captured image
     */
    NSCam::MSize captureImageSize() { return StereoArea(__captureSize).applyRatio(StereoSettingProvider::imageRatio(), E_KEEP_WIDTH); }

    /**
     * \brief Get post view size
     * \details Post view is the thumbnail image showed in AP after capture
     *
     * \param ratio Ratio to query
     * \return Size of post view
     */
    NSCam::MSize postViewSize(ENUM_STEREO_RATIO ratio=StereoSettingProvider::imageRatio());

#if (16 == STEREO_HAL_VER || 17 == STEREO_HAL_VER || STEREO_HAL_VER >= 30)
    /**
     * \brief Get custom IMGO and RRZO YUV sizes
     *
     * \param sensor Sensor to get size
     * \param port Output port of pass1
     * \param outSize Output size
     * \return true if success
     */
    bool getcustomYUVSize( ENUM_STEREO_SENSOR sensor,
                           EPortIndex port,
                           NSCam::MSize &outSize
                         );

    /**
     * \brief Get depthmap size for 3rd party
     *
     * \param ratio Image ratio of the image
     * \return Size of depthmap for 3rd party
     */
    NSCam::MSize thirdPartyDepthmapSize(ENUM_STEREO_SENSOR_PROFILE profile, ENUM_STEREO_RATIO ratio) const;
#endif

    /**
     * \brief Set current preview size, must set before start preview
     *
     * \param size Preview size
     */
    void setPreviewSize(NSCam::MSize size);

    /**
     * \brief Get current preview size
     * \return Preview size
     */
    NSCam::MSize getPreviewSize() const {
        return __previewSize;
    }

#if (30 == STEREO_HAL_VER)
    /**
     * \brief Get custom IR sensor RRZO sizes
     *
     * \param sensor Sensor to get size
     * \param outSize Output size
     * \return true if success
     */
    bool getcustomIRRRZOSize( ENUM_STEREO_SENSOR sensor,
                              NSCam::MSize &outSize
                            );
#endif

public:
    //For updating sizes when user called StereoSettingProvider::setStereoPrifile()
    friend class StereoSettingProvider;
    friend class StereoSize;

protected:
    StereoSizeProvider();
    virtual ~StereoSizeProvider() {};

private:
#if (1 == STEREO_DENOISE_SUPPORTED)
    bool __updateBMDeNoiseSizes();
#endif
    bool __getCenterCrop(NSCam::MSize &srcSize, NSCam::MRect &rCrop);

    void __setIMGOYUVSize(NSCam::MSize main1Size, NSCam::MSize main2Size) {
        __imgoYuvSize[0] = main1Size;
        __imgoYuvSize[1] = main2Size;
    }
    void __setRRZOYUVSize(NSCam::MSize main1Size, NSCam::MSize main2Size) {
        __rrzoYuvSize[0] = main1Size;
        __rrzoYuvSize[1] = main2Size;
    }
    void __setPostviewSizes(std::map<ENUM_STEREO_RATIO, NSCam::MSize> &sizes) {
        __postviewSizes = sizes;
    }
    void __setPreviewSize(NSCam::MSize size) {
        __previewSize = size;
    }

#if (30 == STEREO_HAL_VER)
    void __setIRRRZOSize(NSCam::MSize main1Size, NSCam::MSize main2Size) {
        __irRrzoSize[0] = main1Size;
        __irRrzoSize[1] = main2Size;
    }
#endif

#if (17 == STEREO_HAL_VER || STEREO_HAL_VER >= 30)
    void __setCustomizedSize(ENUM_STEREO_RATIO ratio, StereoArea &sizeConfig)
    {
        __customizedSize[ratio] = sizeConfig;
        __hasCustomizedSize = true;
    }
#else
    void __setCustomizedSize(StereoArea &sizeConfig_16_9, StereoArea &sizeConfig_4_3) {
        __customizedSize[eRatio_16_9] = sizeConfig_16_9;
        __customizedSize[eRatio_4_3]  = sizeConfig_4_3;
        __hasCustomizedSize = true;
    }
#endif

    bool __getCustomziedSize(StereoArea &size) {
        if(__hasCustomizedSize) {
            ENUM_STEREO_RATIO ratio = StereoSettingProvider::imageRatio();
            if(StereoSettingProvider::isDeNoise()) {
                ratio = eRatio_4_3;
            }

            if(__customizedSize.find(ratio) != __customizedSize.end()) {
                size = __customizedSize[ratio];
                return true;
            }

            return false;
        }

        return __hasCustomizedSize;
    }

private:
    static android::Mutex   __lock;
    static android::Mutex   __P1Lock;
    NSCam::MSize            __previewSize = NSCam::MSize(1920, 1080);
    NSCam::MSize            __captureSize = NSCam::MSize(4208, 3120);
    NSCam::MSize            __imgoYuvSize[2];  //[0]: main1 [1]: main2
    NSCam::MSize            __rrzoYuvSize[2];  //[0]: main1 [1]: main2
    std::map<ENUM_STEREO_RATIO, NSCam::MSize> __postviewSizes;
    NSCam::MSize            __irRrzoSize[2];   //[0]: main1 [1]: main2

    bool                    __hasCustomizedSize = false;
    std::map<ENUM_STEREO_RATIO, StereoArea> __customizedSize;

#if (1 == STEREO_DENOISE_SUPPORTED)
    NSCam::MSize            __BMDeNoiseAlgoSize_main1;
    NSCam::MSize            __BMDeNoiseAlgoSize_main2;
    NSCam::MSize            __BMDeNoiseFullRawSize_main1;
    NSCam::MSize            __BMDeNoiseFullRawSize_main2;
    NSCam::MRect            __BMDeNoiseFullRawCropSize_main1;
    NSCam::MRect            __BMDeNoiseFullRawCropSize_main2;
#endif

    std::map<MUINT32, NSCam::MRect> __pass1TgCropRectMap;
    std::map<MUINT32, NSCam::MSize> __pass1OutSizeMap;
    std::map<MUINT32, MUINT32>      __pass1StrideMap;

    const bool              LOG_ENABLED;
};

#endif