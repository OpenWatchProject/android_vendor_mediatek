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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "StereoSizeProvider"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
CAM_ULOG_DECLARE_MODULE_ID(CAM_ULOG_MODULE_ID);

#include <math.h>
#include <algorithm>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>

#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include "stereo_size_provider_imp.h"
#include "pass2/pass2A_size_providers.h"
#include <vsdof/hal/ProfileUtil.h>
#include <stereo_tuning_provider.h>
#include "../setting-provider/stereo_setting_provider_kernel.h"
#include "base_size.h"
#include <stereo_crop_util.h>

#if (1==HAS_AIDEPTH)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <MTKAIDepth.h>
#include <MTKVideoAIDepth.h>
#pragma GCC diagnostic pop
#endif

using namespace android;
using namespace NSCam;

#define STEREO_SIZE_PROVIDER_DEBUG

#ifdef STEREO_SIZE_PROVIDER_DEBUG    // Enable debug log.

#undef __func__
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else   // Disable debug log.
#define MY_LOGD(a,...)
#define MY_LOGI(a,...)
#define MY_LOGW(a,...)
#define MY_LOGE(a,...)
#endif  // STEREO_SIZE_PROVIDER_DEBUG

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

const char *STEREO_BUFFER_NAMES[] =
{
    //P2A output for N3D
    "RECT_IN_M",
    "RECT_IN_S",
    "MASK_IN_M",
    "MASK_IN_S",
    //P2A output for DL-Depth
    "DLD_P2A_M",
    "DLD_P2A_S",

    //N3D Output
    "MV_Y",
    "MASK_M_Y",
    "SV_Y",
    "MASK_S_Y",
    "LDC",

    "WARP_MAP_M",
    "WARP_MAP_S",

    //N3D before MDP for capture
    "MV_Y_LARGE",
    "MASK_M_Y_LARGE",
    "SV_Y_LARGE",
    "MASK_S_Y_LARGE",

    //DPE Output
    "DMP_H",
    "CFM_H",
    "RESPO",

    //DPE2
    "DV_LR",
    "ASF_CRM",
    "ASF_RD",
    "ASF_HF",
    "CFM_M",

    //DL Depth
    "DL_DEPTHMAP",

    //OCC Output
    "MY_S",
    "DMH",
    "OCC",
    "NOC",

    //WMF Output
    "DMW",
    "DEPTH_MAP",

    //Video AIDepth output
    "VAIDEPTH_DEPTHMAP",

    //GF Output
    "DMBG",
    "DMG",
    "INK",

    //GF input sizes is from output size, so put them into the same group
    "GF_IN_IMG_2X",
    "GF_IN_IMG_4X",

    //Bokeh Output
    "BOKEH_WROT",
    "BOKEH_WDMA", //Clean image
    "BOKEH_3DNR",
    "BOKEH_PACKED_BUFFER",
};

static_assert(STEREO_BUFFER_NAME_COUNT == sizeof(STEREO_BUFFER_NAMES)/sizeof(*STEREO_BUFFER_NAMES),
              "Please check content of STEREO_BUFFER_NAMES and ENUM_BUFFER_NAME");

// STEREO_BASE_SIZE_4_3 is defined in base_size.h under platform folder or using default if not exist

std::unordered_map<int, StereoArea> sizeCache1x;
StereoArea
StereoSize::getStereoArea1x(const StereoSizeConfig &config)
{
    StereoArea area = STEREO_BASE_SIZE_4_3;
#ifdef CUSTOMIZE_REFINE_NONE_SIZE
    if(StereoSettingProvider::isMTKDepthmapMode() &&
       E_DEPTHMAP_REFINE_NONE == StereoSettingProvider::getDepthmapRefineLevel())
    {
        area = STEREO_BASE_SIZE_4_3_REFINE_NONE;
    }
#endif

    const bool IS_VERTICAL_MODULE = (StereoSettingProvider::getModuleRotation() & 0x2);
    ENUM_STEREO_RATIO imageRatio = StereoSettingProvider::imageRatio();
    if(!StereoSizeProviderImp::getInstance()->getCustomziedSize(imageRatio, area)) {
        auto iter = sizeCache1x.find(imageRatio);
        if(iter != sizeCache1x.end()) {
            area = iter->second;
        } else {
            int m, n;
            imageRatioMToN(imageRatio, m, n);

            ENUM_IMAGE_RATIO_ALIGNMENT alignment = E_KEEP_AREA_SIZE;
#ifdef CUSTOMIZE_RATIO_ALIGNMENT
            alignment = CUSTOMIZE_RATIO_ALIGNMENT;
#endif
            if(E_KEEP_WIDTH == alignment) {
                if(m * 3 / 4 < n) {
                    alignment = E_KEEP_HEIGHT;
                }
            }
            else if(E_KEEP_HEIGHT == alignment)
            {
                if(n * 4 / 3 < m) {
                    alignment = E_KEEP_WIDTH;
                }
            }

            area.applyRatio(imageRatio, alignment);

            MSize pvSize = StereoSizeProvider::getInstance()->getPreviewSize();
            if(area.size.w > pvSize.w ||
               area.size.h > pvSize.h)
            {
                MY_LOGD("Base size changes to preview size %dx%d", pvSize.w, pvSize.h);
                area.size = pvSize;
            }

            area.rotatedByModule();

            MSize contentSize = area.contentSize();
            int nAlign = 16;
#ifdef CUSTOMIZE_BUFFER_ALIGNMENT
            nAlign = CUSTOMIZE_BUFFER_ALIGNMENT;
#endif
            applyNAlign(nAlign, contentSize.w);
            if(IS_VERTICAL_MODULE) {
                contentSize.h = contentSize.w * m / n;
            } else {
                contentSize.h = contentSize.w * n / m;
            }
            contentSize.h = (contentSize.h+1) & (~1);

            area.size = area.padding + contentSize;

            sizeCache1x.emplace(imageRatio, area);

            char msg[128];
            sprintf(msg, "StereoArea1x(%d:%d)", m, n);
            area.print(LOG_TAG, msg);
        }
    }

    //Vertical size is rotated, only rotate for horizontal request
    if(IS_VERTICAL_MODULE &&
       !config.needRotate)
    {
        area.rotate(eRotate_90);
    }

    if(!config.hasPadding) {
        area.removePadding();
    }

    return area;
}

