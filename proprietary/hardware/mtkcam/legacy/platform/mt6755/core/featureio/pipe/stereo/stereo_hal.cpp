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

#define LOG_TAG "stereo_hal"

#include <stdlib.h>     // for rand()

#include <cutils/properties.h>

#include <mtkcam/Log.h>
#include "MTKStereoKernelScenario.h"    // For MTKStereoKernel class/INPUT_FORMAT_ENUM/STEREO_KERNEL_TUNING_PARA_STRUCT. Must be included before stereo_hal_base.h/stereo_hal.h.
#include "MTKStereoKernel.h"    // For MTKStereoKernel class/INPUT_FORMAT_ENUM/STEREO_KERNEL_TUNING_PARA_STRUCT. Must be included before stereo_hal_base.h/stereo_hal.h.
#include "stereo_hal.h"         // For StereoHal class.
#include "pip_hal.h"            // For query max frame rate.

#include "IHalSensor.h"
#include "camera_custom_nvram.h"
#include "nvbuf_util.h"
#include "camera_custom_stereo.h"  // For CUST_STEREO_* definitions.
#include <android/hardware_buffer.h>
#include "aaa_hal_common.h" // For DAF_TBL_STRUCT.
#include <math.h>
#include <mtkcam/featureio/fd_hal_base.h>
#include "stereo_setting_provider.h"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

using namespace NSCam;
using namespace android;
using namespace rapidjson;
using namespace std;
using android::Mutex;           // For android::Mutex in stereo_hal.h.

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define ENABLE_GPU                  1

/*******************************************************************************
*
********************************************************************************/
#define STEREO_HAL_DEBUG

#ifdef STEREO_HAL_DEBUG    // Enable debug log.

#undef __func__
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
#endif  // STEREO_HAL_DEBUG

// teexit 20150320
//#include <limits.h>	// get max unsigned int
//

/* Sample code:
 * struct timespec t_start, t_end, t_result;
 * clock_gettime(CLOCK_MONOTONIC, &t_start);
 * ...
 * clock_gettime(CLOCK_MONOTONIC, &t_end);
 * t_result = timeDiff(t_start, t_end);
 * MY_LOGD("Runnning Time: %lu.%.9lu", t_result.tv_sec, t_result.tv_nsec);
 */
struct timespec timeDiff( struct timespec start, struct timespec end)
{
    struct timespec t_result;

    if( ( end.tv_nsec - start.tv_nsec ) < 0) {
        t_result.tv_sec = end.tv_sec - start.tv_sec - 1;
        t_result.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        t_result.tv_sec = end.tv_sec - start.tv_sec;
        t_result.tv_nsec = end.tv_nsec - start.tv_nsec;
    }

    return t_result;
}

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static StereoHalBase *pStereoHal = NULL;

STEREO_KERNEL_TUNING_PARA_STRUCT        gStereoKernelTuningParaInfo;
STEREO_KERNEL_SET_ENV_INFO_STRUCT       gStereoKernelInitInfo;
STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT  gStereoKernelWorkBufInfo;   // Working Buffer information.
STEREO_KERNEL_SET_PROC_INFO_STRUCT      gStereoKernelProcInfo;      // Setprocess information. Mostly addresses.
STEREO_KERNEL_RESULT_STRUCT             gStereoKernelResultInfo;    // STEREO algorithm results.

MFLOAT main1_FOV_horizontal = 78.0f;
//MFLOAT main1_FOV_vertical   = 65;
//MUINT main1_Capture_Width   = 4096;
//MUINT main1_Capture_Height  = 3072;
//MUINT main1_Capture_Offsetx = 0;
//MUINT main1_Capture_Offsety = 0;
MFLOAT main2_FOV_horizontal = 89.0f;
//MFLOAT main2_FOV_vertical   = 65;
//MUINT main2_Capture_Width   = 4096;
//MUINT main2_Capture_Height  = 3072;
//MUINT main2_Capture_Offsetx = 0;
//MUINT main2_Capture_Offsety = 0;
//MUINT FE_BLOCK_NUM          = 0;
//MUINT FE_IMAGE_WIDTH        = 1536;
//MUINT FE_IMAGE_HEIGHT       = 864;
//MUINT RGBA_IMAGE_WIDTH      = 160;
//MUINT RGBA_IMAGE_HEIGHT     = 90;

// Stereo ratio
const MFLOAT STEREO_FACTOR      = 1.0f;     // unit: %
const MFLOAT RRZ_CAPIBILITY     = 0.4f;

DAF_TBL_STRUCT *g_prDafTbl = NULL;
#define INVALID_QUERY 0xFFFFFFFF

//
/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
#define ENABLE_ALGO 1

#define STEREO_PROPERTY_PREFIX  "vendor.STEREO."
#define PROPERTY_ENABLE_VERIFY  STEREO_PROPERTY_PREFIX"enable_verify"
#define PERPERTY_ENABLE_CC      STEREO_PROPERTY_PREFIX"enable_cc"
#define PROPERTY_ALGO_BEBUG     STEREO_PROPERTY_PREFIX"dbgdump"
#define PROPERTY_NVRAM_BEBUG    STEREO_PROPERTY_PREFIX"dump_nvram"
#define PROPERTY_JSON_BEBUG     STEREO_PROPERTY_PREFIX"dump_json"

#define MODULE_ROTATE 0   //Should align to the result in JPS

/**
 * @param PROPERTY_NAME The property to query, e.g. "vendor.STEREO.enable_verify"
 * @param HAS_DEFAULT If ture, refer to DEFAULT value
 * @param DEFAULT Default value of the property. If not set, it will be 0.
 * @return: -1: property not been set; otherwise the property value; if HAS_DEFAULT return >= 0
 */
inline const int checkStereoProperty(const char *PROPERTY_NAME, const bool HAS_DEFAULT=true, const int DEFAULT=0)
{
    char val[PROPERTY_VALUE_MAX];
    ::memset(val, 0, sizeof(char)*PROPERTY_VALUE_MAX);

    int len = 0;
    if(HAS_DEFAULT) {
        char strDefault[PROPERTY_VALUE_MAX];
        sprintf(strDefault, "%d", DEFAULT);
        len = property_get(PROPERTY_NAME, val, strDefault);
    } else {
        len = property_get(PROPERTY_NAME, val, NULL);
    }

    if(len <= 0) {
        return -1; //property not exist
    }

    return (!strcmp(val, "1"));
}

/*******************************************************************************
*
********************************************************************************/
StereoHal::
StereoHal()
    : mUsers(0)
    , m_pStereoDrv(NULL)
    , mScenario(STEREO_SCENARIO_UNKNOWN) // default value
    , mAlgoSize()
    , mMainSize()
    , mFEImgSize(MSize(960, 544)*STEREO_FACTOR)
    , m_pFullMask(NULL)
    , m_uSensorRelativePosition(0)  // L-R 0: main-minor , 1: minor-main
    , m_nMain1SensorIdx(0)
    , m_nPrecropDegree(12)
    , m_debugInfo(NULL)
{
}


/*******************************************************************************
*
********************************************************************************/
StereoHal::~StereoHal()
{

}

StereoHal* StereoHal::
createInstance()
{
    StereoHal *pStereoHal = StereoHal::getInstance();
    pStereoHal->init();
    return pStereoHal;
}

