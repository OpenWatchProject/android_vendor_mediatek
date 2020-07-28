/*********************************************************************************************
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
#include <stdlib.h>

#include "n3d_hal_imp.h"         // For N3D_HAL class.
#include "../inc/stereo_dp_util.h"

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/INvBufUtil.h>
#include <af_param.h>          // For AF_ROI_SEL_FD
#include <camera_custom_stereo.h>   // For CUST_STEREO_* definitions.
#include <math.h>
#include <mtkcam/drv/mem/cam_cal_drv.h> //For CAM_CAL_DATA_STRUCT

#include <vsdof/hal/rapidjson/writer.h>
#include <vsdof/hal/rapidjson/stringbuffer.h>
#include <vsdof/hal/rapidjson/document.h>     // rapidjson's DOM-style API
#include <vsdof/hal/rapidjson/prettywriter.h> // for stringify JSON
#include <vsdof/hal/rapidjson/filewritestream.h>
#include <vsdof/hal/rapidjson/writer.h>
#include <vsdof/hal/extradata_def.h>

#include "MaskSpliter.h"

#include <mtkcam/utils/std/Log.h>
#include <vsdof/hal/ProfileUtil.h>
#include <vsdof/hal/AffinityUtil.h>
#include <vsdof/hal/json_util.h>
#include <stereo_crop_util.h>
#include <swfefm_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_fd_proxy.h>
#include <algorithm>

#define N3D_HAL_DEBUG

#ifdef N3D_HAL_DEBUG    // Enable debug log.
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

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

using namespace rapidjson;
using android::Mutex;           // For android::Mutex in stereo_hal.h.

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define PROPERTY_ENABLE_VERIFY  STEREO_PROPERTY_PREFIX"enable_verify"
#define PROPERTY_ENABLE_CC      STEREO_PROPERTY_PREFIX"enable_cc"
#define PROPERTY_DISABLE_GPU    STEREO_PROPERTY_PREFIX"disable_gpu"
#define PROPERTY_ALGO_BEBUG     STEREO_PROPERTY_PREFIX"dbgdump"
#define PROPERTY_DUMP_NVRAM     STEREO_PROPERTY_PREFIX"dump_nvram"
#define PROPERTY_DUMP_OUTPUT    STEREO_PROPERTY_PREFIX"dump_n3d"
#define PROPERTY_DUMP_CAPTURE   STEREO_PROPERTY_PREFIX"dump_n3d_capture"
#define PROPERTY_SKIP_N3D       STEREO_PROPERTY_PREFIX"skip_n3d"
#define PROPERTY_DUMP_JSON      STEREO_PROPERTY_PREFIX"dump_json"

const MUINT32   NOT_QUERY      = 0xFFFFFFFF;
const MFLOAT    RRZ_CAPIBILITY = 0.25f;

const bool DUMP_INPUT_NVRAM  = true;
const bool DUMP_OUTPUT_NVRAM = !DUMP_INPUT_NVRAM;
/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
Mutex N3D_HAL_IMP::mLock;
Mutex N3D_HAL_IMP::mLogLock;
Mutex N3D_HAL_IMP::mRunLock;
NVRAM_CAMERA_GEOMETRY_STRUCT* N3D_HAL_IMP::m_pVoidGeoData = NULL;
MTKStereoKernel *N3D_HAL_IMP::__previewStereoDrv = NULL;
MTKStereoKernel *N3D_HAL_IMP::__captureStereoDrv = NULL;

/**************************************************************************
 *       Public Functions                                                 *
 **************************************************************************/

N3D_HAL *
N3D_HAL::createInstance(N3D_HAL_INIT_PARAM &n3dInitParam)
{
    return new N3D_HAL_IMP(n3dInitParam);
}

N3D_HAL_IMP::N3D_HAL_IMP(N3D_HAL_INIT_PARAM &n3dInitParam)
    : LOG_ENABLED(StereoSettingProvider::isLogEnabled(LOG_PERPERTY))
    , DISABLE_GPU(checkStereoProperty(PROPERTY_DISABLE_GPU) == 1)
    , DUMP_START(checkStereoProperty("depthmap.pipe.dump.start", -1))
    , RUN_N3D(checkStereoProperty(PROPERTY_SKIP_N3D) != 1)
    , DUMP_CAPTURE(checkStereoProperty(PROPERTY_DUMP_CAPTURE))
    , DUMP_PREVIEW(checkStereoProperty(PROPERTY_DUMP_OUTPUT))
    , DUMP_NVRAM(checkStereoProperty(PROPERTY_DUMP_NVRAM))
    , m_eScenario(eSTEREO_SCENARIO_UNKNOWN)
    , m_pWorkBuf(NULL)
    , m_workBufSize(0)
    , m_pAFTable{NULL, NULL}
    , m_isAF{false, false}
    , m_magicNumber{0, 0}
    , m_requestNumber(0)
    , m_pStereoDrv(NULL)
#if (1 == DEPTH_AF_SUPPORTED)
    , m_pDepthAFHAL(NULL)
#endif
    , m_main1Mask(NULL)
    , m_main2Mask(NULL)
    , m_main2MaskSize(0)
    , __inputMain2CPU(NULL)
    , m_stereoExtraData(NULL)
    , m_queryIndex(NOT_QUERY)
    , __fastLogger(LOG_TAG, LOG_PERPERTY)
{
    __fastLogger.setSingleLineMode(SINGLE_LINE_LOG);
    if(RUN_N3D) {
        m_pStereoDrv = MTKStereoKernel::createInstance();
    }

#if (1==ENABLE_MULTITHREAD_INIT)
    __initThread = std::thread([&, n3dInitParam]() mutable
#endif
    {
        _initN3DHAL(n3dInitParam);
    }
#if (1==ENABLE_MULTITHREAD_INIT)
    );
#endif
}

N3D_HAL_IMP::~N3D_HAL_IMP()
{
    Mutex::Autolock lock(mLock);    //To protect NVRAM access and staic instance

#if (1==ENABLE_MULTITHREAD_INIT)
    _waitInitThreadFinish();
#endif

    _saveNVRAM();

    if(m_main1Mask) {
        delete [] m_main1Mask;
        m_main1Mask = NULL;
    }

    if(m_main2Mask) {
        delete [] m_main2Mask;
        m_main2Mask = NULL;
    }

    if(m_outputGBMain2.get()) {
        StereoDpUtil::freeImageBuffer(LOG_TAG, m_outputGBMain2);
    }

    if(__inputMain2CPU) {
        delete [] __inputMain2CPU;
        __inputMain2CPU = NULL;
    }

    if (m_pWorkBuf) {
        delete [] m_pWorkBuf;
        m_pWorkBuf = NULL;
    }

#if (1 == DEPTH_AF_SUPPORTED)
    if(m_pDepthAFHAL) {
        m_pDepthAFHAL->destroyInstance();
        m_pDepthAFHAL = NULL;
    }
#endif

    if(__captureStereoDrv == m_pStereoDrv) {
        __captureStereoDrv = NULL;
    } else if(__previewStereoDrv == m_pStereoDrv) {
        __previewStereoDrv = NULL;
    } else {
        MY_LOGE("Stereo drv does not match: this %p, preview %p, capture %p",
                m_pStereoDrv, __previewStereoDrv, __captureStereoDrv);
    }

    if(m_pStereoDrv) {
        delete m_pStereoDrv;
        m_pStereoDrv = NULL;
    }

    if(m_stereoExtraData) {
        delete [] m_stereoExtraData;
        m_stereoExtraData = NULL;
    }

    if(__debugBufferHeader) {
        delete [] __debugBufferHeader;
        __debugBufferHeader = NULL;
    }
}

bool
N3D_HAL_IMP::_initN3DHAL(N3D_HAL_INIT_PARAM &n3dInitParam)
{
    __sensorSenarios[0] = StereoSettingProvider::getSensorScenarioMain1();
    __sensorSenarios[1] = StereoSettingProvider::getSensorScenarioMain2();

    m_eScenario = n3dInitParam.eScenario;
    switch(n3dInitParam.eScenario) {
        case eSTEREO_SCENARIO_CAPTURE:
            if( StereoSettingProvider::isDeNoise() )
            {
                m_algoInitInfo.scenario = STEREO_KERNEL_SCENARIO_ONLY_CAPTURE;
            } else {
                m_algoInitInfo.scenario = STEREO_KERNEL_SCENARIO_IMAGE_CAPTURE;
            }

            __captureStereoDrv = m_pStereoDrv;

            break;
        case eSTEREO_SCENARIO_PREVIEW:
        case eSTEREO_SCENARIO_RECORD:
            if( StereoSettingProvider::isDeNoise() )
            {
                m_algoInitInfo.scenario = STEREO_KERNEL_SCENARIO_ONLY_CAPTURE;
            } else {
                // m_algoInitInfo.scenario = STEREO_KERNEL_SCENARIO_IMAGE_PREVIEW;
                m_algoInitInfo.scenario = STEREO_KERNEL_SCENARIO_VIDEO_RECORD;
            }
            __previewStereoDrv = m_pStereoDrv;

            break;
        default:
            break;
    }

    int32_t main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    if(!_initLensInfo(main1Idx, m_pAFTable[0], m_isAF[0])) {
        return false;
    }

    if(!_initLensInfo(main2Idx, m_pAFTable[1], m_isAF[1])) {
        return false;
    }

    _initAFWinTransform(n3dInitParam.eScenario);

    Pass2SizeInfo pass2SizeInfo;
    StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_2, m_eScenario, pass2SizeInfo);
    m_algoInitInfo.iio_main.inp_w = pass2SizeInfo.areaWDMA.size.w;
    m_algoInitInfo.iio_main.inp_h = pass2SizeInfo.areaWDMA.size.h;

    StereoArea areaLDC = StereoSizeProvider::getInstance()->getBufferSize(E_LDC, m_eScenario);
    MSize szLDCContent = areaLDC.contentSize();
    m_algoInitInfo.iio_cmp.inp_w = szLDCContent.w;
    m_algoInitInfo.iio_cmp.inp_h = szLDCContent.h;
    m_algoInitInfo.iio_cmp.src_w = szLDCContent.w;
    m_algoInitInfo.iio_cmp.src_h = szLDCContent.h;
    m_algoInitInfo.iio_cmp.out_w = areaLDC.size.w;
    m_algoInitInfo.iio_cmp.out_h = areaLDC.size.h;

    if(eSTEREO_SCENARIO_CAPTURE != m_eScenario) {
        //272x144
        m_algoInitInfo.iio_main.src_w = m_algoInitInfo.iio_main.inp_w;
        m_algoInitInfo.iio_main.src_h = m_algoInitInfo.iio_main.inp_h;

        MSize szOutput = StereoSizeProvider::getInstance()->getBufferSize(E_SV_Y, m_eScenario);
        m_algoInitInfo.iio_main.out_w = szOutput.w;
        m_algoInitInfo.iio_main.out_h = szOutput.h;

        Pass2SizeInfo pass2SizeInfo;
        StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_P_2, m_eScenario, pass2SizeInfo);
        m_algoInitInfo.iio_auxi.src_w = pass2SizeInfo.areaWDMA.size.w;
        m_algoInitInfo.iio_auxi.src_h = pass2SizeInfo.areaWDMA.size.h;
        m_algoInitInfo.iio_auxi.inp_w = pass2SizeInfo.areaWDMA.size.w;
        m_algoInitInfo.iio_auxi.inp_h = pass2SizeInfo.areaWDMA.size.h;
    } else {
        m_algoInitInfo.iio_main.src_w = pass2SizeInfo.areaWDMA.contentSize().w;
        m_algoInitInfo.iio_main.src_h = pass2SizeInfo.areaWDMA.contentSize().h;

        Pass2SizeInfo pass2SizeInfo;
        StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_P_2, m_eScenario, pass2SizeInfo);
        m_algoInitInfo.iio_auxi.src_w = pass2SizeInfo.areaWDMA.contentSize().w;
        m_algoInitInfo.iio_auxi.src_h = pass2SizeInfo.areaWDMA.contentSize().h;
        m_algoInitInfo.iio_auxi.inp_w = pass2SizeInfo.areaWDMA.size.w;
        m_algoInitInfo.iio_auxi.inp_h = pass2SizeInfo.areaWDMA.size.h;

        MSize szOutput = StereoSizeProvider::getInstance()->getBufferSize(E_SV_Y_LARGE, m_eScenario);
        m_algoInitInfo.iio_main.out_w = szOutput.w;
        m_algoInitInfo.iio_main.out_h = szOutput.h;

        //init m_main1Mask
        StereoArea areaMask = StereoSizeProvider::getInstance()->getBufferSize(E_MASK_M_Y, m_eScenario);
        areaMask.startPt.x += 4;
        areaMask.startPt.y += 4;
        areaMask.padding.w += 8;
        areaMask.padding.h += 8;
        MUINT32 length = areaMask.size.w * areaMask.size.h;
        if(length > 0) {
            if(NULL == m_main1Mask) {
                m_main1Mask = new MUINT8[length];
                ::memset(m_main1Mask, 0, sizeof(MUINT8)*length);
                MUINT8 *startPos = m_main1Mask + areaMask.startPt.x+areaMask.size.w*areaMask.startPt.y;
                const MUINT32 END_Y = areaMask.contentSize().h;
                const MUINT32 CONTENT_W = areaMask.contentSize().w * sizeof(MUINT8);
                for(unsigned int y = 0; y < END_Y; y++) {
                    ::memset(startPos, 0xFF, CONTENT_W);
                    startPos += areaMask.size.w;
                }
            }
        } else {
            MY_LOGE("Size of MASK_M_Y is 0");
        }
    }

    //Not used
    m_algoInitInfo.iio_auxi.out_w = m_algoInitInfo.iio_main.out_w;
    m_algoInitInfo.iio_auxi.out_h = m_algoInitInfo.iio_main.out_h;

    // HWFE INPUT - the actual size for HWFE (after SRZ)
    m_algoInitInfo.geo_info  = n3dInitParam.fefmRound;   //N3D_HAL_INIT_PARAM.fefmRound
    m_algoInitInfo.geo_info |= 3<<2;
    _initN3DGeoInfo(m_algoInitInfo.geo_img);              //FEFM setting

    // COLOR CORRECTION INPUT
    _initCCImgInfo(m_algoInitInfo.pho_img);       //settings of main = auxi

    // Learning
    SensorFOV main1FOV, main2FOV;
    StereoSettingProvider::getStereoCameraFOV(main1FOV, main2FOV);
    m_algoInitInfo.fov_main[0]     = main1FOV.fov_horizontal;
    m_algoInitInfo.fov_main[1]     = main1FOV.fov_vertical;
    m_algoInitInfo.fov_auxi[0]     = main2FOV.fov_horizontal;
    m_algoInitInfo.fov_auxi[1]     = main2FOV.fov_vertical;

    m_algoInitInfo.baseline     = StereoSettingProvider::getStereoBaseline();
    m_algoInitInfo.system_cfg   = _prepareSystemConfig();

    // Learning Coordinates RE-MAPPING
    _getStereoRemapInfo(m_algoInitInfo.flow_main, m_algoInitInfo.flow_auxi, m_eScenario);

