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
#ifndef N3D_HAL_KERNEL_H_
#define N3D_HAL_KERNEL_H_

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "N3D_HAL_KERNEL"

#include <bitset>
#include <thread>
#include <n3d_hal.h>

#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <camera_custom_nvram.h>
#include <vector>
#include <cutils/atomic.h>
#include <MTKStereoKernel.h>
#include <mtkcam/feature/stereo/hal/FastLogger.h>
#include <mtkcam/utils/std/Log.h>
#include <vsdof/hal/ProfileUtil.h>
#include <mtkcam/aaa/aaa_hal_common.h>

using namespace android;
using namespace NS3Av3;
using namespace StereoHAL;
using namespace NSCam;
using namespace std;

#define PROPERTY_SKIP_N3D       STEREO_PROPERTY_PREFIX"skip_n3d"
#define PROPERTY_DISABLE_GPU    STEREO_PROPERTY_PREFIX"disable_gpu"
#define PROPERTY_ALGO_BEBUG     STEREO_PROPERTY_PREFIX"dbgdump"

//NOTICE: property has 31 characters limitation
//debug.STEREO.log.hal.n3d [0: disable] [1: enable]
#define LOG_PERPERTY  PROPERTY_ENABLE_LOG".hal.n3d"

#define N3D_HAL_DEBUG 1
#define SINGLE_LINE_LOG 0

enum ENUM_N3D_RUM_STAGE
{
    E_WARPING_STAGE,
    E_LEARNING_STAGE
};

#define MAX_FEFM_RING_BUFFER_SIZE 1
struct FEFM_BUFFER_T
{
    MUINT8 *addr = NULL;
    size_t size  = 0;    //bytes
};

struct FEFM_RING_BUFFER_T : FEFM_BUFFER_T
{
    size_t totalLevel = 0;
    FEFM_BUFFER_T feMain1[MAX_GEO_LEVEL];
    FEFM_BUFFER_T feMain2[MAX_GEO_LEVEL];
    FEFM_BUFFER_T fmLR[MAX_GEO_LEVEL];
    FEFM_BUFFER_T fmRL[MAX_GEO_LEVEL];

    FEFM_RING_BUFFER_T() {}
    FEFM_RING_BUFFER_T(HWFEFM_DATA &data)
    {
        setHWFEFMData(data);
    }

    void setHWFEFMData(HWFEFM_DATA &data);
};

struct N3D_HAL_KERNEL_INIT_PARAM
{
    ENUM_STEREO_SCENARIO    eScenario = eSTEREO_SCENARIO_UNKNOWN;
    MUINT8                  fefmRound = 2;

    //The ring buffers for GPU warping of main2
    std::vector<NSCam::IImageBuffer *> inputImageBuffers;
    std::vector<NSCam::IImageBuffer *> outputImageBuffers;
    std::vector<NSCam::IImageBuffer *> outputMaskBuffers;

    N3D_HAL_KERNEL_INIT_PARAM()
    {
    }

    N3D_HAL_KERNEL_INIT_PARAM(N3D_HAL_INIT_PARAM &initParam)
    {
        eScenario          = initParam.eScenario;
        fefmRound          = initParam.fefmRound;
        inputImageBuffers  = initParam.inputImageBuffers;
        outputImageBuffers = initParam.outputImageBuffers;
        outputMaskBuffers  = initParam.outputMaskBuffers;
    }
};

class N3D_HAL_KERNEL
{
public:
    N3D_HAL_KERNEL(N3D_HAL_KERNEL_INIT_PARAM n3dInitParam);

    virtual ~N3D_HAL_KERNEL();

    virtual bool initN3D(STEREO_KERNEL_SET_ENV_INFO_STRUCT &n3dInitInfo, NVRAM_CAMERA_GEOMETRY_STRUCT *nvram, float *LDC);

    virtual bool initModel();

    virtual void initMain1Mask(StereoArea maskArea);

    virtual bool WarpMain1(IImageBuffer *main1Input, IImageBuffer *main1Output, IImageBuffer *main1Mask);

    virtual bool WarpMain2(N3D_HAL_PARAM &n3dParams, STEREO_KERNEL_AF_INFO_STRUCT *afInfoMain1, STEREO_KERNEL_AF_INFO_STRUCT *afInfoMain2,
                           N3D_HAL_OUTPUT &n3dOutput);

    virtual bool WarpMain2(N3D_HAL_PARAM_CAPTURE &n3dParams, STEREO_KERNEL_AF_INFO_STRUCT *afInfoMain1, STEREO_KERNEL_AF_INFO_STRUCT *afInfoMain2,
                           N3D_HAL_OUTPUT_CAPTURE &n3dOutput, STEREO_KERNEL_RESULT_STRUCT &algoResult);

    virtual bool setNVRAM(const NVRAM_CAMERA_GEOMETRY_STRUCT *nvram);

    virtual bool updateNVRAM(NVRAM_CAMERA_GEOMETRY_STRUCT *nvram);

    virtual MUINT32 getCalibrationOffsetInNVRAM();

    //Log
    virtual void logN3DInitInfo();
    virtual void logN3DProcInfo();
    virtual void logN3DResult(STEREO_KERNEL_RESULT_STRUCT &result);

