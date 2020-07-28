/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#define LOG_TAG "cctsvr_handle"

#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/poll.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <pthread.h>

#include "kd_imgsensor_define.h"

#include "sensor_drv.h"
#include <mtkcam/drv/IHalSensor.h>

//#include "CctIF.h"   			// path:vendor/mediatek/proprietary/hardware/mtkcam/include/mtkcam/main/acdk
//#include "AcdkCommon.h"		// path:vendor/mediatek/proprietary/hardware/mtkcam/include/mtkcam/main/acdk

#include "cct_feature.h"
#include "cct_op_handle.h"

#include "cct_imgtool.h"

#include <mtkcam/aaa/cct/errcode.h>
#include <mtkcam/utils/std/ULog.h>

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_hal_if.h>
#include "awb_mgr_if.h"

#include <isp_reg.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include "af_mgr_if.h"
#include <mtkcam/def/common.h>
using namespace NSCam;
#include "ae_mgr_if.h"

#include "awb_param.h"
#include "af_param.h"
#include "ae_param.h"
#include "flash_mgr.h"
#include "isp_tuning_mgr.h"
#include "isp_mgr.h"
#include <lsc/ILscMgr.h>
#include <ILscNvram.h>
#include <nvbuf_util.h>
//#include <mtkcam/aaa/aaa_hal_common.h>
//#include "IHal3A.h"
#include "isp_tuning_idx.h"


#include "liblsctrans/ShadingTblTransform.h" //LSC_CALI_INFO_T, vLsc_Remap_to_Bayer0, ...


/********************************************************************************/
#define MY_LOGD(fmt, arg...)        ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        ALOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define system_call(cmd)            ({ int ret;   printf("%s\n", cmd);   ALOGD("[SystemCall] %s", cmd);   ret = system(cmd);   ALOGD("[SystemCall] ret=%d", ret);   ret; })
#define system_sleep(milisec)       (usleep(milisec*1000))
/********************************************************************************/

using namespace NSIspTuningv3;
using namespace NS3Av3;
using namespace NSCam;


#define ISP_ID_TEXT_LENGTH          (8)

char gb_MT6763_ISP_ID_TEXT[ISP_ID_TEXT_LENGTH] = "6763";


#define CCT_AWB_NVRAM_TBL_NO            (2)     //(AWB_NVRAM_IDX_NUM)

#define CCT_CMD_OUTBUF_SIZE_COUNT       (FT_CCT_OP_SENSOR_TYPE_OP_NO + FT_CCT_OP_3A_TYPE_OP_NO + FT_CCT_OP_ISP_TYPE_OP_NO + FT_CCT_OP_CAL_TYPE_OP_NO + FT_CCT_OP_INTERNAL_TYPE_OP_NO + FT_CCT_OP_EMCAM_TYPE_OP_NO)

#define MS_SLEEP(x)             (usleep(x*1000))


MINT32 giCctCmdOutBufSize[CCT_CMD_OUTBUF_SIZE_COUNT][2] = {
#if (CAM3_3A_ISP_40_EN)
   {  FT_CCT_OP_GET_SENSOR,                     sizeof(FT_CCT_ON_BOARD_SENSOR) },
#else
   {  FT_CCT_OP_GET_SENSOR,                     sizeof(ACDK_CCT_SENSOR_INFO_STRUCT) },
#endif
   {  FT_CCT_OP_SET_SENSOR_REG,                 0 },
   {  FT_CCT_OP_GET_SENSOR_REG,                 sizeof(ACDK_CCT_REG_RW_STRUCT) },
   {  FT_CCT_OP_LSC_GET_SENSOR_RESOLUTION,      sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT) },
   {  FT_CCT_OP_GET_SENSOR_MODE_NUM,            sizeof(MINT32) },

   {  FT_CCT_OP_AE_GET_ON_OFF,                  sizeof(MINT32) },
   {  FT_CCT_OP_AE_SET_ON_OFF,                  0 },
   {  FT_CCT_OP_AE_GET_BAND,                    sizeof(MINT32) },
   {  FT_CCT_OP_AE_SET_BAND,                    0 },
   {  FT_CCT_OP_AE_GET_METERING_MODE,           sizeof(MINT32) },
   {  FT_CCT_OP_AE_SET_METERING_MODE,           0 },
   {  FT_CCT_OP_AE_GET_SCENE_MODE,              sizeof(MINT32) },
   {  FT_CCT_OP_AE_SET_SCENE_MODE,              0 },
   {  FT_CCT_OP_AE_GET_AUTO_PARA,               sizeof(ACDK_AE_MODE_CFG_T) },
   {  FT_CCT_OP_AE_SET_AUTO_PARA,               0 },
   {  FT_CCT_OP_AE_GET_CAPTURE_PARA,            sizeof(ACDK_AE_MODE_CFG_T) },
   {  FT_CCT_OP_AE_SET_CAPTURE_PARA,            0 },
   {  FT_CCT_OP_AF_GET_RANGE,                   sizeof(FOCUS_RANGE_T) },
   {  FT_CCT_OP_AF_GET_POS,                     sizeof(MINT32) },
   {  FT_CCT_OP_AF_SET_POS,                     0 },
   {  FT_CCT_OP_AWB_GET_ON_OFF,                 sizeof(MINT32) },
   {  FT_CCT_OP_AWB_SET_ON_OFF,                 0 },
   {  FT_CCT_OP_AWB_GET_LIGHT_PROB,             sizeof(AWB_LIGHT_PROBABILITY_T) },
   {  FT_CCT_OP_AWB_GET_MODE,                   sizeof(MINT32) },
   {  FT_CCT_OP_AWB_SET_MODE,                   0 },
   {  FT_CCT_OP_AWB_GET_GAIN,                   sizeof(AWB_GAIN_T) },
   {  FT_CCT_OP_AWB_SET_GAIN,                   0 },
   {  FT_CCT_OP_FLASH_GET_MODE,                 sizeof(MINT32) },
   {  FT_CCT_OP_FLASH_SET_MODE,                 0 },

   {  FT_CCT_OP_GET_ID,                         ISP_ID_TEXT_LENGTH },
   {  FT_CCT_OP_ISP_GET_ON_OFF,                 sizeof(MINT32) },
   {  FT_CCT_OP_ISP_SET_ON_OFF,                 0 },
   {  FT_CCT_OP_ISP_GET_CCM_FIXED_ON_OFF,       sizeof(MINT32) },
   {  FT_CCT_OP_ISP_SET_CCM_FIXED_ON_OFF,       0 },
   {  FT_CCT_OP_ISP_GET_CCM_MATRIX,             sizeof(ACDK_CCT_CCM_STRUCT) },
   {  FT_CCT_OP_ISP_SET_CCM_MATRIX,             0 },
   {  FT_CCT_OP_ISP_GET_INDEX,                  sizeof(CUSTOM_NVRAM_REG_INDEX) },

   {  FT_CCT_OP_ISP_GET_SMOOTH_CCM,             sizeof(MUINT32) },
   {  FT_CCT_OP_ISP_SET_SMOOTH_CCM,             0 },
#if (CAM3_3A_ISP_40_EN)
   {  FT_CCT_OP_GET_SHADING_ON_OFF,             sizeof(MINT32) },
#else
   {  FT_CCT_OP_GET_SHADING_ON_OFF,             sizeof(ACDK_CCT_MODULE_CTRL_STRUCT) },
#endif
   {  FT_CCT_OP_SET_SHADING_ON_OFF,             0 },
   {  FT_CCT_OP_GET_SHADING_INDEX,              sizeof(MINT32) },
   {  FT_CCT_OP_SET_SHADING_INDEX,              0 },
   {  FT_CCT_OP_GET_SHADING_TSF_ON_OFF,         sizeof(MINT32) },
   {  FT_CCT_OP_SET_SHADING_TSF_ON_OFF,         0 },

   {  FT_CCT_OP_CAL_CDVT_CALIBRATION,           sizeof(ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T) },
   {  FT_CCT_OP_CAL_CDVT_TEST,                  sizeof(AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T) },
   {  FT_CCT_OP_CAL_SHADING,                    sizeof(MUINT32) },
   {  FT_CCT_OP_CAL_AE_GET_FALRE,               sizeof(MUINT32) },
   {  FT_CCT_OP_CAL_AE_GET_EV,                  sizeof(MUINT32) },
   {  FT_CCT_OP_CAL_FLASH,                      sizeof(MUINT32) },

   {  FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF,    0 },
   {  FT_CCT_OP_AE_SET_AEMODE,                  0 },
   {  FT_CCT_OP_AE_GET_AEMODE,                  sizeof(MINT32) },
   {  FT_CCT_OP_AE_SET_SENSOR_PARA,             0 },
   {  FT_CCT_OP_AE_GET_SENSOR_PARA,             sizeof(AE_SENSOR_PARAM_T) },
   {  FT_CCT_OP_AF_SET_ON_OFF,                  0 },
   {  FT_CCT_OP_GET_SHADING_PARA,               sizeof(ACDK_CCT_SHADING_COMP_STRUCT) },
   {  FT_CCT_OP_SET_SHADING_TABLE_POLYCOEF,     0 },

   {  FT_CCT_OP_AF_BRECKET_STEP,                0 },
   {  FT_CCT_OP_AE_BRECKET_STEP,                0 },
   {  FT_CCT_OP_AF_SET_AFMODE,                  0 },
   {  FT_CCT_OP_AF_AUTOFOCUS,                   0 },
   {  FT_CCT_OP_AF_FULL_SCAN_SET_INTERVAL,      0 },
   {  FT_CCT_OP_AF_FULL_SCAN_SET_DACSTEP,       0 },
   {  FT_CCT_OP_AF_FULL_SCAN_TRIGGER,           0 },
   {  FT_CCT_OP_AF_SET_AREA,                    0 },
   {  FT_CCT_OP_FLASH_CALIBRATION,              0 },
   {  FT_CCT_OP_AE_SET_CAPTURE_ISO,             0 },
   {  FT_CCT_OP_AE_SET_CAPTURE_SENSOR_GAIN,     0 },
   {  FT_CCT_OP_AE_APPLY_CAPTURE_AE_PARAM,      0 },
   {  FT_CCT_OP_AE_SET_CAPTURE_EXP_TIME_US,     0 },
   {  FT_CCT_OP_AE_SET_VHDR_RATIO,              0 },
   {  FT_CCT_OP_AE_SET_AE_MODE,                 0 },
   {  FT_CCT_OP_AWB_SET_MTK_ENABLE,             0 },
   {  FT_CCT_OP_AWB_SET_SENSOR_ENABLE,          0 },
   {  FT_CCT_OP_AE_EV_CALIBRATION,              0 },
   {  FT_CCT_OP_SET_RESULT_FILE_PATH,           0 },
   {  FT_CCT_OP_AE_BRECKET_STEP_HIGH_PERSION_EV,0 },
   {  FT_CCT_OP_AE_LOCK_EXPOSURE,               0 },
   {  FT_CCT_OP_UPDATE_CAPTURE_PARAMS,          0 }

//   {  FT_CCT_OP_ISP_GET_NVRAM_NUMBER,           CCT_NVRAM_STRUCT_MAXIMUM * sizeof(MINT32) },
//   {  FT_CCT_OP_ISP_GET_NVRAM_DATA,             0 },
//   {  FT_CCT_OP_ISP_SET_NVRAM_DATA,             0 },
//   {  FT_CCT_OP_ISP_SAVE_NVRAM_DATA,            0 },

};

MINT32 giCctNvramCmdOutBufSize[CCT_NVRAM_DATA_ENUM_MAX][2] = {
   {  CCT_NVRAM_DATA_LSC_PARA,          sizeof(winmo_cct_shading_comp_struct) },
   {  CCT_NVRAM_DATA_LSC_TABLE,         sizeof(CCT_SHADING_TAB_STRUCT) },
   {  CCT_NVRAM_DATA_LSC,               sizeof(NVRAM_CAMERA_SHADING_STRUCT) },
   {  CCT_NVRAM_DATA_AE_PLINE,          sizeof(AE_PLINETABLE_T) },
   {  CCT_NVRAM_DATA_AE,                sizeof(AE_NVRAM_T) * CAM_SCENARIO_NUM },                   // need to support 7 scenarios ... Done
   {  CCT_NVRAM_DATA_AF,                sizeof(NVRAM_LENS_PARA_STRUCT) },       // need to support 7 scenarios
   {  CCT_NVRAM_DATA_AWB,               sizeof(AWB_NVRAM_T) * CAM_SCENARIO_NUM },   // need to support 7 scenarios ... Done
   {  CCT_NVRAM_DATA_ISP,               sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT) },
   {  CCT_NVRAM_DATA_FEATURE,           sizeof(NVRAM_CAMERA_FEATURE_STRUCT) },
   {  CCT_NVRAM_DATA_STROBE,            sizeof(NVRAM_CAMERA_STROBE_STRUCT) },
   {  CCT_NVRAM_DATA_FLASH_AWB,         sizeof(FLASH_AWB_NVRAM_T) }
//   {  CCT_NVRAM_DATA_FLASH_CALIBRATION, sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT) },
//   {  CCT_NVRAM_DATA_3A,                sizeof(NVRAM_CAMERA_3A_STRUCT) }
};


CctHandle*
CctHandle::
createInstance(CAMERA_DUAL_CAMERA_SENSOR_ENUM eSensorEnum)
{
    NVRAM_CAMERA_ISP_PARAM_STRUCT*  pbuf_isp;
    NVRAM_CAMERA_SHADING_STRUCT*    pbuf_shd;
    NVRAM_CAMERA_3A_STRUCT*    pbuf_3a;
    NVRAM_LENS_PARA_STRUCT*    pbuf_ln;
    NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT*    pbuf_flash_cal;
    int err;
    NvramDrvBase*	pnvram_drv;

    MY_LOGD("CctHandle createInstance +");
    pnvram_drv = NvramDrvBase::createInstance();

//    MY_LOGD("Init sensor +");
//    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
//    pIHalSensorList->searchSensors();
//    int sensorCount = pIHalSensorList->queryNumberOfSensors();
//    MY_LOGD("Init sensor -, sensorCount(%d)", sensorCount);

    NvBufUtil::getInstance().setAndroidMode(0);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, eSensorEnum, (void*&)pbuf_isp);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_SHADING, eSensorEnum, (void*&)pbuf_shd);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, eSensorEnum, (void*&)pbuf_3a);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, eSensorEnum, (void*&)pbuf_ln);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FLASH_CALIBRATION, eSensorEnum, (void*&)pbuf_flash_cal);

    MY_LOGD("CctHandle createInstance -");
    return new CctHandle(pbuf_isp, pbuf_shd, pbuf_3a, pbuf_ln, pbuf_flash_cal, pnvram_drv, eSensorEnum);
}


void
CctHandle::
destroyInstance()
{
    delete this;
}

CctHandle::
CctHandle( NVRAM_CAMERA_ISP_PARAM_STRUCT* pbuf_isp, NVRAM_CAMERA_SHADING_STRUCT* pbuf_shd, NVRAM_CAMERA_3A_STRUCT* pbuf_3a, NVRAM_LENS_PARA_STRUCT* pbuf_ln, NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT* pbuf_flash_cal, NvramDrvBase*	pnvram_drv, MUINT32 isensor_dev)
    : m_bGetSensorStaticInfo(MFALSE)
    , m_eSensorEnum((CAMERA_DUAL_CAMERA_SENSOR_ENUM)isensor_dev)
    , m_rBuf_ISP( *pbuf_isp )
    , m_rBuf_SD( *pbuf_shd )
    , m_rBuf_3A( *pbuf_3a )
    , m_rBuf_LN( *pbuf_ln )
    , m_rBuf_FC( *pbuf_flash_cal )
    , m_pNvramDrv( pnvram_drv )
    , m_rISPComm( m_rBuf_ISP.ISPComm )
    , m_rISPRegs( m_rBuf_ISP.ISPRegs )
    , m_rISPRegsIdx( m_rBuf_ISP.ISPRegs.Idx )
    , rawWidth( 0 )
    , rawHeight( 0 )
    , rawBitDepth( 0 )
    , rawColorOrder( 0 )
    , rawUnpackBuf( NULL )
    , unpackBufSize( 0 )
    , packFileSize( 0 )
    , capturCallBack(NULL)
//    , m_rISPPca( m_rBuf_ISP.ISPPca )
{
    MY_LOGD("CctHandle construct +");
    MY_LOGD("Sensor Dev (%d) is used", m_eSensorEnum);
    mSensorDev = m_eSensorEnum;
    m_pIHal3A = NULL;
    MY_LOGD("CctHandle construct -");
}


CctHandle::
~CctHandle()
{
    m_pIHal3A->send3ACtrl(E3ACtrl_Enable3ASetParams, MTRUE, 0);
    //m_pIHal3A->send3ACtrl(E3ACtrl_SetOperMode, NSIspTuning::EOperMode_Normal, 0);

    if( rawUnpackBuf )
    {
        free(rawUnpackBuf);
    }
}


void
CctHandle::
init(std::function<void(void*, MINT32)> capCB)
{
    MY_LOGD("CctHandle init +");

    capturCallBack = capCB;
    MY_LOGD("save capturCallBack function");

    if (m_eSensorEnum == DUAL_CAMERA_MAIN_SENSOR ) {
        m_pIHal3A = MAKE_Hal3A(0, "CctHandle");
    } else if (m_eSensorEnum == DUAL_CAMERA_SUB_SENSOR ) {
        m_pIHal3A = MAKE_Hal3A(1, "CctHandle");
    } else if (m_eSensorEnum == DUAL_CAMERA_MAIN_2_SENSOR ) {
        m_pIHal3A = MAKE_Hal3A(2, "CctHandle");
    } else {
        m_pIHal3A = MAKE_Hal3A(3, "CctHandle");
    }

    MY_LOGD("set AF mode to 4 for trigger AF");
    NS3Av3::IAfMgr::getInstance().setAFMode(mSensorDev, 4);

    m_pIHal3A->send3ACtrl(E3ACtrl_Enable3ASetParams, MFALSE, 0);
    //m_pIHal3A->send3ACtrl(E3ACtrl_SetOperMode, NSIspTuning::EOperMode_Meta, 0);

    MY_LOGD("CctHandle init -");
}



MINT32
CctHandle::
cct_getSensorStaticInfo()
{
    if (m_bGetSensorStaticInfo) return CCTIF_NO_ERROR;

    MINT32 err = SENSOR_NO_ERROR;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    CAM_ULOG_ASSERT(Utils::ULog::MOD_ISP_MGR, pHalSensorList != nullptr, "[%s]: make HAL sensor list fail!", __FUNCTION__);
    pHalSensorList->querySensorStaticInfo(mSensorDev,&m_SensorStaticInfo);
    m_bGetSensorStaticInfo = MTRUE;
    return SENSOR_NO_ERROR;
}


MINT32
CctHandle::
cct_QuerySensor(MVOID *a_pCCTSensorInfoOut, MUINT32 *pRealParaOutLen)
{

    MINT32 err = SENSOR_NO_ERROR;
#if 0
    char *str;
    str = (char *) a_pCCTSensorInfoOut;
    strcpy(str, "cct_QuerySensor executed");
    *pRealParaOutLen = strlen(str);
#elif (!CAM3_3A_ISP_40_EN)
    cct_getSensorStaticInfo();

    ACDK_CCT_SENSOR_INFO_STRUCT *pSensorEngInfoOut = (ACDK_CCT_SENSOR_INFO_STRUCT*)a_pCCTSensorInfoOut;
    pSensorEngInfoOut->DeviceId = m_SensorStaticInfo.sensorDevID;
    pSensorEngInfoOut->Type = static_cast<ACDK_CCT_REG_TYPE_ENUM>(m_SensorStaticInfo.sensorType);
    pSensorEngInfoOut->StartPixelBayerPtn = static_cast<ACDK_SENSOR_OUTPUT_DATA_FORMAT_ENUM>(m_SensorStaticInfo.sensorFormatOrder);
    pSensorEngInfoOut->GrabXOffset = 0;
    pSensorEngInfoOut->GrabYOffset = 0;

    printf("[CCTOPQuerySensor] Id = 0x%x\n", pSensorEngInfoOut->DeviceId);
    printf("[CCTOPQuerySensor] Type = %d\n", pSensorEngInfoOut->Type);
    printf("[CCTOPQuerySensor] StartPixelBayerPtn = %d\n", pSensorEngInfoOut->StartPixelBayerPtn);
    printf("[CCTOPQuerySensor] GrabXOffset = %d\n", pSensorEngInfoOut->GrabXOffset);
    printf("[CCTOPQuerySensor] GrabYOffset = %d\n", pSensorEngInfoOut->GrabYOffset);

    MY_LOGD("[CCTOPQuerySensor] Id = 0x%x\n", pSensorEngInfoOut->DeviceId);
    MY_LOGD("[CCTOPQuerySensor] Type = %d\n", pSensorEngInfoOut->Type);
    MY_LOGD("[CCTOPQuerySensor] StartPixelBayerPtn = %d\n", pSensorEngInfoOut->StartPixelBayerPtn);
    MY_LOGD("[CCTOPQuerySensor] GrabXOffset = %d\n", pSensorEngInfoOut->GrabXOffset);
    MY_LOGD("[CCTOPQuerySensor] GrabYOffset = %d\n", pSensorEngInfoOut->GrabYOffset);

    *pRealParaOutLen = sizeof(ACDK_CCT_SENSOR_INFO_STRUCT);
#else

    memset((void*)&m_SensorStaticInfo, 0x0, sizeof(NSCam::SensorStaticInfo));
    cct_getSensorStaticInfo();


    FT_CCT_ON_BOARD_SENSOR *pSensorEngInfoOut = (FT_CCT_ON_BOARD_SENSOR*)a_pCCTSensorInfoOut;
    pSensorEngInfoOut->sensor_count = 1;

    pSensorEngInfoOut->sensor[0].type = m_SensorStaticInfo.sensorType; // ACDK_Sensor.Type
    pSensorEngInfoOut->sensor[0].device_id = m_SensorStaticInfo.sensorDevID; // ACDK_Sensor.DeviceId
    pSensorEngInfoOut->sensor[0].width = m_SensorStaticInfo.captureWidth;
    pSensorEngInfoOut->sensor[0].height = m_SensorStaticInfo.captureHeight;
    pSensorEngInfoOut->sensor[0].start_pixel_bayer_ptn = m_SensorStaticInfo.sensorFormatOrder; // ACDK_Sensor.StartPixelBayerPtn => AP_SENSOR_OUTPUT_FORMAT_RAW_R, ...
    pSensorEngInfoOut->sensor[0].grab_x_offset = 0; // ACDK_Sensor.GrabXOffset
    pSensorEngInfoOut->sensor[0].grab_y_offset = 0;

    memset( &pSensorEngInfoOut->sensor[1], 0x0, sizeof(FT_CCT_SENSOR) );
    memset( &pSensorEngInfoOut->sensor[2], 0x0, sizeof(FT_CCT_SENSOR) );
    memset( &pSensorEngInfoOut->sensor[3], 0x0, sizeof(FT_CCT_SENSOR) );


    *pRealParaOutLen = sizeof(FT_CCT_ON_BOARD_SENSOR);


#endif
    return err;
}


MINT32
CctHandle::
cct_GetSensorRes(MVOID *pCCTSensorResOut, MUINT32 *pRealParaOutLen)
{
    MINT32 err = SENSOR_NO_ERROR;
#if 0
    char *str;
    str = (char *) pCCTSensorResOut;
    strcpy(str, "cct_GetSensorRes executed");
    *pRealParaOutLen = strlen(str);
#else
    cct_getSensorStaticInfo();

    ACDK_CCT_SENSOR_RESOLUTION_STRUCT *pSensorResolution = (ACDK_CCT_SENSOR_RESOLUTION_STRUCT *)pCCTSensorResOut;
    pSensorResolution->SensorPreviewWidth  = m_SensorStaticInfo.previewWidth;
    pSensorResolution->SensorPreviewHeight = m_SensorStaticInfo.previewHeight;
    pSensorResolution->SensorFullWidth     = m_SensorStaticInfo.captureWidth;
    pSensorResolution->SensorFullHeight    = m_SensorStaticInfo.captureHeight;
    pSensorResolution->SensorVideoWidth    = m_SensorStaticInfo.videoWidth;
    pSensorResolution->SensorVideoHeight   = m_SensorStaticInfo.videoHeight;
    pSensorResolution->SensorVideo1Width   = m_SensorStaticInfo.video1Width;
    pSensorResolution->SensorVideo1Height  = m_SensorStaticInfo.video1Height;
    pSensorResolution->SensorVideo2Width   = m_SensorStaticInfo.video2Width;
    pSensorResolution->SensorVideo2Height  = m_SensorStaticInfo.video2Height;
    pSensorResolution->SensorCustom1Width   = m_SensorStaticInfo.SensorCustom1Width;
    pSensorResolution->SensorCustom1Height  = m_SensorStaticInfo.SensorCustom1Height;
    pSensorResolution->SensorCustom2Width   = m_SensorStaticInfo.SensorCustom2Width;
    pSensorResolution->SensorCustom2Height  = m_SensorStaticInfo.SensorCustom2Height;
    pSensorResolution->SensorCustom3Width   = m_SensorStaticInfo.SensorCustom3Width;
    pSensorResolution->SensorCustom3Height  = m_SensorStaticInfo.SensorCustom3Height;
    pSensorResolution->SensorCustom4Width   = m_SensorStaticInfo.SensorCustom4Width;
    pSensorResolution->SensorCustom4Height  = m_SensorStaticInfo.SensorCustom4Height;
    pSensorResolution->SensorCustom5Width   = m_SensorStaticInfo.SensorCustom5Width;
    pSensorResolution->SensorCustom5Height  = m_SensorStaticInfo.SensorCustom5Height;

    printf("[CCTOPGetSensorRes] PreviewWidth = %d, PreviewHeight = %d\n", pSensorResolution->SensorPreviewWidth, pSensorResolution->SensorPreviewHeight);
    printf("[CCTOPGetSensorRes] SensorFullWidth = %d, SensorFullHeight = %d\n", pSensorResolution->SensorFullWidth, pSensorResolution->SensorFullHeight);
    printf("[CCTOPGetSensorRes] SensorVideoWidth = %d, SensorVideoHeight = %d\n", pSensorResolution->SensorVideoWidth, pSensorResolution->SensorVideoHeight);
    printf("[CCTOPGetSensorRes] SensorVideo1Width = %d, SensorVideo1Height = %d\n", pSensorResolution->SensorVideo1Width, pSensorResolution->SensorVideo1Height);
    printf("[CCTOPGetSensorRes] SensorVideo2Width = %d, SensorVideo2Height = %d\n", pSensorResolution->SensorVideo2Width, pSensorResolution->SensorVideo2Height);
    printf("[CCTOPGetSensorRes] SensorCustom1Width = %d, SensorCustom1Height = %d\n", pSensorResolution->SensorCustom1Width, pSensorResolution->SensorCustom1Height);
    printf("[CCTOPGetSensorRes] SensorCustom2Width = %d, SensorCustom2Height = %d\n", pSensorResolution->SensorCustom2Width, pSensorResolution->SensorCustom2Height);
    printf("[CCTOPGetSensorRes] SensorCustom3Width = %d, SensorCustom3Height = %d\n", pSensorResolution->SensorCustom3Width, pSensorResolution->SensorCustom3Height);
    printf("[CCTOPGetSensorRes] SensorCustom4Width = %d, SensorCustom4Height = %d\n", pSensorResolution->SensorCustom4Width, pSensorResolution->SensorCustom4Height);
    printf("[CCTOPGetSensorRes] SensorCustom5Width = %d, SensorCustom5Height = %d\n", pSensorResolution->SensorCustom5Width, pSensorResolution->SensorCustom5Height);

    MY_LOGD("[CCTOPGetSensorRes] PreviewWidth = %d, PreviewHeight = %d\n", pSensorResolution->SensorPreviewWidth, pSensorResolution->SensorPreviewHeight);
    MY_LOGD("[CCTOPGetSensorRes] SensorFullWidth = %d, SensorFullHeight = %d\n", pSensorResolution->SensorFullWidth, pSensorResolution->SensorFullHeight);
    MY_LOGD("[CCTOPGetSensorRes] SensorVideoWidth = %d, SensorVideoHeight = %d\n", pSensorResolution->SensorVideoWidth, pSensorResolution->SensorVideoHeight);
    MY_LOGD("[CCTOPGetSensorRes] SensorVideo1Width = %d, SensorVideo1Height = %d\n", pSensorResolution->SensorVideo1Width, pSensorResolution->SensorVideo1Height);
    MY_LOGD("[CCTOPGetSensorRes] SensorVideo2Width = %d, SensorVideo2Height = %d\n", pSensorResolution->SensorVideo2Width, pSensorResolution->SensorVideo2Height);
    MY_LOGD("[CCTOPGetSensorRes] SensorCustom1Width = %d, SensorCustom1Height = %d\n", pSensorResolution->SensorCustom1Width, pSensorResolution->SensorCustom1Height);
    MY_LOGD("[CCTOPGetSensorRes] SensorCustom2Width = %d, SensorCustom2Height = %d\n", pSensorResolution->SensorCustom2Width, pSensorResolution->SensorCustom2Height);
    MY_LOGD("[CCTOPGetSensorRes] SensorCustom3Width = %d, SensorCustom3Height = %d\n", pSensorResolution->SensorCustom3Width, pSensorResolution->SensorCustom3Height);
    MY_LOGD("[CCTOPGetSensorRes] SensorCustom4Width = %d, SensorCustom4Height = %d\n", pSensorResolution->SensorCustom4Width, pSensorResolution->SensorCustom4Height);
    MY_LOGD("[CCTOPGetSensorRes] SensorCustom5Width = %d, SensorCustom5Height = %d\n", pSensorResolution->SensorCustom5Width, pSensorResolution->SensorCustom5Height);

    *pRealParaOutLen = sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT);
#endif
    return err;
}


