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
#define LOG_TAG "AIDEPTH_HAL"

#include "aidepth_hal_imp.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_VSDOF_HAL
CAM_ULOG_DECLARE_MODULE_ID(MOD_VSDOF_HAL);

#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <stereo_tuning_provider.h>
#include "../inc/stereo_dp_util.h"
#include <vsdof/hal/ProfileUtil.h>
#include <mtkcam/utils/LogicalCam/LogicalCamJSONUtil.h>
#include <fstream>
#include <iomanip>
#include <mtkcam/utils/TuningUtils/FileReadRule.h>    //For extract

using namespace StereoHAL;

//vendor.STEREO.log.hal.aidepth [0: disable] [1: enable]
#define LOG_PERPERTY    PROPERTY_ENABLE_LOG".hal.aidepth"

#define AIDEPTH_HAL_DEBUG

#ifdef AIDEPTH_HAL_DEBUG    // Enable debug log.

#if defined(__func__)
#undef __func__
#endif
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

#endif  // AIDEPTH_HAL_DEBUG

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define SINGLE_LINE_LOG 0
#if (1==SINGLE_LINE_LOG)
#define FAST_LOGD(fmt, arg...)  __fastLogger.FastLogD(fmt, ##arg)
#define FAST_LOGI(fmt, arg...)  __fastLogger.FastLogI(fmt, ##arg)
#else
#define FAST_LOGD(fmt, arg...)  __fastLogger.FastLogD("[%s]" fmt, __func__, ##arg)
#define FAST_LOGI(fmt, arg...)  __fastLogger.FastLogI("[%s]" fmt, __func__, ##arg)
#endif
#define FAST_LOGW(fmt, arg...)  __fastLogger.FastLogW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define FAST_LOGE(fmt, arg...)  __fastLogger.FastLogE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define FAST_LOG_PRINT  __fastLogger.print()

inline bool checkImageBuffer(IImageBuffer *image, const char *bufferName, int planeCountToCheck=1)
{
    if(image == nullptr) {
        MY_LOGE("%s is NULL", bufferName);
        return false;
    }

    if(planeCountToCheck > 0)
    {
        for(int p = 0; p < planeCountToCheck; ++p) {
            if(image->getBufVA(p) == 0) {
                MY_LOGE("%s, plane %d is NULL", bufferName, p);
                return false;
            }
        }
    }
    else
    {
        if(image->getImageBufferHeap() == nullptr ||
           image->getImageBufferHeap()->getHWBuffer() == nullptr)
        {
            MY_LOGE("%s, getHWBuffer is NULL", bufferName);
            return false;
        }
    }

    return true;
}

AIDEPTH_HAL *
AIDEPTH_HAL::createInstance()
{
    return new AIDEPTH_HAL_IMP();
}

AIDEPTH_HAL_IMP::AIDEPTH_HAL_IMP()
    : LOG_ENABLED( StereoSettingProvider::isLogEnabled(LOG_PERPERTY) )
    , __fastLogger(LOG_TAG, LOG_PERPERTY)
{
    __fastLogger.setSingleLineMode(SINGLE_LINE_LOG);

    AutoProfileUtil profile(LOG_TAG, "AIDepthHALInit");

    ::memset(&__initInfo,   0, sizeof(__initInfo));
    ::memset(&__tuningInfo, 0, sizeof(__tuningInfo));
    ::memset(&__paramInfo,  0, sizeof(__paramInfo));
    ::memset(&__imgInfo,    0, sizeof(__imgInfo));
    ::memset(&__resultInfo, 0, sizeof(__resultInfo));

    //Create AIDepth instance
    __pAIDepth = MTKAIDepth::createInstance(DRV_AIDEPTH_OBJ_SW);
    if(NULL == __pAIDepth) {
        MY_LOGE("Cannot create instance of AIDepth");
        return;
    }

    __mainCamPos = StereoSettingProvider::getSensorRelativePosition();
    __imgSize = StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y, eSTEREO_SCENARIO_CAPTURE);
    __init();
}

