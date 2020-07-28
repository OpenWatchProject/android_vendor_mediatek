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

#define LOG_TAG "stereodepth_hal"
#define MTK_LOG_ENABLE 1

#include <stdlib.h>     // for rand()

#include <mtkcam/utils/std/Log.h>
#include <libstereodepth/MTKStereoDepth.h>         // For P_STEREODEPTH_INIT_PARAM_STRUCT/MTKStereoDepth class Must be included before stereodepth_hal_base.h/stereodepth_hal.h.
#include "stereodepth_hal.h"        // For StereoHal class.
//#include <IHalSensor.h>             // For IHalSensorList class.
#include "camera_custom_nvram.h"    // For NVRAM_CAMERA_GEOMETRY_STRUCT/CAMERA_NVRAM_DATA_GEOMETRY.
#include <mtkcam/aaa/INvBufUtil.h>             // For NvBufUtil class.
#include <cutils/properties.h>      // For property_get().
#include <mtkcam/feature/stereo/hal/stereo_common.h>

//#include "camera_custom_stereo.h"  // For CUST_STEREO_* definitions.

using namespace NSCam;
using namespace NS3Av3;
using android::Mutex;           // For android::Mutex in stereodepth_hal.h.

/*******************************************************************************
*
********************************************************************************/
#define STEREODEPTH_HAL_DEBUG

#ifdef STEREODEPTH_HAL_DEBUG    // Enable debug log.

#undef __func__
#define __func__ __FUNCTION__

#define DF_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define DF_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define DF_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define DF_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else   // Disable debug log.
#define DF_LOGD(a,...)
#define DF_LOGI(a,...)
#define DF_LOGW(a,...)
#define DF_LOGE(a,...)
#endif  // STEREODEPTH_HAL_DEBUG

#define DF_LOGV_IF(cond, arg...)    if (cond) { DF_LOGV(arg); }
#define DF_LOGD_IF(cond, arg...)    if (cond) { DF_LOGD(arg); }
#define DF_LOGI_IF(cond, arg...)    if (cond) { DF_LOGI(arg); }


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
//#define ENABLE_GPU                  0

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
//static StereoDepthHalBase *pStereoDepthHal = NULL;

MFLOAT *gWorkBuf = NULL ; // Working Buffer

// Init
static STEREODEPTH_INIT_PARAM_STRUCT gStereoDepthInitInfo;

// Learning
STEREODEPTH_LEARN_INPARAM_STRUCT     gStereoDepthLearnParamIn;
STEREODEPTH_LEARN_OUTPARAM_STRUCT    gStereoDepthLearnParamOut;

// Query
STEREODEPTH_QUERY_INPARAM_STRUCT     gStereoDepthQueryParamIn;
STEREODEPTH_QUERY_OUTPARAM_STRUCT    gStereoDepthQueryParamOut;

// input data
//static MFLOAT gNVRAM_StreoAF[NVRAM_SIZE] ; // STORE the On-line Learning Information
static MFLOAT gRF_Info[RF_INFO_SIZE];
static MFLOAT gAF_Info[AF_INFO_SIZE];
static MFLOAT gFM_Info[FM_INFO_SIZE];

static MUINT8 gAF_Stat;
static MUINT16 gAF_dac, gAF_start_x, gAF_end_x, gAF_start_y, gAF_end_y;
static MFLOAT gAF_Confidence;

MFLOAT  *G_pNvRamDataArray; // For debug use.

char G_aInputValue[PROPERTY_VALUE_MAX] = {'\0'};
//    property_get(STEREO_PROPERTY_PREFIX"afinfo", G_aInputValue, "0");
//    if (aInputValue[0] == '1')
//    {
//        LOG_DBG("Force GMV X/Y (%d, %d)", i4TempNmvXFromQueue, i4TempNmvYFromQueue);
//    }


/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/

/*******************************************************************************
*
********************************************************************************/
StereoDepthHal::
StereoDepthHal()
    : mUsers(0)
    , mfRunFirstTime(MFALSE)
    , m_pStereoDepthDrv(NULL)
    , m_pHal3A(NULL)
    , m_prDafTbl(NULL)
    , LOG_ENABLED(StereoSettingProvider::isLogEnabled(PERPERTY_DEPTHMAP_NODE_LOG))
    , BENCHMARK_ENABLED(StereoSettingProvider::isProfileLogEnabled())
{
}


/*******************************************************************************
*
********************************************************************************/
StereoDepthHal::
~StereoDepthHal()
{

}

StereoDepthHal*
StereoDepthHal::
createInstance()
{
    StereoDepthHal *pStereoDepthHal = StereoDepthHal::getInstance();
    pStereoDepthHal->init();
    return pStereoDepthHal;
}

/*******************************************************************************
*
********************************************************************************/
StereoDepthHal*
StereoDepthHal::
getInstance()
{
    // DF_LOGD_IF(LOG_ENABLED, "StereoDepthHal getInstance.");
    static StereoDepthHal singleton;
    return &singleton;
}