MINT32
CctHandle::
cct_ReadSensorReg(MVOID *puParaIn, MVOID *puParaOut, MUINT32 *pu4RealParaOutLen)
{
    MINT32 err = SENSOR_NO_ERROR;
    ACDK_SENSOR_FEATURE_ENUM eSensorFeature = SENSOR_FEATURE_GET_REGISTER;
    PACDK_CCT_REG_RW_STRUCT pSensorRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    PACDK_CCT_REG_RW_STRUCT pSensorRegInfoOut = (PACDK_CCT_REG_RW_STRUCT)puParaOut;
    MUINT32 Data[2], sensorParaLen;
    IHalSensor *pHalSensorObj;

    MY_LOGD("[ACDK_CCT_OP_READ_SENSOR_REG]\n");

    Data[0] = pSensorRegInfoIn->RegAddr;
    Data[1] = 0;

    sensorParaLen = 2 * sizeof(MUINT32);

    // Sensor hal init
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    CAM_ULOG_ASSERT(Utils::ULog::MOD_ISP_MGR, pIHalSensorList != nullptr, "[%s]: make HAL sensor list fail!", __FUNCTION__);
    pHalSensorObj = pIHalSensorList->createSensor("cct_sensor_access", 0);

    if(pHalSensorObj == NULL) {
        MY_LOGE("[AAA Sensor Mgr] Can not create SensorHal obj\n");
        return SENSOR_INVALID_SENSOR;
    }

    err = pHalSensorObj->sendCommand(mSensorDev, SENSOR_CMD_SET_CCT_FEATURE_CONTROL, (MUINTPTR)&eSensorFeature, (MUINTPTR)&Data[0], (MUINTPTR)&sensorParaLen);

    pSensorRegInfoOut->RegAddr = pSensorRegInfoIn->RegAddr;
    pSensorRegInfoOut->RegData = Data[1];

    *pu4RealParaOutLen = sizeof(ACDK_CCT_REG_RW_STRUCT);

    if (err != SENSOR_NO_ERROR) {
        MY_LOGE("[CCTOReadSensorReg() error]\n");
        return err;
    }

    MY_LOGD("[CCTOReadSensorReg] regAddr = %x, regData = %x\n", Data[0], Data[1]);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
CctHandle::
cct_WriteSensorReg(MVOID *puParaIn)
{
    MINT32 err = SENSOR_NO_ERROR;
    ACDK_SENSOR_FEATURE_ENUM eSensorFeature = SENSOR_FEATURE_SET_REGISTER;
    PACDK_CCT_REG_RW_STRUCT pSensorRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    MUINT32 Data[2], sensorParaLen;
    IHalSensor *pHalSensorObj;

    MY_LOGD("[ACDK_CCT_OP_WRITE_SENSOR_REG]\n");

    Data[0] = pSensorRegInfoIn->RegAddr;
    Data[1] = pSensorRegInfoIn->RegData;

    sensorParaLen = 2 * sizeof(MUINT32);

    // Sensor hal init
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    CAM_ULOG_ASSERT(Utils::ULog::MOD_ISP_MGR, pIHalSensorList != nullptr, "[%s]: make HAL sensor list fail!", __FUNCTION__);
    pHalSensorObj = pIHalSensorList->createSensor("cct_sensor_access", 0);

    if(pHalSensorObj == NULL) {
        MY_LOGE("[AAA Sensor Mgr] Can not create SensorHal obj\n");
        return SENSOR_INVALID_SENSOR;
    }

    err = pHalSensorObj->sendCommand(mSensorDev, SENSOR_CMD_SET_CCT_FEATURE_CONTROL, (MUINTPTR)&eSensorFeature, (MUINTPTR)&Data[0], (MUINTPTR)&sensorParaLen);

    if (err != SENSOR_NO_ERROR) {
        MY_LOGE("[CCTOPWriteSensorReg() error]\n");
        return err;
    }

    MY_LOGD("[CCTOPWriteSensorReg] regAddr = %x, regData = %x\n", Data[0], Data[1]);

    return err;
}

MVOID
CctHandle::
setIspOnOff_SL2F(MBOOL const fgOn)
{
    ISP_MGR_SL2F_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_DBS(MBOOL const fgOn)
{
    ISP_MGR_DBS_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuning::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MVOID
CctHandle::
setIspOnOff_OBC(MBOOL const fgOn)
{
    ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctHandle::
setIspOnOff_BPC(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.BNR_BPC;

    //m_rISPRegs.BPC[u4Index].con.bits.BPC_ENABLE = fgOn;
    //m_rISPRegs.BNR_BPC[u4Index].con.bits.BPC_EN = fgOn; //definition change: CAM_BPC_CON CAM+0800H

    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTBPCEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MVOID
CctHandle::
setIspOnOff_NR1(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.BNR_NR1;

    //m_rISPRegs.BNR_NR1[u4Index].con.bits.NR1_CT_EN = fgOn;

    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_PDC(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.BNR_PDC;

    //m_rISPRegs.BNR_PDC[u4Index].con.bits.PDC_EN = fgOn;

    ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_RMM(MBOOL const fgOn)
{

    ISP_MGR_RMM_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_RNR(MBOOL const fgOn)
{

    ISP_MGR_RNR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_SL2(MBOOL const fgOn)
{

    ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}


MVOID
CctHandle::
setIspOnOff_UDM(MBOOL const fgOn)
{

    MUINT32 u4Index = m_rISPRegsIdx.UDM;

    ISP_MGR_UDM::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);

    (fgOn == MTRUE) ? (ISP_MGR_UDM::getInstance((ESensorDev_T)m_eSensorEnum).put(m_rISPRegs.UDM[u4Index]))
                    : (ISP_MGR_UDM::getInstance((ESensorDev_T)m_eSensorEnum).put(m_rISPRegs.UDM[NVRAM_UDM_TBL_NUM - 1]));

    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_CFA::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_CFA::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctHandle::
setIspOnOff_CCM(MBOOL const fgOn)
{
    ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_GGM(MBOOL const fgOn)
{
    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_IHDR_GGM(MBOOL const fgOn)
{
//    ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
//    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
    //ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_PCA(MBOOL const fgOn)
{
    ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}



MVOID
CctHandle::
setIspOnOff_ANR(MBOOL const fgOn)
{

    ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTANR1Enable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctHandle::
setIspOnOff_ANR2(MBOOL const fgOn)
{

    ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTANR2Enable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctHandle::
setIspOnOff_CCR(MBOOL const fgOn)
{

    ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTCCREnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctHandle::
setIspOnOff_BOK(MBOOL const fgOn)
{

    ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTBOKEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctHandle::
setIspOnOff_HFG(MBOOL const fgOn)
{

    ISP_MGR_HFG_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctHandle::
setIspOnOff_EE(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.EE;

// Choo
//    ISP_DIP_X_SEEE_SRK_CTRL_T &EECtrl_SRK = m_rISPRegs.EE[u4Index].srk_ctrl.bits;
//    ISP_DIP_X_SEEE_CLIP_CTRL_T &EECtrl_Clip = m_rISPRegs.EE[u4Index].clip_ctrl.bits;

//    EECtrl_Clip.SEEE_OVRSH_CLIP_EN = fgOn;//definition change: CAM_SEEE_CLIP_CTRL CAM+AA4H

    ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEEEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);

}

MVOID
CctHandle::
setIspOnOff_NR3D(MBOOL const fgOn)
{
    //Temp. Mark
    ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).setNr3dEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_MFB(MBOOL const fgOn)
{
    //Temp. Mark
    ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_MIXER3(MBOOL const fgOn)
{
    //Temp. Mark
    ISP_MGR_MIXER3_T::getInstance((ESensorDev_T)m_eSensorEnum).setEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_COLOR(MBOOL const fgOn)
{
    ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTColorEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOGD("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_HLR(MBOOL const fgOn)
{
    //ISP_MGR_HLR_T::getInstance((ESensorDev_T)m_eSensorEnum).setCCTEnable(fgOn);
    //NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] HLR not enabled, fgOn(%d)", __FUNCTION__, fgOn);
}

MVOID
CctHandle::
setIspOnOff_ABF(MBOOL const fgOn)
{
    ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).setABFEnable(fgOn);
    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
    MY_LOG("[%s] fgOn = %d", __FUNCTION__, fgOn);
}

MBOOL
CctHandle::
getIspOnOff_SL2F() const
{
    return ISP_MGR_SL2F_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctHandle::
getIspOnOff_DBS() const
{
    return ISP_MGR_DBS_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}


MBOOL
CctHandle::
getIspOnOff_OBC() const
{
    return ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctHandle::
getIspOnOff_BPC() const
{
    return  ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTBPCEnable();
}


MBOOL
CctHandle::
getIspOnOff_NR1() const
{
    return  ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTCTEnable();
}

MBOOL
CctHandle::
getIspOnOff_PDC() const
{
    return  ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTPDCEnable();
}

MBOOL
CctHandle::
getIspOnOff_RMM() const
{

    return  ISP_MGR_RMM_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();

}

MBOOL
CctHandle::
getIspOnOff_RNR() const
{

    return  ISP_MGR_RNR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();

}

MBOOL
CctHandle::
getIspOnOff_SL2() const
{

    return  ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();

}


MBOOL
CctHandle::
getIspOnOff_UDM() const
{

    return  ISP_MGR_UDM::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();

}

MBOOL
CctHandle::
getIspOnOff_CCM() const
{
    return ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctHandle::
getIspOnOff_GGM() const
{
    return  ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctHandle::
getIspOnOff_IHDR_GGM() const
{
    return MFALSE;   // temp.
//    return  ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctHandle::
getIspOnOff_PCA() const
{
    return  ISP_MGR_PCA_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctHandle::
getIspOnOff_ANR() const
{

        return (ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTANR1Enable());
}

MBOOL
CctHandle::
getIspOnOff_ANR2() const
{

        return (ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTANR2Enable());
}


MBOOL
CctHandle::
getIspOnOff_CCR() const
{

        return ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTCCREnable();

}

MBOOL
CctHandle::
getIspOnOff_BOK() const
{

        return ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTBOKEnable();

}

MBOOL
CctHandle::
getIspOnOff_HFG() const
{
    return ISP_MGR_HFG_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctHandle::
getIspOnOff_EE() const
{
    return ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEEEnable();
}

MBOOL
CctHandle::
getIspOnOff_NR3D() const
{
    return ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).isNr3dEnable();
}

MBOOL
CctHandle::
getIspOnOff_MFB() const
{
    return MFALSE;
    //Temp. Mark
    //return ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
}

MBOOL
CctHandle::
getIspOnOff_MIXER3() const
{
    return MFALSE;
    //Temp. Mark
    //return ISP_MGR_MIXER3_T::getInstance((ESensorDev_T)m_eSensorEnum).isEnable();
}

MBOOL
CctHandle::
getIspOnOff_COLOR() const
{
    return  ISP_MGR_NR3D_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTColorEnable();
}

MBOOL
CctHandle::
getIspOnOff_HLR() const
{
    return  MFALSE;
    //return  ISP_MGR_HLR_T::getInstance((ESensorDev_T)m_eSensorEnum).isCCTEnable();
}

MBOOL
CctHandle::
getIspOnOff_ABF() const
{
    return  ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).isABFEnable();
}

MINT32
CctHandle::
setIspOnOff(MUINT32 const u4Category, MBOOL const fgOn)
{
#define SET_ISP_ON_OFF(_category)\
    case ISP_CATEGORY_##_category:\
        setIspOnOff_##_category(fgOn);\
        MY_LOGD("[setIspOnOff] < %s >", #_category);\
        break

    switch  ( u4Category )
    {
        SET_ISP_ON_OFF(SL2F);
        SET_ISP_ON_OFF(DBS);
        SET_ISP_ON_OFF(OBC);
        SET_ISP_ON_OFF(BPC);    //BNR_BPC
        SET_ISP_ON_OFF(NR1);    //BNR_NR1
        SET_ISP_ON_OFF(PDC);    //BNR_PDC
        SET_ISP_ON_OFF(RMM);
        SET_ISP_ON_OFF(RNR);
        SET_ISP_ON_OFF(SL2);
        SET_ISP_ON_OFF(UDM);
        SET_ISP_ON_OFF(CCM);
        SET_ISP_ON_OFF(GGM);         // new
//        SET_ISP_ON_OFF(IHDR_GGM);    // new
        SET_ISP_ON_OFF(ANR);
        SET_ISP_ON_OFF(ANR2);
        SET_ISP_ON_OFF(CCR);
        SET_ISP_ON_OFF(BOK);         // new
        SET_ISP_ON_OFF(HFG);
        SET_ISP_ON_OFF(EE);
//        SET_ISP_ON_OFF(NR3D);        // new   // based on Choo, CCT should not control NR3D
        SET_ISP_ON_OFF(MFB);
        SET_ISP_ON_OFF(MIXER3);
        SET_ISP_ON_OFF(PCA);       // new
        SET_ISP_ON_OFF(COLOR);       // new
        SET_ISP_ON_OFF(HLR);       // new
        SET_ISP_ON_OFF(ABF);       // new

        default:
            MY_ERR("[setIspOnOff] Unsupported Category(%d)", u4Category);
            return  CCTIF_BAD_PARAM;
    }
    MY_LOGD("[%s] (u4Category, fgOn) = (%d, %d)", __FUNCTION__, u4Category, fgOn);
    return  CCTIF_NO_ERROR;
}


MINT32
CctHandle::
getIspOnOff(MUINT32 const u4Category, MBOOL& rfgOn) const
{
#define GET_ISP_ON_OFF(_category)\
    case ISP_CATEGORY_##_category:\
        MY_LOGD("[getIspOnOff] < %s >", #_category);\
        rfgOn = getIspOnOff_##_category();\
        break

    switch  ( u4Category )
    {
        GET_ISP_ON_OFF(SL2F);
        GET_ISP_ON_OFF(DBS);
        GET_ISP_ON_OFF(OBC);
        GET_ISP_ON_OFF(BPC);    //BNR_BPC
        GET_ISP_ON_OFF(NR1);    //BNR_NR1
        GET_ISP_ON_OFF(PDC);    //BNR_PDC
        GET_ISP_ON_OFF(RMM);
        GET_ISP_ON_OFF(RNR);
        GET_ISP_ON_OFF(SL2);
        GET_ISP_ON_OFF(UDM);
        GET_ISP_ON_OFF(CCM);
        GET_ISP_ON_OFF(GGM);        // new
//        GET_ISP_ON_OFF(IHDR_GGM);   // new
        GET_ISP_ON_OFF(ANR);
        GET_ISP_ON_OFF(ANR2);
        GET_ISP_ON_OFF(CCR);
        GET_ISP_ON_OFF(BOK);        // new
        GET_ISP_ON_OFF(HFG);
        GET_ISP_ON_OFF(EE);
        GET_ISP_ON_OFF(NR3D);       // new
        GET_ISP_ON_OFF(MFB);
        GET_ISP_ON_OFF(MIXER3);
        GET_ISP_ON_OFF(PCA);      // new
        GET_ISP_ON_OFF(COLOR);      // new
        GET_ISP_ON_OFF(HLR);      // new
        GET_ISP_ON_OFF(ABF);      // new

        default:
            MY_ERR("[getIspOnOff] Unsupported Category(%d)", u4Category);
            return  CCTIF_BAD_PARAM;
    }
    MY_LOGD("[%s] (u4Category, rfgOn) = (%d, %d)", __FUNCTION__, u4Category, rfgOn);
    return  CCTIF_NO_ERROR;
}


MINT32
CctHandle::
cct_HandleSensorOp(CCT_OP_ID op,
                MUINT32 u4ParaInLen,
                MUINT8 *puParaIn,
                MUINT32 u4ParaOutLen,
                MUINT8 *puParaOut,
                MUINT32 *pu4RealParaOutLen )
{
    MINT32 err = CCTIF_NO_ERROR;
    MY_LOGD("[%s] op(%d), u4ParaInLen(%d), puParaIn(%p), u4ParaOutLen(%d), puParaOut(%p), pu4RealParaOutLen(%p)", __FUNCTION__, \
      op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);

    switch(op) {
    case FT_CCT_OP_GET_SENSOR:
        if (u4ParaOutLen == sizeof(FT_CCT_ON_BOARD_SENSOR) && puParaOut != NULL)
            err = cct_QuerySensor(puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_SWITCH_SENSOR:
        break;
    case FT_CCT_OP_SET_SENSOR_REG:
        if (u4ParaInLen == sizeof(ACDK_CCT_REG_RW_STRUCT) && puParaIn != NULL)
            err = cct_WriteSensorReg(puParaIn);
        else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_GET_SENSOR_REG:
        if (u4ParaInLen == sizeof(ACDK_CCT_REG_RW_STRUCT) && puParaIn != NULL && puParaOut != NULL)
            err = cct_ReadSensorReg(puParaIn, puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_LSC_GET_SENSOR_RESOLUTION:
        if (puParaOut != NULL)
            err = cct_GetSensorRes(puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_GET_SENSOR_MODE_NUM:
        if (puParaOut != NULL) {
            *(MINT32*)puParaOut = 3;
            *pu4RealParaOutLen = 4;
        }
        break;
    case FT_CCT_OP_TEST_SET_PROP:       // cmd for test only
        if (puParaIn != NULL && u4ParaInLen > 0) {
            char cmd[1024];
            char val[1024];
            MINT32 cmdlen;

            cmdlen = strlen((char *)puParaIn);

            strncpy(cmd, (char *)puParaIn, 1023);
            strncpy(val, (char *)(puParaIn+cmdlen+1), 1023);
            MY_LOGD("Property %s set to %s",cmd,val);
            property_set( cmd, val );
        }
        else {
            err = CCTIF_BAD_PARAM;
        }
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_TEST_GET_PROP:       // cmd for test only
        if (puParaIn != NULL && u4ParaInLen > 0) {
            char cmd[1024];
            char val[1024];
            MINT32 cmdlen;

            cmdlen = strlen((char *)puParaIn);

            strncpy(cmd, (char *)puParaIn, cmdlen);
            property_get( cmd, val, NULL );
            MY_LOGD("Property %s is %s",cmd,val);
        }
        else {
            err = CCTIF_BAD_PARAM;
        }
        *pu4RealParaOutLen = 0;
        break;
    default:
        MY_LOGD("Not support cmd %d", (int)op);
        break;
    }
    return err;
}


MINT32
CctHandle::
cct_Handle3AOp(CCT_OP_ID op,
                MUINT32 u4ParaInLen,
                MUINT8 *puParaIn,
                MUINT32 u4ParaOutLen,
                MUINT8 *puParaOut,
                MUINT32 *pu4RealParaOutLen )
{
    MY_LOGD("[%s] op(%d), u4ParaInLen(%d), puParaIn(%p), u4ParaOutLen(%d), puParaOut(%p), pu4RealParaOutLen(%p)", __FUNCTION__, \
      op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);

    int ret;
    MINT32 err = CCTIF_NO_ERROR;
    MINT32 *i32In = NULL;
    MUINT32 mode;
    MBOOL enable;

    switch(op) {
    case FT_CCT_OP_AE_GET_ON_OFF:
        if (puParaOut != NULL)
            err = NS3Av3::IAeMgr::getInstance().CCTOPAEGetEnableInfo(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AE_SET_ON_OFF:
        if (u4ParaInLen == sizeof(MBOOL)) {
            enable = (MBOOL) *puParaIn;
            if (enable) {
                MY_LOGD("set normal mode");
                m_pIHal3A->send3ACtrl(E3ACtrl_SetOperMode, NSIspTuning::EOperMode_Normal, 0);
                err = NS3Av3::IAeMgr::getInstance().CCTOPAEEnable(mSensorDev);
            }
            else {
                MY_LOGD("set meta mode");
                m_pIHal3A->send3ACtrl(E3ACtrl_SetOperMode, NSIspTuning::EOperMode_Meta, 0);
                err = NS3Av3::IAeMgr::getInstance().CCTOPAEDisable(mSensorDev);
            }
        } else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_GET_BAND:
        if (puParaOut != NULL)
            err = NS3Av3::IAeMgr::getInstance().CCTOPAEGetFlickerMode(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AE_SET_BAND:
        if (u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            err = NS3Av3::IAeMgr::getInstance().CCTOPAESetFlickerMode(mSensorDev, *i32In);
        }
        else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_GET_METERING_MODE:
        if (puParaOut != NULL)
            err = NS3Av3::IAeMgr::getInstance().CCTOPAEGetMeteringMode(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AE_SET_METERING_MODE:
        if (u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            err = NS3Av3::IAeMgr::getInstance().CCTOPAESetMeteringMode(mSensorDev, *i32In);
        }
        else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_GET_SCENE_MODE:
        if (puParaOut != NULL)
            err = NS3Av3::IAeMgr::getInstance().CCTOPAEGetAEScene(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AE_SET_SCENE_MODE:
        if (u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            err = NS3Av3::IAeMgr::getInstance().CCTOPAESetAEScene(mSensorDev, *i32In);
        }
        else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_GET_AUTO_PARA:
        if (puParaOut != NULL)
            err = NS3Av3::IAeMgr::getInstance().CCTOPAEGetExpParam(mSensorDev, (VOID *)puParaIn, (VOID *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AE_SET_AUTO_PARA:
        if ( u4ParaInLen == sizeof(ACDK_AE_MODE_CFG_T))
            err = NS3Av3::IAeMgr::getInstance().CCTOPAEApplyExpParam(mSensorDev, (VOID *)puParaIn);
        else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_GET_CAPTURE_PARA:
        if (puParaOut != NULL) {
            err = NS3Av3::IAeMgr::getInstance().CCTOGetCaptureParams(mSensorDev, (VOID *)puParaOut);
            *pu4RealParaOutLen = sizeof(ACDK_AE_MODE_CFG_T);
        } else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AE_SET_CAPTURE_PARA:
        if ( u4ParaInLen == sizeof(ACDK_AE_MODE_CFG_T))
            err = NS3Av3::IAeMgr::getInstance().CCTOSetCaptureParams(mSensorDev, (VOID *)puParaIn);
        else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AF_GET_RANGE:
        if (puParaOut != NULL)
            err = NS3Av3::IAfMgr::getInstance().CCTOPAFGetFocusRange(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AF_GET_POS:
        if (puParaOut != NULL)
            err = NS3Av3::IAfMgr::getInstance().CCTOPAFGetBestPos(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AF_SET_POS:
        if (u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            if (*i32In < 0)
                err = NS3Av3::IAfMgr::getInstance().CCTOPAFOpeartion(mSensorDev);
            else
                err = NS3Av3::IAfMgr::getInstance().CCTOPMFOpeartion(mSensorDev, *i32In);
        }
        else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AWB_GET_ON_OFF:
        if (puParaOut != NULL)
            err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBGetEnableInfo(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AWB_SET_ON_OFF:
        if (u4ParaInLen == sizeof(MBOOL)) {
            enable = (MBOOL) *puParaIn;
            if (enable)
                err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBEnable(mSensorDev);
            else
                err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBDisable(mSensorDev);
        } else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AWB_GET_LIGHT_PROB:
        if (puParaOut != NULL)
            err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBGetLightProb(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AWB_GET_MODE:
        if (puParaOut != NULL)
            err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBGetAWBMode(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AWB_SET_MODE:
        if (u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            if(*i32In < (MINT32)LIB3A_AWB_MODE_NUM)
              err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBSetAWBMode(mSensorDev, *i32In);
        }
        else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AWB_GET_GAIN:
        if (puParaOut != NULL)
            err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBGetAWBGain(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AWB_SET_GAIN:
        if (u4ParaInLen == sizeof(AWB_GAIN_T))
            err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBSetAWBGain(mSensorDev, (VOID *)puParaIn);
        else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_FLASH_GET_MODE:
        if (puParaOut != NULL) {
            MY_LOGD("ACDK_CCT_OP_FLASH_GET_INFO line=%d\n",__LINE__);
#if (CAM3_3A_ISP_50_EN)
            ret = FlashMgr::getInstance(mSensorDev)->cctGetFlashInfo((int*)puParaOut);
            if(ret!=0)
                err=CCTIF_BAD_CTRL_CODE;
#else
            ret = FlashMgr::getInstance().cctGetFlashInfo(mSensorDev, (int*)puParaOut);
            if(ret!=1)
                err=CCTIF_BAD_CTRL_CODE;
#endif
            *pu4RealParaOutLen = sizeof(MINT32);
        } else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_FLASH_SET_MODE:
        if (u4ParaInLen == sizeof(MINT32)) {
            MY_LOGD("ACDK_CCT_OP_FLASH_ENABLE line=%d\n",__LINE__);
            CCT_FL_MODE_T mode = (CCT_FL_MODE_T) (*(MUINT32 *)puParaIn);
#if (CAM3_3A_ISP_50_EN)
            if (mode == CCT_FL_MODE_OFF) {    //disable
                ret = FlashMgr::getInstance(mSensorDev)->cctFlashEnable(0);
            }
            else if (mode == CCT_FL_MODE_HI_TEMP) {   //enable ... high color temp
                ret = FlashMgr::getInstance(mSensorDev)->cctFlashEnable(1);
                ret = FlashMgr::getInstance(mSensorDev)->clearManualFlash();
                ret = FlashMgr::getInstance(mSensorDev)->setManualFlash(1, 0);
            }
            else if (mode == CCT_FL_MODE_LO_TEMP) {   //enable ... low color temp
                ret = FlashMgr::getInstance(mSensorDev)->cctFlashEnable(1);
                ret = FlashMgr::getInstance(mSensorDev)->clearManualFlash();
                ret = FlashMgr::getInstance(mSensorDev)->setManualFlash(0, 1);
            }
            else if (mode == CCT_FL_MODE_MIX_TEMP) {   //enable ... mix both color temp
                ret = FlashMgr::getInstance(mSensorDev)->cctFlashEnable(1);
                ret = FlashMgr::getInstance(mSensorDev)->clearManualFlash();
                ret = FlashMgr::getInstance(mSensorDev)->setManualFlash(1, 1);
            }
            else
                ret = 0;

            if(ret!=0)
                err=CCTIF_BAD_CTRL_CODE;
#else
            if (mode == CCT_FL_MODE_OFF) {    //disable
                ret = FlashMgr::getInstance().cctFlashEnable(mSensorDev, 0);
            }
            else if (mode == CCT_FL_MODE_HI_TEMP) {   //enable ... high color temp
                ret = FlashMgr::getInstance().cctFlashEnable(mSensorDev, 1);
                ret = FlashMgr::getInstance().clearManualFlash(mSensorDev);
                ret = FlashMgr::getInstance().setManualFlash(mSensorDev, 1, 0);
            }
            else if (mode == CCT_FL_MODE_LO_TEMP) {   //enable ... low color temp
                ret = FlashMgr::getInstance().cctFlashEnable(mSensorDev, 1);
                ret = FlashMgr::getInstance().clearManualFlash(mSensorDev);
                ret = FlashMgr::getInstance().setManualFlash(mSensorDev, 0, 1);
            }
            else if (mode == CCT_FL_MODE_MIX_TEMP) {   //enable ... mix both color temp
                ret = FlashMgr::getInstance().cctFlashEnable(mSensorDev, 1);
                ret = FlashMgr::getInstance().clearManualFlash(mSensorDev);
                ret = FlashMgr::getInstance().setManualFlash(mSensorDev, 1, 1);
            }
            else
                ret = 0;

            if(ret!=1)
                err=CCTIF_BAD_CTRL_CODE;
#endif
        } else {
            err = CCTIF_BAD_PARAM;
        }
        *pu4RealParaOutLen = 0;
        break;
    default:
        MY_LOGD("Not support cmd %d", (int)op);
        break;

    }
    return err;
}


MINT32
CctHandle::
cct_HandleIspOp(CCT_OP_ID op,
                MUINT32 u4ParaInLen,
                MUINT8 *puParaIn,
                MUINT32 u4ParaOutLen,
                MUINT8 *puParaOut,
                MUINT32 *pu4RealParaOutLen )
{
    MBOOL ret;
    MINT32 err = CCTIF_NO_ERROR;

    switch(op) {
    case FT_CCT_OP_GET_ID:
        {
        if ( ISP_ID_TEXT_LENGTH != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut ) {
            err = CCTIF_BAD_PARAM;
        }
        else {
            *pu4RealParaOutLen = 0;
            memset((void *)puParaOut, 0, ISP_ID_TEXT_LENGTH );
            strncpy((char *)puParaOut, gb_MT6763_ISP_ID_TEXT, ISP_ID_TEXT_LENGTH);
            *pu4RealParaOutLen = ISP_ID_TEXT_LENGTH;
            MY_LOGD("( FT_CCT_OP_GET_ID ) done, ID:%s\n", (char *)(puParaOut));
        }
        }
        break;
    case FT_CCT_OP_ISP_GET_ON_OFF:
        {
        if  ( sizeof(CCT_ISP_CATEGORY_T) != u4ParaInLen || ! puParaIn ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        if  ( sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        CCT_ISP_CATEGORY_T const eCategory = *reinterpret_cast<CCT_ISP_CATEGORY_T*>(puParaIn);
        MBOOL&       rfgEnable = reinterpret_cast<ACDK_CCT_FUNCTION_ENABLE_STRUCT*>(puParaOut)->Enable;

        err = getIspOnOff(eCategory, rfgEnable);
        *pu4RealParaOutLen = sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT);
        MY_LOGD("[-FT_CCT_OP_ISP_GET_ON_OFF] (eCategory, rfgEnable)=(%d, %d)", eCategory, rfgEnable);
        }
        break;
    case FT_CCT_OP_ISP_SET_ON_OFF:
        {
        MY_LOGD("[-FT_CCT_OP_ISP_SET_ON_OFF] size=%d, %d, %d, %p", sizeof(CCT_ISP_CATEGORY_T), sizeof(MBOOL), u4ParaInLen, puParaIn);
        if  ( (sizeof(CCT_ISP_CATEGORY_T) + sizeof(MBOOL)) != u4ParaInLen || ! puParaIn ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        CCT_ISP_CATEGORY_T const eCategory = *reinterpret_cast<CCT_ISP_CATEGORY_T const*>(puParaIn);
        MBOOL enable = (MBOOL) *(puParaIn + 4);

        if (enable)
            err = setIspOnOff(eCategory, 1);
        else
            err = setIspOnOff(eCategory, 0);

        MY_LOGD("[-FT_CCT_OP_ISP_SET_ON_OFF] eCategory(%d), err(%x)", eCategory, err);
        }
        break;
    case FT_CCT_OP_ISP_GET_CCM_FIXED_ON_OFF:
        {
        MINT32 en = 0, dyccm = 0;

        MY_LOGD("[FT_CCT_OP_ISP_GET_CCM_FIXED_ON_OFF]\n");
        if  ( sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
            return  CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;

        dyccm = NSIspTuningv3::IspTuningMgr::getInstance().getDynamicCCM((MINT32)m_eSensorEnum);
        if(dyccm < 0) {
            MY_LOGD("( FT_CCT_OP_ISP_GET_CCM_FIXED_ON_OFF ) fail, dyccm=%d\n", dyccm);
            err = CCTIF_UNKNOWN_ERROR;
            break;
        }

        en = (MFALSE == dyccm)? MTRUE:MFALSE; //switch true/false
        reinterpret_cast<ACDK_CCT_FUNCTION_ENABLE_STRUCT*>(puParaOut)->Enable = en;
        *pu4RealParaOutLen = sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT);
        err = CCTIF_NO_ERROR;

        MY_LOGD("( FT_CCT_OP_ISP_GET_CCM_FIXED_ON_OFF ) done, en=%d\n", en);
        }
        break;
    case FT_CCT_OP_ISP_SET_CCM_FIXED_ON_OFF:
        {
        if(puParaIn == NULL)
        {
            err = CCTIF_BAD_PARAM;
            break;
        }
        MBOOL enable = (MBOOL) *(puParaIn);
        MY_LOGD("CCM ON/OFF, DYNAMIC_BYPASS_MODE =%d", enable);

        if (enable) {
            ret = NSIspTuningv3::IspTuningMgr::getInstance().setDynamicCCM((MINT32)m_eSensorEnum, MFALSE);
            ret &= NSIspTuningv3::IspTuningMgr::getInstance().setDynamicBypass((MINT32)m_eSensorEnum, MTRUE);
        }
        else {
            ret = NSIspTuningv3::IspTuningMgr::getInstance().setDynamicCCM((MINT32)m_eSensorEnum, MTRUE);
            ret &= NSIspTuningv3::IspTuningMgr::getInstance().setDynamicBypass((MINT32)m_eSensorEnum, MFALSE);
        }

        *pu4RealParaOutLen = 0;
        if (ret == MTRUE)
            err = CCTIF_NO_ERROR;
        else
            err = CCTIF_UNKNOWN_ERROR;
        }
        break;
    case FT_CCT_OP_ISP_GET_CCM_MATRIX:
        {
        if  ( sizeof(ACDK_CCT_CCM_STRUCT) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        ACDK_CCT_CCM_STRUCT*const     pDst = reinterpret_cast<ACDK_CCT_CCM_STRUCT*>(puParaOut);
        ISP_NVRAM_CCM_T ccm;

        ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
        ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).get(ccm);

        pDst->M11 = ccm.cnv_1.bits.G2G_CNV_00;//ccm.conv0a.bits.G2G_CNV_00;
        pDst->M12 = ccm.cnv_1.bits.G2G_CNV_01;//ccm.conv0a.bits.G2G_CNV_01;
        pDst->M13 = ccm.cnv_2.bits.G2G_CNV_02;//ccm.conv0b.bits.G2G_CNV_02;
        pDst->M21 = ccm.cnv_3.bits.G2G_CNV_10;//ccm.conv1a.bits.G2G_CNV_10;
        pDst->M22 = ccm.cnv_3.bits.G2G_CNV_11;//ccm.conv1a.bits.G2G_CNV_11;
        pDst->M23 = ccm.cnv_4.bits.G2G_CNV_12;//ccm.conv1b.bits.G2G_CNV_12;
        pDst->M31 = ccm.cnv_5.bits.G2G_CNV_20;//ccm.conv2a.bits.G2G_CNV_20;
        pDst->M32 = ccm.cnv_5.bits.G2G_CNV_21;//ccm.conv2a.bits.G2G_CNV_21;
        pDst->M33 = ccm.cnv_6.bits.G2G_CNV_22;//ccm.conv2b.bits.G2G_CNV_22;

        *pu4RealParaOutLen = sizeof(ACDK_CCT_CCM_STRUCT);

        MY_LOGD("[ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM]\n");
        MY_LOGD("M11 0x%03X\n", pDst->M11);
        MY_LOGD("M12 0x%03X\n", pDst->M12);
        MY_LOGD("M13 0x%03X\n", pDst->M13);
        MY_LOGD("M21 0x%03X\n", pDst->M21);
        MY_LOGD("M22 0x%03X\n", pDst->M22);
        MY_LOGD("M23 0x%03X\n", pDst->M23);
        MY_LOGD("M31 0x%03X\n", pDst->M31);
        MY_LOGD("M32 0x%03X\n", pDst->M32);
        MY_LOGD("M33 0x%03X\n", pDst->M33);

        MY_LOGD("( FT_CCT_OP_ISP_GET_CCM_MATRIX ) done\n");
        }
        break;
    case FT_CCT_OP_ISP_SET_CCM_MATRIX:
        {
        if  ( sizeof(ACDK_CCT_CCM_STRUCT) != u4ParaInLen || ! puParaIn ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        MY_LOGD("[ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM]\n");

        ISP_NVRAM_CCM_T rDst;
        ACDK_CCT_CCM_STRUCT*const       pSrc = reinterpret_cast<ACDK_CCT_CCM_STRUCT*>(puParaIn);
    /*
        rDst.conv0a.bits.G2G_CNV_00 = pSrc->M11;
        rDst.conv0a.bits.G2G_CNV_01 = pSrc->M12;
        rDst.conv0b.bits.G2G_CNV_02 = pSrc->M13;
        rDst.conv1a.bits.G2G_CNV_10 = pSrc->M21;
        rDst.conv1a.bits.G2G_CNV_11 = pSrc->M22;
        rDst.conv1b.bits.G2G_CNV_12 = pSrc->M23;
        rDst.conv2a.bits.G2G_CNV_20 = pSrc->M31;
        rDst.conv2a.bits.G2G_CNV_21 = pSrc->M32;
        rDst.conv2b.bits.G2G_CNV_22 = pSrc->M33;
    */
        rDst.cnv_1.bits.G2G_CNV_00 = pSrc->M11;
        rDst.cnv_1.bits.G2G_CNV_01 = pSrc->M12;
        rDst.cnv_2.bits.G2G_CNV_02 = pSrc->M13;
        rDst.cnv_3.bits.G2G_CNV_10 = pSrc->M21;
        rDst.cnv_3.bits.G2G_CNV_11 = pSrc->M22;
        rDst.cnv_4.bits.G2G_CNV_12 = pSrc->M23;
        rDst.cnv_5.bits.G2G_CNV_20 = pSrc->M31;
        rDst.cnv_5.bits.G2G_CNV_21 = pSrc->M32;
        rDst.cnv_6.bits.G2G_CNV_22 = pSrc->M33;

        ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).reset();
        ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).put(rDst);
        NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);
        //ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(EIspProfile_Preview);
        //NSIspTuningv3::ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).apply(NSIspTuningv3::EIspProfile_NormalCapture);
        usleep(200000);
        MY_LOGD("M11 0x%03X", pSrc->M11);
        MY_LOGD("M12 0x%03X", pSrc->M12);
        MY_LOGD("M13 0x%03X", pSrc->M13);
        MY_LOGD("M21 0x%03X", pSrc->M21);
        MY_LOGD("M22 0x%03X", pSrc->M22);
        MY_LOGD("M23 0x%03X", pSrc->M23);
        MY_LOGD("M31 0x%03X", pSrc->M31);
        MY_LOGD("M32 0x%03X", pSrc->M32);
        MY_LOGD("M33 0x%03X", pSrc->M33);

        MY_LOGD("( FT_CCT_OP_ISP_SET_CCM_MATRIX ) done\n");
        }
        break;
    case FT_CCT_OP_ISP_GET_INDEX:
        {
        if ( sizeof(CUSTOM_NVRAM_REG_INDEX) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        if  ( sizeof(ACDK_CCT_QUERY_ISP_INDEX_INPUT_STRUCT) != u4ParaInLen || ! puParaIn ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        ACDK_CCT_QUERY_ISP_INDEX_INPUT_STRUCT *pIspIndexInput = reinterpret_cast<ACDK_CCT_QUERY_ISP_INDEX_INPUT_STRUCT*>(puParaIn);
        MVOID* pIndex = IspTuningMgr::getInstance().getDefaultISPIndex(m_eSensorEnum, pIspIndexInput->profile, pIspIndexInput->sensorMode, pIspIndexInput->iso_idx);

        memcpy(puParaOut, pIndex, sizeof(CUSTOM_NVRAM_REG_INDEX));
        *pu4RealParaOutLen = sizeof(CUSTOM_NVRAM_REG_INDEX);

        MY_LOGD("FT_CCT_OP_ISP_GET_INDEX done\n");
        }
        break;
    case FT_CCT_OP_ISP_GET_SMOOTH_CCM:
        {
        typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
        if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
            return  CCTIF_BAD_PARAM;

        reinterpret_cast<o_type*>(puParaOut)->Enable = m_rBuf_ISP.ISPColorTbl.ISPMulitCCM.CCM_Method;
        *pu4RealParaOutLen = sizeof(o_type);
        MY_LOGD("IMP_CCT_CTRL( ACDK_CCT_V2_OP_CCM_GET_SMOOTH_SWITCH ) done, %d\n", m_rBuf_ISP.ISPColorTbl.ISPMulitCCM.CCM_Method);
        }
        break;
    case FT_CCT_OP_ISP_SET_SMOOTH_CCM:
        {
        *pu4RealParaOutLen = 0;

        typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT i_type;
        if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
            return  CCTIF_BAD_PARAM;

        m_rBuf_ISP.ISPColorTbl.ISPMulitCCM.CCM_Method = reinterpret_cast<i_type*>(puParaIn)->Enable ? 1 : 0;
        MY_LOGD("IMP_CCT_CTRL( ACDK_CCT_V2_OP_CCM_SET_SMOOTH_SWITCH ) done, %d\n", m_rBuf_ISP.ISPColorTbl.ISPMulitCCM.CCM_Method);
        return  CCTIF_NO_ERROR;
        }
        break;

    case FT_CCT_OP_GET_SHADING_ON_OFF:
#if (CAM3_3A_ISP_40_EN)
        {
        if ( sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        ACDK_CCT_FUNCTION_ENABLE_STRUCT*const pShadingPara = reinterpret_cast<ACDK_CCT_FUNCTION_ENABLE_STRUCT*>(puParaOut);

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        pShadingPara->Enable = pLscMgr->getOnOff();
        *pu4RealParaOutLen = sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT);
        MY_LOGD("[%s] GET_SHADING_ON_OFF(%s)", __FUNCTION__, (pShadingPara->Enable ? "On":"Off"));
        }
        break;
#else
        {
        if ( sizeof(ACDK_CCT_MODULE_CTRL_STRUCT) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        ACDK_CCT_MODULE_CTRL_STRUCT*const pShadingPara = reinterpret_cast<ACDK_CCT_MODULE_CTRL_STRUCT*>(puParaOut);

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        pShadingPara->Enable = pLscMgr->getOnOff();
        *pu4RealParaOutLen = sizeof(ACDK_CCT_MODULE_CTRL_STRUCT);
        MY_LOG("[%s] GET_SHADING_ON_OFF(%s)", __FUNCTION__, (pShadingPara->Enable ? "On":"Off"));
        }
        break;
#endif
    case FT_CCT_OP_SET_SHADING_ON_OFF:
#if (CAM3_3A_ISP_40_EN)
        {
        if ( sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT) != u4ParaInLen || ! puParaIn ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        ACDK_CCT_FUNCTION_ENABLE_STRUCT*const pShadingPara = reinterpret_cast<ACDK_CCT_FUNCTION_ENABLE_STRUCT*>(puParaIn);

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        pLscMgr->setOnOff(pShadingPara->Enable);
        pLscMgr->updateLsc();

        MY_LOGD("[%s] SET_SHADING_ON_OFF(%s)", __FUNCTION__, (pShadingPara->Enable ? "On":"Off"));
        }
        break;
#else
        {
        if ( sizeof(ACDK_CCT_MODULE_CTRL_STRUCT) != u4ParaInLen || ! puParaIn ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        ACDK_CCT_MODULE_CTRL_STRUCT*const pShadingPara = reinterpret_cast<ACDK_CCT_MODULE_CTRL_STRUCT*>(puParaIn);

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        pLscMgr->setOnOff(pShadingPara->Enable);
        pLscMgr->updateLsc();

        MY_LOG("[%s] SET_SHADING_ON_OFF(%s)", __FUNCTION__, (pShadingPara->Enable ? "On":"Off"));
        }
        break;
#endif

    case FT_CCT_OP_GET_SHADING_INDEX:
        {
        if  ( ! puParaOut ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        MUINT32 *pShadingIndex = reinterpret_cast<MUINT32*>(puParaOut);

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        *pShadingIndex = pLscMgr->getCTIdx();

        *pu4RealParaOutLen = sizeof(MUINT32);
        MY_LOGD("[%s] GET_SHADING_INDEX(%d)", __FUNCTION__, *pShadingIndex);

        }
        break;
    case FT_CCT_OP_SET_SHADING_INDEX:
        {
        if ( ! puParaIn ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        MUINT32 u4CCT = *reinterpret_cast<MUINT8*>(puParaIn);

        MY_LOGD("[%s] SET_SHADING_INDEX(%d)", __FUNCTION__, u4CCT);

        IspTuningMgr::getInstance().enableDynamicShading(m_eSensorEnum, MFALSE);
        IspTuningMgr::getInstance().setIndex_Shading(m_eSensorEnum, u4CCT);
        }
        break;
    case FT_CCT_OP_GET_SHADING_TSF_ON_OFF:
        {
        if  ( ! puParaOut ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        UINT32 u4OnOff;
        *pu4RealParaOutLen = 0;
        MY_LOG("[%s] + GET_SHADING_TSF_ONOFF", __FUNCTION__);

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        u4OnOff = pLscMgr->getTsfOnOff();
        *(reinterpret_cast<UINT32*>(puParaOut)) = u4OnOff;

        MY_LOGD("[%s] - GET_SHADING_TSF_ONOFF(%d)", __FUNCTION__, u4OnOff);
        }
        break;
    case FT_CCT_OP_SET_SHADING_TSF_ON_OFF:
        {
        if(puParaIn == NULL)
        {
            err = CCTIF_BAD_PARAM;
            break;
        }
        UINT32 u4OnOff = *(reinterpret_cast<UINT32*>(puParaIn));
        *pu4RealParaOutLen = 0;
        MY_LOGD("[%s] + SET_SHADING_TSF_ONOFF(%d)", __FUNCTION__, u4OnOff);

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        pLscMgr->setTsfOnOff(u4OnOff);

        MY_LOGD("[%s] - SET_SHADING_TSF_ONOFF", __FUNCTION__);
        }
        break;
    default:
        MY_LOGD("Not support cmd %d", (int)op);
        break;
    }
    return err;
}


MINT32
CctHandle::
cct_HandleNvramOp(CCT_OP_ID op,
                    MUINT32 u4ParaInLen,
                    MUINT8 *puParaIn,
                    MUINT32 u4ParaOutLen,
                    MUINT8 *puParaOut,
                    MUINT32 *pu4RealParaOutLen )
{
    CCT_NVRAM_DATA_T dtype;
    MINT32 inSize, outSize;
    MINT32 *pRealOutSize = NULL;
    MUINT32 inOffset;
    MUINT8 *pInBuf, *pOutBuf;
    MINT32 status = 0;

    dtype = *((CCT_NVRAM_DATA_T *) puParaIn);

    switch(op) {
    case FT_CCT_OP_ISP_GET_NVRAM_DATA:
        if  ( ! puParaOut ) {
            break;
        }
        //cctNvram_GetNvramData(CCT_NVRAM_DATA_T dataType, MINT32 outSize, void *pOutBuf, MINT32 *pRealOutSize)
        inSize = u4ParaInLen - sizeof(CCT_NVRAM_DATA_T);
        pInBuf = (puParaIn + sizeof(CCT_NVRAM_DATA_T));
        outSize = u4ParaOutLen;
        pOutBuf = puParaOut;
        pRealOutSize = (MINT32 *) pu4RealParaOutLen;
        status = cctNvram_GetNvramData(dtype, inSize, pInBuf, outSize, (void *)pOutBuf, pRealOutSize);
        break;
    case FT_CCT_OP_ISP_SET_NVRAM_DATA:
        //cctNvram_SetNvramData(CCT_NVRAM_DATA_T dataType, MINT32 inSize, MUINT8 *pInBuf)
        inSize = u4ParaInLen - sizeof(CCT_NVRAM_DATA_T);
        pInBuf = (puParaIn + sizeof(CCT_NVRAM_DATA_T));
        status = cctNvram_SetNvramData(dtype, inSize, pInBuf);
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_ISP_SET_PARTIAL_NVRAM_DATA:
        // For this command FT_CCT_OP_ISP_SET_PARTIAL_NVRAM_DATA, there are 2 parameters in the beginning of
        // puParaIn buffer. The 1st parameter is dtype (data type) and the 2nd is inOffset (in buffer offset)
        //
        inOffset = *(((CCT_NVRAM_DATA_T *) puParaIn)+1);
        inSize = u4ParaInLen - sizeof(CCT_NVRAM_DATA_T) - sizeof(MUINT32);
        pInBuf = (puParaIn + sizeof(CCT_NVRAM_DATA_T) + sizeof(MUINT32));
        status = cctNvram_SetPartialNvramData(dtype, inSize, inOffset, pInBuf);
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_ISP_SAVE_NVRAM_DATA:
        //cctNvram_SaveNvramData(CCT_NVRAM_DATA_T dataType)
        status = cctNvram_SaveNvramData(dtype);
        *pu4RealParaOutLen = 0;
        break;
    default:
        MY_LOGD("Not support cmd %d", (int)op);
        break;

    }
    return status;
}


MINT32
CctHandle::
cct_HandleEmcamOp(CCT_OP_ID op,
                    MUINT32 u4ParaInLen,
                    MUINT8 *puParaIn,
                    MUINT32 u4ParaOutLen,
                    MUINT8 *puParaOut,
                    MUINT32 *pu4RealParaOutLen )
{
    MY_LOGD("[%s] op(%x), u4ParaInLen(%d), puParaIn(%p), (*puParaIn)(%d), u4ParaOutLen(%d), puParaOut(%p), pu4RealParaOutLen(%p)", __FUNCTION__, \
      op, u4ParaInLen, puParaIn, (MINT32 *)puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);

    int ret;
    MINT32 err = CCTIF_NO_ERROR;
    MINT32 *i32In = NULL;
    MUINT32 u4SubCmdRealOutLen = 0;
    MBOOL enable;
    AE_MODE_CFG_T ae_cfg;
    ACDK_AE_MODE_CFG_T acdk_ae_cfg;
    // lambda function for saving result to file
    auto saveResult = [this](android::String8 sResult) -> MBOOL
    {
        FILE *fp = NULL;
        if ((fp = fopen(this->mEmResultPath.string(), "wb")) != NULL)
        {

            fwrite(sResult.string(), sResult.length(), 1, fp);
            fclose(fp);
            //MY_LOGD("fwrite to %s, %s",this->mEmResultPath.string(),sResult.string());
            return MTRUE;
        }
        else
        {
            MY_LOGE("fopen %s fail!",this->mEmResultPath.string());
            return MFALSE;
        }
    };
    //
    android::String8 sResult = android::String8::format("Result=Fail\n");
    saveResult(sResult);
    //
    switch(op) {

    case FT_CCT_OP_AF_BRECKET_STEP:
        *pu4RealParaOutLen = 0;
        MY_LOGD("FT_CCT_OP_AF_BRECKET_STEP");
        if (u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            FOCUS_RANGE_T af_range;
            MINT32 currentPos = 0;
            err = NS3Av3::IAfMgr::getInstance().CCTOPAFGetFocusRange(mSensorDev, (VOID *)&af_range, &u4SubCmdRealOutLen);
            if( err != S_AF_OK || u4SubCmdRealOutLen != sizeof(FOCUS_RANGE_T) ){
                MY_LOGE("[%s] CCTOPAFGetFocusRange failed", __FUNCTION__);
                break;
            }

            ACDK_AF_INFO_T af_info;
            err = NS3Av3::IAfMgr::getInstance().CCTOPAFGetAFInfo(mSensorDev, &af_info, &u4SubCmdRealOutLen);
            if( err != S_AF_OK || u4SubCmdRealOutLen != sizeof(ACDK_AF_INFO_T) ){
                MY_LOGE("[%s] CCTOPAFGetAFInfo failed", __FUNCTION__);
                break;
            }
            currentPos = af_info.i4CurrPos;
            //
            MINT32 manualPos;
            i32In = (MINT32 *)puParaIn;
            manualPos = currentPos + *i32In;
            MY_LOGD("AF breacket set manual af POS: %d=%d+%d, manual range = (%d, %d)", manualPos, currentPos, *i32In, af_range.i4InfPos, af_range.i4MacroPos);
            if(manualPos > af_range.i4InfPos && manualPos < af_range.i4MacroPos){
                MY_LOGD("manualPos:(%d) is in the manual range, move to the manual pos",manualPos);
                err = NS3Av3::IAfMgr::getInstance().CCTOPMFOpeartion(mSensorDev, manualPos);
                if( err != S_AF_OK ){
                    MY_LOGE("[%s] CCTOPMFOpeartion failed", __FUNCTION__);
                    break;
                }
            } else {
                MY_LOGE("manualPos:(%d) is out of the manual range!!! It will do AF and move to best pos",manualPos);
                err = NS3Av3::IAfMgr::getInstance().CCTOPAFOpeartion(mSensorDev);
                if( err != S_AF_OK ){
                    MY_LOGE("[%s] CCTOPMFOpeartion failed", __FUNCTION__);
                    break;
                }
            }
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("[%s] CCTIF_BAD_PARAM", __FUNCTION__);
            err = CCTIF_BAD_PARAM;
        }
        break;
    case FT_CCT_OP_AE_BRECKET_STEP:
        *pu4RealParaOutLen = 0;
        MY_LOGD("FT_CCT_OP_AE_BRECKET_STEP");
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            //get realISO
            MINT64 isoBase = 100;
            FrameOutputParam_T RTParams;
            err = IAeMgr::getInstance().getRTParams(mSensorDev, *reinterpret_cast<FrameOutputParam_T*>(&RTParams));
            if( err != S_AE_OK )
            {
                MY_LOGE("[%s] getRTParams fail", __FUNCTION__);
            }
            else
            {
                isoBase = (MINT64)RTParams.u4RealISOValue* 1024 / (MINT64)RTParams.u4PreviewSensorGain_x1024* 1024 / (MINT64)RTParams.u4PreviewISPGain_x1024;
                MY_LOGD("setManualAEControl: isoBase(%lld),u4RealISOValue(%ld),u4PreviewSensorGain_x1024(%ld),u4PreviewISPGain_x1024(%ld)",
                        isoBase, RTParams.u4RealISOValue, RTParams.u4PreviewSensorGain_x1024, RTParams.u4PreviewISPGain_x1024);
            }
            //get current EV value
            strAEOutput aeOutput;
            i32In = (MINT32 *) puParaIn;
            IAeMgr::getInstance().switchCapureDiffEVState(mSensorDev, (MINT8) (*i32In/10), aeOutput);
            MY_LOGD("setAeDiffEvValue : %d ", ((*i32In)/10));
            //set EVValue
            //AE off
            {
                MUINT32 aeMode = MTK_CONTROL_AE_MODE_OFF;
                IAeMgr::getInstance().setAEMode(mSensorDev, aeMode);
                MY_LOGD("set MTK_CONTROL_AE_MODE (OFF)");
            }
            AE_SENSOR_PARAM_T strSensorParams;
            strSensorParams.u4Sensitivity   = ( MUINT32 )( ( MINT64 ) aeOutput.EvSetting.u4AfeGain*aeOutput.EvSetting.u4IspGain*isoBase/1024/1024);
            strSensorParams.u8ExposureTime  = ( ( MINT64 ) aeOutput.EvSetting.u4Eposuretime)*1000;
            //strSensorParams.u8FrameDuration = rNewParam.i8FrameDuration;
            MY_LOGD("set MTK_SENSOR_SENSITIVITY (%d)", strSensorParams.u4Sensitivity);
            MY_LOGD("set MTK_SENSOR_EXPOSURE_TIME (%d) ", strSensorParams.u8ExposureTime);
            IAeMgr::getInstance().UpdateSensorParams(mSensorDev, strSensorParams);
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("[%s] CCTIF_BAD_PARAM", __FUNCTION__);
            err = CCTIF_BAD_PARAM;
        }
        break;
    case FT_CCT_OP_AE_BRECKET_STEP_HIGH_PERSION_EV:
        *pu4RealParaOutLen = 0;
         MY_LOGD("FT_CCT_OP_AE_BRECKET_STEP_HIGH_PERSION_EV not support!!!");
        break;

    case FT_CCT_OP_AF_SET_AFMODE:
        *pu4RealParaOutLen = 0;
        MY_LOGD("FT_CCT_OP_AF_SET_AFMODE");
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            MY_LOGD("setAFMode(%d)",*i32In);
            err = NS3Av3::IAfMgr::getInstance().setAFMode(mSensorDev, *i32In);
            if( err != S_AF_OK ){
                MY_LOGE("[%s] setAFMode failed", __FUNCTION__);
                break;
            }
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("[%s] CCTIF_BAD_PARAM", __FUNCTION__);
            err = CCTIF_BAD_PARAM;
        }
        break;
    case FT_CCT_OP_AE_SET_AE_MODE:
        *pu4RealParaOutLen = 0;
        MY_LOGD("FT_CCT_OP_AE_SET_AE_MODE");
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            MY_LOGD("setAEMode(%d)",*i32In);
            err = NS3Av3::IAeMgr::getInstance().setAEMode(mSensorDev, *i32In);
            if( err != S_AF_OK ){
                MY_LOGE("[%s] setAEMode failed", __FUNCTION__);
                break;
            }
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
            err = CCTIF_BAD_PARAM;
        break;
    case FT_CCT_OP_AF_AUTOFOCUS:
        *pu4RealParaOutLen = 0;
        MY_LOGD("FT_CCT_OP_AF_AUTOFOCUS");
        err = NS3Av3::IAfMgr::getInstance().CCTOPAFOpeartion(mSensorDev);
        if( err != S_AF_OK ){
            MY_LOGE("[%s] CCTOPMFOpeartion failed", __FUNCTION__);
            break;
        }
        //
        sResult = android::String8::format("Result=OK\n");
        saveResult(sResult);
        break;
    case FT_CCT_OP_AF_FULL_SCAN_SET_INTERVAL:
        *pu4RealParaOutLen = 0;
        MY_LOGD("FT_CCT_OP_AF_FULL_SCAN_SET_INTERVAL");
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            MY_LOGD("setFullScanInterval(%d)",*i32In);
            mEmFullScanInterval = *i32In;
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
            err = CCTIF_BAD_PARAM;
        break;
    case FT_CCT_OP_AF_FULL_SCAN_SET_DACSTEP:
        *pu4RealParaOutLen = 0;
        MY_LOGD("FT_CCT_OP_AF_FULL_SCAN_SET_DACSTEP");
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            MY_LOGD("setFullScanDacStep(%d)",*i32In);
            mEmFullScanDacStep = *i32In;
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
            err = CCTIF_BAD_PARAM;
        break;
    case FT_CCT_OP_AF_FULL_SCAN_TRIGGER: {
        *pu4RealParaOutLen = 0;
        MY_LOGD("FT_CCT_OP_AF_FULL_SCAN_TRIGGER");
        MINT32 i4FullScanStep = ((mEmFullScanInterval & 0xFFFF) << 16) + ((mEmFullScanDacStep) & 0xFFFF);
        MY_LOGD("setFullScanstep(%d)",i4FullScanStep);
        err = NS3Av3::IAfMgr::getInstance().setFullScanstep(mSensorDev, i4FullScanStep);
        err = NS3Av3::IAfMgr::getInstance().CCTOPAFOpeartion(mSensorDev);
        usleep(200*1000);   //sleep 100ms (need wait at least 2 frame)
        while(!(NS3Av3::IAfMgr::getInstance().CCTOPCheckAutoFocusDone(mSensorDev)))
        {
            usleep(500*1000);   //sleep 500ms
            MY_LOGD("still wait AutoFocusDone");
        }
        MY_LOGD("AutoFocusDone");
        if( err != S_AF_OK ){
            MY_LOGE("[%s] setFullScanstep failed", __FUNCTION__);
            break;
        }
        //
        sResult = android::String8::format("Result=OK\n");
        saveResult(sResult);
        //
        break;
    }
    case FT_CCT_OP_AF_SET_AREA:
        MY_LOGD("FT_CCT_OP_AF_SET_AREA");
        *pu4RealParaOutLen = 0;
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            if(*i32In <= 0 || *i32In > 100)
            {
                MY_LOGE("set CCTOPAFSetAfArea(%d), the value is invalid (must between 1~100)",*i32In);
            }
            else
            {
                err = NS3Av3::IAfMgr::getInstance().CCTOPAFSetAfArea(mSensorDev, *i32In);
                MY_LOGD("set CCTOPAFSetAfArea(%d)",*i32In);
                //
                sResult = android::String8::format("Result=OK\n");
                saveResult(sResult);
            }
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        break;
    case FT_CCT_OP_AE_GET_ON_OFF:
        if (puParaOut != NULL)
            err = NS3Av3::IAeMgr::getInstance().CCTOPAEGetEnableInfo(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AE_SET_ON_OFF:
        if (u4ParaInLen == sizeof(MBOOL)) {
            enable = (MBOOL) *puParaIn;
            if (enable) {
                MY_LOGD("set normal mode");
                m_pIHal3A->send3ACtrl(E3ACtrl_SetOperMode, NSIspTuning::EOperMode_Normal, 0);
                err = NS3Av3::IAeMgr::getInstance().CCTOPAEEnable(mSensorDev);
            }
            else {
                MY_LOGD("set meta mode");
                m_pIHal3A->send3ACtrl(E3ACtrl_SetOperMode, NSIspTuning::EOperMode_Meta, 0);
                err = NS3Av3::IAeMgr::getInstance().CCTOPAEDisable(mSensorDev);
            }
        } else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_FLASH_CALIBRATION:
        MY_LOGD("FT_CCT_OP_FLASH_CALIBRATION");
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            if(*i32In>=1)
            {
                MY_LOGD("i32In(%d),cctSetSpModeQuickCalibration2",*i32In);
#if (CAM3_3A_ISP_50_EN)
                err = FlashMgr::getInstance(mSensorDev)->cctSetSpModeQuickCalibration2();
#else
                err = FlashMgr::getInstance().cctSetSpModeQuickCalibration2(mSensorDev);
#endif
            }
            else
            {
                MY_LOGD("i32In(%d),cctSetSpModeNormal",*i32In);
#if (CAM3_3A_ISP_50_EN)
                err = FlashMgr::getInstance(mSensorDev)->cctSetSpModeNormal();
#else
                err = FlashMgr::getInstance().cctSetSpModeNormal(mSensorDev);
#endif
            }
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_SET_CAPTURE_ISO:
        MY_LOGD("FT_CCT_OP_AE_SET_CAPTURE_ISO");
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            mEmCaptureIso = *i32In;
            mEmCaptureGainMode = 1;
            MY_LOGD("set mEmCaptureIso(%d) mEmCaptureGainMode(%d)",mEmCaptureIso,mEmCaptureGainMode);
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_SET_CAPTURE_SENSOR_GAIN:
        MY_LOGD("FT_CCT_OP_AE_SET_CAPTURE_SENSOR_GAIN");
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            mEmCaptureSensorGain = *i32In;
            mEmCaptureGainMode = 0;
            MY_LOGD("set mEmCaptureSensorGain(%d) mEmCaptureGainMode(%d)",mEmCaptureSensorGain,mEmCaptureGainMode);
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_SET_CAPTURE_EXP_TIME_US:
        MY_LOGD("FT_CCT_OP_AE_SET_CAPTURE_EXP_TIME_US");
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            mEmCaptureExpTimeUs = *i32In;
            MY_LOGD("set mEmCaptureExpTimeUs(%d)",mEmCaptureExpTimeUs);
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_APPLY_CAPTURE_AE_PARAM:
        MY_LOGD("FT_CCT_OP_AE_APPLY_CAPTURE_AE_PARAM");
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            //AE off
            {
                MUINT32 aeMode = MTK_CONTROL_AE_MODE_OFF;
                IAeMgr::getInstance().setAEMode(mSensorDev, aeMode);
                MY_LOGD("set MTK_CONTROL_AE_MODE (OFF)");
            }
            //
            AE_SENSOR_PARAM_T strSensorParams;
            err = NS3Av3::IAeMgr::getInstance().getPreviewParams(mSensorDev, ae_cfg);
            //
            acdk_ae_cfg.u4ExposureMode = ae_cfg.u4ExposureMode;
            acdk_ae_cfg.u4Eposuretime = ae_cfg.u4Eposuretime;
            acdk_ae_cfg.u4GainMode = mEmCaptureGainMode;
            acdk_ae_cfg.u4AfeGain = ae_cfg.u4AfeGain;
            acdk_ae_cfg.u4IspGain = ae_cfg.u4IspGain;
            acdk_ae_cfg.u4ISO = ae_cfg.u4RealISO;
            acdk_ae_cfg.u2FrameRate = ae_cfg.u2FrameRate;
            acdk_ae_cfg.u2CaptureFlareGain = ae_cfg.i2FlareGain;
            acdk_ae_cfg.u2CaptureFlareValue = ae_cfg.i2FlareOffset;
            //
            strSensorParams.u4Sensitivity   = acdk_ae_cfg.u4ISO;
            strSensorParams.u8ExposureTime  = ((MINT64)acdk_ae_cfg.u4Eposuretime)*1000;
            //
            if(mEmCaptureGainMode == 1) //iso
            {
                if(mEmCaptureIso == 0)
                {
                    MY_LOGD("mEmCaptureGainMode == 1 but mEmCaptureIso == 0, no need to update capture iso");
                }
                else
                {
                    acdk_ae_cfg.u4GainMode = 1;
                    acdk_ae_cfg.u4ISO = mEmCaptureIso;
                    strSensorParams.u4Sensitivity   = acdk_ae_cfg.u4ISO;
                    MY_LOGD("apply u4GainMode(%d) u4ISO(%d)",acdk_ae_cfg.u4GainMode,acdk_ae_cfg.u4ISO);
                }
            }
            else    //sensor gain
            {
                if(mEmCaptureSensorGain == 0)
                {
                    MY_LOGD("mEmCaptureGainMode == 0 but mEmCaptureSensorGain == 0, no need to update capture sensor gain");
                }
                else
                {
                    acdk_ae_cfg.u4GainMode = 0;
                    acdk_ae_cfg.u4AfeGain = mEmCaptureSensorGain;
                    //get realISO
                    MINT64 isoBase = 100;
                    FrameOutputParam_T RTParams;
                    err = IAeMgr::getInstance().getRTParams(mSensorDev, *reinterpret_cast<FrameOutputParam_T*>(&RTParams));
                    if( err != S_AE_OK )
                    {
                        MY_LOGE("[%s] getRTParams fail", __FUNCTION__);
                    }
                    else
                    {
                        isoBase = (MINT64)RTParams.u4RealISOValue* 1024 / (MINT64)RTParams.u4PreviewSensorGain_x1024* 1024 / (MINT64)RTParams.u4PreviewISPGain_x1024;
                        MY_LOGD("isoBase(%lld),u4RealISOValue(%ld),u4PreviewSensorGain_x1024(%ld),u4PreviewISPGain_x1024(%ld)",
                                isoBase, RTParams.u4RealISOValue, RTParams.u4PreviewSensorGain_x1024, RTParams.u4PreviewISPGain_x1024);
                    }
                    strSensorParams.u4Sensitivity   = ( MUINT32 )( ( MINT64 ) acdk_ae_cfg.u4AfeGain*acdk_ae_cfg.u4IspGain*isoBase/1024/1024); //ISP6: must /4096, ISP5 & older: must / 1024
                    MY_LOGD("u4Sensitivity(%d) = u4AfeGain(%d)*u4IspGain(%d)*isoBase(%d)/1024/4096",strSensorParams.u4Sensitivity,acdk_ae_cfg.u4AfeGain,acdk_ae_cfg.u4IspGain,isoBase);
                    MY_LOGD("apply u4GainMode(%d) u4AfeGain(%d)",acdk_ae_cfg.u4GainMode,acdk_ae_cfg.u4AfeGain);
                }
            }
            //
            if(mEmCaptureExpTimeUs == 0)
            {
                MY_LOGD("mEmCaptureExpTimeUs == 0 , no need to update capture exp time");
            }
            else
            {
                acdk_ae_cfg.u4Eposuretime = mEmCaptureExpTimeUs;
                strSensorParams.u8ExposureTime  = ((MINT64)acdk_ae_cfg.u4Eposuretime)*1000;
                MY_LOGD("apply u4Eposuretime(%d)",acdk_ae_cfg.u4Eposuretime);
            }
            //
            err = NS3Av3::IAeMgr::getInstance().CCTOSetCaptureParams(mSensorDev, (VOID *)(&acdk_ae_cfg));
            //
            MY_LOGD("set AE_SENSOR_PARAM_T:u4Sensitivity (%d)", strSensorParams.u4Sensitivity);
            MY_LOGD("set AE_SENSOR_PARAM_T:u8ExposureTime (%d) ", strSensorParams.u8ExposureTime);
            IAeMgr::getInstance().UpdateSensorParams(mSensorDev, strSensorParams);
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_SET_VHDR_RATIO:
        MY_LOGD("FT_CCT_OP_AE_SET_VHDR_RATIO");
        *pu4RealParaOutLen = 0;
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            IAeMgr::getInstance().setEMVHDRratio(mSensorDev, (MUINT32)(*i32In));
            MY_LOGD("set setEMVHDRratio(%d)",*i32In);
            //
            //AE mode off
            {
                MUINT32 aeMode = MTK_CONTROL_AE_MODE_OFF;
                IAeMgr::getInstance().setAEMode(mSensorDev, aeMode);
                MY_LOGD("set MTK_CONTROL_AE_MODE (OFF)");
            }
            //UpdateSensorParams to apply hdr ratio
            AE_SENSOR_PARAM_T strSensorParams;
            IAeMgr::getInstance().getSensorParams(mSensorDev, strSensorParams);
            IAeMgr::getInstance().UpdateSensorParams(mSensorDev, strSensorParams);
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        break;
    case FT_CCT_OP_AWB_SET_MTK_ENABLE:
        MY_LOGD("FT_CCT_OP_AWB_SET_MTK_ENABLE");
        *pu4RealParaOutLen = 0;
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            m_pIHal3A->send3ACtrl(E3ACtrl_SetAwbBypCalibration, *i32In, 0);
            MY_LOGD("send3ACtrl(E3ACtrl_SetAwbBypCalibration , %d, 0)",*i32In);
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        break;
    case FT_CCT_OP_AWB_SET_SENSOR_ENABLE:
        MY_LOGD("FT_CCT_OP_AWB_SET_SENSOR_ENABLE");
        *pu4RealParaOutLen = 0;
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            IHalSensor *pHalSensorObj;
            // Sensor hal init
            IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
            CAM_ULOG_ASSERT(Utils::ULog::MOD_ISP_MGR, pIHalSensorList != nullptr, "[%s]: make HAL sensor list fail!", __FUNCTION__);
            pHalSensorObj = pIHalSensorList->createSensor("cct_sensor_access", 0);

            if(pHalSensorObj == NULL) {
                MY_LOGE("[AAA Sensor Mgr] Can not create SensorHal obj\n");
                err = CCTIF_UNKNOWN_ERROR;
            }
            else
            {
                MINT32 errSensor = SENSOR_NO_ERROR;
                errSensor = pHalSensorObj->sendCommand(pIHalSensorList->querySensorDevIdx(mSensorDev),
                                                            SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD,(MUINTPTR)i32In,0,0);
                MY_LOGD("pHalSensorObj->sendCommand(SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD , %d, 0)",*i32In);

                if (errSensor != SENSOR_NO_ERROR) {
                    MY_LOGE("pHalSensorObj->sendCommand(SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD), err=%d", errSensor);
                    err = CCTIF_UNKNOWN_ERROR;
                }
            }
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        break;
    case FT_CCT_OP_AE_EV_CALIBRATION:
        MY_LOGD("FT_CCT_OP_AE_EV_CALIBRATION");
        *pu4RealParaOutLen = 0;
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            i32In = (MINT32 *)puParaIn;
            MINT32 iInputValue = *i32In;
            MINT32 iAECurrentEV = 0;
            MINT32 i4BVOffset = 0;
            MUINT32 iOutLen = 0;
            MINT32 iResult = 0;
            m_pIHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetCurrentEV, reinterpret_cast<MINTPTR>(&iAECurrentEV), reinterpret_cast<MINTPTR>(&iOutLen));
            m_pIHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetBVOffset, reinterpret_cast<MINTPTR>(&i4BVOffset), reinterpret_cast<MINTPTR>(&iOutLen));
            iResult = (iAECurrentEV + i4BVOffset) - iInputValue + 50;
            MY_LOGD("iResult(%d) = (iAECurrentEV(%d) + i4BVOffset(%d)) - iInputValue(%d) + 50",iResult, iAECurrentEV, i4BVOffset, iInputValue);
            //
            sResult = android::String8::format("Result=%d\n",iResult);
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        break;
    case FT_CCT_OP_SET_RESULT_FILE_PATH:
        MY_LOGD("FT_CCT_OP_SET_RESULT_FILE_PATH");
        *pu4RealParaOutLen = 0;
        char cmd[1024];
        memset(cmd,0,1024);
        MINT32 cmdlen;
        cmdlen = u4ParaInLen;
        strncpy(cmd, (char *)puParaIn, cmdlen);
        //
        mEmResultPath = String8(cmd);
        MY_LOGD("mEmResultPath = %s, length(%d), u4ParaInLen(%d)",mEmResultPath.string(),mEmResultPath.length(),u4ParaInLen);
        //
        sResult = android::String8::format("Result=OK\n");
        saveResult(sResult);
        //
        break;
    case FT_CCT_OP_AE_LOCK_EXPOSURE:
        MY_LOGD("FT_CCT_OP_AE_LOCK_EXPOSURE");
        *pu4RealParaOutLen = 0;
        if(u4ParaInLen == sizeof(MINT32) && puParaIn != NULL)
        {
            enable = (MBOOL) *puParaIn;
            if (enable)
            {
                MY_LOGD("enable(%d) : CCTOPAELockExpSetting",enable);
                err = NS3Av3::IAeMgr::getInstance().CCTOPAELockExpSetting(mSensorDev);
            }
            else
            {
                MY_LOGD("enable(%d) : CCTOPAEUnLockExpSetting",enable);
                err = NS3Av3::IAeMgr::getInstance().CCTOPAEUnLockExpSetting(mSensorDev);
            }
            //
            sResult = android::String8::format("Result=OK\n");
            saveResult(sResult);
        }
        else
        {
            MY_LOGE("CCTIF_BAD_PARAM u4ParaInLen(%d),puParaIn(%p)",u4ParaInLen,puParaIn);
            err = CCTIF_BAD_PARAM;
        }
        break;
    case FT_CCT_OP_UPDATE_CAPTURE_PARAMS:
        MY_LOGD("FT_CCT_OP_UPDATE_CAPTURE_PARAMS");
        *pu4RealParaOutLen = 0;
        err = NS3Av3::IAeMgr::getInstance().CCTOGetCaptureParams(mSensorDev, (VOID *)(&acdk_ae_cfg));
        MY_LOGD("CCTOSetCaptureParams: u4Eposuretime(%d) u4ISO(%d)",acdk_ae_cfg.u4Eposuretime,acdk_ae_cfg.u4ISO);
        err = NS3Av3::IAeMgr::getInstance().CCTOSetCaptureParams(mSensorDev, (VOID *)(&acdk_ae_cfg));
        err = CCTIF_NO_ERROR;
        break;
    default:
        MY_LOGD("Not support cmd %d", (int)op);
        break;

    }
    return err;
}



MUINT32 CctHandle::SetAFMode(MBOOL isAutoMode)
{
    MY_LOGD("isAutoMode(%d)\n", isAutoMode);
    // set parameter
#if 0
    DefaultKeyedVector<String8,String8> map;
    if (mDevice->ops->get_parameters) {
        char *temp = mDevice->ops->get_parameters(mDevice);
        String8 str_parms(temp);
        if (mDevice->ops->put_parameters)
            mDevice->ops->put_parameters(mDevice, temp);
        else
            free(temp);
        unflatten(str_parms, map);
    }

    if(isAutoMode)
    {
        set(map, CameraParameters::KEY_FOCUS_MODE, CameraParameters::FOCUS_MODE_AUTO);
    }
    else
    {
        set(map, CameraParameters::KEY_FOCUS_MODE, CameraParameters::FOCUS_MODE_INFINITY);
    }

    mDevice->ops->set_parameters(mDevice, flatten(map).string());
#endif
    return MTRUE;
}

MBOOL CCTIF_FeatureCtrl(MUINT32 a_u4Ioctl, MUINT8 *puParaIn, MUINT32 u4ParaInLen, MUINT8 *puParaOut, MUINT32 u4ParaOutLen, MUINT32 *pu4RealParaOutLen)
{
return 1;
}
MBOOL bTrueFlag = MTRUE;
MBOOL bFalseFlag = MFALSE;


MRESULT CctHandle::UnpackRAWImage()
{
    MRESULT result = S_CCT_CALIBRATION_OK;
    FILE *packFile = NULL;
    MUINT8 *t_packBuf = NULL;
    CctImgTool mImgToolObj;

    MY_LOGD("+");

    // src check
    if( packFileSize <= 0 )
    {
        MY_LOGE("no file to unpack");
        result = E_CCT_CALIBRATION_FILE_OPEN_FAIL;
        goto Exit;
    }

    // dst buffer
    if( unpackBufSize != (rawWidth * rawHeight * 2 * sizeof(MUINT8)) )
    {
        MY_LOGD("unpack buffer resize: %d => %d", unpackBufSize, (rawWidth * rawHeight * 2 * sizeof(MUINT8)));

        if( rawUnpackBuf )
            free(rawUnpackBuf);

        unpackBufSize = rawWidth * rawHeight * 2 * sizeof(MUINT8);
        rawUnpackBuf = (MUINT8 *) malloc (unpackBufSize);
        if( rawUnpackBuf == NULL )
        {
            MY_LOGE("rawUnpackBuf memory allocate failed");
            result = E_CCT_CALIBRATION_MEMORY_MAX;
            goto Exit;
        }
    }


    // src buffer & unpacking
    if( !(packFile = fopen(rawFilePath, "rb")) )
    {
        MY_LOGE("packFile fopen fail");
        result = E_CCT_CALIBRATION_FILE_OPEN_FAIL;
        goto Exit;
    }

    // special case: already unpacked pure raw (.raw file)
    if( packFileSize == unpackBufSize )
    {
        fread(rawUnpackBuf, 1, unpackBufSize, packFile);
    }
    else
    {
        if( !(t_packBuf = (MUINT8 *) malloc (packFileSize)) )
        {
            MY_LOGE("t_packBuf memory allocate failed");
            result = E_CCT_CALIBRATION_MEMORY_MAX;
            fclose(packFile);
            goto Exit;
        }
        fread(t_packBuf, 1, packFileSize, packFile);

        // unpacking
        mImgToolObj.vUnPackRawImg(t_packBuf, rawUnpackBuf, packFileSize, rawWidth, rawHeight, rawBitDepth);

        free(t_packBuf);
    }

    // close
    fclose(packFile);

    MY_LOGD("pass");
Exit:

    MY_LOGD("-");
    return result;
}

MRESULT CctHandle::AnalyzeRAWImage(ACDK_CDVT_RAW_ANALYSIS_RESULT_T &RawAnlysRslt)
{
    //RawAnlysRslt.fRAvg  = 0;
    //RawAnlysRslt.fGrAvg = 0;
    //RawAnlysRslt.fGbAvg = 0;
    //RawAnlysRslt.fBAvg  = 0;

    MRESULT result = S_CCT_CALIBRATION_OK;
    ROIRect rROI;
    CctImgTool mImgToolObj;

    // unpack raw
    result = UnpackRAWImage();
    if( result != S_CCT_CALIBRATION_OK )
    {
        MY_LOGE("Unpack Failed!");
        goto Exit;
    }

    // analysis: ROI Parameters Should be Even
    rROI.u4ROIWidth = (rawWidth / 5) &(~(UINT32)0x01);
    rROI.u4ROIHeight = (rawHeight / 5) &(~(UINT32)0x01) ;
    rROI.u4StartX = ((rawWidth - rROI.u4ROIWidth) >> 1) & (~(UINT32)0x01);
    rROI.u4StartY = ((rawHeight - rROI.u4ROIHeight) >> 1) & (~(UINT32)0x01);

    result = mImgToolObj.mrAnalyzeRAWImage(rawUnpackBuf,
                                          rawWidth,
                                          rawHeight,
                                          (eRAW_ColorOrder)rawColorOrder,
                                          rROI,
                                          RawAnlysRslt);
    if (result != S_CCT_IMGTOOL_OK)
    {
        MY_LOGE(" mImgToolObj.mrAnalyzeRAWImage() error=%d", result);
        result = E_CCT_CALIBRATION_API_FAIL;
    }
    else
    {
        result = S_CCT_CALIBRATION_OK;
    }

Exit:
    return result;
}


typedef enum {
     CT_FORMAT_PURE = 1
    ,CT_FORMAT_PROC
    ,CT_FORMAT_JPEG
    //,CT_FORMAT_YV12
} CT_OUT_FORMAT_T;
typedef enum {
     CT_PROFILE_MODE_PREVIEW = 1
    ,CT_PROFILE_MODE_CAPTURE
    ,CT_PROFILE_MODE_VIDEO
} CT_PROFILE_MODE_T;
struct CCT_CAPTURE_CALLBACK
{
    int out_format;
    int profile_mode;
    int dump_time;
};

MRESULT CctHandle::TakePicture(MUINT32 a_i4SensorMode, MUINT32 a_i4OutFormat, MINT32 expTime = -1)
{
    MY_LOGD("TakePicture: sensorMode=%d, outFormat=%d, expTime=%d", a_i4SensorMode, a_i4OutFormat, expTime);
    MRESULT result = S_CCT_CALIBRATION_OK;
    int ret = 0;

    MY_LOGD("TakePicture: capturCallBack start");
    CCT_CAPTURE_CALLBACK cb_param;
    cb_param.out_format = (int)a_i4OutFormat;
    cb_param.profile_mode = (int)a_i4SensorMode;
    cb_param.dump_time = expTime;
    usleep(expTime/5);

    capturCallBack((void*)&cb_param, sizeof(CCT_CAPTURE_CALLBACK));
    MY_LOGD("TakePicture: capturCallBack done");

    MY_LOGD("Check Raw File" );
    char rawFilename[512];
    memset(rawFilename, 0, 512);

    rawWidth = 0;
    rawHeight = 0;
    rawBitDepth = 0;
    rawColorOrder = 0;

    memset(rawFilePath, 0, 1024);
    packFileSize = 0;

    char raw_prefix[10] = "";
    if ( CT_FORMAT_PURE == a_i4OutFormat ) {
        strncpy(raw_prefix, "pure__", 9);
    }
    else if ( CT_FORMAT_PROC == a_i4OutFormat ) {
        strncpy(raw_prefix, "proc__", 9);
    }

    // 1. Get rawFilename
    MY_LOGD("Get rawFilename" );
    DIR *dir;
    if ((dir=opendir("/mnt/vendor/cct")))
    {
        struct dirent *ptr;

        while ((ptr=readdir(dir)) != NULL)
        {
            if( ptr->d_name == strstr(ptr->d_name, raw_prefix) )
            {
                MY_LOGD("rawFilename=%s", ptr->d_name);

                strncpy(rawFilename, ptr->d_name, 511);
                rawFilename[511] = 0;
                break;
            }
        }
        closedir(dir);
    }
    else
    {
        MY_LOGE("Failed to open dir: /mnt/vendor/cct" );
        result = E_CCT_CALIBRATION_API_FAIL;
        goto CaptureFail;
    }

    //2. Get Raw Width/Height/BitDepth/ColorOrder
    MY_LOGD("Get Raw Width/Height/BitDepth/ColorOrder" );

    if(rawFilename[0])
    {
        auto getValue = [] (int &value, char *&_p) -> void
        {
            value = 0;
            int c;
            int digFlag = 0;
            do
            {
                c = *_p++;
                if( c >= '0' && c <= '9' )
                {
                    value = 10*value + (c - '0');
                    digFlag = 1;
                }
                else if( digFlag )
                {
                    break;
                }
            } while (*_p);
        };

        char *t_fname = rawFilename;
        getValue(rawWidth, t_fname);
        getValue(rawHeight, t_fname);
        getValue(rawBitDepth, t_fname);
        getValue(rawColorOrder, t_fname);
    }
    else
    {
        MY_LOGE("Failed to Get Raw Width/Height/BitDepth/ColorOrder" );
        result = E_CCT_CALIBRATION_API_FAIL;
        goto CaptureFail;
    }

    // 3. Get rawFilePath/packFileSize
    MY_LOGD("Get rawFilePath/packFileSize" );

    if(rawFilename[0])
    {
        sprintf(rawFilePath, "%s/%s", "/mnt/vendor/cct", rawFilename);
        FILE *rawFile = fopen(rawFilePath, "rb");
        if(rawFile)
        {
            fseek(rawFile, 0, SEEK_END);
            packFileSize = ftell(rawFile);
            fclose(rawFile);
        }
        else
        {
            MY_LOGE("File open failed: %s", rawFilePath );
            result = E_CCT_CALIBRATION_FILE_OPEN_FAIL;
            goto CaptureFail;
        }
    }
    else
    {
        MY_LOGE("Failed to Get rawFilePath/packFileSize" );
        result = E_CCT_CALIBRATION_API_FAIL;
        goto CaptureFail;
    }

    return result;

CaptureFail:
    memset(rawFilePath, 0, 1024);
    packFileSize = 0;
    rawWidth = 0;
    rawHeight = 0;
    rawBitDepth = 0;
    rawColorOrder = 0;

    return result;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrCalculateSlope () -
//!  brief calculate slope
//!
/////////////////////////////////////////////////////////////////////////
MRESULT CctHandle::CalculateSlope(DOUBLE a_dX0,
                                          DOUBLE a_dY0,
                                          DOUBLE a_dX1,
                                          DOUBLE a_dY1,
                                          DOUBLE a_dX2,
                                          DOUBLE a_dY2,
                                          DOUBLE a_dX3,
                                          DOUBLE a_dY3,
                                          DOUBLE &a_dSlope)
{
    DOUBLE dTemp0 = 4 * (a_dX0 * a_dY0 + a_dX1 * a_dY1 + a_dX2 * a_dY2 + a_dX3 * a_dY3);
    DOUBLE dTemp1 = (a_dX0 + a_dX1 + a_dX2 + a_dX3) * (a_dY0 + a_dY1 + a_dY2 + a_dY3);
    DOUBLE dTemp2 = 4 * (a_dX0 * a_dX0 + a_dX1 * a_dX1 + a_dX2 * a_dX2 + a_dX3 * a_dX3);
    DOUBLE dTemp3 = (a_dX0 + a_dX1 + a_dX2 + a_dX3) * (a_dX0 + a_dX1 + a_dX2 + a_dX3);

    a_dSlope = (dTemp0 - dTemp1) / (dTemp2 - dTemp3);

    return S_CCT_CALIBRATION_OK;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrExpLinearity () -
//!  @brief exposure time linearity test
//!  @param a_i4Gain: snsor gain (base = 1024)
//!  @param a_i4ExpMode: [0] exposure line [1] exposure time in us
//!  @param a_i4ExpStart: the starting exposure line or exposure time
//!  @param a_i4ExpEnd: the ending exposure line or exposure time
//!  @param a_i4ExpInterval: The exposure interval in exposure line or exposure time
//!  @param a_i4SensorMode: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT CctHandle::mrExpLinearity(MINT32 a_i4Gain,
                                  MINT32 a_i4ExpMode,
                                  MINT32 a_i4ExpStart,
                                  MINT32 a_i4ExpEnd,
                                  MINT32 a_i4ExpInterval,
                                  MINT32 a_i4SensorMode,
                                  ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput)
{
    MY_LOGD("+");
    MY_LOGD("a_i4Gain = %d", a_i4Gain);
    MY_LOGD("a_i4ExpMode = %d", a_i4ExpMode);
    MY_LOGD("a_i4ExpStart = %d", a_i4ExpStart);
    MY_LOGD("a_i4ExpEnd = %d", a_i4ExpEnd);
    MY_LOGD("a_i4ExpInterval = %d", a_i4ExpInterval);
    MY_LOGD("a_i4SensorMode = %d", a_i4SensorMode);

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;
    AE_SENSOR_PARAM_T sensor_cfg;
    MUINT32 restoreAeMode = 0, off_AeMode = MTK_CONTROL_AE_MODE_OFF;
    MUINT32 minISOGain = 0;

    a_prSensorTestOutput->i4ErrorCode = 0; // No error

    MY_LOGD("Disable 3A");
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_AEMODE, 0, NULL, sizeof(MUINT32), (MUINT8*)&restoreAeMode, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&off_AeMode, 0, NULL, &u4RetLen);

        //m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        MUINT32 pAENVRAMLen = sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM;
        char* pAENVRAMBuffer = (char*)malloc(pAENVRAMLen);
        memset(pAENVRAMBuffer,0, pAENVRAMLen);
        NS3Av3::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(mSensorDev, (MVOID *)pAENVRAMBuffer, &pAENVRAMLen);
        AE_NVRAM_T (*pAENVRAMArr)[CAM_SCENARIO_NUM] = (AE_NVRAM_T(*)[CAM_SCENARIO_NUM])pAENVRAMBuffer;
        minISOGain = pAENVRAMArr[0]->rDevicesInfo.u4MiniISOGain;
        free(pAENVRAMBuffer);

        MY_LOGD("minISOGain = %d", minISOGain);
    }

    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_OP_AF_DISABLE, NULL, 0, NULL, 0, &u4RetLen);
    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4SensorMode, sizeof(a_i4SensorMode), NULL, 0, &u4RetLen);

    for (MINT32 i4Exp = a_i4ExpStart; i4Exp <= a_i4ExpEnd; i4Exp += a_i4ExpInterval)
    {
        if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
            cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
            //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

            ae_cfg.u4ExposureMode = a_i4ExpMode;
            ae_cfg.u4Eposuretime = i4Exp;
            ae_cfg.u4AfeGain = a_i4Gain;
            //ae_cfg.u4IspGain = a_i4Gain;

            MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
            cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        }else{
            cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
            MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

            sensor_cfg.u8ExposureTime = (MINT64)i4Exp * 1000;
            sensor_cfg.u4Sensitivity = (a_i4Gain * minISOGain) / 1024;
            //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

            MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
            cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        }

        system_sleep(100);

        /*
        *   Take Picture
        */
        if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PURE, (5 * i4Exp) )) != S_CCT_CALIBRATION_OK)
        {
            MY_LOGE("TakePicture Fail");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        ACDK_CDVT_RAW_ANALYSIS_RESULT_T rawAnalysisResult;
        if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
            MY_LOGE("AnalyzeRAWImage Fail");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        a_prSensorTestOutput->rRAWAnalysisResult[i4SequenceNo] = rawAnalysisResult;

        MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
                rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);
        i4SequenceNo++;
    }

    a_prSensorTestOutput->i4TestCount = i4SequenceNo;

Exit:
    if(a_i4ExpMode) {
        MY_LOGD("Restore Exp Mode to Time base");
        cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        ae_cfg.u4ExposureMode = 0;
        cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
    }

#if USE_DYNAMIC_BUF
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }
#endif

    MY_LOGD("Restore 3A");
    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&restoreAeMode, 0, NULL, &u4RetLen);
    }
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);

    MY_LOGD("-");

    return mrRet;
}


/////////////////////////////////////////////////////////////////////////
//
//   mrGainLinearity () -
//!  @brief sensor gain linearity test
//!  @param a_i4ExpTime: exposure time in us
//!  @param a_i4GainStart: the starting senaor gain (base = 1024)
//!  @param a_i4GainEnd: the ending senaor gain (base = 1024)
//!  @param a_i4GainInterval: The test senaor gain interval (base = 1024)
//!  @param a_i4SensorMode: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT CctHandle::mrGainLinearity(MINT32 a_i4ExpTime,
                                         MINT32 a_i4GainStart,
                                         MINT32 a_i4GainEnd,
                                         MINT32 a_i4GainInterval,
                                         MINT32 a_i4SensorMode,
                                         ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput)
{

    MY_LOGD("+");
    MY_LOGD("a_i4ExpTime = %d", a_i4ExpTime);
    MY_LOGD("a_i4GainStart = %d", a_i4GainStart);
    MY_LOGD("a_i4GainEnd = %d", a_i4GainEnd);
    MY_LOGD("a_i4GainInterval = %d", a_i4GainInterval);
    MY_LOGD("a_i4SensorMode = %d", a_i4SensorMode);

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;
    AE_SENSOR_PARAM_T sensor_cfg;
    MUINT32 restoreAeMode = 0, off_AeMode = MTK_CONTROL_AE_MODE_OFF;
    MUINT32 minISOGain = 0;


    a_prSensorTestOutput->i4ErrorCode = 0; // No error


    MY_LOGD("Disable 3A");
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);

    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_AEMODE, 0, NULL, sizeof(MUINT32), (MUINT8*)&restoreAeMode, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&off_AeMode, 0, NULL, &u4RetLen);

        //m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        MUINT32 pAENVRAMLen = sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM;
        char* pAENVRAMBuffer = (char*)malloc(pAENVRAMLen);
        memset(pAENVRAMBuffer,0, pAENVRAMLen);
        NS3Av3::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(mSensorDev, (MVOID *)pAENVRAMBuffer, &pAENVRAMLen);
        AE_NVRAM_T (*pAENVRAMArr)[CAM_SCENARIO_NUM] = (AE_NVRAM_T(*)[CAM_SCENARIO_NUM])pAENVRAMBuffer;
        minISOGain = pAENVRAMArr[0]->rDevicesInfo.u4MiniISOGain;
        free(pAENVRAMBuffer);
        MY_LOGD("minISOGain = %d", minISOGain);
    }

    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4SensorMode, sizeof(a_i4SensorMode), NULL, 0, &u4RetLen);

    for (MINT32 i4Gain = a_i4GainStart; i4Gain <= a_i4GainEnd; i4Gain += a_i4GainInterval)
    {
        if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
            cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
            //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

            ae_cfg.u4Eposuretime = a_i4ExpTime;
            ae_cfg.u4AfeGain = i4Gain;
            //ae_cfg.u4IspGain = i4Gain;

            MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
            cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        }else{
            cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
            MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

            sensor_cfg.u8ExposureTime = (MINT64)a_i4ExpTime * 1000;
            sensor_cfg.u4Sensitivity = (i4Gain * minISOGain) / 1024;
            //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

            MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
            cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        }

        system_sleep(100);

        /*
        *   Take Picture
        */
        if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PURE, (5 * a_i4ExpTime) )) != S_CCT_CALIBRATION_OK)
        {
            MY_LOGE("TakePicture Fail");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        ACDK_CDVT_RAW_ANALYSIS_RESULT_T rawAnalysisResult;
        if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
            MY_LOGE("AnalyzeRAWImage Fail");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        a_prSensorTestOutput->rRAWAnalysisResult[i4SequenceNo] = rawAnalysisResult;

        MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
                rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);

        i4SequenceNo++;
    }

    a_prSensorTestOutput->i4TestCount = i4SequenceNo;

Exit:

#if USE_DYNAMIC_BUF
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }
#endif

    MY_LOGD("Restore 3A");
    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&restoreAeMode, 0, NULL, &u4RetLen);
    }
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);

    MY_LOGD("-");

    return mrRet;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrGainTableLinearity () -
//!  @brief sensor gain linearity test (gain table)
//!  @param a_i4ExpTime: exposure time in us
//!  @param a_i4GainTableSize: gain table dimension
//!  @param a_pi4GainTable: pointer to gain table
//!  @param a_i4SensorMode: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT CctHandle::mrGainTableLinearity(MINT32 a_i4ExpTime,
                                              MINT32 a_i4GainTableSize,
                                              MINT32 *a_pi4GainTable,
                                              MINT32 a_i4SensorMode,
                                              ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput)
{

    MY_LOGD("+");
    MY_LOGD("a_i4ExpTime = %d", a_i4ExpTime);
    MY_LOGD("a_i4GainTableSize = %d", a_i4GainTableSize);

    for (MINT32 i = 0; i < a_i4GainTableSize; i++)
    {
        MY_LOGD("GAIN_TABLE [%d] = %d", i, a_pi4GainTable[i]);
    }
    MY_LOGD("a_i4SensorMode = %d\n", a_i4SensorMode);
    MY_LOGD("=====================\n");

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;
    AE_SENSOR_PARAM_T sensor_cfg;
    MUINT32 restoreAeMode = 0, off_AeMode = MTK_CONTROL_AE_MODE_OFF;
    MUINT32 minISOGain = 0;


    a_prSensorTestOutput->i4ErrorCode = 0; // No error


    MY_LOGD("Disable 3A");
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);

    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_AEMODE, 0, NULL, sizeof(MUINT32), (MUINT8*)&restoreAeMode, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&off_AeMode, 0, NULL, &u4RetLen);

        //m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        MUINT32 pAENVRAMLen = sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM;
        char* pAENVRAMBuffer = (char*)malloc(pAENVRAMLen);
        memset(pAENVRAMBuffer,0, pAENVRAMLen);
        NS3Av3::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(mSensorDev, (MVOID *)pAENVRAMBuffer, &pAENVRAMLen);
        AE_NVRAM_T (*pAENVRAMArr)[CAM_SCENARIO_NUM] = (AE_NVRAM_T(*)[CAM_SCENARIO_NUM])pAENVRAMBuffer;
        minISOGain = pAENVRAMArr[0]->rDevicesInfo.u4MiniISOGain;
        free(pAENVRAMBuffer);
        MY_LOGD("minISOGain = %d", minISOGain);
    }


    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4SensorMode, sizeof(a_i4SensorMode), NULL, 0, &u4RetLen);

    for (MINT32 i4GainTableIndex = 0; i4GainTableIndex < a_i4GainTableSize; i4GainTableIndex ++)
    {
        if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
            cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
            //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

            ae_cfg.u4Eposuretime = a_i4ExpTime;
            ae_cfg.u4AfeGain = a_pi4GainTable[i4GainTableIndex];
            //ae_cfg.u4IspGain = a_pi4GainTable[i4GainTableIndex];

            MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
            cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        }else{
            cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
            MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

            sensor_cfg.u8ExposureTime = (MINT64)a_i4ExpTime * 1000;
            sensor_cfg.u4Sensitivity = (a_pi4GainTable[i4GainTableIndex] * minISOGain) / 1024;
            //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

            MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
            cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        }



        system_sleep(100);



        /*
        *   Take Picture
        */
        if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PURE, (5 * a_i4ExpTime) )) != S_CCT_CALIBRATION_OK)
        {
            MY_LOGE("TakePicture Fail \n");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        ACDK_CDVT_RAW_ANALYSIS_RESULT_T rawAnalysisResult;
        if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
            MY_LOGE("AnalyzeRAWImage Fail");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        a_prSensorTestOutput->rRAWAnalysisResult[i4SequenceNo] = rawAnalysisResult;

        MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
                rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);
        i4SequenceNo++;
    }

    a_prSensorTestOutput->i4TestCount = i4SequenceNo;


Exit:

#if USE_DYNAMIC_BUF
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }
#endif

    MY_LOGD("Restore 3A");
    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&restoreAeMode, 0, NULL, &u4RetLen);
    }

    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);

    MY_LOGD("-");

    return mrRet;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrOBStability () -
//!  @brief OB stability test
//!  @param a_i4ExpTime: exposure time in us
//!  @param a_i4GainStart: the starting senaor gain (base = 1024)
//!  @param a_i4GainEnd: the ending senaor gain (base = 1024)
//!  @param a_i4GainInterval: The test senaor gain interval (base = 1024)
//!  @param a_i4SensorMode: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT CctHandle::mrOBStability(MINT32 a_i4ExpTime,
                                       MINT32 a_i4GainStart,
                                       MINT32 a_i4GainEnd,
                                       MINT32 a_i4GainInterval,
                                       MINT32 a_i4SensorMode,
                                       ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput)
{
    MY_LOGD("+");
    MY_LOGD("a_i4ExpTime = %d", a_i4ExpTime);
    MY_LOGD("a_i4GainStart = %d", a_i4GainStart);
    MY_LOGD("a_i4GainEnd = %d", a_i4GainEnd);
    MY_LOGD("a_i4GainInterval = %d", a_i4GainInterval);
    MY_LOGD("a_i4SensorMode = %d", a_i4SensorMode);

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 i4TestCnt, i4Gain;
    ACDK_AE_MODE_CFG_T ae_cfg;
    AE_SENSOR_PARAM_T sensor_cfg;
    MUINT32 restoreAeMode = 0, off_AeMode = MTK_CONTROL_AE_MODE_OFF;
    MUINT32 minISOGain = 0;


    a_prSensorTestOutput->i4ErrorCode = 0; // No error


    if (a_i4GainStart == a_i4GainEnd)
    {
        i4TestCnt = a_i4GainInterval;
    }
    else
    {
        i4TestCnt = ((a_i4GainEnd - a_i4GainStart) + (a_i4GainInterval - 1))/ a_i4GainInterval;

        if (((a_i4GainEnd - a_i4GainStart) % a_i4GainInterval) == 0)
        {
            i4TestCnt++;
        }
    }

    MY_LOGD("[mrOBStability] Test Count = %d", i4TestCnt);

    MY_LOGD("Disable 3A");
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);

    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_AEMODE, 0, NULL, sizeof(MUINT32), (MUINT8*)&restoreAeMode, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&off_AeMode, 0, NULL, &u4RetLen);

        //m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        MUINT32 pAENVRAMLen = sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM;
        char* pAENVRAMBuffer = (char*)malloc(pAENVRAMLen);
        memset(pAENVRAMBuffer,0, pAENVRAMLen);
        NS3Av3::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(mSensorDev, (MVOID *)pAENVRAMBuffer, &pAENVRAMLen);
        AE_NVRAM_T (*pAENVRAMArr)[CAM_SCENARIO_NUM] = (AE_NVRAM_T(*)[CAM_SCENARIO_NUM])pAENVRAMBuffer;
        minISOGain = pAENVRAMArr[0]->rDevicesInfo.u4MiniISOGain;
        free(pAENVRAMBuffer);
        MY_LOGD("minISOGain = %d", minISOGain);
    }


    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4SensorMode, sizeof(a_i4SensorMode), NULL, 0, &u4RetLen);

    for (int i = 0; i < i4TestCnt; i ++)
    {

        (a_i4GainStart == a_i4GainEnd) ? (i4Gain = a_i4GainStart)
                                       : (i4Gain = a_i4GainStart + (i * a_i4GainInterval ));
        if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
            cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
            //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

            ae_cfg.u4Eposuretime = a_i4ExpTime;
            ae_cfg.u4AfeGain = i4Gain;
            //ae_cfg.u4IspGain = i4Gain;

            MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
            cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        }else{
            cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
            MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

            sensor_cfg.u8ExposureTime = (MINT64)a_i4ExpTime * 1000;
            sensor_cfg.u4Sensitivity = (i4Gain * minISOGain) / 1024;
            //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

            MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
            cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        }

        system_sleep(100);



        /*
        *   Take Picture
        */
        if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PURE, (5 * a_i4ExpTime) )) != S_CCT_CALIBRATION_OK)
        {
            MY_LOGE("TakePicture Fail ");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        ACDK_CDVT_RAW_ANALYSIS_RESULT_T rawAnalysisResult;
        if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
            MY_LOGE("AnalyzeRAWImage Fail");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        a_prSensorTestOutput->rRAWAnalysisResult[i4SequenceNo] = rawAnalysisResult;

        MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
                rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);

        i4SequenceNo++;
    }

    a_prSensorTestOutput->i4TestCount = i4SequenceNo;


Exit:

#if USE_DYNAMIC_BUF
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }
#endif

    MY_LOGD("Restore 3A");
    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&restoreAeMode, 0, NULL, &u4RetLen);
    }

    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);

    MY_LOGD("-");

    return mrRet;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrGainTableOBStability () -
//!  @brief OB stability test (gain table)
//!  @param a_i4ExpTime: exposure time in us
//!  @param a_i4GainTableSize: gain table dimension
//!  @param a_pi4GainTable: pointer to gain table
//!  @param a_i4SensorMode: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT CctHandle::mrGainTableOBStability(MINT32 a_i4ExpTime,
                                                MINT32 a_i4GainTableSize,
                                                MINT32 *a_pi4GainTable,
                                                MINT32 a_i4SensorMode,
                                                ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput)
{

    MY_LOGD("+");
    MY_LOGD("a_i4ExpTime = %d", a_i4ExpTime);
    MY_LOGD("a_i4GainTableSize = %d", a_i4GainTableSize);
    MY_LOGD("a_i4SensorMode = %d", a_i4SensorMode);

    for (int i = 0; i < a_i4GainTableSize; i++)
    {
        MY_LOGD("GAIN_TABLE[%d] = %d", i, a_pi4GainTable[i]);
    }

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 i4TestCnt, i4Gain;
    ACDK_AE_MODE_CFG_T ae_cfg;
    AE_SENSOR_PARAM_T sensor_cfg;
    MUINT32 restoreAeMode = 0, off_AeMode = MTK_CONTROL_AE_MODE_OFF;
    MUINT32 minISOGain = 0;


    a_prSensorTestOutput->i4ErrorCode = 0; // No error


    MY_LOGD("Disable 3A");
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);

    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_AEMODE, 0, NULL, sizeof(MUINT32), (MUINT8*)&restoreAeMode, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&off_AeMode, 0, NULL, &u4RetLen);

        //m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        MUINT32 pAENVRAMLen = sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM;
        char* pAENVRAMBuffer = (char*)malloc(pAENVRAMLen);
        memset(pAENVRAMBuffer,0, pAENVRAMLen);
        NS3Av3::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(mSensorDev, (MVOID *)pAENVRAMBuffer, &pAENVRAMLen);
        AE_NVRAM_T (*pAENVRAMArr)[CAM_SCENARIO_NUM] = (AE_NVRAM_T(*)[CAM_SCENARIO_NUM])pAENVRAMBuffer;
        minISOGain = pAENVRAMArr[0]->rDevicesInfo.u4MiniISOGain;
        free(pAENVRAMBuffer);
        MY_LOGD("minISOGain = %d", minISOGain);
    }


    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4SensorMode, sizeof(a_i4SensorMode), NULL, 0, &u4RetLen);

    for (MINT32 i4GainTableIndex = 0; i4GainTableIndex < a_i4GainTableSize; i4GainTableIndex++)
    {
        if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
            cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
            //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

            ae_cfg.u4Eposuretime = a_i4ExpTime;
            ae_cfg.u4AfeGain = a_pi4GainTable[i4GainTableIndex];
            //ae_cfg.u4IspGain = a_pi4GainTable[i4GainTableIndex];

            MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
            cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        }else{
            cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
            MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

            sensor_cfg.u8ExposureTime = (MINT64)a_i4ExpTime * 1000;
            sensor_cfg.u4Sensitivity = (a_pi4GainTable[i4GainTableIndex] * minISOGain) / 1024;
            //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

            MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
            cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        }

        system_sleep(100);



        /*
        *   Take Picture
        */
        if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PURE, (5 * a_i4ExpTime) )) != S_CCT_CALIBRATION_OK)
        {
            MY_LOGE("TakePicture Fail ");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        ACDK_CDVT_RAW_ANALYSIS_RESULT_T rawAnalysisResult;
        if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
            MY_LOGE("AnalyzeRAWImage Fail");
            a_prSensorTestOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        a_prSensorTestOutput->rRAWAnalysisResult[i4SequenceNo] = rawAnalysisResult;

        MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
                rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);

        i4SequenceNo++;
    }

    a_prSensorTestOutput->i4TestCount = i4SequenceNo;