AIDEPTH_HAL_IMP::~AIDEPTH_HAL_IMP()
{
    MY_LOGD_IF(LOG_ENABLED, "+");
    if(__initInfo.WorkingBuffAddr) {
        delete [] __initInfo.WorkingBuffAddr;
        __initInfo.WorkingBuffAddr = NULL;
    }

    if(__pAIDepth) {
        __pAIDepth->AIDepthReset();
        __pAIDepth->destroyInstance(__pAIDepth);
        __pAIDepth = NULL;
    }

    if(NULL != __aidepthOutputImg.get()) {
        StereoDpUtil::freeImageBuffer(LOG_TAG, __aidepthOutputImg);
    }

    MY_LOGD_IF(LOG_ENABLED, "-");
}

bool
AIDEPTH_HAL_IMP::AIDepthHALRun(AIDEPTH_HAL_PARAMS &param, AIDEPTH_HAL_OUTPUT &output)
{
    AutoProfileUtil profile(LOG_TAG, "AIDepthHALRun");

    return (__setAIDepthParams(param) &&
            __runAIDepth(output));
}

void
AIDEPTH_HAL_IMP::__init()
{
    AutoProfileUtil profile(LOG_TAG, "AIDepth Init");
    //Init AIDepth
    //=== Init tuning info ===
    __initInfo.pTuningInfo = &__tuningInfo;

    __tuningInfo.imgWidth  = __imgSize.w;
    __tuningInfo.imgHeight = __imgSize.h;
    __tuningInfo.flipFlag  = __mainCamPos;
    __outputSize = StereoSizeProvider::getInstance()->getBufferSize(E_DL_DEPTHMAP, eSTEREO_SCENARIO_CAPTURE);

    if(NULL != __aidepthOutputImg.get()) {
        StereoDpUtil::freeImageBuffer(LOG_TAG, __aidepthOutputImg);
    }
    if(!StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, __imgSize, !IS_ALLOC_GB, __aidepthOutputImg)) {
        MY_LOGE("Fail to create temp image with size %dx%d", __imgSize.w, __imgSize.h);
    }

    __tuningInfo.imgFinalWidth = __outputSize.w;

    TUNING_PAIR_LIST_T n3dTuningParams;
    StereoTuningProvider::getN3DTuningInfo(eSTEREO_SCENARIO_CAPTURE, n3dTuningParams);
    for(auto &tuning : n3dTuningParams) {
        if(tuning.first == "n3d.c_offset") {
            __tuningInfo.convergenceOffset = tuning.second;
            break;
        }
    }

#ifdef AIDEPTH_CUSTOM_PARAM
    TUNING_PAIR_LIST_T tuningParamList;
    StereoTuningProvider::getAIDepthTuningInfo(__initInfo.pTuningInfo->CoreNumber, tuningParamList);

    std::vector<AIDepthCustomParam> tuningParams;
    for(auto &param : tuningParamList) {
        if(param.first == "dispGain") {
            __tuningInfo.dispGain = param.second;
        } else {
            tuningParams.push_back({(char *)param.first.c_str(), param.second});
        }
    }

    __initInfo.pTuningInfo->NumOfParam = tuningParams.size();
    __initInfo.pTuningInfo->params = &tuningParams[0];
#endif
    __logInitData();

    __pAIDepth->AIDepthReset();
    __pAIDepth->AIDepthInit((MUINT32 *)&__initInfo, NULL);
    //Get working buffer size
    __pAIDepth->AIDepthFeatureCtrl(AIDEPTH_FEATURE_GET_WORKBUF_SIZE, NULL, &__initInfo.WorkingBuffSize);

    //Allocate working buffer and set to AIDepth
    if(__initInfo.WorkingBuffSize > 0 &&
       __workingBufferSize != __initInfo.WorkingBuffSize)
    {
        if(__initInfo.WorkingBuffAddr) {
            delete [] __initInfo.WorkingBuffAddr;
            __initInfo.WorkingBuffAddr = NULL;
        }

        __initInfo.WorkingBuffAddr = new(std::nothrow) MUINT8[__initInfo.WorkingBuffSize];
        if(__initInfo.WorkingBuffAddr) {
            __workingBufferSize = __initInfo.WorkingBuffSize;
            MY_LOGD_IF(LOG_ENABLED, "Alloc %d bytes for AIDepth working buffer", __initInfo.WorkingBuffSize);
            __pAIDepth->AIDepthFeatureCtrl(AIDEPTH_FEATURE_SET_WORKBUF_ADDR, &__initInfo, 0);
        } else {
            MY_LOGE("Cannot create AIDepth working buffer of size %d", __initInfo.WorkingBuffSize);
        }
    }
}