//===============================================================
//  StereoSizeProvider implementation
//===============================================================

//===============================================================
//  Singleton and init operations
//===============================================================
Mutex __lock;
Mutex StereoSizeProviderImp::__pass1SizeLock;
Mutex StereoSizeProviderImp::__activeCropLock;
Mutex StereoSizeProviderImp::__bufferSizeLock;
Mutex StereoSizeProviderImp::__resetLock;

StereoSizeProviderImp *g_pInstance = nullptr;
#define IS_CACHED (true)

StereoSizeProvider *
StereoSizeProvider::getInstance()
{
    if(g_pInstance == nullptr) {
        Mutex::Autolock lock(__lock);
        // Double check is required
        if(g_pInstance == nullptr) {
            g_pInstance = new StereoSizeProviderImp();
        }
    }

    return g_pInstance;
}

void
StereoSizeProvider::destroyInstance()
{
    if(g_pInstance) {
        Mutex::Autolock lock(__lock);
        // Double check is required
        if(g_pInstance) {
            delete g_pInstance;
            g_pInstance = nullptr;
        }
    }
}

StereoSizeProviderImp::StereoSizeProviderImp()
{
}

StereoSizeProviderImp::~StereoSizeProviderImp()
{
}

void
StereoSizeProviderImp::reset()
{
    Mutex::Autolock lock(__resetLock);
    __LOG_ENABLED = StereoSettingProvider::isLogEnabled();
    MY_LOGD_IF(__LOG_ENABLED, "");

    __imgoYuvSizes.clear();
    __rrzoYuvSizes.clear();
    __customizedSize.clear();
    __aidepthModelSize.clear();
    __vaidepthModelSize.clear();
    __pass1SizeMap.clear();
    __activeCropMap.clear();
    __bufferSizeMap.clear();
    sizeCache1x.clear();
}

bool
StereoSizeProviderImp::getPass1Size( const int32_t SENSOR_INDEX,
                                     NSCam::EImageFormat format,
                                     NSImageio::NSIspio::EPortIndex port,
                                     StereoHAL::ENUM_STEREO_SCENARIO scenario,
                                     NSCam::MRect &tgCropRect,
                                     NSCam::MSize &outSize,
                                     MUINT32 &strideInBytes,
                                     ENUM_STEREO_RATIO requiredRatio
                                   )
{
    ENUM_STEREO_SENSOR sensor = StereoSettingProvider::sensorIdToEnum(SENSOR_INDEX);
    if(sensor == eSTEREO_SENSOR_UNKNOWN) {
        return false;
    }

    return getPass1Size(sensor, format, port, scenario, tgCropRect, outSize, strideInBytes, requiredRatio);
}

