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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "ImgSensorDrv"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/ioctl.h>

#include <utils/threads.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>

#include <camera_custom_imgsensor_cfg.h>
#include <mtkcam/utils/std/ULog.h>
#include "imgsensor_drv.h"
#include "kd_imgsensor.h"

#ifndef USING_MTK_LDVT
#define LOG_MSG(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    CAM_ULOGMD("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    CAM_ULOGME("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#else
#include "uvvf.h"

#if 1
#define LOG_MSG(fmt, arg...)    VV_MSG("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_MSG("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define LOG_MSG(fmt, arg...)
#define LOG_WRN(fmt, arg...)
#define LOG_ERR(fmt, arg...)
#endif
#endif

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

/******************************************************************************
 *
 ******************************************************************************/
ImgSensorDrv*
ImgSensorDrv::
getInstance(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    static ImgSensorDrv inst[IMGSENSOR_SENSOR_IDX_MAX_NUM];

    if(sensorIdx < IMGSENSOR_SENSOR_IDX_MAX_NUM && sensorIdx >= IMGSENSOR_SENSOR_IDX_MIN_NUM)
        return &inst[sensorIdx];
    else
        return NULL;
}

/*******************************************************************************
*
********************************************************************************/
ImgSensorDrv::
ImgSensorDrv()
{
    m_LineTimeInus = 31;
    m_MinFrameLength = 0;
    m_fd = -1;
    m_user = 0;
    m_pCustomCfg = NULL;
    m_sensorIdx = IMGSENSOR_SENSOR_IDX_NONE;
    memset(&m_drvInfo, 0, sizeof(SENSORDRV_INFO_STRUCT));
}