#if (1 == DEPTH_AF_SUPPORTED)
    if(NULL == m_pDepthAFHAL &&
       eSTEREO_SCENARIO_CAPTURE != m_eScenario)
    {
        m_pDepthAFHAL = StereoDepthHal::createInstance();    // init() has already run inside createInstance()

        if(m_pDepthAFHAL) {
            STEREODEPTH_HAL_INIT_PARAM_STRUCT stStereodepthHalInitParam;
            stStereodepthHalInitParam.stereo_fov_main       = m_algoInitInfo.fov_main[0];
            stStereodepthHalInitParam.stereo_fov_main2      = m_algoInitInfo.fov_auxi[0];
            stStereodepthHalInitParam.stereo_baseline       = m_algoInitInfo.baseline;
            stStereodepthHalInitParam.stereo_pxlarr_width   = m_algoInitInfo.flow_main.pixel_array_width;
            stStereodepthHalInitParam.stereo_pxlarr_height  = m_algoInitInfo.flow_main.pixel_array_height;
            stStereodepthHalInitParam.stereo_main12_pos     = StereoSettingProvider::getSensorRelativePosition();
            stStereodepthHalInitParam.pNvRamDataArray       = m_pVoidGeoData->StereoNvramData.DepthAfData;

            AutoProfileUtil profile(LOG_TAG, "Init StereoDepthHAL");
            m_pDepthAFHAL->StereoDepthInit(&stStereodepthHalInitParam);
        }
    }
#endif

    ::memset(&m_algoInitInfo.af_init_main, 0, sizeof(STEREO_KERNEL_AF_INIT_STRUCT));
    ::memset(&m_algoInitInfo.af_init_auxi, 0, sizeof(STEREO_KERNEL_AF_INIT_STRUCT));

    if ( !_doStereoKernelInit(false) ) {
        MY_LOGE("Init N3D algo failed");
        return false;
    }

    return true;
}

bool
N3D_HAL_IMP::N3DHALRun(N3D_HAL_PARAM &n3dParams, N3D_HAL_OUTPUT &n3dOutput)
{
    //Capture and preview instance cannot run simultaneously
    Mutex::Autolock lock(mRunLock);

    if(eSTEREO_SCENARIO_CAPTURE == m_eScenario) {
        MY_LOGW("Cannot run capture here");
        return false;
    }

#if (1==ENABLE_MULTITHREAD_INIT)
    _waitInitThreadFinish();
#endif

    AutoProfileUtil profile(LOG_TAG, "N3DHALRun(Preview/VR)");
    m_requestNumber = n3dParams.requestNumber;
    _dumpNVRAM(DUMP_INPUT_NVRAM);

    //DAC info should be ready now, and scenario may change due to EIS, re-init N3D if needed
    if(!__isInit) {
        ENUM_STEREO_SCENARIO scenario = (n3dParams.eisData.isON) ? eSTEREO_SCENARIO_RECORD : eSTEREO_SCENARIO_PREVIEW;
        MUINT sensorScenarioMain1 = StereoSettingProvider::getSensorScenarioMain1();
        MUINT sensorScenarioMain2 = StereoSettingProvider::getSensorScenarioMain2();
        if(__sensorSenarios[0] != sensorScenarioMain1 ||
           __sensorSenarios[1] != sensorScenarioMain2 ||
           m_eScenario != scenario)
        {
            m_eScenario = scenario;
            __sensorSenarios[0] = sensorScenarioMain1;
            __sensorSenarios[1] = sensorScenarioMain2;

            Pass2SizeInfo pass2SizeInfo;
            StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_2, scenario, pass2SizeInfo);
            m_algoInitInfo.iio_main.src_w = pass2SizeInfo.areaWDMA.size.w;
            m_algoInitInfo.iio_main.src_h = pass2SizeInfo.areaWDMA.size.h;
            m_algoInitInfo.iio_main.inp_w = m_algoInitInfo.iio_main.src_w;
            m_algoInitInfo.iio_main.inp_h = m_algoInitInfo.iio_main.src_h;

            StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_P_2, scenario, pass2SizeInfo);
            m_algoInitInfo.iio_auxi.src_w = pass2SizeInfo.areaWDMA.size.w;
            m_algoInitInfo.iio_auxi.src_h = pass2SizeInfo.areaWDMA.size.h;
            m_algoInitInfo.iio_auxi.inp_w = pass2SizeInfo.areaWDMA.size.w;
            m_algoInitInfo.iio_auxi.inp_h = pass2SizeInfo.areaWDMA.size.h;

            MY_LOGI("Src size change: main1: %dx%d, main2: %dx%d, re-init N3D",
                    m_algoInitInfo.iio_main.src_w, m_algoInitInfo.iio_main.src_h,
                    m_algoInitInfo.iio_auxi.src_w, m_algoInitInfo.iio_auxi.src_h);

            // Learning Coordinates RE-MAPPING
            _getStereoRemapInfo(m_algoInitInfo.flow_main, m_algoInitInfo.flow_auxi, m_eScenario);
        }

        _doStereoKernelInit(true);
    }

    _setN3DParams(n3dParams, n3dOutput);
    _runN3D(n3dOutput);
    _dumpNVRAM(DUMP_OUTPUT_NVRAM);
    _updateSceneInfo(n3dOutput);
    if(RUN_N3D) {
        n3dOutput.convOffset = ((MFLOAT*)m_algoResult.out_p[STEREO_KERNEL_OUTPUT_DEPTH])[2];
    }

#if (1 == DEPTH_AF_SUPPORTED)
    if(RUN_N3D &&
       m_isAF[0] &&
       NULL != m_pAFTable[0])
    {
        if(n3dParams.isDepthAFON ||
           n3dParams.isDistanceMeasurementON)
        {
            m_pAFTable[0]->is_daf_run |= E_DAF_RUN_DEPTH_ENGINE;   //Turn on Depth-AF in af_mgr
            _runDepthAF(m_magicNumber[0] % DAF_TBL_QLEN);

            if(n3dParams.isDistanceMeasurementON)
            {
                n3dOutput.distance = _getDistance();
            }
        } else {
            m_pAFTable[0]->is_daf_run &= ~E_DAF_RUN_DEPTH_ENGINE;
        }
    }
#endif

    //De-noise feature may not pass ldcMain1, we have to take care of it
    if(NULL == n3dOutput.ldcMain1 &&
       NULL != m_algoProcInfo.addr_ml)
    {
        delete [] m_algoProcInfo.addr_ml;
        m_algoProcInfo.addr_ml = NULL;
    }

    return true;
}

bool
N3D_HAL_IMP::N3DHALRun(N3D_HAL_PARAM_CAPTURE &n3dParams, N3D_HAL_OUTPUT_CAPTURE &n3dOutput)
{
    //Capture and preview instance cannot run simultaneously
    Mutex::Autolock lock(mRunLock);

    // CPUMask cpuMask(CPUCoreL, 4);
    // AutoCPUAffinity affinity(cpuMask);
    _prepareDebugBuffer(n3dOutput.debugBuffer);

#if (1==ENABLE_MULTITHREAD_INIT)
    _waitInitThreadFinish();
#endif

    AutoProfileUtil profile(LOG_TAG, "N3DHALRun(Capture)");
    m_requestNumber = n3dParams.requestNumber;

    //DAC info should be ready now, re-init DAC for N3D
    if(!__isInit) {
       _doStereoKernelInit(true);
    }

    _syncInternalNVRAM(__previewStereoDrv, __captureStereoDrv);   //nvram: preview->capture

    _dumpNVRAM(DUMP_INPUT_NVRAM);

    bool isSuccess  = false;
    _setN3DCaptureParams(n3dParams, n3dOutput);
    if(!DISABLE_GPU) {
        isSuccess = _runN3DCapture(n3dParams, n3dOutput);
    } else {
        // Run preview flow
        if(RUN_N3D) {
            AutoProfileUtil profile(LOG_TAG, "N3D set proc(Capture+CPU)");
            MINT32 err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SET_PROC_INFO, &m_algoProcInfo, NULL);
            if (err) {
                MY_LOGE("StereoKernelFeatureCtrl(SET_PROC) fail. error code: %d.", err);
            } else {
                _dumpSetProcInfo("", m_algoProcInfo);
            }
        }

        isSuccess = _runN3D(n3dOutput);

        //Prepare debug buffer
        if(n3dOutput.debugBuffer) {
            //Copy mask main1
            const size_t IMG_LENGTH = m_algoInitInfo.iio_main.out_w * m_algoInitInfo.iio_main.out_h;
            const size_t LDC_LENGTH = m_algoInitInfo.iio_cmp.out_w * m_algoInitInfo.iio_cmp.out_h;
            const int OFFSET_MV_Y   = __debugContentOffset;
            const int OFFSET_MASK_M = __debugContentOffset + IMG_LENGTH;
            const int OFFSET_SV_Y   = __debugContentOffset + IMG_LENGTH*2;
            const int OFFSET_MASK_S = __debugContentOffset + IMG_LENGTH*3;
            const int OFFSET_LDC    = __debugContentOffset + IMG_LENGTH*4;

            ::memcpy(n3dOutput.debugBuffer+OFFSET_MV_Y,   m_algoProcInfo.addr_md, IMG_LENGTH);
            ::memcpy(n3dOutput.debugBuffer+OFFSET_MASK_M, m_algoProcInfo.addr_mm, IMG_LENGTH);
            ::memcpy(n3dOutput.debugBuffer+OFFSET_SV_Y,   m_algoProcInfo.addr_ad, IMG_LENGTH);
            ::memcpy(n3dOutput.debugBuffer+OFFSET_MASK_S, m_algoProcInfo.addr_am, IMG_LENGTH);
            ::memcpy(n3dOutput.debugBuffer+OFFSET_LDC,    m_algoProcInfo.addr_ml, LDC_LENGTH);
        }
    }
    _dumpDebugBuffer(n3dOutput.debugBuffer);

    _updateSceneInfo(n3dOutput);
    n3dOutput.convOffset        = ((MFLOAT*)m_algoResult.out_p[STEREO_KERNEL_OUTPUT_DEPTH])[2];
    n3dOutput.warpingMatrixSize = m_algoResult.out_n[STEREO_KERNEL_OUTPUT_RECT_CAP];
    if(n3dOutput.warpingMatrix) {
        const MUINT32 RESULT_SIZE = m_algoResult.out_n[STEREO_KERNEL_OUTPUT_RECT_CAP] * sizeof(MFLOAT);
        if( RESULT_SIZE > 0 &&
            RESULT_SIZE <= StereoSettingProvider::getMaxWarpingMatrixBufferSizeInBytes() &&
            NULL != m_algoResult.out_p[STEREO_KERNEL_OUTPUT_RECT_CAP] )
        {
            ::memcpy(n3dOutput.warpingMatrix,
                     m_algoResult.out_p[STEREO_KERNEL_OUTPUT_RECT_CAP],
                     RESULT_SIZE);
        } else {
            MY_LOGE("Invalid warpping matrix size %d(Max: %d), out_p %p",
                    RESULT_SIZE,
                    StereoSettingProvider::getMaxWarpingMatrixBufferSizeInBytes(),
                    m_algoResult.out_p[STEREO_KERNEL_OUTPUT_RECT_CAP]);
        }
    }

    //Denoise will learn in capture
    if( StereoSettingProvider::isDeNoise() )
    {
        _syncInternalNVRAM(__captureStereoDrv, __previewStereoDrv);   //nvram: capture->preview

        _dumpNVRAM(DUMP_OUTPUT_NVRAM);
    }
    //
    if(NULL == n3dOutput.ldcMain1 &&
       NULL != m_algoProcInfo.addr_ml)
    {
        delete [] m_algoProcInfo.addr_ml;
        m_algoProcInfo.addr_ml = NULL;
    }

    return isSuccess;
}

bool
N3D_HAL_IMP::N3DHALRun(N3D_HAL_PARAM_CAPTURE_SWFE &n3dParams, N3D_HAL_OUTPUT_CAPTURE &n3dOutput)
{
    //Capture and preview instance cannot run simultaneously
    Mutex::Autolock lock(mRunLock);

    _setN3DSWFECaptureParams(n3dParams, n3dOutput);
    bool isSuccess = _runN3DCaptureSWFE(n3dOutput);
    return isSuccess;
}

void
N3D_HAL_IMP::_updateSceneInfo(N3D_HAL_OUTPUT &n3dOutput)
{
    n3dOutput.sceneInfoSize = m_algoResult.out_n[STEREO_KERNEL_OUTPUT_SCENE_INFO];
    if(n3dOutput.sceneInfo) {
        const MUINT32 RESULT_SIZE = m_algoResult.out_n[STEREO_KERNEL_OUTPUT_SCENE_INFO] * sizeof(MINT32);
        if( RESULT_SIZE > 0 &&
            RESULT_SIZE <= StereoSettingProvider::getMaxSceneInfoBufferSizeInBytes() &&
            NULL != m_algoResult.out_p[STEREO_KERNEL_OUTPUT_SCENE_INFO] )
        {
            ::memcpy(n3dOutput.sceneInfo,
                     m_algoResult.out_p[STEREO_KERNEL_OUTPUT_SCENE_INFO],
                     RESULT_SIZE);
            n3dOutput.sceneInfo[14] = m_algoProcInfo.af_main.dac_i;
        } else {
            MY_LOGE("Invalid Scene Info size %d(Max: %d), out_p %p",
                    RESULT_SIZE,
                    StereoSettingProvider::getMaxSceneInfoBufferSizeInBytes(),
                    m_algoResult.out_p[STEREO_KERNEL_OUTPUT_SCENE_INFO]);
        }

        n3dOutput.sceneInfo[9] = ( (m_algoInitInfo.af_init_main.dac_mcr << 10) + (m_algoInitInfo.af_init_main.dac_inf) );  //Short distance for denoise
    }
}

char *
N3D_HAL_IMP::getStereoExtraData()
{
    //Only support capture
    if(eSTEREO_SCENARIO_CAPTURE != m_eScenario) {
        return NULL;
    }

    if(NULL == m_stereoExtraData) {
        _prepareStereoExtraData();
    }

    return m_stereoExtraData;
}
/**************************************************************************
 *       Private Functions                                                *
 **************************************************************************/
bool
N3D_HAL_IMP::_getFEOInputInfo(ENUM_PASS2_ROUND pass2Round,
                              ENUM_STEREO_SCENARIO eScenario,
                              STEREO_KERNEL_IMG_INFO_STRUCT &imgInfo)
{
    imgInfo.depth    = 1;    //pixel depth, YUV:1, RGB: 3, RGBA: 4
    imgInfo.format   = 0;    //YUV:0, RGB: 1

    Pass2SizeInfo pass2Info;
    StereoSizeProvider::getInstance()->getPass2SizeInfo(pass2Round, eScenario, pass2Info);

    imgInfo.width        = pass2Info.areaFEO.size.w;
    imgInfo.height       = pass2Info.areaFEO.size.h;
    imgInfo.stride       = imgInfo.width;
    imgInfo.act_width    = pass2Info.areaFEO.size.w - pass2Info.areaFEO.padding.w;
    imgInfo.act_height   = pass2Info.areaFEO.size.h - pass2Info.areaFEO.padding.h;

    if(pass2Round <= PASS2A_3) {
        imgInfo.offset_x = 0;
        imgInfo.offset_y = 0;
    } else {
        imgInfo.offset_x = (imgInfo.width - imgInfo.act_width)>>1;
        imgInfo.offset_y = (imgInfo.height - imgInfo.act_height)>>1;
    }

    return true;
}