bool
StereoSizeProviderImp::getPass1Size( ENUM_STEREO_SENSOR sensor,
                                     EImageFormat format,
                                     EPortIndex port,
                                     ENUM_STEREO_SCENARIO scenario,
                                     MRect &tgCropRect,
                                     MSize &outSize,
                                     MUINT32 &strideInBytes,
                                     ENUM_STEREO_RATIO requiredRatio
                                   )
{
    // AutoProfileUtil profile(LOG_TAG, "getPass1Size");
    ENUM_STEREO_RATIO currentRatio = (EPortIndex_IMGO == port) ? eRatio_4_3 : StereoSettingProvider::imageRatio();
    if(requiredRatio != eRatio_Unknown){
        MY_LOGD_IF(__LOG_ENABLED, "user required ratio:%d", requiredRatio);
        currentRatio = requiredRatio;
    }

    int sensorScenario = StereoSettingProvider::getCurrentSensorScenario(sensor);
    if(SENSOR_SCENARIO_ID_UNNAMED_START == sensorScenario) {
        MY_LOGE("Unknown sensor scenario for sensor %d", sensor-1);
        return false;
    }

    enum {
        SENSOR_BITS          = 3,   //Up to 7(~main7) sensors in a logical device
        PORT_BITS            = 3,   //RRZO, IMGO, CRZO_R2, RSSO_R2
        SCENARIO_BITS        = 2,
        FORMAT_BITS          = 22,  //Last line
    };

    // SENSOR_BITS
    KEY_TYPE key = sensor;
    int shiftBits = SENSOR_BITS;

    // PORT_BITS
    switch(port) {
        case EPortIndex_RRZO:
        case EPortIndex_YUVO:
        default:
            // Default is key |= 0<<shiftBits
            break;
        case EPortIndex_IMGO:
            key |= 1<<shiftBits;
            break;
        case EPortIndex_CRZO_R2:
            key |= 2<<shiftBits;
            break;
        case EPortIndex_RSSO_R2:
            key |= 3<<shiftBits;
            break;
    }
    shiftBits += PORT_BITS;

    // SCENARIO_BITS
    key |= scenario<<shiftBits;
    shiftBits += SCENARIO_BITS;

    key |= format << shiftBits; //put this line at last

    int m, n;
    imageRatioMToN(currentRatio, m, n);

    int sensorID = StereoSettingProvider::sensorEnumToId(sensor);
    if(sensorID < 0) {
        return false;
    }

    auto printSize = [&](bool isCached) {
        if(!__LOG_ENABLED) {
            return;
        }

        std::ostringstream oss;
        if(isCached) {
            oss << "Use";
        } else {
            oss << "New";
        }
        oss << " result of";
        oss << " Sensor main" << sensor << "(" << sensorID << ")";
        oss << ", Port ";
        switch(port)
        {
            case EPortIndex_RRZO:
                oss << "RRZO";
                break;
            case EPortIndex_IMGO:
                oss << "IMGO";
                break;
            case EPortIndex_CRZO_R2:
                oss << "CRZO_R2";
                break;
            case EPortIndex_RSSO_R2:
                oss << "RSSO_R2";
                break;
            case EPortIndex_YUVO:
                oss << "YUVO";
                break;
            default:
                oss << "Unknown/Unsupported";
        }
        oss << ", Format 0x" << std::hex << format << std::dec;
        oss << ", Scenario " << scenario;
        oss << ": tg (" << tgCropRect.p.x << ", " << tgCropRect.p.y << "), " << tgCropRect.s.w << "x" << tgCropRect.s.h;
        oss << ", outSize " << outSize.w << "x" << outSize.h;
        oss << ", stride " << strideInBytes;

        // For printing
        #ifdef __func__
        #undef __func__
        #endif
        #define __func__ "getPass1Size"
        MY_LOGD("%s", oss.str().c_str());
        #undef __func__
    };

    {
        Mutex::Autolock lock(__pass1SizeLock);
        auto iter = __pass1SizeMap.find(key);
        if(iter != __pass1SizeMap.end()) {
            tgCropRect    = iter->second.tgCropRect;
            outSize       = iter->second.outSize;
            strideInBytes = iter->second.strideInBytes;
            printSize(IS_CACHED);
            return true;
        }
    }

    auto saveToCache = [&] {
        Mutex::Autolock lock(__pass1SizeLock);
        Pass1SizeResult result(tgCropRect, outSize, strideInBytes);
        __pass1SizeMap.emplace(key, result);
        printSize(!IS_CACHED);
    };

    //Return true if size has been changed
    auto updateSizeAndStride = [&](MSize &size, MUINT32 &stride) -> bool
    {
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo queryRst;
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryIn input;
        input.width = size.w;
        input.pixMode = NSCam::NSIoPipe::NSCamIOPipe::ePixMode_NONE;
        NSCam::NSIoPipe::NSCamIOPipe::INormalPipeModule::get()->query(
                port,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
                format,
                input,
                queryRst);

        stride = queryRst.stride_byte;
        if(EPortIndex_IMGO != port &&
           size.w != queryRst.x_pix)
        {
            size.w = queryRst.x_pix;
            return true;
        }

        return false;
    };

    // Prepare sensor hal
    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    if(pHalDeviceList == nullptr) {
        return false;
    }
    int sensorDevIndex = pHalDeviceList->querySensorDevIdx(sensorID);
    IHalSensorList* sensorList = MAKE_HalSensorList();
    if(NULL == sensorList) {
        MY_LOGE("Cannot get sensor list");
        return false;
    }

    IHalSensor* pIHalSensor = sensorList->createSensor(LOG_TAG, sensorID);
    if(NULL == pIHalSensor) {
        MY_LOGE("Cannot get hal sensor");
        return false;
    }

    //Get sensor crop win info
    SensorCropWinInfo rSensorCropInfo;
    int err = pIHalSensor->sendCommand(sensorDevIndex, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                       (MUINTPTR)&sensorScenario, (MUINTPTR)&rSensorCropInfo, 0);
    pIHalSensor->destroyInstance(LOG_TAG);

    if(err) {
        MY_LOGE("Cannot get sensor crop win info");
        return false;
    }

    tgCropRect.p.x = 0;
    tgCropRect.p.y = 0;
    tgCropRect.s.w = rSensorCropInfo.w2_tg_size;
    tgCropRect.s.h = rSensorCropInfo.h2_tg_size;

    outSize.w = rSensorCropInfo.w2_tg_size;
    outSize.h = rSensorCropInfo.h2_tg_size;

    // IMGO does not support resize & crop, so we just update stride and return
    if( EPortIndex_IMGO == port ) {
        updateSizeAndStride(outSize, strideInBytes);
        saveToCache();
        return true;
    }

    //Calculate TG CROP without FOV cropping
    if( EPortIndex_RRZO == port ||
        EPortIndex_YUVO == port )
    {
        __getRRZOSize(sensor, format, scenario, tgCropRect, outSize, currentRatio);
    }
    else if( EPortIndex_CRZO_R2 == port ||
             EPortIndex_RSSO_R2 == port )
    {
        __cropRectByFOVCropAndImageRatio(sensor, currentRatio, tgCropRect);

        //Make sure output size meet the following rules:
        // 1. image ratio fit 4:3 or 16:9
        // 2. smaller than TG size
        // 3. Is even number
        outSize = tgCropRect.s;
        fitSizeToImageRatio(outSize, currentRatio);

        //P1 YUV is used only for preview
        //RRZO is for preview/record only, if scenario is capture, query by preview
        ENUM_STEREO_SCENARIO queryScenario = (eSTEREO_SCENARIO_CAPTURE == scenario)
                                             ? eSTEREO_SCENARIO_PREVIEW
                                             : scenario;
        MSize outputSize = [&] {
            StereoArea area;
            switch(sensor) {
                case eSTEREO_SENSOR_MAIN1:
                default:
                    area = getBufferSize(E_RECT_IN_M, queryScenario);
                    break;
                case eSTEREO_SENSOR_MAIN2:
                    area = getBufferSize(E_RECT_IN_S, queryScenario);
                    break;
            }

            return area.removePadding().rotatedByModule().size;
        }();

        if(outputSize.h > outputSize.w) {
            std::swap(outputSize.w, outputSize.h);
        }

        // YUV sizes come from RRZO, so they must <= RRZO size
        MRect junkRect;
        MSize rrzoSize;
        MUINT32 junkStride;
        if(getPass1Size( sensor, eImgFmt_FG_BAYER10, EPortIndex_RRZO, scenario,
                         junkRect, rrzoSize, junkStride, requiredRatio ))
        {
            if(outputSize.w > rrzoSize.w ||
               outputSize.h > rrzoSize.h)
            {
                outputSize = rrzoSize;
            }
        }

        //Pass1 does not support rotation
        if(outputSize.w < outSize.w ||
           outputSize.h < outSize.h)
        {
            outSize = outputSize;
        }
    }

    // Make sure all sizes <= IMGO size
    outSize.w = std::min((int)rSensorCropInfo.w2_tg_size, outSize.w);
    outSize.h = std::min((int)rSensorCropInfo.h2_tg_size, outSize.h);

    bool isSizeUpdated = updateSizeAndStride(outSize, strideInBytes);
    if(isSizeUpdated)
    {
        // Keep flexibility for 3rd party to config sizes
        if(!StereoSettingProvider::is3rdParty(scenario))
        {
            // Fit ratio and apply 2-align
            outSize.h = (outSize.w * n / m)&~0x1;
        }

        MY_LOGD("Out size changes to %dx%d", outSize.w, outSize.h);
    }

    saveToCache();

    return true;
}