    //Dump
    virtual void setDumpConfig(bool dump, TuningUtils::FILE_DUMP_NAMING_HINT *dumpHint=NULL);
    virtual void dumpN3DInitConfig();
    virtual void dumpN3DRuntimeConfig();

private:
    bool __initGraphicBuffers(N3D_HAL_KERNEL_INIT_PARAM &n3dInitParam);
    bool __initN3DKernel(STEREO_KERNEL_SET_ENV_INFO_STRUCT &n3dInitInfo, NVRAM_CAMERA_GEOMETRY_STRUCT *nvram, float *LDC);
    void __initMain1Mask();

    bool __isMain1AF()
    {
        return (__algoInitInfo.system_cfg & (1<<9));
    }

    bool __isMain2AF()
    {
        return (__algoInitInfo.system_cfg & (1<<10));
    }

    bool __setLensInfo(float *LDC);
    void __initWorkingBuffer(const size_t BUFFER_SIZE);
    bool __doStereoKernelInit();

    void __setN3DCommonParams(N3D_HAL_PARAM_COMMON &n3dParams, N3D_HAL_OUTPUT &n3dOutput);
    void __setN3DParams(N3D_HAL_PARAM &n3dParams, N3D_HAL_OUTPUT &n3dOutput);
    void __setN3DCaptureParams(N3D_HAL_PARAM_CAPTURE &n3dParams, N3D_HAL_OUTPUT_CAPTURE &n3dOutput);

    bool __runN3DCommon(int stage);
    bool __runN3DCapture();
    void __runN3DLearning();    //run in __learningThread
    void __runN3DLearningThread();    //run in __learningThread

    void __updateSceneInfo(N3D_HAL_OUTPUT &n3dOutput);

    //For new FEFM flow
    void __createFEFMRingBuffer();
    void __setFEFMData(HWFEFM_DATA &data);

    //n3d_hal_kernel.log.cpp
    void __logInitInfo(const STEREO_KERNEL_SET_ENV_INFO_STRUCT &initInfo);
    void __logImgInfo(const char *prefix, const STEREO_KERNEL_IMG_INFO_STRUCT &imgInfo);
    void __logRemapInfo(const char *prefix, const STEREO_KERNEL_FLOW_INFO_STRUCT &remapInfo);
    void __logAFInitInfo(const char *prefix, const STEREO_KERNEL_AF_INIT_STRUCT &afInitInfo);
    void __logTuningInfo(const char *prefix, const STEREO_KERNEL_TUNING_PARA_STRUCT &tuningInfo);
    void __logSetProcInfo(const char *prefix, const STEREO_KERNEL_SET_PROC_INFO_STRUCT &setprocInfo);
    void __logResult(const char *prefix, const STEREO_KERNEL_RESULT_STRUCT &n3dResult);

    std::string __getN3DLogPath();
    void __dumpN3DLog(std::string dumpPath);

private:
    static Mutex            __sLock; //static member is mutable, so we no need to add mutable
    static Mutex            __sLogLock;

    const bool              __LOG_ENABLED = StereoSettingProvider::isLogEnabled(LOG_PERPERTY);
    bool                    __DISABLE_GPU = (checkStereoProperty(PROPERTY_DISABLE_GPU) == 1);

    const bool              __RUN_N3D = (checkStereoProperty(PROPERTY_SKIP_N3D) != 1);
    ENUM_STEREO_SCENARIO    __eScenario = eSTEREO_SCENARIO_UNKNOWN;

    bool                    __isInit = false;

    sp<IImageBuffer>         __workBufImage;        // Working Buffer

    MTKStereoKernel         *__pStereoDrv = NULL;
    N3D_HAL_KERNEL_INIT_PARAM              __n3dInitParam;
    STEREO_KERNEL_SET_ENV_INFO_STRUCT      __algoInitInfo;
    STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT __algoWorkBufInfo;
    STEREO_KERNEL_SET_PROC_INFO_STRUCT     __algoProcInfo;
    STEREO_KERNEL_TUNING_PARA_STRUCT       __algoTuningInfo;
    STEREO_KERNEL_RESULT_STRUCT            __algoResult;   //Rectify and verifications

    MUINT8                  *__main1Mask = NULL;
    StereoArea              __areaMask;
    MUINT8                  *__inputMain2CPU = NULL;   //For CPU capture

    FastLogger              __fastLogger;

    bool                    __hasFEFM = false;
    thread                  __learningThread;
    sp<IImageBuffer>        __fefmRingBufferImage;
    FEFM_RING_BUFFER_T      __fefmRingBuffer[MAX_FEFM_RING_BUFFER_SIZE];    //addr point to va of __fefmRingBufferImage
    MUINT32                 __fefmRingBufferIndex = 0;  //last write index
    MUINT32                 __learningIndex = 0;    //last used index
    std::mutex              __learningMutex;
    std::condition_variable __learningCondVar;
    bool                    __wakeupLearning = false;

    bool                    __isDump = false;
    TuningUtils::FILE_DUMP_NAMING_HINT *__dumpHint = NULL;
    std::string             __initInfoJSON;
};

#if (1==N3D_HAL_DEBUG)    // Enable debug log.
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#ifndef GTEST
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)
#else
#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)
#endif

#else   // Disable debug log.
#define MY_LOGD(a,...)
#define MY_LOGI(a,...)
#define MY_LOGW(a,...)
#define MY_LOGE(a,...)

#endif  // N3D_HAL_DEBUG

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define CHECK_BUFFER(ptr, ptr_name)  MY_LOGE_IF((NULL == (void *)ptr), "%s is NULL", ptr_name)

#endif  // N3D_HAL_KERNEL_H_