bool
N3D_HAL_IMP::_initN3DGeoInfo(STEREO_KERNEL_GEO_INFO_STRUCT geo_img[])
{
    ::memset(geo_img, 0, MAX_GEO_LEVEL * sizeof(STEREO_KERNEL_GEO_INFO_STRUCT));
//    if(MAX_GEO_LEVEL > 0) {
//        geo_img[0].size = StereoSettingProvider::fefmBlockSize(1);    //16
//        _getFEOInputInfo(PASS2A,        m_eScenario,    geo_img[0].img_main);
//        _getFEOInputInfo(PASS2A_P,      m_eScenario,    geo_img[0].img_auxi);
//    }

//    if(MAX_GEO_LEVEL > 1) {
        geo_img[0].size = StereoSettingProvider::fefmBlockSize(1);    //16
        _getFEOInputInfo(PASS2A_2,      m_eScenario,    geo_img[0].img_main);
        _getFEOInputInfo(PASS2A_P_2,    m_eScenario,    geo_img[0].img_auxi);
//    }

//    if(MAX_GEO_LEVEL > 2) {
        geo_img[1].size = StereoSettingProvider::fefmBlockSize(2);    //8
        _getFEOInputInfo(PASS2A_3,      m_eScenario,    geo_img[1].img_main);
        _getFEOInputInfo(PASS2A_P_3,    m_eScenario,    geo_img[1].img_auxi);
//    }

    return true;
}

bool
N3D_HAL_IMP::_initCCImgInfo(STEREO_KERNEL_IMG_INFO_STRUCT &ccImgInfo)
{
    Pass2SizeInfo pass2Info;
    StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_3, m_eScenario, pass2Info);
    MSize szCCImg = pass2Info.areaIMG2O;
    ccImgInfo.width         = szCCImg.w;
    ccImgInfo.height        = szCCImg.h;
    ccImgInfo.depth         = 1;            //pixel depth, YUV:1, RGB: 3, RGBA: 4
    ccImgInfo.stride        = szCCImg.w;
    ccImgInfo.format        = 4;//0;            //YUV:0, RGB: 1

    StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_P_3, m_eScenario, pass2Info);
    szCCImg = pass2Info.areaIMG2O;
    ccImgInfo.act_width     = szCCImg.w;
    ccImgInfo.act_height    = szCCImg.h;
    ccImgInfo.offset_x      = 0;
    ccImgInfo.offset_y      = 0;

    return true;
}

MUINT8 *
N3D_HAL_IMP::_initWorkingBuffer(const MUINT32 BUFFER_SIZE)
{
    if(!RUN_N3D) {
        return NULL;
    }

    if(m_workBufSize < BUFFER_SIZE) {
        if (m_pWorkBuf) {
            delete [] m_pWorkBuf;
            m_pWorkBuf = NULL;
        }

        const MUINT32 BUFFER_LEN = sizeof(MUINT8) * BUFFER_SIZE;
        m_pWorkBuf = new(std::nothrow) MUINT8[BUFFER_LEN];
        if(NULL == m_pWorkBuf) {
            MY_LOGE("Cannot create N3D working buffer of size %d", BUFFER_LEN);
            return NULL;
        } else {
            MY_LOGD_IF(LOG_ENABLED, "Alloc %d bytes for N3D working buffer", BUFFER_LEN);
            ::memset(m_pWorkBuf, 0, BUFFER_LEN);
        }

        m_workBufSize = BUFFER_SIZE;
    }

    MINT32 err = 0; // 0: no error. other value: error.
    // Allocate working buffer.
    //     Allocate memory
    //     Set WorkBufInfo
    m_algoWorkBufInfo.ext_mem_size       = BUFFER_SIZE;
    m_algoWorkBufInfo.ext_mem_start_addr = m_pWorkBuf;

    err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SET_WORK_BUF_INFO,
                                                &m_algoWorkBufInfo, NULL);

    if (err)
    {
        MY_LOGE("StereoKernelFeatureCtrl(SET_WORK_BUF_INFO) fail. error code: %d.", err);
        return NULL;
    }

    return m_pWorkBuf;
}

bool
N3D_HAL_IMP::_doStereoKernelInit(bool isRuntime)
{
    if(!RUN_N3D) {
        return true;
    }

    bool result = false;

    if(isRuntime) {
        if(__isInit) {
            return true;
        }

        //DAC info should be ready now, re-init DAC for N3D
        if(!_initAFInfo(m_algoInitInfo.af_init_main, m_algoInitInfo.af_init_auxi)) {
            MY_LOGE("Fail to init AF info");
            return false;
        }
    }

    _loadNVRAM();

    MRESULT err = 0;
    err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_DEFAULT_TUNING, NULL, &m_algoTuningInfo);
    if (err) {
        MY_LOGE("StereoKernelFeatureCtrl(GET_DEFAULT_TUNING) fail. error code: %d.", err);
        return false;
    } else {
        _dumpTuningInfo("", m_algoTuningInfo);
    }

    AutoProfileUtil profile(LOG_TAG, "N3D init");
    err = m_pStereoDrv->StereoKernelInit(&m_algoInitInfo, &m_algoTuningInfo);
    if (err) {
        MY_LOGE("Init N3D algo failed(err %d)", err);
    } else {
        err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_WORK_BUF_INFO, NULL,
                                                    &m_algoInitInfo.working_buffer_size);
        if(err) {
            MY_LOGD("Get working buffer size failed, err: %d", err);
        } else {
            result = true;
            _initWorkingBuffer(m_algoInitInfo.working_buffer_size);
            _loadLensInfo();
            {
                Mutex::Autolock lock(mLogLock);
                _dumpInitInfo(m_algoInitInfo);
            }

        }
    }

    if(isRuntime) {
        __isInit = (err == 0);
    }

    return result;
}

#if (1==ENABLE_MULTITHREAD_INIT)
void
N3D_HAL_IMP::_waitInitThreadFinish()
{
    if(__initThread.joinable()) {
        MY_LOGD_IF(LOG_ENABLED, "Wait for init thread to finish");
        __initThread.join();
        MY_LOGD_IF(LOG_ENABLED, "Init thread finished");
    }
}
#endif

void
N3D_HAL_IMP::_setN3DCommonParams(N3D_HAL_PARAM_COMMON &n3dParams, N3D_HAL_OUTPUT &n3dOutput, STEREO_KERNEL_SET_PROC_INFO_STRUCT &setprocInfo)
{
    //Check common input
    CHECK_BUFFER(n3dParams.rectifyImgMain1,              "n3dParams.rectifyImgMain1");
    CHECK_BUFFER(n3dParams.rectifyImgMain1->getBufVA(0), "n3dParams.rectifyImgMain1->getBufVA(0)");
    CHECK_BUFFER(n3dParams.ccImage[0],                   "n3dParams.ccImage[0]");
    CHECK_BUFFER(n3dParams.ccImage[0]->getBufVA(0),      "n3dParams.ccImage[0]->getBufVA(0)");
    CHECK_BUFFER(n3dParams.ccImage[1],                   "n3dParams.ccImage[1]");
    CHECK_BUFFER(n3dParams.ccImage[1]->getBufVA(0),      "n3dParams.ccImage[1]->getBufVA(0)");
    //Check common output
    CHECK_BUFFER(n3dOutput.rectifyImgMain1,              "n3dOutput.rectifyImgMain1");
    CHECK_BUFFER(n3dOutput.rectifyImgMain1->getBufVA(0), "n3dOutput.rectifyImgMain1->getBufVA(0)");
    CHECK_BUFFER(n3dOutput.maskMain1,                    "n3dOutput.maskMain1");
    CHECK_BUFFER(n3dOutput.rectifyImgMain2,              "n3dOutput.rectifyImgMain2");
    CHECK_BUFFER(n3dOutput.rectifyImgMain2->getBufVA(0), "n3dOutput.rectifyImgMain2->getBufVA(0)");
    CHECK_BUFFER(n3dOutput.maskMain2,                    "n3dOutput.maskMain2");
    if(!StereoSettingProvider::isDeNoise()) {
        CHECK_BUFFER(n3dOutput.ldcMain1, "n3dOutput.ldcMain1");
    }

    ::memset(&setprocInfo, 0, sizeof(STEREO_KERNEL_SET_PROC_INFO_STRUCT));
    m_magicNumber[0]   = n3dParams.magicNumber[0];
    m_magicNumber[1]   = n3dParams.magicNumber[1];
    m_requestNumber    = n3dParams.requestNumber;

    //buffer is squential, so getBufVA(0) can get whole image
    setprocInfo.addr_ms  = (MUINT8*)n3dParams.rectifyImgMain1->getBufVA(0);
    setprocInfo.addr_md  = (MUINT8*)n3dOutput.rectifyImgMain1->getBufVA(0);
    setprocInfo.addr_mm  = n3dOutput.maskMain1;
    //De-noise feature may not pass ldcMain1, we have to take care of it
    if(n3dOutput.ldcMain1) {
        setprocInfo.addr_ml = n3dOutput.ldcMain1;
    } else {
        setprocInfo.addr_ml = new MUINT8[m_algoInitInfo.iio_cmp.out_w*m_algoInitInfo.iio_cmp.out_h];
    }

    // for Photometric Correction
    //buffer is squential, so getBufVA(0) can get whole image
    setprocInfo.addr_mp   = (MUINT8*)n3dParams.ccImage[0]->getBufVA(0);
    setprocInfo.addr_ap   = (MUINT8*)n3dParams.ccImage[1]->getBufVA(0);
    // HWFE
    for(int i = 0; i < MAX_GEO_LEVEL; i++) {
        setprocInfo.addr_me[i] = n3dParams.hwfefmData.geoDataMain1[i];
        setprocInfo.addr_ae[i] = n3dParams.hwfefmData.geoDataMain2[i];
        setprocInfo.addr_lr[i] = n3dParams.hwfefmData.geoDataLeftToRight[i];
        setprocInfo.addr_rl[i] = n3dParams.hwfefmData.geoDataRightToLeft[i];
    }

    // AF INFO
    if(m_isAF[0] &&
       NULL != m_pAFTable[0])
    {
        const int AF_INDEX = m_magicNumber[0] % DAF_TBL_QLEN;
        setprocInfo.af_main.dac_i = m_pAFTable[0]->daf_vec[AF_INDEX].af_dac_pos;
        setprocInfo.af_main.dac_v = m_pAFTable[0]->daf_vec[AF_INDEX].is_af_stable;
        setprocInfo.af_main.dac_c = m_pAFTable[0]->daf_vec[AF_INDEX].af_confidence;
        setprocInfo.af_main.dac_w[0] = m_pAFTable[0]->daf_vec[AF_INDEX].af_win_start_x;
        setprocInfo.af_main.dac_w[1] = m_pAFTable[0]->daf_vec[AF_INDEX].af_win_start_y;
        setprocInfo.af_main.dac_w[2] = m_pAFTable[0]->daf_vec[AF_INDEX].af_win_end_x;
        setprocInfo.af_main.dac_w[3] = m_pAFTable[0]->daf_vec[AF_INDEX].af_win_end_y;
        m_pAFTable[0]->is_daf_run |= E_DAF_RUN_STEREO;  //Backup plan, should be set when init
    }

    if(m_isAF[1] &&
       NULL != m_pAFTable[1])
    {
        const int AF_INDEX = m_magicNumber[1] % DAF_TBL_QLEN;
        setprocInfo.af_auxi.dac_i = m_pAFTable[1]->daf_vec[AF_INDEX].af_dac_pos;
        setprocInfo.af_auxi.dac_v = m_pAFTable[1]->daf_vec[AF_INDEX].is_af_stable;
        setprocInfo.af_auxi.dac_c = m_pAFTable[1]->daf_vec[AF_INDEX].af_confidence;
        setprocInfo.af_auxi.dac_w[0] = m_pAFTable[1]->daf_vec[AF_INDEX].af_win_start_x;
        setprocInfo.af_auxi.dac_w[1] = m_pAFTable[1]->daf_vec[AF_INDEX].af_win_start_y;
        setprocInfo.af_auxi.dac_w[2] = m_pAFTable[1]->daf_vec[AF_INDEX].af_win_end_x;
        setprocInfo.af_auxi.dac_w[3] = m_pAFTable[1]->daf_vec[AF_INDEX].af_win_end_y;
        m_pAFTable[1]->is_daf_run |= E_DAF_RUN_STEREO;  //Backup plan, should be set when init
    }

    setprocInfo.runtime_cfg = 1;
    bool hasSWFEFM = SWFEFMSettingProvider::getInstance()->needToRunSWFEFM(m_magicNumber[0], m_magicNumber[1],(m_eScenario==eSTEREO_SCENARIO_CAPTURE));
    setprocInfo.runtime_cfg |= (hasSWFEFM<<1);
}

void
N3D_HAL_IMP::_setN3DParams(N3D_HAL_PARAM &n3dParams, N3D_HAL_OUTPUT &n3dOutput)
{
    _setN3DCommonParams(n3dParams, n3dOutput, m_algoProcInfo);

    CHECK_BUFFER(n3dParams.rectifyImgMain2,              "n3dParams.rectifyImgMain2");
    CHECK_BUFFER(n3dParams.rectifyImgMain2->getBufVA(0), "n3dParams.rectifyImgMain2->getBufVA(0)");

    // StereoArea areaLDC = StereoSizeProvider::getInstance()->getBufferSize(E_LDC, m_eScenario);
    // MSize szLDCContent = areaLDC.contentSize();
    m_algoProcInfo.iio_cap.inp_w = m_algoInitInfo.iio_cmp.inp_w; //szLDCContent.w;
    m_algoProcInfo.iio_cap.inp_h = m_algoInitInfo.iio_cmp.inp_h; //szLDCContent.h;
    m_algoProcInfo.iio_cap.src_w = m_algoInitInfo.iio_cmp.src_w; //szLDCContent.w;
    m_algoProcInfo.iio_cap.src_h = m_algoInitInfo.iio_cmp.src_h; //szLDCContent.h;
    m_algoProcInfo.iio_cap.out_w = m_algoInitInfo.iio_cmp.out_w; //areaLDC.size.w;
    m_algoProcInfo.iio_cap.out_h = m_algoInitInfo.iio_cmp.out_h; //areaLDC.size.h;

    //buffer is squential, so getBufVA(0) can get whole image
    m_algoProcInfo.addr_as = (MUINT8*)n3dParams.rectifyImgMain2->getBufVA(0);
    m_algoProcInfo.addr_ad = (MUINT8*)n3dOutput.rectifyImgMain2->getBufVA(0);
    m_algoProcInfo.addr_am = n3dOutput.maskMain2;

    // EIS INFO.
    if(n3dParams.eisData.isON) {
        m_algoProcInfo.eis[0] = n3dParams.eisData.eisOffset.x - (m_algoInitInfo.flow_main.rrz_out_width  - n3dParams.eisData.eisImgSize.w)/2;
        m_algoProcInfo.eis[1] = n3dParams.eisData.eisOffset.y - (m_algoInitInfo.flow_main.rrz_out_height - n3dParams.eisData.eisImgSize.h)/2;
        m_algoProcInfo.eis[2] = m_algoInitInfo.flow_main.rrz_out_width;
        m_algoProcInfo.eis[3] = m_algoInitInfo.flow_main.rrz_out_height;
    } else {
        m_algoProcInfo.eis[0] = 0;
        m_algoProcInfo.eis[1] = 0;
        m_algoProcInfo.eis[2] = 0;
        m_algoProcInfo.eis[3] = 0;
    }

    if(RUN_N3D) {
        AutoProfileUtil profile(LOG_TAG, "N3D set proc(Preview/VR)");
        MINT32 err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SET_PROC_INFO, &m_algoProcInfo, NULL);
        if (err) {
            MY_LOGE("StereoKernelFeatureCtrl(SET_PROC) fail. error code: %d.", err);
        } else {
            _dumpSetProcInfo("", m_algoProcInfo);
        }
    }
}

