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

#define LOG_TAG "stereo_hal_vendor"
#define STEREO_VENDOR_HAL_NAME      "stereo_hal_vendor"

#include <stdlib.h>     // for rand()

#include <cutils/properties.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/Log.h>
#include "stereo_hal_vendor.h"         // For StereoHal class.
#include <mtkcam/v1/IParamsManager.h>

//	#include "pip_hal.h"            // For query max frame rate.

#include "camera_custom_stereo.h"  // For CUST_STEREO_* definitions.
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
using namespace NSCam::Utils;
using namespace android;
using namespace rapidjson;
using namespace std;
using android::Mutex;           // For android::Mutex in stereo_hal.h.

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define ENABLE_GPU                  1
#define DUMP_PATH "/sdcard/mtklog/stereodump/"
#define INVALID_QUERY 0xFFFFFFFF

#define STEREO_VENDOR_DUMP_ALL           1
#define STEREO_VENDOR_DUMP_MAIN1_ONLY    2
#define STEREO_VENDOR_DUMP_MAIN2_ONLY    3

/*******************************************************************************
*
********************************************************************************/
#define STEREO_HAL_VENDOR_DEBUG

#ifdef STEREO_HAL_VENDOR_DEBUG    // Enable debug log.

#undef __func__
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)

#else   // Disable debug log.

#define MY_LOGD(a,...)
#define MY_LOGI(a,...)
#define MY_LOGW(a,...)
#define MY_LOGE(a,...)

#define MY_LOGD_IF(cond, ...)
#define MY_LOGI_IF(cond, ...)
#define MY_LOGW_IF(cond, ...)
#define MY_LOGE_IF(cond, ...)
#endif  // STEREO_HAL_DEBUG

static struct timespec timeDiff( struct timespec start, struct timespec end)
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
static StereoHalBase *pStereoHalVendor = NULL;

//
/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
#define MODULE_ROTATE 0   //Should align to the result in JPS

/*******************************************************************************
*
********************************************************************************/
StereoHalVendor::
StereoHalVendor()
    : mUsers(0)
    , mAlgoSize()
    , mMainSize()
    , mFEImgSize(MSize(960, 544))
    , m_nMain1SensorIdx(0)
    , m_nMain2SensorIdx(2)
    , m_debugInfo(NULL)
    , mpImgTransform(NULL)
    , mPrvCnt(0)
    , mCapCnt(0)
    , mDumpBuf(0)
    , mFakeAlgoTest(0)
    , mpIHalSensor(NULL)
    , mpHal3A(NULL)
    , mpHalFD(NULL)
    , mEnableDebugLog(0)
    , mAlgoSrcBuf_main1_prv_rrz(NULL)
    , mAlgoSrcBuf_main2_prv_rrz(NULL)
    , mAlgoDstBuf_prv(NULL)
    , mAlgoSrcBuf_main1_cap_fullsize(NULL)
    , mAlgoSrcBuf_main1_cap_rrz(NULL)
    , mAlgoSrcBuf_main2_cap_rrz(NULL)
    , mAlgoDstBuf_cap(NULL)
    , mDepthmapDstBuf_cap(NULL)
    , m_nCapOrientation(0)
    , mprDafTbl(NULL)
{
    mpImgTransform = IImageTransform::createInstance();
    if( !mpImgTransform )
    {
        MY_LOGE("imageTransform create failed");
    }

    // init FD structure
    mFD_info.number_of_faces = 0;
    mFD_info.faces = mfaces;
    mFD_info.posInfo = mposInfo;

    // init AF structure
    memset(&mdaf_vec, 0, sizeof(DAF_VEC_STRUCT));
}


/*******************************************************************************
*
********************************************************************************/
StereoHalVendor::~StereoHalVendor()
{
    if( mpImgTransform )
    {
        mpImgTransform->destroyInstance();
        mpImgTransform = NULL;
    }

}

StereoHalVendor* StereoHalVendor::
createInstance()
{
    StereoHalVendor *pStereoHalVendor = StereoHalVendor::getInstance();
    pStereoHalVendor->init();
    return pStereoHalVendor;
}