Exit:

#if USE_DYNAMIC_BUF
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }
#endif

    MY_LOGD("Restore 3A");
    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&restoreAeMode, 0, NULL, &u4RetLen);
    }

    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);

    MY_LOGD("-");

    return mrRet;
}


/////////////////////////////////////////////////////////////////////////
//
//   mrCalOB () -
//!  @brief OB calibration
//!  @param a_i4ExpTime: exposure time in us
//!  @param a_i4Gain: senaor gain (base = 1024)
//!  @param a_i4RepeatTimes: repeat times for OB calibration
//!  @param a_i4SensorMode: [0] preview [1] capture
//!  @param prSensorCalibrationOutput: pointer to sensor calibration output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT CctHandle::mrCalOB(MINT32 a_i4ExpTime,
                                 MINT32 a_i4Gain,
                                 MINT32 a_i4RepeatTimes,
                                 MINT32 a_i4SensorMode,
                                 ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput)
{
    MY_LOGD("+");
    MY_LOGD("a_i4ExpTime = %d\n", a_i4ExpTime);
    MY_LOGD("a_i4Gain = %d\n", a_i4Gain);
    MY_LOGD("a_i4RepeatTimes = %d\n", a_i4RepeatTimes);
    MY_LOGD("a_i4SensorMode = %d\n", a_i4SensorMode);

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 u4OBSum = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;
    AE_SENSOR_PARAM_T sensor_cfg;
    MUINT32 restoreAeMode = 0, off_AeMode = MTK_CONTROL_AE_MODE_OFF;
    MUINT32 minISOGain = 0;


    prSensorCalibrationOutput->i4ErrorCode = 0; // No error


    MY_LOGD("Disable 3A");
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);

    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_AEMODE, 0, NULL, sizeof(MUINT32), (MUINT8*)&restoreAeMode, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&off_AeMode, 0, NULL, &u4RetLen);

        //m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        MUINT32 pAENVRAMLen = sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM;
        char* pAENVRAMBuffer = (char*)malloc(pAENVRAMLen);
        memset(pAENVRAMBuffer,0, pAENVRAMLen);
        NS3Av3::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(mSensorDev, (MVOID *)pAENVRAMBuffer, &pAENVRAMLen);
        AE_NVRAM_T (*pAENVRAMArr)[CAM_SCENARIO_NUM] = (AE_NVRAM_T(*)[CAM_SCENARIO_NUM])pAENVRAMBuffer;
        minISOGain = pAENVRAMArr[0]->rDevicesInfo.u4MiniISOGain;
        free(pAENVRAMBuffer);
        MY_LOGD("minISOGain = %d", minISOGain);
    }


    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4SensorMode, sizeof(a_i4SensorMode), NULL, 0, &u4RetLen);

    for (MINT32 i = 0; i < a_i4RepeatTimes; i++)
    {
        if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
            cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
            //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

            ae_cfg.u4Eposuretime = a_i4ExpTime;
            ae_cfg.u4AfeGain = a_i4Gain;
            //ae_cfg.u4IspGain = a_i4Gain;

            MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
            cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        }else{
            cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
            MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

            sensor_cfg.u8ExposureTime = (MINT64)a_i4ExpTime * 1000;
            sensor_cfg.u4Sensitivity = (a_i4Gain * minISOGain) / 1024;
            //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

            MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
            cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        }

        system_sleep(100);



        /*
        *   Take Picture
        */
        if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PURE, (5 * a_i4ExpTime) )) != S_CCT_CALIBRATION_OK)
        {
            MY_LOGE("TakePicture Fail \n");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        ACDK_CDVT_RAW_ANALYSIS_RESULT_T rawAnalysisResult;
        if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
            MY_LOGE("AnalyzeRAWImage Fail");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }
        else
        {
            u4OBSum += rawAnalysisResult.u4Median;
        }

        MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
                rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);

        i4SequenceNo++;
    }

    prSensorCalibrationOutput->i4OB = (MINT32)((((DOUBLE)u4OBSum)/a_i4RepeatTimes)+0.5);