void
N3D_HAL_IMP::_setN3DCaptureParams(N3D_HAL_PARAM_CAPTURE &n3dParams, N3D_HAL_OUTPUT_CAPTURE &n3dOutput)
{
    AutoProfileUtil profile(LOG_TAG, "Set params(Capture): Total   ");
    m_captureOrientation = n3dParams.captureOrientation;
    __touchPt.x = n3dParams.touchPosX;
    __touchPt.y = n3dParams.touchPosY;

    //Common Part
    _setN3DCommonParams(n3dParams, n3dOutput, m_algoProcInfo);

    CHECK_BUFFER(n3dParams.rectifyGBMain2, "n3dParams.rectifyGBMain2");

    //Capture Part
    MSize captureSize = StereoSizeProvider::getInstance()->captureImageSize();
#if (1 == STEREO_DENOISE_SUPPORTED)
    if(StereoSettingProvider::isDeNoise()) {
        // captureSize = StereoSizeProvider::getInstance()->getBufferSize(E_BM_PREPROCESS_W_1, m_eScenario).contentSize();

        //N3D should use tg size for capture since ratio cropping will happen after warping
        if(StereoSettingProvider::getModuleRotation() & 0x2) {
            captureSize.w = m_algoInitInfo.flow_main.tg_size_h;
            captureSize.h = m_algoInitInfo.flow_main.tg_size_w;
        } else {
            captureSize.w = m_algoInitInfo.flow_main.tg_size_w;
            captureSize.h = m_algoInitInfo.flow_main.tg_size_h;
        }
    }
#endif

    m_algoProcInfo.iio_cap.inp_w = captureSize.w;
    m_algoProcInfo.iio_cap.inp_h = captureSize.h;
    m_algoProcInfo.iio_cap.src_w = captureSize.w;
    m_algoProcInfo.iio_cap.src_h = captureSize.h;
    m_algoProcInfo.iio_cap.out_w = captureSize.w;
    m_algoProcInfo.iio_cap.out_h = captureSize.h;

    if( DUMP_CAPTURE ) {
        _mkdir();
        char dumpPath[PATH_MAX+1];
        char folder[PATH_MAX+1];
        sprintf(dumpPath, "%s/N3D_main2_GB_in_%dx%d.yuv", _getDumpFolderName(m_requestNumber, folder),
                          m_algoInitInfo.iio_auxi.inp_w, m_algoInitInfo.iio_auxi.inp_h);
        n3dParams.rectifyGBMain2->saveToFile(dumpPath);
    }

    AHardwareBuffer* srcGBArray[1];
    AHardwareBuffer* dstGBArray[1];
    if(!DISABLE_GPU) {
        m_algoProcInfo.src_gb.mGraphicBuffer = n3dParams.rectifyGBMain2->getImageBufferHeap()->getHWBuffer();
        if(NULL == m_outputGBMain2.get()) {
            if( !StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_RGBA8888,
                                                MSize(m_algoInitInfo.iio_main.out_w, m_algoInitInfo.iio_main.out_h),
                                                IS_ALLOC_GB, m_outputGBMain2) )
            {
                MY_LOGE("Cannot alloc image buffer");
            }
        }
        m_algoProcInfo.dst_gb.mGraphicBuffer = (void *)m_outputGBMain2.get()->getImageBufferHeap()->getHWBuffer();

        srcGBArray[0] = (AHardwareBuffer*)m_algoProcInfo.src_gb.mGraphicBuffer;
        dstGBArray[0] = (AHardwareBuffer*)m_algoProcInfo.dst_gb.mGraphicBuffer;
        m_algoProcInfo.InputGB   = (void*)&srcGBArray;
        m_algoProcInfo.OutputGB  = (void*)&dstGBArray;
    } else {
        size_t imgLength = m_algoInitInfo.iio_auxi.inp_w*m_algoInitInfo.iio_auxi.inp_h;
        size_t bufferSize = imgLength*3/2;  //YV12
        if(NULL == __inputMain2CPU) {
            __inputMain2CPU = new(std::nothrow) MUINT8[bufferSize];
        }
        ::memset(__inputMain2CPU, 0, bufferSize);

        MUINT8 *dst = __inputMain2CPU;
        ::memcpy(dst, (MUINT8*)n3dParams.rectifyGBMain2->getBufVA(0), imgLength);
        if(n3dParams.rectifyGBMain2->getBufVA(1) != 0) {
            dst += imgLength;
            imgLength >>= 2;
            ::memcpy(dst, (MUINT8*)n3dParams.rectifyGBMain2->getBufVA(1), imgLength);
            dst += imgLength;
            ::memcpy(dst, (MUINT8*)n3dParams.rectifyGBMain2->getBufVA(2), imgLength);
        }

        m_algoProcInfo.addr_as = __inputMain2CPU;
        m_algoProcInfo.addr_ad = (MUINT8*)n3dOutput.rectifyImgMain2->getBufVA(0);
        m_algoProcInfo.addr_am = n3dOutput.maskMain2;
    }

    if(RUN_N3D) {
        if(!DISABLE_GPU) {   //will print later
            _dumpSetProcInfo("", m_algoProcInfo);
        }

        AutoProfileUtil profile2(LOG_TAG, "Set params(Capture): N3D Algo");
        MINT32 err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SET_PROC_INFO, &m_algoProcInfo, NULL);
        if (err) {
            MY_LOGE("StereoKernelFeatureCtrl(SET_PROC) fail. error code: %d.", err);
        }
    }
}

bool
N3D_HAL_IMP::_runN3DCommon()
{
    bool bResult = true;
    if(!RUN_N3D) {
        return true;
    }

    MINT32 err = 0; // 0: no error. other value: error.

    MY_LOGD_IF(LOG_ENABLED, "StereoKernelMain +");
    {
        AutoProfileUtil profile(LOG_TAG, "Run N3D main");
        err = m_pStereoDrv->StereoKernelMain();
    }
    MY_LOGD_IF(LOG_ENABLED, "StereoKernelMain -");

    if (err) {
        MY_LOGE("StereoKernelMain() fail. error code: %d.", err);
        bResult = MFALSE;
    } else {
        // Get result.
        err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_RESULT, NULL, &m_algoResult);
        if (err) {
            MY_LOGE("StereoKernelFeatureCtrl(GET_RESULT) fail. error code: %d.", err);
            bResult = MFALSE;
        }
    }

    if(!err) {
        _dumpResult("[N3D Result]", m_algoResult);

        //Depth AF
        m_algoResult.out_n[0] ;
        m_algoResult.out_p[0] ;
        m_algoResult.out_n[1] ;
        m_algoResult.out_p[1] ;
    }

    _dumpN3DLog();

    return bResult;
}

bool
N3D_HAL_IMP::_runN3D(N3D_HAL_OUTPUT &n3dOutput __attribute__((unused)))
{
    bool bResult = true;
    bResult = _runN3DCommon();

    return bResult;
}

void
N3D_HAL_IMP::_initAFWinTransform(ENUM_STEREO_SCENARIO scenario)
{
    MINT32 err = 0;
    int main1SensorIndex, main2SensorIndex;
    StereoSettingProvider::getStereoSensorIndex(main1SensorIndex, main2SensorIndex);

    int main1SensorDevIndex, main2SensorDevIndex;
    StereoSettingProvider::getStereoSensorDevIndex(main1SensorDevIndex, main2SensorDevIndex);

    IHalSensorList* sensorList = MAKE_HalSensorList();
    IHalSensor* pIHalSensor = NULL;

    if(NULL == sensorList) {
        MY_LOGE("Cannot get sensor list");
    } else {
        //========= Get main1 size =========
        IHalSensor* pIHalSensor = sensorList->createSensor(LOG_TAG, main1SensorIndex);
        if(NULL == pIHalSensor) {
            MY_LOGE("Cannot get hal sensor of main1");
            err = 1;
        } else {
            SensorCropWinInfo sensorCropInfo;
            ::memset(&sensorCropInfo, 0, sizeof(SensorCropWinInfo));
            int sensorScenario = StereoSettingProvider::getSensorScenarioMain1();
            err = pIHalSensor->sendCommand(main1SensorDevIndex, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                           (MUINTPTR)&sensorScenario, (MUINTPTR)&sensorCropInfo, 0);

            if(err) {
                MY_LOGE("Cannot get sensor crop info for scenario %d", scenario);
            } else {
                //For image refocus
                switch(StereoSettingProvider::imageRatio()) {
                case eRatio_4_3:
                    {
                        m_afOffsetX = sensorCropInfo.x2_tg_offset;
                        m_afOffsetY = sensorCropInfo.y2_tg_offset;
                        m_afScaleW = 2000.0f / sensorCropInfo.w2_tg_size;
                        m_afScaleH = 2000.0f / sensorCropInfo.h2_tg_size;
                    }
                    break;
                case eRatio_16_9:
                default:
                    {
                        //4:3->16:9
                        m_afOffsetX = sensorCropInfo.x2_tg_offset;
                        m_afOffsetY = sensorCropInfo.y2_tg_offset + sensorCropInfo.h2_tg_size/8;
                        m_afScaleW = 2000.0f / sensorCropInfo.w2_tg_size;
                        m_afScaleH = 2000.0f / (sensorCropInfo.h2_tg_size * 3 / 4);
                    }
                    break;
                }

                MY_LOGD_IF(LOG_ENABLED,
                           "AF Transform: scenario: %d, offset(%d, %d), scale(%f, %f)",
                           scenario, m_afOffsetX, m_afOffsetY, m_afScaleW, m_afScaleH);
            }

            pIHalSensor->destroyInstance(LOG_TAG);
        }
    }
}

void
N3D_HAL_IMP::_transferAFWin(const AF_WIN_COORDINATE_STRUCT &in, AF_WIN_COORDINATE_STRUCT &out)
{

    if(eSTEREO_SCENARIO_CAPTURE == m_eScenario) {
        //For image refocus
        //ROI correction
        int x_correction = 0;
        if(in.af_win_start_x < m_afOffsetX) {
            x_correction = m_afOffsetX - in.af_win_start_x;
        }

        int y_correction = 0;
        if(in.af_win_start_y < m_afOffsetY) {
            y_correction = m_afOffsetY - in.af_win_start_y;
        }

        int correction = MAX(x_correction, y_correction);

        //ROI correction should not change the center point
        out.af_win_start_x = (in.af_win_start_x - m_afOffsetX + correction) * m_afScaleW - 1000.0f;
        out.af_win_start_y = (in.af_win_start_y - m_afOffsetY + correction) * m_afScaleH - 1000.0f;
        out.af_win_end_x   = (in.af_win_end_x   - m_afOffsetX - correction) * m_afScaleW - 1000.0f;
        out.af_win_end_y   = (in.af_win_end_y   - m_afOffsetY - correction) * m_afScaleH - 1000.0f;

        //Check boundary and adjust
        int offset = out.af_win_start_x - (-1000);
        if(offset < 0) {
            out.af_win_start_x = -1000;
            out.af_win_end_x += offset;
        }

        offset = out.af_win_start_y - (-1000);
        if(offset < 0) {
            out.af_win_start_y = -1000;
            out.af_win_end_y += offset;
        }

        offset = out.af_win_end_x - 1000;
        if(offset > 0) {
            out.af_win_end_x = 1000;
            out.af_win_start_x += offset;
        }

        offset = out.af_win_end_y - 1000;
        if(offset > 0) {
            out.af_win_end_y = 1000;
            out.af_win_start_y += offset;
        }
    } else {
        //For Depth AF
        out.af_win_start_x = in.af_win_start_x - m_afOffsetX;
        out.af_win_start_y = in.af_win_start_y - m_afOffsetY;
        out.af_win_end_x   = in.af_win_end_x   - m_afOffsetX;
        out.af_win_end_y   = in.af_win_end_y   - m_afOffsetY;

        if(eSTEREO_SCENARIO_RECORD == m_eScenario) {
            out.af_win_start_x *= m_afScaleW;
            out.af_win_start_y *= m_afScaleH;
            out.af_win_end_x   *= m_afScaleW;
            out.af_win_end_y   *= m_afScaleH;
        }
    }

    // Verify input AF window to protect camera from NE
    if (in.af_win_start_x == 0 && in.af_win_start_y == 0 && in.af_win_end_x == 0 && in.af_win_end_y == 0 ) {
        out.af_win_start_x = 0;
        out.af_win_start_y = 0;
        out.af_win_end_x   = 0;
        out.af_win_end_y   = 0;
        MY_LOGW("Invalid AF ROI, force output window to be (%d %d %d %d)", out.af_win_start_x, out.af_win_start_y, out.af_win_end_x, out.af_win_end_y);
    }

    MY_LOGD_IF(LOG_ENABLED,
               "AF ROI: %d %d %d %d -> %d %d %d %d",
               in.af_win_start_x, in.af_win_start_y, in.af_win_end_x, in.af_win_end_y,
               out.af_win_start_x, out.af_win_start_y, out.af_win_end_x, out.af_win_end_y);
}

//Must run after getting result
#if (1 == DEPTH_AF_SUPPORTED)
bool
N3D_HAL_IMP::_runDepthAF(const int AF_INDEX)
{
    if(eSTEREO_SCENARIO_CAPTURE == m_eScenario ||
       NULL == m_pDepthAFHAL ||
       NULL == m_algoResult.out_p[1] ||
       NULL == m_pAFTable[0])
    {
        MY_LOGW("Run DAF fail: m_eScenario: %d, HAL: %p, out_p: %p, AF Table %p",
                m_eScenario, m_pDepthAFHAL, m_algoResult.out_p[1], m_pAFTable[0]);
        return false;
    }
    //
    STEREODEPTH_HAL_INIT_PARAM_STRUCT stStereodepthHalInitParam;
    stStereodepthHalInitParam.pCoordTransParam = (MFLOAT *)m_algoResult.out_p[1];
    m_pDepthAFHAL->StereoDepthSetParams(&stStereodepthHalInitParam);

    AF_WIN_COORDINATE_STRUCT afWin(m_pAFTable[0]->daf_vec[AF_INDEX].af_win_start_x,
                                   m_pAFTable[0]->daf_vec[AF_INDEX].af_win_start_y,
                                   m_pAFTable[0]->daf_vec[AF_INDEX].af_win_end_x,
                                   m_pAFTable[0]->daf_vec[AF_INDEX].af_win_end_y);
    AF_WIN_COORDINATE_STRUCT dafWin;
    _transferAFWin(afWin, dafWin);

    static int sGetDistanceIndex = 0;
    if(NOT_QUERY == m_pAFTable[0]->is_query_happen) {   //Not query, a.k.a. is learning
        if (0 != m_algoResult.out_n[0] &&
            NULL != m_algoResult.out_p[0] &&
            m_pAFTable[0]->daf_vec[AF_INDEX].is_af_stable) //not to learn when dac is not stable
        {
            MY_LOGD_IF(LOG_ENABLED, "[DAF][Learning] out_n[0] = %d, out_p[0] = %p", m_algoResult.out_n[0], m_algoResult.out_p[0]);
            AutoProfileUtil profile(LOG_TAG, "DAF learning");
            m_pDepthAFHAL->StereoDepthRunLearning(m_algoResult.out_n[0], (MFLOAT *)m_algoResult.out_p[0], &dafWin);
        } else {
            MY_LOGD_IF(LOG_ENABLED, "[DAF][Learning] Ignore, af done: %d, out_n[0] = %d, out_p[0] = %p",
                       m_pAFTable[0]->daf_vec[AF_INDEX].is_af_stable, m_algoResult.out_n[0], m_algoResult.out_p[0]);
        }
    } else {
        MY_LOGD_IF(LOG_ENABLED,
                   "[DAF][Query] is_query_happen %d, af idx %d, out_n[0] = %d, out_p[0] = %p, AF win: (%d, %d), (%d, %d)",
                    m_pAFTable[0]->is_query_happen,
                    AF_INDEX,
                    m_algoResult.out_n[0], m_algoResult.out_p[0],
                    dafWin.af_win_start_x, dafWin.af_win_start_y,
                    dafWin.af_win_end_x, dafWin.af_win_end_y);

        AutoProfileUtil profile(LOG_TAG, "DAF query");

        m_queryIndex = AF_INDEX;
        m_pDepthAFHAL->StereoDepthQuery(m_algoResult.out_n[0], (MFLOAT *)m_algoResult.out_p[0], &dafWin);

        m_ptQueryDistance = dafWin.centerPoint();
    }

    return true;
}