void StereoSizeProviderImp::__getRRZOSize( ENUM_STEREO_SENSOR sensor,
                                           EImageFormat format,
                                           ENUM_STEREO_SCENARIO scenario,
                                           MRect &tgCropRect,
                                           MSize &outSize,
                                           ENUM_STEREO_RATIO currentRatio
                                         )
{
    //RRZO is for preview/record only, if scenario is capture, query by preview
    ENUM_STEREO_SCENARIO queryScenario = (eSTEREO_SCENARIO_CAPTURE == scenario)
                                         ? eSTEREO_SCENARIO_PREVIEW
                                         : scenario;

    //Check min RRZO size
    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo queryRst;
    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryIn input;
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipeModule::get()->query(
        EPortIndex_RRZO,
        NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BS_RATIO,
        format,
        input,
        queryRst);
    const int RRZ_CAPIBILITY = queryRst.bs_ratio;

    auto getMinRRZOSize = [&] (const MSize &tgSize) -> MSize
    {
        return MSize( (tgSize.w * RRZ_CAPIBILITY/100 + 1)&~1,
                      (tgSize.h * RRZ_CAPIBILITY/100 + 1)&~1 );
    };

    if(StereoSettingProvider::is3rdParty(queryScenario))
    {
        CropUtil::cropRectByImageRatio(tgCropRect, currentRatio);
        MSize MIN_RRZ_SIZE = getMinRRZOSize(tgCropRect.s);

        int sensorID = StereoSettingProvider::sensorEnumToId(sensor);
        outSize = [&] {
            if(sensorID >= 0) {
                auto iter = __rrzoYuvSizes.find(sensorID);
                if(iter != __rrzoYuvSizes.end())
                {
                    return iter->second;
                }
            }

            MY_LOGI("NOTICE! RRZOYUV Size is not configured for sensor %d, use tg size", sensorID);
            return tgCropRect.s;
        }();

        if(outSize.w < MIN_RRZ_SIZE.w ||
           outSize.h < MIN_RRZ_SIZE.h)
        {
            MY_LOGD_IF(__LOG_ENABLED, "RRZO %dx%d->%dx%d (limited by RRZ capibility: %.2f)",
                       outSize.w, outSize.h, MIN_RRZ_SIZE.w, MIN_RRZ_SIZE.h,
                       RRZ_CAPIBILITY/100.0f);
            outSize = MIN_RRZ_SIZE;
        }

        MY_LOGD_IF(__LOG_ENABLED, "3rdParty RRZO size=%dx%d, sensor: main%d(%d)", outSize.w, outSize.h, sensor, sensorID);
    }
    else
    {
        __cropRectByFOVCropAndImageRatio(sensor, currentRatio, tgCropRect);
        MSize MIN_RRZ_SIZE = getMinRRZOSize(tgCropRect.s);

        //Make sure output size meet the following rules:
        // 1. image ratio fit 4:3 or 16:9
        // 2. smaller than TG size
        // 3. Is even number
        outSize = tgCropRect.s;
        fitSizeToImageRatio(outSize, currentRatio);

        //Use smaller size for RRZO if possible
        Pass2SizeInfo p2SizeInfo;
        MSize p2SizeNeeded;

        if(eSTEREO_SENSOR_MAIN1 == sensor) {
            //Compare with PASS2A.WDMA (Preview size)
            Pass2SizeInfo p2SizeInfo;
            getPass2SizeInfo(PASS2A, queryScenario, p2SizeInfo);
            p2SizeNeeded = p2SizeInfo.areaWDMA;
        } else {
            //Compare with PASS2A_P.WROT (FE input size)
            Pass2SizeInfo p2APSizeInfo;
            Pass2SizeInfo p2AP2SizeInfo;
            getPass2SizeInfo(PASS2A_P, queryScenario, p2APSizeInfo);
            getPass2SizeInfo(PASS2A_P_2, queryScenario, p2AP2SizeInfo);
            MSize p2aWROTSize = p2APSizeInfo.areaWROT;
            MSize p2ap2WDMASize = p2AP2SizeInfo.areaWDMA;
            if(StereoSettingProvider::getModuleRotation() & 0x2) {
                std::swap(p2aWROTSize.w, p2aWROTSize.h);
                std::swap(p2ap2WDMASize.w, p2ap2WDMASize.h);
            }

            if(p2aWROTSize.w > p2ap2WDMASize.w) {
                p2SizeNeeded = p2aWROTSize;
            } else {
                p2SizeNeeded = p2ap2WDMASize;
            }

            //Make sure RRZO output fit ratio
            int m, n;
            imageRatioMToN(currentRatio, m, n);
            NSCam::MSize::value_type baseSize = std::ceil(std::max(p2SizeNeeded.w/(m*2.0f), p2SizeNeeded.h/(n*2.0f)));
            p2SizeNeeded.w = baseSize * m * 2;
            p2SizeNeeded.h = baseSize * n * 2;
        }

        if(p2SizeNeeded.w < MIN_RRZ_SIZE.w ||
           p2SizeNeeded.h < MIN_RRZ_SIZE.h)
        {
            p2SizeNeeded = MIN_RRZ_SIZE;
            MY_LOGD_IF(__LOG_ENABLED, "RRZO %dx%d->%dx%d(%d%%) (scenario %d, limited by: RRZ capibility %.2f)",
                   outSize.w, outSize.h, p2SizeNeeded.w, p2SizeNeeded.h,
                   (int)(p2SizeNeeded.w*p2SizeNeeded.h*100.0f/(outSize.w*outSize.h)),
                   queryScenario, RRZ_CAPIBILITY/100.0f);
        }
        else if(p2SizeNeeded.w > outSize.w ||
                p2SizeNeeded.h > outSize.h)
        {
            p2SizeNeeded = outSize;
            MY_LOGD_IF(__LOG_ENABLED, "RRZO %dx%d->%dx%d(%d%%) (scenario %d, limited by: RRZO cannot scale up)",
                   outSize.w, outSize.h, p2SizeNeeded.w, p2SizeNeeded.h,
                   (int)(p2SizeNeeded.w*p2SizeNeeded.h*100.0f/(outSize.w*outSize.h)), queryScenario);
        } else {
            MY_LOGD_IF(__LOG_ENABLED, "RRZO %dx%d->%dx%d(%d%%) (scenario %d)",
                   outSize.w, outSize.h, p2SizeNeeded.w, p2SizeNeeded.h,
                   (int)(p2SizeNeeded.w*p2SizeNeeded.h*100.0f/(outSize.w*outSize.h)),
                   queryScenario);
        }

        if(p2SizeNeeded.w < outSize.w &&
           p2SizeNeeded.h < outSize.h )
        {
            outSize = p2SizeNeeded;
        }

#if (HAS_P1YUV > 1)
        if(scenario == eSTEREO_SCENARIO_RECORD) {
            //WPE must be 64-align
            outSize = StereoArea(outSize).apply64Align().size;
        }
#endif
    }
}