/*******************************************************************************
*
********************************************************************************/
void
StereoDepthHal::
destroyInstance()
{
    uninit();
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoDepthHal::init()
{
    DF_LOGD_IF(LOG_ENABLED, "+. mUsers: %d.", mUsers);
    MBOOL Result = MTRUE;   // TRUE: no error. FALSE: error.

    Mutex::Autolock lock(mLock);

    if (mUsers > 0)
    {
        DF_LOGD_IF(LOG_ENABLED, "StereoDepthHal has already inited.");
        goto lb_Normal_Exit;
    }

    // Create StereoDrv instance.
    m_pStereoDepthDrv = MTKStereoDepth::createInstance(DRV_STEREODEPTH_OBJ_SW);
    if (!m_pStereoDepthDrv)
    {
        DF_LOGE("MTKStereoDepth::createInstance() fail!");
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

    if(NULL == gWorkBuf) {
        gWorkBuf = (MFLOAT*) malloc(sizeof(MFLOAT) * WORK_BUF_SIZE);
    }
    m_pHal3A = MAKE_Hal3A(0/*mOpenedSensorIndex*/, "MTKStereoDepthAf");
    if (!m_pHal3A)
    {
        DF_LOGE("IHal3A::createInstance failed!");
        goto lb_Abnormal_Exit;
    }

    // Load learning data from NVRAM.
//    Result = LoadFromNvram((int*)gDataArray, MTK_STEREO_KERNEL_NVRAM_LENGTH);
//    if (!Result)    // Maybe no data in NVRAM, so read from EEPROM.
//    {
//        DF_LOGD_IF(LOG_ENABLED, "Load from NVRAM fail (Maybe 1st time so no data in NVRAM yet).");
//    }

lb_Normal_Exit:
    android_atomic_inc(&mUsers);

    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;

lb_Abnormal_Exit:
    // m_pStereoDepthDrv Init failed, destroy StereoDepthDrv/Hal3A instance.
    if (m_pStereoDepthDrv)
    {
        m_pStereoDepthDrv->destroyInstance();
        m_pStereoDepthDrv = NULL;
    }

    if (m_pHal3A)
    {
        m_pHal3A->destroyInstance("MTKStereoDepthAf");
        m_pHal3A = NULL;
    }

    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;

}


/*******************************************************************************
*
********************************************************************************/
bool
StereoDepthHal::uninit()
{
    DF_LOGD_IF(LOG_ENABLED, "+. mUsers: %d.", mUsers);
    MBOOL Result = MTRUE;

    Mutex::Autolock lock(mLock);

    if (mUsers > 1)  // More than one user, so decrease one mUsers.
    {
        android_atomic_dec(&mUsers);
    }
    else if (mUsers == 1)   // Last user, must do some un-init procedure.
    {
        android_atomic_dec(&mUsers);

        // Save learning data to NVRAM.
//        Result = SaveToNvram((int*)gStereoKernelInitInfo.learning_data, MTK_STEREO_KERNEL_NVRAM_LENGTH);
//        if (!Result)
//        {
//            DF_LOGD_IF(LOG_ENABLED, "Save to NVRAM fail.");
//        }

        // Destroy StereoDrv instance.
        if (m_pStereoDepthDrv)
        {
            StereoDepthUninit();
            m_pStereoDepthDrv->destroyInstance();
            m_pStereoDepthDrv = NULL;
        }

        // Release working buffer memory.
        if(gWorkBuf) {
            free(gWorkBuf);
            gWorkBuf = NULL;
        }

        // Destroy Hal3A instance.
        if (m_pHal3A)
        {
            m_pHal3A->destroyInstance("MTKStereoDepthAf");
            m_pHal3A = NULL;
        }
    }
    else // mUsers <= 0. No StereoHal user, do nothing.
    {
        // do nothing.
        DF_LOGW("No StereoDepthHal to un-init. Do nothing.");
    }


    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d. mUsers: %d.", Result, mUsers);
    return Result;

}

//#if 1   // Load/Save FromNvram
/*******************************************************************************
* aDstBuf: The destination buffer which will be used to store the data read from NVRAM.
* Readlength: must use "byte" as unit.
* adb shell setprop debuglog.stereo.printloadednvram 1: print arrNVRAM that loaded from NVRAM.
********************************************************************************/
//bool
//StereoDepthHal::ReadFromNvram(/*signed int*/ MFLOAT *aDstBuf, unsigned int ReadLength)
//{
//    DF_LOGD_IF(LOG_ENABLED, "+. aDstBuf: 0x%08x. ReadLength: %d.", aDstBuf, ReadLength);
//
//    MBOOL Result = MTRUE;
//    MINT32 err = 0; // 0: no error. other value: error.
//    MUINT main1_sensor_index = 0;
//    int sensorDevIdx = 0;
////    char acDbgLogLevel[32] = {'\0'};
//
//    // Check if arrNVRAM is valid.
//    if (aDstBuf == NULL)
//    {
//        DF_LOGE("aDstBuf is NULL.");
//        Result = MFALSE;
//        goto lb_Abnormal_Exit;
//    }
//
//    IHalSensorList* sensorlsit;
//    sensorlsit = MAKE_HalSensorList();
//    sensorDevIdx = sensorlsit->querySensorDevIdx(main1_sensor_index);
//
//    NVRAM_CAMERA_GEOMETRY_STRUCT* pVoidGeoData;
//    auto pNvBufUtil = MAKE_NvBufUtil();
//    err = pNvBufUtil->getBufAndRead(CAMERA_NVRAM_DATA_GEOMETRY, sensorDevIdx, (void*&)pVoidGeoData);
//    if (err != 0)
//    {
//        DF_LOGE("Read from NVRAM fail.");
//        Result = MFALSE;
//        goto lb_Abnormal_Exit;
//    }
//
//lb_Abnormal_Exit:
//
//    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d.", Result);
//
//    return Result;
//
//}


/*******************************************************************************
* aSrcBuff: The source buffer which will write to NVRAM.
* WriteLength: must use "byte" as unit.
* adb shell setprop debuglog.stereo.erasenvram 1: set to 1 to write all 0's into NVRAM. (Remember to set to 0 after erased.)
* adb shell setprop debuglog.stereo.printsavednvram 1: print arrNVRAM that saved to NVRAM.
********************************************************************************/
//bool
//StereoDepthHal::WriteToNvram(/*signed int*/ MFLOAT *aSrcBuff, unsigned int WriteLength)
//{
//    DF_LOGD_IF(LOG_ENABLED, "+. aSrcBuff: 0x%08x. Writelength: %d.", aSrcBuff, WriteLength);
//
//    MBOOL Result = MTRUE;
//    MINT32 err = 0; // 0: no error. other value: error.
//    MUINT main1_sensor_index = 0;
//    int sensorDevIdx = 0;
////    char acDbgLogLevel[32] = {'\0'};
//
//    // Check if arrNVRAM is valid.
//    if (aSrcBuff == NULL)
//    {
//        DF_LOGE("aSrcBuff is NULL.");
//        Result = MFALSE;
//        goto lb_Abnormal_Exit;
//    }
//
//    IHalSensorList* sensorlsit;
//    sensorlsit = MAKE_HalSensorList();
//    sensorDevIdx = sensorlsit->querySensorDevIdx(main1_sensor_index);
//
//    NVRAM_CAMERA_GEOMETRY_STRUCT* pVoidGeoData;
//    auto pNvBufUtil = MAKE_NvBufUtil();
//    //err = pNvBufUtil->getBufAndRead(CAMERA_NVRAM_DATA_GEOMETRY, sensorDevIdx, (void*&)pVoidGeoData);
//    err = pNvBufUtil->write(CAMERA_NVRAM_DATA_GEOMETRY, sensorDevIdx);
//    if (err != 0)
//    {
//        DF_LOGE("Write to NVRAM fail.");
//        Result = MFALSE;
//        goto lb_Abnormal_Exit;
//    }
//
//lb_Abnormal_Exit:
//
//    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d.", Result);
//
//    return Result;
//
//}
//#endif   // Load/Save FromNvram


/*******************************************************************************
*
********************************************************************************/
bool
StereoDepthHal::StereoDepthInit(P_STEREODEPTH_HAL_INIT_PARAM_STRUCT pstStereodepthHalInitParam)
{
    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.

    DF_LOGD_IF(LOG_ENABLED, "+. fov_main(%f),fov_main2(%f),baseline(%f),pxlarr_w/h(%d,%d),main12pos(%d),pNvRamData(0x%08x,%f,%f,%f)",
        pstStereodepthHalInitParam->stereo_fov_main,
        pstStereodepthHalInitParam->stereo_fov_main2,
        pstStereodepthHalInitParam->stereo_baseline,
        pstStereodepthHalInitParam->stereo_pxlarr_width,
        pstStereodepthHalInitParam->stereo_pxlarr_height,
        pstStereodepthHalInitParam->stereo_main12_pos,
        pstStereodepthHalInitParam->pNvRamDataArray,
        *(pstStereodepthHalInitParam->pNvRamDataArray),
        *(pstStereodepthHalInitParam->pNvRamDataArray + 1),
        *(pstStereodepthHalInitParam->pNvRamDataArray + 2)
    );

    // Prepare Init Data (gStereoDepthInitInfo struct).
	P_STEREODEPTH_INIT_PARAM_STRUCT pInit = &gStereoDepthInitInfo;
    //     Step 1: Load NVRAM data into gNVRAM_StreoAF[]. => NVRAM is load in Stereo HAL init, which includes Depth-based AF NVRAM data. So don't have to do it again.
    //     Step 2: Get DAF Table.
    m_pHal3A->send3ACtrl(E3ACtrl_GetAFDAFTable, (uintptr_t)&m_prDafTbl, 0);

    //     Step 4: Config init data.
    //         Working buffer
	pInit->workbuf_addr                         = (void*) gWorkBuf;
	pInit->workbuf_size                         = WORK_BUF_SIZE;
    //         Customized parameters
    pInit->custom_param.af_dac_min              = m_prDafTbl->af_dac_min;   // get from AF HAL interface.
    pInit->custom_param.af_dac_max              = m_prDafTbl->af_dac_max;   // get from AF HAL interface.  // VENT_WORKAROUND
    pInit->custom_param.stereo_fov_main         = pstStereodepthHalInitParam->stereo_fov_main;      // get from Stereo Cam HAL, Stereo Cam HAL's initialized parameters.
    pInit->custom_param.stereo_fov_main2        = pstStereodepthHalInitParam->stereo_fov_main2;     // get from Stereo Cam HAL, Stereo Cam HAL's initialized parameters.
    pInit->custom_param.stereo_baseline         = pstStereodepthHalInitParam->stereo_baseline;      // get from Stereo Cam HAL, Stereo Cam HAL's initialized parameters.
    pInit->custom_param.stereo_pxlarr_width     = pstStereodepthHalInitParam->stereo_pxlarr_width;  // get from Stereo Cam HAL, Stereo Cam HAL's initialized parameters.
    pInit->custom_param.stereo_pxlarr_height    = pstStereodepthHalInitParam->stereo_pxlarr_height; // get from Stereo Cam HAL, Stereo Cam HAL's initialized parameters.
    pInit->custom_param.stereo_main12_pos       = pstStereodepthHalInitParam->stereo_main12_pos;    // get from Stereo Cam HAL, Stereo Cam HAL's initialized parameters.
    //         NVRAM addr
    pInit->pDataNVRam                           = pstStereodepthHalInitParam->pNvRamDataArray;
    G_pNvRamDataArray                           = pstStereodepthHalInitParam->pNvRamDataArray;   // For debug use. Used to print NVRAM data in another function (StereoDepthUninit).

    // Print first 10 NVRAM data for checking.
    property_get(STEREO_PROPERTY_PREFIX"nvram", G_aInputValue, "0");
    if (G_aInputValue[0] == '1')  // check pRFParam[i] result.
    {
        DF_LOGD_IF(LOG_ENABLED, "<pDataNVRam[]: 0x%08x (first 10 elements)>", pInit->pDataNVRam);
        for (int i = 0; i < 10; i++)
        {
            DF_LOGD_IF(LOG_ENABLED, "%f", *(pInit->pDataNVRam + i));
        }
    }

    #if 1   // Check input parameters.
    DF_LOGD_IF(LOG_ENABLED, "Wbuf_adr/sz(%p, %d),dac_mn/Mx(%d,%d),fov1/2(%f,%f),bsln(%f),pxl_w/h(%d,%d),pos(%d)",
        pInit->workbuf_addr,
        pInit->workbuf_size,
        pInit->custom_param.af_dac_min,
        pInit->custom_param.af_dac_max,
        pInit->custom_param.stereo_fov_main,
        pInit->custom_param.stereo_fov_main2,
        pInit->custom_param.stereo_baseline,
        pInit->custom_param.stereo_pxlarr_width,
        pInit->custom_param.stereo_pxlarr_height,
        pInit->custom_param.stereo_main12_pos
    );

    StereoDepthPrintDafTable();
    #endif  // Check input parameters.

    // Call Init function.
    err = m_pStereoDepthDrv->StereoDepthInit(pInit, (void *)NULL);
    if (err != 0)
    {
        DF_LOGE("Stereo Depth Init fail. err code (%d)", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

lb_Abnormal_Exit:

    mfRunFirstTime = MTRUE;

    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d.", Result);

    return Result;

}

/*******************************************************************************
*
********************************************************************************/
bool
StereoDepthHal::StereoDepthSetParams(P_STEREODEPTH_HAL_INIT_PARAM_STRUCT pstStereodepthHalInitParam)
{
    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.

    if (mfRunFirstTime) // Only run once.
    {
        DF_LOGD_IF(LOG_ENABLED, "+. pCoordTransParam(0x%08x,%f,%f,%f)",
            pstStereodepthHalInitParam->pCoordTransParam,
            *(pstStereodepthHalInitParam->pCoordTransParam),
            *(pstStereodepthHalInitParam->pCoordTransParam + 1),
            *(pstStereodepthHalInitParam->pCoordTransParam + 2)
        );

    	P_STEREODEPTH_INIT_PARAM_STRUCT pInit = &gStereoDepthInitInfo;
    	//         Rectification model
    	pInit->pRFParam                             = pstStereodepthHalInitParam->pCoordTransParam;
    	pInit->is_finish_learning                   = 1;
        // Print first 10 pRFParam[] data for checking.
        property_get(STEREO_PROPERTY_PREFIX"rfparam", G_aInputValue, "0");
        if (G_aInputValue[0] == '1')  // check pRFParam[i] result.
        {
            DF_LOGD_IF(LOG_ENABLED, "<pRFParam[]: 0x%08x (first 10 elements)>", pInit->pRFParam);
            for (int i = 0; i < 10; i++)
            {
                DF_LOGD_IF(LOG_ENABLED, "%f", *(pInit->pRFParam + i));
            }
        }


        err = m_pStereoDepthDrv->StereoDepthFeatureCtrl(STEREODEPTH_FTCTRL_SET_PROC_PARAM, (void *)pInit, (void *)NULL);
        if (err != 0)
        {
            DF_LOGE("StereoDepthFeatureCtrl(SET_PROC_PARAM) fail. err code (%d)", err);
            Result = MFALSE;
            goto lb_Abnormal_Exit;
        }

lb_Abnormal_Exit:

        DF_LOGD_IF(LOG_ENABLED, "-. Result: %d.", Result); // Only show log once when first run.
    }

    mfRunFirstTime = MFALSE;

    return Result;
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoDepthHal::StereoDepthRunLearning(MUINT16 u2NumHwfeMatch, MFLOAT *pfHwfeMatchData, P_AF_WIN_COORDINATE_STRUCT pstAfWinCoordinate)
{
    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    DF_LOGD_IF(LOG_ENABLED, "+. curr_p2_frm_num: %d (DafTbl CurrIdx: %d). u2NumHwfeMatch: %d. pfHwfeMatchData: 0x%08x (value: %d). sxsyexey(%d,%d,%d,%d)",
        m_prDafTbl->curr_p2_frm_num,
        m_prDafTbl->curr_p2_frm_num % DAF_TBL_QLEN,
        u2NumHwfeMatch,
        pfHwfeMatchData,
        *pfHwfeMatchData,
        pstAfWinCoordinate->af_win_start_x,
        pstAfWinCoordinate->af_win_end_x,
        pstAfWinCoordinate->af_win_start_y,
        pstAfWinCoordinate->af_win_end_y
    );

    // Prepare learning algorithm input parameters (gStereoDepthLearnParamIn struct).
    P_STEREODEPTH_LEARN_INPARAM_STRUCT p_input_param = &gStereoDepthLearnParamIn;
    //     Step 1: Load learning AF info into gAF_Info[].
    MUINT32 u4CurrIdx = m_prDafTbl->curr_p2_frm_num % DAF_TBL_QLEN;
    gAF_Info[0] = (MFLOAT) 0;                                           // gAF_Stat.        // always set as 0
    gAF_Info[1] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_confidence;  // gAF_Confidence.  // get from DAF_TBL
    gAF_Info[2] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_dac_index;   // gAF_dac.         // get from DAF_TBL
//    gAF_Info[3] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_win_start_x; // gAF_start_x.     // get original value from DAF_TBL, should be remapped
//    gAF_Info[4] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_win_end_x;   // gAF_end_x.       // get original value from DAF_TBL, should be remapped
//    gAF_Info[5] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_win_start_y; // gAF_start_y.     // get original value from DAF_TBL, should be remapped
//    gAF_Info[6] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_win_end_y;   // gAF_end_y.       // get original value from DAF_TBL, should be remapped
    gAF_Info[3] = (MFLOAT) pstAfWinCoordinate->af_win_start_x; // gAF_start_x.     // get original value from DAF_TBL, should be remapped
    gAF_Info[4] = (MFLOAT) pstAfWinCoordinate->af_win_end_x;   // gAF_end_x.       // get original value from DAF_TBL, should be remapped
    gAF_Info[5] = (MFLOAT) pstAfWinCoordinate->af_win_start_y; // gAF_start_y.     // get original value from DAF_TBL, should be remapped
    gAF_Info[6] = (MFLOAT) pstAfWinCoordinate->af_win_end_y;   // gAF_end_y.       // get original value from DAF_TBL, should be remapped

    DF_LOGD_IF(LOG_ENABLED, "Learning AF stat: %f; valid: %.2f; DAC: %f; CurrIdx: %d (curr_p2_frm_num: %d)", gAF_Info[0], gAF_Info[1], gAF_Info[2], u4CurrIdx, m_prDafTbl->curr_p2_frm_num);
    DF_LOGD_IF(LOG_ENABLED, "Learning AF_win: (%f, %f, %f, %f)", gAF_Info[3], gAF_Info[4], gAF_Info[5], gAF_Info[6]);

    //     Step 2: Load learning FM info into gFM_Info[].
	//         ... Update gFM_Info[i] here.
    gFM_Info[0] = u2NumHwfeMatch;
    for (int i = 0; i < u2NumHwfeMatch * FM_INFO_VECTOR_SIZE; i++)
    {
        gFM_Info[i+1] = *(pfHwfeMatchData + i);
    }

    property_get(STEREO_PROPERTY_PREFIX"learnfminfo", G_aInputValue, "0");
    if (G_aInputValue[0] == '1')  // check gFM_Info[i] result.
    {
        DF_LOGD_IF(LOG_ENABLED, "Learning gFM_Info[0]: %f ((NumHwfeMatch)", gFM_Info[0]);

        for (int i = 0; i < u2NumHwfeMatch * FM_INFO_VECTOR_SIZE; i++)
        {
            DF_LOGD_IF(LOG_ENABLED, "Learning gFM_Info[%d]: %f", i+1, gFM_Info[i+1]);
        }
    }

    //     Step 3: Config learning algorithm input parameters.
	p_input_param->input_af_info = gAF_Info;
	p_input_param->input_fm_info = gFM_Info;

    // Call Main function.
	err = m_pStereoDepthDrv->StereoDepthMain(STEREODEPTH_LEARNING_PROC, (void *) &gStereoDepthLearnParamIn, (void *) &gStereoDepthLearnParamOut);
    if (err != 0)
    {
        DF_LOGE("StereoDepthMain(STEREODEPTH_LEARNING_PROC) fail. err code (%d)", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

lb_Abnormal_Exit:

    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d.", Result);

    return Result;
}


/*******************************************************************************
*
********************************************************************************/
bool
StereoDepthHal::StereoDepthRunQuerying(MUINT16 u2NumHwfeMatch, MFLOAT *pfHwfeMatchData, P_AF_WIN_COORDINATE_STRUCT pstAfWinCoordinate)
{
    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    DF_LOGD_IF(LOG_ENABLED, "+. curr_p2_frm_num: %d (Idx: %d). is_query_happen: %d (Idx: %d). u2NumHwfeMatch: %d. pfHwfeMatchData: 0x%08x (value: %d). sxsyexey(%d,%d,%d,%d)",
        m_prDafTbl->curr_p2_frm_num,
        m_prDafTbl->curr_p2_frm_num % DAF_TBL_QLEN,
        m_prDafTbl->is_query_happen,
        m_prDafTbl->is_query_happen % DAF_TBL_QLEN,
        u2NumHwfeMatch,
        pfHwfeMatchData,
        *pfHwfeMatchData,
        pstAfWinCoordinate->af_win_start_x,
        pstAfWinCoordinate->af_win_end_x,
        pstAfWinCoordinate->af_win_start_y,
        pstAfWinCoordinate->af_win_end_y
    );

    // Prepare querying algorithm input parameters (gStereoDepthQueryParamIn struct).
    P_STEREODEPTH_QUERY_INPARAM_STRUCT p_input_param   = &gStereoDepthQueryParamIn;
    P_STEREODEPTH_QUERY_OUTPARAM_STRUCT p_output_param = &gStereoDepthQueryParamOut;
    MUINT32 u4CurrIdx   = m_prDafTbl->is_query_happen % DAF_TBL_QLEN;
    MUINT32 u4OutputIdx = m_prDafTbl->is_query_happen % DAF_TBL_QLEN;

    //     Step 1: Load querying AF info into gAF_Info[].
    gAF_Info[0] = (MFLOAT) 0;                                           // gAF_Stat.        // always set as 0
    gAF_Info[1] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_confidence;  // gAF_Confidence.  // get from DAF_TBL
    gAF_Info[2] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_dac_index;   // gAF_dac.         // get from DAF_TBL
//    gAF_Info[3] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_win_start_x; // gAF_start_x.     // get original value from DAF_TBL, should be remapped
//    gAF_Info[4] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_win_end_x;   // gAF_end_x.       // get original value from DAF_TBL, should be remapped
//    gAF_Info[5] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_win_start_y; // gAF_start_y.     // get original value from DAF_TBL, should be remapped
//    gAF_Info[6] = (MFLOAT) m_prDafTbl->daf_vec[u4CurrIdx].af_win_end_y;   // gAF_end_y.       // get original value from DAF_TBL, should be remapped
    gAF_Info[3] = (MFLOAT) pstAfWinCoordinate->af_win_start_x; // gAF_start_x.     // get original value from DAF_TBL, should be remapped
    gAF_Info[4] = (MFLOAT) pstAfWinCoordinate->af_win_end_x;   // gAF_end_x.       // get original value from DAF_TBL, should be remapped
    gAF_Info[5] = (MFLOAT) pstAfWinCoordinate->af_win_start_y; // gAF_start_y.     // get original value from DAF_TBL, should be remapped
    gAF_Info[6] = (MFLOAT) pstAfWinCoordinate->af_win_end_y;   // gAF_end_y.       // get original value from DAF_TBL, should be remapped

    DF_LOGD_IF(LOG_ENABLED, "Querying AF stat: %f; valid: %.2f; DAC: %f; CurrIdx: %d (curr_p2_frm_num: %d)", gAF_Info[0], gAF_Info[1], gAF_Info[2], u4CurrIdx, m_prDafTbl->curr_p2_frm_num);
    DF_LOGD_IF(LOG_ENABLED, "Querying AF_win: (%f, %f, %f, %f)", gAF_Info[3], gAF_Info[4], gAF_Info[5], gAF_Info[6]);

    //     Step 2: Load querying FM info into gFM_Info[].
	//         ... Update gFM_Info[i] here.
    gFM_Info[0] = u2NumHwfeMatch;
    for (int i = 0; i < u2NumHwfeMatch * FM_INFO_VECTOR_SIZE; i++)
    {
        gFM_Info[i+1] = *(pfHwfeMatchData + i);
    }

    property_get(STEREO_PROPERTY_PREFIX"queryfminfo", G_aInputValue, "0");
    if (G_aInputValue[0] == '1')  // check gFM_Info[i] result.
    {
        DF_LOGD_IF(LOG_ENABLED, "Querying gFM_Info[0]: %f ((NumHwfeMatch)", gFM_Info[0]);

        for (int i = 0; i < u2NumHwfeMatch * FM_INFO_VECTOR_SIZE; i++)
        {
            DF_LOGD_IF(LOG_ENABLED, "Querying gFM_Info[%d]: %f", i+1, gFM_Info[i+1]);
        }
    }

    //     Step 3: Config querying algorithm input parameters.
	p_input_param->input_af_info = gAF_Info;
	p_input_param->input_fm_info = gFM_Info;

    // Call Main function.
    err = m_pStereoDepthDrv->StereoDepthMain(STEREODEPTH_QUERYING_PROC, (void *) &gStereoDepthQueryParamIn, (void *) &gStereoDepthQueryParamOut);
    if (err != 0)
    {
        DF_LOGE("StereoDepthMain(STEREODEPTH_QUERYING_PROC) fail. err code (%d)", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

    // Write to DAF Table.
    m_prDafTbl->daf_vec[u4OutputIdx].daf_dac_index  = p_output_param->depth_vec[0].dac_stereo;  // VENT_TODO: index use 0?
    m_prDafTbl->daf_vec[u4OutputIdx].daf_confidence = (MUINT16)p_output_param->depth_vec[0].confidence;
    m_prDafTbl->daf_vec[u4OutputIdx].daf_distance   = p_output_param->depth_vec[0].object_distance;

    // Check result
    if (p_output_param->vec_num)
    {
        DF_LOGD_IF(LOG_ENABLED, "input: %d, result: %d, AF: %d; distance: %d cm (valid: %.3f)",
    		0/*input_dac*/,
    		p_output_param->depth_vec[0].dac_stereo,
    		gAF_Info[2]/*gAF_dac*/,
    		p_output_param->depth_vec[0].object_distance,
    		p_output_param->depth_vec[0].confidence
        );
    }
    else
    {
    	DF_LOGD_IF(LOG_ENABLED, "input dac: %d, no confidence result!", 0/*input_dac*/);
    }

lb_Abnormal_Exit:

    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d. OutputIdx: %d (is_query_happen: %d),curr_p2_frm_num(%d),daf_dac_index(%d),daf_confidence(%d),daf_distance(%d)",
        Result,
        u4OutputIdx,
        m_prDafTbl->is_query_happen,
        m_prDafTbl->curr_p2_frm_num,
        m_prDafTbl->daf_vec[u4OutputIdx].daf_dac_index,
        m_prDafTbl->daf_vec[u4OutputIdx].daf_confidence,
        m_prDafTbl->daf_vec[u4OutputIdx].daf_distance
    );

    return Result;
}

/*******************************************************************************
*
********************************************************************************/
bool
StereoDepthHal::StereoDepthQuery(MUINT16 u2NumHwfeMatch, MFLOAT *pfHwfeMatchData, P_AF_WIN_COORDINATE_STRUCT pstAfWinCoordinate)
{
    MBOOL Result = MTRUE;
    const MUINT32 QUERY_IDX = m_prDafTbl->is_query_happen % DAF_TBL_QLEN;
    DF_LOGD_IF(LOG_ENABLED, "query_frame: %d (Idx: %d). u2NumHwfeMatch: %d. pfHwfeMatchData: %p. AF win: (%u, %u), (%u, %u)",
        m_prDafTbl->is_query_happen,
        QUERY_IDX,
        u2NumHwfeMatch,
        pfHwfeMatchData,
        pstAfWinCoordinate->af_win_start_x,
        pstAfWinCoordinate->af_win_start_y,
        pstAfWinCoordinate->af_win_end_x,
        pstAfWinCoordinate->af_win_end_y
    );

    //     Step 1: Load querying AF info into gAF_Info[].
    static MFLOAT arrAFInfo[AF_INFO_SIZE];
    arrAFInfo[0] = (MFLOAT) 0;                                           // gAF_Stat.        // always set as 0
    arrAFInfo[1] = (MFLOAT) m_prDafTbl->daf_vec[QUERY_IDX].af_confidence;  // gAF_Confidence.  // get from DAF_TBL
    arrAFInfo[2] = (MFLOAT) m_prDafTbl->daf_vec[QUERY_IDX].af_dac_index;   // gAF_dac.         // get from DAF_TBL
    arrAFInfo[3] = (MFLOAT) pstAfWinCoordinate->af_win_start_x; // gAF_start_x.     // get original value from DAF_TBL, should be remapped
    arrAFInfo[4] = (MFLOAT) pstAfWinCoordinate->af_win_end_x;   // gAF_end_x.       // get original value from DAF_TBL, should be remapped
    arrAFInfo[5] = (MFLOAT) pstAfWinCoordinate->af_win_start_y; // gAF_start_y.     // get original value from DAF_TBL, should be remapped
    arrAFInfo[6] = (MFLOAT) pstAfWinCoordinate->af_win_end_y;   // gAF_end_y.       // get original value from DAF_TBL, should be remapped

    DF_LOGD_IF(LOG_ENABLED, "Querying AF stat: %f; valid: %.2f; DAC: %f; CurrIdx: %d",
             arrAFInfo[0], arrAFInfo[1], arrAFInfo[2], QUERY_IDX);
    DF_LOGD_IF(LOG_ENABLED, "Querying AF_win: (%f, %f) (%f, %f)", arrAFInfo[3], arrAFInfo[4], arrAFInfo[5], arrAFInfo[6]);

    //     Step 2: Load querying FM info into arrFMInfo[].
    //         ... Update arrFMInfo[i] here.
    static MFLOAT arrFMInfo[FM_INFO_SIZE];
    ::memset(&arrFMInfo, 0, sizeof(MFLOAT)*FM_INFO_SIZE);
    arrFMInfo[0] = u2NumHwfeMatch;
//    for (int i = 0; i < u2NumHwfeMatch * FM_INFO_VECTOR_SIZE; i++)
    for(int i = u2NumHwfeMatch * FM_INFO_VECTOR_SIZE - 1; i >= 0; i--)
    {
        arrFMInfo[i+1] = *(pfHwfeMatchData + i);
    }

    property_get(STEREO_PROPERTY_PREFIX"queryfminfo", G_aInputValue, "0");
    if (G_aInputValue[0] == '1')  // check gFM_Info[i] result.
    {
        DF_LOGD_IF(LOG_ENABLED, "Querying arrFMInfo[0]: %f ((NumHwfeMatch)", arrFMInfo[0]);

        for (int i = 0; i < u2NumHwfeMatch * FM_INFO_VECTOR_SIZE; i++)
        {
            DF_LOGD_IF(LOG_ENABLED, "Querying arrFMInfo[%d]: %f", i+1, arrFMInfo[i+1]);
        }
    }

    //     Step 3: Config querying algorithm input parameters.
    STEREODEPTH_QUERY_INPARAM_STRUCT    stereoDepthQueryInput;
	stereoDepthQueryInput.input_af_info = arrAFInfo;
	stereoDepthQueryInput.input_fm_info = arrFMInfo;

    // Call Main function.
    STEREODEPTH_QUERY_OUTPARAM_STRUCT   stereoDepthQueryOutput;
    MINT32 err = m_pStereoDepthDrv->StereoDepthMain(STEREODEPTH_QUERYING_PROC,
                                                    (void *)&stereoDepthQueryInput,
                                                    (void *)&stereoDepthQueryOutput);
    if (0 == err)
    {
        // Write to DAF Table.
        m_prDafTbl->daf_vec[QUERY_IDX].daf_dac_index  = stereoDepthQueryOutput.depth_vec[0].dac_stereo;  // VENT_TODO: index use 0?
        m_prDafTbl->daf_vec[QUERY_IDX].daf_confidence = (MUINT16)stereoDepthQueryOutput.depth_vec[0].confidence;
        m_prDafTbl->daf_vec[QUERY_IDX].daf_distance   = stereoDepthQueryOutput.depth_vec[0].object_distance;

        // Check result
        if (stereoDepthQueryOutput.vec_num)
        {
            DF_LOGD_IF(LOG_ENABLED, "input: %d, result: %d, AF: %d; distance: %d cm (valid: %.3f)",
        		0/*input_dac*/,
        		stereoDepthQueryOutput.depth_vec[0].dac_stereo,
        		arrAFInfo[2],
        		stereoDepthQueryOutput.depth_vec[0].object_distance,
        		stereoDepthQueryOutput.depth_vec[0].confidence
            );
        }
        else
        {
        	DF_LOGD_IF(LOG_ENABLED, "input dac: %d, no confidence result!", 0/*input_dac*/);
        }
    } else {
        DF_LOGE("StereoDepthMain(STEREODEPTH_QUERYING_PROC) fail. err code (%d)", err);
        Result = MFALSE;
    }

    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d. daf_dac_index(%d), daf_confidence(%d), daf_distance(%d)",
        Result,
        m_prDafTbl->daf_vec[QUERY_IDX].daf_dac_index,
        m_prDafTbl->daf_vec[QUERY_IDX].daf_confidence,
        m_prDafTbl->daf_vec[QUERY_IDX].daf_distance
    );

    return Result;
}

/*******************************************************************************
*
********************************************************************************/
bool
StereoDepthHal::StereoDepthUninit(void)
{
    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    DF_LOGD_IF(LOG_ENABLED, "+");

    err = m_pStereoDepthDrv->StereoDepthFinalize();
    if (err != 0)
    {
        DF_LOGE("StereoDepthFinalize fail. err code (%d)", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

    err = m_pStereoDepthDrv->StereoDepthReset();
    if (err != 0)
    {
        DF_LOGE("StereoDepthReset fail. err code (%d)", err);
        Result = MFALSE;
        goto lb_Abnormal_Exit;
    }

lb_Abnormal_Exit:

    // Release working buffer memory.
    free(gWorkBuf);

    // Note: NVRAM is save in Stereo HAL uninit, which includes Depth-based AF NVRAM data. So don't have to do it again.
    // Print first 10 NVRAM data for checking.
#if 1
    // Print first 10 NVRAM data for checking.
    DF_LOGD_IF(LOG_ENABLED, "<NvRamDataArray (first 10 elements)>");
    for (int i = 0; i < 10; i++)
    {
        DF_LOGD_IF(LOG_ENABLED, "%f", *(G_pNvRamDataArray + i));
    }
#endif


    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d.", Result);

    return Result;
}



/*******************************************************************************
*
********************************************************************************/
void
StereoDepthHal::StereoDepthPrintDafTable(void)
{
    property_get(STEREO_PROPERTY_PREFIX"printdaftable", G_aInputValue, "0");
    if (G_aInputValue[0] == '1')  // check pRFParam[i] result.
    {
        DF_LOGD_IF(LOG_ENABLED, "m_prDafTbl->FrmNo_p1/p2(%d,%d),dac_mn/Mx(%d,%d)",
            m_prDafTbl->curr_p1_frm_num,
            m_prDafTbl->curr_p2_frm_num,
            m_prDafTbl->af_dac_min,
            m_prDafTbl->af_dac_max
        );
        for (int i = 0; i < DAF_TBL_QLEN; i++)
        {
            DF_LOGD_IF(LOG_ENABLED, "m_prDafTbl->daf_vec[%d] FrmNo(%d),islearn/isquery(%d,%d),af_valid(%d),af_dac_index(%d),af_confidence(%d),SxSyExEy(%d,%d,%d,%d),daf_dac_index(%d),daf_confidence(%d),daf_distance(%d)",
                i,
                m_prDafTbl->daf_vec[i].frm_mun,
                m_prDafTbl->daf_vec[i].is_learning,
                m_prDafTbl->daf_vec[i].is_querying,
                m_prDafTbl->daf_vec[i].af_valid,
                m_prDafTbl->daf_vec[i].af_dac_index,
                m_prDafTbl->daf_vec[i].af_confidence,
                m_prDafTbl->daf_vec[i].af_win_start_x,
                m_prDafTbl->daf_vec[i].af_win_start_y,
                m_prDafTbl->daf_vec[i].af_win_end_x,
                m_prDafTbl->daf_vec[i].af_win_end_y,
                m_prDafTbl->daf_vec[i].daf_dac_index,
                m_prDafTbl->daf_vec[i].daf_confidence,
                m_prDafTbl->daf_vec[i].daf_distance
            );
        }
    }

}


#if 0   // Obesolete.
/*******************************************************************************
* [FLOW]
* sensor  -N3D->  algo.  -GPU->  -MDP->  display  -PANEL->  touch
*
* [TOUCH PANEL]
*
*             yD (-1000)
*          ---------------
*         |               |
*   xL    |               |  xR
* (-1000) |    +(X,Y)     |(1000)
*          ---------------
*            yT ( 1000)
*
* (X,Y) - touch position
* TOUCH_bound_xL, TOUCH_bound_xR, TOUCH_bound_yT, TOUCH_bound_yD - definition of touch panel
*
* [DISPLAY]
* P2OUT - MPD -> DISPLAY
* MDP_out_size_w, MDP_out_size_h - output
* MDP_in_size_w, MDP_in_size_h - input == GPU out
* MDP_offset_x, MDP_offset_y - offset
* MDP_crop_w, MDP_crop_h - usage
*
* [GPU]
* GPU_in_size_w, GPU_in_size_h   == ALG_in
* GPU_out_size_w, GPU_out_size_h == MDP_in
*
* [N3D]
* SENSOR -[P1-P2]-> ALG -GPU-> OUTPUT
********************************************************************************/
bool
StereoDepthHal::CoordinateRemapping(void)
{
    MBOOL Result = MTRUE;
    MINT32 err = 0; // 0: no error. other value: error.
    DF_LOGD_IF(LOG_ENABLED, "+.");

    MINT32 TOUCH_bound_xL;
    MINT32 TOUCH_bound_xR;
    MINT32 TOUCH_bound_yT;
    MINT32 TOUCH_bound_yD;

    // MDP
    MINT32 MDP_offset_x;
    MINT32 MDP_offset_y;
    MINT32 MDP_in_size_w;
    MINT32 MDP_in_size_h;
    MINT32 MDP_out_size_w;
    MINT32 MDP_out_size_h;
    MINT32 MDP_crop_w;
    MINT32 MDP_crop_h;

    // GPU
    WarpImageExtInfo *pGPU_Grid; // defined in MTKWarp.h

    // ALGO IN to SENSOR
    STEREO_KERNEL_GET_WIN_REMAP_INFO_STRUCT gStereoKernelGetWinRemapInfo ; // defined in MTKStereoKernel.h



#if 1  // CoordinateRemapping() mark all.
    MUINT32 FX, FY ;    // Remapping Cooridnates for Refocus Capture
    MUINT32 RX, RY ;    // Remapping Coordinates for Depth-AF
    MUINT32  X,  Y ;    // Input Focus Coordinates (TOUCH PANEL POSITION)
    MFLOAT tmpX, tmpY ; // Calculational parameters

    // TOUCH PANEL to DISPLAY
    tmpX = (MFLOAT)(X-TOUCH_bound_xL)/(TOUCH_bound_xR-TOUCH_bound_xL) * (MDP_out_size_w-1) ;
    tmpY = (MFLOAT)(Y-TOUCH_bound_yD)/(TOUCH_bound_yT-TOUCH_bound_yD) * (MDP_out_size_h-1) ;

    // DISPLAY to GPU OUT
    tmpX = tmpX * (MFLOAT)(MDP_crop_w-1)/(MDP_out_size_w-1) + MDP_offset_x ;
    tmpY = tmpY * (MFLOAT)(MDP_crop_h-1)/(MDP_out_size_h-1) + MDP_offset_y ;

    // GPU OUT to ALGO IN
    MINT32 GPU_MAP_w, GPU_MAP_h, *pGPU_GRID_x, *pGPU_GRID_y, idx ;
    GPU_MAP_w = pGPU_Grid->WarpMapSize[0][0] ;
    GPU_MAP_h = pGPU_Grid->WarpMapSize[0][1] ;
    pGPU_GRID_x = (MINT32*)pGPU_Grid->WarpMapAddr[0][0] ;   // pGPU_MAP_x = (MINT32*)pGPU_Grid->WarpMapAddr[0][0] ;
    pGPU_GRID_y = (MINT32*)pGPU_Grid->WarpMapAddr[0][1] ;   // pGPU_MAP_y = (MINT32*)pGPU_Grid->WarpMapAddr[0][1] ;
    tmpX = tmpX * (MFLOAT)(GPU_MAP_w-1)/(MDP_in_size_w-1) ;
    tmpY = tmpY * (MFLOAT)(GPU_MAP_h-1)/(MDP_in_size_h-1) ;
    idx = (MINT32)(tmpY+0.5f) * GPU_MAP_w + (MINT32)(tmpX+0.5f) ;
    tmpX = pGPU_GRID_x[idx] ;
    tmpY = pGPU_GRID_y[idx] ;

    // CAMERA NON-CROPPING CASE

    // ALGO IN to SENSOR
//    MTKStereoKernel* MyStereoKernel->StereoKernelFeatureCtrl(STEREO_KERNEL_FEATURE_GET_WIN_REMAP_INFO, NULL, &gStereoKernelGetWinRemapInfo); // obtain the remapped info.
    RX = (MUINT32)( tmpX * gStereoKernelGetWinRemapInfo.win_remap_depth_af_scale_x + gStereoKernelGetWinRemapInfo.win_remap_depth_af_offset_x + 0.5f ) ;
    RY = (MUINT32)( tmpY * gStereoKernelGetWinRemapInfo.win_remap_depth_af_scale_y + gStereoKernelGetWinRemapInfo.win_remap_depth_af_offset_y + 0.5f ) ;

    // ALGO IN to CAPTURE
    FX = (MUINT32)( tmpX * gStereoKernelGetWinRemapInfo.win_remap_refocus_ic_scale_x + gStereoKernelGetWinRemapInfo.win_remap_refocus_ic_offset_x + 0.5f ) ;
    FY = (MUINT32)( tmpY * gStereoKernelGetWinRemapInfo.win_remap_refocus_ic_scale_y + gStereoKernelGetWinRemapInfo.win_remap_refocus_ic_offset_y + 0.5f ) ;

#endif  // CoordinateRemapping() mark all.

    DF_LOGD_IF(LOG_ENABLED, "-. Result: %d.", Result);

    return Result;

}
#endif  // Obesolete.

