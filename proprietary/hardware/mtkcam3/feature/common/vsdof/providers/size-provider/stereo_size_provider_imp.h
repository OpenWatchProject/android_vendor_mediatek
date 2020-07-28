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
#ifndef _STEREO_SIZE_PROVIDER_IMP_H_
#define _STEREO_SIZE_PROVIDER_IMP_H_

#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <unordered_map>
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

#define WPE_MAX_SIZE NSCam::MSize(640, 480) //dots, not bytes
typedef size_t KEY_TYPE;

using namespace NSImageio::NSIspio;
using namespace StereoHAL;
using namespace NSCam;

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

struct Pass1SizeResult
{
    NSCam::MRect tgCropRect;
    NSCam::MSize outSize;
    MUINT32      strideInBytes;

    Pass1SizeResult(NSCam::MRect &rect, NSCam::MSize &size, MUINT32 stride)
    {
        tgCropRect    = rect;
        outSize       = size;
        strideInBytes = stride;
    }
};

class StereoSizeProviderImp : public StereoSizeProvider
{
public:
    StereoSizeProviderImp();
    virtual ~StereoSizeProviderImp();

    virtual void reset() override;

    virtual bool getPass1Size( ENUM_STEREO_SENSOR sensor,
                               NSCam::EImageFormat format,
                               EPortIndex port,
                               ENUM_STEREO_SCENARIO scenario,
                               NSCam::MRect &tgCropRect,
                               NSCam::MSize &outSize,
                               MUINT32 &strideInBytes,
                               ENUM_STEREO_RATIO requiredRatio = eRatio_Unknown
                               ) override;

    virtual bool getPass1Size( const int32_t SENSOR_INDEX,
                               NSCam::EImageFormat format,
                               NSImageio::NSIspio::EPortIndex port,
                               StereoHAL::ENUM_STEREO_SCENARIO scenario,
                               NSCam::MRect &tgCropRect,
                               NSCam::MSize &outSize,
                               MUINT32 &strideInBytes,
                               ENUM_STEREO_RATIO requiredRatio = eRatio_Unknown
                             ) override;

    virtual bool getPass1ActiveArrayCrop(ENUM_STEREO_SENSOR sensor, NSCam::MRect &cropRect, bool isCropNeeded=true) override;

    virtual bool getPass2SizeInfo(ENUM_PASS2_ROUND round, ENUM_STEREO_SCENARIO eScenario, Pass2SizeInfo &pass2SizeInfo) const override;

    virtual StereoArea getBufferSize(ENUM_BUFFER_NAME eName,
                                     ENUM_STEREO_SCENARIO eScenario = eSTEREO_SCENARIO_UNKNOWN,
                                     ENUM_ROTATION capOrientation = eRotate_0) override;

    virtual void setCaptureImageSize(int w, int h) override;

    virtual NSCam::MSize captureImageSize()  override
            { return StereoArea(__captureSize).applyRatio(StereoSettingProvider::imageRatio(), E_KEEP_WIDTH); }

    virtual bool getcustomYUVSize( ENUM_STEREO_SENSOR sensor,
                                   EPortIndex port,
                                   NSCam::MSize &outSize
                                 ) override;

    virtual NSCam::MSize thirdPartyDepthmapSize(ENUM_STEREO_RATIO ratio, ENUM_STEREO_SCENARIO senario=eSTEREO_SCENARIO_PREVIEW) const override;

    virtual void setPreviewSize(NSCam::MSize size) override;

    virtual NSCam::MSize getPreviewSize() const  override {
        return __previewSize;
    }

    virtual bool getDualcamP2IMGOYuvCropResizeInfo(const int SENSOR_INDEX, NSCam::MRect &cropRect, NSCam::MSize &targetSize) override;

    virtual void setIMGOYUVSize(int32_t sensorIndex, NSCam::MSize &size) override
    {
        __imgoYuvSizes.emplace(sensorIndex, size);
    }

    virtual void setRRZOYUVSize(int32_t sensorIndex, NSCam::MSize &size) override
    {
        __rrzoYuvSizes.emplace(sensorIndex, size);
    }

    virtual void setCustomizedSize(ENUM_STEREO_RATIO ratio, StereoArea &sizeConfig) override
    {
        __customizedSize.emplace(ratio, sizeConfig);
    }

    virtual bool getCustomziedSize(ENUM_STEREO_RATIO ratio, StereoArea &size) override
    {
        auto iter = __customizedSize.find(ratio);
        if(iter != __customizedSize.end()) {
            size = iter->second;
            return true;
        }

        return false;
    }

private:
    bool __getCenterCrop(NSCam::MSize &srcSize, NSCam::MRect &rCrop);

    void __setPreviewSize(NSCam::MSize size) {
        __previewSize = size;
    }

    void __getRRZOSize( ENUM_STEREO_SENSOR sensor,
                        EImageFormat format,
                        ENUM_STEREO_SCENARIO scenario,
                        MRect &tgCropRect,
                        MSize &outSize,
                        ENUM_STEREO_RATIO currentRatio );

    // Can be customized in buffer-size/[default|platform]/buffer_size.cpp
    StereoArea __getBufferSize(ENUM_BUFFER_NAME eName,
                               ENUM_STEREO_SCENARIO eScenario,
                               ENUM_ROTATION capOrientation);

    float __getFOVCropRatio(ENUM_STEREO_SENSOR sensor);

    void __cropRectByFOVCropAndImageRatio(ENUM_STEREO_SENSOR sensor, ENUM_STEREO_RATIO imageRatio, MRect &rect);

#if (1==HAS_AIDEPTH)
    MSize __getAIDepthModelSize(ENUM_STEREO_RATIO ratio=StereoSettingProvider::imageRatio());
    MSize __getVideoAIDepthModelSize(ENUM_STEREO_RATIO ratio=StereoSettingProvider::imageRatio());
#endif

private:
    static android::Mutex   __pass1SizeLock;
    static android::Mutex   __activeCropLock;
    static android::Mutex   __bufferSizeLock;
    static android::Mutex   __resetLock;

    int                     __LOG_ENABLED = StereoSettingProvider::isLogEnabled();
    NSCam::MSize            __previewSize = NSCam::MSize(1920, 1080);
    NSCam::MSize            __captureSize = NSCam::MSize(4208, 3120);
    std::unordered_map<int32_t, NSCam::MSize> __imgoYuvSizes;
    std::unordered_map<int32_t, NSCam::MSize> __rrzoYuvSizes;

    std::unordered_map<ENUM_STEREO_RATIO, StereoArea> __customizedSize;

    std::unordered_map<ENUM_STEREO_RATIO, MSize> __aidepthModelSize;
    std::unordered_map<ENUM_STEREO_RATIO, MSize> __vaidepthModelSize;

    std::unordered_map<KEY_TYPE, Pass1SizeResult>   __pass1SizeMap;
    std::unordered_map<KEY_TYPE, NSCam::MRect>      __activeCropMap;
    std::unordered_map<KEY_TYPE, StereoArea>        __bufferSizeMap;
};

#endif