MFLOAT
N3D_HAL_IMP::_getDistance()
{
    if(eSTEREO_SCENARIO_CAPTURE == m_eScenario ||
       NULL == m_pDepthAFHAL ||
       NULL == m_pAFTable[0])
    {
        return 0.0f;
    }

    AutoProfileUtil profile(LOG_TAG, "Get distance");
    MFLOAT distance = 0.0f;
    if(NOT_QUERY != m_queryIndex &&
       m_pAFTable[0]->daf_vec[m_queryIndex].is_af_stable)  //Get distance after dac is stable
    {
        distance = m_pAFTable[0]->daf_vec[m_queryIndex].daf_distance;
        //TODO: use m_ptQueryDistance to query distance from algo
        // distance = 1.2f;    //UNIT: meter
        m_queryIndex = NOT_QUERY;
        MY_LOGD_IF(LOG_ENABLED, "[DAF][Distance] %f, confidence: %d", distance, m_pAFTable[0]->daf_vec[m_queryIndex].daf_confidence);
    }

    return distance;
}
#endif

bool
N3D_HAL_IMP::_runN3DCapture(N3D_HAL_PARAM_CAPTURE &n3dParams, N3D_HAL_OUTPUT_CAPTURE &n3dOutput)
{
    MY_LOGD_IF(LOG_ENABLED, "+");

    if(eSTEREO_SCENARIO_CAPTURE != m_eScenario) {
        MY_LOGW("Wrong scenario, expect %d, fact: %d", eSTEREO_SCENARIO_CAPTURE, m_eScenario);
        return false;
    }

    bool isSuccess = _runN3DCommon();
    if(!isSuccess) {
        return false;
    }

    if( RUN_N3D &&
        1 == checkStereoProperty(PROPERTY_ALGO_BEBUG) )
    {
        static MUINT snLogCount = 0;
        m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SAVE_LOG, &snLogCount, NULL);
    }

    //Copy mask main1
    StereoArea areaMask = StereoSizeProvider::getInstance()->getBufferSize(E_MASK_M_Y, m_eScenario);
    const MUINT32 IMG_LENGTH = areaMask.size.w * areaMask.size.h;
    ::memcpy(n3dOutput.maskMain1, m_main1Mask, IMG_LENGTH * sizeof(MUINT8));

    const int OFFSET_MV_Y   = __debugContentOffset;
    const int OFFSET_MASK_M = __debugContentOffset + IMG_LENGTH;
    const int OFFSET_SV_Y   = __debugContentOffset + IMG_LENGTH*2;
    const int OFFSET_MASK_S = __debugContentOffset + IMG_LENGTH*3;
    const int OFFSET_LDC    = __debugContentOffset + IMG_LENGTH*4;

    size_t ldcBufferSize = m_algoInitInfo.iio_cmp.out_w * m_algoInitInfo.iio_cmp.out_h;
    if(n3dOutput.debugBuffer) {
        ::memcpy(n3dOutput.debugBuffer+OFFSET_MASK_M, m_main1Mask, IMG_LENGTH);
        ::memcpy(n3dOutput.debugBuffer+OFFSET_LDC, m_algoProcInfo.addr_ml, ldcBufferSize);
    }

    if(DUMP_CAPTURE) {
        char dumpPath[PATH_MAX+1];
        char folder[PATH_MAX+1];
        _getDumpFolderName(m_requestNumber, folder);
        sprintf(dumpPath, "%s/N3D_LDC_%dx%d.y", _getDumpFolderName(m_requestNumber, folder),
                m_algoInitInfo.iio_cmp.out_w, m_algoInitInfo.iio_cmp.out_h);
        FILE *fp = fopen(dumpPath, "wb");
        if(fp) {
            fwrite(m_algoProcInfo.addr_ml, 1, ldcBufferSize, fp);
            fflush(fp);
            fclose(fp);
        } else {
            MY_LOGE("Cannot open %s", dumpPath);
        }
    }

    MSize szSrc(m_algoInitInfo.iio_main.out_w, m_algoInitInfo.iio_main.out_h);
    StereoArea newArea = StereoSizeProvider::getInstance()->getBufferSize(E_SV_Y, m_eScenario);
    MSize newSize = newArea.size;
    MUINT32 offset = 0;
    sp<IImageBuffer> srcImg;
    sp<IImageBuffer> dstImg;

    //Thread: processing mask:
    // Split mask ---- Encode extra data (software in new thread)
    //             +-- Resize mask and truncate(MDP & software)
    vector<std::thread> threads;
    threads.push_back(std::thread(
        [&]() mutable
        {
            //=== Split mask ===
            AutoProfileUtil profile(LOG_TAG, "Thread: Main2 mask: Total");
            _splitMask();

            std::thread encodeThread([&]() mutable
            {
                //=== Transfer data to JSON ===
                _prepareStereoExtraData();
            });

            //Use MDP to resize main2 mask and truccate, n3dOutput.maskMain2
            {
                const MINT32 FULL_LEN   = szSrc.w * szSrc.h;
                const MINT32 RESIZE_LEN = newSize.w * newSize.h;

                ::memset(n3dOutput.maskMain2, 0, RESIZE_LEN);

                if(FULL_LEN == RESIZE_LEN) {
                    ::memcpy(n3dOutput.maskMain2, m_main2Mask, FULL_LEN);
                } else if(FULL_LEN % RESIZE_LEN == 0) {
                    AutoProfileUtil profile(LOG_TAG, "Thread: Main2 mask: Resize main2 mask(SW)");
                    const int COL_STEP = szSrc.w/newSize.w;
                    const int ROW_STEP = (szSrc.h/newSize.h-1)*szSrc.w;
                    //Begin from the center of first block
                    MUINT8 *readPos = m_main2Mask + (szSrc.h/newSize.h/2-1) * szSrc.w + COL_STEP/2 - 1;
                    int writePos = 0;
                    for(int row = 0; row < newSize.h; ++row) {
                        for(int col = 0; col < newSize.w; ++col, readPos += COL_STEP) {
                            *(n3dOutput.maskMain2 + writePos++) = (*readPos);
                        }

                        readPos += ROW_STEP;
                    }
                } else {
                    AutoProfileUtil profile(LOG_TAG, "Thread: Main2 mask: Resize main2 mask(MDP)");
                    sp<IImageBuffer> maskImg;
                    if(StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, szSrc, !IS_ALLOC_GB, maskImg)) {
                        //Copy mask to maskImg
                        MY_LOGD_IF(LOG_ENABLED, "Copy main2 mask, Y8, len: %d", FULL_LEN);
                        ::memcpy((MUINT8*)maskImg.get()->getBufVA(0), m_main2Mask, FULL_LEN);

                        sp<IImageBuffer> resizedMask;
                        if(StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, newSize, !IS_ALLOC_GB, resizedMask)) {
                            if(StereoDpUtil::transformImage(maskImg.get(), resizedMask.get())) {
                                //Copy data to n3dOutput.maskMain2
                                MY_LOGD_IF(LOG_ENABLED, "Copy resized main2 mask, len: %d", RESIZE_LEN);
                                ::memcpy(n3dOutput.maskMain2,         (MUINT8*)resizedMask.get()->getBufVA(0), RESIZE_LEN);
                            } else {
                                MY_LOGE("Resize main2 mask fail");
                                StereoDpUtil::freeImageBuffer(LOG_TAG, maskImg);
                                StereoDpUtil::freeImageBuffer(LOG_TAG, resizedMask);
                                isSuccess = false;
                            }
                            StereoDpUtil::freeImageBuffer(LOG_TAG, resizedMask);
                        } else {
                            StereoDpUtil::freeImageBuffer(LOG_TAG, maskImg);
                            MY_LOGE("Cannot alloc resized mask image buffer, Y8, size %dx%d", newSize.w, newSize.h);
                            isSuccess = false;
                        }

                        StereoDpUtil::freeImageBuffer(LOG_TAG, maskImg);
                    } else {
                        MY_LOGE("Cannot alloc original mask image buffer, Y8, size %dx%d", szSrc.w, szSrc.h);
                        isSuccess = false;
                    }

                    //Truncate
                    MY_LOGD_IF(LOG_ENABLED, "Truncate resized main2 mask");
                    for(MINT32 i = RESIZE_LEN-1; i >= 0 ; --i) {
                        *(n3dOutput.maskMain2+i) &= 0xFF;
                    }
                }

                if(n3dOutput.debugBuffer) {
                    ::memcpy(n3dOutput.debugBuffer+OFFSET_MASK_S, n3dOutput.maskMain2, IMG_LENGTH);
                }
            }

            encodeThread.join();
        }
    ));

    //Thread: Use MDP to resize and centerize main1 image, n3dOutput.rectifyImgMain1
    threads.push_back(std::thread(
        [&]() mutable
        {
            AutoProfileUtil profile(LOG_TAG, "Thread: Resize main1(MDP)");

            // Centralize by MDP
            StereoArea areaSrc = MSize(m_algoInitInfo.iio_main.inp_w, m_algoInitInfo.iio_main.inp_h);
            MSize srcContentSize = MSize(m_algoInitInfo.iio_main.src_w, m_algoInitInfo.iio_main.src_h);
            DpRect srcROI(0, 0, srcContentSize.w, srcContentSize.h);
            DpRect dstROI(newArea.startPt.x, newArea.startPt.y, newArea.contentSize().w, newArea.contentSize().h);
            if(StereoDpUtil::transformImage(n3dParams.rectifyImgMain1, n3dOutput.rectifyImgMain1, eRotate_0, &srcROI, &dstROI)) {
                if( DUMP_CAPTURE ) {
                    char dumpPath[PATH_MAX+1];
                    char folder[PATH_MAX+1];
                    _getDumpFolderName(m_requestNumber, folder);

                    sprintf(dumpPath, "%s/N3D_main1_in_%dx%d.yuv", folder, areaSrc.size.w, areaSrc.size.h);
                    n3dParams.rectifyImgMain1->saveToFile(dumpPath);

                    sprintf(dumpPath, "%s/N3D_main1_out_%dx%d.yuv", folder, newSize.w, newSize.h);
                    n3dOutput.rectifyImgMain1->saveToFile(dumpPath);
                }

                if(n3dOutput.debugBuffer) {
                    ::memcpy(n3dOutput.debugBuffer+OFFSET_MV_Y, (void *)n3dOutput.rectifyImgMain1->getBufVA(0), IMG_LENGTH);
                }
            } else {
                MY_LOGE("Resize main1 image fail");
                isSuccess = false;
            }
        }
    ));

    //Thread: Use MDP to convert main2 image(m_outputGBMain2) from RGBA to YV12 for JPS, n3dOutput.jpsImgMain2
    threads.push_back(std::thread(
        [&]() mutable
        {
            AutoProfileUtil profile(LOG_TAG, "Thread: Convert main2 from RGBA to YV12 and resize(MDP)");

            if( DUMP_CAPTURE ) {
                char dumpPath[PATH_MAX+1];
                char folder[PATH_MAX+1];
                _getDumpFolderName(m_requestNumber, folder);
                sprintf(dumpPath, "%s/N3D_main2_GB_out_%dx%d.rgba", _getDumpFolderName(m_requestNumber, folder),
                        m_outputGBMain2.get()->getImgSize().w, m_outputGBMain2.get()->getImgSize().h);
                m_outputGBMain2.get()->saveToFile(dumpPath);
            }

            //Transform to YV12 by MDP
            isSuccess = StereoDpUtil::transformImage(m_outputGBMain2.get(),
                                                     n3dOutput.rectifyImgMain2);

            if(isSuccess &&
               DUMP_CAPTURE)
            {
                char dumpPath[PATH_MAX+1];
                char folder[PATH_MAX+1];

                sprintf(dumpPath, "%s/N3D_main2_out_%dx%d.yuv", _getDumpFolderName(m_requestNumber, folder),
                        n3dOutput.rectifyImgMain2->getImgSize().w, n3dOutput.rectifyImgMain2->getImgSize().h);
                n3dOutput.rectifyImgMain2->saveToFile(dumpPath);
            }

            if(n3dOutput.debugBuffer) {
                ::memcpy(n3dOutput.debugBuffer+OFFSET_SV_Y, (void *)n3dOutput.rectifyImgMain2->getBufVA(0), IMG_LENGTH);
            }
        }
    ));

    std::for_each(threads.begin(), threads.end(), [](std::thread &t) {
        t.join();
    });

    MY_LOGD_IF(LOG_ENABLED, "-");

    return isSuccess;
}

void
N3D_HAL_IMP::_setN3DSWFECaptureParams(N3D_HAL_PARAM_CAPTURE_SWFE &n3dParams, N3D_HAL_OUTPUT_CAPTURE &n3dOutput)
{
    _setN3DCommonParams(n3dParams, n3dOutput, m_algoProcInfo);
//    MUINT8* addr_mg[MAX_GEO_LEVEL] ;  //SWFEFM_DATA.geo_src_image_main1
//    MUINT8* addr_ag[MAX_GEO_LEVEL] ;  //SWFEFM_DATA.geo_src_image_main2

    _dumpSetProcInfo("", m_algoProcInfo);
}

bool
N3D_HAL_IMP::_initLensInfo(const int32_t SENSOR_INDEX, DAF_TBL_STRUCT *&pAFTable, bool &isAF)
{
    const int DEFAULT_DAC = 0;
    isAF = false;

    IHal3A *pHal3A = MAKE_Hal3A(SENSOR_INDEX, LOG_TAG);
    if(NULL == pHal3A) {
        MY_LOGE("Cannot get 3A HAL of sensor %d", SENSOR_INDEX);
        return false;
    } else {
        isAF = StereoSettingProvider::isSensorAF(SENSOR_INDEX);
        if(!isAF) {
            MY_LOGD_IF(LOG_ENABLED, "[FF] Use default min dac of sensor %d: %d", SENSOR_INDEX, DEFAULT_DAC);
        } else {
            pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetAFDAFTable, (MUINTPTR)&pAFTable, 0);
            if(NULL == pAFTable) {
                MY_LOGE("[AF] Cannot get AF table of sensor %d", SENSOR_INDEX);
                pHal3A->destroyInstance(LOG_TAG);
                return false;
            }

            if (0 == pAFTable->af_dac_min) {
                MY_LOGW("[AF] Cannot get af_dac_min of sensor %d", SENSOR_INDEX);
            } else {
                pAFTable->is_daf_run |= E_DAF_RUN_STEREO;
                MY_LOGD_IF(LOG_ENABLED, "[AF] Min dac of sensor %d: %d", SENSOR_INDEX, pAFTable->af_dac_min);
            }
        }

        pHal3A->destroyInstance(LOG_TAG);
    }

    return true;
}