/*******************************************************************************
*
********************************************************************************/
ImgSensorDrv::
~ImgSensorDrv()
{
    LOG_MSG("[~ImgSensorDrv]");
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::searchSensor()
{
    if (m_list.id != 0) {
        LOG_MSG("[searchSensor] Already processed");
        return SENSOR_ALREADY_SEARCH;
    }

    LOG_MSG("searchSensor idx = %d", m_sensorIdx);

    featureControl(SENSOR_FEATURE_SET_DRIVER, (MUINT8 *)&m_list, sizeof(IMGSENSOR_SENSOR_LIST));

    if(m_list.id == 0) {
        LOG_WRN("Search fail");
        m_list.id = 0xFFFFFFFF;
        return SENSOR_INVALID_DRIVER;
    } else {
        LOG_MSG("found <%x/%s>", m_list.id, m_list.name);
        getInfo();
        return SENSOR_NO_ERROR;
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::init(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    char cBuf[64];

    LOG_MSG("[init] imgsensor_drv");
    Mutex::Autolock _l(mLock);

    if (m_fd == -1) {
        sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
        if ((m_fd = ::open(cBuf, O_RDWR)) < 0) {
            LOG_ERR("[init]: error opening %s: %s", cBuf, strerror(errno));
            return SENSOR_INVALID_DRIVER;
        }
    }

    m_sensorIdx = sensorIdx;
    m_pCustomCfg = NSCamCustomSensor::getCustomConfig(sensorIdx);

    if (m_pCustomCfg == NULL) {
        LOG_ERR("Invalid custom configuration");
        return SENSOR_UNKNOWN_ERROR;
    }

    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::uninit()
{
    LOG_MSG("[uninit] imgsensor_drv");
    Mutex::Autolock _l(mLock);

    if (m_fd > 0) {
        ::close(m_fd);
        m_fd = -1;
    }

    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::setScenario(MSDK_SCENARIO_ID_ENUM scenarioID, MUINT32 framerate, MUINT32 hdrMode, MUINT32 pdafMode)
{
    MINT32 ret = SENSOR_NO_ERROR;

    MUINT32 i = 0;
    ACDK_SENSOR_EXPOSURE_WINDOW_STRUCT ImageWindow;
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;

    ACDK_SENSOR_CONTROL_STRUCT sensorCtrl;

    MUINT16 pFeaturePara16[2];
    MUINT32 FeaturePara32;

    //set sensor preview/capture mode
    sensorCtrl.InvokeCamera = m_sensorIdx;
    sensorCtrl.ScenarioId = scenarioID;
    sensorCtrl.pImageWindow = &ImageWindow;
    sensorCtrl.pSensorConfigData = &SensorConfigData;

    ret = featureControl(SENSOR_FEATURE_SET_FRAMERATE, (MUINT8*)&framerate, sizeof(MUINTPTR));
    if (ret < 0) {
         LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_FRAMERATE error");
         return SENSOR_UNKNOWN_ERROR;
    }

    ret = featureControl(SENSOR_FEATURE_SET_HDR, (MUINT8*)&hdrMode, sizeof(MUINTPTR));
    if (ret < 0) {
         LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_HDR error");
         return SENSOR_UNKNOWN_ERROR;
    }

    ret = featureControl(SENSOR_FEATURE_SET_PDAF, (MUINT8*)&pdafMode, sizeof(MUINTPTR));
    if (ret < 0) {
         LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_PDAF error");
         return SENSOR_UNKNOWN_ERROR;
    }

    if ((ret = ioctl(m_fd, KDIMGSENSORIOC_X_CONTROL , &sensorCtrl)) < 0) {
        LOG_ERR("[setScenario]Err-ctrlCode (%s)", strerror(errno));
        return -errno;
    }
    //get exposure line time for each scenario
    ret = featureControl(SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ, (MUINT8*)&FeaturePara32, sizeof(MUINTPTR));
    if (ret < 0) {
       LOG_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error");
       return SENSOR_UNKNOWN_ERROR;
    }

    ret = featureControl(SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16, sizeof(pFeaturePara16));
    if (ret < 0) {
        LOG_ERR("[setScenario]: SENSOR_FEATURE_GET_PERIOD error");
        return SENSOR_UNKNOWN_ERROR;
    }

    if (FeaturePara32) {
        if(FeaturePara32 >= 1000) {
            m_LineTimeInus = (MUINT32)(((MUINT64)pFeaturePara16[0] * 1000000 + ((FeaturePara32 / 1000) - 1)) / (FeaturePara32 / 1000));   // 1000 base , 33657 mean 33.657 us
            m_MinFrameLength = (MUINT32)pFeaturePara16[1];
        }
        else {
            LOG_ERR("[setScenario]: Sensor clock too slow = %d %d", FeaturePara32, pFeaturePara16[0]);
        }

        LOG_MSG("[setScenario]DevID = %d, m_LineTimeInus = %d Scenario id = %d, PixelClk = %d, PixelInLine = %d, Framelength = %d",
            m_sensorIdx, m_LineTimeInus, scenarioID, FeaturePara32, pFeaturePara16[0], pFeaturePara16[1]);
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::sendCommand(
    MUINT32 cmd,
    MUINTPTR arg1,
    MUINTPTR arg2,
    MUINTPTR arg3
)
{
    // parg#: pointer to paras, regard pointed variable as MUINT32 type
    /// Notice: Regard the type of varibale used in middleware to be MUINT32
    /// Should discuss this part with middleware when you need to create a new sendCommand enum
    MUINT32* parg1 = (MUINT32*)arg1;
    MUINT32* parg2 = (MUINT32*)arg2;
    MUINT32* parg3 = (MUINT32*)arg3;

    // parg#Addr: pointer to an adress. Regard pointed variable as MUINTPTR type, cause it's a pointer address.
    //MUINTPTR* parg1Addr = (MUINTPTR*)parg1;
    MUINTPTR* parg2Addr = (MUINTPTR*)parg2;
    MUINTPTR* parg3Addr = (MUINTPTR*)parg3;

    MINT32 err = SENSOR_NO_ERROR;

    ACDK_SENSOR_FEATURE_ENUM FeatureId = SENSOR_FEATURE_BEGIN;
    MUINT8 *pFeaturePara = NULL; // Pointer to feature data that communicate with kernel
    MUINT32 FeatureParaLen = 0; // The length of feature data

    MUINT64 FeaturePara[4] = {0}; // Convert input paras to match the format used in kernel
    MUINT64 u8temp = 0;
    MUINT32 u4temp = 0;
    MUINT8 i;
    float ratio[3] = {0};
    bool cmdBeforePowerOn = false;
    SENSOR_VC_INFO2_STRUCT vcInfo2;
    SensorVCInfo2* pInfo2 = NULL;
    SENSOR_VC_INFO_STRUCT vcInfo;//for legacy driver compatible
    MUINT scenarioId = 0;
    SINGLE_VC_INFO2 vc[5];
    NSCam::IHalSensor::ConfigParam *pConfigParam = NULL;

//#define SENDCMD_LOG // Open sendcommand log, for test only
    switch (cmd) {
    case CMD_SENSOR_SET_DRIVE_CURRENT:
    {
        FeatureId = SENSOR_FEATURE_SET_MCLK_DRIVE_CURRENT;
        std::string str_prop("vendor.debug.seninf.mclk");
        str_prop += std::to_string(m_sensorIdx);
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get(str_prop.c_str(), value, "-1");
        int inputCurrent = atoi(value);

        FeaturePara[0] = 0;
        if (inputCurrent >= ISP_DRIVING_MAX_NUM || inputCurrent < ISP_DRIVING_2MA)
            FeaturePara[0] = *parg1;
        else
            FeaturePara[0] = inputCurrent;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_SET_MCLK_DRIVE_CURRENT] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    }
    case CMD_SENSOR_SET_SENSOR_EXP_TIME:
        u8temp = *parg1;/*64bit extend */
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        FeaturePara[0] = ((1000 * (u8temp)) / m_LineTimeInus);
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d", (long long)FeaturePara[0], m_LineTimeInus);
            FeaturePara[0] = 1;
        }
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_EXP_LINE:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_EXP_LINE] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_GAIN:
        FeatureId = SENSOR_FEATURE_SET_GAIN;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[0] >>= 4;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_GAIN] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_DUAL_GAIN:
        FeatureId = SENSOR_FEATURE_SET_DUAL_GAIN;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[0] >>= 4;
        FeaturePara[1] = *parg2; //from 10b to 6b base
        FeaturePara[1] >>= 4;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_DUAL_GAIN] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_FLICKER_FRAME_RATE:
        FeatureId = SENSOR_FEATURE_SET_AUTO_FLICKER_MODE;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_SET_VIDEO_FRAME_RATE:
        FeatureId = SENSOR_FEATURE_SET_VIDEO_MODE;
        FeaturePara[0]= *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_SYNC;

        FeaturePara[0] = *parg1; // RAW Gain R, Gr
        FeaturePara[1] = *(parg1+1);  // RAW Gain Gb, B
        FeaturePara[2] = *(parg1+2);  // Exposure time
        FeaturePara[2] = ((1000 * FeaturePara[2]) / m_LineTimeInus);
        if(FeaturePara[2]  == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_SENSOR_SYNC] LineTimeInus = %llu %d", (long long)FeaturePara[2] , m_LineTimeInus);
            FeaturePara[2]  = 1;
        }
        FeaturePara[2] = (FeaturePara[2] ) | (((MUINT64)(*(parg1+3))>>4) << 16); // Sensor gain from 10b to 6b base
        FeaturePara[3] = *(parg1+4);  // Delay frame cnt
        FeatureParaLen = sizeof(MUINT64) * 4;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_CCT_FEATURE_CONTROL:
        FeatureId = (ACDK_SENSOR_FEATURE_ENUM)*parg1;
        pFeaturePara = (MUINT8*)parg2;
        FeatureParaLen = (MUINT32)*parg3;
        break;

    case CMD_SENSOR_SET_SENSOR_CALIBRATION_DATA:
        FeatureId = SENSOR_FEATURE_SET_CALIBRATION_DATA;
        pFeaturePara = (MUINT8*)parg1;
        FeatureParaLen = sizeof(SET_SENSOR_CALIBRATION_DATA_STRUCT);
        break;

    case CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO:
        FeatureId = SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_TEST_PATTERN_OUTPUT:
        FeatureId = SENSOR_FEATURE_SET_TEST_PATTERN;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_TEST_PATTERN_OUTPUT] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

#ifdef CONFIG_MTK_CAM_SECURE
    case CMD_SENSOR_SET_AS_SECURE_DRIVER:
        FeatureId = SENSOR_FEATURE_SET_AS_SECURE_DRIVER;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;


#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_AS_SECURE_DRIVER] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
#endif

    case CMD_SENSOR_SET_ESHUTTER_GAIN:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER_GAIN;
        FeaturePara[0] = *parg1; // exposure time (us)
        FeaturePara[0] = ((1000 * FeaturePara[0] ) / m_LineTimeInus);

        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_ESHUTTER_GAIN] m_LineTime = %llu %d", (long long)FeaturePara[0] , m_LineTimeInus);
            FeaturePara[0]  = 1;
        }
        FeaturePara[2] = (FeaturePara[0] ) | (((MUINT64)(*(parg1+1))>>4) << 16); // Sensor gain from 10b to 6b base
        FeaturePara[0] = 0; // RAW Gain R, Gr
        FeaturePara[1] = 0;  // RAW Gain Gb, B
        FeaturePara[3] = 0;  // Delay frame cnt
        LOG_MSG("CMD_SENSOR_SET_ESHUTTER_GAIN: Exp=%d, SensorGain=%d", (MUINT32)FeaturePara[2]&0x0000FFFF, (MUINT32)FeaturePara[2]>>16);
        FeatureParaLen = sizeof(MUINT64) * 4;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_ESHUTTER_GAIN] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
         break;

    case CMD_SENSOR_SET_OB_LOCK:
        FeatureId = SENSOR_FEATURE_SET_OB_LOCK;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        break;

    case CMD_SNESOR_SET_SENSOR_OTP_AWB_CMD:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_OTP_AWB_CMD;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        break;

    case CMD_SNESOR_SET_SENSOR_OTP_LSC_CMD:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_OTP_LSC_CMD;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        break;
    /*Set shuttter and fraem time at the same time*/
    case CMD_SENSOR_SET_SENSOR_EXP_FRAME_TIME:
        FeatureId = SENSOR_FEATURE_SET_SHUTTER_FRAME_TIME;
        FeaturePara[0] = ((1000 * (MUINT64)(*parg1)) / m_LineTimeInus) + ((1000 * (MUINT64)(*parg1))%m_LineTimeInus ?1 :0);
        FeaturePara[1] = ((1000 * (MUINT64)(*parg2)) / m_LineTimeInus) + ((1000 * (MUINT64)(*parg1))%m_LineTimeInus ?1 :0);

        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] LineTimeInus = %llu %d", (long long)FeaturePara[0], m_LineTimeInus);
            FeaturePara[0] = 1;
        }
        if(FeaturePara[1] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] LineTimeInus = %llu %d", (long long)FeaturePara[1], m_LineTimeInus);
            FeaturePara[1] = 1;
        }
        LOG_MSG("[SET_SENSOR_EXP_FRAME_TIME] ExpTime/FrmTime=(%u/%u) ExpLine/FrmLength/LineTimeInus/MinFrmLength=(%llu/%llu/%u/%u)", \
        *parg1, *parg2, (long long)FeaturePara[0], (long long)FeaturePara[1], m_LineTimeInus, m_MinFrameLength);

        if(m_MinFrameLength > FeaturePara[1]) {
            LOG_ERR("[SET_SENSOR_EXP_FRAME_TIME] Error: Set framelength (%llu) is smaller than MinFrmLength (%u)!!!", (long long)FeaturePara[1], m_MinFrameLength);
            err = SENSOR_INVALID_PARA;
            return err;
        }
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE:
        u8temp = *parg1; /*64bit extend */
        FeatureId = SENSOR_FEATURE_SET_SHUTTER_BUF_MODE;

        FeaturePara[0] = ((1000 * (u8temp)) / m_LineTimeInus);
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE] LineTimeInus = %llu %d", (long long)FeaturePara[0], m_LineTimeInus);
            FeaturePara[0] = 1;
        }
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
    break;

    case CMD_SENSOR_SET_SENSOR_EXP_LINE_BUF_MODE:
        FeatureId = SENSOR_FEATURE_SET_SHUTTER_BUF_MODE;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_ERR("[CMD_SENSOR_SET_SENSOR_EXP_LINE_BUF_MODE] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_GAIN_BUF_MODE:
        FeatureId = SENSOR_FEATURE_SET_GAIN_BUF_MODE;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[0] >>= 4;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_GAIN_BUF_MODE] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_I2C_BUF_MODE_EN:
        FeatureId = 	SENSOR_FEATURE_SET_I2C_BUF_MODE_EN;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_I2C_BUF_MODE_EN] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_STREAMING_SUSPEND:
        FeatureId = SENSOR_FEATURE_SET_STREAMING_SUSPEND;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        //LOG_MSG("SET_STREAMING_SUSPEND, senDev(%d)", sensorIdx);
        break;

    case CMD_SENSOR_SET_STREAMING_RESUME:
        FeatureId = SENSOR_FEATURE_SET_STREAMING_RESUME;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_MCLK_CONNECTION:
        *parg1 = m_pCustomCfg->mclk;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_MCLK_CONNECTION] *parg1 = %d", (MUINT32)*parg1);