bool
StereoSizeProviderImp::getPass1ActiveArrayCrop(ENUM_STEREO_SENSOR sensor, MRect &cropRect, bool isCropNeeded)
{
    enum {
        SENSOR_BITS          = 4,   //Up to 16 sensors in a logical device
        NEED_CROP_BITS       = 1,
    };

    // SENSOR_BITS
    KEY_TYPE key = sensor;
    int shiftBits = SENSOR_BITS;

    // NEED_CROP_BIT
    key |= isCropNeeded << shiftBits;

    // Prepare sensor hal
    int32_t sensorId = StereoSettingProvider::sensorEnumToId(sensor);
    if(sensorId < 0) {
        return false;
    }

    ENUM_STEREO_RATIO imageRatio = StereoSettingProvider::imageRatio();
    auto printSize = [&](bool isCached) {
        if(!__LOG_ENABLED) {
            return;
        }

        int m, n;
        imageRatioMToN(imageRatio, m, n);
        // For printing
        #ifdef __func__
        #undef __func__
        #endif
        #define __func__ "getPass1ActiveArrayCrop"
        MY_LOGD("Active domain for sensor %d: offset (%d, %d), size %dx%d, ratio %d:%d, is crop %d(cache %d)",
                sensorId, cropRect.p.x, cropRect.p.y, cropRect.s.w, cropRect.s.h, m, n, isCropNeeded, isCached);
        #undef __func__
    };

    {
        Mutex::Autolock lock(__activeCropLock);
        auto iter = __activeCropMap.find(key);
        if(iter != __activeCropMap.end()) {
            cropRect = iter->second;
            printSize(IS_CACHED);
            return true;
        }
    }

    // Prepare sensor static info
    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    if(pHalDeviceList == nullptr) {
        return false;
    }
    int sensorDevIndex = pHalDeviceList->querySensorDevIdx(sensorId);
    SensorStaticInfo sensorStaticInfo;
    memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
    pHalDeviceList->querySensorStaticInfo(sensorDevIndex, &sensorStaticInfo);

    cropRect.p = MPOINT_ZERO;
    cropRect.s = MSize(sensorStaticInfo.captureWidth, sensorStaticInfo.captureHeight);

    if(isCropNeeded)
    {
        // Use preview to query since capture should align preview's setting
        if(StereoSettingProvider::is3rdParty(eSTEREO_SCENARIO_PREVIEW)) {
            CropUtil::cropRectByImageRatio(cropRect, imageRatio);
        } else {
            __cropRectByFOVCropAndImageRatio(sensor, imageRatio, cropRect);
        }
    }

    {
        Mutex::Autolock lock(__activeCropLock);
        __activeCropMap[key] = cropRect;
    }

    printSize(!IS_CACHED);
    return true;
}