Exit:

#if USE_DYNAMIC_BUF
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }
#endif
    MY_LOGD("Restore 3A");
    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&restoreAeMode, 0, NULL, &u4RetLen);
    }

    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);

    MY_LOGD("-");

   return mrRet;

}

#define OB_SLOPEx1_UNIT         (512)
#include <math.h>


/////////////////////////////////////////////////////////////////////////
//
//   mrCalMinISO () -
//!  @brief minimum ISO calibration
//!  @param a_i4LV: LV of light source box (format: real LV x 10; ex. 100 = LV10)
//!  @param a_i4FNo: lens f/number (format: real f/number x 10; ex. 28 = f/2.8)
//!  @param a_i4OBLevel: sensor OB level
//!  @param a_i450Hz60Hz: [0] 50Hz [1] 60Hz
//!  @param a_i4SensorMode: [0] preview [1] capture
//!  @param prSensorCalibrationOutput: pointer to sensor calibration output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT CctHandle::mrCalMinISO(MINT32 a_i4LV,
                                     MINT32 a_i4FNo,
                                     MINT32 a_i4OBLevel,
                                     MINT32 a_i450Hz60Hz,
                                     MINT32 a_i4SensorMode,
                                     ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput)
{
    MY_LOGD("+");
    MY_LOGD("a_i4LV = %d", a_i4LV);
    MY_LOGD("a_i4FNo = %d", a_i4FNo);
    MY_LOGD("a_i4OBLevel = %d", a_i4OBLevel);
    MY_LOGD("a_i450Hz60Hz = %d", a_i450Hz60Hz);
    MY_LOGD("a_i4SensorMode = %d", a_i4SensorMode);

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 u4OBSum = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;
    AE_SENSOR_PARAM_T sensor_cfg;
    MUINT32 restoreAeMode = 0, off_AeMode = MTK_CONTROL_AE_MODE_OFF;
    MUINT32 minISOGain = 0;


    prSensorCalibrationOutput->i4ErrorCode = 0; // No error


    /*
    *   Disable Auto Mode for Capture
    */
    MY_LOGD("Disable 3A");
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);

    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_AEMODE, 0, NULL, sizeof(MUINT32), (MUINT8*)&restoreAeMode, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&off_AeMode, 0, NULL, &u4RetLen);

        //m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        MUINT32 pAENVRAMLen = sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM;
        char* pAENVRAMBuffer = (char*)malloc(pAENVRAMLen);
        memset(pAENVRAMBuffer,0, pAENVRAMLen);
        NS3Av3::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(mSensorDev, (MVOID *)pAENVRAMBuffer, &pAENVRAMLen);
        AE_NVRAM_T (*pAENVRAMArr)[CAM_SCENARIO_NUM] = (AE_NVRAM_T(*)[CAM_SCENARIO_NUM])pAENVRAMBuffer;
        minISOGain = pAENVRAMArr[0]->rDevicesInfo.u4MiniISOGain;
        free(pAENVRAMBuffer);
        MY_LOGD("minISOGain = %d", minISOGain);
    }


    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4SensorMode, sizeof(a_i4SensorMode), NULL, 0, &u4RetLen);


    DOUBLE dFNo; // F/Number
    DOUBLE dAV;  // AV
    DOUBLE dLV;  // LV
    DOUBLE dTV;  // TV
    DOUBLE dDefaultExpTInSec; // Default exposure time in sec
    MINT32 i4DefaultExpTInUS;  // Default exposure time in us
    DOUBLE dDefaultExpTGavg;  // G average with default exposure time
    DOUBLE dTargetGavg;       // Target G average (18% reflectance)
    DOUBLE dTargetExpTInSec;  // Target exposure time in sec
    MINT32 i4TargetExpTInUS;   // Target exposure time in us
    MINT32 i4TargetExpTInUs_UA;// Flicker-free target exposure time (upper approximation)
    MINT32 i4TargetExpTInUs_LA;// Flicker-free target exposure time (lower approximation)
    DOUBLE dTargetGavg_UA;    // G average with i4TargetExpTInUs_UA
    DOUBLE dTargetGavg_LA;    // G average with i4TargetExpTInUs_LA
    DOUBLE dMinimumISO;       // Minimum ISO
    MINT32 i4Gain = 1024; // Sensor gain = 1024
    ISP_NVRAM_OBC_T obc;
    MINT32 backup_bypass;
    ACDK_CDVT_RAW_ANALYSIS_RESULT_T rawAnalysisResult;

    /*
    *   Use OBCGain to Compensate OB Level
    */
    MUINT32 u4OBISPRAWGain = (MUINT32)((((DOUBLE)1023)/(1023-a_i4OBLevel))*OB_SLOPEx1_UNIT+0.5);

    /*
    *   Enable Dynamic Bypass
    */
    if((backup_bypass = NSIspTuningv3::IspTuningMgr::getInstance().getDynamicBypass(mSensorDev)) == -1)
    {
        MY_LOGE("[getDynamicBypass] Get Dynamic Bypass Fail ");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }
    //NSIspTuningv3::IspTuningMgr::getInstance().setDynamicBypass(mSensorDev, MTRUE);

    /*
    *   Prepare OB
    */
    /*  temp mark, using pure raw don't need the OBC setting
    obc.offst0.val = a_i4OBLevel;
    obc.offst1.val = a_i4OBLevel;
    obc.offst2.val = a_i4OBLevel;
    obc.offst3.val = a_i4OBLevel;
    obc.gain0.val = u4OBISPRAWGain;
    obc.gain1.val = u4OBISPRAWGain;
    obc.gain2.val = u4OBISPRAWGain;
    obc.gain3.val = u4OBISPRAWGain;


    NSIspTuningv3::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).setEnable(MTRUE);
    NSIspTuningv3::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).put(obc);
    NSIspTuningv3::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).apply(NSIspTuning::EIspProfile_NormalPreview);
    //Temp. Mark
    //NSIspTuningv3::IspTuningMgr::getInstance().setPureOBCInfo(&obc);
    */
    prSensorCalibrationOutput->i4ErrorCode = 0; // no error

    /*  STEP 1
    *   Calculate Flicker Free Default Exposure Time
    */
    MY_LOGD("____STEP 1. Calculate Flicker-Free Default Exposure Time____");

    /*
    *   Calculate default exposure time (assume minimum ISO = 100)
    */
    dFNo = ((DOUBLE)a_i4FNo)/10;
    dAV = 2*log10((DOUBLE)dFNo)/log10((DOUBLE)2); // AV = 2xlog2(f/no) = 2xlog10(f/no)/log10(2);
    dLV = ((DOUBLE)a_i4LV)/10;
    dTV = dLV - dAV; // TV = LV - AV
    dDefaultExpTInSec = pow(2, 0-dTV); // T = 2^-TV (uint: sec)
    i4DefaultExpTInUS = (MINT32)(dDefaultExpTInSec*1000000+0.5);

    if (a_i450Hz60Hz == 0) // 50Hz
    {
        i4DefaultExpTInUS = (i4DefaultExpTInUS/10000+1)*10000;
    }
    else // 60Hz
    {
      i4DefaultExpTInUS = (i4DefaultExpTInUS/8333+1)*8333;
    }

    dDefaultExpTInSec = ((DOUBLE)i4DefaultExpTInUS)/1000000;

    MY_LOGD("[F/Number] = %4.2f", dFNo);
    MY_LOGD("[AV] = %4.2f", dAV);
    MY_LOGD("[LV] = %4.2f", dLV);
    MY_LOGD("[TV] = %4.2f", dTV);
    MY_LOGD("[Default Exposure Time in SEC] = %2.15f", dDefaultExpTInSec);
    MY_LOGD("[Default Exposure Time in US] = %d", i4DefaultExpTInUS);
    MY_LOGD("[Flicker-Free Default Exposure Time in SEC] = %2.15f", dDefaultExpTInSec);
    MY_LOGD("[Flicker-Free Default Exposure Time in US] = %d", i4DefaultExpTInUS);

    /*  STEP 2
    *   Calculate Target Exposure Time
    */
    MY_LOGD("____STEP 2. Calculate Target Exposure Time____");

    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

        ae_cfg.u4Eposuretime = i4DefaultExpTInUS;
        ae_cfg.u4AfeGain = i4Gain;
        //ae_cfg.u4IspGain = i4Gain;

        MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
        cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

        sensor_cfg.u8ExposureTime = (MINT64)i4DefaultExpTInUS * 1000;
            sensor_cfg.u4Sensitivity = (i4Gain * minISOGain) / 1024;
        //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

        MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
        cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
    }

    system_sleep(100);



    /*
    *   Take Picture
    */
    if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PROC, (5 * i4DefaultExpTInUS) )) != S_CCT_CALIBRATION_OK)
    {
        MY_LOGE("TakePicture Fail ");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    /*
    *   Raw Analysis
    */
    if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
        MY_LOGE("AnalyzeRAWImage Fail");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
            rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);

    // Calculate target exposure time
    dDefaultExpTGavg = (rawAnalysisResult.fGrAvg + rawAnalysisResult.fGbAvg)/2;
    dTargetGavg = 0.18 * 1023; // 18% reflectance
    dTargetExpTInSec = (dDefaultExpTInSec * dTargetGavg) / dDefaultExpTGavg;
    i4TargetExpTInUS = (MINT32)(dTargetExpTInSec*1000000+0.5);

    MY_LOGD("[Gavg with Default Exposure Time] = %4.2f", dDefaultExpTGavg);
    MY_LOGD("[Target Gavg] = %4.2f", dTargetGavg);
    MY_LOGD("[Target Exposure Time in SEC] = %2.15f", dTargetExpTInSec);
    MY_LOGD("[Target Exposure Time in US] = %d", i4TargetExpTInUS);

    if (i4TargetExpTInUS > 1000000) // > 1sec
    {
        prSensorCalibrationOutput->i4ErrorCode = 2; // Luminance level is too dark
        goto Exit;
    }

    if (a_i450Hz60Hz == 0) // 50Hz
    {
        if (i4TargetExpTInUS < 10000)
        {
            prSensorCalibrationOutput->i4ErrorCode = 3; // Luminance level is too bright
            goto Exit;
        }

        i4TargetExpTInUs_LA = (i4TargetExpTInUS/10000)*10000;
        i4TargetExpTInUs_UA = i4TargetExpTInUs_LA + 10000;
    }
    else // 60Hz
    {
        if (i4TargetExpTInUS < 8333)
        {
            prSensorCalibrationOutput->i4ErrorCode = 3; // Luminance level is too bright
            goto Exit;
        }

        i4TargetExpTInUs_LA = (i4TargetExpTInUS/8333)*8333;
        i4TargetExpTInUs_UA = i4TargetExpTInUs_LA + 8333;
    }

    MY_LOGD("[Flicker-Free Target Exposure Time Upper Approximation in US] = %d", i4TargetExpTInUs_UA);
    MY_LOGD("[Flicker-Free Target Exposure Time Lower Approximation in US] = %d", i4TargetExpTInUs_LA);

    /*
    *  Step 3. Get target G average (lower approximation)____
    */
    MY_LOGD("____STEP 3. Get Target G Average (Lower Approximation)____");

    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

        ae_cfg.u4Eposuretime = i4TargetExpTInUs_LA;
        ae_cfg.u4AfeGain = i4Gain;
        //ae_cfg.u4IspGain = i4Gain;

        MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
        cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

        sensor_cfg.u8ExposureTime = (MINT64)i4TargetExpTInUs_LA * 1000;
            sensor_cfg.u4Sensitivity = (i4Gain * minISOGain) / 1024;
        //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

        MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
        cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
    }

    system_sleep(100);


    /*
    *   Take Picture
    */
    if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PROC, (5 * i4TargetExpTInUs_LA) )) != S_CCT_CALIBRATION_OK)
    {
        MY_LOGE("TakePicture Fail ");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
        MY_LOGE("AnalyzeRAWImage Fail");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
            rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);

    /*
    *   Get target G average (lower approximation)
    */
    dTargetGavg_LA = (rawAnalysisResult.fGrAvg + rawAnalysisResult.fGbAvg)/2;

    MY_LOGD("[Target G Average (Lower Approximation)] = %4.2f", dTargetGavg_LA);

    /*
    *    STEP 4. Get target G average (upper approximation)____
    */
    MY_LOGD("____STEP 4. Get Target G Average (Upper Approximation)____");

    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

        ae_cfg.u4Eposuretime = i4TargetExpTInUs_UA;
        ae_cfg.u4AfeGain = i4Gain;
        //ae_cfg.u4IspGain = i4Gain;

        MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
        cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

        sensor_cfg.u8ExposureTime = (MINT64)i4TargetExpTInUs_UA * 1000;
            sensor_cfg.u4Sensitivity = (i4Gain * minISOGain) / 1024;
        //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

        MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
        cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
    }

    system_sleep(100);


    /*
    *   Take Picture
    */
    if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PROC, (5 * i4TargetExpTInUs_UA) )) != S_CCT_CALIBRATION_OK)
    {
        MY_LOGE("TakePicture Fail ");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    /*
    *   Raw Analysis
    */
    if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
        MY_LOGE("AnalyzeRAWImage Fail");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }

    MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
            rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);

    // Get target G average (upper approximation)
    dTargetGavg_UA = (rawAnalysisResult.fGrAvg + rawAnalysisResult.fGbAvg)/2;

    MY_LOGD("[Target G Average (Upper Approximation)] = %4.2f", dTargetGavg_UA);

    /*
    *   Step 5. calculate minimum ISO____
    */
    MY_LOGD("____STEP 5. Calculate Minimum ISO____");

    i4TargetExpTInUS = i4TargetExpTInUs_UA - (dTargetGavg_UA-dTargetGavg)*(i4TargetExpTInUs_UA-i4TargetExpTInUs_LA)/(dTargetGavg_UA-dTargetGavg_LA);
    dTargetExpTInSec = ((DOUBLE)i4TargetExpTInUS)/1000000;
    dTV = 0 - log10((DOUBLE)dTargetExpTInSec)/log10((DOUBLE)2);
    dMinimumISO = 100 * pow(2, dAV+dTV-dLV);
    prSensorCalibrationOutput->i4MinISO = (MINT32)(dMinimumISO+0.5);

    MY_LOGD("[Target Exposure Time in US] = %d", i4TargetExpTInUS);
    MY_LOGD("[Target Exposure Time in SEC] = %1.15f", dTargetExpTInSec);
    MY_LOGD("[TV] = %4.2f", dTV);
    MY_LOGD("[AV] = %4.2f", dAV);
    MY_LOGD("[LV] = %4.2f", dLV);
    MY_LOGD("[Minimum ISO] = %4.0f", dMinimumISO);



Exit:

#if USE_DYNAMIC_BUF
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }
#endif

    if(backup_bypass != -1)
        NSIspTuningv3::IspTuningMgr::getInstance().setDynamicBypass(0, (MBOOL)backup_bypass);

    MY_LOGD("Restore 3A");
    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&restoreAeMode, 0, NULL, &u4RetLen);
    }

    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);

    MY_LOGD("-");

    return mrRet;

}