/*******************************************************************************
*
********************************************************************************/
StereoHal* StereoHal::
getInstance()
{
    MY_LOGD("StereoHal getInstance.");
    static StereoHal singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
StereoHal::
destroyInstance()
{
    uninit();
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::init()
{
    MY_LOGD("- E. mUsers: %d.", mUsers);
    MBOOL Result = MTRUE;   // TRUE: no error. FALSE: error.
    MINT32 err = 0;
    Mutex::Autolock lock(mLock);

    //memset((void*)&gStereoKernelTuningParaInfo,0,sizeof(gStereoKernelTuningParaInfo));
    //memset((void*)&gStereoKernelInitInfo,0,sizeof(gStereoKernelInitInfo));
    //memset((void*)&gStereoKernelWorkBufInfo,0,sizeof(gStereoKernelWorkBufInfo));
    //memset((void*)&gStereoKernelProcInfo,0,sizeof(gStereoKernelProcInfo));
    //memset((void*)&gStereoKernelResultInfo,0,sizeof(gStereoKernelResultInfo));

    // Create StereoDrv instance.
    m_pStereoDrv = MTKStereoKernel::createInstance();
    if (!m_pStereoDrv)
    {
        MY_LOGE("MTKStereoKernel::createInstance() fail.");
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

    err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_DEFAULT_TUNING, NULL, &gStereoKernelTuningParaInfo);
    if (err)
    {
        MY_LOGE("StereoKernelFeatureCtrl(GET_DEFAULT_TUNING) fail. error code: %d.", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

    if (mUsers > 0)
    {
        MY_LOGD("StereoHal has already inited.");
        goto lb_Normal_Exit;
    }

    // Load learning data from NVRAM.
#if ENABLE_ALGO
    MY_LOGD("Load NVRAM");
    Result = _loadFromNvram();

    if( 1 == checkStereoProperty(PROPERTY_NVRAM_BEBUG) ) {
        MY_LOGD("=== Dump loaded NVRAM ===");
        for(int i=0; i < MTK_STEREO_KERNEL_NVRAM_LENGTH; i++) {
            MY_LOGD("[%4d] %d. ", i, m_pVoidGeoData->StereoNvramData.StereoData[i]);
        }
    }

    err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_LOAD_NVRAM,
                                                (void*)&m_pVoidGeoData->StereoNvramData.StereoData, NULL);
    if (err)
    {
        MY_LOGE("StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_LOAD_NVRAM) fail. error code: %d.", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

    if (!Result)    // Maybe no data in NVRAM, so read from EEPROM.
    {
        MY_LOGD("Load from NVRAM fail (Maybe 1st time so no data in NVRAM yet).");
    }
#endif

lb_Normal_Exit:
    android_atomic_inc(&mUsers);

    MY_LOGD("- X. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;

lb_Abnormal_Exit:
    // StereoDrv Init failed, destroy StereoDrv instance.
    if (m_pStereoDrv)
    {
        m_pStereoDrv->destroyInstance();
        m_pStereoDrv = NULL;
    }

    MY_LOGD("- X. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;

}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::uninit()
{
    MY_LOGD("uninit mUsers: %d.", mUsers);
    MBOOL Result = MTRUE;
    MINT32 err = 0;
    Mutex::Autolock lock(mLock);

    if (mUsers > 1)  // More than one user, so decrease one mUsers.
    {
        android_atomic_dec(&mUsers);
    }
    else if (mUsers == 1)   // Last user, must do some un-init procedure.
    {
        android_atomic_dec(&mUsers);

        // Save learning data to NVRAM.
#if ENABLE_ALGO
        err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SAVE_NVRAM,
                                                    (void*)&m_pVoidGeoData->StereoNvramData.StereoData, NULL);
        if (err)
        {
            MY_LOGE("StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SAVE_NVRAM) fail. error code: %d.", err);
            Result = MFALSE;
        }
        MY_LOGD("Save NVRAM");
        if( 1 == checkStereoProperty(PROPERTY_NVRAM_BEBUG) ) {
            MY_LOGD("=== Dump saving NVRAM ===");
            for(int i=0; i < MTK_STEREO_KERNEL_NVRAM_LENGTH; i++) {
                MY_LOGD("[%4d] %d. ", i, m_pVoidGeoData->StereoNvramData.StereoData[i]);
            }
        }
        Result = _saveToNvram();
#endif
        if (!Result)
        {
            MY_LOGD("Save to NVRAM fail.");
        }

        // Destroy StereoDrv instance.
        if (m_pStereoDrv)
        {
            m_pStereoDrv->destroyInstance();
            m_pStereoDrv = NULL;
        }

        if(m_debugInfo) {
            delete m_debugInfo;
            m_debugInfo = NULL;
        }
    }
    else // mUsers <= 0. No StereoHal user, do nothing.
    {
        // do nothing.
        MY_LOGW("No StereoHal to un-init.");
    }

    MY_LOGD("- X. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;

}

/*******************************************************************************
*
********************************************************************************/
void
StereoHal::setParameters(sp<IParamsManager> spParamsMgr)
{
/*
    // Stereo Feature
    const char MtkCameraParameters::KEY_STEREO_REFOCUS_MODE[] = "stereo-image-refocus";
    const char MtkCameraParameters::KEY_STEREO_DEPTHAF_MODE[] = "stereo-depth-af";
    const char MtkCameraParameters::KEY_STEREO_DISTANCE_MODE[] = "stereo-distance-measurement";
*/
//    mRefocusMode = ( ::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_STEREO_REFOCUS_MODE), MtkCameraParameters::ON) == 0 ) ? MTRUE : MFALSE;
//    mDepthAF = ( ::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_STEREO_DEPTHAF_MODE), MtkCameraParameters::ON) == 0 ) ? MTRUE : MFALSE;
//    mDistanceMeasure = ( ::strcmp(spParamsMgr->getStr(MtkCameraParameters::KEY_STEREO_DISTANCE_MODE), MtkCameraParameters::ON) == 0 ) ? MTRUE : MFALSE;
}

/*******************************************************************************
*
********************************************************************************/
MSize
StereoHal::getMainSize(MSize const imgSize) const
{
    return MSize(imgSize.w * STEREO_FACTOR, imgSize.h * STEREO_FACTOR);
}

/*******************************************************************************
*
********************************************************************************/
MSize
StereoHal::getAlgoInputSize(MUINT32 const SENSOR_INDEX) const
{
	// idx: 0 means main sensor, otherwise, main2 sensor.
//#if ENABLE_ALGO
//    return (SENSOR_INDEX == m_nMain1SensorIdx) ?
//        MSize(gStereoKernelInitInfo.remap_main.rrz_out_width,  gStereoKernelInitInfo.remap_main.rrz_out_height) :
//        MSize(gStereoKernelInitInfo.remap_minor.rrz_out_width, gStereoKernelInitInfo.remap_minor.rrz_out_height);
//#else
//    return (SENSOR_INDEX == m_nMain1SensorIdx) ? StereoSettingProvider::imageResolution().szMainCam : StereoSettingProvider::imageResolution().szSubCam;
//#endif

    return (SENSOR_INDEX == m_nMain1SensorIdx) ? StereoSettingProvider::imageResolution().szMainCam : StereoSettingProvider::imageResolution().szSubCam;
}

/*******************************************************************************
*
********************************************************************************/
MSize
StereoHal::getRrzSize(MUINT32 const SENSOR_INDEX) const
{
    // idx: 0 means main sensor, otherwise, main2 sensor.
#if ENABLE_ALGO
    return (SENSOR_INDEX == m_nMain1SensorIdx) ?
        MSize(gStereoKernelInitInfo.remap_main.rrz_out_width,  gStereoKernelInitInfo.remap_main.rrz_out_height) :
        MSize(gStereoKernelInitInfo.remap_minor.rrz_out_width, gStereoKernelInitInfo.remap_minor.rrz_out_height);
        // MSize(1600,912);
#else
    return (SENSOR_INDEX == m_nMain1SensorIdx) ? StereoSettingProvider::imageResolution().szMainCam : StereoSettingProvider::imageResolution().szSubCam;
#endif
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREOGetRrzInfo(RRZ_DATA_STEREO_T &OutData) const
{
#if ENABLE_ALGO
    OutData.rrz_crop_main1  = MRect(MPoint(gStereoKernelInitInfo.remap_main.rrz_offset_x,
                                           gStereoKernelInitInfo.remap_main.rrz_offset_y),
                                    MSize(gStereoKernelInitInfo.remap_main.rrz_usage_width,
                                          gStereoKernelInitInfo.remap_main.rrz_usage_height));
    OutData.rrz_size_main1  = getRrzSize(m_nMain1SensorIdx); //MSize(gStereoKernelInitInfo.remap_main.rrz_out_width,gStereoKernelInitInfo.remap_main.rrz_out_height);
    OutData.rrz_crop_main2  = MRect(MPoint(gStereoKernelInitInfo.remap_minor.rrz_offset_x,
                                           gStereoKernelInitInfo.remap_minor.rrz_offset_y),
                                    MSize(gStereoKernelInitInfo.remap_minor.rrz_usage_width,
                                          gStereoKernelInitInfo.remap_minor.rrz_usage_height));
    OutData.rrz_size_main2  = getRrzSize(m_nMain2SensorIdx); //MSize(gStereoKernelInitInfo.remap_minor.rrz_out_width,gStereoKernelInitInfo.remap_minor.rrz_out_height);
#else
    //TODO: update for sensor setting
    OutData.rrz_crop_main1  = MRect(MPoint(0,100), MSize(2096,1178));
    OutData.rrz_size_main1  = getRrzSize(m_nMain1SensorIdx);
    OutData.rrz_crop_main2  = MRect(MPoint(0,240), MSize(2560,1440));
    OutData.rrz_size_main2  = getRrzSize(m_nMain2SensorIdx);
#endif
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREOGetInfo(HW_DATA_STEREO_T &OutData) const
{
    MBOOL ret = MTRUE;

    //SWFE doesn't need hwfe_block_size
//    switch (FE_BLOCK_NUM)
//    {
//    	 case 0:
//           OutData.hwfe_block_size     = 32;
//       break;
//       case 1:
//           OutData.hwfe_block_size     = 16;
//       break;
//       case 2:
//           OutData.hwfe_block_size     = 8;
//       break;
//    }
#if ENABLE_ALGO
    OutData.rgba_image_width    = gStereoKernelTuningParaInfo.rgba_in_width;
    OutData.rgba_image_height   = gStereoKernelTuningParaInfo.rgba_in_height;

    //SWFE doesn't need to calculate size
    OutData.fefm_image_width    = gStereoKernelTuningParaInfo.fefm_in_width;
    OutData.fefm_imgae_height   = gStereoKernelTuningParaInfo.fefm_in_height;
#else
    OutData.rgba_image_width    = 160;
    OutData.rgba_image_height   = 90;

    //SWFE doesn't need to calculate size
    OutData.fefm_image_width    = getFEImgSize().w;
    OutData.fefm_imgae_height   = getFEImgSize().h;
#endif
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREOInit(INIT_DATA_STEREO_IN_T InData, INIT_DATA_STEREO_OUT_T &OutData)
{
    MBOOL bResult = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    int nCheckCC = -1;  //must declare before goto
    if (m_pWorkBuf) {
        free(m_pWorkBuf);
    }
    m_pWorkBuf = NULL;

    //
    m_nCapOrientation = InData.orientation;
    STEREO_KERNEL_SCENARIO_ENUM algoScenario = STEREO_KERNEL_SCENARIO_IMAGE_PREVIEW;    // default value
    mScenario = InData.eScenario;
    switch ( mScenario )
    {
        case STEREO_SCENARIO_PREVIEW:
            MY_LOGD("mScenario = STEREO_SCENARIO_PREVIEW %d", STEREO_SCENARIO_PREVIEW);
            algoScenario = STEREO_KERNEL_SCENARIO_IMAGE_PREVIEW;
            break;
        case STEREO_SCENARIO_RECORD:
            MY_LOGD("mScenario = STEREO_SCENARIO_RECORD %d", STEREO_SCENARIO_RECORD);
            algoScenario = STEREO_KERNEL_SCENARIO_VIDEO_RECORD;
            break;
        case STEREO_SCENARIO_CAPTURE:
            MY_LOGD("mScenario = STEREO_SCENARIO_CAPTURE %d", STEREO_SCENARIO_CAPTURE);
            algoScenario = STEREO_KERNEL_SCENARIO_IMAGE_CAPTURE_ONLY;
            break;
        default:
            MY_LOGE("unsupport scenario(%d)", mScenario);
            break;
    }

    StereoSettingProvider::setImageRatio(InData.eImageRatio);
    _setAlgoImgSize( InData.algo_image_size );
    _setMainImgSize( InData.main_image_size );

#if ENABLE_ALGO
    SensorFOV fovMain1, fovMain2;
    getStereoFOV(STEREO_SENSOR_PROFILE_REAR_REAR, fovMain1, fovMain2);

    SensorFOV targetFovMain1, targetFovMain2;
    getStereoTargetFOV(STEREO_SENSOR_PROFILE_REAR_REAR, targetFovMain1, targetFovMain2);

    main1_FOV_horizontal = targetFovMain1.fov_horizontal;
    main2_FOV_horizontal = targetFovMain2.fov_horizontal;

    m_nMain1SensorIdx = InData.main1_sensor_index;
    IHalSensorList* sensorlsit = IHalSensorList::get();
    IHalSensor* pIHalSensor = sensorlsit->createSensor("Stereo hal", m_nMain1SensorIdx);
    int sensorDevIdx = sensorlsit->querySensorDevIdx(m_nMain1SensorIdx);
    MY_LOGD("main1 sensorDevIdx %d. sensorIdx %d", sensorDevIdx, m_nMain1SensorIdx);
    MY_LOGD("InData.main1_sensor_index = %d. InData.main2_sensor_index = %d", InData.main1_sensor_index, InData.main2_sensor_index);

    //TODO: adjust to configurable
    MFLOAT fSourceImageRatioX = 1.0f;
    MFLOAT fSourceImageRatioY = 1.0f;

    gStereoKernelInitInfo.main_source_image_width  = InData.main_image_size.w * fSourceImageRatioX;
    gStereoKernelInitInfo.main_source_image_height = InData.main_image_size.h * fSourceImageRatioY;
    gStereoKernelInitInfo.main_output_image_width  = InData.main_image_size.w ;
    gStereoKernelInitInfo.main_output_image_height = InData.main_image_size.h;

    gStereoKernelInitInfo.algo_source_image_width  = StereoSettingProvider::imageResolution().szMainCam.w * fSourceImageRatioX;
    gStereoKernelInitInfo.algo_source_image_height = StereoSettingProvider::imageResolution().szMainCam.h * fSourceImageRatioY;
    gStereoKernelInitInfo.algo_output_image_width  = StereoSettingProvider::imageResolution().szMainCam.w + StereoSettingProvider::refocusSize().w;
    gStereoKernelInitInfo.algo_output_image_height = StereoSettingProvider::imageResolution().szMainCam.h + StereoSettingProvider::refocusSize().h;

    SensorCropWinInfo rSensorCropInfo;
    ::memset(&rSensorCropInfo, 0, sizeof(SensorCropWinInfo));
    SensorStaticInfo pSensorStaticInfo;

    if(sensorlsit) {
        // Get sensor info
        sensorlsit->querySensorStaticInfo(sensorDevIdx, &pSensorStaticInfo);
        // Get FOV
        main1_FOV_horizontal = pSensorStaticInfo.horizontalViewAngle;
        //main1_FOV_vertical = pSensorStaticInfo.verticalViewAngle;
        MY_LOGD("Main sensor FOV = %.1f", main1_FOV_horizontal);
        gStereoKernelInitInfo.hori_fov_main = main1_FOV_horizontal;

        // Get crop info
        err = pIHalSensor->sendCommand(sensorDevIdx, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                       (MUINTPTR)&InData.main1_sensor_scenario, (MUINTPTR)&rSensorCropInfo, 0);
        if(err)
        {
            MY_LOGE("SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO() fail. error code: %d.", err);
            bResult = MFALSE;
            goto lb_Abnormal_Exit;
        }

        // Set remap info for algo
        gStereoKernelInitInfo.remap_main.pixel_array_width  = rSensorCropInfo.full_w;
        gStereoKernelInitInfo.remap_main.pixel_array_height = rSensorCropInfo.full_h ;
        gStereoKernelInitInfo.remap_main.sensor_offset_x0   = rSensorCropInfo.x0_offset ;
        gStereoKernelInitInfo.remap_main.sensor_offset_y0   = rSensorCropInfo.y0_offset ;
        gStereoKernelInitInfo.remap_main.sensor_size_w0     = rSensorCropInfo.w0_size ;
        gStereoKernelInitInfo.remap_main.sensor_size_h0     = rSensorCropInfo.h0_size ;
        gStereoKernelInitInfo.remap_main.sensor_scale_w     = rSensorCropInfo.scale_w ;
        gStereoKernelInitInfo.remap_main.sensor_scale_h     = rSensorCropInfo.scale_h ;
        gStereoKernelInitInfo.remap_main.sensor_offset_x1   = rSensorCropInfo.x1_offset ;
        gStereoKernelInitInfo.remap_main.sensor_offset_y1   = rSensorCropInfo.y1_offset ;
        gStereoKernelInitInfo.remap_main.tg_offset_x        = rSensorCropInfo.x2_tg_offset ;
        gStereoKernelInitInfo.remap_main.tg_offset_y        = rSensorCropInfo.y2_tg_offset ;

//        MUINT32 uMaxRRZSize = (MUINT32)ceil(gStereoKernelInitInfo.remap_main.rrz_usage_width * RRZ_CAPIBILITY);
//        if(uMaxRRZSize & 0x1) { uMaxRRZSize++ ; }   //rrz_out_width must be even number
//        if(uMaxRRZSize > gStereoKernelInitInfo.algo_source_image_width) {
//            gStereoKernelInitInfo.remap_main.rrz_out_width = uMaxRRZSize;
//        } else {
//            gStereoKernelInitInfo.remap_main.rrz_out_width = gStereoKernelInitInfo.algo_source_image_width;
//        }
//
//        uMaxRRZSize = (MUINT32)ceil(gStereoKernelInitInfo.remap_main.rrz_usage_height * RRZ_CAPIBILITY);
//        if(uMaxRRZSize & 0x1) { uMaxRRZSize++ ; }   //rrz_out_width must be even number
//        if(uMaxRRZSize > gStereoKernelInitInfo.algo_source_image_height) {
//            gStereoKernelInitInfo.remap_main.rrz_out_height  = uMaxRRZSize;
//        } else {
//            gStereoKernelInitInfo.remap_main.rrz_out_height  = gStereoKernelInitInfo.algo_source_image_height;
//        }
        gStereoKernelInitInfo.remap_main.rrz_out_width  = gStereoKernelInitInfo.algo_source_image_width;
        gStereoKernelInitInfo.remap_main.rrz_out_height = gStereoKernelInitInfo.algo_source_image_height;

        //============ Precrop main1 if FOV and target FOV is different =============
        if(fovMain1.fov_horizontal - targetFovMain1.fov_horizontal < 0.0001f)    //Prevent floating error
        {
            gStereoKernelInitInfo.remap_main.rrz_usage_width    = rSensorCropInfo.w2_tg_size;      //sensor out width;
            gStereoKernelInitInfo.remap_main.rrz_usage_height   = (rSensorCropInfo.w2_tg_size *
                                                                   StereoSettingProvider::imageResolution().uRatioNumerator/
                                                                   StereoSettingProvider::imageResolution().uRatioDenomerator)
                                                                  & ~1; //Image size must be even
        } else {
            //Cropped size = original size * tan(croped-FOV) / tan(original-FOV)
            MSize croppedSize;
            croppedSize.w = (int)(rSensorCropInfo.w2_tg_size * StereoSettingProvider::getFOVCropRatio()) & ~1;
            croppedSize.h = (croppedSize.w *
                             StereoSettingProvider::imageResolution().uRatioNumerator /
                             StereoSettingProvider::imageResolution().uRatioDenomerator)
                            & ~1;   //Image size must be even

            MY_LOGD("Pre-crop main1: FOV(H) %.1f -> %1.f, size: %dx%d -> %dx%d",
                    fovMain1.fov_horizontal, targetFovMain1.fov_horizontal,
                    rSensorCropInfo.w2_tg_size, rSensorCropInfo.h2_tg_size,
                    croppedSize.w, croppedSize.h);

            gStereoKernelInitInfo.remap_main.rrz_usage_width  = croppedSize.w;
            gStereoKernelInitInfo.remap_main.rrz_usage_height = croppedSize.h;
        }
        //===========================================================================
        gStereoKernelInitInfo.remap_main.rrz_offset_x       = ((rSensorCropInfo.w2_tg_size - gStereoKernelInitInfo.remap_main.rrz_usage_width )>>1 ) ;
        gStereoKernelInitInfo.remap_main.rrz_offset_y       = ((rSensorCropInfo.h2_tg_size - gStereoKernelInitInfo.remap_main.rrz_usage_height)>>1 ) ;

        MY_LOGD("main cam");
        MY_LOGD("main full_w %d. main_full_h %d",rSensorCropInfo.full_w,rSensorCropInfo.full_h);
        MY_LOGD("x0_offset %d. y0_offset %d. w0_size %d. h0_size %d.",rSensorCropInfo.x0_offset,rSensorCropInfo.y0_offset,rSensorCropInfo.w0_size,rSensorCropInfo.h0_size);
        MY_LOGD("scale_w %d. scale_h %d",rSensorCropInfo.scale_w,rSensorCropInfo.scale_h);
        MY_LOGD("x1_offset %d. y1_offset %d. w1_size %d. h1_size %d.",rSensorCropInfo.x1_offset,rSensorCropInfo.y1_offset,rSensorCropInfo.w1_size,rSensorCropInfo.h1_size);
        MY_LOGD("x2_tg_offset %d. y2_tg_offset %d. w2_tg_size %d. h2_tg_size %d.",rSensorCropInfo.x2_tg_offset,rSensorCropInfo.y2_tg_offset,rSensorCropInfo.w2_tg_size,rSensorCropInfo.h2_tg_size);
    }

    m_nMain2SensorIdx = InData.main2_sensor_index;
    sensorDevIdx=sensorlsit->querySensorDevIdx(m_nMain2SensorIdx);
    MY_LOGD("main2 sensorDevIdx %d. sensorIdx %d", sensorDevIdx, m_nMain2SensorIdx);
    if(sensorlsit) {
        // Get sensor info
        sensorlsit->querySensorStaticInfo(sensorDevIdx, &pSensorStaticInfo);

        // Get FOV
        main2_FOV_horizontal = pSensorStaticInfo.horizontalViewAngle;
        MY_LOGD("Sub sensor FOV = %.1f", main2_FOV_horizontal);
        gStereoKernelInitInfo.hori_fov_minor = main2_FOV_horizontal;
        //main2_FOV_vertical=pSensorStaticInfo.verticalViewAngle;

        // Get crop info
        err = pIHalSensor->sendCommand(sensorDevIdx, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                       (MUINTPTR)&InData.main2_sensor_scenario, (MUINTPTR)&rSensorCropInfo, 0);
        if(err)
        {
            MY_LOGE("SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO() fail. error code: %d.", err);
            bResult = MFALSE;
            goto lb_Abnormal_Exit;
        }

        // Set remap info for algo
        gStereoKernelInitInfo.remap_minor.pixel_array_width     = rSensorCropInfo.full_w;
        gStereoKernelInitInfo.remap_minor.pixel_array_height    = rSensorCropInfo.full_h ;
        gStereoKernelInitInfo.remap_minor.sensor_offset_x0      = rSensorCropInfo.x0_offset ;
        gStereoKernelInitInfo.remap_minor.sensor_offset_y0      = rSensorCropInfo.y0_offset ;
        gStereoKernelInitInfo.remap_minor.sensor_size_w0        = rSensorCropInfo.w0_size ;
        gStereoKernelInitInfo.remap_minor.sensor_size_h0        = rSensorCropInfo.h0_size ;
        gStereoKernelInitInfo.remap_minor.sensor_scale_w        = rSensorCropInfo.scale_w ;
        gStereoKernelInitInfo.remap_minor.sensor_scale_h        = rSensorCropInfo.scale_h ;
        gStereoKernelInitInfo.remap_minor.sensor_offset_x1      = rSensorCropInfo.x1_offset ;
        gStereoKernelInitInfo.remap_minor.sensor_offset_y1      = rSensorCropInfo.y1_offset ;
        gStereoKernelInitInfo.remap_minor.tg_offset_x           = rSensorCropInfo.x2_tg_offset ;
        gStereoKernelInitInfo.remap_minor.tg_offset_y           = rSensorCropInfo.y2_tg_offset ;

        gStereoKernelInitInfo.remap_minor.rrz_out_width         = StereoSettingProvider::imageResolution().szSubCam.w ;
        gStereoKernelInitInfo.remap_minor.rrz_out_height        = StereoSettingProvider::imageResolution().szSubCam.h ;
//        if(gStereoKernelInitInfo.remap_minor.pixel_array_width>2000) //check sensor size is 5M or 2M
//        {
//            gStereoKernelInitInfo.remap_minor.rrz_out_width  = 1920 ;
//            gStereoKernelInitInfo.remap_minor.rrz_out_height = 1080 ;
//        }
//        else
//        {
//            gStereoKernelInitInfo.remap_minor.rrz_out_width  = 1280 ;
//            gStereoKernelInitInfo.remap_minor.rrz_out_height = 720 ;
//        }

        //Pre-crop for ZSD
        MY_LOGD("[Main2] Before precrop rrz_usage_width  %d", gStereoKernelInitInfo.remap_minor.rrz_usage_width);
        MY_LOGD("[Main2] Before precrop rrz_usage_height %d", gStereoKernelInitInfo.remap_minor.rrz_usage_height);
        if(main2_FOV_horizontal > (main1_FOV_horizontal + m_nPrecropDegree))
        {
            gStereoKernelInitInfo.remap_minor.rrz_usage_width  = (MINT32)( tan((main1_FOV_horizontal+m_nPrecropDegree)/180.0f*acos(0.0f)) / tan(main2_FOV_horizontal/180.0f*acos(0.0f)) * rSensorCropInfo.w2_tg_size/32 )*32 ;
            gStereoKernelInitInfo.hori_fov_minor = main1_FOV_horizontal + m_nPrecropDegree ;

            MY_LOGD("[Main2] Precrop rrz_usage_width to %d", gStereoKernelInitInfo.remap_minor.rrz_usage_width);
	        MY_LOGD("[Main2] Pre-crop FOV to %.1f", gStereoKernelInitInfo.hori_fov_minor);
        }
        else
        {
	         gStereoKernelInitInfo.remap_minor.rrz_usage_width = rSensorCropInfo.w2_tg_size;     //sensor out width ;
	         MY_LOGD("[Main2] Set rrz_usage_width to %d", gStereoKernelInitInfo.remap_minor.rrz_usage_width);
        }

        gStereoKernelInitInfo.remap_minor.rrz_usage_width  = rSensorCropInfo.w2_tg_size;     //sensor out width ;
        gStereoKernelInitInfo.remap_minor.rrz_usage_height = (gStereoKernelInitInfo.remap_minor.rrz_usage_width *
                                                              StereoSettingProvider::imageResolution().uRatioNumerator /
                                                              StereoSettingProvider::imageResolution().uRatioDenomerator)
                                                             & ~1;  //Image size must be even
       	gStereoKernelInitInfo.remap_minor.rrz_offset_x = ((rSensorCropInfo.w2_tg_size - gStereoKernelInitInfo.remap_minor.rrz_usage_width )>>1 ) ;
        gStereoKernelInitInfo.remap_minor.rrz_offset_y = ((rSensorCropInfo.h2_tg_size - gStereoKernelInitInfo.remap_minor.rrz_usage_height)>>1 ) ;

        MY_LOGD("minor cam");
        MY_LOGD("main full_w %d. main_full_h %d",rSensorCropInfo.full_w,rSensorCropInfo.full_h);
        MY_LOGD("x0_offset %d. y0_offset %d. w0_size %d. h0_size %d.",rSensorCropInfo.x0_offset,rSensorCropInfo.y0_offset,rSensorCropInfo.w0_size,rSensorCropInfo.h0_size);
        MY_LOGD("scale_w %d. scale_h %d",rSensorCropInfo.scale_w,rSensorCropInfo.scale_h);
        MY_LOGD("x1_offset %d. y1_offset %d. w1_size %d. h1_size %d.",rSensorCropInfo.x1_offset,rSensorCropInfo.y1_offset,rSensorCropInfo.w1_size,rSensorCropInfo.h1_size);
        MY_LOGD("x2_tg_offset %d. y2_tg_offset %d. w2_tg_size %d. h2_tg_size %d.",rSensorCropInfo.x2_tg_offset,rSensorCropInfo.y2_tg_offset,rSensorCropInfo.w2_tg_size,rSensorCropInfo.h2_tg_size);
    }
    pIHalSensor->destroyInstance("Stereo hal");

    //Give init tuning params to algo
    gStereoKernelInitInfo.ptuning_para  = &gStereoKernelTuningParaInfo; //got in init()
    gStereoKernelInitInfo.scenario      = algoScenario;


    //Set HWFE block number
//    switch (FE_BLOCK_NUM)
//    {
//    case 0:
//        gStereoKernelInitInfo.hwfe_block_size   = 32;
//        break;
//    case 1:
//        gStereoKernelInitInfo.hwfe_block_size   = 16;
//        break;
//    case 2:
//        gStereoKernelInitInfo.hwfe_block_size   = 8;
//        break;
//    }

    //SWFE doesn't need hwfe_block_size
    gStereoKernelInitInfo.hwfe_block_size = 0;  //0 means SWFE

    mAlgoSize.w         = gStereoKernelInitInfo.algo_source_image_width;
    mAlgoSize.h         = gStereoKernelInitInfo.algo_source_image_height;
    mAlgoAppendSize.w   = gStereoKernelInitInfo.algo_source_image_width  + StereoSettingProvider::refocusSize().w;
    mAlgoAppendSize.h   = gStereoKernelInitInfo.algo_source_image_height + StereoSettingProvider::refocusSize().h;
    mMainSize.w         = gStereoKernelInitInfo.main_source_image_width;
    mMainSize.h         = gStereoKernelInitInfo.main_source_image_height;
    mFEImgSize.w        = gStereoKernelTuningParaInfo.fefm_in_width;
    mFEImgSize.h        = gStereoKernelTuningParaInfo.fefm_in_height;

    //Set image size to algo
    gStereoKernelInitInfo.fefm_image_width  = gStereoKernelTuningParaInfo.fefm_in_width;
    gStereoKernelInitInfo.fefm_image_height = gStereoKernelTuningParaInfo.fefm_in_height;
    gStereoKernelInitInfo.rgba_image_width  = gStereoKernelTuningParaInfo.rgba_in_width;
    gStereoKernelInitInfo.rgba_image_height = gStereoKernelTuningParaInfo.rgba_in_height;
    gStereoKernelInitInfo.rgba_image_stride = gStereoKernelTuningParaInfo.rgba_in_width;   //it should 16 align
    gStereoKernelInitInfo.rgba_image_depth  = 4;    //4 bytes

    //Get sensor relative position
    customSensorPos_STEREO_t SensorPos;
    SensorPos = getSensorPosSTEREO();
    m_uSensorRelativePosition = SensorPos.uSensorPos;
    gStereoKernelInitInfo.sensor_config         = SensorPos.uSensorPos ; // L-R 0: main-minor , 1: minor-main
    gStereoKernelInitInfo.enable_cc             = 0;   //enable color correction: 1
    gStereoKernelInitInfo.enable_gpu            = 0;

    gStereoKernelInitInfo.stereo_baseline       = STEREO_BASELINE;
    gStereoKernelInitInfo.support_diff_fov_fm   = 1;    // 0: same FOVs,  1: diff FOVs

    //=== For SWFE ===
    gStereoKernelInitInfo.fefm_image_stride     = getFEImgSize().w;
    gStereoKernelInitInfo.fefm_image_format     = 0;   // 0: YV12, 1: RGBA
//    if(pSensorStaticInfo) {
//        MY_LOGD("Sensor type = %d", pSensorStaticInfo->sensorType);
//        gStereoKernelInitInfo.fefm_image_format = (pSensorStaticInfo->sensorType == IMAGE_SENSOR_TYPE_YUV) ? 0 : 1;
//    } else {
//        gStereoKernelInitInfo.fefm_image_format = 0;   // 0: YV12, 1: RGBA
//    }

    //TODO: read from custom file
    gStereoKernelInitInfo.module_orientation = 0;   // 0: horizontal, 1: vertical
    //=== For SWFE ===

    //Get min/max dac
    mpHal3A = IHal3A::createInstance(IHal3A::E_Camera_1, m_nMain1SensorIdx, "MTKStereoCamera");
    mpHal3A->send3ACtrl(E3ACtrl_GetDAFTBL, (MUINTPTR)&g_prDafTbl, 0);
    MY_LOGD("g_rDafTbl %p", &g_prDafTbl);

    //Since af_mgr::init may run later, we have to wait for it
    for(int nTimes = 10; nTimes > 0; nTimes--) {
        gStereoKernelInitInfo.af_dac_start = g_prDafTbl->af_dac_min;
        if (0 == gStereoKernelInitInfo.af_dac_start) {
            MY_LOGD("Waiting for af_dac_min...");
            usleep(20 * 1000);
        } else {
            break;
        }
    }

    if (0 == gStereoKernelInitInfo.af_dac_start) {
        MY_LOGE("Cannot get af_dac_min");
    }

    if (STEREO_KERNEL_SCENARIO_IMAGE_CAPTURE_ONLY == gStereoKernelInitInfo.scenario)
    {
        //Set algo image size
//        gStereoKernelInitInfo.algo_source_image_width   = InData.algo_image_size.w * fSourceImageRatioX - StereoSettingProvider::refocusSize().w;
//        gStereoKernelInitInfo.algo_source_image_height  = InData.algo_image_size.h * fSourceImageRatioY - StereoSettingProvider::refocusSize().h;
//        gStereoKernelInitInfo.algo_output_image_width   = InData.algo_image_size.w;
//        gStereoKernelInitInfo.algo_output_image_height  = InData.algo_image_size.h;

        gStereoKernelInitInfo.remap_minor.rrz_out_width  = StereoSettingProvider::imageResolution().szSubCam.w ;
        gStereoKernelInitInfo.remap_minor.rrz_out_height = StereoSettingProvider::imageResolution().szSubCam.h ;
//        if(gStereoKernelInitInfo.remap_minor.pixel_array_width > 2000) //check sensor size is 5M or 2M
//        {
//            gStereoKernelInitInfo.remap_minor.rrz_out_width  = 1920 ;
//            gStereoKernelInitInfo.remap_minor.rrz_out_height = 1080 ;
//        }
//        else
//        {
//            gStereoKernelInitInfo.remap_minor.rrz_out_width  = 1280 ;
//            gStereoKernelInitInfo.remap_minor.rrz_out_height = 720 ;
//        }

        //Pre-crop main2
        MY_LOGD("[Capture-RF Main2] Before precrop rrz_usage_width  %d", gStereoKernelInitInfo.remap_minor.rrz_usage_width);
	    MY_LOGD("[Capture-RF Main2] Before precrop rrz_usage_height %d", gStereoKernelInitInfo.remap_minor.rrz_usage_height);
        if(main2_FOV_horizontal > (main1_FOV_horizontal + m_nPrecropDegree))//FOV diff bigger than 10 will do precrop
        {
            gStereoKernelInitInfo.remap_minor.rrz_usage_width  = (MINT32)( tan((main1_FOV_horizontal+m_nPrecropDegree)/180.0f*acos(0.0f)) / tan(main2_FOV_horizontal/180.0f*acos(0.0f)) * rSensorCropInfo.w2_tg_size/32 )*32 ;
            gStereoKernelInitInfo.hori_fov_minor = main1_FOV_horizontal + m_nPrecropDegree ;

            MY_LOGD("[Capture-RF Main2] Precrop rrz_usage_width to %d", gStereoKernelInitInfo.remap_minor.rrz_usage_width);
	        MY_LOGD("[Capture-RF Main2] Pre-crop FOV to %.1f", gStereoKernelInitInfo.hori_fov_minor);
        }
        else
        {
	         gStereoKernelInitInfo.remap_minor.rrz_usage_width = rSensorCropInfo.w2_tg_size;     //sensor out width ;
	         MY_LOGD("[Capture-RF Main2] Set rrz_usage_width to %d", gStereoKernelInitInfo.remap_minor.rrz_usage_width);
        }

        gStereoKernelInitInfo.remap_minor.rrz_usage_height = (gStereoKernelInitInfo.remap_minor.rrz_usage_width *
                                                              StereoSettingProvider::imageResolution().uRatioNumerator /
                                                              StereoSettingProvider::imageResolution().uRatioDenomerator)
                                                             & ~1;  //Image size must be even
    	gStereoKernelInitInfo.remap_minor.rrz_offset_x = ((rSensorCropInfo.w2_tg_size - gStereoKernelInitInfo.remap_minor.rrz_usage_width )>>1 ) ;
        gStereoKernelInitInfo.remap_minor.rrz_offset_y = ((rSensorCropInfo.h2_tg_size - gStereoKernelInitInfo.remap_minor.rrz_usage_height)>>1 ) ;

        gStereoKernelInitInfo.enable_cc     = 1; //enable color correction: 1
        gStereoKernelInitInfo.enable_gpu    = ENABLE_GPU;
    }

    //rrz_usage -> RRZ -> rrz_out, RRZ cannot scale up
    //Adjust main cam
    if(gStereoKernelInitInfo.remap_main.rrz_usage_width < gStereoKernelInitInfo.remap_main.rrz_out_width) {
        MY_LOGD("[Main]Set rrz_out_width(%d) as rrz_usage_width(%d)",
                gStereoKernelInitInfo.remap_main.rrz_out_width,
                gStereoKernelInitInfo.remap_main.rrz_usage_width);
        gStereoKernelInitInfo.remap_main.rrz_out_width  = gStereoKernelInitInfo.remap_main.rrz_usage_width;
    }

    if(gStereoKernelInitInfo.remap_main.rrz_usage_height < gStereoKernelInitInfo.remap_main.rrz_out_height) {
        MY_LOGD("[Main]Set rrz_out_height(%d) as rrz_usage_height(%d)",
                gStereoKernelInitInfo.remap_main.rrz_out_height,
                gStereoKernelInitInfo.remap_main.rrz_usage_height);
        gStereoKernelInitInfo.remap_main.rrz_out_height = gStereoKernelInitInfo.remap_main.rrz_usage_height;
    }

    //Adjust minor cam
    if(gStereoKernelInitInfo.remap_minor.rrz_usage_width < gStereoKernelInitInfo.remap_minor.rrz_out_width) {
        MY_LOGD("[Minor]Set rrz_out_width(%d) as rrz_usage_width(%d)",
                gStereoKernelInitInfo.remap_minor.rrz_out_width,
                gStereoKernelInitInfo.remap_minor.rrz_usage_width);
        gStereoKernelInitInfo.remap_minor.rrz_out_width  = gStereoKernelInitInfo.remap_minor.rrz_usage_width;
    }

    if(gStereoKernelInitInfo.remap_minor.rrz_usage_height < gStereoKernelInitInfo.remap_minor.rrz_out_height) {
        MY_LOGD("[Minor]Set rrz_out_height(%d) as rrz_usage_height(%d)",
                gStereoKernelInitInfo.remap_minor.rrz_out_height,
                gStereoKernelInitInfo.remap_minor.rrz_usage_height);
        gStereoKernelInitInfo.remap_minor.rrz_out_height = gStereoKernelInitInfo.remap_minor.rrz_usage_height;
    }

    //Check if user want to do verification
    gStereoKernelInitInfo.enable_verify = (checkStereoProperty(PROPERTY_ENABLE_VERIFY) > 0) ? 1 : 0;

    // Only change if the property is set
    nCheckCC = checkStereoProperty(PERPERTY_ENABLE_CC, false);
    if(nCheckCC >= 0) {
        MY_LOGD("Override CC setting: %d", nCheckCC);
        gStereoKernelInitInfo.enable_cc = nCheckCC;
    }

//debug ----------------------------------------------------------------------------------------------------------
    MY_LOGD(" ===== Dump init info for algorithm ===== ");
    MY_LOGD("Sensor index: main(%d) minor(%d)", m_nMain1SensorIdx, m_nMain2SensorIdx);
    MY_LOGD("ptuning_para %d.", gStereoKernelInitInfo.ptuning_para->learn_tolerance);
    MY_LOGD("scenario %d.",     gStereoKernelInitInfo.scenario);

    MY_LOGD("main_source_image_width %d.",  gStereoKernelInitInfo.main_source_image_width);
    MY_LOGD("main_source_image_height %d.", gStereoKernelInitInfo.main_source_image_height);
    MY_LOGD("main_output_image_width %d.",  gStereoKernelInitInfo.main_output_image_width);
    MY_LOGD("main_output_image_height %d.", gStereoKernelInitInfo.main_output_image_height);

    MY_LOGD("algo_source_image_width %d.",  gStereoKernelInitInfo.algo_source_image_width);
    MY_LOGD("algo_source_image_height %d.", gStereoKernelInitInfo.algo_source_image_height);
    MY_LOGD("algo_crop_image_width %d.",    gStereoKernelInitInfo.algo_output_image_width);
    MY_LOGD("algo_crop_image_height %d.",   gStereoKernelInitInfo.algo_output_image_height);

    MY_LOGD("hwfe_block_size %d.",      gStereoKernelInitInfo.hwfe_block_size);
    MY_LOGD("fefm_image_width %d.",     gStereoKernelInitInfo.fefm_image_width);
    MY_LOGD("fefm_image_height %d.",    gStereoKernelInitInfo.fefm_image_height);

    MY_LOGD("rgba_image_width %d.",     gStereoKernelInitInfo.rgba_image_width);
    MY_LOGD("rgba_image_height %d.",    gStereoKernelInitInfo.rgba_image_height);
    MY_LOGD("rgba_image_stride %d.",    gStereoKernelInitInfo.rgba_image_stride);
    MY_LOGD("pixel_array_width %d.",    gStereoKernelInitInfo.remap_main.pixel_array_width);
    MY_LOGD("pixel_array_height %d.",   gStereoKernelInitInfo.remap_main.pixel_array_height);

    MY_LOGD("[Main]sensor_offset_x0 %d.",   gStereoKernelInitInfo.remap_main.sensor_offset_x0);
    MY_LOGD("[Main]sensor_offset_y0 %d.",   gStereoKernelInitInfo.remap_main.sensor_offset_y0);
    MY_LOGD("[Main]sensor_size_w0 %d.",     gStereoKernelInitInfo.remap_main.sensor_size_w0);
    MY_LOGD("[Main]sensor_size_h0 %d.",     gStereoKernelInitInfo.remap_main.sensor_size_h0);
    MY_LOGD("[Main]sensor_scale_w %d.",     gStereoKernelInitInfo.remap_main.sensor_scale_w);
    MY_LOGD("[Main]sensor_scale_h %d.",     gStereoKernelInitInfo.remap_main.sensor_scale_h);
    MY_LOGD("[Main]sensor_offset_x1 %d.",   gStereoKernelInitInfo.remap_main.sensor_offset_x1);
    MY_LOGD("[Main]sensor_offset_y1 %d.",   gStereoKernelInitInfo.remap_main.sensor_offset_y1);

    MY_LOGD("[Main]tg_offset_x %d.",        gStereoKernelInitInfo.remap_main.tg_offset_x);
    MY_LOGD("[Main]tg_offset_y %d.",        gStereoKernelInitInfo.remap_main.tg_offset_y);
    MY_LOGD("[Main]rrz_offset_x %d.",       gStereoKernelInitInfo.remap_main.rrz_offset_x);
    MY_LOGD("[Main]rrz_offset_y %d.",       gStereoKernelInitInfo.remap_main.rrz_offset_y);
    MY_LOGD("[Main]rrz_out_width %d.",      gStereoKernelInitInfo.remap_main.rrz_out_width);
    MY_LOGD("[Main]rrz_out_height %d.",     gStereoKernelInitInfo.remap_main.rrz_out_height);
    MY_LOGD("[Main]rrz_usage_width %d.",    gStereoKernelInitInfo.remap_main.rrz_usage_width);
    MY_LOGD("[Main]rrz_usage_height %d.",   gStereoKernelInitInfo.remap_main.rrz_usage_height);

    MY_LOGD("[Minor]pixel_array_width %d.",     gStereoKernelInitInfo.remap_minor.pixel_array_width);
    MY_LOGD("[Minor]pixel_array_height %d.",    gStereoKernelInitInfo.remap_minor.pixel_array_height);
    MY_LOGD("[Minor]sensor_offset_x0 %d.",      gStereoKernelInitInfo.remap_minor.sensor_offset_x0);
    MY_LOGD("[Minor]sensor_offset_y0 %d.",      gStereoKernelInitInfo.remap_minor.sensor_offset_y0);
    MY_LOGD("[Minor]sensor_size_w0 %d.",        gStereoKernelInitInfo.remap_minor.sensor_size_w0);
    MY_LOGD("[Minor]sensor_size_h0 %d.",        gStereoKernelInitInfo.remap_minor.sensor_size_h0);
    MY_LOGD("[Minor]sensor_scale_w %d.",        gStereoKernelInitInfo.remap_minor.sensor_scale_w);
    MY_LOGD("[Minor]sensor_scale_h %d.",        gStereoKernelInitInfo.remap_minor.sensor_scale_h);
    MY_LOGD("[Minor]sensor_offset_x1 %d.",      gStereoKernelInitInfo.remap_minor.sensor_offset_x1);
    MY_LOGD("[Minor]sensor_offset_y1 %d.",      gStereoKernelInitInfo.remap_minor.sensor_offset_y1);

    MY_LOGD("[Minor]tg_offset_x %d.",       gStereoKernelInitInfo.remap_minor.tg_offset_x);
    MY_LOGD("[Minor]tg_offset_y %d.",       gStereoKernelInitInfo.remap_minor.tg_offset_y);
    MY_LOGD("[Minor]rrz_offset_x %d.",      gStereoKernelInitInfo.remap_minor.rrz_offset_x);
    MY_LOGD("[Minor]rrz_offset_y %d.",      gStereoKernelInitInfo.remap_minor.rrz_offset_y);
    MY_LOGD("[Minor]rrz_out_width %d.",     gStereoKernelInitInfo.remap_minor.rrz_out_width);
    MY_LOGD("[Minor]rrz_out_height %d.",    gStereoKernelInitInfo.remap_minor.rrz_out_height);
    MY_LOGD("[Minor]rrz_usage_width %d.",   gStereoKernelInitInfo.remap_minor.rrz_usage_width);
    MY_LOGD("[Minor]rrz_usage_height %d.",  gStereoKernelInitInfo.remap_minor.rrz_usage_height);

    MY_LOGD("sensor_config %d.",    gStereoKernelInitInfo.sensor_config);
    MY_LOGD("enable_cc %d.",        gStereoKernelInitInfo.enable_cc);
    MY_LOGD("enable_gpu %d.",       gStereoKernelInitInfo.enable_gpu);
    MY_LOGD("hori_fov_main %f.",    gStereoKernelInitInfo.hori_fov_main);
    MY_LOGD("hori_fov_minor %f.",   gStereoKernelInitInfo.hori_fov_minor);
    MY_LOGD("stereo_baseline %f.",  gStereoKernelInitInfo.stereo_baseline);
    MY_LOGD("af_dac_start = %d.",   gStereoKernelInitInfo.af_dac_start);
    MY_LOGD("enable_verify %d.",    gStereoKernelInitInfo.enable_verify);
//debug ----------------------------------------------------------------------------------------------------------

    //After a lot of work, we can init algo now
    err = m_pStereoDrv->StereoKernelInit(&gStereoKernelInitInfo);
    if (err)
    {
        MY_LOGE("StereoKernelInit() fail. error code: %d.", err);
        bResult = MFALSE;
        goto lb_Abnormal_Exit;
    }
#endif

    if (   STEREO_SCENARIO_PREVIEW == mScenario
        || STEREO_SCENARIO_RECORD  == mScenario
       )
    {
        OutData.algoin_size = InData.main_image_size;
    }
    else
    {
        OutData.algoin_size = InData.main_image_size * STEREO_FACTOR;
    }

#if ENABLE_ALGO
    //Get algo working buffer size and allocate one
    err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_WORK_BUF_INFO, NULL,
                                                &gStereoKernelInitInfo.working_buffer_size);
    if (err)
    {
        MY_LOGE("StereoKernelFeatureCtrl(Get_WORK_BUF_INFO) fail. error code: %d.", err);
        bResult = MFALSE;
        goto lb_Abnormal_Exit;
    }
    _initWorkBuf(gStereoKernelInitInfo.working_buffer_size);
#endif

lb_Abnormal_Exit:

    //MY_LOGD("- X. Result: %d. work_buf_size: %d. l_crop_offset W/H: (%d, %d). r_crop_offset W/H: (%d, %d).", Result, gStereoKernelInitInfo.working_buffer_size, OutData.left_crop_offset.width, OutData.left_crop_offset.height, OutData.right_crop_offset.width, OutData.right_crop_offset.height);

    return bResult;

}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::_initWorkBuf(const MUINT32 BUFFER_SIZE)
{
    if (m_pWorkBuf) {
        free(m_pWorkBuf);
    }
    m_pWorkBuf = NULL;

    const MUINT32 BUFFER_LEN = sizeof(MUINT8) * BUFFER_SIZE;
    m_pWorkBuf = (MUINT8*)malloc(BUFFER_LEN) ;
    ::memset(m_pWorkBuf, 0, BUFFER_LEN);
    MY_LOGD("Working buffer size: %d addr %p. ", BUFFER_SIZE, m_pWorkBuf);
    MBOOL bResult = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    // Allocate working buffer.
    //     Allocate memory
    //     Set WorkBufInfo
    gStereoKernelWorkBufInfo.ext_mem_size       = gStereoKernelInitInfo.working_buffer_size;
    gStereoKernelWorkBufInfo.ext_mem_start_addr = m_pWorkBuf;

    err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SET_WORK_BUF_INFO,
                                                &gStereoKernelWorkBufInfo, NULL);
    if (err)
    {
        MY_LOGE("StereoKernelFeatureCtrl(SET_WORK_BUF_INFO) fail. error code: %d.", err);
        bResult = MFALSE;
        goto lb_Abnormal_Exit;
    }

lb_Abnormal_Exit:

    MY_LOGD("- X. Result: %d.", bResult);
    return bResult;

}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREOSetParams(SET_DATA_STEREO_T RunData)
{
    MBOOL Result = MTRUE;
    MY_LOGD("SetParams in");
#if ENABLE_ALGO
    MINT32 err = 0; // 0: no error. other value: error.
    //Set graphic buffer to algo
    gStereoKernelProcInfo.src_gb.mGraphicBuffer = RunData.mSrcGraphicBuffer;
    gStereoKernelProcInfo.dst_gb.mGraphicBuffer = RunData.mDstGraphicBuffer;
    gStereoKernelProcInfo.warp_image_addr_src   = (MUINT8*)RunData.mSrcGraphicBufferVA;
    gStereoKernelProcInfo.warp_image_addr_dst   = (MUINT8*)RunData.mDstGraphicBufferVA;
    MY_LOGD("SetParams src:%p dst:%p", gStereoKernelProcInfo.warp_image_addr_src, gStereoKernelProcInfo.warp_image_addr_dst);

    //Set Input/Output graphic buffer to algo
    sp<AHardwareBuffer>* srcGBArray[1];
    sp<AHardwareBuffer>* dstGBArray[1];
    srcGBArray[0] = (sp<AHardwareBuffer>*)RunData.mSrcGraphicBuffer;
    dstGBArray[0] = (sp<AHardwareBuffer>*)RunData.mDstGraphicBuffer;
    gStereoKernelProcInfo.InputGB   = (void*)&srcGBArray;
    gStereoKernelProcInfo.OutputGB  = (void*)&dstGBArray;
    MY_LOGD("gStereoKernelProcInfo.OutputGB=%p", gStereoKernelProcInfo.OutputGB);

    //Set RGBA image and HWFE data
    if(0 == m_uSensorRelativePosition)
    {
        gStereoKernelProcInfo.rgba_image_left_addr  = (MUINT8*)RunData.u4RgbaAddr_main1;
        gStereoKernelProcInfo.rgba_image_right_addr = (MUINT8*)RunData.u4RgbaAddr_main2;
        // hwfe
        gStereoKernelProcInfo.hwfe_data_left  = (MUINT16*)RunData.u4FEBufAddr_main1 ; // for store results of HWFE, Left  Image
        gStereoKernelProcInfo.hwfe_data_right = (MUINT16*)RunData.u4FEBufAddr_main2 ; // for store results of HWFE, Right Image

        gStereoKernelProcInfo.fefm_image_left_addr  = (MUINT8 *)RunData.YV12BufAddr_main1;
        gStereoKernelProcInfo.fefm_image_right_addr = (MUINT8 *)RunData.YV12BufAddr_main2;
    }
    else
    {
    	gStereoKernelProcInfo.rgba_image_left_addr  = (MUINT8*)RunData.u4RgbaAddr_main2;
        gStereoKernelProcInfo.rgba_image_right_addr = (MUINT8*)RunData.u4RgbaAddr_main1;
        // hwfe
        gStereoKernelProcInfo.hwfe_data_left  = (MUINT16*)RunData.u4FEBufAddr_main2; // for store results of HWFE, Left  Image
        gStereoKernelProcInfo.hwfe_data_right = (MUINT16*)RunData.u4FEBufAddr_main1; // for store results of HWFE, Right Image

        gStereoKernelProcInfo.fefm_image_left_addr  = (MUINT8 *)RunData.YV12BufAddr_main2;
        gStereoKernelProcInfo.fefm_image_right_addr = (MUINT8 *)RunData.YV12BufAddr_main1;
    }

    // Set AF info according to scenario
    if (STEREO_KERNEL_SCENARIO_IMAGE_CAPTURE_ONLY == gStereoKernelInitInfo.scenario)
    {
        MY_LOGD("Capture mode get dac");
        gStereoKernelProcInfo.af_dac_index  = g_prDafTbl->daf_vec[g_prDafTbl->curr_p1_frm_num % DAF_TBL_QLEN].af_dac_index;
        gStereoKernelProcInfo.af_confidence = g_prDafTbl->daf_vec[g_prDafTbl->curr_p1_frm_num % DAF_TBL_QLEN].af_confidence;
        gStereoKernelProcInfo.af_valid      = g_prDafTbl->daf_vec[g_prDafTbl->curr_p1_frm_num % DAF_TBL_QLEN].af_valid;
    }
    else
    {
        gStereoKernelProcInfo.af_dac_index  = g_prDafTbl->daf_vec[g_prDafTbl->curr_p2_frm_num % DAF_TBL_QLEN].af_dac_index ;
        gStereoKernelProcInfo.af_confidence = g_prDafTbl->daf_vec[g_prDafTbl->curr_p2_frm_num % DAF_TBL_QLEN].af_confidence ;
        gStereoKernelProcInfo.af_valid      = g_prDafTbl->daf_vec[g_prDafTbl->curr_p2_frm_num % DAF_TBL_QLEN].af_valid ;
    }

    if (g_prDafTbl->is_query_happen != INVALID_QUERY) // If is_query_happen is valid
    {
        gStereoKernelProcInfo.af_win_start_x_remap = g_prDafTbl->daf_vec[g_prDafTbl->is_query_happen % DAF_TBL_QLEN].af_win_start_x ; // 1/8 windows of 4096 = 512
        gStereoKernelProcInfo.af_win_end_x_remap   = g_prDafTbl->daf_vec[g_prDafTbl->is_query_happen % DAF_TBL_QLEN].af_win_end_x ; // 2304-1793+1 = 512
        gStereoKernelProcInfo.af_win_start_y_remap = g_prDafTbl->daf_vec[g_prDafTbl->is_query_happen % DAF_TBL_QLEN].af_win_start_y ; // 1/8 windows of 3072 = 384
        gStereoKernelProcInfo.af_win_end_y_remap   = g_prDafTbl->daf_vec[g_prDafTbl->is_query_happen % DAF_TBL_QLEN].af_win_end_y ; // 1728-1345+1 = 384
    }
    gStereoKernelProcInfo.af_win_start_x_remap = 0;
    gStereoKernelProcInfo.af_win_end_x_remap   = 0;
    gStereoKernelProcInfo.af_win_start_y_remap = 0;
    gStereoKernelProcInfo.af_win_end_y_remap = 0;

    MY_LOGD("curr_p2_frm_num = %d (idx: %d)",g_prDafTbl->curr_p2_frm_num, g_prDafTbl->curr_p2_frm_num% DAF_TBL_QLEN);
    MY_LOGD("is_query_happen = %d (idx: %d)",g_prDafTbl->is_query_happen, g_prDafTbl->is_query_happen % DAF_TBL_QLEN);
    MY_LOGD("AF_DAC_Index = %d ",gStereoKernelProcInfo.af_dac_index);
    MY_LOGD("af_confidence = %f",gStereoKernelProcInfo.af_confidence);
    MY_LOGD("af_valid = %d",gStereoKernelProcInfo.af_valid);
    MY_LOGD("af_win_start_x_remap = %d",gStereoKernelProcInfo.af_win_start_x_remap);
    MY_LOGD("af_win_end_x_remap = %d",gStereoKernelProcInfo.af_win_end_x_remap);
    MY_LOGD("af_win_start_y_remap = %d",gStereoKernelProcInfo.af_win_start_y_remap);
    MY_LOGD("af_win_end_y_remap = %d",gStereoKernelProcInfo.af_win_end_y_remap);
    m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SET_PROC_INFO, &gStereoKernelProcInfo, NULL);
#endif
    MY_LOGD("SetParams out");
    return Result;

}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREORun(OUT_DATA_STEREO_T &OutData, MBOOL bEnableAlgo)
{
    MY_LOGD("Run in (%d)", bEnableAlgo);
    MBOOL bResult = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.

    m_afWindow.start_x = gStereoKernelProcInfo.af_win_start_x_remap ;
    m_afWindow.start_y = gStereoKernelProcInfo.af_win_start_y_remap ;
    m_afWindow.end_x = gStereoKernelProcInfo.af_win_end_x_remap ;
    m_afWindow.end_y = gStereoKernelProcInfo.af_win_end_y_remap ;

#if ENABLE_ALGO
    // Algorithm main.
    if(bEnableAlgo)
    {
        err = m_pStereoDrv->StereoKernelMain();
        MY_LOGD("Run main out");
        if (err)
        {
            MY_LOGE("StereoKernelMain() fail. error code: %d.", err);
            bResult = MFALSE;
            goto lb_Abnormal_Exit;
        }

        // Get result.
        memset((void*)&gStereoKernelResultInfo,0,sizeof(gStereoKernelResultInfo));
        err = m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_RESULT, NULL, &gStereoKernelResultInfo);
        if (err)
        {
            MY_LOGE("StereoKernelFeatureCtrl(GET_RESULT) fail. error code: %d.", err);
            bResult = MFALSE;
            goto lb_Abnormal_Exit;
        }

        //Dump stereo kernel debug log
        if(STEREO_KERNEL_SCENARIO_IMAGE_CAPTURE_ONLY == gStereoKernelInitInfo.scenario) {
            if( 1 == checkStereoProperty(PROPERTY_ALGO_BEBUG) ) {
                static MUINT snLogCount = 0;
                m_pStereoDrv->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_SAVE_LOG, &snLogCount, NULL);
            }

            if (0 == gStereoKernelInitInfo.remap_main.rrz_usage_width ||
                0 == gStereoKernelInitInfo.remap_main.rrz_usage_height )  // exception
            {
                MY_LOGE("remap fail 2.");
                bResult = FALSE;
            }
            else
            {
                MFLOAT scl_w, scl_h;
                scl_w = (MFLOAT)gStereoKernelInitInfo.main_source_image_width / gStereoKernelInitInfo.remap_main.rrz_usage_width ;
                scl_h = (MFLOAT)gStereoKernelInitInfo.main_source_image_height/ gStereoKernelInitInfo.remap_main.rrz_usage_height;

                // Shane 20150105 This project uses Fix lens which means there is no AF window.
                // We have to manually set it to zero
                // m_rfsWindow.start_x = (m_afWindow.start_x - gStereoKernelInitInfo.remap_main.rrz_offset_x) * scl_w ;
                // m_rfsWindow.start_y = (m_afWindow.start_y - gStereoKernelInitInfo.remap_main.rrz_offset_y) * scl_h ;
                // m_rfsWindow.end_x   = (m_afWindow.end_x   - gStereoKernelInitInfo.remap_main.rrz_offset_x) * scl_w ;
                // m_rfsWindow.end_y   = (m_afWindow.end_y   - gStereoKernelInitInfo.remap_main.rrz_offset_y) * scl_h ;
                m_rfsWindow.start_x = 0;
                m_rfsWindow.start_y = 0;
                m_rfsWindow.end_x   = 0;
                m_rfsWindow.end_y   = 0;

                MY_LOGD("STEREORun remapping info rfsWindow.start_x %d rfsWindow.start_y %d rfsWindow.end_x %d rfsWindow.end_y %d",
                         m_rfsWindow.start_x, m_rfsWindow.start_y, m_rfsWindow.end_x, m_rfsWindow.end_y);
            }

            const int WIDTH  = gStereoKernelInitInfo.algo_output_image_width;
            const int HEIGHT = gStereoKernelInitInfo.algo_output_image_height;
            const int IMAGE_SIZE = WIDTH*HEIGHT;

            if(m_pFullMask) {
                free(m_pFullMask);
                m_pFullMask = NULL;
            }

            m_pFullMask = (MUINT8*)malloc( (sizeof(MUINT8) * IMAGE_SIZE));
            ::memset(m_pFullMask, 0, sizeof(MUINT8) * IMAGE_SIZE);

            //==================================================
            //  Get mask & rotate
            //  Mask is 8-bit image, value: 0 or 0xFF
            //==================================================

            //No rotation
            int nCol = 0;
            int nRow = 0;
            int nWritePos = 0;

#if MODULE_ROTATE == 90
            //Rotate 90 degree clockwise
            nCol = HEIGHT - 1;
            nRow = 0;
            nWritePos = nCol;

#elif MODULE_ROTATE == 180
            //Rotate 180 degree
            nWritePos = IMAGE_SIZE - 1;

#elif MODULE_ROTATE == 270
            //Rotate 270 degree clockwise
            nCol = 0;
            nRow = WIDTH - 1;
            nWritePos = IMAGE_SIZE - HEIGHT;
#endif

            //Since graphic buffer is non-cacheable, which means direct access to gbuffer is very slow.
            //So we use cache to read and get mask
            //Performance enhancement is about 300ms -> 66 ms
            const MUINT32 CACHE_SIZE = 128; //experimental result, faster than 64, almost the same as 256, CACHE_SIZE % 8 = 0
            const MUINT32 COPY_SIZE = CACHE_SIZE * sizeof(MUINT32);

            MUINT32* pImgCache = new MUINT32[CACHE_SIZE];
            MUINT32* pGraphicBuffer = (MUINT32*)gStereoKernelProcInfo.warp_image_addr_dst;
            MUINT32* pCachePos = NULL;

            int nIndex = 0;
            for(int nRound = IMAGE_SIZE/CACHE_SIZE - 1; nRound >= 0; nRound--)
            {
                //Cache graphic buffer
                ::memcpy(pImgCache, pGraphicBuffer, COPY_SIZE);
                pGraphicBuffer += CACHE_SIZE;
                pCachePos = pImgCache;

                //Get mask from alpha channel and rotate at a time
                for(nIndex = CACHE_SIZE-1; nIndex >= 0; nIndex--) {
                    *(m_pFullMask + nWritePos) = (MUINT8)((*pCachePos)>>24);
                    ++pCachePos;

#if MODULE_ROTATE == 90
                    //Rotate 90 degree clockwise
                    nWritePos += HEIGHT;
                    ++nRow;
                    if(nRow >= WIDTH) {
                        nRow = 0;
                        --nCol;
                        nWritePos = nCol;
                    }

#elif MODULE_ROTATE == 180
                    //Rotate 180 degree
                    nWritePos--;

#elif MODULE_ROTATE == 270
                    //Rotate 270 degree clockwise
                    nWritePos -= HEIGHT;
                    --nRow;
                    if(nRow < 0) {
                        nRow = WIDTH - 1;
                        ++nCol;
                        nWritePos = IMAGE_SIZE - HEIGHT + nCol;
                    }
#else
                    //No rotation
                    ++nWritePos;
#endif
                }
            }
            delete [] pImgCache;

            struct timespec t_start, t_end, t_result;
            clock_gettime(CLOCK_MONOTONIC, &t_start);
            _compressMask(m_runLengthMaskData);
            clock_gettime(CLOCK_MONOTONIC, &t_end);
            t_result = timeDiff(t_start, t_end);
            MY_LOGD("_compressMask: %lu.%.9lu(len:%zu)", t_result.tv_sec, t_result.tv_nsec, m_runLengthMaskData.size());
        }
    }
    MY_LOGD("Run get result out");
#endif
    // Prepare output data.

lb_Abnormal_Exit:

#if ENABLE_ALGO
    if(0 == m_uSensorRelativePosition)
    {
        OutData.algo_main1.p.x  = gStereoKernelResultInfo.algo_left_offset_x;  // Image Capture
        OutData.algo_main1.p.y  = gStereoKernelResultInfo.algo_left_offset_y;  // Image Capture
        OutData.algo_main1.s    = _getAlgoImgSize(0);

        OutData.algo_main2.p.x  = gStereoKernelResultInfo.algo_right_offset_x ;  // Image Capture
        OutData.algo_main2.p.y  = gStereoKernelResultInfo.algo_right_offset_y ;  // Image Capture
        OutData.algo_main2.s    = _getAlgoImgSize(1);
    }
    else
    {
        OutData.algo_main1.p.x  = gStereoKernelResultInfo.algo_right_offset_x;  // Image Capture
        OutData.algo_main1.p.y  = gStereoKernelResultInfo.algo_right_offset_y;  // Image Capture
        OutData.algo_main1.s    = _getAlgoImgSize(0);

        OutData.algo_main2.p.x  = gStereoKernelResultInfo.algo_left_offset_x ;  // Image Capture
        OutData.algo_main2.p.y  = gStereoKernelResultInfo.algo_left_offset_y ;  // Image Capture
        OutData.algo_main2.s    = _getAlgoImgSize(1);
    }

	OutData.main_crop.p.x   = gStereoKernelResultInfo.main_offset_x;  // Image Refocus
    OutData.main_crop.p.y   = gStereoKernelResultInfo.main_offset_y;  // Image Refocus
    OutData.main_crop.s     = _getMainImgSize();
#else
    OutData.main_crop.p.x   = 10;  // Image Refocus
    OutData.main_crop.p.y   = 10;  // Image Refocus
    OutData.main_crop.s     = _getMainImgSize();

    OutData.algo_main1.p.x   = 5;
    OutData.algo_main1.p.y   = 5;
    OutData.algo_main1.s     = _getAlgoImgSize(0);

    OutData.algo_main2.p.x   = 6;
    OutData.algo_main2.p.y   = 6;
    OutData.algo_main2.s     = _getAlgoImgSize(0);
#endif

    MY_LOGD("- X. algo_main1(%d,%d,%d,%d), algo_main2(%d,%d,%d,%d), main_crop(%d,%d,%d,%d)",
            OutData.algo_main1.p.x, OutData.algo_main1.p.y,
            OutData.algo_main1.s.w, OutData.algo_main1.s.h,
            OutData.algo_main2.p.x, OutData.algo_main2.p.y,
            OutData.algo_main2.s.w, OutData.algo_main2.s.h,
            OutData.main_crop.p.x, OutData.main_crop.p.y,
            OutData.main_crop.s.w, OutData.main_crop.s.h);

    MY_LOGD("Run out");
    return bResult;
}
/*******************************************************************************
*
********************************************************************************/
void
StereoHal::_compressMask(std::vector<RUN_LENGTH_DATA> &compressedMask)
{
    compressedMask.clear();

    const int IMAGE_SIZE = gStereoKernelInitInfo.algo_output_image_width * gStereoKernelInitInfo.algo_output_image_height;
    MUINT32 len = 0;
    MUINT32 offset = 0;
    MUINT8* originMask = m_pFullMask;

    const int CMP_LEN = 128;
    MUINT8 *FF_MASK = new MUINT8[CMP_LEN];
    ::memset(FF_MASK, 0xFF, CMP_LEN);

    for(int i = 0; i < IMAGE_SIZE; i += CMP_LEN) {
        if(0 == memcmp(originMask, FF_MASK, CMP_LEN)) {
            if(0 == len) {
                offset = i;
            }

            len += CMP_LEN;
            originMask += CMP_LEN;
        } else {
            for(int j = 0; j < CMP_LEN; j++, originMask++) {
                if(0 != *originMask) {
                    if(0 != len) {
                        ++len;
                    } else {
                        len = 1;
                        offset = i+j;
                    }
                } else {
                    if(0 != len) {
                        compressedMask.push_back(RUN_LENGTH_DATA(offset, len));
                        len = 0;
                    }
                }
            }
        }
    }

    if(0 != len) {
        compressedMask.push_back(RUN_LENGTH_DATA(offset, len));
    }

    delete [] FF_MASK;
}
/*******************************************************************************
*
********************************************************************************/
void
StereoHal::_encodeDebugInfo()
{
    if(m_debugInfo) {
        delete m_debugInfo;
        m_debugInfo = NULL;
    }

//    "JPS_size": {
//        "width": 4352,
//        "height": 1152
//    },
    Document document(kObjectType);
    Document::AllocatorType& allocator = document.GetAllocator();

    Value JPS_size(kObjectType);
    JPS_size.AddMember("width", mAlgoAppendSize.w*2, allocator);
    JPS_size.AddMember("height", mAlgoAppendSize.h, allocator);
    document.AddMember("JPS_size", JPS_size, allocator);

//    "output_image_size" : {
//        "width": 2176,
//        "height": 1152
//    },
    Value output_image_size(kObjectType);
    output_image_size.AddMember("width", gStereoKernelInitInfo.algo_output_image_width, allocator);
    output_image_size.AddMember("height", gStereoKernelInitInfo.algo_output_image_height, allocator);
    document.AddMember("output_image_size", output_image_size, allocator);

//    "main_cam_align_shift" : {
//        "x": 30,
//        "y": 10
//    },
    Value main_cam_align_shift(kObjectType);
    main_cam_align_shift.AddMember("x", gStereoKernelResultInfo.algo_align_shift_x, allocator);
    main_cam_align_shift.AddMember("y", gStereoKernelResultInfo.algo_align_shift_y, allocator);
    document.AddMember("main_cam_align_shift", main_cam_align_shift, allocator);

//    "input_image_size": {
//        "width": 1920,
//        "height": 1080
//    },
    Value input_image_size(kObjectType);
    input_image_size.AddMember("width",  gStereoKernelInitInfo.algo_source_image_width,  allocator);
    input_image_size.AddMember("height", gStereoKernelInitInfo.algo_source_image_height, allocator);
    document.AddMember("input_image_size", input_image_size, allocator);

//    "capture_orientation": {
//        "orientations_values": ["0: none", "1: flip_horizontal", "2: flip_vertical", "4: 90", "3: 180", "7: 270"],
//        "orientation": 0
//    },
    Value capture_orientation(kObjectType);
    Value orientations_values(kArrayType);
    orientations_values.PushBack(Value("0: none").Move(), allocator);
    orientations_values.PushBack(Value("1: flip_horizontal").Move(), allocator);
    orientations_values.PushBack(Value("2: flip_vertical").Move(), allocator);
    orientations_values.PushBack(Value("4: 90").Move(), allocator);
    orientations_values.PushBack(Value("3: 180").Move(), allocator);
    orientations_values.PushBack(Value("7: 270").Move(), allocator);
    capture_orientation.AddMember("orientations_values", orientations_values, allocator);
    capture_orientation.AddMember("orientation", Value(m_nCapOrientation).Move(), allocator);
    document.AddMember("capture_orientation", capture_orientation, allocator);

//    "depthmap_orientation": {
//        "depthmap_orientation_values": ["0: none", "90: 90 degrees CW", "180: 180 degrees CW", "270: 270 degrees CW"],
//        "orientation": 0
//    },
    int main_image_orientation;
    switch(m_nCapOrientation){
        case 0: main_image_orientation = 0;
            break;
        case 4: main_image_orientation = 90;
            break;
        case 3: main_image_orientation = 180;
            break;
        case 7: main_image_orientation = 270;
            break;
        default:
            main_image_orientation = 0;
            break;
    }

    int _depthmap_orientation = (main_image_orientation + (360 - MODULE_ROTATE))%360;

    Value depthmap_orientation(kObjectType);
    Value depthmap_orientation_values(kArrayType);
    depthmap_orientation_values.PushBack(Value("0: none").Move(), allocator);
    depthmap_orientation_values.PushBack(Value("90: 90 degrees CW").Move(), allocator);
    depthmap_orientation_values.PushBack(Value("180: 180 degrees CW").Move(), allocator);
    depthmap_orientation_values.PushBack(Value("270: 270 degrees CW").Move(), allocator);
    depthmap_orientation.AddMember("depthmap_orientation_values", depthmap_orientation_values, allocator);
    depthmap_orientation.AddMember("orientation", Value(_depthmap_orientation).Move(), allocator);
    document.AddMember("depthmap_orientation", depthmap_orientation, allocator);


//    "sensor_relative_position": {
//        "relative_position_values": ["0: main-minor", "1: minor-main"],
//        "relative_position": 0
//    },
    Value sensor_relative_position(kObjectType);
    Value relative_position_values(kArrayType);
    relative_position_values.PushBack(Value("0: main-minor").Move(), allocator);
    relative_position_values.PushBack(Value("1: minor-main").Move(), allocator);
    sensor_relative_position.AddMember("relative_position_values", relative_position_values, allocator);
    sensor_relative_position.AddMember("relative_position", Value(m_uSensorRelativePosition).Move(), allocator);
    document.AddMember("sensor_relative_position", sensor_relative_position, allocator);

//    "focus_roi": {
//        "top": 0,
//        "left": 10,
//        "bottom": 10,
//        "right": 30
//    },
    Value focus_roi(kObjectType);
    focus_roi.AddMember("top",      Value(m_rfsWindow.start_y).Move(), allocator);
    focus_roi.AddMember("left",     Value(m_rfsWindow.start_x).Move(), allocator);
    focus_roi.AddMember("bottom",   Value(m_rfsWindow.end_y).Move(), allocator);
    focus_roi.AddMember("right",    Value(m_rfsWindow.end_x).Move(), allocator);
    document.AddMember("focus_roi", focus_roi, allocator);

//    "verify_geo_data": {
//        "quality_level_values": ["0: PASS","1: Cond.Pass","2: FAIL"],
//        "quality_level": 0,
//        "statistics": [0,0,0,0,0,0]
//    },
    Value verify_geo_data(kObjectType);
    Value quality_level_values(kArrayType);
    quality_level_values.PushBack(Value("0: PASS").Move(), allocator);
    quality_level_values.PushBack(Value("1: Cond.Pass").Move(), allocator);
    quality_level_values.PushBack(Value("2: FAIL").Move(), allocator);
    verify_geo_data.AddMember("quality_level_values", quality_level_values, allocator);
    verify_geo_data.AddMember("quality_level", Value(gStereoKernelResultInfo.verify_geo_quality_level).Move(), allocator);
    Value geo_statistics(kArrayType);
    for(int i = 0; i < 6; i++) {
        geo_statistics.PushBack(Value(gStereoKernelResultInfo.verify_geo_statistics[i]).Move(), allocator);
    }
    verify_geo_data.AddMember("statistics", geo_statistics, allocator);
    document.AddMember("verify_geo_data", verify_geo_data, allocator);

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
    verify_pho_data.AddMember("quality_level_values", pho_quality_level_values, allocator);
    verify_pho_data.AddMember("quality_level", Value(gStereoKernelResultInfo.verify_pho_quality_level).Move(), allocator);
    Value pho_statistics(kArrayType);
    for(int i = 0; i < 4; i++) {
        pho_statistics.PushBack(Value(gStereoKernelResultInfo.verify_pho_statistics[i]).Move(), allocator);
    }
    verify_pho_data.AddMember("statistics", pho_statistics, allocator);
    document.AddMember("verify_pho_data", verify_pho_data, allocator);

//    "verify_mtk_cha": {
//        "check_values": ["0: PASS","1: FAIL"],
//        "check": 0,
//        "score": 0
//    },
    Value verify_mtk_cha(kObjectType);
    Value verify_mtk_cha_values(kArrayType);
    verify_mtk_cha_values.PushBack(Value("0: PASS").Move(), allocator);
    verify_mtk_cha_values.PushBack(Value("1: FAIL").Move(), allocator);
    verify_mtk_cha.AddMember("check_values", verify_mtk_cha_values, allocator);
    verify_mtk_cha.AddMember("check", Value(gStereoKernelResultInfo.verify_cha_statistics[0]).Move(), allocator);
    verify_mtk_cha.AddMember("score", Value(gStereoKernelResultInfo.verify_cha_statistics[1]).Move(), allocator);
    document.AddMember("verify_mtk_cha", verify_mtk_cha, allocator);

//    "face_detections" : [
//        {
//            "left": 0,
//            "top": 10,
//            "right": 10,
//            "bottom": 30,
//            "rotation-in-plane": 0
//        },
//        {
//            "left": 20,
//            "top": 30,
//            "right": 40,
//            "bottom": 50,
//            "rotation-in-plane": 11
//        }
//    ],

    MY_LOGD("FD count %zu", m_fdInfos.size());
    if(m_fdInfos.size() > 0){
        Value face_detections(kArrayType);
        for(size_t f = 0; f < m_fdInfos.size(); f++) {
            Value face(kObjectType);
            face.AddMember("left",      Value(m_fdInfos[f].left).Move(), allocator);
            face.AddMember("top",       Value(m_fdInfos[f].top).Move(), allocator);
            face.AddMember("right",     Value(m_fdInfos[f].right).Move(), allocator);
            face.AddMember("bottom",    Value(m_fdInfos[f].bottom).Move(), allocator);
            face.AddMember("rotation-in-plane", Value(m_fdInfos[f].rotation).Move(), allocator);  //0, 1, ...11

            face_detections.PushBack(face.Move(), allocator);
        }

        document.AddMember("face_detections", face_detections, allocator);
    }

//    "mask_info" : {
//        "width":2176,
//        "height":1152,
//        "mask description": "Data(0xFF), format: [offset,length]",
//        "mask": [[28,1296],[1372,1296],[2716,1296],...]
//    }
    Value mask_info(kObjectType);
    mask_info.AddMember("width", gStereoKernelInitInfo.algo_output_image_width, allocator);
    mask_info.AddMember("height", gStereoKernelInitInfo.algo_output_image_height, allocator);
    mask_info.AddMember("mask description", "Data(0xFF), format: [offset,length]", allocator);

    struct timespec t_start, t_end, t_result;
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    Value mask(kArrayType);
    for(std::vector<RUN_LENGTH_DATA>::iterator it = m_runLengthMaskData.begin(); it != m_runLengthMaskData.end(); ++it) {
        Value maskData(kArrayType);
        maskData.PushBack(Value(it->offset).Move(), allocator);
        maskData.PushBack(Value(it->len).Move(), allocator);
        mask.PushBack(maskData.Move(), allocator);
    }
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    t_result = timeDiff(t_start, t_end);
    MY_LOGD("Mask to JSON: %lu.%.9lu", t_result.tv_sec, t_result.tv_nsec);
    mask_info.AddMember("mask", mask, allocator);
    document.AddMember("mask_info", mask_info, allocator);

    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

    if(1 == checkStereoProperty(PROPERTY_JSON_BEBUG)) {
        FILE* fp = fopen("/sdcard/stereo.json", "wb"); // non-Windows use "w"
        char writeBuffer[1024];
        FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
        Writer<FileWriteStream> writer(os);
        document.Accept(writer);
        fclose(fp);
    }

    const char *debugInfo = sb.GetString();
    if(debugInfo) {
        const int STR_LEN = strlen(debugInfo);
        if(STR_LEN > 0) {
            m_debugInfo = new char[STR_LEN+1];
            strncpy(m_debugInfo, debugInfo, (size_t)STR_LEN);
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREOGetDebugInfo(DBG_DATA_STEREO_T &DbgData)
{
    MBOOL Result = MTRUE;

    struct timespec t_start, t_end, t_result;
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    _encodeDebugInfo();
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    t_result = timeDiff(t_start, t_end);
    MY_LOGD("_encodeDebugInfo: %lu.%.9lu", t_result.tv_sec, t_result.tv_nsec);

    DbgData.dbgDataSize = (m_debugInfo) ? strlen(m_debugInfo)+1 : 0;
    DbgData.dbgDataAddr = (MUINT8*)m_debugInfo;
    return Result;
}

/*******************************************************************************
*
********************************************************************************/
bool
StereoHal::STEREODestroy(void)
{
    MY_LOGD("- E.");
    MBOOL Result = MTRUE;

#if ENABLE_ALGO
    // Reset algorithm.
    m_pStereoDrv->StereoKernelReset();
    if(m_pWorkBuf) {
        free(m_pWorkBuf);
        m_pWorkBuf = NULL;
    }

    if(m_pFullMask) {
        free(m_pFullMask);
        m_pFullMask = NULL;
    }
#endif

    mpHal3A->destroyInstance("MTKStereoCamera");

    MY_LOGD("- X. Result: %d.", Result);
    return Result;

}

/*******************************************************************************
* lenNVRAM: must use "byte" as unit.
* adb shell setprop debuglog.stereo.printloadednvram 1: print arrNVRAM that loaded from NVRAM.
********************************************************************************/
bool
StereoHal::_loadFromNvram()
{
    MBOOL Result = MTRUE;

    struct timespec t_start, t_end, t_result;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    IHalSensorList* sensorlsit;
    sensorlsit=IHalSensorList::get();
    int sensorDevIdx = sensorlsit->querySensorDevIdx(m_nMain1SensorIdx);

    MINT32 err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_GEOMETRY, sensorDevIdx, (void*&)m_pVoidGeoData);
    Result = !err;
    MY_LOGD("- X. Result: %d.", Result);

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    t_result = timeDiff(t_start, t_end);
    MY_LOGD("_loadFromNvram: %lu.%.9lu", t_result.tv_sec, t_result.tv_nsec);
    return Result;

}


/*******************************************************************************
* lenNVRAM: must use "byte" as unit.
* adb shell setprop debuglog.stereo.erasenvram 1: set to 1 to write all 0's into NVRAM. (Remember to set to 0 after erased.)
* adb shell setprop debuglog.stereo.printsavednvram 1: print arrNVRAM that saved to NVRAM.
********************************************************************************/
bool
StereoHal::_saveToNvram()
{
    struct timespec t_start, t_end, t_result;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    IHalSensorList* sensorlsit;
    sensorlsit=IHalSensorList::get();
    int sensorDevIdx = sensorlsit->querySensorDevIdx(m_nMain1SensorIdx);
    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_GEOMETRY, sensorDevIdx);

    if (err!=0)
    {
        MY_LOGE("Write to NVRAM fail.");
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

lb_Abnormal_Exit:

    MY_LOGD("- X. Result: %d.", Result);
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    t_result = timeDiff(t_start, t_end);
    MY_LOGD("_saveToNvram: %lu.%.9lu", t_result.tv_sec, t_result.tv_nsec);

    return Result;

}