bool
StereoSizeProviderImp::getPass2SizeInfo(ENUM_PASS2_ROUND round, ENUM_STEREO_SCENARIO eScenario, Pass2SizeInfo &pass2SizeInfo) const
{
    bool isSuccess = true;
    switch(round) {
        case PASS2A:
            pass2SizeInfo = Pass2A_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_2:
            pass2SizeInfo = Pass2A_2_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_3:
            pass2SizeInfo = Pass2A_3_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_P:
            pass2SizeInfo = Pass2A_P_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_P_2:
            pass2SizeInfo = Pass2A_P_2_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_P_3:
            pass2SizeInfo = Pass2A_P_3_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_B:
            pass2SizeInfo = Pass2A_B_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_CROP:
            pass2SizeInfo = Pass2A_Crop_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_B_CROP:
            pass2SizeInfo = Pass2A_B_Crop_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_P_CROP:
            pass2SizeInfo = Pass2A_P_Crop_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        default:
            isSuccess = false;
    }

    return isSuccess;
}

#if (1==HAS_AIDEPTH)
MSize
StereoSizeProviderImp::__getAIDepthModelSize(ENUM_STEREO_RATIO ratio)
{
    MSize result(896, 512);
    auto iter = __aidepthModelSize.find(ratio);
    if(iter != __aidepthModelSize.end())
    {
        result = iter->second;
    }
    else
    {
        MTKAIDepth *pAIDepth = MTKAIDepth::createInstance(DRV_AIDEPTH_OBJ_SW);
        if(pAIDepth) {
            AIDepthModelInfo info;
#if 0
            if(StereoSettingProvider::getModuleRotation()&0x2)
            {
                if(eRatio_4_3 == StereoSettingProvider::imageRatio())
                {
                    pAIDepth->AIDepthGetModelInfo(AIDEPTH_FEATURE_3_4, (void *)&info);
                }
                else
                {
                    pAIDepth->AIDepthGetModelInfo(AIDEPTH_FEATURE_9_16, (void *)&info);
                }
                result.w = info.modelHeight;
                result.h = info.modelWidth;
            }
            else
            {
                if(eRatio_4_3 == StereoSettingProvider::imageRatio())
                {
                    pAIDepth->AIDepthGetModelInfo(AIDEPTH_FEATURE_4_3, (void *)&info);
                }
                else
                {
                    pAIDepth->AIDepthGetModelInfo(AIDEPTH_FEATURE_16_9, (void *)&info);
                }
                result.w = info.modelWidth;
                result.h = info.modelHeight;
            }
#else
            pAIDepth->AIDepthGetModelInfo(AIDEPTH_FEATURE_9_16, (void *)&info);
            result.w = info.modelHeight;
            result.h = info.modelWidth;
#endif
            pAIDepth->AIDepthReset();
            pAIDepth->destroyInstance(pAIDepth);
            pAIDepth = NULL;

            __aidepthModelSize.emplace(ratio, result);
        }
    }

    return result;
}

MSize
StereoSizeProviderImp::__getVideoAIDepthModelSize(ENUM_STEREO_RATIO ratio)
{
    MSize result(896, 512);
    auto iter = __vaidepthModelSize.find(ratio);
    if(iter != __vaidepthModelSize.end())
    {
        result = iter->second;
    }
    else
    {
        MTKVideoAIDepth *pAIDepth = MTKVideoAIDepth::createInstance(DRV_VIDEOAIDEPTH_OBJ_SW);
        if(pAIDepth) {
            VideoAIDepthModelInfo info;
#if 0
            if(StereoSettingProvider::getModuleRotation()&0x2)
            {
                if(eRatio_4_3 == StereoSettingProvider::imageRatio())
                {
                    pAIDepth->AIDepthGetModelInfo(AIDEPTH_FEATURE_3_4, (void *)&info);
                }
                else
                {
                    pAIDepth->AIDepthGetModelInfo(AIDEPTH_FEATURE_9_16, (void *)&info);
                }
                result.w = info.modelHeight;
                result.h = info.modelWidth;
            }
            else
            {
                if(eRatio_4_3 == StereoSettingProvider::imageRatio())
                {
                    pAIDepth->AIDepthGetModelInfo(AIDEPTH_FEATURE_4_3, (void *)&info);
                }
                else
                {
                    pAIDepth->AIDepthGetModelInfo(AIDEPTH_FEATURE_16_9, (void *)&info);
                }
                result.w = info.modelWidth;
                result.h = info.modelHeight;
            }
#else
            pAIDepth->VideoAIDepthGetModelInfo(AIDEPTH_FEATURE_9_16, (void *)&info);
            result.w = info.modelWidth;
            result.h = info.modelHeight;
#endif
            pAIDepth->VideoAIDepthReset();
            pAIDepth->destroyInstance(pAIDepth);
            pAIDepth = NULL;

            __vaidepthModelSize.emplace(ratio, result);
        }
    }

    return result;
}

#endif  //#if (1==HAS_AIDEPTH)