/////////////////////////////////////////////////////////////////////////
//
//   mrCalMinimumSaturationGain () -
//!  @brief minimum saturation gain calibration
//!  @param a_i4TargetDeclineRate: target decline rate for knee point detection
//!  @param a_i4GainBuffer: gain buffer to cover sensor module difference
//!  @param a_i4OBLevel: sensor OB level
//!  @param a_i450Hz60Hz: [0] 50Hz [1] 60Hz
//!  @param a_i4SensorMode: [0] preview [1] capture
//!  @param a_i4MinSatGain: minimum saturation gain calibration result
//!  @param prSensorCalibrationOutput: pointer to sensor calibration output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT CctHandle::mrCalMinimumSaturationGain(MINT32 a_i4TargetDeclineRate,
                                                    MINT32 a_i4GainBuffer,
                                                    MINT32 a_i4OBLevel,
                                                    MINT32 a_i450Hz60Hz,
                                                    MINT32 a_i4SensorMode,
                                                    ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput)
{

    MY_LOGD("+");
    MY_LOGD("a_i4TargetDeclineRate = %d", a_i4TargetDeclineRate);
    MY_LOGD("a_i4GainBuffer = %d", a_i4GainBuffer);
    MY_LOGD("a_i4OBLevel = %d", a_i4OBLevel);
    MY_LOGD("a_i450Hz60Hz = %d", a_i450Hz60Hz);
    MY_LOGD("a_i4SensorMode = %d", a_i4SensorMode);

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 MFPos = 0, u4OBSum = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;
    AE_SENSOR_PARAM_T sensor_cfg;
    MUINT32 restoreAeMode = 0, off_AeMode = MTK_CONTROL_AE_MODE_OFF;
    MUINT32 minISOGain = 0;

    ACDK_CDVT_RAW_ANALYSIS_RESULT_T rRAWAnalysisResult[100]; // for temp storage and debug purpose
    ACDK_CDVT_RAW_ANALYSIS_RESULT_T rawAnalysisResult;
    MUINT32 u4MaxExposureUS = 250000;

    memset(rRAWAnalysisResult, 0, sizeof(ACDK_CDVT_RAW_ANALYSIS_RESULT_T) * 100);


    prSensorCalibrationOutput->i4ErrorCode = 0; // No error


    MY_LOGD("Disable 3A");
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);

    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_GET_AEMODE, 0, NULL, sizeof(MUINT32), (MUINT8*)&restoreAeMode, &u4RetLen);
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&off_AeMode, 0, NULL, &u4RetLen);

        //m_p3ANVRAM->AE[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        MUINT32 pAENVRAMLen = sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM;
        char* pAENVRAMBuffer = (char*)malloc(pAENVRAMLen);
        memset(pAENVRAMBuffer,0, pAENVRAMLen);
        NS3Av3::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(mSensorDev, (MVOID *)pAENVRAMBuffer, &pAENVRAMLen);
        AE_NVRAM_T (*pAENVRAMArr)[CAM_SCENARIO_NUM] = (AE_NVRAM_T(*)[CAM_SCENARIO_NUM])pAENVRAMBuffer;
        minISOGain = pAENVRAMArr[0]->rDevicesInfo.u4MiniISOGain;
        free(pAENVRAMBuffer);
        MY_LOGD("minISOGain = %d", minISOGain);
    }


    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_CAPTURE_MODE, (MUINT8 *)&a_i4SensorMode, sizeof(a_i4SensorMode), NULL, 0, &u4RetLen);


    /*
    *   STEP 1. check if sensor can saturate with exposure time = 0.5 sec____
    */
    MY_LOGD("____STEP 1. Check If Sensor Can Saturate with Exposure Time = 0.5 sec____");

    MINT32 i4ExpStartInUS, i4ExpEndInUS, i4ExpInterval;
    DOUBLE dG0,dG1,dG2,dG3; // for slope calculation
    DOUBLE dSlope; // for slope calculation
    DOUBLE dMovingSlope[60] = {}; // moving slope
    MINT32 i4Gain = 1024; // sensor gain = 1024
    ISP_NVRAM_OBC_T obc;
    MINT32 backup_bypass;

    MINT32 i,n;
    DOUBLE dCumulativeMovingSlope; // Cumulative moving slope
    DOUBLE dDeclineRate; // Slope decline rate
    DOUBLE dTargetDeclineRate = ((DOUBLE)a_i4TargetDeclineRate)/100;
    DOUBLE dGainBuffer = ((DOUBLE)a_i4GainBuffer)/100;
    DOUBLE dPreviousDeclineRate = 0; // previous slope decline rate
    DOUBLE dMaxLinearG = 0; // maximum linear G (decline rate = 90%)
    DOUBLE dDefaultMinSatGain; // default minimum gain
    DOUBLE dMinSatGain; // add 10% buffer to cover different sensor module

    /*
    *   Use OBCGain to Compensate OB Level
    */
    MUINT32 u4OBISPRAWGain = (MUINT32)((((DOUBLE)1023)/(1023-a_i4OBLevel))*OB_SLOPEx1_UNIT+0.5);

    /*
    *   Enable Dynamic Bypass
    */
    if((backup_bypass = NSIspTuningv3::IspTuningMgr::getInstance().getDynamicBypass(mSensorDev)) == -1)
    {
        MY_LOGE("[getDynamicBypass] Get Dynamic Bypass Fail ");
        prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
        goto Exit;
    }
    //NSIspTuningv3::IspTuningMgr::getInstance().setDynamicBypass(mSensorDev, MTRUE);

    /*
    *   Prepare OB
    */
    /* temp mark for pure raw control
    obc.offst0.val = a_i4OBLevel;
    obc.offst1.val = a_i4OBLevel;
    obc.offst2.val = a_i4OBLevel;
    obc.offst3.val = a_i4OBLevel;
    obc.gain0.val = u4OBISPRAWGain;
    obc.gain1.val = u4OBISPRAWGain;
    obc.gain2.val = u4OBISPRAWGain;
    obc.gain3.val = u4OBISPRAWGain;

    NSIspTuningv3::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).setEnable(MTRUE);
    NSIspTuningv3::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).put(obc);
    NSIspTuningv3::ISP_MGR_OBC_T::getInstance(ESensorDev_Main).apply(NSIspTuning::EIspProfile_NormalPreview);
    //Temp. Mark
    //NSIspTuningv3::IspTuningMgr::getInstance().setPureOBCInfo(&obc);
    */
    // determine ExpStart, ExpEnd, and ExpInterval
    if (a_i450Hz60Hz == 0) // 50Hz
    {
        i4ExpEndInUS = u4MaxExposureUS;
        i4ExpStartInUS = u4MaxExposureUS - 3*10000;
        i4ExpInterval = 10000;
    }
    else // 60Hz
    {
        i4ExpEndInUS = u4MaxExposureUS;
        i4ExpStartInUS = u4MaxExposureUS - 3*8333;
        i4ExpInterval = 8333;
    }


    for (MINT32 ExpT = i4ExpStartInUS; ExpT <= i4ExpEndInUS; ExpT += i4ExpInterval)
    {
        MY_LOGD("[CctHandle::mrCalMinimumSaturationGain] 1st ExpT = %d, Pat.No = %d", ExpT, i4SequenceNo);

        if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
            cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
            //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

            ae_cfg.u4Eposuretime = ExpT;
            ae_cfg.u4AfeGain = i4Gain;
            //ae_cfg.u4IspGain = i4Gain;

            MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
            cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        }else{
            cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
            MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

            sensor_cfg.u8ExposureTime = (MINT64)ExpT * 1000;
            sensor_cfg.u4Sensitivity = (i4Gain * minISOGain) / 1024;
            //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

            MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
            cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        }

        system_sleep(100);



        /*
        *   Take Picture
        */
        if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PROC, (5 * ExpT)) ) != S_CCT_CALIBRATION_OK)
        {
            MY_LOGE("TakePicture Fail ");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
            MY_LOGE("AnalyzeRAWImage Fail");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }
        rRAWAnalysisResult[i4SequenceNo] = rawAnalysisResult;

        //MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
        //        rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);

        i4SequenceNo++;
    }

    dG0 = (rRAWAnalysisResult[0].fGrAvg + rRAWAnalysisResult[0].fGbAvg)/2;
    dG1 = (rRAWAnalysisResult[1].fGrAvg + rRAWAnalysisResult[1].fGbAvg)/2;
    dG2 = (rRAWAnalysisResult[2].fGrAvg + rRAWAnalysisResult[2].fGbAvg)/2;
    dG3 = (rRAWAnalysisResult[3].fGrAvg + rRAWAnalysisResult[3].fGbAvg)/2;

    CalculateSlope(0, dG0, 1, dG1, 2, dG2, 3, dG3, dSlope);

    MY_LOGD("===================");
    MY_LOGD("1st [G0] = %4.2f", dG0);
    MY_LOGD("1st [G1] = %4.2f", dG1);
    MY_LOGD("1st [G2] = %4.2f", dG2);
    MY_LOGD("1st [G3] = %4.2f", dG3);
    MY_LOGD("1st [Slope] = %4.2f", dSlope);
    MY_LOGD("===================");

    if (dSlope > 2)
    {
        MY_LOGD("____STEP 1. dSlope > 2");
        prSensorCalibrationOutput->i4ErrorCode = 2; // Luminance level is too dark
        goto Exit;
    }

    /*
    *   Step 2. collect moving slope based on 4 consecutive Gavg____
    */
    MY_LOGD("____STEP 2. Collect Moving Slope Based on 4 Consecutive Gavg____");

   // determine ExpStart, ExpEnd, and ExpInterval
    if (a_i450Hz60Hz == 0) // 50Hz
    {
        i4ExpEndInUS = u4MaxExposureUS;
        i4ExpStartInUS = 10000;
        i4ExpInterval = 10000;
    }
    else // 60Hz
    {
        i4ExpEndInUS = u4MaxExposureUS;
        i4ExpStartInUS = 8333;
        i4ExpInterval = 8333;
    }


    i4SequenceNo = 0;

    for (MINT32 ExpT = i4ExpStartInUS; ExpT <= i4ExpEndInUS; ExpT += i4ExpInterval)
    {
        MY_LOGD("[CctHandle::mrCalMinimumSaturationGain] 2nd ExpT = %d, Pat.No = %d", ExpT, i4SequenceNo);

        if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
            cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
            //MY_LOGD("[GET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);

            ae_cfg.u4Eposuretime = ExpT;
            ae_cfg.u4AfeGain = i4Gain;
            //ae_cfg.u4IspGain = i4Gain;

            MY_LOGD("[SET] Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
            cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
        }else{
            cct_InternalOp(FT_CCT_OP_AE_GET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
            MY_LOGD("[GET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);

            sensor_cfg.u8ExposureTime = (MINT64)ExpT * 1000;
            sensor_cfg.u4Sensitivity = (i4Gain * minISOGain) / 1024;
            //sensor_cfg.u8FrameDuration = (MINT64)sensor_cfg.u8ExposureTime;

            MY_LOGD("[SET] u8ExposureTime = %lld, u4Sensitivity = %d, u8FrameDuration = %lld", sensor_cfg.u8ExposureTime, sensor_cfg.u4Sensitivity, sensor_cfg.u8FrameDuration);
            cct_InternalOp(FT_CCT_OP_AE_SET_SENSOR_PARA, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, sizeof(sensor_cfg), (MUINT8*)&sensor_cfg, &u4RetLen);
        }

        system_sleep(100);



        /*
        *   Take Picture
        */
        if((mrRet = TakePicture(a_i4SensorMode, CT_FORMAT_PROC, (5 * ExpT)) ) != S_CCT_CALIBRATION_OK)
        {
            MY_LOGE("TakePicture 2nd Fail ");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        /*
        *   Raw Analysis
        */
        if((mrRet = AnalyzeRAWImage(rawAnalysisResult)) != S_CCT_CALIBRATION_OK) {
            MY_LOGE("AnalyzeRAWImage 2nd Fail");
            prSensorCalibrationOutput->i4ErrorCode = 1; // RAW analysis error
            goto Exit;
        }

        rRAWAnalysisResult[i4SequenceNo] = rawAnalysisResult;

        //MY_LOGD("(%d) Raw: Ravg=%4.2f, Gravg=%4.2f, Gbavg=%4.2f, Bavg=%4.2f, Median=%d", i4SequenceNo,
        //        rawAnalysisResult.fRAvg, rawAnalysisResult.fGrAvg, rawAnalysisResult.fGbAvg, rawAnalysisResult.fBAvg, rawAnalysisResult.u4Median);


        /*
        *   Collect moving slope
        */
        if (i4SequenceNo >= 3)
        {
            dG0 = (rRAWAnalysisResult[i4SequenceNo-3].fGrAvg + rRAWAnalysisResult[i4SequenceNo-3].fGbAvg)/2;
            dG1 = (rRAWAnalysisResult[i4SequenceNo-2].fGrAvg + rRAWAnalysisResult[i4SequenceNo-2].fGbAvg)/2;
            dG2 = (rRAWAnalysisResult[i4SequenceNo-1].fGrAvg + rRAWAnalysisResult[i4SequenceNo-1].fGbAvg)/2;
            dG3 = (rRAWAnalysisResult[i4SequenceNo].fGrAvg + rRAWAnalysisResult[i4SequenceNo].fGbAvg)/2;

            CalculateSlope(0, dG0, 1, dG1, 2, dG2, 3, dG3, dSlope);

            dMovingSlope[i4SequenceNo] = dSlope;

            MY_LOGD("2nd [G0] = %4.2f", dG0);
            MY_LOGD("2nd [G1] = %4.2f", dG1);
            MY_LOGD("2nd [G2] = %4.2f", dG2);
            MY_LOGD("2nd [G3] = %4.2f", dG3);
            MY_LOGD("2nd [Slope] = %4.2f", dSlope);

            if ((dSlope > -2) && (dSlope < 2)) // Almost saturate
            {
                break;
            }
        }

        i4SequenceNo++;

    }

    if (i4SequenceNo < 10)
    {
        MY_LOGE("[i4SequenceNo < 13], Luminance level is too bright ");
        prSensorCalibrationOutput->i4ErrorCode = 3; // Luminance level is too bright
        goto Exit;
    }

    /*
    *   ____Step 3. Calculate minimum saturation gain____
    */
    MY_LOGD("____STEP 3. Calculate Minimum Saturation Gain____");

    n = 0;
    dCumulativeMovingSlope = dMovingSlope[3];

    for (i=3; i<= i4SequenceNo; i++)
    {
        dDeclineRate = 1 - (dMovingSlope[i]/dCumulativeMovingSlope);

        MY_LOGD("[%d] dTargetDeclineRate = %4.4f; dDeclineRate = %4.4f; dMovingSlope = %4.2f; dCumulativeMovingSlope = %4.2f",
                    i,
                    dTargetDeclineRate,
                    dDeclineRate,
                    dMovingSlope[i],
                    dCumulativeMovingSlope);


        if (dDeclineRate < dTargetDeclineRate)
        {
            dPreviousDeclineRate = dDeclineRate; // Backup previous decline rate
            dCumulativeMovingSlope = (dMovingSlope[i] + dCumulativeMovingSlope * n) / (n+1);
            n++;
        }
        else // Estimate maximum linear G
        {
            dG0 = (rRAWAnalysisResult[i-1].fGrAvg + rRAWAnalysisResult[i-1].fGbAvg)/2;
            dG1 = (rRAWAnalysisResult[i].fGrAvg + rRAWAnalysisResult[i].fGbAvg)/2;

            dMaxLinearG = dG0 + (dG1-dG0)*(dTargetDeclineRate-dPreviousDeclineRate)/(dDeclineRate-dPreviousDeclineRate);

            MY_LOGD("[DeclineRate0] = %4.4f",dPreviousDeclineRate);
            MY_LOGD("[DeclineRate1] = %4.4f",dDeclineRate);
            MY_LOGD("[TargetDeclineRate] = %4.4f",dTargetDeclineRate);
            MY_LOGD("[Index0] = %d",i-1);
            MY_LOGD("[Index1] = %d",i);
            MY_LOGD("[G0] = %4.2f",dG0);
            MY_LOGD("[G1] = %4.2f",dG1);
            MY_LOGD("[MaxLinearG] = %4.2f",dMaxLinearG);

            break;
        }
    }

    dDefaultMinSatGain = ((DOUBLE)1023)/dMaxLinearG;
    dMinSatGain = dDefaultMinSatGain * (1+dGainBuffer);
    prSensorCalibrationOutput->i4MinSatGain = ((MINT32)(dMinSatGain * 1024 + 4) / 8) * 8; // must be multiple of 8


    MY_LOGD("dDefaultMinSatGain = %4.4f",dDefaultMinSatGain);
    MY_LOGD("dGainBuffer = %4.4f",dGainBuffer);
    MY_LOGD("dMinSatGain = %4.4f",dMinSatGain);
    MY_LOGD("i4MinSatGain = %d",prSensorCalibrationOutput->i4MinSatGain);

    MY_LOGD("============ Debug Info =============");
    MY_LOGD("[Exp]  [R]   [Gr]   [Gb]   [B]");

    for (MINT32 i=0; i<= i4SequenceNo; i++)
    {
        MY_LOGD("[%d] %4.2f %4.2f %4.2f %4.2f", i+1,
                                                    rRAWAnalysisResult[i].fRAvg,
                                                    rRAWAnalysisResult[i].fGrAvg,
                                                    rRAWAnalysisResult[i].fGbAvg,
                                                    rRAWAnalysisResult[i].fBAvg);
    }

    MY_LOGD("======================================");

Exit:

#if USE_DYNAMIC_BUF
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }
#endif

    if(backup_bypass != -1)
        NSIspTuningv3::IspTuningMgr::getInstance().setDynamicBypass(0, (MBOOL)backup_bypass);

    MY_LOGD("Restore 3A");
    if(a_i4SensorMode==CT_PROFILE_MODE_CAPTURE){
        cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    }else{
        cct_InternalOp(FT_CCT_OP_AE_SET_AEMODE, sizeof(MUINT32), (MUINT8*)&restoreAeMode, 0, NULL, &u4RetLen);
    }

    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);

    MY_LOGD("-");

    return mrRet;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrAEPlineTableLinearity () -
//!  @brief exposure time linearity test
//!  @param a_i4Gain: snsor gain (base = 1024)
//!  @param a_i4ExpMode: [0] exposure line [1] exposure time in us
//!  @param a_i4ExpStart: the starting exposure line or exposure time
//!  @param a_i4ExpEnd: the ending exposure line or exposure time
//!  @param a_i4ExpInterval: The exposure interval in exposure line or exposure time
//!  @param a_i4SensorMode: [0] preview [1] capture
//!  @param a_prSensorTestOutput: pointer to sensor test output structure
//!
/////////////////////////////////////////////////////////////////////////
MRESULT CctHandle::mrAEPlineTableLinearity(
                                        ACDK_CDVT_AE_PLINE_TEST_INPUT_T* in,
                                        int /*inSize*/,
                                        ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T* out,
                                        int /*outSize*/,
                                        MUINT32* /*realOutSize*/)
{
    MY_LOGD("+");

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MUINT32 u4RetLen = 0;
    MINT32 i4SequenceNo = 0;
    MINT32 MFPos = 0;
    ACDK_AE_MODE_CFG_T ae_cfg;
    MINT32 i4ShutterDelayFrame;
    MINT32 i4SensorGainDelayFrame;
    MINT32 i4ISPGainDelayFrame;
    MINT32 i4TestSteps;
    MINT32 i4RepeatTime;
    MINT32 i4IndexScanStart;
    MINT32 i4IndexScanEnd;
    MINT32 i4Index;
    MINT32 i4RepeatIndex;
    MINT32 i4ShutterTime;
    MINT32 i4SensorGain;
    MINT32 i4ISPGain;
    MINT32 i4Yvalue;
    MBOOL bFrameUpdate;
    MINT32 i4WaitYvalueIndex;

    i4ShutterDelayFrame = in->rAEPlinetableInfo.i4ShutterDelayFrame;
    i4SensorGainDelayFrame = in->rAEPlinetableInfo.i4SensorGainDelayFrame;
    i4ISPGainDelayFrame = in->rAEPlinetableInfo.i4ISPGainDelayFrame;
    i4TestSteps = in->rAEPlinetableInfo.i4TestSteps;
    i4RepeatTime = in->rAEPlinetableInfo.i4RepeatTime;
    i4IndexScanStart = in->rAEPlinetableInfo.i4IndexScanStart;
    i4IndexScanEnd = in->rAEPlinetableInfo.i4IndexScanEnd;
    MY_LOGD("Sensor mode = %d\n", in->eSensorMode);
    MY_LOGD("i4ShutterDelayFrame = %d\n", i4ShutterDelayFrame);
    MY_LOGD("i4SensorGainDelayFrame = %d\n", i4SensorGainDelayFrame);
    MY_LOGD("i4ISPGainDelayFrame = %d\n", i4ISPGainDelayFrame);
    MY_LOGD("i4TestSteps = %d\n", i4TestSteps);
    MY_LOGD("i4RepeatTime = %d\n", i4RepeatTime);
    MY_LOGD("i4IndexScanStart = %d\n", i4IndexScanStart);
    MY_LOGD("i4IndexScanEnd = %d\n", i4IndexScanEnd);

    out->i4ErrorCode = 0; // No error
    IAeMgr::getInstance().updateSensorDelayInfo(in->eSensorMode, &i4ShutterDelayFrame, &i4SensorGainDelayFrame, &i4ISPGainDelayFrame);

    // Set the sensor mode.
    // in->rAEPlinetableInfo.eSensorMode

    MY_LOGD("Disable 3A");
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    MY_LOGD("Lock Exposure Setting");
    cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);

    for (i4Index = i4IndexScanStart; i4Index <= i4IndexScanEnd; i4Index += i4TestSteps) {
        i4ShutterTime = in->rAEPlinetableInfo.i4PlineTable[i4Index].i4ShutterTime;
        i4SensorGain = in->rAEPlinetableInfo.i4PlineTable[i4Index].i4SensorGain;
        i4ISPGain = in->rAEPlinetableInfo.i4PlineTable[i4Index].i4ISPGain;

        IAeMgr::getInstance().getBrightnessValue(mSensorDev, &bFrameUpdate, &i4Yvalue);
        for (i4RepeatIndex = 0; i4RepeatIndex < i4RepeatTime; i4RepeatIndex++) {

            cct_OpDispatch(FT_CCT_OP_AE_GET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
            //ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA

            ae_cfg.u4Eposuretime = i4ShutterTime;
            ae_cfg.u4AfeGain = i4SensorGain;
            ae_cfg.u4IspGain = i4ISPGain;

            MY_LOGD("Eposuretime = %d, AfeGain = %d, IspGain = %d", ae_cfg.u4Eposuretime, ae_cfg.u4AfeGain, ae_cfg.u4IspGain);
            cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ae_cfg), (MUINT8*)&ae_cfg, sizeof(ae_cfg), (MUINT8*)&ae_cfg, &u4RetLen);
            //ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO


            //Wait stable
           bFrameUpdate = MFALSE;
           i4WaitYvalueIndex = 0;
           while(i4WaitYvalueIndex <= i4ISPGainDelayFrame) {
                system_sleep(10);    //delay for 10ms
                IAeMgr::getInstance().getBrightnessValue(mSensorDev, &bFrameUpdate, &i4Yvalue);
                if(bFrameUpdate == MTRUE) {
                    i4WaitYvalueIndex++;
                }
                MY_LOGD("[mrAEPlineTableLinearity] Wait frame update i4Yvalue:%d\n", i4Yvalue);
            }

            out->rYAnalysisResult[i4Index*i4RepeatTime + i4RepeatIndex].i4Index = i4Index;
            out->rYAnalysisResult[i4Index*i4RepeatTime + i4RepeatIndex].i4ShutterTime = i4ShutterTime;
            out->rYAnalysisResult[i4Index*i4RepeatTime + i4RepeatIndex].i4SensorGain = i4SensorGain;
            out->rYAnalysisResult[i4Index*i4RepeatTime + i4RepeatIndex].i4ISPGain = i4ISPGain;
            out->rYAnalysisResult[i4Index*i4RepeatTime + i4RepeatIndex].i4Yvalue = i4Yvalue;
        }
    }

    out->i4TestCount = i4Index*i4RepeatTime;

Exit:

#if USE_DYNAMIC_BUF
    //free image buffer if it exist
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf = NULL;
    }
#endif

    MY_LOGD("UnLock Exposure Setting");
    cct_InternalOp(FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    MY_LOGD("Restore 3A");
    cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    cct_InternalOp(FT_CCT_OP_AF_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AWB_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);

    MY_LOGD("-");

   return mrRet;
}


#if 1 // yiyi, shading
extern LSC_CALI_INFO_T g_lsc_cali_info;
/////////////////////////////////////////////////////////////////////////
//
//   mrCalLenShading () -
//!  @brief calibrate lens shading test
//!  @param a_i4GridXNUM: X Grid number
//!  @param a_i4GridYNUM: Y Grid number
//!  @param a_i4XINIBorder: X initial pixel
//!  @param a_i4XENDBorder: X end pixel = width - a_i4XENDBorder
//!  @param a_i4YINIBorder: Y initial pixel
//!  @param a_i4YENDBorder: Y end pixel = height - a_i4YENDBorder
//!  @param a_i4Iris : The Iris
//!  @param a_i4SensorMode: The preview/capture mode
//!  @param a_u1FixShadingIndex: set all shading table for different color temperature to the same value
//!
/////////////////////////////////////////////////////////////////////////
MRESULT
CctHandle::
SetShadingAE(MINT32 i4SensorDev)
{
    MRESULT mrRet = S_CCT_CALIBRATION_OK;

    MUINT32 u4RetLen = 0;
    INT32 i4AEMeteringMode = 0; //0 : LIB3A_AE_METERING_MODE_CENTER_WEIGHT, 2:LIB3A_AE_METERING_MODE_AVERAGE

    ACDK_AE_MODE_CFG_T rAEExpPara;
    MUINT32 u4AfeGainTemp = 0;
    MUINT32 u4ExposureTimeTemp = 0;

    MINT32 i4AEFlickerMode = 0;

    //! ====================================================
    //! Get AE parameter (sensor gain & flicker setting) by user,
    //! User should set AE sensor gain & flicker by CCT tool to a specificated value for calibration.
    //! ====================================================
    memset(&rAEExpPara,0, sizeof(ACDK_AE_MODE_CFG_T));
    cct_OpDispatch(FT_CCT_OP_AE_GET_AUTO_PARA, sizeof(ACDK_AE_MODE_CFG_T), (MUINT8*)&rAEExpPara, sizeof(ACDK_AE_MODE_CFG_T), (MUINT8*)&rAEExpPara, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AE_GET_BAND, 0, NULL, sizeof(MINT32), (MUINT8*)&i4AEFlickerMode, &u4RetLen);

    MY_LOGD("AfeGain(%d), flicker mode(%d), exp time(%d)", u4AfeGainTemp, i4AEFlickerMode, rAEExpPara.u4Eposuretime);

    //average mode : disable histogram condition check.
    //DisableAE before set AE parameter
    cct_OpDispatch(FT_CCT_OP_AE_SET_METERING_MODE, sizeof(MINT32), (MUINT8*)&i4AEMeteringMode, 0, NULL, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    sleep(1); //wait 1 sec for AE stable

    // get current AE parameter
    memset(&rAEExpPara, 0, sizeof(ACDK_AE_MODE_CFG_T));

    //ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_OP_AE_GET_CAPTURE_PARA,
    cct_OpDispatch(FT_CCT_OP_AE_GET_AUTO_PARA, sizeof(ACDK_AE_MODE_CFG_T), (MUINT8*)&rAEExpPara, sizeof(ACDK_AE_MODE_CFG_T), (MUINT8*)&rAEExpPara, &u4RetLen);

    MY_LOGE("ACDK_CCT_OP_AE_GET_CAPTURE_PARA: AfeGain(%d), exposure(%d)", rAEExpPara.u4AfeGain, rAEExpPara.u4Eposuretime);

    MUINT32 pAENVRAMLen = sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM;
    char* pAENVRAMBuffer = (char*)malloc(pAENVRAMLen);
    memset(pAENVRAMBuffer,0, pAENVRAMLen);

    NS3Av3::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(i4SensorDev, (MVOID *)pAENVRAMBuffer, &pAENVRAMLen);
    AE_NVRAM_T (*pAENVRAMArr)[CAM_SCENARIO_NUM] = (AE_NVRAM_T(*)[CAM_SCENARIO_NUM])pAENVRAMBuffer;

    //u4AfeGainTemp = rAEExpPara.u4AfeGain;
    u4AfeGainTemp = pAENVRAMArr[0]->rDevicesInfo.u4MinGain;
    MY_LOGD("AE (AfeGain, exposure, minGain) = (%d, %d, %d)\n"
                , rAEExpPara.u4AfeGain
                , rAEExpPara.u4Eposuretime
                , u4AfeGainTemp
        );

     //DisableAE before set AE parameter
    cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);

    //recalculate exposure gain and set AE parameter
    //this is for using user setting sensor gain to do calibration
    u4ExposureTimeTemp = rAEExpPara.u4Eposuretime * rAEExpPara.u4AfeGain / u4AfeGainTemp;

    MY_LOGE("[AcdkCalibration::mrCalLenShading]"
                        "(u4ExposureTimeTemp ++) = (%d)\n"
                        , u4ExposureTimeTemp
                        );
    if (i4AEFlickerMode != 1) //if user not select, default is 60Hz
        {
        u4ExposureTimeTemp = (u4ExposureTimeTemp/83)*83;  // 1/120 = 8333 us
    }
    else //LIB3A_AE_FLICKER_MODE_50HZ
    {
        u4ExposureTimeTemp = (u4ExposureTimeTemp/100)*100;  // 1/100 = 10000 us
    }


    MY_LOGE("[AcdkCalibration::mrCalLenShading]"
                        "(u4ExposureTimeTemp --) = (%d)\n"
                        , u4ExposureTimeTemp
                        );

     if (u4ExposureTimeTemp < 2000000)
     {
         rAEExpPara.u4Eposuretime = u4ExposureTimeTemp;
         rAEExpPara.u4GainMode = 0; // AfeGain
         rAEExpPara.u4AfeGain = u4AfeGainTemp;
     }

    MY_LOGE("[AcdkCalibration::mrCalLenShading]"
                        "Calibration (AfeGain, exposure) = (%d, %d)\n"
                        , rAEExpPara.u4AfeGain
                        , rAEExpPara.u4Eposuretime
                        );

    //ACDK_CCT_OP_AE_SET_CAPTURE_PARA,
    //[ORI] CCTIF_FeatureCtrl(ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO,  (UINT8 *)&rAEExpPara,  sizeof(ACDK_AE_MODE_CFG_T), NULL, 0, &u4RetLen);
    cct_OpDispatch(FT_CCT_OP_AE_SET_CAPTURE_PARA, sizeof(ACDK_AE_MODE_CFG_T), (MUINT8*)&rAEExpPara, 0, NULL, &u4RetLen);
    MS_SLEEP(500);    //delay for 500ms

    free(pAENVRAMBuffer);
    pAENVRAMArr = NULL;

    return S_CCT_CALIBRATION_OK;
}

MBOOL
CctHandle::
getShadingOnOff()
{
    MUINT32 u4RetLen = 0;
    ACDK_CCT_FUNCTION_ENABLE_STRUCT rOnOff;

    cct_OpDispatch(FT_CCT_OP_GET_SHADING_ON_OFF, 0, NULL, sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT), (MUINT8*)&rOnOff, &u4RetLen);

    MY_LOGD("fgOnOff(%d)", rOnOff.Enable);

    return rOnOff.Enable;
}

MRESULT
CctHandle::
setShadingOnOff(MBOOL fgOnOff)
{
    MUINT32 u4RetLen = 0;
    ACDK_CCT_FUNCTION_ENABLE_STRUCT rOnOff;
    rOnOff.Enable = fgOnOff;

    MY_LOGD("fgOnOff(%d)", fgOnOff);

    cct_OpDispatch(FT_CCT_OP_SET_SHADING_ON_OFF, sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT), (MUINT8*)&rOnOff, 0, NULL, &u4RetLen);

    return S_CCT_CALIBRATION_OK;
}


MBOOL
CctHandle::
GetShadingRaw(eACDK_CAMERA_MODE mode, UINT8 /*ColorTemp*/) {
    MBOOL enable = MTRUE;
    MBOOL disable = MFALSE;
    MBOOL SHAD_EN;
    MUINT32 len;

#define ispEnumSize sizeof(CCT_ISP_CATEGORY_T)
#define boolSize sizeof(MBOOL)
#define paraSize (ispEnumSize + boolSize)

    MUINT8 paraInOBC[paraSize] = {0};
    CCT_ISP_CATEGORY_T ispMduOBC = ISP_CATEGORY_OBC;
    memcpy(paraInOBC, (MUINT8 *)&ispMduOBC, ispEnumSize);
    memcpy(paraInOBC + ispEnumSize, (MUINT8 *)&enable, boolSize);
    MY_LOGD("paraInOBC(%p)", paraInOBC);
    cct_OpDispatch(FT_CCT_OP_ISP_SET_ON_OFF, (MUINT32)paraSize, (MUINT8*)paraInOBC, 0, NULL, &len);

#if 0
    MUINT8 paraInANR[paraSize] = {0};
    CCT_ISP_CATEGORY_T ispMduANR = ISP_CATEGORY_ANR;
    memcpy(paraInANR, (MUINT8 *)&ispMduANR, ispEnumSize);
    memcpy(paraInANR + ispEnumSize, (MUINT8 *)&enable, boolSize);
    MY_LOGD("paraInANR(%p)", paraInANR);
    cct_OpDispatch(FT_CCT_OP_ISP_SET_ON_OFF, (MUINT32)paraSize, (MUINT8*)paraInANR, 0, NULL, &len);
#endif

    SHAD_EN = getShadingOnOff();
    setShadingOnOff(MFALSE);

    if (TakePicture(mode, CT_FORMAT_PROC) != S_CCT_CALIBRATION_OK)
    {
        MY_LOGE("[%s] Get RAW Image Fail \n", __FUNCTION__);
        MY_LOGE(" Get Capture Image Buffer Addr Fail\n");
        setShadingOnOff(SHAD_EN);
        return MFALSE;
    }

    setShadingOnOff(SHAD_EN);

    // unpack raw
    if( UnpackRAWImage() != S_CCT_CALIBRATION_OK )
    {
        MY_LOGE("Unpack Failed!");
        return MFALSE;
    }

    return MTRUE;
}

static MINT32
ConvertToHwTbl(float *p_pgn_float, unsigned int *p_lsc_tbl, int grid_x, int grid_y, int RawImgW, int RawImgH)
{
    float* afWorkingBuf = new float[BUFFERSIZE];
    if (!afWorkingBuf)
    {
        MY_LOGE("Allocate afWorkingBuf Fail");
        return -1;
    }

    MUINT32 u4RetLSCHwTbl;

    u4RetLSCHwTbl =
        LscGaintoHWTbl(p_pgn_float,
                       p_lsc_tbl,
                       grid_x,
                       grid_y,
                       RawImgW,
                       RawImgH,
                       (void*)afWorkingBuf,
                       BUFFERSIZE);

    delete [] afWorkingBuf;

    return u4RetLSCHwTbl;
}

MRESULT CctHandle::mrCalLenShading(INT32 a_i4XINIBorder, INT32 a_i4XENDBorder, INT32 a_i4YINIBorder, INT32 a_i4YENDBorder,  UINT16 a_u2AttnRatio, UINT32 a_u4Index, INT32 a_i4SensorMode, UINT8 a_u1FixShadingIndex, INT32 a_i4AvgWinSize)
{
    MY_LOGD(" Calibrate Lens Shading +");

    MY_LOGD(" XS(%d) XE(%d) YS(%d) YE(%d) a_u2AttnRatio(%d) Ct(%d)  SensorMode(%d) FixShadingIndex(%d) WinSize(%d)",
        a_i4XINIBorder, a_i4XENDBorder, a_i4YINIBorder, a_i4YENDBorder,
        a_u2AttnRatio, a_u4Index, a_i4SensorMode, a_u1FixShadingIndex, a_i4AvgWinSize);


    if (a_u4Index > 4)
    {
        MY_LOGE("Error calibration index \n");
        return E_CCT_CALIBRATION_BAD_ARG;
    }

    MRESULT mrRet = S_CCT_CALIBRATION_OK;
    MRESULT mrRetHWTbl = S_CCT_CALIBRATION_OK;
    MINT32  u4RetLSCHwTbl;
    LSC_CAL_INI_PARAM_T rLSCCaliINIParam;
    memset (&rLSCCaliINIParam, 0, sizeof(LSC_CAL_INI_PARAM_T));
    MUINT32 u4RetLen = 0;
    MINT16 u2SVDTermNum = 7;

    INT32 i4AWBEnable;
    MINT32 u4AEEnable;

    MUINT8 index = (UINT8)a_u4Index;
    MUINT8 uCompMode = CAMERA_TUNING_CAPTURE_SET;
    unsigned short* g_raw_bayer0 = NULL;

    MUINT32 u4DstTblSize;

    //store AE status
    cct_OpDispatch(FT_CCT_OP_AE_GET_ON_OFF, 0, NULL, sizeof(MINT32), (MUINT8*)&u4AEEnable, &u4RetLen);

    if (SetShadingAE(mSensorDev) != S_CCT_CALIBRATION_OK)
        goto Exit;


    cct_OpDispatch(FT_CCT_OP_SET_SHADING_INDEX, sizeof(MUINT32), (MUINT8*)&a_u4Index, 0, NULL, &u4RetLen);
    {
        uCompMode = a_i4SensorMode;

        ACDK_CCT_SHADING_COMP_STRUCT ACDK_Output;
        memset(&ACDK_Output,0,sizeof(ACDK_CCT_SHADING_COMP_STRUCT));
        winmo_cct_shading_comp_struct CCT_shading;
        memset(&CCT_shading,0,sizeof(winmo_cct_shading_comp_struct));
        ACDK_Output.pShadingComp = &CCT_shading;

        cct_InternalOp(FT_CCT_OP_GET_SHADING_PARA, sizeof(MUINT8), (MUINT8*)&uCompMode, sizeof(ACDK_CCT_SHADING_COMP_STRUCT), (MUINT8*)&ACDK_Output, &u4RetLen);


        MY_LOGD("[AcdkCalibration:%s] Before takePicture", __FUNCTION__);

        if (MFALSE == GetShadingRaw((eACDK_CAMERA_MODE)a_i4SensorMode, a_u4Index))
            goto Exit;

        MY_LOGD("[AcdkCalibration:%s] After takePicture", __FUNCTION__);

        rLSCCaliINIParam.i4GridXNUM   = CCT_shading.SHADINGBLK_XNUM + 1;    //a_i4GridXNUM;
        rLSCCaliINIParam.i4GridYNUM   = CCT_shading.SHADINGBLK_YNUM + 1;    //a_i4GridYNUM;
        rLSCCaliINIParam.i4AvgWinSize = a_i4AvgWinSize;
        rLSCCaliINIParam.i4XINIBorder = a_i4XINIBorder;
        rLSCCaliINIParam.i4XENDBorder = a_i4XENDBorder;
        rLSCCaliINIParam.i4YINIBorder = a_i4YINIBorder;
        rLSCCaliINIParam.i4YENDBorder = a_i4YENDBorder;
        rLSCCaliINIParam.u4ImgWidth   = rawWidth;
        rLSCCaliINIParam.u4ImgHeight  = rawHeight;
        rLSCCaliINIParam.u2BayerStart = (UINT16)rawColorOrder;
        rLSCCaliINIParam.poly_coef.coef_a = (float)ShadingATNTable[a_u2AttnRatio][0] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_b = (float)ShadingATNTable[a_u2AttnRatio][1] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_c = (float)ShadingATNTable[a_u2AttnRatio][2] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_d = (float)ShadingATNTable[a_u2AttnRatio][3] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_e = (float)ShadingATNTable[a_u2AttnRatio][4] / ShadingATNTable[a_u2AttnRatio][5];
        rLSCCaliINIParam.poly_coef.coef_f = 1;
        rLSCCaliINIParam.i4CompensationIdx = a_u2AttnRatio;

        g_raw_bayer0 = (unsigned short*)malloc(rawWidth*rawHeight*2*sizeof(MUINT8));
        if(!g_raw_bayer0)
        {
             MY_LOGD(" Mem allocation Fail \n");
            return E_CCT_CALIBRATION_NULL_OBJ;
        }

        MY_LOGD("Before vLsc_Remap_to_Bayer0: unpackBufSize=%d, rawUnpackBuf=%p, rawWidth=%d, rawHeight=%d", unpackBufSize, rawUnpackBuf, rawWidth, rawHeight);
        vLsc_Remap_to_Bayer0((unsigned short*)rawUnpackBuf, g_raw_bayer0, rawWidth, rawHeight, rLSCCaliINIParam.u2BayerStart);

        MY_LOGD("GridXNUM(%d), GridYNUM(%d)", rLSCCaliINIParam.i4GridXNUM, rLSCCaliINIParam.i4GridYNUM);
        MY_LOGD("Attenuation coef[%d]: (%8f, %8f, %8f, %8f ,%8f, %8f)", a_u2AttnRatio,
                rLSCCaliINIParam.poly_coef.coef_a,
                rLSCCaliINIParam.poly_coef.coef_b,
                rLSCCaliINIParam.poly_coef.coef_c,
                rLSCCaliINIParam.poly_coef.coef_d,
                rLSCCaliINIParam.poly_coef.coef_e,
                rLSCCaliINIParam.poly_coef.coef_f);

        if (a_u2AttnRatio == 0)
        {
            rLSCCaliINIParam.poly_coef.ratio_poly_flag = 0;
        }
        else
        {
            rLSCCaliINIParam.poly_coef.ratio_poly_flag = 1;
        }

        MY_LOGD("vLSC_PARAM_INIT");
        vLSC_PARAM_INIT(rLSCCaliINIParam);
        MY_LOGD("vLSC_Calibration_INIT");
        vLSC_Calibration_INIT(g_raw_bayer0);
        MY_LOGD("mrLSC_Calibrate");
        mrLSC_Calibrate(g_lsc_cali_info, 0 /* Preview MODE */, u2SVDTermNum, 0);//preview

#if SHADING_VER == 5
        u4DstTblSize = (rLSCCaliINIParam.i4GridXNUM-1)*(rLSCCaliINIParam.i4GridYNUM-1)*24;
#else
        u4DstTblSize = (rLSCCaliINIParam.i4GridXNUM-1)*(rLSCCaliINIParam.i4GridYNUM-1)*16;
#endif

        NSIspTuning::ILscTbl rTblFloat(NSIspTuning::ILscTable::GAIN_FLOAT);
        NSIspTuning::ILscTbl rTblFixed(NSIspTuning::ILscTable::GAIN_FIXED);
        rTblFloat.setConfig(rawWidth, rawHeight, rLSCCaliINIParam.i4GridXNUM, rLSCCaliINIParam.i4GridYNUM);
        rTblFloat.setData(g_lsc_cali_info.tbl_info.src_tbl_addr_float, rTblFloat.getSize());
        char strFile[512] = {'\0'};
        sprintf(strFile, "/data/vendor/shading/KLSC_F_CFG(%dx%d_%dx%d)_BD(%d,%d,%d,%d)_ATT(%d)_WIN(%d)_IDX(%d).log",
            rLSCCaliINIParam.i4GridXNUM, rLSCCaliINIParam.i4GridYNUM, rawWidth, rawHeight,
            a_i4XINIBorder, a_i4XENDBorder, a_i4YINIBorder, a_i4YENDBorder, a_u2AttnRatio, a_i4AvgWinSize, a_u4Index);
        rTblFloat.dump(strFile);
        sprintf(strFile, "/data/vendor/shading/KLSC_G_CFG(%dx%d_%dx%d)_BD(%d,%d,%d,%d)_ATT(%d)_WIN(%d)_IDX(%d).log",
                    rLSCCaliINIParam.i4GridXNUM, rLSCCaliINIParam.i4GridYNUM, rawWidth, rawHeight,
                    a_i4XINIBorder, a_i4XENDBorder, a_i4YINIBorder, a_i4YENDBorder, a_u2AttnRatio, a_i4AvgWinSize, a_u4Index);
        rTblFloat.convert(rTblFixed);
        rTblFixed.dump(strFile);

        u4RetLSCHwTbl = ConvertToHwTbl(g_lsc_cali_info.tbl_info.src_tbl_addr_float,
                                       g_lsc_cali_info.tbl_info.dst_tbl_addr,
                                       rLSCCaliINIParam.i4GridXNUM,
                                       rLSCCaliINIParam.i4GridYNUM,
                                       rawWidth,
                                       rawHeight);
        if (u4RetLSCHwTbl != S_LSC_CONVERT_OK)
        {
            MY_LOGE("ConvertToHwTbl Overflow");
            mrRetHWTbl = E_CCT_CALIBRATION_COEF_OVERFLOW;
        }

        ACDK_CCT_TABLE_SET_STRUCT  shadingTablePolyCoef;
        memset(&shadingTablePolyCoef, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
        shadingTablePolyCoef.Offset = 0;
        shadingTablePolyCoef.Length = u4DstTblSize;
        shadingTablePolyCoef.pBuffer = (UINT32 *)g_lsc_cali_info.tbl_info.dst_tbl_addr;//g_lsc_cali_info.tbl_info.dst_tbl_addr;
        shadingTablePolyCoef.Mode = (CAMERA_TUNING_SET_ENUM)a_i4SensorMode;
        if (a_u1FixShadingIndex == 0)
        {
            shadingTablePolyCoef.ColorTemp = a_u4Index;
            cct_InternalOp(FT_CCT_OP_SET_SHADING_TABLE_POLYCOEF, sizeof(ACDK_CCT_TABLE_SET_STRUCT), (MUINT8*)&shadingTablePolyCoef, 0, NULL, &u4RetLen);
        }
        else
        {
            int i;
            for (i = 0; i < 4; i++)
            {
                MY_LOGD("[%s] color temp %d", __FUNCTION__, i);

                shadingTablePolyCoef.ColorTemp = i;
                cct_InternalOp(FT_CCT_OP_SET_SHADING_TABLE_POLYCOEF, sizeof(ACDK_CCT_TABLE_SET_STRUCT), (MUINT8*)&shadingTablePolyCoef, 0, NULL, &u4RetLen);
            }
        }

        vLSC_Calibration_END();

        MY_LOGD("SensorMode(%d) Done", a_i4SensorMode);
    }

Exit:
    MY_LOGD("Exit:");

    if (u4AEEnable == 1)
    {
        // Enable AE
        MY_LOGD("Exit: ACDK_CCT_OP_AE_ENABLE");
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bTrueFlag, 0, NULL, &u4RetLen);
    }
    else
    {
        // Disable AE
        MY_LOGD("Exit: ACDK_CCT_OP_AE_DISABLE");
        cct_OpDispatch(FT_CCT_OP_AE_SET_ON_OFF, sizeof(MBOOL), (MUINT8*)&bFalseFlag, 0, NULL, &u4RetLen);
    }

    //restore color temperature index change by AWB 2nd flow, this index is not used when capture raw, but will use in later actions.
    MY_LOGD("Exit: ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX");
    cct_OpDispatch(FT_CCT_OP_SET_SHADING_INDEX, sizeof(MUINT32), (MUINT8*)&index, 0, NULL, &u4RetLen);


//#define MY_LOGD(fmt,arg...)      XLOGD("[%s] " fmt, __func__, ##arg)
    //m_eRawFormat = TWO_BYTE_RAW;

#if USE_DYNAMIC_BUF
    if (m_pucRawBuf != NULL)
    {
        free(m_pucRawBuf);
        m_pucRawBuf =NULL;
    }
#endif

    MY_LOGD("Exit: Lens Shading Calibration Finish !! mrRetHWTbl(0x%08x), mrRet(0x%08x)", mrRetHWTbl, mrRet);

    if(g_raw_bayer0 != NULL)
    {
        free(g_raw_bayer0);
    }

    return ((E_CCT_CALIBRATION_COEF_OVERFLOW  == mrRetHWTbl) ?
        E_CCT_CALIBRATION_COEF_OVERFLOW  : mrRet);
}
#endif // yiyi, shading






