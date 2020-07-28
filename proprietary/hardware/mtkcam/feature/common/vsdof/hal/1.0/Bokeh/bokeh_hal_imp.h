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
#ifndef BOKEH_HAL_IMP_H_
#define BOKEH_HAL_IMP_H_

#include <bokeh_hal.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <librefocus/MTKRefocus.h>
#pragma GCC diagnostic pop
#include <thread>
#include <mtkcam/feature/stereo/hal/FastLogger.h>
#include <rapidjson/document.h>     // rapidjson's DOM-style API

#include <mtkcam/feature/abf/AbfAdapter.h>
#include <stereo_tuning_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

using namespace StereoHAL;
using namespace std;
using namespace NSCam;
using namespace android;
using namespace rapidjson;

class BOKEH_HAL_IMP : public BOKEH_HAL
{
public:
    BOKEH_HAL_IMP(BOKEH_HAL_INIT_PARAMS *initParams = NULL);
    virtual void destroyInstance();
    virtual ~BOKEH_HAL_IMP();

    virtual bool Run(BOKEH_HAL_PARAMS &param, BOKEH_HAL_OUTPUT &output);
protected:

private:
    void    __waitInitThread();
    void    __initBokeh(BOKEH_HAL_INIT_PARAMS *initParams);
    void    __initTuningInfo(BOKEH_HAL_INIT_PARAMS *initParams);
    void    __createWorkingBuffer();
    void    __destroyWorkingBuffer();
    void    __setImage(BOKEH_HAL_PARAMS &param);
    bool    __parseExtraData(char *extraData);
    bool    __isNeededToRunABF(MUINT32 iso);
    void    __runABF(MUINT32 iso, IImageBuffer *image);
    void    __run(BOKEH_HAL_PARAMS &param, BOKEH_HAL_OUTPUT &output);

    //log.cpp
    void    __logInitData();
    void    __logExtraData(char *extraData);
    void    __logImageData();
    void    __logResult();

    //dump.cpp
    void    __dumpInitData();
    void    __dumpExtraData(Document &document);
    void    __dumpImageData(BOKEH_HAL_PARAMS &param);
    void    __dumpResult(BOKEH_HAL_OUTPUT &output);

    char    *__getDumpFolderName(int folderNumber, char path[]);
    void    __mkdir();  //make output dir for debug
private:
    MTKRefocus                  *__pDrv              = NULL;

    thread                      __initThread;
    RefocusInitInfo             __initInfo;
    RefocusTuningInfo           __tuningInfo;
    TUNING_PAIR_LIST_T          __tuningParamList;
    vector<RefocusTuningParam>  __tuningParams;
    vector<int>                 __clearTable;
    RefocusImageInfo            __imgInfo;
    RefocusResultInfo           __resultInfo;
    MUINT8                      *__pWorkingBuf       = NULL;
    MSize                       __viewSize[2];      //0 for 0/180, 1 for 90/270
    ENUM_ROTATION               __capOrientation = eRotate_0;
    ENUM_ROTATION               __depthRotation  = eRotate_0;
    sp<IImageBuffer>            __workingImage; //Store clean image and bokeh image

    const bool                  LOG_ENABLED;
    const bool                  DUMP_ENABLED;
    int                         __requestNumber      = 0;

    const MSize                 DEPTH_SIZE;

    FastLogger                  __fastLogger;

    AbfAdapter*                 __abfAdapter        = NULL;
    bool                        __abfEnabled        = true; //can be turned off by tuning params
    bool                        __needABF           = false;
    std::thread                 __convertABFResultThread;

    //Cached setting provider values
    ENUM_STEREO_SENSOR_PROFILE  __stereoProfile = StereoSettingProvider::stereoProfile();
    ENUM_ROTATION               __moduleRotation = StereoSettingProvider::getModuleRotation(__stereoProfile);
    MSize                       __captureSize = StereoSizeProvider::getInstance()->captureImageSize();
};

//NOTICE: property has 31 characters limitation
#define LOG_PERPERTY    PROPERTY_ENABLE_LOG".hal.bokeh"     //debug.STEREO.log.hal.bokeh
#define DUMP_PERPERTY   PROPERTY_ENABLE_DUMP".hal.bokeh"    //debug.STEREO.dump.hal.bokeh

#define BOKEH_HAL_DEBUG

#ifdef BOKEH_HAL_DEBUG    // Enable debug log.

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

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

#else   // Disable debug log.
#define MY_LOGD(a,...)
#define MY_LOGI(a,...)
#define MY_LOGW(a,...)
#define MY_LOGE(a,...)

#define FAST_LOGD(a,...)
#define FAST_LOGI(a,...)
#define FAST_LOGW(a,...)
#define FAST_LOGE(a,...)

#endif  // BOKEH_HAL_DEBUG

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#endif