StereoArea
StereoSizeProviderImp::getBufferSize(ENUM_BUFFER_NAME eName,
                                     ENUM_STEREO_SCENARIO eScenario,
                                     ENUM_ROTATION capOrientation)
{
    StereoArea result = STEREO_AREA_ZERO;
    enum {
        BUFFER_NAME_BITS     = 8,
        SCENARIO_BITS        = 2,
        CAP_ORIENTATIN_BITS  = 2,   //1: 90, 2: 180, 3: 270
    };

    // SENSOR_BITS
    KEY_TYPE key = eName;
    int shiftBits = BUFFER_NAME_BITS;

    // SCENARIO_BITS
    key |= eScenario<<shiftBits;
    shiftBits += SCENARIO_BITS;

    //put this line at last
    if(eSTEREO_SCENARIO_CAPTURE == eScenario)
    {
        switch(capOrientation)
        {
            case 0:
            default:
                break;
            case 90:
                key |= 1 << shiftBits;
                break;
            case 270:
                key |= 3 << shiftBits;
                break;
            case 180:
                key |= 2 << shiftBits;
                break;
        }
    }

    bool isCached = false;
    {
        Mutex::Autolock lock(__bufferSizeLock);
        auto iter = __bufferSizeMap.find(key);
        if(iter != __bufferSizeMap.end()) {
            isCached = true;
            result = iter->second;
        }
    }

    if(!isCached) {
    result = __getBufferSize(eName, eScenario, capOrientation);

        Mutex::Autolock lock(__bufferSizeLock);
        __bufferSizeMap.emplace(key, result);
    }

    MY_LOGD_IF(__LOG_ENABLED, "Buffer %s, scenario %d, capOrientation %d: size %dx%d, padding %dx%d, startPt (%d, %d)(%s)",
                              STEREO_BUFFER_NAMES[eName], eScenario, capOrientation,
                              result.size.w, result.size.h, result.padding.w, result.padding.h, result.startPt.x, result.startPt.y,
                              (isCached) ? "Cached" : "New");

    return result;
}

void
StereoSizeProviderImp::setCaptureImageSize(int w, int h)
{
    __captureSize.w = w;
    __captureSize.h = h;
    MY_LOGD("Set capture size %dx%d", w, h);
}

bool
StereoSizeProviderImp::getcustomYUVSize( ENUM_STEREO_SENSOR sensor,
                                         EPortIndex port,
                                         MSize &outSize
                                       )
{
    outSize = MSIZE_ZERO;
    if( EPortIndex_IMGO == port)
    {
        outSize = [&] {
            int sensorID = StereoSettingProvider::sensorEnumToId(sensor);
            if(sensorID >= 0) {
                auto iter = __imgoYuvSizes.find(sensor);
                if(iter != __imgoYuvSizes.end())
                {
                    return iter->second;
                }
            }

            Pass2SizeInfo p2SizeInfo;
            ENUM_PASS2_ROUND queryRound = (eSTEREO_SENSOR_MAIN1 == sensor) ? PASS2A_CROP : PASS2A_P_CROP;
            getPass2SizeInfo(queryRound, eSTEREO_SCENARIO_CAPTURE, p2SizeInfo);
            return p2SizeInfo.areaWDMA.contentSize();
        }();

        //apply 4 align for MFLL
        StereoHAL::applyNAlign(4, outSize.w);
        StereoHAL::applyNAlign(4, outSize.h);
    }
    else
    {
        //Use preview to query since RRZO is for preview
        if(StereoSettingProvider::is3rdParty(eSTEREO_SCENARIO_PREVIEW)) {
            auto iter = __rrzoYuvSizes.find(sensor);
            if(iter != __rrzoYuvSizes.end())
            {
                outSize = iter->second;
            }
            else
            {
                Pass2SizeInfo p2SizeInfo;
                if(sensor < eSTEREO_SENSOR_MAIN3)
                {
                    ENUM_PASS2_ROUND queryRound = (eSTEREO_SENSOR_MAIN1 == sensor) ? PASS2A_CROP : PASS2A_P_CROP;
                    getPass2SizeInfo(queryRound, eSTEREO_SCENARIO_CAPTURE, p2SizeInfo);
                    outSize = p2SizeInfo.areaWDMA.contentSize();
                }
                else
                {
                    getcustomYUVSize(sensor, EPortIndex_IMGO, outSize);
                }

                outSize.w /= 2;
                outSize.h /= 2;
            }

            StereoHAL::applyNAlign(2, outSize.w);
            StereoHAL::applyNAlign(2, outSize.h);
        } else {
            MUINT32 junkStride;
            MRect   tgCropRect;

            getPass1Size(sensor, eImgFmt_FG_BAYER10, EPortIndex_RRZO, eSTEREO_SCENARIO_PREVIEW,
                         //below are outputs
                         tgCropRect, outSize, junkStride);
        }
    }

    return true;
}

MSize
StereoSizeProviderImp::thirdPartyDepthmapSize(ENUM_STEREO_RATIO ratio, ENUM_STEREO_SCENARIO senario) const
{
    StereoSensorConbinationSetting_T *sensorCombination = StereoSettingProviderKernel::getInstance()->getSensorCombinationSetting();
    MSize depthmapSize;
    int m, n;
    imageRatioMToN(ratio, m, n);
    if(eSTEREO_SCENARIO_PREVIEW == senario) {
        if(sensorCombination) {
            if(sensorCombination->depthmapSize.find(ratio) != sensorCombination->depthmapSize.end()) {
                depthmapSize = sensorCombination->depthmapSize[ratio];
            }
        }

        if(0 == depthmapSize.w ||
           0 == depthmapSize.h)
        {
            StereoArea area(480, 360);
            if(m * 3 / 4 >= n) {
                area.applyRatio(ratio, E_KEEP_WIDTH);
            } else {
                area.applyRatio(ratio, E_KEEP_HEIGHT);
            }
            depthmapSize = area.size;
        }

        MY_LOGD("3rd party preview depthmap size for %d:%d: %dx%d", m, n, depthmapSize.w, depthmapSize.h);
    } else {
        if(sensorCombination) {
            if(sensorCombination->depthmapSizeCapture.find(ratio) != sensorCombination->depthmapSizeCapture.end()) {
                depthmapSize = sensorCombination->depthmapSizeCapture[ratio];
            }
        }

        if(0 == depthmapSize.w ||
           0 == depthmapSize.h)
        {
            Pass2SizeInfo p2SizeInfo;
            getPass2SizeInfo(PASS2A_CROP, eSTEREO_SCENARIO_CAPTURE, p2SizeInfo);
            p2SizeInfo.areaWDMA /= 4;
            p2SizeInfo.areaWDMA.removePadding().apply2Align();
            depthmapSize = p2SizeInfo.areaWDMA.contentSize();
        }

        MY_LOGD("3rd party capture depthmap size for %d:%d: %dx%d", m, n, depthmapSize.w, depthmapSize.h);
    }

    return depthmapSize;
}