bool
AIDEPTH_HAL_IMP::__setAIDepthParams(AIDEPTH_HAL_PARAMS &param)
{
    __dumpHint = param.dumpHint;

    //================================
    //  Set input data
    //================================
    if(    !checkImageBuffer(param.imageMain1, "imageMain1")
        || !checkImageBuffer(param.imageMain2, "imageMain2")
      )
    {
        return false;
    }

    if(param.mainCamPos != __mainCamPos ||
       param.imageMain1->getImgSize().w != __imgSize.w ||
       param.imageMain1->getImgSize().h != __imgSize.h)
    {
        MY_LOGD("Setting changes, re-init. MainCamPos: %d->%d, Size %dx%d -> %dx%d",
                __mainCamPos, param.mainCamPos,
                __imgSize.w, __imgSize.h,
                param.imageMain1->getImgSize().w, param.imageMain1->getImgSize().h);

        __mainCamPos = param.mainCamPos;
        __imgSize = param.imageMain1->getImgSize();
        __init();
    }

    //================================
    //  Set to AIDepth
    //================================
    {
        AutoProfileUtil profile(LOG_TAG, "AIDepth Set Proc");
        __paramInfo.ISOValue     = param.iso;
        __paramInfo.ExposureTime = param.exposureTime;
        if(__dumpHint) {
            genFileName_VSDOF_BUFFER(__dumpPrefix, DUMP_PATH_SIZE, __dumpHint, "");
            __paramInfo.prefix = __dumpPrefix;
        } else {
            __paramInfo.prefix = NULL;
        }
        __pAIDepth->AIDepthFeatureCtrl(AIDEPTH_FEATURE_SET_PARAM, &__paramInfo, NULL);
    }

    {
        AutoProfileUtil profile(LOG_TAG, "AIDepth Add Img");
        __imgInfo.Width      = param.imageMain1->getImgSize().w;
        __imgInfo.Height     = param.imageMain1->getImgSize().h;
        __imgInfo.Stride     = param.imageMain1->getBufStridesInBytes(0);
        __imgInfo.ImgLAddr   = (MUINT8*)param.imageMain1->getBufVA(0);
        __imgInfo.ImgRAddr   = (MUINT8*)param.imageMain2->getBufVA(0);

        __pAIDepth->AIDepthFeatureCtrl(AIDEPTH_FEATURE_ADD_IMGS, &__imgInfo, NULL);
    }

    __logSetProcData();

    return true;
}