MINT32
CctHandle::
cct_HandleCalibrationOp(CCT_OP_ID op,
                    MUINT32 u4ParaInLen,
                    MUINT8 *puParaIn,
                    MUINT32 u4ParaOutLen,
                    MUINT8 *puParaOut,
                    MUINT32 *pu4RealParaOutLen )
{
    MY_LOGD("[%s] op(%d), u4ParaInLen(%d), puParaIn(%p), u4ParaOutLen(%d), puParaOut(%p), pu4RealParaOutLen(%p)", __FUNCTION__, \
      op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);

    MINT32 err = CCTIF_NO_ERROR;
    MINT32 fgRet = S_CCT_CALIBRATION_OK;


    switch(op) {
    case FT_CCT_OP_CAL_CDVT_CALIBRATION:
        {
            if( puParaOut == NULL )
            {
                MY_LOGE("[%s] puParaOut is NULL", __FUNCTION__);
                err = CCTIF_UNKNOWN_ERROR;
                break;
            }
            ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T *prSensorCalibrationInput = (ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T *)puParaIn;
            ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput = (ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *)puParaOut;
            memset(prSensorCalibrationOutput, 0x0, sizeof(ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T));
            *pu4RealParaOutLen = sizeof(ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T);

            MY_LOGD("=========================================");
            MY_LOGD("eCalibrationItem = %d",prSensorCalibrationInput->eCalibrationItem);
            MY_LOGD("eSensorMode = %d",prSensorCalibrationInput->eSensorMode);
            MY_LOGD("rOB.i4ExpTime = %d",prSensorCalibrationInput->rOB.i4ExpTime);
            MY_LOGD("rOB.i4Gain = %d",prSensorCalibrationInput->rOB.i4Gain);
            MY_LOGD("rOB.i4RepeatTimes = %d",prSensorCalibrationInput->rOB.i4RepeatTimes);
            MY_LOGD("rMinISO.i4LV = %d",prSensorCalibrationInput->rMinISO.i4LV);
            MY_LOGD("rMinISO.i4FNumber = %d",prSensorCalibrationInput->rMinISO.i4FNumber);
            MY_LOGD("rMinISO.eFlicker = %d",prSensorCalibrationInput->rMinISO.eFlicker);
            MY_LOGD("rMinISO.i4OB = %d",prSensorCalibrationInput->rMinISO.i4OB);
            MY_LOGD("rMinSatGain.i4TargetDeclineRate = %d",prSensorCalibrationInput->rMinSatGain.i4TargetDeclineRate);
            MY_LOGD("rMinSatGain.i4GainBuffer = %d",prSensorCalibrationInput->rMinSatGain.i4GainBuffer);
            MY_LOGD("rMinSatGain.eFlicker = %d",prSensorCalibrationInput->rMinSatGain.eFlicker);
            MY_LOGD("rMinSatGain.i4OB = %d",prSensorCalibrationInput->rMinSatGain.i4OB);
            MY_LOGD("=========================================");

            switch (prSensorCalibrationInput->eCalibrationItem)
            {
                case ACDK_CDVT_CALIBRATION_OB:
                    mrCalOB(prSensorCalibrationInput->rOB.i4ExpTime,
                                   prSensorCalibrationInput->rOB.i4Gain,
                                   prSensorCalibrationInput->rOB.i4RepeatTimes,
                                   (INT32)prSensorCalibrationInput->eSensorMode,
                                   prSensorCalibrationOutput);

                    break;
                case ACDK_CDVT_CALIBRATION_MIN_ISO:
                    mrCalMinISO(prSensorCalibrationInput->rMinISO.i4LV,
                                         prSensorCalibrationInput->rMinISO.i4FNumber,
                                         prSensorCalibrationInput->rMinISO.i4OB,
                                         (INT32)prSensorCalibrationInput->rMinISO.eFlicker,
                                         (INT32)prSensorCalibrationInput->eSensorMode,
                                         prSensorCalibrationOutput);
                    break;
                case ACDK_CDVT_CALIBRATION_MIN_SAT_GAIN:
                    mrCalMinimumSaturationGain(prSensorCalibrationInput->rMinSatGain.i4TargetDeclineRate,
                                                                    prSensorCalibrationInput->rMinSatGain.i4GainBuffer,
                                                                    prSensorCalibrationInput->rMinSatGain.i4OB,
                                                                    (INT32)prSensorCalibrationInput->rMinSatGain.eFlicker,
                                                                   (INT32)prSensorCalibrationInput->eSensorMode,
                                                                    prSensorCalibrationOutput);
                    break;
            }
            MY_LOGD("=========================================");
            MY_LOGD("i4ErrorCode = %d",prSensorCalibrationOutput->i4ErrorCode);
            MY_LOGD("i4MinISO = %d",prSensorCalibrationOutput->i4MinISO);
            MY_LOGD("i4MinSatGain = %d",prSensorCalibrationOutput->i4MinSatGain);
            MY_LOGD("i4OB = %d",prSensorCalibrationOutput->i4OB);
            MY_LOGD("=========================================");
        }
        break;

    case FT_CCT_OP_CAL_CDVT_TEST:
        {
            if( puParaOut == NULL )
            {
                MY_LOGE("[%s] puParaOut is NULL", __FUNCTION__);
                err = CCTIF_UNKNOWN_ERROR;
                break;
            }
            ACDK_CDVT_SENSOR_TEST_INPUT_T *prSensorTestInput = (ACDK_CDVT_SENSOR_TEST_INPUT_T *)puParaIn;
            AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T *prSensorTestOutput = (AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T *)puParaOut;
            ACDK_CDVT_SENSOR_TEST_OUTPUT_T sensor_test_output;
            memset(prSensorTestOutput, 0x0, sizeof(AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T));
            memset(&sensor_test_output, 0x0, sizeof(ACDK_CDVT_SENSOR_TEST_OUTPUT_T));
            *pu4RealParaOutLen = sizeof(AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T);

            MY_LOGD("=========================================\n");
            MY_LOGD("eTestItem = %d",prSensorTestInput->eTestItem);
            MY_LOGD("eSensorMode = %d",prSensorTestInput->eSensorMode);
            MY_LOGD("rExpLinearity.i4Gain = %d",prSensorTestInput->rExpLinearity.i4Gain);
            MY_LOGD("rExpLinearity.eExpMode = %d",prSensorTestInput->rExpLinearity.eExpMode);
            MY_LOGD("rExpLinearity.i4ExpStart = %d",prSensorTestInput->rExpLinearity.i4ExpStart);
            MY_LOGD("rExpLinearity.i4ExpEnd = %d",prSensorTestInput->rExpLinearity.i4ExpEnd);
            MY_LOGD("rExpLinearity.i4ExpInterval = %d",prSensorTestInput->rExpLinearity.i4ExpInterval);
            MY_LOGD("rGainLinearityOBStability.eGainControlMode = %d",prSensorTestInput->rGainLinearityOBStability.eGainControlMode);
            MY_LOGD("rGainLinearityOBStability.i4ExpTime = %d",prSensorTestInput->rGainLinearityOBStability.i4ExpTime);
            MY_LOGD("prSensorTestInput->rGainLinearityOBStability.i4GainStart = %d",prSensorTestInput->rGainLinearityOBStability.i4GainStart);
            MY_LOGD("prSensorTestInput->rGainLinearityOBStability.i4GainEnd = %d",prSensorTestInput->rGainLinearityOBStability.i4GainEnd);
            MY_LOGD("prSensorTestInput->rGainLinearityOBStability.i4GainInterval = %d",prSensorTestInput->rGainLinearityOBStability.i4GainInterval);
            MY_LOGD("=========================================\n");

            switch (prSensorTestInput->eTestItem)
            {
                case ACDK_CDVT_TEST_EXPOSURE_LINEARITY:
                    mrExpLinearity( prSensorTestInput->rExpLinearity.i4Gain,
                                    (INT32)prSensorTestInput->rExpLinearity.eExpMode,
                                    prSensorTestInput->rExpLinearity.i4ExpStart,
                                    prSensorTestInput->rExpLinearity.i4ExpEnd,
                                    prSensorTestInput->rExpLinearity.i4ExpInterval,
                                    (INT32)prSensorTestInput->eSensorMode,
                                    &sensor_test_output);
                    break;

                case ACDK_CDVT_TEST_GAIN_LINEARITY:
                    if (prSensorTestInput->rGainLinearityOBStability.eGainControlMode == ACDK_CDVT_GAIN_CONFIG)
                    {
                        mrGainLinearity(    prSensorTestInput->rGainLinearityOBStability.i4ExpTime,
                                            prSensorTestInput->rGainLinearityOBStability.i4GainStart,
                                            prSensorTestInput->rGainLinearityOBStability.i4GainEnd,
                                            prSensorTestInput->rGainLinearityOBStability.i4GainInterval,
                                            (INT32)prSensorTestInput->eSensorMode,
                                            &sensor_test_output);
                    }
                    else // ACDK_CDVT_GAIN_TABLE
                    {
                        mrGainTableLinearity(   prSensorTestInput->rGainLinearityOBStability.i4ExpTime,
                                                prSensorTestInput->rGainLinearityOBStability.i4GainTableSize,
                                                prSensorTestInput->rGainLinearityOBStability.i4GainTable,
                                                (INT32)prSensorTestInput->eSensorMode,
                                                &sensor_test_output);
                    }
                    break;
                case ACDK_CDVT_TEST_OB_STABILITY:
                    if (prSensorTestInput->rGainLinearityOBStability.eGainControlMode == ACDK_CDVT_GAIN_CONFIG)
                    {
                        mrOBStability(  prSensorTestInput->rGainLinearityOBStability.i4ExpTime,
                                        prSensorTestInput->rGainLinearityOBStability.i4GainStart,
                                        prSensorTestInput->rGainLinearityOBStability.i4GainEnd,
                                        prSensorTestInput->rGainLinearityOBStability.i4GainInterval,
                                        (INT32)prSensorTestInput->eSensorMode,
                                        &sensor_test_output);
                    }
                    else // ACDK_CDVT_GAIN_TABLE
                    {
                        mrGainTableOBStability( prSensorTestInput->rGainLinearityOBStability.i4ExpTime,
                                                prSensorTestInput->rGainLinearityOBStability.i4GainTableSize,
                                                prSensorTestInput->rGainLinearityOBStability.i4GainTable,
                                                (INT32)prSensorTestInput->eSensorMode,
                                                &sensor_test_output);
                    }
                    break;
                default:
                    MY_LOGE("No this test: prSensorTestInput->eTestItem = %d", prSensorTestInput->eTestItem);
                    break;
            }
            MY_LOGD("=========================================\n");
            MY_LOGD("i4ErrorCode = %d\n",sensor_test_output.i4ErrorCode);
            MY_LOGD("i4TestCount = %d\n",sensor_test_output.i4TestCount);
            for (INT32 i=0; i<sensor_test_output.i4TestCount; i++)
            {
                MY_LOGD(    "[%d] %4.2f %4.2f %4.2f %4.2f %d \n",
                            i,
                            sensor_test_output.rRAWAnalysisResult[i].fRAvg,
                            sensor_test_output.rRAWAnalysisResult[i].fGrAvg,
                            sensor_test_output.rRAWAnalysisResult[i].fGbAvg,
                            sensor_test_output.rRAWAnalysisResult[i].fBAvg,
                            sensor_test_output.rRAWAnalysisResult[i].u4Median);
            }
            MY_LOGD("=========================================\n");
            MY_LOGD("Translate to AP_ACDK_CDVT_SENSOR_TEST_OUTPUT_T\n");
            prSensorTestOutput->i4ErrorCode = sensor_test_output.i4ErrorCode;
            prSensorTestOutput->i4TestCount = sensor_test_output.i4TestCount;
            for(int cnt = 0; cnt < prSensorTestOutput->i4TestCount; cnt ++){
                prSensorTestOutput->rRAWAnalysisResult[cnt].fRAvg = (int)(sensor_test_output.rRAWAnalysisResult[cnt].fRAvg * 1000);
                prSensorTestOutput->rRAWAnalysisResult[cnt].fGrAvg = (int)(sensor_test_output.rRAWAnalysisResult[cnt].fGrAvg * 1000);
                prSensorTestOutput->rRAWAnalysisResult[cnt].fGbAvg = (int)(sensor_test_output.rRAWAnalysisResult[cnt].fGbAvg * 1000);
                prSensorTestOutput->rRAWAnalysisResult[cnt].fBAvg = (int)(sensor_test_output.rRAWAnalysisResult[cnt].fBAvg * 1000);
                prSensorTestOutput->rRAWAnalysisResult[cnt].u4Median = sensor_test_output.rRAWAnalysisResult[cnt].u4Median;
            }
            MY_LOGD("=========================================\n");
        }
        break;
    case FT_CCT_OP_CAL_SHADING:
#if 1 // yiyi, shading
        {
            typedef ACDK_CCT_LSC_CAL_SET_STRUCT i_type;
            i_type* const pShadingCalSet = reinterpret_cast<i_type*> (puParaIn);

            if ( sizeof (i_type) !=  u4ParaInLen || ! puParaIn )
                return E_CCT_CALIBRATION_BAD_ARG;

            MY_LOGD("ACDK_CCT_V2_OP_SHADING_CAL: mode(%d), ctIdx(%d)"
                    ,pShadingCalSet->mode
                    ,pShadingCalSet->colorTemp);
            MUINT32 _mode = (MUINT32)pShadingCalSet->mode;
            MUINT32 _max_mode = (MUINT32)CAMERA_MODE_NUM;
            if ( _mode < _max_mode)
            {
                MY_LOGD("Cal Shading xstart(%d) ystart(%d) xend(%d) yend(%d) ratio(%d) ct(%d)",
                    pShadingCalSet->boundaryStartX, pShadingCalSet->boundaryStartY,
                    pShadingCalSet->boundaryEndX, pShadingCalSet->boundaryEndY,
                    pShadingCalSet->attnRatio, pShadingCalSet->colorTemp);
                fgRet = mrCalLenShading(
                     pShadingCalSet->boundaryStartX
                    ,pShadingCalSet->boundaryStartY
                    ,pShadingCalSet->boundaryEndX
                    ,pShadingCalSet->boundaryEndY
                    ,pShadingCalSet->attnRatio
                    ,pShadingCalSet->colorTemp
                    ,pShadingCalSet->mode
                    ,pShadingCalSet->u1FixShadingIndex
                    ,pShadingCalSet->avgWinSize);
            }
            else
            {
                MY_LOGE("Erro Cal mode:(%d)", pShadingCalSet->mode);
                fgRet = E_CCT_CALIBRATION_BAD_ARG;
            }
        }
#endif
        break;
    case FT_CCT_OP_CAL_AE_GET_FALRE:
        if( puParaOut == NULL )
        {
            MY_LOGE("[%s] puParaOut is NULL", __FUNCTION__);
            err = CCTIF_UNKNOWN_ERROR;
            break;
        }
        //yiyi, em, todo
        //err = NS3Av3::IAeMgr::getInstance().CCTOPAEGetFlareOffset(mSensorDev, *((MUINT32 *)puParaIn), (MUINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case FT_CCT_OP_CAL_AE_GET_EV:
        if( puParaOut == NULL )
        {
            MY_LOGE("[%s] puParaOut is NULL", __FUNCTION__);
            err = CCTIF_UNKNOWN_ERROR;
            break;
        }
        err = NS3Av3::IAeMgr::getInstance().CCTOPAEGetCurrentEV(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case FT_CCT_OP_CAL_FLASH:
        {
            MY_LOGD("Flash Calibration Start: mSensorDev=%d", mSensorDev);
#if (CAM3_3A_ISP_50_EN)
            FlashMgr::getInstance(mSensorDev)->cctSetSpModeCalibration();
            MINT32 fgRetTemp = TakePicture(CT_PROFILE_MODE_CAPTURE, CT_FORMAT_PURE);
            FlashMgr::getInstance(mSensorDev)->cctSetSpModeNormal();
#else
            FlashMgr::getInstance().cctSetSpModeCalibration(mSensorDev);
            MINT32 fgRetTemp = TakePicture(CT_PROFILE_MODE_CAPTURE, CT_FORMAT_PURE);
            FlashMgr::getInstance().cctSetSpModeNormal(mSensorDev);
#endif

            if(fgRetTemp != S_CCT_CALIBRATION_OK){
                MY_LOGE("CCT TakePicture Fail, Please use Flash Calibration Property for instead");
            }
            else{
                MY_LOGD("Flash Calibration Done");
            }
        }
        break;
        /*
    case FT_CCT_OP_CAL_AE_PLINE_TABLE:
        {
            ACDK_CDVT_AE_PLINE_TEST_INPUT_T *prSensorAEPlineTableInput = (ACDK_CDVT_AE_PLINE_TEST_INPUT_T *)puParaIn;
            ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T *prSensorAEPlineTableOutput = (ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T *)puParaOut;

            mrAEPlineTableLinearity(prSensorAEPlineTableInput, u4ParaInLen, prSensorAEPlineTableOutput, u4ParaOutLen, pu4RealParaOutLen);
        }
        break;
        */
    default:
        MY_LOGE("Not support cmd %d", (int)op);
        fgRet = E_CCT_CALIBRATION_BAD_ARG;
        break;

    }

    if(fgRet != S_CCT_CALIBRATION_OK)
    {
        MY_LOGE("fgRet=0x%x, err=0x%x", fgRet, err);
        err = CCTIF_UNKNOWN_ERROR;
    }
    return err;
}



MINT32
CctHandle::
cct_OpDispatch(CCT_OP_ID op,
                MUINT32 u4ParaInLen,
                MUINT8 *puParaIn,
                MUINT32 u4ParaOutLen,
                MUINT8 *puParaOut,
                MUINT32 *pu4RealParaOutLen )
{
    MINT32 status = 0;

    if (op >= FT_CCT_OP_SENSOR_START && op < FT_CCT_OP_3A_START ) {
        status = cct_HandleSensorOp(op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);
    } else if (op >= FT_CCT_OP_3A_START && op < FT_CCT_OP_ISP_START ) {
        status = cct_Handle3AOp(op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);
    } else if (op >= FT_CCT_OP_ISP_START && op < FT_CCT_OP_NVRAM_START ) {
        status = cct_HandleIspOp(op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);
    } else if (op >= FT_CCT_OP_NVRAM_START && op < FT_CCT_OP_CAL_START ) {
        status = cct_HandleNvramOp(op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);
    } else if (op >= FT_CCT_OP_CAL_START && op < FT_CCT_OP_SHELL_START ) {
        status = cct_HandleCalibrationOp(op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);
    } else if (op >= FT_CCT_OP_INTERNAL_START && op < FT_CCT_OP_EMCAM_START ) {
        MY_LOGE("[%s] op=0x%x is not open for use", op);
        //status = cct_InternalOp(op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);
    } else if (op >= FT_CCT_OP_EMCAM_START && op < FT_CCT_OP_END ) {
        status = cct_HandleEmcamOp(op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);
    } else {
        MY_LOGE("[%s] op=0x%x is unknown", op);
    }

    return status;
}

MINT32
CctHandle::
cct_InternalOp( CCT_OP_ID op,
                MUINT32 u4ParaInLen,
                MUINT8 *puParaIn,
                MUINT32 u4ParaOutLen,
                MUINT8 *puParaOut,
                MUINT32 *pu4RealParaOutLen )
{
    MY_LOGD("[%s] op(%d), u4ParaInLen(%d), puParaIn(%p), u4ParaOutLen(%d), puParaOut(%p), pu4RealParaOutLen(%p)", __FUNCTION__, \
      op, u4ParaInLen, puParaIn, u4ParaOutLen, puParaOut, pu4RealParaOutLen);

    MINT32 err = CCTIF_NO_ERROR;
    MBOOL enable;

    switch(op) {
    case FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF:
        if (u4ParaInLen == sizeof(MBOOL)) {
            enable = (MBOOL) *puParaIn;
            if (enable)
                err = NS3Av3::IAeMgr::getInstance().CCTOPAEUnLockExpSetting(mSensorDev);
            else
                err = NS3Av3::IAeMgr::getInstance().CCTOPAELockExpSetting(mSensorDev);
        } else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_SET_AEMODE:
        if (u4ParaInLen == sizeof(MUINT32)) {
            MUINT32 mode = (MUINT32) *puParaIn;
            err = NS3Av3::IAeMgr::getInstance().setAEMode(mSensorDev, mode);
        } else {
            err = CCTIF_BAD_PARAM;
        }
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_GET_AEMODE:
        if (puParaOut != NULL) {
            MINT32* pMode = reinterpret_cast<MINT32*>(puParaOut);
            *pMode = NS3Av3::IAeMgr::getInstance().getAEMode(mSensorDev);
            *pu4RealParaOutLen = sizeof(MINT32);
        } else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AE_SET_SENSOR_PARA:
        if (u4ParaInLen == sizeof(AE_SENSOR_PARAM_T)) {
            AE_SENSOR_PARAM_T* param = reinterpret_cast<AE_SENSOR_PARAM_T*>(puParaIn);
            err = NS3Av3::IAeMgr::getInstance().UpdateSensorParams(mSensorDev, *param);
        } else {
            err = CCTIF_BAD_PARAM;
        }
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_AE_GET_SENSOR_PARA:
        if (puParaOut != NULL) {
            AE_SENSOR_PARAM_T* param = reinterpret_cast<AE_SENSOR_PARAM_T*>(puParaOut);
            err = NS3Av3::IAeMgr::getInstance().getSensorParams(mSensorDev, *param);
            *pu4RealParaOutLen = sizeof(AE_SENSOR_PARAM_T);
        } else {
            err = CCTIF_BAD_PARAM;
            *pu4RealParaOutLen = 0;
        }
        break;
    case FT_CCT_OP_AF_SET_ON_OFF:
        if (u4ParaInLen == sizeof(MBOOL)) {
            enable = (MBOOL) *puParaIn;
            err = SetAFMode(enable);
            if (enable)
                err |= NS3Av3::IAfMgr::getInstance().CCTOPAFEnable(mSensorDev);
            else
                err |= NS3Av3::IAfMgr::getInstance().CCTOPAFDisable(mSensorDev);
        } else
            err = CCTIF_BAD_PARAM;
        *pu4RealParaOutLen = 0;
        break;
    case FT_CCT_OP_GET_SHADING_PARA:
        {
        if ( ! puParaIn ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        if ( ! puParaOut ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        ESensorMode_T eSensorMode = *reinterpret_cast<ESensorMode_T*>(puParaIn);
        ACDK_CCT_SHADING_COMP_STRUCT*const pShadingPara = reinterpret_cast<ACDK_CCT_SHADING_COMP_STRUCT*>(puParaOut);

        MY_LOGD("[%s] GET_SHADING_PARA SensorMode(%d)", __FUNCTION__, eSensorMode);

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        if (pLscMgr == NULL)
        {
            MY_ERR("GET_SHADING_PARA fail! NULL pLscMgr!");
            return CCTIF_BAD_PARAM;
        }
        const NSIspTuning::ILscTbl* pTbl = pLscMgr->getCapLut(2);
        if (pTbl == NULL)
        {
            MY_ERR("GET_SHADING_PARA fail to get table!");
            return CCTIF_BAD_PARAM;
        }
        const NSIspTuning::ILscTable::Config rCfg = pTbl->getConfig();

        pShadingPara->pShadingComp->SHADING_EN          = pLscMgr->getOnOff();
        pShadingPara->pShadingComp->SHADINGBLK_XNUM     = rCfg.rCfgBlk.i4BlkX+1;
        pShadingPara->pShadingComp->SHADINGBLK_YNUM     = rCfg.rCfgBlk.i4BlkY+1;
        pShadingPara->pShadingComp->SHADINGBLK_WIDTH    = rCfg.rCfgBlk.i4BlkW;
        pShadingPara->pShadingComp->SHADINGBLK_HEIGHT   = rCfg.rCfgBlk.i4BlkH;
        pShadingPara->pShadingComp->SHADING_RADDR       = 0;
        pShadingPara->pShadingComp->SD_LWIDTH           = rCfg.rCfgBlk.i4BlkLastW;
        pShadingPara->pShadingComp->SD_LHEIGHT          = rCfg.rCfgBlk.i4BlkLastH;
        pShadingPara->pShadingComp->SDBLK_RATIO00       = 32;
        pShadingPara->pShadingComp->SDBLK_RATIO01       = 32;
        pShadingPara->pShadingComp->SDBLK_RATIO10       = 32;
        pShadingPara->pShadingComp->SDBLK_RATIO11       = 32;

        // log nvram data
        MY_LOGD("[%s -] GET_SHADING_PARA SensorMode(%d): (%dx%d, %d,%d,%d,%d)", __FUNCTION__, eSensorMode,
            rCfg.rCfgBlk.i4BlkX, rCfg.rCfgBlk.i4BlkY, rCfg.rCfgBlk.i4BlkW, rCfg.rCfgBlk.i4BlkH,
            rCfg.rCfgBlk.i4BlkLastW, rCfg.rCfgBlk.i4BlkLastH);
        }
        break;
    case FT_CCT_OP_SET_SHADING_TABLE_POLYCOEF:
        {
        if ( ! puParaIn ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        *pu4RealParaOutLen = 0;
        ACDK_CCT_TABLE_SET_STRUCT* pShadingtabledata = reinterpret_cast<ACDK_CCT_TABLE_SET_STRUCT*>(puParaIn);

        MUINT8* pSrc = (MUINT8*)(pShadingtabledata->pBuffer);
        MUINT32 u4CtIdx = pShadingtabledata->ColorTemp;
        MUINT32 u4Size = pShadingtabledata->Length;

        MY_LOGD("[%s] + SET_SHADING_TABLE: SensorMode(%d),CT(%d),Src(%p),Size(%d)", __FUNCTION__, pShadingtabledata->Mode, u4CtIdx, pSrc, u4Size);

        if (pSrc == NULL)
        {
            MY_LOGE("SET_SHADING_TABLE: NULL pSrc");
            return CCTIF_BAD_PARAM;
        }

        if (u4CtIdx >= 4)
        {
            MY_LOGE("SET_SHADING_TABLE: Wrong CtIdx(%d)", u4CtIdx);
            return CCTIF_BAD_PARAM;
        }

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);
        const ISP_SHADING_STRUCT* pLscData = pLscNvram->getLscNvram();

        // write to nvram buffer
        MUINT32* pDst = pLscNvram->getLut(ESensorMode_Capture, u4CtIdx);
        ::memcpy(pDst, pSrc, u4Size*sizeof(MUINT32));
        // reset flow to validate ?
        pLscMgr->CCTOPReset();

        //NSIspTuning::ILscTbl rTbl(NSIspTuning::ILscTable::HWTBL);
        //rTbl.setConfig(pLscData->Width, pLscData->Height, pLscData->GridXNum, pLscData->GridYNum);
        //rTbl.setData(pSrc, u4Size*sizeof(MUINT32));
        //if (!pLscMgr->syncTbl(rTbl))
        //{
        //    MY_ERR("Fail to sync LUT to DRAM");
        //    return CCTIF_UNKNOWN_ERROR;
        //}
        MY_LOGD("[%s] - SET_SHADING_TABLE", __FUNCTION__);


        }
        break;
    }
    if(err != CCTIF_NO_ERROR)
    {
        MY_LOGE("ret = %d", err);
    }
    return err;
}


MINT32
CctHandle::
cct_GetCctOutBufSize(CCT_OP_ID op, MINT32 dtype)
{
    MINT32 idx;

    if (op >= FT_CCT_OP_NVRAM_START && op < FT_CCT_OP_END ) {
        // determine the output data size by according the op and dtype
        if ( op == FT_CCT_OP_ISP_SAVE_NVRAM_DATA || op == FT_CCT_OP_ISP_SET_NVRAM_DATA )
            return 0;
        if ( op == FT_CCT_OP_ISP_GET_NVRAM_DATA ) {
            if (dtype >= CCT_NVRAM_DATA_ENUM_MAX )
                return -2;      // data type not found
            else
                return giCctNvramCmdOutBufSize[dtype][1];
        }
        return -1;
    } else {
        // determine the output data size by according the op only
        idx=0;
        for(int i = 0; i < CCT_CMD_OUTBUF_SIZE_COUNT; i++)
        {
            if(giCctCmdOutBufSize[i][0] == op)
            {
                idx = i;
                break;
            }
        }

        if (giCctCmdOutBufSize[idx][0] == op)
            return giCctCmdOutBufSize[idx][1];
        else
            return -1;      // OP_NOT_FOUND
    }
}


MINT32
CctHandle::
cct_GetCctOutBufSize(CCT_OP_ID op, MUINT32 u4ParaInLen, MUINT8 *puParaIn)
{
    MINT32 idx;
    MINT32 dtype;

    if (op >= FT_CCT_OP_NVRAM_START && op < FT_CCT_OP_NVRAM_TYPE_MAX ) {
        // determine the output data size by according the op and dtype
        if ( puParaIn == NULL || u4ParaInLen < 4 )
            return -2;
        if ( op == FT_CCT_OP_ISP_SAVE_NVRAM_DATA || op == FT_CCT_OP_ISP_SET_NVRAM_DATA )
            return 0;
        if ( op == FT_CCT_OP_ISP_GET_NVRAM_DATA ) {
            if (u4ParaInLen >= sizeof(MINT32)) {
                dtype = *((MINT32 *)puParaIn);
                if (dtype >= CCT_NVRAM_DATA_ENUM_MAX )
                    return -2;      // data type not found
                else {
                    MY_LOGD("CCT NVRAM cmd enum[%d] Out Buffer size:%d",dtype,giCctNvramCmdOutBufSize[dtype][1]);
                    return giCctNvramCmdOutBufSize[dtype][1];
                }
            } else
                return -2;
        }
        return -1;
    } else {
        // determine the output data size by according the op only
        idx=0;
        for(int i = 0; i < CCT_CMD_OUTBUF_SIZE_COUNT; i++)
        {
            if(giCctCmdOutBufSize[i][0] == op)
            {
                idx = i;
                break;
            }
        }

        if (giCctCmdOutBufSize[idx][0] == op) {
            MY_LOGD("CCT cmd Out Buffer size:%d",giCctCmdOutBufSize[idx][1]);
            return giCctCmdOutBufSize[idx][1];
        } else
            return 0;      // for test only
            //return -1;      // OP_NOT_FOUND
    }
}


MINT32
CctHandle::
cctNvram_GetNvramData(CCT_NVRAM_DATA_T dataType, MINT32 inSize, MUINT8 *pInBuf, MINT32 outSize, void *pOutBuf, MINT32 *pRealOutSize)
{
    MINT32 err = CCTIF_NO_ERROR;
    MINT32 ret;

    MY_LOGD("[%s] dataType=%d, inSize=%d, outSize=%d", __FUNCTION__, dataType, inSize, outSize);
    switch (dataType) {
    case CCT_NVRAM_DATA_LSC_PARA:
        {
        winmo_cct_shading_comp_struct*const pShadingPara = reinterpret_cast<winmo_cct_shading_comp_struct*>(pOutBuf);

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        if (pLscMgr == NULL)
        {
            MY_ERR("GET_SHADING_PARA fail! NULL pLscMgr!");
            err = CCTIF_BAD_PARAM;
            break;
        }
        const NSIspTuning::ILscTbl* pTbl = pLscMgr->getCapLut(2);
        if (pTbl == NULL)
        {
            MY_ERR("GET_SHADING_PARA fail to get table!");
            err = CCTIF_BAD_PARAM;
            break;
        }
        const NSIspTuning::ILscTable::Config rCfg = pTbl->getConfig();

        pShadingPara->SHADING_EN          = pLscMgr->getOnOff();
        pShadingPara->SHADINGBLK_XNUM     = rCfg.rCfgBlk.i4BlkX+1;
        pShadingPara->SHADINGBLK_YNUM     = rCfg.rCfgBlk.i4BlkY+1;
        pShadingPara->SHADINGBLK_WIDTH    = rCfg.rCfgBlk.i4BlkW;
        pShadingPara->SHADINGBLK_HEIGHT   = rCfg.rCfgBlk.i4BlkH;
        pShadingPara->SHADING_RADDR       = 0;
        pShadingPara->SD_LWIDTH           = rCfg.rCfgBlk.i4BlkLastW;
        pShadingPara->SD_LHEIGHT          = rCfg.rCfgBlk.i4BlkLastH;
        pShadingPara->SDBLK_RATIO00       = 32;
        pShadingPara->SDBLK_RATIO01       = 32;
        pShadingPara->SDBLK_RATIO10       = 32;
        pShadingPara->SDBLK_RATIO11       = 32;
        *pRealOutSize = sizeof(winmo_cct_shading_comp_struct);
        }
        break;
    case CCT_NVRAM_DATA_LSC_TABLE:
        {
        CCT_SHADING_TAB_STRUCT *CctTabPtr;

        if  (sizeof(CCT_SHADING_TAB_STRUCT) != outSize || ! pRealOutSize || ! pOutBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        if  ((inSize < 4) || ! pInBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }

        CctTabPtr = (CCT_SHADING_TAB_STRUCT *) pInBuf;

        CCT_SHADING_TAB_STRUCT*const pShadingtabledata  = reinterpret_cast<CCT_SHADING_TAB_STRUCT*> (pOutBuf);
        pShadingtabledata->length= MAX_SHADING_PvwFrm_SIZE;
        pShadingtabledata->color_temperature = CctTabPtr->color_temperature;
        pShadingtabledata->offset = CctTabPtr->offset;
        pShadingtabledata->mode= CctTabPtr->mode;

        NSIspTuning::ESensorMode_T eLscScn = (NSIspTuning::ESensorMode_T)pShadingtabledata->mode;
        MUINT8* pDst = (MUINT8*)(pShadingtabledata->table);
        MUINT32 u4CtIdx = pShadingtabledata->color_temperature;
        MUINT32 u4Size = pShadingtabledata->length;

        ALOGD("[%s +] GET_SHADING_TABLE: SensorMode(%d),CT(%d),Src(%p),Size(%d)", __FUNCTION__,
            eLscScn, u4CtIdx, pDst, u4Size);

        if (pDst == NULL)
        {
            MY_ERR("GET_SHADING_TABLE: NULL pDst");
            err = CCTIF_BAD_PARAM;
            break;
        }

        if (u4CtIdx >= 4)
        {
            MY_ERR("GET_SHADING_TABLE: Wrong CtIdx(%d)", u4CtIdx);
            err = CCTIF_BAD_PARAM;
            break;
        }

        //NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);
        //const ISP_SHADING_STRUCT* pLscData = pLscNvram->getLscNvram();

        // read from nvram buffer
        MUINT32* pSrc = pLscNvram->getLut(ESensorMode_Capture, u4CtIdx);
        ::memcpy(pDst, pSrc, u4Size*sizeof(MUINT32));

        MY_LOGD("[%s -] GET_SHADING_TABLE", __FUNCTION__);
        *pRealOutSize = sizeof(CCT_SHADING_TAB_STRUCT);
        }
        break;
    case CCT_NVRAM_DATA_LSC:
        if  (sizeof(NVRAM_CAMERA_SHADING_STRUCT) != outSize || ! pRealOutSize || ! pOutBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        ::memcpy(pOutBuf, &m_rBuf_SD, sizeof(NVRAM_CAMERA_SHADING_STRUCT));
        *pRealOutSize = sizeof(NVRAM_CAMERA_SHADING_STRUCT);
        break;
    case CCT_NVRAM_DATA_AE_PLINE:
        if  (sizeof(AE_PLINETABLE_T) != outSize || ! pRealOutSize || ! pOutBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        err = NS3Av3::IAeMgr::getInstance().CCTOPAEGetPlineNVRAM(mSensorDev, (VOID *)pOutBuf, (MUINT32 *) pRealOutSize);
        break;
    case CCT_NVRAM_DATA_AE:
        if  (sizeof(AE_NVRAM_T)*CAM_SCENARIO_NUM != outSize || ! pRealOutSize || ! pOutBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        //::memcpy(pOutBuf, &(m_rBuf_3A.rAENVRAM[0]), sizeof(AE_NVRAM_T)*AE_NVRAM_IDX_NUM);
        //*pRealOutSize = sizeof(AE_NVRAM_T)*AE_NVRAM_IDX_NUM;

        //or
        err = NS3Av3::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(mSensorDev, (VOID *)pOutBuf, (MUINT32 *) pRealOutSize);

        break;
    case CCT_NVRAM_DATA_AF:
        if  (sizeof(NVRAM_LENS_PARA_STRUCT) != outSize || ! pRealOutSize || ! pOutBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        //::memcpy(pOutBuf, &m_rBuf_LN, sizeof(NVRAM_LENS_PARA_STRUCT));
        //*pRealOutSize = sizeof(NVRAM_LENS_PARA_STRUCT);

        //or
        err = NS3Av3::IAfMgr::getInstance().CCTOPAFGetNVRAMParam(mSensorDev, (VOID *)pOutBuf, (MUINT32 *) pRealOutSize);

        break;
    case CCT_NVRAM_DATA_AWB:
        if  (sizeof(AWB_NVRAM_T)*CAM_SCENARIO_NUM != outSize || ! pRealOutSize || ! pOutBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        //::memcpy(pOutBuf, &(m_rBuf_3A.rAWBNVRAM[0]), sizeof(AWB_NVRAM_T)*CCT_AWB_NVRAM_TBL_NO);
        //*pRealOutSize = sizeof(AWB_NVRAM_T)*CCT_AWB_NVRAM_TBL_NO;

        //or
        err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBGetNVRAMParam(mSensorDev, (VOID *)pOutBuf, (MUINT32 *) pRealOutSize);

        break;
    case CCT_NVRAM_DATA_ISP:
        if  (sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT) != outSize || ! pRealOutSize || ! pOutBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        ::memcpy(pOutBuf, &m_rBuf_ISP, sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
        *pRealOutSize = sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT);
        break;
	case CCT_NVRAM_DATA_FEATURE:
        {
        if  (sizeof(NVRAM_CAMERA_FEATURE_STRUCT) != outSize || ! pRealOutSize || ! pOutBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        NVRAM_CAMERA_FEATURE_STRUCT* pNvram;
        NVRAM_CAMERA_FEATURE_STRUCT* pCctNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_STRUCT*>(pOutBuf);
        err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, mSensorDev, (void*&)pNvram, 1);
        *pCctNvram = *pNvram;
        *pRealOutSize = sizeof(NVRAM_CAMERA_FEATURE_STRUCT);
        }
        break;
    case CCT_NVRAM_DATA_STROBE:
        if  (sizeof(NVRAM_CAMERA_STROBE_STRUCT) != outSize || ! pRealOutSize || ! pOutBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
#if (CAM3_3A_ISP_50_EN)
        ret = FlashMgr::getInstance(mSensorDev)->cctReadNvramToPcMeta((VOID *)pOutBuf, (MUINT32 *) pRealOutSize);
        if(ret!=0)
            err=CCTIF_UNKNOWN_ERROR;
#else
        ret = FlashMgr::getInstance().cctReadNvramToPcMeta(mSensorDev, (VOID *)pOutBuf, (MUINT32 *) pRealOutSize);
        if(ret!=1)
            err=CCTIF_UNKNOWN_ERROR;
#endif
        *pRealOutSize = sizeof(NVRAM_CAMERA_STROBE_STRUCT);
        break;
    case CCT_NVRAM_DATA_FLASH_AWB:
        if  (sizeof(FLASH_AWB_NVRAM_T) != outSize || ! pRealOutSize || ! pOutBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        ::memcpy(pOutBuf, &(m_rBuf_3A.rFlashAWBNVRAM), sizeof(FLASH_AWB_NVRAM_T));
        *pRealOutSize = sizeof(FLASH_AWB_NVRAM_T);
        break;
    default:
        MY_LOGD("Not support cmd %d", (int)dataType);
        break;
    }
    if( pRealOutSize ) {
        MY_LOGD("[%s] err=%d, pRealOutSize=%d", __FUNCTION__, err, *pRealOutSize);
    }
    return err;
}


MINT32
CctHandle::
cctNvram_SetNvramData(CCT_NVRAM_DATA_T dataType, MINT32 inSize, MUINT8 *pInBuf)
{
    MINT32 err = CCTIF_NO_ERROR;
    MINT32 ret;

    MY_LOGD("[%s] dataType=%d, inSize=%d", __FUNCTION__, dataType, inSize);
    switch (dataType) {
    case CCT_NVRAM_DATA_LSC_PARA:
        {
        if  ( sizeof(winmo_cct_shading_comp_struct) !=  inSize || ! pInBuf ) {
            err = CCTIF_BAD_PARAM;
            break;
        }

        winmo_cct_shading_comp_struct*const pShadingPara = reinterpret_cast<winmo_cct_shading_comp_struct*>(pInBuf);

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);
        ISP_SHADING_STRUCT* pLscData = pLscNvram->getLscNvram();
        const NSIspTuning::ILscTbl* pTbl = pLscMgr->getCapLut(2);
        const NSIspTuning::ILscTable::Config rCfg = pTbl->getConfig();

        // only change grid number to NVRAM, do not let HW take effect immediately.
        pLscData->GridXNum = pShadingPara->SHADINGBLK_XNUM + 1;
        pLscData->GridYNum = pShadingPara->SHADINGBLK_YNUM + 1;
        pLscData->Width    = rCfg.i4ImgWd;
        pLscData->Height   = rCfg.i4ImgHt;
        }
        break;
    case CCT_NVRAM_DATA_LSC_TABLE:
        {
        if  ( sizeof(CCT_SHADING_TAB_STRUCT) !=  inSize || ! pInBuf ) {
            err = CCTIF_BAD_PARAM;
            break;
        }

        CCT_SHADING_TAB_STRUCT*const pShadingtabledata  = reinterpret_cast<CCT_SHADING_TAB_STRUCT*> (pInBuf);

        MUINT8* pSrc = (MUINT8*)(pShadingtabledata->table);
        MUINT32 u4CtIdx = pShadingtabledata->color_temperature;
        MUINT32 u4Size = pShadingtabledata->length;

        MY_LOGD("[%s +] SET_SHADING_TABLE: SensorMode(%d),CT(%d),Src(%p),Size(%d)", __FUNCTION__,
            pShadingtabledata->mode, u4CtIdx, pSrc, u4Size);

        if (pSrc == NULL)
        {
            MY_ERR("SET_SHADING_TABLE: NULL pSrc");
            err = CCTIF_BAD_PARAM;
            break;
        }

        if (u4CtIdx >= 4)
        {
            MY_ERR("SET_SHADING_TABLE: Wrong CtIdx(%d)", u4CtIdx);
            err = CCTIF_BAD_PARAM;
            break;
        }

        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);
        //const ISP_SHADING_STRUCT* pLscData = pLscNvram->getLscNvram();

        // write to nvram buffer
        MUINT32* pDst = pLscNvram->getLut(ESensorMode_Capture, u4CtIdx);
        ::memcpy(pDst, pSrc, u4Size*sizeof(MUINT32));
        // reset flow to validate ?
        pLscMgr->CCTOPReset();
        }
        break;
    case CCT_NVRAM_DATA_LSC:
        // This case combined CCT_NVRAM_DATA_LSC_PARA and CCT_NVRAM_DATA_LSC_TABLE
        {
        if  ( sizeof(NVRAM_CAMERA_SHADING_STRUCT) !=  inSize || ! pInBuf ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        NVRAM_CAMERA_SHADING_STRUCT* const pShadingNvram  = reinterpret_cast<NVRAM_CAMERA_SHADING_STRUCT*> (pInBuf);
        // get LSC nvram
        NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);
        ISP_SHADING_STRUCT* pDst = pLscNvram->getLscNvram();
        // need to overwrite sensor real witdth, height
        NSIspTuning::ILscMgr* pLscMgr = NSIspTuning::ILscMgr::getInstance((ESensorDev_T)m_eSensorEnum);
        const NSIspTuning::ILscTbl* pTbl = pLscMgr->getCapLut(2);
        const NSIspTuning::ILscTable::Config rCfg = pTbl->getConfig();

        // write to nvram buffer
        ::memcpy(pDst, &(pShadingNvram->Shading), sizeof(ISP_SHADING_STRUCT));
        pDst->Width    = rCfg.i4ImgWd;
        pDst->Height   = rCfg.i4ImgHt;

        // reset flow to validate ?
        pLscMgr->CCTOPReset();
        }
        break;
    case CCT_NVRAM_DATA_AE_PLINE:
        if  ( sizeof(AE_PLINETABLE_T) !=  inSize || ! pInBuf ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        err = NS3Av3::IAeMgr::getInstance().CCTOPAEApplyPlineNVRAM(mSensorDev, (VOID *)pInBuf);
        break;
    case CCT_NVRAM_DATA_AE:
        {
        if  ( sizeof(AE_NVRAM_T) != (inSize-4) || ! pInBuf ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        MINT32 scenarioMode = *(MINT32 *)pInBuf;
        //err = NS3Av3::IAeMgr::getInstance().CCTOPAEApplyNVRAMParam(mSensorDev, (VOID *)pInBuf);
        err = NS3Av3::IAeMgr::getInstance().CCTOPAEApplyNVRAMParam(mSensorDev, (VOID *)(pInBuf+4), scenarioMode);
        break;
        }
    case CCT_NVRAM_DATA_AF:
        {
        if  ( sizeof(NVRAM_LENS_DATA_PARA_STRUCT) != (inSize-4) || ! pInBuf ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        MINT32 scenarioMode = *(MINT32 *)pInBuf;
        //err = NS3Av3::IAfMgr::getInstance().CCTOPAFApplyNVRAMParam(mSensorDev, (VOID *)pInBuf);
        err = NS3Av3::IAfMgr::getInstance().CCTOPAFApplyNVRAMParam(mSensorDev, (VOID *)(pInBuf+4), scenarioMode);
        break;
        }
    case CCT_NVRAM_DATA_AWB:
        {
        if  ( sizeof(AWB_NVRAM_T) != (inSize-4) || ! pInBuf ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        MINT32 scenarioMode = *(MINT32 *)pInBuf;
        //err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBApplyNVRAMParam(mSensorDev, (VOID *)pInBuf);
        err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBApplyNVRAMParam(mSensorDev, (VOID *)(pInBuf+4), scenarioMode);
        break;
        }
    case CCT_NVRAM_DATA_ISP:
        {
        // TO DO ......
        // There is no related implementation found in the previous CCT. Need to add the implementation here.
        if ( sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT) != inSize || ! pInBuf) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        //NVRAM_CAMERA_ISP_PARAM_STRUCT* pIspParam = reinterpret_cast<NVRAM_CAMERA_ISP_PARAM_STRUCT*>(pInBuf);
        //err = cctNvram_SetIspNvramData( &(pIspParam->ISPRegs) );
        memcpy((void *)&m_rBuf_ISP, (void *)pInBuf, sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
        break;
        }
	case CCT_NVRAM_DATA_FEATURE:
        {
        if  ( sizeof(NVRAM_CAMERA_FEATURE_STRUCT) !=  inSize || ! pInBuf ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        NVRAM_CAMERA_FEATURE_STRUCT* pNvram;
        NVRAM_CAMERA_FEATURE_STRUCT* pCctNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_STRUCT*>(pInBuf);

        //test only
        MUINT32 th_old;
        MUINT32 th_apply;
        MUINT32 th_new;

        err = NvBufUtil::getInstance().getBuf(CAMERA_NVRAM_DATA_FEATURE, mSensorDev, (void*&)pNvram);

        //test only
        th_old = pNvram->mfll.mfll_iso_th;
        th_apply = pCctNvram->mfll.mfll_iso_th;

        *pNvram = *pCctNvram;

        //test only
        NVRAM_CAMERA_FEATURE_STRUCT* pReadNvram;
        err = NvBufUtil::getInstance().getBuf(CAMERA_NVRAM_DATA_FEATURE, mSensorDev, (void*&)pReadNvram);
        th_new = pReadNvram->mfll.mfll_iso_th;
        MY_LOGD("apply mfll_iso_th old(%d) apply(%d) new(%d)", th_old, th_apply, th_new );
        }
        break;
    case CCT_NVRAM_DATA_STROBE:
        if  ( sizeof(NVRAM_CAMERA_STROBE_STRUCT) !=  inSize || ! pInBuf ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
#if (CAM3_3A_ISP_50_EN)
        ret = FlashMgr::getInstance(mSensorDev)->cctSetNvdataMeta(pInBuf, inSize);
        if(ret!=0)
            err=CCTIF_UNKNOWN_ERROR;
#else
        ret = FlashMgr::getInstance().cctSetNvdataMeta(mSensorDev, pInBuf, inSize);
        if(ret!=1)
            err=CCTIF_UNKNOWN_ERROR;
#endif
        break;
    case CCT_NVRAM_DATA_FLASH_AWB:
        if  ( sizeof(FLASH_AWB_NVRAM_T) !=  inSize || ! pInBuf ) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        err = NS3Av3::IAwbMgr::getInstance().CCTOPFlashAWBApplyNVRAMParam(mSensorDev, (VOID *)pInBuf);
        break;
    default:
        MY_LOGD("Not support cmd %d", (int)dataType);
        break;
    }
    MY_LOGD("[%s] err=%d", __FUNCTION__, err);
    return err;
}


MINT32
CctHandle::
cctNvram_SetPartialNvramData(CCT_NVRAM_DATA_T dataType, MINT32 inSize, MUINT32 bufOffset, MUINT8 *pInBuf)
{
    MINT32 err = CCTIF_NO_ERROR;
    MINT32 ret;

    MY_LOGD("[%s] dataType=%d, inSize=%d, bufOffset=%d", __FUNCTION__, dataType, inSize, bufOffset);
    switch (dataType) {
    case CCT_NVRAM_DATA_ISP:
        {
        MUINT8 *pIspParamBuf;
        if ( inSize <= 0 || ! pInBuf || bufOffset >= sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT)) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        if ( (inSize + bufOffset) > sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT)) {
            err = CCTIF_BAD_PARAM;
            break;
        }
        pIspParamBuf = (MUINT8 *)&m_rBuf_ISP;
        pIspParamBuf += bufOffset;
        memcpy((void *)pIspParamBuf, (void *)pInBuf, inSize);
        }
        break;
    default:
        err = CCTIF_BAD_PARAM;
        break;
    }
    MY_LOGD("[%s] err=%d", __FUNCTION__, err);
    return err;
}



MINT32
CctHandle::
cctNvram_SaveNvramData(CCT_NVRAM_DATA_T dataType)
{
    MINT32 err = CCTIF_NO_ERROR;
    MINT32 ret;

    MY_LOGD("[%s] dataType=%d", __FUNCTION__, dataType);
    switch (dataType) {
    case CCT_NVRAM_DATA_LSC_PARA:
        break;
    case CCT_NVRAM_DATA_LSC_TABLE:
        {
        NSIspTuning::ILscNvram* pLscNvram = NSIspTuning::ILscNvram::getInstance((ESensorDev_T)m_eSensorEnum);

        if (pLscNvram->writeNvramTbl()) {
            MY_LOGD("[%s] SDTBL_SAVE_TO_NVRAM OK", __FUNCTION__);
        }
        else {
            MY_ERR("SDTBL_SAVE_TO_NVRAM fail");
            err = CCTIF_UNKNOWN_ERROR;
        }
        }
        break;
    case CCT_NVRAM_DATA_AE_PLINE:
        err = NS3Av3::IAeMgr::getInstance().CCTOPAESavePlineNVRAM(mSensorDev);
        break;
    case CCT_NVRAM_DATA_AE:
        err = NS3Av3::IAeMgr::getInstance().CCTOPAESaveNVRAMParam(mSensorDev);
        break;
    case CCT_NVRAM_DATA_AF:
        err = NS3Av3::IAfMgr::getInstance().CCTOPAFSaveNVRAMParam(mSensorDev);
        break;
    case CCT_NVRAM_DATA_AWB:
        err = NS3Av3::IAwbMgr::getInstance().CCTOPAWBSaveNVRAMParam(mSensorDev);
        break;
    case CCT_NVRAM_DATA_ISP:
        MY_LOGD("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM )");

        ret = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_ISP, m_eSensorEnum);
        if  ( 0 != ret ) {
            MY_ERR("[ACDK_CCT_OP_ISP_SAVE_TO_NVRAM] write fail err=%d(0x%x)\n", ret , ret );
            err = CCTIF_UNKNOWN_ERROR;
        } else
            MY_LOGD("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM ) done");
        break;
    case CCT_NVRAM_DATA_FEATURE:
        err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_FEATURE, mSensorDev);
        break;
    case CCT_NVRAM_DATA_STROBE:
        MY_LOGD("ACDK_CCT_OP_STROBE_WRITE_NVRAM");
#if (CAM3_3A_ISP_50_EN)
        ret = FlashMgr::getInstance(mSensorDev)->cctWriteNvram();
        if(ret!=0)
            err=CCTIF_UNKNOWN_ERROR;
#else
        ret = FlashMgr::getInstance().cctWriteNvram(mSensorDev);
        if(ret!=1)
            err=CCTIF_UNKNOWN_ERROR;
#endif
        break;
    case CCT_NVRAM_DATA_FLASH_AWB:
        err = NS3Av3::IAwbMgr::getInstance().CCTOPFlashAWBSaveNVRAMParam(mSensorDev);
        break;
    default:
        MY_LOGD("Not support cmd %d", (int)dataType);
        break;
    }
    MY_LOGD("[%s] err=%d", __FUNCTION__, err);
    return err;
}


MINT32
CctHandle::
cctNvram_SetIspNvramData(ISP_NVRAM_REGISTER_STRUCT *pIspRegs)
{
    MINT32 index;

    index = pIspRegs->Idx.SL2F;
    if (index < NVRAM_SL2F_TBL_NUM) {
        m_rISPRegs.SL2F[index] = (pIspRegs->SL2F[index]);
        m_rISPRegsIdx.SL2F = static_cast<MUINT8>(index);
        ISP_MGR_SL2F_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->SL2F[index]));
        ISP_MGR_SL2G_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->SL2F[index]));
    }

    index = pIspRegs->Idx.DBS;
    if (index < NVRAM_DBS_TBL_NUM) {
        m_rISPRegs.DBS[index] = (pIspRegs->DBS[index]);
        m_rISPRegsIdx.DBS = static_cast<MUINT8>(index);
        ISP_MGR_DBS_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->DBS[index]));
        ISP_MGR_DBS2_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->DBS[index]));
    }

    index = pIspRegs->Idx.OBC;
    if (index < NVRAM_OBC_TBL_NUM) {
        m_rISPRegs.OBC[index] = (pIspRegs->OBC[index]);
        m_rISPRegsIdx.OBC = static_cast<MUINT8>(index);
        ISP_MGR_OBC_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->OBC[index]));
        ISP_MGR_OBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->OBC[index]));
    }

    index = pIspRegs->Idx.BNR_BPC;
    if (index < NVRAM_BPC_TBL_NUM) {
        m_rISPRegs.BNR_BPC[index] = (pIspRegs->BNR_BPC[index]);
        m_rISPRegsIdx.BNR_BPC = static_cast<MUINT8>(index);
        ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->BNR_BPC[index]));
        ISP_MGR_BNR2_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->BNR_BPC[index]));
    }

    index = pIspRegs->Idx.BNR_NR1;
    if (index < NVRAM_NR1_TBL_NUM) {
        m_rISPRegs.BNR_NR1[index] = (pIspRegs->BNR_NR1[index]);
        m_rISPRegsIdx.BNR_NR1 = static_cast<MUINT8>(index);
        ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->BNR_NR1[index]));
        ISP_MGR_BNR2_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->BNR_NR1[index]));
    }

    index = pIspRegs->Idx.BNR_PDC;
    if (index < NVRAM_PDC_TBL_NUM) {
        m_rISPRegs.BNR_PDC[index] = (pIspRegs->BNR_PDC[index]);
        m_rISPRegsIdx.BNR_PDC = static_cast<MUINT8>(index);
        ISP_MGR_BNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->BNR_PDC[index]));
        ISP_MGR_BNR2_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->BNR_PDC[index]));
    }