/*******************************************************************************
*
********************************************************************************/
StereoHalVendor* StereoHalVendor::
getInstance()
{
    MY_LOGD("StereoHalVendor getInstance.");
    static StereoHalVendor singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
StereoHalVendor::
destroyInstance()
{
    uninit();
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHalVendor::init()
{
    MY_LOGD("- Enter. mUsers: %d.", mUsers);
    MBOOL Result = MTRUE;   // TRUE: no error. FALSE: error.
    MINT32 err = 0;
    Mutex::Autolock lock(mLock);

    ////////////////////////////////////////////
    // TO-DO : Add Init code here
    // Be careful that init code will be used multiple times
    if(mpIHalSensor == NULL)
    {
        IHalSensorList* sensorlsit = IHalSensorList::get();
        mpIHalSensor = sensorlsit->createSensor("Stereo hal vendor", m_nMain1SensorIdx);
        if(mpIHalSensor==NULL)
        {
            MY_LOGE("fail to get HALSensor (%p)",mpIHalSensor);
        }
    }

    if(mpHal3A == NULL)
    {
        // get AF table for preview
        mpHal3A = IHal3A::createInstance(IHal3A::E_Camera_1, m_nMain1SensorIdx, "MTKStereoCamera");
        if(mpHal3A)
        {
            mpHal3A->send3ACtrl(E3ACtrl_GetDAFTBL, (MUINTPTR)&mprDafTbl, 0);
            MY_LOGD("create instance for HAL3A (%p)", mpHal3A);
            mprDafTbl->is_daf_run = 1;  // to enable AF table update
        }
        else
        {
            MY_LOGE("fail to get HAL3A (%p)",mpHal3A);
        }
    }

    if(mpHalFD == NULL)
    {
        mpHalFD = halFDBase::createInstance(HAL_FD_OBJ_HW);
        if(mpHalFD == NULL)
        {
            MY_LOGE("fail to get HALFD (%p)",mpHalFD);
        }
    }


    ////////////////////////////////////////////
    if (mUsers > 0)
    {
        MY_LOGD("StereoHalVendor has already inited.");
    }
    android_atomic_inc(&mUsers);
    MY_LOGD("- Exit. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHalVendor::uninit()
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
        ////////////////////////////////////////////
        // TO-DO : Add Uninit code here
        // Here is last user uninit
        if(mpIHalSensor)
        {
            MY_LOGD("destroy instance for HALSensor (%p)", mpIHalSensor);
            mpIHalSensor->destroyInstance("Stereo vendor hal");
            mpIHalSensor = NULL;
        }

        if(mpHal3A)
        {
            MY_LOGD("destroy instance for HAL3A (%p)", mpHal3A);
            mpHal3A->destroyInstance("MTKStereoCamera");
            mpHal3A = NULL;
        }
        if(mpHalFD)
        {
            MY_LOGD("destroy instance for HALFD (%p)", mpHalFD);
            mpHalFD->destroyInstance();
            mpHalFD = NULL;
        }

        ////////////////////////////////////////////

        if(m_debugInfo) {
            delete m_debugInfo;
            m_debugInfo = NULL;
        }
    }
    else // mUsers <= 0. No StereoHalVendor user, do nothing.
    {
        // do nothing.
        MY_LOGW("No StereoHalVendor to un-init.");
    }

    MY_LOGD("- Exit. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;

}

/*******************************************************************************
*
********************************************************************************/
void
StereoHalVendor::setParameters(sp<IParamsManager> spParamsMgr)
{
    mParamsMgr = spParamsMgr;

// How to get DOF level
//	    m_DofLevel = mParamsMgr->getInt(MtkCameraParameters::KEY_STEREO_DOF_LEVEL);

}

/*******************************************************************************
*
********************************************************************************/
MSize
StereoHalVendor::getMainSize(MSize const imgSize) const
{
    if( getAlgoInputTransform() == eTransform_ROT_90  ||
        getAlgoInputTransform() == eTransform_ROT_270 ||
        getAlgoInputTransform() == (eTransform_FLIP_V |eTransform_ROT_90)||
        getAlgoInputTransform() == (eTransform_FLIP_H |eTransform_ROT_90))
    {
        // switch size due to pic rotation
        return MSize(imgSize.h , imgSize.w);
    }
    else
    {
        return MSize(imgSize.w , imgSize.h);
    }
}

/*******************************************************************************
*
********************************************************************************/
MSize
StereoHalVendor::getAlgoInputSize(MUINT32 const SENSOR_INDEX) const
{
	// idx: 0 means main sensor, otherwise, main2 sensor.
    // szMainCam           = MSize(1920, 1080);
    // szSubCam            = MSize(960, 540);
    return (SENSOR_INDEX == m_nMain1SensorIdx) ? StereoSettingProvider::imageResolution().szMainCam : StereoSettingProvider::imageResolution().szSubCam;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
StereoHalVendor::getAlgoInputTransform() const
{
    // get custom folder setting for sensor transform
    return getStereoSensorTransform();
}

/*******************************************************************************
*
********************************************************************************/
bool
StereoHalVendor::STEREOGetRrzInfo(RRZ_DATA_STEREO_T &OutData) const
{
    // get p1 rrzo output crop & image size
    StereoResolution StResolution = getStereoAlgoInputResolution();
    if((!StResolution.bUseSensorMaxCropSz) || (mpIHalSensor==NULL))
    {
        OutData.rrz_crop_main1  = StResolution.p1_rrz_crop_main1;
        OutData.rrz_crop_main2  = StResolution.p1_rrz_crop_main2;
    }
    else
    {
        OutData.rrz_crop_main1  = mMain1SensorMaxCrop;
        OutData.rrz_crop_main2  = mMain2SensorMaxCrop;
    }

    OutData.rrz_size_main1  = getAlgoInputSize(m_nMain1SensorIdx);
    OutData.rrz_size_main2  = getAlgoInputSize(m_nMain2SensorIdx);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHalVendor::STEREOGetInfo(HW_DATA_STEREO_T &OutData) const
{
    MBOOL ret = MTRUE;

    // use some dummy size to keep flow stable
    OutData.rgba_image_width    = 160;
    OutData.rgba_image_height   = 90;

    OutData.fefm_image_width    = getFEImgSize().w;
    OutData.fefm_imgae_height   = getFEImgSize().h;

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
bool
StereoHalVendor::STEREOInit(INIT_DATA_STEREO_IN_T InData, INIT_DATA_STEREO_OUT_T &OutData)
{
    MBOOL bResult = MTRUE;
    mDumpBuf = ::property_get_int32("debug.camera.stereo.vendump", 0);
    mFakeAlgoTest = ::property_get_int32("debug.camera.stereo.fakealgo", 0);
    mEnableDebugLog = ::property_get_int32("debug.camera.stereo.vendlog", 0);

    m_nCapOrientation = InData.orientation;
    m_nMain1SensorIdx = InData.main1_sensor_index;
    m_nMain2SensorIdx = InData.main2_sensor_index;
    switch ( InData.eScenario )
    {
        case STEREO_SCENARIO_PREVIEW:
            MY_LOGD("mScenario = STEREO_SCENARIO_PREVIEW %d", STEREO_SCENARIO_PREVIEW);
            mAlgoSize_main1_prv_rrz = InData.algo_image_size;
            mAlgoSize_main2_prv_rrz = InData.algo_image_size_main2;
            OutData.algoin_size = InData.algo_image_size;
            break;
        case STEREO_SCENARIO_CAPTURE:
            MY_LOGD("mScenario = STEREO_SCENARIO_CAPTURE %d", STEREO_SCENARIO_CAPTURE);
            mAlgoSize_cap_fullsize = InData.main_image_size;
            mAlgoSize_main1_cap_rrz = InData.algo_image_size;
            mAlgoSize_main2_cap_rrz = InData.algo_image_size_main2;
            OutData.algoin_size = InData.main_image_size;
            break;
        case STEREO_SCENARIO_RECORD:
        default:
            MY_LOGE("unsupport scenario(%d)", InData.eScenario);
            break;
    }

    // get main1/main2 sensor max crop size
    MINT32 err = 0; // 0: no error. other value: error.
    SensorCropWinInfo rSensorCropInfo;
    ::memset(&rSensorCropInfo, 0, sizeof(SensorCropWinInfo));
    IHalSensorList* sensorlsit = IHalSensorList::get();
    int sensorDevIdx = sensorlsit->querySensorDevIdx(m_nMain1SensorIdx);
    if(mpIHalSensor)
    {
        err = mpIHalSensor->sendCommand(sensorDevIdx, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                       (MUINTPTR)&InData.main1_sensor_scenario, (MUINTPTR)&rSensorCropInfo, 0);
        if(err)
        {
            MY_LOGE("SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO() for main1 fail. error code: %d.", err);
        }
        else
        {
            mMain1SensorMaxCrop.s.w = rSensorCropInfo.w2_tg_size;
            mMain1SensorMaxCrop.s.h = (rSensorCropInfo.w2_tg_size *
                                       StereoSettingProvider::imageResolution().uRatioNumerator/
                                       StereoSettingProvider::imageResolution().uRatioDenomerator)
                                      & ~1; //Image size must be even
            mMain1SensorMaxCrop.p.x = 0;
            mMain1SensorMaxCrop.p.y = ((rSensorCropInfo.h2_tg_size - mMain1SensorMaxCrop.s.h)>>1 );
        }

        sensorDevIdx = sensorlsit->querySensorDevIdx(m_nMain2SensorIdx);
        err = mpIHalSensor->sendCommand(sensorDevIdx, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                       (MUINTPTR)&InData.main2_sensor_scenario, (MUINTPTR)&rSensorCropInfo, 0);
        if(err)
        {
            MY_LOGE("SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO() for main1 fail. error code: %d.", err);
        }
        else
        {
            mMain2SensorMaxCrop.s.w = rSensorCropInfo.w2_tg_size;
            mMain2SensorMaxCrop.s.h = (rSensorCropInfo.w2_tg_size *
                                       StereoSettingProvider::imageResolution().uRatioNumerator/
                                       StereoSettingProvider::imageResolution().uRatioDenomerator)
                                      & ~1; //Image size must be even
            mMain2SensorMaxCrop.p.x = 0;
            mMain2SensorMaxCrop.p.y = ((rSensorCropInfo.h2_tg_size - mMain2SensorMaxCrop.s.h)>>1 );
        }
        MY_LOGD("Main1_MaxCrop(%d,%d,%dx%d),Main2_MaxCrop(%d,%d,%dx%d)",
            mMain1SensorMaxCrop.p.x,
            mMain1SensorMaxCrop.p.y,
            mMain1SensorMaxCrop.s.w,
            mMain1SensorMaxCrop.s.h,
            mMain2SensorMaxCrop.p.x,
            mMain2SensorMaxCrop.p.y,
            mMain2SensorMaxCrop.s.w,
            mMain2SensorMaxCrop.s.h);
    }

    //Since af_mgr::init may run later, we have to wait for it
    mprDafTbl->is_daf_run = 1;
    for(int nTimes = 10; nTimes > 0; nTimes--) {
        if (0 == mprDafTbl->af_dac_min) {
            MY_LOGD("Waiting for af_dac_min...");
            usleep(20 * 1000);
        } else {
            break;
        }
    }

    if (0 == mprDafTbl->af_dac_min) {
        MY_LOGE("Cannot get af_dac_min");
    }


    return bResult;
}

/*******************************************************************************
*
********************************************************************************/
bool
StereoHalVendor::STEREOSetParams(SET_DATA_STEREO_T RunData)
{
    MBOOL Result = MTRUE;
    MY_LOGD_IF(mEnableDebugLog,"SetParams in, Scenario = %d", RunData.eScenario);
    mprDafTbl->is_daf_run = 1;

    switch ( RunData.eScenario )
    {
        case STEREO_SCENARIO_PREVIEW:
            if((RunData.AlgoSrcBuf_main1_prv_rrz==NULL)||
                (RunData.AlgoSrcBuf_main2_prv_rrz==NULL)||
                (RunData.AlgoDstBuf_prv==NULL))
            {
                MY_LOGE("Preview param error: main1_IN=%p, main2_IN=%p, dst=%p",
                        RunData.AlgoSrcBuf_main1_prv_rrz,
                        RunData.AlgoSrcBuf_main2_prv_rrz,
                        RunData.AlgoDstBuf_prv);
                Result = MFALSE;
                break;
            }
            mAlgoSrcBuf_main1_prv_rrz = (IImageBuffer*)RunData.AlgoSrcBuf_main1_prv_rrz;
            mAlgoSrcBuf_main2_prv_rrz = (IImageBuffer*)RunData.AlgoSrcBuf_main2_prv_rrz;
            mAlgoDstBuf_prv = (IImageBuffer*)RunData.AlgoDstBuf_prv;

            break;
        case STEREO_SCENARIO_CAPTURE:
            if((RunData.AlgoSrcBuf_main1_cap_fullsize==NULL)||
                (RunData.AlgoSrcBuf_main1_cap_rrz==NULL)||
                (RunData.AlgoSrcBuf_main2_cap_rrz==NULL)||
                (RunData.AlgoDstBuf_cap==NULL))             // ||(RunData.DepthmapDstBuf_cap==NULL) )
            {
                MY_LOGE("Capture param error: main1_full_IN=%p, main1_IN=%p, main2_IN=%p, dst=%p, depthmap=%p",
                        RunData.AlgoSrcBuf_main1_cap_fullsize,
                        RunData.AlgoSrcBuf_main1_cap_rrz,
                        RunData.AlgoSrcBuf_main2_cap_rrz,
                        RunData.AlgoDstBuf_cap,
                        RunData.DepthmapDstBuf_cap);
                Result = MFALSE;
                break;
            }
            mAlgoSrcBuf_main1_cap_fullsize = (IImageBuffer*)RunData.AlgoSrcBuf_main1_cap_fullsize;
            mAlgoSrcBuf_main1_cap_rrz = (IImageBuffer*)RunData.AlgoSrcBuf_main1_cap_rrz;
            mAlgoSrcBuf_main2_cap_rrz = (IImageBuffer*)RunData.AlgoSrcBuf_main2_cap_rrz;
            mAlgoDstBuf_cap = (IImageBuffer*)RunData.AlgoDstBuf_cap;
            mDepthmapDstBuf_cap = (IImageBuffer*)RunData.DepthmapDstBuf_cap;
            break;
        case STEREO_SCENARIO_RECORD:
        default:
            MY_LOGE("unsupport scenario(%d)", RunData.eScenario);
            break;
    }

    MY_LOGD_IF(mEnableDebugLog,"SetParams out");
    return Result;
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoHalVendor::STEREORun(OUT_DATA_STEREO_T &OutData, MBOOL bEnableAlgo)
{
    MBOOL bResult = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    /////////// Get Stereo parameter for Bokeh process ///////////
    int DofLevel = mParamsMgr->getInt(MtkCameraParameters::KEY_STEREO_DOF_LEVEL);
    MY_LOGD_IF(mEnableDebugLog,"DofLevel=%d",DofLevel);

    // Get AF info
    _setDaf_info(mprDafTbl->daf_vec[mprDafTbl->curr_p1_frm_num % DAF_TBL_QLEN]);
    DAF_VEC_STRUCT daf_vec = _getDaf_info();
    MY_LOGD_IF(mEnableDebugLog,"curr_p1_frm_num = %d (idx: %d), curr_p2_frm_num = %d (idx: %d), is_query_happen = %d, is_daf_run=%d, af_dac_min=%d, af_dac_max=%d,af_dac_start=%d",
        mprDafTbl->curr_p1_frm_num, mprDafTbl->curr_p1_frm_num% DAF_TBL_QLEN,
        mprDafTbl->curr_p2_frm_num, mprDafTbl->curr_p2_frm_num% DAF_TBL_QLEN,
        mprDafTbl->is_query_happen,
        mprDafTbl->is_daf_run, mprDafTbl->af_dac_min, mprDafTbl->af_dac_max , mprDafTbl->af_dac_start );
    MY_LOGD_IF(mEnableDebugLog,"AF_DAC_Index = %d, af_confidence = %d, af_valid = %d, (%d,%d)~(%d,%d)",
        daf_vec.af_dac_index,daf_vec.af_confidence,mdaf_vec.af_valid,
        daf_vec.af_win_start_x,daf_vec.af_win_start_y,
        daf_vec.af_win_end_x,daf_vec.af_win_end_y);

    // Get FD info from HALFD
    if((mpHalFD) && (0 == mpHalFD->halFDGetFaceInfo(&mFD_info)))
    {
        int32_t fd_size = mFD_info.number_of_faces;
        MY_LOGD_IF(mEnableDebugLog,"FD count =%d",fd_size);
        for(int32_t i=0;i<fd_size;i++)
        {
            MY_LOGD_IF(mEnableDebugLog,"face[%d]: left=%d, top=%d, right=%d, bottom=%d",
                i,mFD_info.faces[i].rect[0],
                mFD_info.faces[i].rect[1],
                mFD_info.faces[i].rect[2],
                mFD_info.faces[i].rect[3]);
        }
    }

    ////////////////////////////////////////////
    // TO-DO : Add algo here
    // for copy test (use MDP)
    if(OutData.eScenario == STEREO_SCENARIO_PREVIEW)
    {
        if((mAlgoSrcBuf_main1_prv_rrz == NULL)||
            (mAlgoSrcBuf_main2_prv_rrz == NULL)||
            (mAlgoDstBuf_prv == NULL))
        {
            MY_LOGE("preview buffer NULL: mAlgoSrcBuf_main1_prv_rrz=%p, mAlgoSrcBuf_main2_prv_rrz=%p, mAlgoDstBuf_prv=%p",
                mAlgoSrcBuf_main1_prv_rrz,mAlgoSrcBuf_main2_prv_rrz,mAlgoDstBuf_prv);
            return MFALSE;
        }
        if(mFakeAlgoTest == 1)
        {
            // do mdp copy
            MRect crop(mAlgoSize_main1_prv_rrz.w,mAlgoSize_main1_prv_rrz.h);
            MY_LOGD_IF(mEnableDebugLog,"mdp copy prv buffer: mAlgoSrcBuf_main1_prv_rrz=%p, mAlgoDstBuf_prv=%p, resolution= %d x %d",
                    mAlgoSrcBuf_main1_prv_rrz, mAlgoDstBuf_prv, crop.s.w, crop.s.h);
            bResult = mpImgTransform->execute(mAlgoSrcBuf_main1_prv_rrz,
                                                mAlgoDstBuf_prv, NULL,
                                                crop, eTransform_None, 0xFFFFFFFF);
        }
        else if(mFakeAlgoTest == 0)
        {
            // do cpu copy
            size_t plane_cnt = mAlgoSrcBuf_main1_prv_rrz->getPlaneCount();
            for(size_t i=0;i<plane_cnt;i++)
            {
                unsigned char* InBuf = (unsigned char*)mAlgoSrcBuf_main1_prv_rrz->getBufVA(i);
                unsigned char* OutBuf = (unsigned char*)mAlgoDstBuf_prv->getBufVA(i);
                size_t buf_size = mAlgoSrcBuf_main1_prv_rrz->getBufSizeInBytes(i);
                MY_LOGD_IF(mEnableDebugLog,"CPU copy preview buffer: plan%d, InBuf=%p, OutBuf=%p, buf_size=%d",i, InBuf,OutBuf,buf_size);
                memcpy(OutBuf,InBuf,buf_size);
            }
            mAlgoDstBuf_prv->syncCache();
        }
        else if(mFakeAlgoTest == 2)
        {
            _DoFakeAlgo(true);
        }
        else if(mFakeAlgoTest == 3)
        {
            // do mdp copy from main2
            MRect crop(mAlgoSize_main2_prv_rrz.w,mAlgoSize_main2_prv_rrz.h);
            MY_LOGD_IF(mEnableDebugLog,"mdp copy prv buffer from main2: mAlgoSrcBuf_main2_prv_rrz=%p, mAlgoDstBuf_prv=%p, resolution= %d x %d",
                    mAlgoSrcBuf_main2_prv_rrz, mAlgoDstBuf_prv, crop.s.w, crop.s.h);
            bResult = mpImgTransform->execute(mAlgoSrcBuf_main2_prv_rrz,
                                                mAlgoDstBuf_prv, NULL,
                                                crop, eTransform_None, 0xFFFFFFFF);
        }

        OutData.main_crop.s     = mAlgoSize_main1_prv_rrz;
        OutData.algo_main1.s     = mAlgoSize_main1_prv_rrz;
        OutData.algo_main2.s     = mAlgoSize_main2_prv_rrz;
        _dumpBuffer(1);
        mPrvCnt++;
    }
    else if(OutData.eScenario == STEREO_SCENARIO_CAPTURE)
    {
        if((mAlgoSrcBuf_main1_cap_fullsize == NULL)||
            (mAlgoSrcBuf_main1_cap_rrz == NULL)||
            (mAlgoSrcBuf_main2_cap_rrz == NULL)||
            (mAlgoDstBuf_cap == NULL)||
            (mDepthmapDstBuf_cap == NULL))
        {
            MY_LOGE("capture buffer NULL: mAlgoSrcBuf_main1_cap_fullsize=%p, mAlgoSrcBuf_main1_cap_rrz=%p, mAlgoSrcBuf_main2_cap_rrz=%p, mAlgoDstBuf_cap=%p, mDepthmapDstBuf_cap=%p",
                mAlgoSrcBuf_main1_cap_fullsize, mAlgoSrcBuf_main1_cap_rrz, mAlgoSrcBuf_main2_cap_rrz, mAlgoDstBuf_cap, mDepthmapDstBuf_cap);
            return MFALSE;
        }

        if(mFakeAlgoTest == 1)
        {
            MRect crop(mAlgoSize_cap_fullsize.w,mAlgoSize_cap_fullsize.h);
            MSize SrcImgSize= mAlgoSrcBuf_main1_cap_fullsize->getImgSize();
            MY_LOGD_IF(mEnableDebugLog,"mdp copy cap buffer: mAlgoSrcBuf_main1_cap_fullsize=%p, src resolution= %d x %d, mAlgoDstBuf_cap=%p, dst resolution= %d x %d",
                    mAlgoSrcBuf_main1_cap_fullsize, SrcImgSize.w, SrcImgSize.h,
                    mAlgoDstBuf_cap, crop.s.w, crop.s.h);
            bResult = mpImgTransform->execute(mAlgoSrcBuf_main1_cap_fullsize,
                                                mAlgoDstBuf_cap, NULL,
                                                crop, eTransform_None, 0xFFFFFFFF);
        }
        else if(mFakeAlgoTest == 0)
        {
            // do cpu copy
            size_t plane_cnt = mAlgoSrcBuf_main1_cap_fullsize->getPlaneCount();
            MSize SrcImgSize= mAlgoSrcBuf_main1_cap_fullsize->getImgSize();
            for(size_t i=0;i<plane_cnt;i++)
            {
                unsigned char* InBuf = (unsigned char*)mAlgoSrcBuf_main1_cap_fullsize->getBufVA(i);
                unsigned char* OutBuf = (unsigned char*)mAlgoDstBuf_cap->getBufVA(i);
                size_t buf_size = mAlgoSrcBuf_main1_cap_fullsize->getBufSizeInBytes(i);
                MSize SrcImgSize= mAlgoSrcBuf_main1_cap_fullsize->getImgSize();
                MY_LOGD_IF(mEnableDebugLog,"CPU copy capture buffer: plan%d, InBuf=%p, OutBuf=%p, buf_size=%d, src resolution= %d x %d, dst resolution= %d x %d",
                    i, InBuf,OutBuf,buf_size,
                    SrcImgSize.w,SrcImgSize.h,
                    mAlgoSrcBuf_main1_cap_fullsize->getImgSize().w,mAlgoSrcBuf_main1_cap_fullsize->getImgSize().h);
                memcpy(OutBuf,InBuf,buf_size);
            }
            mAlgoDstBuf_cap->syncCache();
        }
        else
        {
            _DoFakeAlgo(false);
        }

        /////////////////////////////////////////////////
        // write dummy data into depthmap buffer for test
        // depthmap format is Y8, should be one plane only
        unsigned char* DepthmapBuf = (unsigned char*)mDepthmapDstBuf_cap->getBufVA(0);
        size_t Depthmap_size = mDepthmapDstBuf_cap->getBufSizeInBytes(0);
        unsigned char cnt=0;
        for(size_t j=0;j<Depthmap_size;j++)
        {
            DepthmapBuf[j] = cnt;
            cnt++;
            if(cnt==0xff) cnt=0;
        }
        mDepthmapDstBuf_cap->syncCache();
        /////////////////////////////////////////////////

        OutData.main_crop.s     = mAlgoSize_cap_fullsize;
        OutData.algo_main1.s     = mAlgoSize_main1_cap_rrz;
        OutData.algo_main2.s     = mAlgoSize_main2_cap_rrz;
        _dumpBuffer(0);
        mCapCnt++;
    }
    ////////////////////////////////////////////

    OutData.main_crop.p.x   = 0;  // Image Refocus
    OutData.main_crop.p.y   = 0;  // Image Refocus

    OutData.algo_main1.p.x   = 0;
    OutData.algo_main1.p.y   = 0;

    OutData.algo_main2.p.x   = 0;
    OutData.algo_main2.p.y   = 0;

    MY_LOGD_IF(mEnableDebugLog,"- X. algo_main1(%d,%d,%d,%d), algo_main2(%d,%d,%d,%d), main_crop(%d,%d,%d,%d)",
            OutData.algo_main1.p.x, OutData.algo_main1.p.y,
            OutData.algo_main1.s.w, OutData.algo_main1.s.h,
            OutData.algo_main2.p.x, OutData.algo_main2.p.y,
            OutData.algo_main2.s.w, OutData.algo_main2.s.h,
            OutData.main_crop.p.x, OutData.main_crop.p.y,
            OutData.main_crop.s.w, OutData.main_crop.s.h);

    return bResult;
}

/*******************************************************************************
*
********************************************************************************/
void
StereoHalVendor::_encodeDebugInfo()
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

    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.


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
StereoHalVendor::STEREOGetDebugInfo(DBG_DATA_STEREO_T &DbgData)
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
StereoHalVendor::STEREODestroy(void)
{
    MY_LOGD("- Enter.");
    MBOOL Result = MTRUE;
    ////////////////////////////////////////////
    // TO-DO : Add Destroy code here

    ////////////////////////////////////////////
    MY_LOGD("- Exit. Result: %d.", Result);
    return Result;

}

void
StereoHalVendor::_dumpBuffer(MBOOL isPreview)
{
    if(mDumpBuf==0)
        return;

    if(!makePath(DUMP_PATH,0660))
    {
        MY_LOGE("makePath [%s] fail",DUMP_PATH);
    }
    if(isPreview)
    {
        if((mAlgoSrcBuf_main1_prv_rrz==NULL)||(mAlgoSrcBuf_main2_prv_rrz==NULL))
        {
            MY_LOGE("error image buffer, main1=%p, main2=%p",
                mAlgoSrcBuf_main1_prv_rrz,mAlgoSrcBuf_main2_prv_rrz);
            return;
        }

        IImageBuffer* buffer = mAlgoSrcBuf_main1_prv_rrz;
        char filename[256];
        sprintf(filename, "%smain1_prv_rrz_%dx%d_%d_%06d.yuv",
                DUMP_PATH,
                buffer->getImgSize().w,
                buffer->getImgSize().h,
                buffer->getBufStridesInBytes(0),
                mPrvCnt);
        if((mDumpBuf == STEREO_VENDOR_DUMP_ALL) || (mDumpBuf == STEREO_VENDOR_DUMP_MAIN1_ONLY))
            buffer->saveToFile(filename);

        buffer = mAlgoSrcBuf_main2_prv_rrz;
        sprintf(filename, "%smain2_prv_rrz_%dx%d_%d_%06d.yuv",
                DUMP_PATH,
                buffer->getImgSize().w,
                buffer->getImgSize().h,
                buffer->getBufStridesInBytes(0),
                mPrvCnt);
        if((mDumpBuf == STEREO_VENDOR_DUMP_ALL) || (mDumpBuf == STEREO_VENDOR_DUMP_MAIN2_ONLY))
            buffer->saveToFile(filename);
        MY_LOGD_IF(mEnableDebugLog,"dump preview buffer, cnt=%d",mPrvCnt);
    }
    else
    {
        if((mAlgoSrcBuf_main1_cap_fullsize==NULL)||
            (mAlgoSrcBuf_main1_cap_rrz==NULL)||
            (mAlgoSrcBuf_main2_cap_rrz==NULL) ||
            (mDepthmapDstBuf_cap==NULL))
        {
            MY_LOGE("error image buffer, main1_full=%p, main1=%p, main2=%p",
                mAlgoSrcBuf_main1_cap_fullsize, mAlgoSrcBuf_main1_cap_rrz,mAlgoSrcBuf_main2_cap_rrz);
            return;
        }

        IImageBuffer* buffer = mAlgoSrcBuf_main1_cap_fullsize;
        char filename[256];
        sprintf(filename, "%smain1_cap_full_%dx%d_%d_%06d.yuv",
                DUMP_PATH,
                buffer->getImgSize().w,
                buffer->getImgSize().h,
                buffer->getBufStridesInBytes(0),
                mCapCnt);
        if((mDumpBuf == STEREO_VENDOR_DUMP_ALL) || (mDumpBuf == STEREO_VENDOR_DUMP_MAIN1_ONLY))
            buffer->saveToFile(filename);

        buffer = mAlgoSrcBuf_main1_cap_rrz;
        sprintf(filename, "%smain1_cap_rrz_%dx%d_%d_%06d.yuv",
                DUMP_PATH,
                buffer->getImgSize().w,
                buffer->getImgSize().h,
                buffer->getBufStridesInBytes(0),
                mCapCnt);
        if((mDumpBuf == STEREO_VENDOR_DUMP_ALL) || (mDumpBuf == STEREO_VENDOR_DUMP_MAIN1_ONLY))
            buffer->saveToFile(filename);

        buffer = mAlgoSrcBuf_main2_cap_rrz;
        sprintf(filename, "%smain2_cap_rrz_%dx%d_%d_%06d.yuv",
                DUMP_PATH,
                buffer->getImgSize().w,
                buffer->getImgSize().h,
                buffer->getBufStridesInBytes(0),
                mCapCnt);
        if((mDumpBuf == STEREO_VENDOR_DUMP_ALL) || (mDumpBuf == STEREO_VENDOR_DUMP_MAIN2_ONLY))
            buffer->saveToFile(filename);

        buffer = mDepthmapDstBuf_cap;
        sprintf(filename, "%sdepthmap_out_%dx%d_%d_%06d.yuv",
                DUMP_PATH,
                buffer->getImgSize().w,
                buffer->getImgSize().h,
                buffer->getBufStridesInBytes(0),
                mCapCnt);
        if(mDumpBuf == STEREO_VENDOR_DUMP_ALL)
            buffer->saveToFile(filename);

        MY_LOGD_IF(mEnableDebugLog,"dump capture buffer, cnt=%d",mCapCnt);
    }

}

void
StereoHalVendor::_DoFakeAlgo(MBOOL isPreview)
{
    size_t plane_cnt;
    unsigned char* InMain1Buf;
    unsigned char* InMain2Buf;
    unsigned char* OutBuf;
    size_t Main1_buf_size;
    size_t Main2_buf_size;
    size_t Main1_strides;
    size_t Main2_strides;
    IImageBuffer* OutImg;
    size_t OutImg_buf_size;
    IImageBuffer* InMain1Img;
    IImageBuffer* InMain2Img;
    size_t OutBuf_pos;
    size_t Strides_diff;

    if(isPreview)
    {
        InMain1Img = mAlgoSrcBuf_main1_prv_rrz;
        InMain2Img = mAlgoSrcBuf_main2_prv_rrz;
        OutImg = mAlgoDstBuf_prv;
    }
    else
    {
        InMain1Img = mAlgoSrcBuf_main1_cap_fullsize;
        InMain2Img = mAlgoSrcBuf_main2_cap_rrz;
        OutImg = mAlgoDstBuf_cap;
    }

    // do fake algo copy test
    // force to paste main2 image into output buffer
    plane_cnt = InMain1Img->getPlaneCount();
    for(size_t i=0;i<plane_cnt;i++)
    {
        InMain1Buf = (unsigned char*)InMain1Img->getBufVA(i);
        InMain2Buf = (unsigned char*)InMain2Img->getBufVA(i);

        OutBuf = (unsigned char*)OutImg->getBufVA(i);
        OutImg_buf_size = OutImg->getBufSizeInBytes(i);

        Main1_buf_size = InMain1Img->getBufSizeInBytes(i);
        Main1_strides = InMain1Img->getBufStridesInBytes(i);

        Main2_buf_size = InMain2Img->getBufSizeInBytes(i);
        Main2_strides = InMain2Img->getBufStridesInBytes(i);

        memcpy(OutBuf,InMain1Buf,Main1_buf_size);
        if(Main2_strides>Main1_strides)
        {
            MY_LOGE("main2 strides bigger than main1: %d > %d", Main2_strides, Main1_strides);
            return;
        }
        Strides_diff = Main1_strides-Main2_strides;

        OutBuf_pos = 0;
        for(size_t j=0;j<Main2_buf_size;j++)
        {
            if(((j%Main2_strides)==0) && (j!=0))
                OutBuf_pos += Strides_diff;

            if(OutBuf_pos<Main1_buf_size)
                OutBuf[OutBuf_pos] = InMain2Buf[j];

            OutBuf_pos++;
        }
    }
    OutImg->syncCache();
}