bool
AIDEPTH_HAL_IMP::__runAIDepth(AIDEPTH_HAL_OUTPUT &output)
{
    //================================
    //  Run AIDepth
    //================================
    {
        AutoProfileUtil profile(LOG_TAG, "Run AIDepth Main");
        __pAIDepth->AIDepthMain();
    }

    //================================
    //  Get result
    //================================
    {
        AutoProfileUtil profile(LOG_TAG, "AIDepth get result");
        ::memset(&__resultInfo, 0, sizeof(__resultInfo));
        __pAIDepth->AIDepthFeatureCtrl(AIDEPTH_FEATURE_GET_RESULT, NULL, &__resultInfo);
    }

    __logAIDepthResult();

    if(output.depthMap)
    {
        if((MUINT32)__aidepthOutputImg->getImgSize().w == __resultInfo.DepthImageWidth &&
           (MUINT32)__aidepthOutputImg->getImgSize().h == __resultInfo.DepthImageHeight)
        {
            AutoProfileUtil profile(LOG_TAG, "Copy result to buffer");
            MY_LOGD_IF(LOG_ENABLED, "Copy Depthmap");
            ::memcpy((void*)__aidepthOutputImg->getBufVA(0), __resultInfo.DepthImageAddr,
                     __resultInfo.DepthImageWidth * __resultInfo.DepthImageHeight);

            if(__dumpHint) {
                char dumpPath[DUMP_PATH_SIZE];
                extract(__dumpHint, __aidepthOutputImg.get());
                genFileName_YUV(dumpPath, DUMP_PATH_SIZE, __dumpHint, TuningUtils::YUV_PORT_UNDEFINED, "AIDEPTH_OUT_TEMP");
                __aidepthOutputImg->saveToFile(dumpPath);
            }

            //Use MDP to resize
            int rotation = 360 - StereoSettingProvider::getModuleRotation();
            if(rotation == 360) {
                rotation = 0;
            }

            if(!StereoDpUtil::transformImage(__aidepthOutputImg.get(), output.depthMap, static_cast<ENUM_ROTATION>(rotation))) {
                MY_LOGE("Fail to resize depthmap");
                return false;
            }
            #if 0
            else if(__dumpHint)
            {
                char dumpPath[DUMP_PATH_SIZE];
                extract(__dumpHint, output.depthMap);
                genFileName_YUV(dumpPath, DUMP_PATH_SIZE, __dumpHint, TuningUtils::YUV_PORT_UNDEFINED, "AIDEPTH_OUT_DEPTHMAP");
                output.depthMap->saveToFile(dumpPath);
            }
            #endif
        } else {
            MY_LOGE("Depthmap size mismatch: dst %dx%d, src %dx%d",
                    __aidepthOutputImg->getImgSize().w, __aidepthOutputImg->getImgSize().h,
                    __resultInfo.DepthImageWidth, __resultInfo.DepthImageHeight);
            return false;
        }
    } else {
        MY_LOGE("Output depthmap is NULL");
        return false;
    }

    return true;
}

void
AIDEPTH_HAL_IMP::__logInitData()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= AIDepth Init Info =========");
    FAST_LOGD("[TuningInfo.CoreNumber]          %d", __tuningInfo.CoreNumber);
    FAST_LOGD("[TuningInfo.imgWidth]            %d", __tuningInfo.imgWidth);
    FAST_LOGD("[TuningInfo.imgHeight]           %d", __tuningInfo.imgHeight);
    FAST_LOGD("[TuningInfo.flipFlag]            %d", __tuningInfo.flipFlag);
    FAST_LOGD("[TuningInfo.warpFlag             %d", __tuningInfo.warpFlag);
    FAST_LOGD("[TuningInfo.imgFinalWidth        %d", __tuningInfo.imgFinalWidth);
    FAST_LOGD("[TuningInfo.imgFinalHeight       %d", __outputSize.h);
    FAST_LOGD("[TuningInfo.dispGain             %d", __tuningInfo.dispGain);
    FAST_LOGD("[TuningInfo.convergenceOffset    %d", __tuningInfo.convergenceOffset);

#ifdef AIDEPTH_CUSTOM_PARAM
    FAST_LOGD("[TuningInfo.NumOfParam]          %d", __tuningInfo.NumOfParam);
    for(MUINT32 j = 0; j < __tuningInfo.NumOfParam; ++j) {
    FAST_LOGD("[TuningInfo.params][% 2d]          %s: %d", j,
                  __tuningInfo.params[j].key,
                  __tuningInfo.params[j].value);
    }
#endif

    FAST_LOG_PRINT;
}