#endif
        break;

    case CMD_SENSOR_GET_MIPI_SENSOR_PORT:
        *parg1 = m_pCustomCfg->port;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_MIPI_SENSOR_PORT]");
#endif
        break;

    case CMD_SENSOR_GET_INPUT_BIT_ORDER:
        *parg1 = m_pCustomCfg->bitOrder;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_INPUT_BIT_ORDER] *parg1 = %d", (MUINT32)*parg1);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE:
        *parg1 = m_pCustomCfg->orientation;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE] *parg1 = %d", (MUINT32)*parg1);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_FACING_DIRECTION:
        *parg1 = m_pCustomCfg->dir;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_FACING_DIRECTION] *parg1 = %d", (MUINT32)*parg1);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_VIEWANGLE:
        *parg1 = m_pCustomCfg->horizontalFov;
        *parg2 = m_pCustomCfg->verticalFov;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_FACING_DIRECTION] *parg1 = %d", (MUINT32)*parg1);
#endif
        break;

    case CMD_SENSOR_GET_PIXEL_CLOCK_FREQ:
        FeatureId = SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_PIXEL_CLOCK_FREQ] parg1 = %p", parg1);
#endif
        break;

    case CMD_SENSOR_GET_AE_EFFECTIVE_FRAME_FOR_LE:
        FeatureId = SENSOR_FEATURE_GET_AE_EFFECTIVE_FRAME_FOR_LE;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_AE_FRAME_MODE_FOR_LE:
        FeatureId = SENSOR_FEATURE_GET_AE_FRAME_MODE_FOR_LE;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM:
        FeatureId = SENSOR_FEATURE_GET_PERIOD;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO:
        FeatureId = SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0;
        FeaturePara[2] = parg3 != NULL ?(MUINT32)*parg3 :0;
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
        break;
    case CMD_SENSOR_GET_OFFSET_TO_START_OF_EXPOSURE:
        FeatureId = SENSOR_FEATURE_GET_OFFSET_TO_START_OF_EXPOSURE;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0;
        FeaturePara[2] = parg3 != NULL ?(MUINT32)*parg3 :0;
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
        break;


    case CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM_BY_SCENARIO:
        FeatureId = SENSOR_FEATURE_GET_PERIOD_BY_SCENARIO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0;
        FeaturePara[2] = parg3 != NULL ?(MUINT32)*parg3 :0;
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        FeatureId = SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %lu, FeaturePara[1] = %lx, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
        LOG_MSG("FeaturePara %d",(FeaturePara[1]));
        LOG_MSG("framerate = %d",(MUINT32)(*parg2));