#if 0
    index = pIspRegs->Idx.RMM;
    if (index < NVRAM_RMM_TBL_NUM) {
        m_rISPRegs.RMM[index] = (pIspRegs->RMM[index]);
        m_rISPRegsIdx.RMM = static_cast<MUINT8>(index);
        ISP_MGR_RMM_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->RMM[index]));
        ISP_MGR_RMM2_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->RMM[index]));
    }
#endif
    index = pIspRegs->Idx.RNR;
    if (index < NVRAM_RNR_TBL_NUM) {
        m_rISPRegs.RNR[index] = (pIspRegs->RNR[index]);
        m_rISPRegsIdx.RNR = static_cast<MUINT8>(index);
        ISP_MGR_RNR_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->RNR[index]));
    }

    index = pIspRegs->Idx.SL2;
    if (index < NVRAM_SL2_TBL_NUM) {
        m_rISPRegs.SL2[index] = (pIspRegs->SL2[index]);
        m_rISPRegsIdx.SL2 = static_cast<MUINT8>(index);
        ISP_MGR_SL2_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->SL2[index]));
    }

    index = pIspRegs->Idx.UDM;
    if (index < NVRAM_UDM_TBL_NUM) {
        m_rISPRegs.UDM[index] = (pIspRegs->UDM[index]);
        m_rISPRegsIdx.UDM = static_cast<MUINT8>(index);
        ISP_MGR_UDM_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->UDM[index]));
    }

#if 0
    index = pIspRegs->Idx.CCM;
    if (index < NVRAM_CCM_TBL_NUM) {
        m_rISPRegs.CCM[index] = (pIspRegs->CCM[index]);
        m_rISPRegsIdx.CCM = static_cast<MUINT8>(index);
        ISP_MGR_CCM_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->CCM[index]));
    }

    index = pIspRegs->Idx.GGM;
    if (index < NVRAM_GGM_TBL_NUM) {
        m_rISPRegs.GGM[index] = (pIspRegs->GGM[index]);
        m_rISPRegsIdx.GGM = static_cast<MUINT8>(index);
        ISP_MGR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->GGM[index]));
    }

    index = pIspRegs->Idx.IHDR_GGM;
    if (index < NVRAM_IHDR_GGM_TBL_NUM) {
        m_rISPRegs.IHDR_GGM[index] = (pIspRegs->IHDR_GGM[index]);
        m_rISPRegsIdx.IHDR_GGM = static_cast<MUINT8>(index);
        //
        // Need to check how to apply the IHDR_GGM setting to the hardware. There is no ISP_MGR_IHDR_GGM_T existed.
        //ISP_MGR_IHDR_GGM_T::getInstance((ESensorDev_T)m_eSensorEnum).put(&(pIspRegs->IHDR_GGM[index]));     //????????
    }
#endif

    index = pIspRegs->Idx.ANR;
    if (index < NVRAM_ANR_TBL_NUM) {
        m_rISPRegs.ANR[index] = (pIspRegs->ANR[index]);
        m_rISPRegsIdx.ANR = static_cast<MUINT8>(index);
        ISP_MGR_NBC_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->ANR[index]));
    }

    index = pIspRegs->Idx.ANR2;
    if (index < NVRAM_ANR2_TBL_NUM) {
        m_rISPRegs.ANR2[index] = (pIspRegs->ANR2[index]);
        m_rISPRegsIdx.ANR2 = static_cast<MUINT8>(index);
        ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->ANR2[index]));
    }

    index = pIspRegs->Idx.CCR;
    if (index < NVRAM_CCR_TBL_NUM) {
        m_rISPRegs.CCR[index] = (pIspRegs->CCR[index]);
        m_rISPRegsIdx.CCR = static_cast<MUINT8>(index);
        ISP_MGR_NBC2_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->CCR[index]));
    }

    index = pIspRegs->Idx.HFG;
    if (index < NVRAM_HFG_TBL_NUM) {
        m_rISPRegs.HFG[index] = (pIspRegs->HFG[index]);
        m_rISPRegsIdx.HFG = static_cast<MUINT8>(index);
        ISP_MGR_HFG_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->HFG[index]));
    }

    index = pIspRegs->Idx.EE;
    if (index < NVRAM_EE_TBL_NUM) {
        m_rISPRegs.EE[index] = (pIspRegs->EE[index]);
        m_rISPRegsIdx.EE = static_cast<MUINT8>(index);
        ISP_MGR_SEEE_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->EE[index]));
    }

    index = pIspRegs->Idx.MFB;
    if (index < NVRAM_MFB_TBL_NUM) {
        m_rISPRegs.MFB[index] = (pIspRegs->MFB[index]);
        m_rISPRegsIdx.MFB = static_cast<MUINT8>(index);
        ISP_MGR_MFB_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->MFB[index]));
    }

    index = pIspRegs->Idx.MIXER3;
    if (index < NVRAM_MIXER3_TBL_NUM) {
        m_rISPRegs.MIXER3[index] = (pIspRegs->MIXER3[index]);
        m_rISPRegsIdx.MIXER3 = static_cast<MUINT8>(index);
        ISP_MGR_MIXER3_T::getInstance((ESensorDev_T)m_eSensorEnum).put((pIspRegs->MIXER3[index]));
    }

    NSIspTuningv3::IspTuningMgr::getInstance().forceValidate((MINT32)m_eSensorEnum);

    MY_LOGD("[%s] done ", __FUNCTION__);
    return  CCTIF_NO_ERROR;
}