void
AIDEPTH_HAL_IMP::__logSetProcData()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= AIDepth Param Info =========");
    FAST_LOGD("[ISO]                      %d", __paramInfo.ISOValue);
    FAST_LOGD("[ExposureTime]             %d", __paramInfo.ExposureTime);
    FAST_LOGD("[Prefix]                   %s", __paramInfo.prefix);

    FAST_LOGD("========= AIDepth Img Info =========");
    FAST_LOGD("[Width]                    %d", __imgInfo.Width);
    FAST_LOGD("[Height]                   %d", __imgInfo.Height);
    FAST_LOGD("[Stide]                    %d", __imgInfo.Stride);
    FAST_LOGD("[ImgLAddr]                 %p", __imgInfo.ImgLAddr);
    FAST_LOGD("[ImgRAddr]                 %p", __imgInfo.ImgRAddr);

    FAST_LOG_PRINT;
}

void
AIDEPTH_HAL_IMP::__logAIDepthResult()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= AIDepth Output Info =========");
    FAST_LOGD("[Return code]            %d", __resultInfo.RetCode);
    FAST_LOGD("[DepthImageWidth]        %d", __resultInfo.DepthImageWidth);
    FAST_LOGD("[DepthImageHeight]       %d", __resultInfo.DepthImageHeight);
    FAST_LOGD("[DepthImageAddr]         %p", __resultInfo.DepthImageAddr);

    FAST_LOG_PRINT;
}

void
AIDEPTH_HAL_IMP::__dumpInitData()
{
    if(NULL == __dumpHint) {
        return;
    }

    char dumpPath[DUMP_PATH_SIZE];
    genFileName_VSDOF_BUFFER(dumpPath, DUMP_PATH_SIZE, __dumpHint, "AIDEPTH_INIT_INFO.json");
    json initInfoJson;

    initInfoJson["TuningInfo"]["CoreNumber"]        = __tuningInfo.CoreNumber;
    initInfoJson["TuningInfo"]["imgWidth"]          = __tuningInfo.imgWidth;
    initInfoJson["TuningInfo"]["imgHeight"]         = __tuningInfo.imgHeight;
    initInfoJson["TuningInfo"]["flipFlag"]          = __tuningInfo.flipFlag;
    initInfoJson["TuningInfo"]["warpFlag"]          = __tuningInfo.warpFlag;
    initInfoJson["TuningInfo"]["imgFinalWidth"]     = __tuningInfo.imgFinalWidth;
    initInfoJson["TuningInfo"]["imgFinalHeight"]    = __outputSize.h;
    initInfoJson["TuningInfo"]["dispGain"]          = __tuningInfo.dispGain;
    initInfoJson["TuningInfo"]["convergenceOffset"] = __tuningInfo.convergenceOffset;
    initInfoJson["TuningInfo"]["NumOfParam"]        = __tuningInfo.NumOfParam;
    for(MUINT32 i = 0; i < __tuningInfo.NumOfParam; ++i) {
        if(__tuningInfo.params[i].key) {
            initInfoJson["TuningInfo"]["params"].push_back({__tuningInfo.params[i].key, __tuningInfo.params[i].value});
        }
    }

    std::ofstream of(dumpPath);
    of << std::setw(4) << initInfoJson;
}

void
AIDEPTH_HAL_IMP::__dumpSetProcData()
{
    if(NULL == __dumpHint) {
        return;
    }

    char dumpPath[DUMP_PATH_SIZE];
    genFileName_VSDOF_BUFFER(dumpPath, DUMP_PATH_SIZE, __dumpHint, "AIDEPTH_PROC_INFO.json");
    json procInfoJson;

    procInfoJson["ParamInfo"]["ISOValue"]     = __paramInfo.ISOValue;
    procInfoJson["ParamInfo"]["ExposureTime"] = __paramInfo.ExposureTime;
    procInfoJson["ParamInfo"]["prefix"]       = __paramInfo.prefix;

    procInfoJson["ImgInfo"]["Width"]    = __imgInfo.Width;
    procInfoJson["ImgInfo"]["Height"]   = __imgInfo.Height;
    procInfoJson["ImgInfo"]["Stride"]   = __imgInfo.Stride;

    std::ofstream of(dumpPath);
    of << std::setw(4) << procInfoJson;
}