bool
N3D_HAL_IMP::_getStereoRemapInfo(STEREO_KERNEL_FLOW_INFO_STRUCT &infoMain1,
                                 STEREO_KERNEL_FLOW_INFO_STRUCT &infoMain2,
                                 ENUM_STEREO_SCENARIO eScenario)
{
    int sensorIndex[2];
    StereoSettingProvider::getStereoSensorIndex(sensorIndex[0], sensorIndex[1]);

    int sensorDevIndex[2];
    StereoSettingProvider::getStereoSensorDevIndex(sensorDevIndex[0], sensorDevIndex[1]);

    STEREO_KERNEL_FLOW_INFO_STRUCT *kernelRemapInfos[2] { &infoMain1, &infoMain2 };
    ENUM_STEREO_SENSOR sensorEnums[2] { eSTEREO_SENSOR_MAIN1, eSTEREO_SENSOR_MAIN2 };

    IHalSensorList* sensorList = MAKE_HalSensorList();
    IHalSensor* pIHalSensor = NULL;
    SensorCropWinInfo rSensorCropInfo;

    if(NULL == sensorList) {
        MY_LOGE("Cannot get sensor list");
    } else {
        MUINT32 junkStride;
        MSize   szMain1RRZO;
        MRect   tgCropRect;
        MINT32  err = 0;
        //========= Get main1 size =========
        for(int sensor = 0 ; sensor < 2; sensor++) {
            IHalSensor* pIHalSensor = sensorList->createSensor(LOG_TAG, sensorIndex[sensor]);
            if(NULL == pIHalSensor) {
                MY_LOGE("Cannot get hal sensor of sensor %d", sensorIndex[sensor]);
                err = 1;
            } else {
                ::memset(&rSensorCropInfo, 0, sizeof(SensorCropWinInfo));
                err = pIHalSensor->sendCommand(sensorDevIndex[sensor], SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                               (MUINTPTR)&__sensorSenarios[sensor], (MUINTPTR)&rSensorCropInfo, 0);
            }

            if(!err) {
                StereoSizeProvider::getInstance()->getPass1Size( sensorEnums[sensor],
                                                                 eImgFmt_FG_BAYER10,
                                                                 EPortIndex_RRZO,
                                                                 eScenario,
                                                                 //below are outputs
                                                                 tgCropRect,
                                                                 szMain1RRZO,
                                                                 junkStride);

                STEREO_KERNEL_FLOW_INFO_STRUCT *kernelInfo = kernelRemapInfos[sensor];

                kernelInfo->pixel_array_width  = rSensorCropInfo.full_w;
                kernelInfo->pixel_array_height = rSensorCropInfo.full_h ;
                kernelInfo->sensor_offset_x0   = rSensorCropInfo.x0_offset ;
                kernelInfo->sensor_offset_y0   = rSensorCropInfo.y0_offset ;
                kernelInfo->sensor_size_w0     = rSensorCropInfo.w0_size ;
                kernelInfo->sensor_size_h0     = rSensorCropInfo.h0_size ;
                kernelInfo->sensor_scale_w     = rSensorCropInfo.scale_w ;
                kernelInfo->sensor_scale_h     = rSensorCropInfo.scale_h ;
                kernelInfo->sensor_offset_x1   = rSensorCropInfo.x1_offset ;
                kernelInfo->sensor_offset_y1   = rSensorCropInfo.y1_offset ;
                kernelInfo->tg_offset_x        = rSensorCropInfo.x2_tg_offset ;
                kernelInfo->tg_offset_y        = rSensorCropInfo.y2_tg_offset ;
                kernelInfo->sensor_size_w1     = rSensorCropInfo.w1_size ;
                kernelInfo->sensor_size_h1     = rSensorCropInfo.h1_size ;
                kernelInfo->tg_size_w          = rSensorCropInfo.w2_tg_size ;
                kernelInfo->tg_size_h          = rSensorCropInfo.h2_tg_size ;
                kernelInfo->mdp_rotate         = (StereoSettingProvider::getModuleRotation() != 0) ;

                //Pre-crop main1 FOV if difference of FOV is not good enough
                if(0 == sensor)
                {
                    if(eSTEREO_SCENARIO_CAPTURE == eScenario) {
                        Pass2SizeInfo pass2SizeInfo;
                        StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_CROP, eScenario, pass2SizeInfo);
                        StereoArea cropArea = pass2SizeInfo.areaWDMA;
                        kernelInfo->rrz_usage_width  = cropArea.contentSize().w;
                        kernelInfo->rrz_usage_height = cropArea.contentSize().h;

                        kernelInfo->rrz_offset_x = cropArea.startPt.x;
                        kernelInfo->rrz_offset_y = cropArea.startPt.y;
                    } else {
                        kernelInfo->rrz_usage_width  = tgCropRect.s.w;
                        kernelInfo->rrz_usage_height = tgCropRect.s.h;
                        kernelInfo->rrz_offset_x     = tgCropRect.p.x;
                        kernelInfo->rrz_offset_y     = tgCropRect.p.y;
                    }
                } else {    //Only for RRZO
                    kernelInfo->rrz_usage_width  = tgCropRect.s.w;
                    kernelInfo->rrz_usage_height = tgCropRect.s.h;

                    kernelInfo->rrz_offset_x     = tgCropRect.p.x;
                    kernelInfo->rrz_offset_y     = tgCropRect.p.y;
                }

                kernelInfo->rrz_out_width  = szMain1RRZO.w;
                kernelInfo->rrz_out_height = szMain1RRZO.h;
            }

            if(pIHalSensor) {
                pIHalSensor->destroyInstance(LOG_TAG);
            }
        }
    }

    return true;
}

void
N3D_HAL_IMP::_splitMask()
{
    AutoProfileUtil profile(LOG_TAG, "Thread: Main2 mask: Split Mask");
    //====================================================================
    //  SPLITER: Split and rotate mask according to module orientation
    //  Result is stored in m_main2Mask(2176x1152)
    //====================================================================
    const MUINT32 IMAGE_SIZE = m_outputGBMain2.get()->getImgSize().w * m_outputGBMain2.get()->getImgSize().h;

    // init other memory for save rotate image.
    if(m_main2MaskSize < IMAGE_SIZE) {
        if(NULL != m_main2Mask) {
            delete [] m_main2Mask;
            m_main2Mask = NULL;
        }
    }
    m_main2MaskSize = IMAGE_SIZE;

    if(NULL == m_main2Mask) {
        m_main2Mask = new MUINT8[IMAGE_SIZE];
    }
    ::memset(m_main2Mask, 0, IMAGE_SIZE*sizeof(MUINT8));

    MaskSpliter::splitMask(m_outputGBMain2.get(), m_main2Mask);

    //Apply boundary to main2 mask
    MUINT8 *filter = new MUINT8[IMAGE_SIZE];
    ::memset(filter, 0, sizeof(MUINT8)*IMAGE_SIZE);
    MSize filterSize = m_outputGBMain2.get()->getImgSize();
    StereoArea filterArea(filterSize, MSize(8, 8), MPoint(4, 4));
    MUINT8 *startPos = filter + filterArea.startPt.x+filterArea.size.w*filterArea.startPt.y;
    const MUINT32 END_Y = filterArea.contentSize().h;
    const MUINT32 CONTENT_W = filterArea.contentSize().w * sizeof(MUINT8);
    for(unsigned int y = 0; y < END_Y; y++) {
        ::memset(startPos, 0xFF, CONTENT_W);
        startPos += filterArea.size.w;
    }

    for(int i = IMAGE_SIZE-1; i >=0; --i) {
        m_main2Mask[i] &= filter[i];
    }
    delete [] filter;
}

bool
N3D_HAL_IMP::_initAFInfo(STEREO_KERNEL_AF_INIT_STRUCT &afMain1, STEREO_KERNEL_AF_INIT_STRUCT &afMain2)
{
    STEREO_KERNEL_AF_INIT_STRUCT *afInitInfo[2] = {&afMain1, &afMain2};

    //Get calibration data
    ENUM_STEREO_SENSOR sensor;

    for(int i = 0; i < 2; ++i) {
        if(m_isAF[i]) {
            afInitInfo[i]->dac_mcr = m_pAFTable[i]->af_dac_max;
            afInitInfo[i]->dac_inf = m_pAFTable[i]->af_dac_min;
            //TODO: -32 is a work-around, need to get from AF with new parameter
            afInitInfo[i]->dac_str = m_pAFTable[i]->af_dac_min - 32;

            sensor = (0 == i) ? eSTEREO_SENSOR_MAIN1 : eSTEREO_SENSOR_MAIN2;
            StereoSettingProvider::getCalibrationDistance(sensor, afInitInfo[i]->dist_mcr, afInitInfo[i]->dist_inf);
        } else {
            ::memset(afInitInfo[i], 0, sizeof(STEREO_KERNEL_AF_INIT_STRUCT));
        }
    }

    return true;
}

bool
N3D_HAL_IMP::_loadLensInfo()
{
    if(!RUN_N3D) {
        return true;
    }

    MRESULT err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_LOAD_LENS_INFO,
                                                        &(StereoSettingProvider::getLDCTable()[0]),
                                                        NULL);
    if(err) {
        return false;
    }

    return true;
}

MUINT32
N3D_HAL_IMP::_prepareSystemConfig()
{
    MUINT32 config = 0x0033;   //00 00 00 0 0 0 1 1 0 0 1 1
    if( eSTEREO_SCENARIO_CAPTURE == m_eScenario &&
        checkStereoProperty(PROPERTY_ENABLE_VERIFY) > 0 )
    {
        config  |= (1<<2);
    }

    if( eSTEREO_SCENARIO_CAPTURE == m_eScenario &&
        !DISABLE_GPU )
    {
        config  |= (1<<3);  //enable GPU
    }

    if( checkStereoProperty(PROPERTY_ENABLE_CC, 1) == 0 ) {
        config  &= ~(1<<4);
    }

    if( eSTEREO_SCENARIO_CAPTURE != m_eScenario &&
        StereoSettingProvider::isDeNoise() )
    {
        config  &= ~(1<<5); //Disable warping in denoise preview
    }

    if( eRotate_90  == StereoSettingProvider::getModuleRotation() ||
        eRotate_270 == StereoSettingProvider::getModuleRotation() )
    {
        config  |= (1<<6);
    }

    if(StereoSettingProvider::getSensorRelativePosition()) {
        config |= (1<<7);
    }

    config |= (StereoSettingProvider::LDCEnabled()<<8);

    config |= (m_isAF[1] <<9);
    config |= (m_isAF[0] <<10);

    if(!StereoSettingProvider::isBayerPlusMono()) {
        // config |= (1 << 11); //auxi Y only
        config |= (1 << 12);    //main is YV12
    }

    if(StereoSettingProvider::isDeNoise()) {
        config |= (1<<15);
    }

    //Use SW FEFM
    config |= (1<<16);

    //Only run when AF trigger
    config |= (1<<17);

    return config;
}

bool
N3D_HAL_IMP::_saveNVRAM()
{
    if(NULL == m_pVoidGeoData) {
        return false;
    }

    if(StereoSettingProvider::isDeNoise()) {
        //For denoise, only save nvram of capture instance
        if(eSTEREO_SCENARIO_CAPTURE != m_eScenario) {
            return false;
        }
    } else {
        //For VSDoF, only save nvram of preview instance
        if(eSTEREO_SCENARIO_CAPTURE == m_eScenario) {
            return false;
        }
    }

    AutoProfileUtil profile(LOG_TAG, "Save NARAM");

    MBOOL bResult = MTRUE;
    if(RUN_N3D) {
        MINT32 err = 0; // 0: no error. other value: error.
        err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SAVE_NVRAM,
                                                    (void*)&m_pVoidGeoData->StereoNvramData.StereoData, NULL);

        if(err) {
            MY_LOGE("StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SAVE_NVRAM) fail. error code: %d.", err);
            return false;
        }

        int32_t main1DevIdx, main2DevIdx;
        StereoSettingProvider::getStereoSensorDevIndex(main1DevIdx, main2DevIdx);
        auto pNvBufUtil = MAKE_NvBufUtil();
        err = (!pNvBufUtil) ? -1 : pNvBufUtil->write(CAMERA_NVRAM_DATA_GEOMETRY, main1DevIdx);
        if(err) {
            MY_LOGE("Write to NVRAM fail. pNvBufUtil:%p", pNvBufUtil);
            bResult = MFALSE;
        }
    }

    m_pVoidGeoData = NULL;

    return bResult;
}

bool
N3D_HAL_IMP::_loadNVRAM()
{
    Mutex::Autolock lock(mLock);

    AutoProfileUtil profile(LOG_TAG, "Load NVRAM");

    MINT32 err = 0;
    if(NULL == m_pVoidGeoData) {
        int32_t main1DevIdx, main2DevIdx;
        StereoSettingProvider::getStereoSensorDevIndex(main1DevIdx, main2DevIdx);
        auto pNvBufUtil = MAKE_NvBufUtil();
        err = (!pNvBufUtil) ? -1 : pNvBufUtil->getBufAndRead(CAMERA_NVRAM_DATA_GEOMETRY, main1DevIdx, (void*&)m_pVoidGeoData);
        if(err ||
           NULL == m_pVoidGeoData)
        {
            MY_LOGE("Read NVRAM fail, data: %p pNvBufUtil:%p", m_pVoidGeoData, pNvBufUtil);
            return false;
        }
#ifdef GTEST
        if(NULL == m_pVoidGeoData) {
            m_pVoidGeoData = new NVRAM_CAMERA_GEOMETRY_STRUCT;
        }
        // === Prepare learning data ===
        const char *LEARN_FILE = "/data/nativetest/VSDoF_HAL_Test/N3D_UT/in/LearnINFO.n3d";
        MY_LOGD_IF(LOG_ENABLED, "Load Learning data from %s(len: %d)", LEARN_FILE, MTK_STEREO_KERNEL_NVRAM_LENGTH);
        FILE *fdata = fopen(LEARN_FILE, "r") ;
        if( fdata )
        {
            memset( &m_pVoidGeoData->StereoNvramData.StereoData, 0, sizeof(MINT32)*MTK_STEREO_KERNEL_NVRAM_LENGTH );
            for( int i=0 ; i < MTK_STEREO_KERNEL_NVRAM_LENGTH ; i++ )
            {
                if(EOF == fscanf( fdata, "%d", &m_pVoidGeoData->StereoNvramData.StereoData[i] ) ) {
                    break;
                }
            }

            fclose(fdata) ;
        } else {
            if(errno) {
                MY_LOGE("Cannot read learning data, error: %s", ::strerror(errno));
            } else {
                MY_LOGE("Cannot read learning data");
            }
        }
#endif
    }

    if(RUN_N3D) {
        err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_LOAD_NVRAM,
                                                    (void*)&m_pVoidGeoData->StereoNvramData.StereoData, NULL);
        if (err)
        {
            MY_LOGE("StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_LOAD_NVRAM) fail. error code: %d.", err);
            return false;
        }
    }

    return true;
}