void
StereoSizeProviderImp::setPreviewSize(NSCam::MSize size)
{
    __previewSize = size;
}

bool
StereoSizeProviderImp::
getDualcamP2IMGOYuvCropResizeInfo(const int SENSOR_INDEX, NSCam::MRect &cropRect, MSize &targetSize)
{
    int sensorIndex[2] = {0};
    bool isOK = StereoSettingProvider::getStereoSensorIndex(sensorIndex[0], sensorIndex[1]);
    if(!isOK) {
        return false;
    }

    ENUM_PASS2_ROUND queryRound = PASS2A_CROP;
    ENUM_STEREO_SENSOR sensor;
    if(SENSOR_INDEX == sensorIndex[0]) {
        queryRound = PASS2A_CROP;
        sensor = eSTEREO_SENSOR_MAIN1;
    } else if(SENSOR_INDEX == sensorIndex[1]) {
        queryRound = PASS2A_P_CROP;
        sensor = eSTEREO_SENSOR_MAIN2;
    } else {
        MY_LOGE("Invalid sensor index %d. Available sensor indexes: %d, %d", SENSOR_INDEX, sensorIndex[0], sensorIndex[1]);
        return false;
    }

    //Get Crop
    Pass2SizeInfo p2SizeInfo;
    getPass2SizeInfo(queryRound, eSTEREO_SCENARIO_CAPTURE, p2SizeInfo);
    cropRect.s = p2SizeInfo.areaWDMA.contentSize();
    cropRect.p = p2SizeInfo.areaWDMA.startPt;

    //Get size
    getcustomYUVSize(sensor, EPortIndex_IMGO, targetSize);

    MY_LOGD_IF(__LOG_ENABLED, "Crop rect: (%d, %d) %dx%d, Size %dx%d",
                            cropRect.p.x, cropRect.p.y, cropRect.s.w, cropRect.s.h,
                            targetSize.w, targetSize.h);

    return true;
}

/*******************************************************************************
 *
 ********************************************************************************/
bool
StereoSizeProviderImp::__getCenterCrop(MSize &srcSize, MRect &rCrop )
{
    // calculate the required image hight according to image ratio
    int iHeight = srcSize.h;
    int m, n;
    imageRatioMToN(StereoSettingProvider::imageRatio(), m, n);
    if(eRatio_4_3 != StereoSettingProvider::imageRatio()) {
        iHeight = srcSize.w * n / m;
        applyNAlign(2, iHeight);
    }

    if(abs(iHeight-srcSize.h) == 0)
    {
        rCrop.p = MPoint(0,0);
        rCrop.s = srcSize;
    }
    else
    {
        rCrop.p.x = 0;
        rCrop.p.y = (srcSize.h - iHeight)/2;
        rCrop.s.w = srcSize.w;
        rCrop.s.h = iHeight;
    }

    MY_LOGD_IF(__LOG_ENABLED, "srcSize:(%d,%d) ratio:%d, rCropStartPt:(%d, %d) rCropSize:(%d,%d)",
                            srcSize.w, srcSize.h, StereoSettingProvider::imageRatio(),
                            rCrop.p.x, rCrop.p.y, rCrop.s.w, rCrop.s.h);

    // apply 16-align to height
    if(eRatio_4_3 != StereoSettingProvider::imageRatio()) {
        applyNAlign(16, rCrop.s.h);
        MY_LOGD_IF(__LOG_ENABLED, "srcSize after 16 align:(%d,%d) ratio:%d, rCropStartPt:(%d, %d) rCropSize:(%d,%d)",
                                srcSize.w, srcSize.h, StereoSettingProvider::imageRatio(),
                                rCrop.p.x, rCrop.p.y, rCrop.s.w, rCrop.s.h);
    }

    return MTRUE;
}

float
StereoSizeProviderImp::__getFOVCropRatio(ENUM_STEREO_SENSOR sensor)
{
    switch(sensor)
    {
        case eSTEREO_SENSOR_MAIN1:
            return StereoSettingProvider::getMain1FOVCropRatio();
        case eSTEREO_SENSOR_MAIN2:
            return StereoSettingProvider::getMain2FOVCropRatio();
        default:
            return 1.0f;
    }
};

void
StereoSizeProviderImp::__cropRectByFOVCropAndImageRatio(ENUM_STEREO_SENSOR sensor, ENUM_STEREO_RATIO imageRatio, MRect &rect)
{
    float CROP_RATIO = __getFOVCropRatio(sensor);
    int m, n;
    imageRatioMToN(imageRatio, m, n);
    if(CROP_RATIO < 1.0f &&
       CROP_RATIO > 0.0f)
    {
        CropUtil::cropRectByFOV(rect, CROP_RATIO, imageRatio);
        MY_LOGD_IF(__LOG_ENABLED, "Crop TG by FOV ratio %.2f, image ratio: %d:%d, result: tg (%d, %d) %dx%d",
                                CROP_RATIO, m, n, rect.p.x, rect.p.y, rect.s.w, rect.s.h);
    }

    CropUtil::cropRectByImageRatio(rect, imageRatio);
    MY_LOGD_IF(__LOG_ENABLED, "Crop TG by image ratio %d:%d, result: offset (%d, %d) size %dx%d",
                            m, n, rect.p.x, rect.p.y, rect.s.w, rect.s.h);
};