#endif
        break;

    case CMD_SENSOR_GET_TEST_PATTERN_CHECKSUM_VALUE:
        FeatureId = SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_TEMPERATURE_VALUE:
        FeatureId = SENSOR_FEATURE_GET_TEMPERATURE_VALUE;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO:
        FeatureId = SENSOR_FEATURE_GET_CROP_INFO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = (MUINTPTR)parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_VC_INFO2:
        memset(&vcInfo2, 0, sizeof(SENSOR_VC_INFO2_STRUCT));
        FeatureId = SENSOR_FEATURE_GET_VC_INFO2;
        FeaturePara[0] = *parg2;
        FeaturePara[1] = (MUINTPTR)&vcInfo2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#if 1
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_VC_INFO2] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_VC_INFO:
        FeatureId = SENSOR_FEATURE_GET_VC_INFO;
        FeaturePara[0] = *parg2;
        FeaturePara[1] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_VC_INFO] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_PDAF_INFO:
        FeatureId = SENSOR_FEATURE_GET_PDAF_INFO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = (MUINTPTR)parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_GET_PDAF_INFO]%lu %lu",FeaturePara[0],FeaturePara[1]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_PDAF_REG_SETTING:
		FeatureId = SENSOR_FEATURE_GET_PDAF_REG_SETTING;
		FeaturePara[0] = *parg1; //size of buff in Byte
		FeaturePara[1] = (MUINTPTR)(*parg2Addr);//the address of pointer pointed
		FeatureParaLen = sizeof(MUINT64) * 2;
		pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
		LOG_MSG("[CMD_SENSOR_GET_SENSOR_PDAF_REG_SETTING] 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1]);
#endif
		break;
    case CMD_SENSOR_SET_SENSOR_PDAF_REG_SETTING:
		FeatureId = SENSOR_FEATURE_SET_PDAF_REG_SETTING;
		FeaturePara[0] = *parg1; //size of buff in Byte
		FeaturePara[1] = (MUINTPTR)(*parg2Addr);//the address of pointer pointed
		FeatureParaLen = sizeof(MUINT64) * 2;
		pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
		LOG_MSG("[CMD_SENSOR_SET_SENSOR_PDAF_REG_SETTING] 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1]);
#endif
		break;
    case CMD_SET_PDFOCUS_AREA:
		FeatureId = SENSOR_FEATURE_SET_PDFOCUS_AREA;
		FeaturePara[0] = *parg1;	//offset
		FeaturePara[1] = *parg2;	//the address of pointer pointed
		FeatureParaLen = sizeof(MUINT64) * 2;
		pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
		LOG_MSG("[CMD_SET_PDFOCUS_AREA] 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1]);
#endif
		break;
    case CMD_SENSOR_GET_PDAF_DATA:
        FeatureId = SENSOR_FEATURE_GET_PDAF_DATA;
        FeaturePara[0] = *parg1;//offset
        FeaturePara[1] = (MUINTPTR)(*parg2Addr);//the address of pointer pointed
        FeaturePara[2] = *parg3;//size of buff
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_PDAF_DATA]0x%llu 0x%llu 0x%lu", (long long)FeaturePara[0], (long long)FeaturePara[1], FeaturePara[2]);
#endif
        break;
case CMD_SENSOR_GET_4CELL_DATA:
        FeatureId = SENSOR_FEATURE_GET_4CELL_DATA;
        FeaturePara[0] = *parg1;//4CELL_CAL_TYPE_ENUM
        FeaturePara[1] = (MUINTPTR)(*parg2Addr);//the address of pointer
        FeaturePara[2] = *parg3;//size of buff
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_4CELL_DATA]0x%llu 0x%llu 0x%lu", (long long)FeaturePara[0], (long long)FeaturePara[1], FeaturePara[2]);
#endif
        break;

    case CMD_SENSOR_GET_MIPI_PIXEL_RATE:
        FeatureId = SENSOR_FEATURE_GET_MIPI_PIXEL_RATE;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0;
        FeaturePara[2] = parg3 != NULL ?(MUINT32)*parg3 :0;
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_MIPI_PIXEL_RATE]0x%llu", (long long)FeaturePara[0]);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_HDR_CAPACITY:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY] FeaturePara[0] = %lu, FeaturePara[1] = %lx, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY] FeaturePara[0] = %lu, FeaturePara[1] = %lx, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_SENSOR_AWB_GAIN:
        FeatureId = SENSOR_FEATURE_SET_AWB_GAIN;
        pFeaturePara = (MUINT8*)parg1;
        FeatureParaLen = sizeof(SET_SENSOR_AWB_GAIN);
        break;

    case CMD_SENSOR_SET_YUV_FEATURE_CMD:
        FeatureId = SENSOR_FEATURE_SET_YUV_CMD;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_YUV_FEATURE_CMD] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_YUV_SINGLE_FOCUS_MODE:
        FeatureId = SENSOR_FEATURE_SINGLE_FOCUS_MODE;
        //LOG_MSG("CMD_SENSOR_SINGLE_FOCUS_MODE");
        break;


    case CMD_SENSOR_SET_YUV_CANCEL_AF:
        FeatureId = SENSOR_FEATURE_CANCEL_AF;
        //LOG_MSG("CMD_SENSOR_CANCEL_AF");
        break;

    case CMD_SENSOR_SET_YUV_CONSTANT_AF:
        FeatureId = SENSOR_FEATURE_CONSTANT_AF;
        break;

    case CMD_SENSOR_SET_YUV_INFINITY_AF:
        FeatureId = SENSOR_FEATURE_INFINITY_AF;
        break;

    case CMD_SENSOR_SET_YUV_AF_WINDOW:
        FeatureId = SENSOR_FEATURE_SET_AF_WINDOW;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("zone_addr=0x%lx", FeaturePara[0]);
        break;

    case CMD_SENSOR_SET_YUV_AE_WINDOW:
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("AEzone_addr=0x%lx", FeaturePara[0]);
        break;

    case CMD_SENSOR_SET_YUV_GAIN_AND_EXP_LINE:
        FeatureId = SENSOR_FEATURE_SET_GAIN_AND_ESHUTTER;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_YUV_FEATURE_CMD] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_YUV_3A_CMD:
        FeatureId = SENSOR_FEATURE_SET_YUV_3A_CMD;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("YUV_3A_CMD=0x%x", (MUINT32)*parg1);
        break;

    case CMD_SENSOR_GET_YUV_AF_STATUS:
        FeatureId = SENSOR_FEATURE_GET_AF_STATUS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AF_STATUS,parg1=0x%x,FeatureParaLen=0x%x,pFeaturePara=0x%x",
        //parg1, FeatureParaLen, pFeaturePara);
        break;

    case CMD_SENSOR_GET_YUV_AE_STATUS:
        FeatureId = SENSOR_FEATURE_GET_AE_STATUS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_YUV_AWB_STATUS:
        FeatureId = SENSOR_FEATURE_GET_AWB_STATUS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_YUV_EV_INFO_AWB_REF_GAIN:
        FeatureId = SENSOR_FEATURE_GET_EV_AWB_REF;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("p_ref=0x%x", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN:
        FeatureId = SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("p_cur=0x%x", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS:
        FeatureId = SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AF_MAX_NUM_FOCUS_AREAS,p_cur=0x%x", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS:
        FeatureId = SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AE_MAX_NUM_METERING_AREAS,p_cur=0x%x", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_EXIF_INFO:
        FeatureId = SENSOR_FEATURE_GET_EXIF_INFO;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("EXIF_addr=0x%x", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_DELAY_INFO:
        FeatureId = SENSOR_FEATURE_GET_DELAY_INFO;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;
    case CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO:
        FeatureId = SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
        break;
    case CMD_SENSOR_GET_YUV_AE_FLASHLIGHT_INFO:
        FeatureId = SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("FLASHLIGHT_INFO=0x%x", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO:
        FeatureId = SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("TRIGGER_FLASHLIGHT=0x%x", (MUINT32)parg1);
        break;
    case CMD_SENSOR_SET_YUV_AUTOTEST:
        FeatureId = SENSOR_FEATURE_AUTOTEST_CMD;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
        break;
    case CMD_SENSOR_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;
    case CMD_SENSOR_SET_N3D_I2C_STREAM_REGDATA:
        FeatureId = SENSOR_FEATURE_SET_N3D_I2C_STREAM_REGDATA;
        //FeatureParaLen = sizeof(MUINT32)*4;   //writeI2CID, writeAddr, writePara, writeByte ==> Check this, why need * 4?
        FeatureParaLen = sizeof(MUINTPTR);
        pFeaturePara = (MUINT8*)parg1;
        break;
    case CMD_SENSOR_SET_N3D_STOP_STREAMING:
        FeatureId = SENSOR_FEATURE_SET_N3D_STOP_STREAMING;
        FeatureParaLen = sizeof(MUINTPTR);
        pFeaturePara = (MUINT8*)parg1;
        break;
    case CMD_SENSOR_SET_N3D_START_STREAMING:
        FeatureId = SENSOR_FEATURE_SET_N3D_START_STREAMING;
        FeatureParaLen = sizeof(MUINTPTR);
        pFeaturePara = (MUINT8*)parg1;
        break;
    case CMD_SENSOR_SET_IHDR_SHUTTER_GAIN:
        FeatureId = SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN;
        FeaturePara[0] = (1000 * (MUINT64)(*parg1)) / m_LineTimeInus;
        FeaturePara[1] = (1000 * (MUINT64)(*parg2)) / m_LineTimeInus;
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] LineTimeInus = %llu %d", (long long)FeaturePara[0], m_LineTimeInus);
            FeaturePara[0] = 1;
        }
        if(FeaturePara[1] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] LineTimeInus = %llu %d", (long long)FeaturePara[1], m_LineTimeInus);
            FeaturePara[1] = 1;
        }

        FeaturePara[2]= *parg3;
        FeaturePara[2] >>= 4; //from 10b to 6b base

        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_HDR_SHUTTER:
        FeatureId = SENSOR_FEATURE_SET_HDR_SHUTTER;
        FeaturePara[0] = (1000 * (MUINT64)(*parg1)) / m_LineTimeInus;
        FeaturePara[1] = (1000 * (MUINT64)(*parg2)) / m_LineTimeInus;
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_HDR_SHUTTER] LineTimeInus = %llu %d", (long long)FeaturePara[0], m_LineTimeInus);
            FeaturePara[0] = 1;
        }
        if(FeaturePara[1] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_HDR_SHUTTER] LineTimeInus = %llu %d", (long long)FeaturePara[1], m_LineTimeInus);
            FeaturePara[1] = 1;
        }
        if(parg3 != NULL)
            FeaturePara[2]= *parg3;

        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_HDR_SHUTTER] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_ISO:
        FeatureId = SENSOR_FEATURE_SET_ISO;
        pFeaturePara = (MUINT8*)parg1;
        FeatureParaLen = sizeof(SET_SENSOR_ISO);
        break;

    case CMD_SENSOR_SET_HDR_ATR:
        FeatureId = SENSOR_FEATURE_SET_HDR_ATR;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = *parg2;
        FeaturePara[2] = *parg3;
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
        LOG_MSG("[CMD_SENSOR_SET_HDR_ATR] FeaturePara[0] = %lu, FeaturePara[1] = %lu, FeaturePara[2] = %lu",
                FeaturePara[0], FeaturePara[1], FeaturePara[2]);
        break;
    case CMD_SENSOR_SET_HDR_TRI_GAIN:
        FeatureId = SENSOR_FEATURE_SET_HDR_TRI_GAIN;
        if (m_LineTimeInus == 0)
            LOG_ERR("LineTime can not be 0");
        for(i = 0; i < 3; i++) {
            if(m_shutter[i] == 0 || m_LineTimeInus == 0) {
                LOG_ERR("Exposure Time or lineTime can not be 0, index:%d, m_shutter:%d", i, m_shutter[i]);
                continue;
            }
            u4temp = (1000 * m_shutter[i]) / m_LineTimeInus; /*driver shutter-line number*/
            if(u4temp == 0) { // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_HDR_TRI_GAIN] shutter[%d]=%d, LineTimeInus=%d",
                    i, m_shutter[i], m_LineTimeInus);
                u4temp = 1;
            }
            ratio[i] = (float)(1000 * m_shutter[i]) / (float)(u4temp * m_LineTimeInus); /*algo /driver shutter*/
            if(ratio[i] == 0)
                ratio[i] = 1;
        }
        LOG_MSG("[CMD_SENSOR_SET_HDR_TRI_GAIN] ratio[0] = %f, ratio[1] = %f, ratio[2] = %f",
            ratio[0], ratio[1], ratio[2]);

        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[0] = FeaturePara[0] * ratio[0];
        //LOG_MSG("[CMD_SENSOR_SET_HDR_TRI_GAIN] LongGain[0] = %lu", FeaturePara[0]);
        FeaturePara[0] >>= 4;
        FeaturePara[1] = (*parg2) * ratio[1];
        //LOG_MSG("[CMD_SENSOR_SET_HDR_TRI_GAIN] MiddleGain[1] = %lu", FeaturePara[1]);
        FeaturePara[1] >>= 4;
        FeaturePara[2] = (*parg3) * ratio[2];
        //LOG_MSG("[CMD_SENSOR_SET_HDR_TRI_GAIN] ShortGain[2] = %lu", FeaturePara[2]);
        FeaturePara[2] >>= 4;
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
        LOG_MSG("[CMD_SENSOR_SET_HDR_TRI_GAIN] LongGain[0] = %lu, MiddleGain = %lu, ShortGain = %lu",
                FeaturePara[0], FeaturePara[1], FeaturePara[2]);
        break;
    case CMD_SENSOR_SET_HDR_TRI_SHUTTER:
        FeatureId = SENSOR_FEATURE_SET_HDR_TRI_SHUTTER;
        FeaturePara[0] = (1000 * (MUINT64)(*parg1)) / m_LineTimeInus;
        FeaturePara[1] = (1000 * (MUINT64)(*parg2)) / m_LineTimeInus;
        FeaturePara[2] = (1000 * (MUINT64)(*parg3)) / m_LineTimeInus;
        for(i = 0; i < 3; i++) {
            if(FeaturePara[i] == 0) { // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_HDR_TRI_SHUTTER] shutter[%d]=%llu, LineTimeInus=%d",
                    i, (long long)FeaturePara[i], m_LineTimeInus);
                FeaturePara[i] = 1;
            }
        }
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;

        m_shutter[0] = *parg1;
        m_shutter[1] = *parg2;
        m_shutter[2] = *parg3;
        LOG_MSG("[CMD_SENSOR_SET_HDR_TRI_SHUTTER] LongShutter = %lu, MiddleShutter = %lu, ShortShutter = %lu",
            FeaturePara[0], FeaturePara[1], FeaturePara[2]);
        break;
    case CMD_SENSOR_SET_SENSOR_LSC_TBL:
        FeatureId = SENSOR_FEATURE_SET_LSC_TBL;
        FeaturePara[0] = (*parg3); //SIZE
        FeaturePara[1] = (MUINTPTR)(*parg2Addr);
        FeaturePara[2] = *parg1; //INDEX
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_LSC_TBL] size:%lu index:%lu",
            FeaturePara[0], FeaturePara[2]);
        break;

    case CMD_SENSOR_SET_DEBUG_DUMP:
        FeatureId = SENSOR_FEATURE_DEBUG_IMGSENSOR;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