bool
N3D_HAL_IMP::_syncInternalNVRAM(MTKStereoKernel *pSrcStereoDrv, MTKStereoKernel *pDstStereoDrv)
{
    if(!RUN_N3D) {
        return true;
    }

    MINT32 err = 0; // 0: no error. other value: error.
    //Step 1: get nvram data from pSrcStereoDrv
    if(pSrcStereoDrv) {
        err = pSrcStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SAVE_NVRAM,
                                                     (void*)&m_pVoidGeoData->StereoNvramData.StereoData, NULL);
        if (err)
        {
            MY_LOGE("StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SAVE_NVRAM) fail. error code: %d.", err);
            return false;
        }
    } else {
        MY_LOGW("No source stereo drv");
        err = 1;
    }

    //Step 2: save nvram data to pDstStereoDrv
    if(!err) {
        if(pDstStereoDrv) {
            err = pDstStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_LOAD_NVRAM,
                                                         (void*)&m_pVoidGeoData->StereoNvramData.StereoData, NULL);
            if (err)
            {
                MY_LOGE("StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_LOAD_NVRAM) fail. error code: %d.", err);
                return false;
            }
        } else {
            MY_LOGW("No dst stereo drv");
        }
    }

    return true;
}

const char *
N3D_HAL_IMP::_prepareStereoExtraData()
{
    AutoProfileUtil profile(LOG_TAG, "Thread: Main2 mask-2: Encode extra data");

    if(m_stereoExtraData) {
        delete [] m_stereoExtraData;
        m_stereoExtraData = NULL;
    }

#if (1 == STEREO_DENOISE_SUPPORTED)
    if( StereoSettingProvider::isDeNoise() &&
        DENOISE_MODE == E_DENOISE_MODE_NORMAL &&
        checkStereoProperty(PROPERTY_ENABLE_VERIFY) <= 0)
    {
        m_stereoExtraData = new char[3];
        ::strcpy(m_stereoExtraData, "{}");
        return m_stereoExtraData;
    }
#endif

    Document document(kObjectType);
    Document::AllocatorType& allocator = document.GetAllocator();

//    "JPS_size": {
//        "width": 4352,
//        "height": 1152
//    },
    Value JPS_size(kObjectType);
    JPS_size.AddMember(EXTRA_DATA_WIDTH, 0, allocator);
    JPS_size.AddMember(EXTRA_DATA_HEIGHT, 0, allocator);
    document.AddMember(EXTRA_DATA_JPS_SIZE, JPS_size, allocator);

//    "output_image_size" : {
//        "width": 2176,
//        "height": 1152
//    },
    Value output_image_size(kObjectType);
    output_image_size.AddMember(EXTRA_DATA_WIDTH, 0, allocator);
    output_image_size.AddMember(EXTRA_DATA_HEIGHT, 0, allocator);
    document.AddMember(EXTRA_DATA_OUTPUT_IMAGE_SIZE, output_image_size, allocator);

//    "main_cam_align_shift" : {
//        "x": 30,
//        "y": 10
//    },
    Value main_cam_align_shift(kObjectType);
    main_cam_align_shift.AddMember(EXTRA_DATA_X, ((MFLOAT *)m_algoResult.out_p[STEREO_KERNEL_OUTPUT_DEPTH])[0], allocator);
    main_cam_align_shift.AddMember(EXTRA_DATA_Y, ((MFLOAT *)m_algoResult.out_p[STEREO_KERNEL_OUTPUT_DEPTH])[1], allocator);
    document.AddMember(EXTRA_DATA_MAIN_CAM_ALIGN_SHIFT, main_cam_align_shift, allocator);

//    "capture_orientation": {
//        "orientations_values": ["0: none", "1: flip_horizontal", "2: flip_vertical", "4: 90", "3: 180", "7: 270"],
//        "orientation": 0
//    },
    int cap_ori = 0;
    switch(m_captureOrientation){
        case 90:
            cap_ori = 4;
            break;
        case 180:
            cap_ori = 3;
            break;
        case 270:
            cap_ori = 7;
            break;
        case 0:
        default:
            cap_ori = 0;
            break;
    }

    Value capture_orientation(kObjectType);
    Value orientations_values(kArrayType);
    orientations_values.PushBack(Value("0: none").Move(), allocator);
    // orientations_values.PushBack(Value("1: flip_horizontal").Move(), allocator);
    // orientations_values.PushBack(Value("2: flip_vertical").Move(), allocator);
    orientations_values.PushBack(Value("4: 90 degrees CW").Move(), allocator);
    orientations_values.PushBack(Value("3: 180 degrees CW").Move(), allocator);
    orientations_values.PushBack(Value("7: 270 degrees CW").Move(), allocator);
    capture_orientation.AddMember(EXTRA_DATA_ORIENTATIONS_VALUES, orientations_values, allocator);
    capture_orientation.AddMember(EXTRA_DATA_ORIENTATION, Value(cap_ori).Move(), allocator);
    document.AddMember(EXTRA_DATA_CAPTURE_ORIENTATION, capture_orientation, allocator);

//    "depthmap_orientation": {
//        "depthmap_orientation_values": ["0: none", "90: 90 degrees CW", "180: 180 degrees CW", "270: 270 degrees CW"],
//        "orientation": 0
//    },
    Value depthmap_orientation(kObjectType);
    Value depthmap_orientation_values(kArrayType);
    depthmap_orientation_values.PushBack(Value("0: none").Move(), allocator);
    depthmap_orientation_values.PushBack(Value("90: 90 degrees CW").Move(), allocator);
    depthmap_orientation_values.PushBack(Value("180: 180 degrees CW").Move(), allocator);
    depthmap_orientation_values.PushBack(Value("270: 270 degrees CW").Move(), allocator);
    depthmap_orientation.AddMember(EXTRA_DATA_DEPTHMAP_ORIENTATION_VALUES, depthmap_orientation_values, allocator);
    // int depthOrientationValue = m_captureOrientation-StereoSettingProvider::getModuleRotation();
    // if(depthOrientationValue < 0) {
    //     depthOrientationValue += 360;
    // }
    depthmap_orientation.AddMember(EXTRA_DATA_ORIENTATION, 0, allocator);
    document.AddMember(EXTRA_DATA_DEPTHMAP_ORIENTATION, depthmap_orientation, allocator);

//    "depth_buffer_size": {
//        "width": 480,
//        "height": 270
//    },
    Value depth_buffer_size(kObjectType);
    MSize szDepthMap = StereoSizeProvider::getInstance()->getBufferSize(E_DEPTH_MAP, m_eScenario);
    if( 90 == m_captureOrientation ||
       270 == m_captureOrientation)
    {
        szDepthMap = MSize(szDepthMap.h, szDepthMap.w);
    }
    depth_buffer_size.AddMember(EXTRA_DATA_WIDTH, szDepthMap.w, allocator);
    depth_buffer_size.AddMember(EXTRA_DATA_HEIGHT, szDepthMap.h, allocator);
    document.AddMember(EXTRA_DATA_DEPTH_BUFFER_SIZE, depth_buffer_size, allocator);

//    "input_image_size": {
//        "width": 1920,
//        "height": 1088
//    },
    MSize viewSize = szDepthMap * 4;
    Value input_image_size(kObjectType);
    input_image_size.AddMember(EXTRA_DATA_WIDTH, viewSize.w, allocator);
    input_image_size.AddMember(EXTRA_DATA_HEIGHT, viewSize.h, allocator);
    document.AddMember(EXTRA_DATA_INPUT_IMAGE_SIZE, input_image_size, allocator);

//    "sensor_relative_position": {
//        "relative_position_values": ["0: main-minor", "1: minor-main"],
//        "relative_position": 0
//    },
    Value sensor_relative_position(kObjectType);
    Value relative_position_values(kArrayType);
    relative_position_values.PushBack(Value("0: main-minor").Move(), allocator);
    relative_position_values.PushBack(Value("1: minor-main").Move(), allocator);
    sensor_relative_position.AddMember(EXTRA_DATA_RELATIVE_POSITION_VALUES, relative_position_values, allocator);
    sensor_relative_position.AddMember(EXTRA_DATA_RELATIVE_POSITION, Value((int)StereoSettingProvider::getSensorRelativePosition()).Move(), allocator);
    document.AddMember(EXTRA_DATA_SENSOR_RELATIVE_POSITION, sensor_relative_position, allocator);

//    "focus_roi": {
//        "top": 0,
//        "left": 10,
//        "bottom": 10,
//        "right": 30
//    },
    const int AF_INDEX = m_magicNumber[0] % DAF_TBL_QLEN;
    DAF_VEC_STRUCT *afVec = &m_pAFTable[0]->daf_vec[AF_INDEX];
    AF_WIN_COORDINATE_STRUCT apAFWin(0, 0, 0, 0);
    if(m_isAF[0] &&
       NULL != m_pAFTable[0])
    {
        DAF_VEC_STRUCT *afVec = &m_pAFTable[0]->daf_vec[AF_INDEX];
        AF_WIN_COORDINATE_STRUCT afWin(afVec->af_win_start_x, afVec->af_win_start_y,
                                       afVec->af_win_end_x, afVec->af_win_end_y);
        _transferAFWin(afWin, apAFWin);
    } else {   //FF
        if(StereoFDProxy::isFocusedOnFace()) {
            MtkCameraFace face = StereoFDProxy::getFocusedFace();
            apAFWin.af_win_start_x = face.rect[2];
            apAFWin.af_win_start_y = face.rect[1];
            apAFWin.af_win_end_x   = face.rect[0];
            apAFWin.af_win_end_y   = face.rect[3];
        } else {
            apAFWin.af_win_start_x = __touchPt.x;
            apAFWin.af_win_start_y = __touchPt.y;
            apAFWin.af_win_end_x   = __touchPt.x;
            apAFWin.af_win_end_y   = __touchPt.y;
        }
    }

    if(0 != m_captureOrientation) {
        MPoint ptTopLeft = MPoint(apAFWin.af_win_start_x, apAFWin.af_win_start_y);
        MPoint ptBottomRight = MPoint(apAFWin.af_win_end_x, apAFWin.af_win_end_y);

        MY_LOGD_IF(LOG_ENABLED, "AF ROI Before Rotate (%d, %d) (%d, %d)",
                   ptTopLeft.x, ptTopLeft.y, ptBottomRight.x, ptBottomRight.y);

        const MSize ROI_DOMAIN(2000, 2000);     //-1000~1000
        const MPoint ORIGIN_OFFSET(1000, 1000);
        MRect roi = StereoHAL::rotateRect( MRect( ptTopLeft+ORIGIN_OFFSET, ptBottomRight+ORIGIN_OFFSET ),
                                           ROI_DOMAIN,
                                           static_cast<ENUM_ROTATION>(m_captureOrientation) );
        ptTopLeft = roi.leftTop() - ORIGIN_OFFSET;
        ptBottomRight = roi.rightBottom() - ORIGIN_OFFSET;

        MY_LOGD_IF(LOG_ENABLED, "AF ROI After Rotate (%d, %d) (%d, %d), rotate %d",
                   ptTopLeft.x, ptTopLeft.y, ptBottomRight.x, ptBottomRight.y,
                   m_captureOrientation);

        apAFWin.af_win_start_x = ptTopLeft.x;
        apAFWin.af_win_start_y = ptTopLeft.y;
        apAFWin.af_win_end_x   = ptBottomRight.x;
        apAFWin.af_win_end_y   = ptBottomRight.y;
    }

    Value focus_roi(kObjectType);
    focus_roi.AddMember(EXTRA_DATA_TOP,      Value( apAFWin.af_win_start_y ).Move(), allocator);
    focus_roi.AddMember(EXTRA_DATA_LEFT,     Value( apAFWin.af_win_start_x ).Move(), allocator);
    focus_roi.AddMember(EXTRA_DATA_BOTTOM,   Value( apAFWin.af_win_end_y   ).Move(), allocator);
    focus_roi.AddMember(EXTRA_DATA_RIGHT,    Value( apAFWin.af_win_end_x   ).Move(), allocator);
    document.AddMember(EXTRA_DATA_FOCUS_ROI, focus_roi, allocator);

//    "focus_info": {
//        "is_face": 0,
//        "face_ratio": 0.07,
//        "dac_cur": 200,
//        "dac_min": 100,
//        "dac_max": 300,
//        "focus_type": 3
//    }
    Value focus_info(kObjectType);
    if(m_isAF[0] &&
       NULL != m_pAFTable[0])
    {
        DAF_VEC_STRUCT *afVec = &m_pAFTable[0]->daf_vec[AF_INDEX];
        focus_info.AddMember(EXTRA_DATA_IS_FACE, Value( AF_ROI_SEL_FD == afVec->af_roi_sel ).Move(), allocator);
        focus_info.AddMember(EXTRA_DATA_FACE_RATIO, Value( (apAFWin.af_win_end_x-apAFWin.af_win_start_x+1)/2000.0f ).Move(), allocator);
        focus_info.AddMember(EXTRA_DATA_DAC_CUR, Value( afVec->af_dac_pos ).Move(), allocator);
        focus_info.AddMember(EXTRA_DATA_DAC_MIN, Value( m_pAFTable[0]->af_dac_min ).Move(), allocator);
        focus_info.AddMember(EXTRA_DATA_DAC_MAX, Value( m_pAFTable[0]->af_dac_max ).Move(), allocator);
        focus_info.AddMember(EXTRA_DATA_FOCUS_TYPE, Value( afVec->af_roi_sel ).Move(), allocator);
    } else {    //FF
        if(StereoFDProxy::isFocusedOnFace()) {
            focus_info.AddMember(EXTRA_DATA_IS_FACE, Value( 1 ).Move(), allocator);
            MRect rect = StereoFDProxy::getFocusedFaceRect();
            focus_info.AddMember(EXTRA_DATA_FACE_RATIO, Value( rect.s.w/2000.0f ).Move(), allocator);
            focus_info.AddMember(EXTRA_DATA_FOCUS_TYPE, Value( AF_ROI_SEL_FD ).Move(), allocator);
        } else {
            focus_info.AddMember(EXTRA_DATA_FOCUS_TYPE, Value( AF_ROI_SEL_AP ).Move(), allocator);
        }
        focus_info.AddMember(EXTRA_DATA_DAC_CUR, Value( 0 ).Move(), allocator);
        focus_info.AddMember(EXTRA_DATA_DAC_MIN, Value( 0 ).Move(), allocator);
        focus_info.AddMember(EXTRA_DATA_DAC_MAX, Value( 0 ).Move(), allocator);
    }

    document.AddMember(EXTRA_DATA_FOCUS_INFO, focus_info, allocator);

//    "verify_geo_data": {
//        "quality_level_values": ["0: PASS","1: Cond.Pass","2: FAIL"],
//        "quality_level": 0,
//        "statistics": [0,0,0,0,0,0]
//    },
    MINT32 *verityOutputs = (MINT32*)m_algoResult.out_p[STEREO_KERNEL_OUTPUT_VERIFY];
    Value verify_geo_data(kObjectType);
    Value quality_level_values(kArrayType);
    quality_level_values.PushBack(Value("0: PASS").Move(), allocator);
    quality_level_values.PushBack(Value("1: Cond.Pass").Move(), allocator);
    quality_level_values.PushBack(Value("2: FAIL").Move(), allocator);
    verify_geo_data.AddMember(EXTRA_DATA_QUALITY_LEVEL_VALUES, quality_level_values, allocator);
    verify_geo_data.AddMember(EXTRA_DATA_QUALITY_LEVEL, Value(verityOutputs[0]).Move(), allocator);
    Value geo_statistics(kArrayType);
    for(int i = 0; i < 6; i++) {
        geo_statistics.PushBack(Value(verityOutputs[i+2]).Move(), allocator);
    }
    verify_geo_data.AddMember(EXTRA_DATA_STATISTICS, geo_statistics, allocator);
    document.AddMember(EXTRA_DATA_VERIFY_GEO_DATA, verify_geo_data, allocator);

//    "verify_pho_data": {
//        "quality_level_values": ["0: PASS","1: Cond.Pass","2: FAIL"],
//        "quality_level": 0,
//        "statistics": [0,0,0,0]
//    },
    Value verify_pho_data(kObjectType);
    Value pho_quality_level_values(kArrayType);
    pho_quality_level_values.PushBack(Value("0: PASS").Move(), allocator);
    pho_quality_level_values.PushBack(Value("1: Cond.Pass").Move(), allocator);
    pho_quality_level_values.PushBack(Value("2: FAIL").Move(), allocator);
    verify_pho_data.AddMember(EXTRA_DATA_QUALITY_LEVEL_VALUES, pho_quality_level_values, allocator);
    verify_pho_data.AddMember(EXTRA_DATA_QUALITY_LEVEL, Value(verityOutputs[1]).Move(), allocator);
    Value pho_statistics(kArrayType);
    for(int i = 0; i < 4; i++) {
        pho_statistics.PushBack(Value(verityOutputs[i+8]).Move(), allocator);
    }
    verify_pho_data.AddMember(EXTRA_DATA_STATISTICS, pho_statistics, allocator);
    document.AddMember(EXTRA_DATA_VERIFY_PHO_DATA, verify_pho_data, allocator);

//    "verify_mtk_cha": {
//        "check_values": ["0: PASS","1: FAIL"],
//        "check": 0,
//        "score": 0,
//        "distance": 300
//    },
    Value verify_mtk_cha(kObjectType);
    Value verify_mtk_cha_values(kArrayType);
    verify_mtk_cha_values.PushBack(Value("0: PASS").Move(), allocator);
    verify_mtk_cha_values.PushBack(Value("1: FAIL").Move(), allocator);
    verify_mtk_cha.AddMember(EXTRA_DATA_CHECK_VALUES, verify_mtk_cha_values, allocator);
    verify_mtk_cha.AddMember(EXTRA_DATA_CHECK, Value(verityOutputs[12]).Move(), allocator);
    verify_mtk_cha.AddMember(EXTRA_DATA_SCORE, Value(verityOutputs[13]).Move(), allocator);
    verify_mtk_cha.AddMember(EXTRA_DATA_DISTANCE, Value(verityOutputs[14]*10).Move(), allocator);
    document.AddMember(EXTRA_DATA_VERIFY_MTK_CHA, verify_mtk_cha, allocator);

//    "ldc_size": {
//        "width": 272,
//        "height": 144
//    },
    Value ldc_size(kObjectType);
    ldc_size.AddMember(EXTRA_DATA_WIDTH, m_algoInitInfo.iio_cmp.out_w, allocator);
    ldc_size.AddMember(EXTRA_DATA_HEIGHT, m_algoInitInfo.iio_cmp.out_h, allocator);
    document.AddMember(EXTRA_DATA_LDC_SIZE, ldc_size, allocator);

//    "GFocus": {
//        "BlurAtInfinity": 0.014506519,
//        "FocalDistance": 25.612852,
//        "FocalPointX": 0.5,
//        "FocalPointY": 0.5
//    },
    Value GFocus(kObjectType);
    GFocus.AddMember(EXTRA_DATA_BLURATINFINITY, 0.014506519, allocator);
    GFocus.AddMember(EXTRA_DATA_FOCALDISTANCE, 25.612852, allocator);
    GFocus.AddMember(EXTRA_DATA_FOCALPOINTX, 0.5, allocator);
    GFocus.AddMember(EXTRA_DATA_FOCALPOINTY, 0.5, allocator);
    document.AddMember(EXTRA_DATA_GFOCUS, GFocus, allocator);

//    "GImage" : {
//        "Mime": "image/jpeg"
//    },
    Value GImage(kObjectType);
    GImage.AddMember(EXTRA_DATA_MIME, "image/jpeg", allocator);
    document.AddMember(EXTRA_DATA_GIMAGE, GImage, allocator);

//    "GDepth": {
//        "Format": "RangeInverse",
//        "Near": 15.12828254699707,
//        "Far": 97.0217514038086,
//        "Mime": "image/png"
//    },
    Value GDepth(kObjectType);
    GDepth.AddMember(EXTRA_DATA_FORMAT, "RangeInverse", allocator);
    GDepth.AddMember(EXTRA_DATA_NEAR, 15.12828254699707, allocator);
    GDepth.AddMember(EXTRA_DATA_FAR, 97.0217514038086, allocator);
    GDepth.AddMember(EXTRA_DATA_MIME, "image/png", allocator);
    document.AddMember(EXTRA_DATA_GDEPTH, GDepth, allocator);

    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

    const char *stereoExtraData = sb.GetString();
    if(stereoExtraData) {
        const int STR_LEN = strlen(stereoExtraData);
        if(STR_LEN > 0) {
            m_stereoExtraData = new char[STR_LEN+1];
            strcpy(m_stereoExtraData, stereoExtraData);
        }
    }

    if(checkStereoProperty(PROPERTY_DUMP_JSON) == 1) {
        _mkdir();

        char fileName[PATH_MAX+1];
        char folder[PATH_MAX+1];
        sprintf(fileName, "%s/N3D_ExtraData.json", _getDumpFolderName(m_requestNumber, folder));
        FILE *fp = fopen(fileName, "wb");
        if(fp) {
            char writeBuffer[65536];
            FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
            PrettyWriter<FileWriteStream> prettyWriter(os);
            document.Accept(prettyWriter);

            fflush(fp);
            fclose(fp);
        }
    }

    return m_stereoExtraData;
}

