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
#ifndef _GF_HAL_IMP_H_
#define _GF_HAL_IMP_H_

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "GF_HAL"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <libgf/MTKGF.h>
#pragma GCC diagnostic pop

#include <gf_hal.h>
#include <mtkcam/aaa/aaa_hal_common.h> // For DAF_TBL_STRUCT
#include <vector>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <deque>
#include <mtkcam/feature/stereo/hal/FastLogger.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/feature/stereo/hal/stereo_fd_proxy.h>
#include <vsdof/hal/ProfileUtil.h>

using namespace NS3Av3;
using namespace android;
using namespace StereoHAL;
using namespace NSCam;
using namespace std;

//NOTICE: property has 31 characters limitation
//vendor.STEREO.log.hal.gf [0: disable] [1: enable]
#define LOG_PERPERTY  PROPERTY_ENABLE_LOG".hal.gf"

enum ENUM_GF_TRIGGER_TIMING
{
    E_TRIGGER_BEFORE_AND_AFTER_AF,
    E_TRIGGER_AFTER_AF,
};

class GF_HAL_IMP : public GF_HAL
{
public:
    GF_HAL_IMP(ENUM_STEREO_SCENARIO eScenario, bool outputDepthmap=false);
    virtual ~GF_HAL_IMP();

    virtual bool GFHALRun(GF_HAL_IN_DATA &inData, GF_HAL_OUT_DATA &outData);
protected:

private:
    void _setGFParams(GF_HAL_IN_DATA &gfHalParam);
    void _runGF(GF_HAL_OUT_DATA &gfHalOutput);

    void _initAFWinTransform();
    bool _validateAFPoint(MPoint &ptAF);
    MPoint &_rotateAFPoint(MPoint &ptAF);
    MPoint _AFToGFPoint(const MPoint &ptAF, ENUM_STEREO_SCENARIO scenario);
    MRect _getAFRect(const int AF_INDEX, ENUM_STEREO_SCENARIO scenario);
    MPoint _getTouchPoint(MPoint ptIn);
    void _updateDACInfo(const int AF_INDEX, GFDacInfo &dacInfo);
    void _updateFDInfo(const int AF_INDEX, ENUM_STEREO_SCENARIO scenario, GFFdInfo &fdInfo);
    void _removePadding(IImageBuffer *src, IImageBuffer *dst);

    void _clearTransformedImages();
    bool _rotateResult(GFBufferInfo &gfResult, IImageBuffer *targetBuffer);

    MPoint __faceRectToTriggerPoint(MRect &rect);

    //Log
    void __logInitData();
    void __logSetProcData();
    void __logGFResult();
    void __logGFBufferInfo(const GFBufferInfo &buf, int index = -1);

    //Dump
    void __dumpInitInfo();
    void __dumpProcInfo();
    void __dumpWorkingBuffer();
private:
    MTKGF           *m_pGfDrv;
    bool            __outputDepthmap = false;
    bool            m_isFirstFrame = true;  //Assumption: GF HAL will be re-created for VR mode

    MINT32          __magicNumber;
    MINT32          __requestNumber;
    ENUM_STEREO_SCENARIO __eScenario;

    GFInitInfo      m_initInfo;
    GFProcInfo      m_procInfo;
    GFResultInfo    m_resultInfo;

    const bool      LOG_ENABLED;

    DAF_TBL_STRUCT          *m_pAFTable      = NULL;
    bool                     m_isAFSupported = false;
    static MPoint            s_lastGFPoint;
    static bool              s_wasAFTrigger;
    //index 0: preview/vr; 1: capture/zsd
    float                    m_afScaleW[2];
    float                    m_afScaleH[2];
    int                      m_afOffsetX[2];
    int                      m_afOffsetY[2];
    float                    m_tgWidth[2];

    std::vector<sp<IImageBuffer>>    m_transformedImages;

    sp<IImageBuffer>    __confidenceMap;

    bool            m_triggerAfterCapture = false;

    const MSize DMH_SIZE;
    const MSize DMBG_SIZE;
    const int DMBG_IMG_SIZE;

    FastLogger              __fastLogger;

    int                     __triggerTiming = checkStereoProperty("vendor.STEREO.gf_trigger_timing", E_TRIGGER_BEFORE_AND_AFTER_AF);

    //FF + FD: Should consider face move in plane(x, y), in depth(z) and in time(t)
    //Consider move in plane(x, y)
    const float             __FD_PLANE_MOVE_THRESHOLD = checkStereoProperty("vendor.STEREO.fd_plane_threshold", 10)/100.0f; //unit: %

    //Consider move in depth(z)
    const float             __FD_DEPTH_MOVE_THRESHOLD = checkStereoProperty("vendor.STEREO.fd_depth_threshold", 3)+100.0f; //unit: %

    //Consider time(t)
    const size_t            __FD_TRIGGER_FRAME_COUNT = checkStereoProperty("vendor.STEREO.fd_trigger_count", 5);
    const size_t            __FD_DELAY_COUNT         = checkStereoProperty("vendor.STEREO.fd_delay_count", 90);

    //How many frames delay to trigger by FD after touch trigger
    size_t                  __fdDelayCount           = __FD_DELAY_COUNT;

    //if __fdFrameCount >= __FD_TRIGGER_FRAME_COUNT, trigger GF
    //reset when: 1. no face detected
    //            2. Touch happend
    size_t                  __fdFrameCount           = 0;

    MRect                   __latestFace;   //if size = 0x0, no face

    MPoint                  __lastTouchPoint = MPoint(-1, -1);
    MUINT                   __captureSensorScenario;

    //For FF
    const float             __COFFSET_THRESHOLD = (float)checkStereoProperty("vendor.STEREO.gf_conv_th", 1);
    float                   __lastCOffset = 0.0f;

    //Dump
    TuningUtils::FILE_DUMP_NAMING_HINT *__dumpHint = NULL;
    const static size_t     __DUMP_BUFFER_SIZE = 512;
    size_t                  __initDumpSize = 0;
    char                    __dumpInitData[__DUMP_BUFFER_SIZE];
    char                    __dumpProcData[__DUMP_BUFFER_SIZE];
};

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define GF_HAL_DEBUG
#ifdef GF_HAL_DEBUG    // Enable debug log.
    #define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
    #define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
    #define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
    #define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)
#else   // Disable debug log.
    #define MY_LOGD(a,...)
    #define MY_LOGI(a,...)
    #define MY_LOGW(a,...)
    #define MY_LOGE(a,...)
#endif  // GF_HAL_DEBUG

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
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

#endif