#ifdef CONFIG_MTK_CAM_SECURE
    case CMD_SENSOR_OPEN_SECURE_SESSION:
        FeatureId = SENSOR_FEATURE_OPEN_SECURE_SESSION;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        LOG_MSG("CMD_SENSOR_OPEN_SECURE_SESSION");
        break;
    case CMD_SENSOR_CLOSE_SECURE_SESSION:
        FeatureId = SENSOR_FEATURE_CLOSE_SECURE_SESSION;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        LOG_MSG("CMD_SENSOR_CLOSE_SECURE_SESSION");
        break;
#endif

    case CMD_SENSOR_GET_SENSOR_SYNC_MODE_CAPACITY:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_SYNC_MODE_CAPACITY;
        FeaturePara[0] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_SYNC_MODE_CAPACITY] FeaturePara[0] = %lu, FeaturePara[1] = %lx, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_SYNC_MODE:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_SYNC_MODE;
        FeaturePara[0] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_SYNC_MODE] FeaturePara[0] = %lu, FeaturePara[1] = %lx, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_SENSOR_SYNC_MODE:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_SYNC_MODE;
        FeaturePara[0]= *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_SYNC_MODE] FeaturePara[0] = %lu, FeaturePara[1] = %lx, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_BINNING_TYPE:
        FeatureId = SENSOR_FEATURE_GET_BINNING_TYPE;
        pConfigParam = (NSCam::IHalSensor::ConfigParam*)parg2;
        LOG_MSG("ScenarioId = %d, HDRMode = %d",
            pConfigParam->scenarioId,
            pConfigParam->HDRMode);
        FeaturePara[0] = 0;
        FeaturePara[1] = pConfigParam->scenarioId;
        FeaturePara[2] = pConfigParam->HDRMode;
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
        break;
    default:
        LOG_ERR("[sendCommand]Command ID = %d is undefined", cmd);
        return SENSOR_UNKNOWN_ERROR;
    }

    {
        Mutex::Autolock _l(mLock);
        if (m_fd == -1) {
            LOG_MSG("sendCommand before init, cmd = 0x%x", cmd);
            char cBuf[64];
            sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
            if ((m_fd = ::open(cBuf, O_RDWR)) < 0) {
                LOG_ERR("[init]: error opening %s: %s", cBuf, strerror(errno));
                return SENSOR_INVALID_DRIVER;
            }
            cmdBeforePowerOn = true;
        }

        err= featureControl(FeatureId, pFeaturePara, FeatureParaLen);

        if (err < 0) {
            LOG_MSG("[sendCommand] cmd = %d, FeatureId = %d", cmd, FeatureId);
            LOG_ERR("[sendCommand] Err-ctrlCode (%s)", strerror(errno));
            return -errno;
        }

        if (cmdBeforePowerOn && m_fd > 0) {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    switch (cmd) {
    case CMD_SENSOR_GET_YUV_AE_STATUS:
    case CMD_SENSOR_GET_YUV_AWB_STATUS:
    case CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS:
    case CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS:
    case CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO:
    case CMD_SENSOR_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME:
    case CMD_SENSOR_GET_TEST_PATTERN_CHECKSUM_VALUE:
    case CMD_SENSOR_GET_TEMPERATURE_VALUE:
    case CMD_SENSOR_GET_YUV_AF_STATUS:

    case CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM:
    case CMD_SENSOR_GET_PIXEL_CLOCK_FREQ:
    case CMD_SENSOR_GET_SENSOR_SYNC_MODE_CAPACITY:
    case CMD_SENSOR_GET_SENSOR_SYNC_MODE:
    case CMD_SENSOR_GET_AE_EFFECTIVE_FRAME_FOR_LE:
    case CMD_SENSOR_GET_AE_FRAME_MODE_FOR_LE:
        *((MUINT32*)parg1)=FeaturePara[0];
        break;
    case CMD_SENSOR_GET_BINNING_TYPE:
        *((MUINT32*)parg1)=FeaturePara[0];
        LOG_MSG("CMD_SENSOR_GET_BINNING_TYPE ratio = %lu",FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO:
    case CMD_SENSOR_SET_YUV_AUTOTEST:
        *((MUINT32*)parg1)=FeaturePara[0];
        *((MUINT32*)parg2)=FeaturePara[1];
        break;
    case CMD_SENSOR_GET_OFFSET_TO_START_OF_EXPOSURE:
    case CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY:
    case CMD_SENSOR_GET_SENSOR_HDR_CAPACITY:
    case CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
    case CMD_SENSOR_GET_MIPI_PIXEL_RATE:
    case CMD_SENSOR_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO:
    case CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM_BY_SCENARIO:
        *((MUINT32*)parg2)=FeaturePara[1];
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %lu, FeaturePara[1] = %lx, FeaturePara[2] = %lu, FeaturePara[3] = %lu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
        //LOG_MSG("FeaturePara %llx",(MUINTPTR)(&FeaturePara[1]));
        LOG_MSG("FeaturePara %d",(FeaturePara[1]));
        LOG_MSG("value = %d",(MUINT32)(*parg2));
#endif
    	break;
    case CMD_SENSOR_GET_SENSOR_VC_INFO2:
        pInfo2 = (SensorVCInfo2*)parg1;
        pInfo2->vcInfo2s.clear();
        if (vcInfo2.updated == 1){
            memcpy(pInfo2, &vcInfo2, sizeof(MUINT16)* 5);/*5 include VC_Num, VC_PixelNum, ModeSelect, EXPO_Ratio, ODValue & RG_STATSMODE*/
            for(int i = 0; i < 8; i++) {
                if(vcInfo2.vc_info[i].VC_FEATURE > VC_NONE
                    &&vcInfo2.vc_info[i].VC_FEATURE < VC_MAX_NUM
                    && vcInfo2.vc_info[i].VC_SIZEH_PIXEL != 0
                    && vcInfo2.vc_info[i].VC_SIZEV != 0) {
                    vcInfo2.vc_info[i].VC_SIZEH_BYTE = vcInfo2.vc_info[i].VC_DataType != 0x2b ?vcInfo2.vc_info[i].VC_SIZEH_PIXEL :vcInfo2.vc_info[i].VC_SIZEH_PIXEL*10/8;
                    pInfo2->vcInfo2s.push_back(vcInfo2.vc_info[i]);
                }
            }
        } else {
            //for legacy driver compatible
            scenarioId = *parg2;
            this->sendCommand(CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&vcInfo, (MUINTPTR)&scenarioId);
            memcpy(pInfo2, &vcInfo, sizeof(MUINT16)* 5);
            if (vcInfo.VC4_DataType != 0 && vcInfo.VC4_SIZEH != 0 && vcInfo.VC4_SIZEV != 0) {
                //imx576 3hdr type
                vc[0].VC_DataType = vcInfo.VC0_DataType;
                vc[0].VC_ID=vcInfo.VC0_ID;
                vc[0].VC_FEATURE = VC_RAW_DATA;
                vc[0].VC_SIZEH_PIXEL = vcInfo.VC0_SIZEH;
                vc[0].VC_SIZEV = vcInfo.VC0_SIZEV;

                vc[1].VC_DataType = vcInfo.VC1_DataType;
                vc[1].VC_ID=vcInfo.VC1_ID;
                vc[1].VC_FEATURE = VC_3HDR_EMBEDDED;
                vc[1].VC_SIZEH_PIXEL = vcInfo.VC1_SIZEH;
                vc[1].VC_SIZEV = vcInfo.VC1_SIZEV;
                vc[1].VC_SIZEH_BYTE = vcInfo.VC1_DataType != 0x2b ?vcInfo.VC1_SIZEH :vcInfo.VC1_SIZEH*10/8;


                vc[2].VC_DataType = vcInfo.VC2_DataType;
                vc[2].VC_ID=vcInfo.VC2_ID;
                vc[2].VC_FEATURE = VC_3HDR_Y;
                vc[2].VC_SIZEH_PIXEL = vcInfo.VC2_SIZEH;
                vc[2].VC_SIZEV = vcInfo.VC2_SIZEV;
                vc[2].VC_SIZEH_BYTE = vcInfo.VC2_DataType != 0x2b ?vcInfo.VC2_SIZEH :vcInfo.VC2_SIZEH*10/8;

                vc[3].VC_DataType =vcInfo.VC3_DataType;
                vc[3].VC_ID=vcInfo.VC3_ID;
                vc[3].VC_FEATURE = VC_3HDR_AE;
                vc[3].VC_SIZEH_PIXEL = vcInfo.VC3_SIZEH;
                vc[3].VC_SIZEV = vcInfo.VC3_SIZEV;
                vc[3].VC_SIZEH_BYTE = vcInfo.VC3_DataType != 0x2b ?vcInfo.VC3_SIZEH :vcInfo.VC3_SIZEH*10/8;

                vc[4].VC_DataType = vcInfo.VC4_DataType;
                vc[4].VC_ID=vcInfo.VC4_ID;
                vc[4].VC_FEATURE = VC_3HDR_FLICKER;
                vc[4].VC_SIZEH_PIXEL = vcInfo.VC4_SIZEH;
                vc[4].VC_SIZEV = vcInfo.VC4_SIZEV;
                vc[4].VC_SIZEH_BYTE = vcInfo.VC4_DataType != 0x2b ?vcInfo.VC4_SIZEH :vcInfo.VC4_SIZEH*10/8;
                for (int i = 0; i < 5; i++)
                    pInfo2->vcInfo2s.push_back(vc[i]);
            }else if (vcInfo.VC0_DataType != 0 && vcInfo.VC0_SIZEH != 0 && vcInfo.VC0_SIZEV != 0){
                vc[0].VC_DataType = vcInfo.VC0_DataType;
                vc[0].VC_ID=vcInfo.VC0_ID;
                vc[0].VC_FEATURE = VC_RAW_DATA;
                vc[0].VC_SIZEH_PIXEL = vcInfo.VC0_SIZEH;
                vc[0].VC_SIZEV = vcInfo.VC0_SIZEV;

                vc[1].VC_DataType = vcInfo.VC1_DataType;
                vc[1].VC_ID=vcInfo.VC1_ID;
                vc[1].VC_FEATURE = VC_HDR_MVHDR;
                vc[1].VC_SIZEH_PIXEL = vcInfo.VC1_SIZEH;
                vc[1].VC_SIZEV = vcInfo.VC1_SIZEV;
                vc[1].VC_SIZEH_BYTE = vcInfo.VC1_DataType != 0x2b ?vcInfo.VC1_SIZEH :vcInfo.VC1_SIZEH*10/8;


                vc[2].VC_DataType = vcInfo.VC2_DataType;
                vc[2].VC_ID=vcInfo.VC2_ID;
                vc[2].VC_FEATURE = VC_PDAF_STATS;
                vc[2].VC_SIZEH_PIXEL = vcInfo.VC2_SIZEH;
                vc[2].VC_SIZEV = vcInfo.VC2_SIZEV;
                vc[2].VC_SIZEH_BYTE = vcInfo.VC2_DataType != 0x2b ?vcInfo.VC2_SIZEH :vcInfo.VC2_SIZEH*10/8;
                for (int i = 0; i < 3; i++)
                    pInfo2->vcInfo2s.push_back(vc[i]);
            }

        }

        break;
    default:
        break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32
ImgSensorDrv::getID()
{
    return m_list.id;
}

/*******************************************************************************
*
********************************************************************************/
MUINT8*
ImgSensorDrv::getName()
{
    return m_list.name;
}

/*******************************************************************************
*
********************************************************************************/
IMAGE_SENSOR_TYPE
ImgSensorDrv::getType()
{
    ACDK_SENSOR_INFO_STRUCT &info = m_drvInfo.info;

    LOG_MSG("[getType]");

    if (info.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_B &&
        info.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_R) {
        return IMAGE_SENSOR_TYPE_RAW;
    } else if (info.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW8_B &&
               info.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW8_R) {
        return IMAGE_SENSOR_TYPE_RAW8;
    } else if (info.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_UYVY &&
               info.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YVYU) {
        return IMAGE_SENSOR_TYPE_YUV;
    } else if (info.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_CbYCrY &&
               info.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YCrYCb) {
        return IMAGE_SENSOR_TYPE_YCBCR;
    } else if (info.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_RWB_B &&
               info.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_RWB_R) {
        return IMAGE_SENSOR_TYPE_RAW;
    } else if (info.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_4CELL_B &&
               info.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_4CELL_R) {
        return IMAGE_SENSOR_TYPE_RAW;
    } else if (info.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_B &&
               info.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_R) {
        return IMAGE_SENSOR_TYPE_RAW;

    } else if (info.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_B &&
               info.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_R) {
        return IMAGE_SENSOR_TYPE_RAW;
    } else if (info.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_RAW_MONO) {
        return IMAGE_SENSOR_TYPE_RAW;
    } else {
        return IMAGE_SENSOR_TYPE_UNKNOWN;
    }

    return IMAGE_SENSOR_TYPE_UNKNOWN;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::open()
{
    MUINT32 featurePara;
    MUINT32 ret = 0;

    if(m_user == 0)
        ret = featureControl(SENSOR_FEATURE_OPEN, (MUINT8 *)&featurePara, sizeof(MUINT32));

    android_atomic_inc(&m_user);
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::close()
{
    MUINT32 featurePara;
    MUINT32 ret = 0;

    if (!m_user) {
        LOG_ERR("imgsensor open/close not sync");
        return -1;
    }

    android_atomic_dec(&m_user);

    if(m_user == 0)
        ret = featureControl(SENSOR_FEATURE_CLOSE, (MUINT8 *)&featurePara, sizeof(MUINT32));

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::getInfo()
{
    IMAGESENSOR_GETINFO_STRUCT getInfo;

    MINT32 err                 = SENSOR_NO_ERROR;
    getInfo.SensorId           = (MUINT32)m_sensorIdx;
    getInfo.pInfo              = &m_drvInfo.info;
    getInfo.pSensorResolution  = &m_drvInfo.resolutionInfo;

    if ((err = ioctl(m_fd, KDIMGSENSORIOC_X_GETINFO2 , &getInfo)) < 0) {
        LOG_ERR("[getInfo2]Err-ctrlCode (%s)", strerror(errno));
        return -errno;
    }

    LOG_MSG("prv w=0x%x,h=0x%x",m_drvInfo.resolutionInfo.SensorPreviewWidth,m_drvInfo.resolutionInfo.SensorPreviewHeight);
    LOG_MSG("cap w=0x%x,h=0x%x",m_drvInfo.resolutionInfo.SensorFullWidth,m_drvInfo.resolutionInfo.SensorFullHeight);
    LOG_MSG("vd  w=0x%x,h=0x%x",m_drvInfo.resolutionInfo.SensorVideoWidth,m_drvInfo.resolutionInfo.SensorVideoHeight);
    LOG_MSG("pre GrapX=0x%x,GrapY=0x%x",m_drvInfo.info.SensorGrabStartX_PRV,m_drvInfo.info.SensorGrabStartY_PRV);
    //LOG_MSG("cap GrapX=%d,GrapY=%d",pSensorInfo->info->SensorGrabStartX_CAP,pSensorInfo->info->SensorGrabStartY_CAP);
    //LOG_MSG("vd  GrapX=%d,GrapY=%d",pSensorInfo->info->SensorGrabStartX_VD,pSensorInfo->info->SensorGrabStartY_VD);
    //LOG_MSG("vd1 GrapX=%d,GrapY=%d",pSensorInfo->info->SensorGrabStartX_VD1,pSensorInfo->info->SensorGrabStartY_VD1);
    //LOG_MSG("vd2 GrapX=%d,GrapY=%d",pSensorInfo->info->SensorGrabStartX_VD2,pSensorInfo->info->SensorGrabStartY_VD2);

   return err;
}

/*******************************************************************************
*
********************************************************************************/
SENSORDRV_INFO_STRUCT*
ImgSensorDrv::getDrvInfo()
{
    return &m_drvInfo;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::featureControl(
    ACDK_SENSOR_FEATURE_ENUM FeatureId,
    MUINT8 *pFeaturePara,
    MUINT32 featureParaLen
)
{
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;

    if (FeatureId == SENSOR_FEATURE_BEGIN) {
        LOG_MSG("[featureControl] Skip due to no FeatureId");
        return SENSOR_NO_ERROR;
    }

    if (m_fd == -1) {
        LOG_ERR("[sendCommand]m_fd fail, sendCommand must be called after init()!");
        return SENSOR_UNKNOWN_ERROR;
    }

    if (pFeaturePara == NULL)
        return SENSOR_INVALID_PARA;

    featureCtrl.InvokeCamera    = m_sensorIdx;
    featureCtrl.FeatureId       = FeatureId;
    featureCtrl.pFeaturePara    = pFeaturePara;
    featureCtrl.pFeatureParaLen = &featureParaLen;

    if (ioctl(m_fd, KDIMGSENSORIOC_X_FEATURECONCTROL, &featureCtrl) < 0) {
        LOG_ERR("[featureControl] Err-ctrlCode (%s)", strerror(errno));
        return -errno;
    }

    return SENSOR_NO_ERROR;
}