char *
N3D_HAL_IMP::_prepareDebugBufferHeader()
{
    if(__debugBufferHeader) {
        return __debugBufferHeader;
    }

    Document document(kObjectType);
    Document::AllocatorType& allocator = document.GetAllocator();

//    "buffer_list": [
//        {
//          "name": "MV_Y",
//          "image_format": "Y8",
//          "image_size": "544x288",
//          "buffer_size": 156672
//        },
//        {
//          "name": "MASK_M",
//          "image_format": "Y8",
//          "image_size": "544x288",
//          "buffer_size": 156672
//        },
//        {
//          "name": "SV_Y",
//          "image_format": "Y8",
//          "image_size": "544x288",
//          "buffer_size": 156672
//        },
//        {
//          "name": "MASK_S",
//          "image_format": "Y8",
//          "image_size": "544x288",
//          "buffer_size": 156672
//        },
//        {
//          "name": "LDC",
//          "image_format": "Y8",
//          "image_size": "544x288",
//          "buffer_size": 156672
//        }
//      ]

    struct N3D_DEBUG_BUFFER {
        const char *NAME;
        const char *FORMAT; //Y8 or YV12
        const MSize IMG_SIZE;
        const int BUFFER_SIZE;
        N3D_DEBUG_BUFFER(const char *name, const char *format, MSize size, int bufferSize)
            : NAME(name)
            , FORMAT(format)
            , IMG_SIZE(size)
            , BUFFER_SIZE(bufferSize)
        {
        }
    };

    MSize szN3DOutput(m_algoInitInfo.iio_main.out_w, m_algoInitInfo.iio_main.out_h);
    MSize szLDC(m_algoInitInfo.iio_cmp.out_w, m_algoInitInfo.iio_cmp.out_h);
    vector<N3D_DEBUG_BUFFER> debugBuffers =
    {
        {"MV_Y",    "Y8", szN3DOutput, szN3DOutput.w*szN3DOutput.h},
        {"MASK_M",  "Y8", szN3DOutput, szN3DOutput.w*szN3DOutput.h},
        {"SV_Y",    "Y8", szN3DOutput, szN3DOutput.w*szN3DOutput.h},
        {"MASK_S",  "Y8", szN3DOutput, szN3DOutput.w*szN3DOutput.h},
        {"LDC",     "Y8", szLDC,       szLDC.w*szLDC.h},
    };

    char strImageSize[debugBuffers.size()][10]; //1234x5678

    Value buffer_list(kArrayType);
    for(size_t i = 0; i < debugBuffers.size(); ++i) {
        Value buffer_info(kObjectType);
        Value buffer_name(kObjectType);
        buffer_info.AddMember(EXTRA_DATA_NAME, StringRef(debugBuffers[i].NAME, strlen(debugBuffers[i].NAME)), allocator);
        buffer_info.AddMember(EXTRA_DATA_IMAGE_FORMAT, StringRef(debugBuffers[i].FORMAT, strlen(debugBuffers[i].FORMAT)), allocator);
        sprintf(strImageSize[i], "%dx%d", debugBuffers[i].IMG_SIZE.w, debugBuffers[i].IMG_SIZE.h);
        buffer_info.AddMember(EXTRA_DATA_IMAGE_SIZE, StringRef(strImageSize[i], strlen(strImageSize[i])), allocator);
        buffer_info.AddMember(EXTRA_DATA_BUFFER_SIZE, debugBuffers[i].BUFFER_SIZE, allocator);
        buffer_list.PushBack(buffer_info.Move(), allocator);
    }
    document.AddMember(EXTRA_DATA_BUFFER_LIST, buffer_list, allocator);

    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

    const char *header = sb.GetString();
    if(header) {
        const int STR_LEN = strlen(header);
        if(STR_LEN > 0) {
            __debugBufferHeader = new char[STR_LEN+1];
            strcpy(__debugBufferHeader, header);
        }
    }

    return __debugBufferHeader;
}

void
N3D_HAL_IMP::_prepareDebugBuffer(MUINT8 *debugBuffer)
{
    if(NULL == debugBuffer) {
        return;
    }

    _prepareDebugBufferHeader();
    MSize bufferSize(m_algoInitInfo.iio_main.out_w, m_algoInitInfo.iio_main.out_h);
    MSize ldcSize(m_algoInitInfo.iio_cmp.out_w, m_algoInitInfo.iio_cmp.out_h);
    const size_t INT_SIZE = sizeof(int);
    const size_t HEADER_SIZE = strlen(__debugBufferHeader)+1;
    const size_t DEBUG_BUFFER_SIZE = INT_SIZE*2 + HEADER_SIZE +
                                     bufferSize.w * bufferSize.h * 4 + ldcSize.w*ldcSize.h;

    ::memcpy(debugBuffer, &DEBUG_BUFFER_SIZE, INT_SIZE);
    ::memcpy(debugBuffer+INT_SIZE, &HEADER_SIZE, INT_SIZE);
    ::memcpy(debugBuffer+INT_SIZE*2, __debugBufferHeader, HEADER_SIZE);
    __debugContentOffset = INT_SIZE * 2 + HEADER_SIZE;
}

void
N3D_HAL_IMP::_dumpDebugBuffer(MUINT8 *debugBuffer)
{
    if( !DUMP_CAPTURE ||
        debugBuffer == nullptr
      )
    {
        return;
    }

    _mkdir();
    const size_t DEBUG_BUFFER_SIZE = (size_t)*((int*)debugBuffer);
    char dumpPath[PATH_MAX+1];
    char folder[PATH_MAX+1];
    _getDumpFolderName(m_requestNumber, folder);
    sprintf(dumpPath, "%s/N3D_DEBUG_BUFFER.data", _getDumpFolderName(m_requestNumber, folder));
    FILE *fp = fopen(dumpPath, "wb");
    if(fp) {
        fwrite(debugBuffer, 1, DEBUG_BUFFER_SIZE, fp);
        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Cannot open %s", dumpPath);
    }
}

char *
N3D_HAL_IMP::_getDumpFolderName(int folderNumber, char path[])
{
    const char *DIRECTION = (StereoSettingProvider::stereoProfile() == STEREO_SENSOR_PROFILE_FRONT_FRONT) ? "Front" : "Rear";
    if(eSTEREO_SCENARIO_CAPTURE == m_eScenario) {
        sprintf(path, "/sdcard/vsdof/capture/%s/%d/N3DNode", DIRECTION, folderNumber); //tmp solution
    } else {
        sprintf(path, "/sdcard/vsdof/pv_vr/%s/%d/N3DNode", DIRECTION, folderNumber);
    }

    return path;
}


void
N3D_HAL_IMP::_mkdir()
{
    if( !RUN_N3D ||
        !(DUMP_NVRAM || DUMP_CAPTURE || DUMP_PREVIEW) ||
        m_requestNumber < DUMP_START
      )
    {
        return;
    }

    char path[PATH_MAX+1] = {0};
    _getDumpFolderName(m_requestNumber, path);
    MY_LOGD("mkdir %s", path);
    #define DELIM "/"

    struct stat st;
    ::memset(&st, 0, sizeof(struct stat));
    if(stat(path, &st) == -1) {
        char *folder = strtok(path, DELIM);
        char createPath[PATH_MAX+1] = {0};
        createPath[0] = '/';
        while(folder) {
            strcat(createPath, folder);
            if (mkdir (createPath, 0755) != 0 && errno != EEXIST) {
                MY_LOGE("Create %s failed, error: %s", createPath, strerror(errno));
                break;
            }

            folder = strtok(NULL, DELIM);
            strcat(createPath, DELIM);
        }
    }
}

void
N3D_HAL_IMP::_dumpNVRAM(bool isInput)
{
    if( !RUN_N3D ||
        NULL == m_pVoidGeoData ||
        !(DUMP_NVRAM || DUMP_CAPTURE || DUMP_PREVIEW) ||
        m_requestNumber < DUMP_START
      )
    {
        return;
    }

    _mkdir();

    char dumpPath[PATH_MAX+1];
    char folder[PATH_MAX+1];
    if(isInput) {
        sprintf(dumpPath, "%s/N3D_nvram_in", _getDumpFolderName(m_requestNumber, folder));
    } else {
        sprintf(dumpPath, "%s/N3D_nvram_out", _getDumpFolderName(m_requestNumber, folder));

        // Extract NVRAM content from N3D
        MINT32 err = 0; // 0: no error. other value: error.
        err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SAVE_NVRAM,
                                                    (void*)&m_pVoidGeoData->StereoNvramData.StereoData, NULL);
        if (err)
        {
            MY_LOGE("StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SAVE_NVRAM) fail. error code: %d.", err);
            return;
        }
    }

    FILE *fpNVRAM = NULL;
    fpNVRAM = fopen(dumpPath, "wb");
    if(fpNVRAM) {
        fwrite(m_pVoidGeoData->StereoNvramData.StereoData, 1, sizeof(MUINT32)*MTK_STEREO_KERNEL_NVRAM_LENGTH, fpNVRAM);
        fflush(fpNVRAM);
        fclose(fpNVRAM);
        fpNVRAM = NULL;
    } else {
        MY_LOGE("Cannot dump NVRAM to %s, error: %s", dumpPath, strerror(errno));
    }
}

void
N3D_HAL_IMP::_dumpN3DLog()
{
    if( !RUN_N3D ||
        !(DUMP_CAPTURE || DUMP_PREVIEW) ||
        m_requestNumber < DUMP_START
      )
    {
        return;
    }

    _mkdir();

    char dumpPath[PATH_MAX+1];
    char folder[PATH_MAX+1];
    sprintf(dumpPath, "%s/N3D_Algo_Debug.log", _getDumpFolderName(m_requestNumber, folder));
    FILE *fp = NULL;
    fp = fopen(dumpPath, "w");
    if(fp) {
        m_pStereoDrv->StereoKernelFeatureCtrl( STEREO_KERNEL_FEATURE_DEBUG, fp, (void *)"XXX_DEBUG_INFO" );
        fflush(fp);
        fclose(fp);
        fp = NULL;
    } else {
        MY_LOGE("Cannot save N3D Log to %s, error: %s", dumpPath, strerror(errno));
    }
}