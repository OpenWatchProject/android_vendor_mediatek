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
#include "imgsensor_drv.h"
#include "kd_imgsensor.h"

/******************************************************************************
 *
 ******************************************************************************/
SensorDrv*
SensorDrv::
get()
{
    return ImgSensorDrv::singleton();
}

/******************************************************************************
 *
 ******************************************************************************/
ImgSensorDrv*
ImgSensorDrv::
singleton()
{
    static ImgSensorDrv inst;
    return &inst;
}

/*******************************************************************************
*
********************************************************************************/
SensorDrv*
ImgSensorDrv::
getInstance()
{
    LOG_MSG("[ImgSensorDrv] getInstance");
    static ImgSensorDrv singleton;

    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
ImgSensorDrv::
ImgSensorDrv()
    : SensorDrv()
    , m_fdSensor(-1)
    , mUsers(0)
{
    for(int i=IMGSENSOR_SENSOR_IDX_MIN_NUM; i<IMGSENSOR_SENSOR_IDX_MAX_NUM; i++) {
        m_LineTimeInus[i] = 31;
        m_sensorIdx[i] = BAD_SENSOR_INDEX;
    }
}

/*******************************************************************************
*
********************************************************************************/
ImgSensorDrv::
~ImgSensorDrv()
{
    for(int i=IMGSENSOR_SENSOR_IDX_MIN_NUM; i<IMGSENSOR_SENSOR_IDX_MAX_NUM; i++) {
        m_sensorIdx[i] = BAD_SENSOR_INDEX;
    }

    LOG_MSG ("[~ImgSensorDrv]");
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::searchSensor(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    MSDK_SENSOR_INIT_FUNCTION_STRUCT *pSensorInitFunc;

    //! If imp sensor search process already done before,
    //! only need to return the sensorDevs, not need to
    //! search again.
    if (m_sensorIdx[sensorIdx] != BAD_SENSOR_INDEX) {
        //been processed.
        LOG_MSG("[searchSensor] Already processed");

        return SENSOR_ALREADY_SEARCH;
    }

    GetSensorInitFuncList(&pSensorInitFunc);

    LOG_MSG("SENSOR search start");

    //set sensor driver
    MUINT32                           featureParaLen = sizeof(MUINT32);
    MINT32                            idx = 0;

    featureControl(sensorIdx, SENSOR_FEATURE_SET_DRIVER, (MUINT8 *)&idx, &featureParaLen);
    LOG_MSG("set sensor driver id =%x", idx);

    if((m_sensorIdx[sensorIdx] = (idx < 0) ? UNKNOWN_SENSOR_INDEX : idx) >= UNKNOWN_SENSOR_INDEX)
        return SENSOR_INVALID_DRIVER;

    NSFeature::SensorInfoBase* pSensorInfo = pSensorInitFunc[idx].pSensorInfo;

    if (pSensorInfo)
        LOG_MSG("sensorIdx %d found <%#x/%s/%s>", sensorIdx, pSensorInfo->GetID(), pSensorInfo->getDrvName(), pSensorInfo->getDrvMacroName());
    else
        LOG_ERR("m_pSensorInfo[%d] = NULL check if sensor list sync with kernel & user", sensorIdx);

    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::init(
)
{
    UCHAR cBuf[64];

    Mutex::Autolock lock(mLock);
    if (mUsers == 0 && m_fdSensor == -1) {
        sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);

        if ((m_fdSensor = ::open(cBuf, O_RDWR)) < 0) {
            LOG_ERR("[init]: error opening %s: %s", cBuf, strerror(errno));
            return SENSOR_INVALID_DRIVER;
        }
    }
    android_atomic_inc(&mUsers);
#ifdef IMGSENSOR_DRV_OPEN_CLOSE
    LOG_MSG("[init] imgsensor_drv %d",mUsers);
#endif

    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::uninit(
)
{

    Mutex::Autolock lock(mLock);
    android_atomic_dec(&mUsers);
#ifdef IMGSENSOR_DRV_OPEN_CLOSE
    LOG_MSG("[uninit] imgsensor_drv %d", mUsers);
#endif


    if (mUsers == 0 && m_fdSensor > 0) {
        ::close(m_fdSensor);
        m_fdSensor = -1;
    }

    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::setScenario(IMGSENSOR_SENSOR_IDX sensorIdx, SENSOR_DRIVER_SCENARIO_T scenarioconf)
{
    MINT32 ret = SENSOR_NO_ERROR;

    ACDK_SENSOR_EXPOSURE_WINDOW_STRUCT ImageWindow;
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;

    ACDK_SENSOR_CONTROL_STRUCT sensorCtrl;

    MUINT16 FeaturePara16[4] = {0};
    MUINT32 FeaturePara32[2] = {0};
    MUINT32 FeatureParaLen;

//    if(SENSOR_SCENARIO_ID_UNNAMED_START != scenarioconf.sId) {//ToDo: remove
//    }

    //if(ACDK_SCENARIO_ID_MAX != sId[i]) {
    //FPS
    FeaturePara32[0] = scenarioconf.InitFPS;
    FeatureParaLen = sizeof(MUINT32);
    ret = featureControl(sensorIdx, SENSOR_FEATURE_SET_FRAMERATE,  (MUINT8*)FeaturePara32,(MUINT32*)&FeatureParaLen);
    if (ret < 0) {
         LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_FRAMERATE error");
         return SENSOR_UNKNOWN_ERROR;
    }
    //HDR Enable
    //if(scenarioconf.HDRMode <= 1)
    {
        FeaturePara32[0] = scenarioconf.HDRMode;
        FeatureParaLen = sizeof(MUINT32);
        ret = featureControl(sensorIdx, SENSOR_FEATURE_SET_HDR,  (MUINT8*)FeaturePara32,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
             LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_HDR error");
             return SENSOR_UNKNOWN_ERROR;
        }
    }
    /* PDAF Type */
    {
        FeaturePara32[0] = scenarioconf.PDAFMode != PDAF_SUPPORT_NA ;
        FeatureParaLen = sizeof(MUINT32);
        ret = featureControl(sensorIdx, SENSOR_FEATURE_SET_PDAF,  (MUINT8*)FeaturePara32,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
             LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_PDAF error");
             return SENSOR_UNKNOWN_ERROR;
        }
    }
    SensorConfigData.SensorImageMirror = ACDK_SENSOR_IMAGE_NORMAL;

    //set sensor preview/capture mode
    sensorCtrl.InvokeCamera = sensorIdx;
    sensorCtrl.ScenarioId = (MSDK_SCENARIO_ID_ENUM)scenarioconf.sId;
    sensorCtrl.pImageWindow = &ImageWindow;
    sensorCtrl.pSensorConfigData = &SensorConfigData;

    if ((ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_CONTROL , &sensorCtrl)) < 0) {
        LOG_ERR("[setScenario]Err-ctrlCode (%s)", strerror(errno));
        return -errno;
    }
    //get exposure line time for each scenario
    FeatureParaLen = sizeof(MUINT64);
    ret = featureControl(sensorIdx,SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)FeaturePara32,(MUINT32*)&FeatureParaLen);//ToDo: remove

    if (ret < 0) {
       LOG_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error");
       return SENSOR_UNKNOWN_ERROR;
    }

    FeatureParaLen = sizeof(MUINT64);
    ret = featureControl(sensorIdx, SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)FeaturePara16,(MUINT32*)&FeatureParaLen);
    if (ret < 0) {
        LOG_ERR("[setScenario]: SENSOR_FEATURE_GET_PERIOD error");
        return SENSOR_UNKNOWN_ERROR;
    }

    if (FeaturePara32[0]) {
        if(FeaturePara32[0] >= 1000) {
            m_LineTimeInus[sensorIdx] = (MUINT32)(((MUINT64)FeaturePara16[0]*1000000 + ((FeaturePara32[0]/1000)-1))/(FeaturePara32[0]/1000));   // 1000 base , 33657 mean 33.657 us
            m_MinFrameLength[sensorIdx] = (MUINT32)FeaturePara16[1];
        }
        else {
            LOG_ERR("[setScenario]: Sensor clock too slow = %d %d", FeaturePara32[0], FeaturePara16[0]);
        }

        LOG_MSG("[setScenario]DevID = %d, m_LineTimeInus = %d Scenario id = %d, PixelClk = %d, PixelInLine = %d, Framelength = %d",
            sensorIdx, m_LineTimeInus[sensorIdx], scenarioconf.sId, FeaturePara32[0], FeaturePara16[0], FeaturePara16[1]);
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::sendCommand(
        IMGSENSOR_SENSOR_IDX sensorIdx,
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
    MUINTPTR* parg1Addr = (MUINTPTR*)parg1;
    MUINTPTR* parg2Addr = (MUINTPTR*)parg2;

    MINT32 err = SENSOR_NO_ERROR;
    MUINT32  LineTimeInus = 0;
    MUINT32  MinFrameLength = 0;

    ACDK_SENSOR_FEATURE_ENUM FeatureId = SENSOR_FEATURE_BEGIN;
    MUINT8 *pFeaturePara = NULL; // Pointer to feature data that communicate with kernel
    MUINT32 FeatureParaLen = 0; // The length of feature data

    MUINT64 FeaturePara[4] = {0}; // Convert input paras to match the format used in kernel
    MUINT64  u8temp = 0;


//#define SENDCMD_LOG // Open sendcommand log, for test only
    switch (cmd) {
    case CMD_SENSOR_SET_DRIVE_CURRENT:
    {
        FeatureId = SENSOR_FEATURE_SET_MCLK_DRIVE_CURRENT;
        std::string str_prop("vendor.debug.seninf.mclk");
        str_prop += std::to_string(sensorIdx);
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get(str_prop.c_str(), value, "-1");
        int inputCurrent = atoi(value);

        FeaturePara[0] = 0;
        if (inputCurrent >= ISP_DRIVING_MAX_NUM || inputCurrent < ISP_DRIVING_2MA)
            FeaturePara[0] = (MUINT64)*parg1;
        else
            FeaturePara[0] = inputCurrent;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_SET_MCLK_DRIVE_CURRENT] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    }
    case CMD_SENSOR_SET_SENSOR_EXP_TIME:
        u8temp = *parg1;/*64bit extend */
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        LineTimeInus = m_LineTimeInus[sensorIdx];
        FeaturePara[0] = ((1000 * (u8temp)) / LineTimeInus);
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d", (long long)FeaturePara[0], LineTimeInus);
            FeaturePara[0] = 1;
        }
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_EXP_LINE:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_EXP_LINE] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_GAIN:
        FeatureId = SENSOR_FEATURE_SET_GAIN;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[0] >>= 4;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_DUAL_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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

        LineTimeInus = m_LineTimeInus[sensorIdx];

        FeaturePara[0] = *parg1; // RAW Gain R, Gr
        FeaturePara[1] = *(parg1+1);  // RAW Gain Gb, B
        FeaturePara[2] = *(parg1+2);  // Exposure time
        FeaturePara[2] = ((1000 * FeaturePara[2]) / LineTimeInus);
        if(FeaturePara[2]  == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_SENSOR_SYNC] LineTimeInus = %llu %d", (long long)FeaturePara[2] , LineTimeInus);
            FeaturePara[2]  = 1;
        }
        FeaturePara[2] = (FeaturePara[2] ) | (((MUINT64)((*(parg1+3))>>4)) << 16); // Sensor gain from 10b to 6b base
        FeaturePara[3] = *(parg1+4);  // Delay frame cnt
        FeatureParaLen = sizeof(MUINT64) * 4;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_CCT_FEATURE_CONTROL:
        FeatureId = (ACDK_SENSOR_FEATURE_ENUM)*parg1;
        pFeaturePara = (MUINT8*)parg2;
        FeatureParaLen = (MUINT32)*parg3;
        break;

    case CMD_SENSOR_SET_SENSOR_CALIBRATION_DATA:
        FeatureId = SENSOR_FEATURE_SET_CALIBRATION_DATA;
        pFeaturePara = (UINT8*)parg1;
        FeatureParaLen = sizeof(SET_SENSOR_CALIBRATION_DATA_STRUCT);
        break;

    case CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO:
        FeatureId = SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_TEST_PATTERN_OUTPUT:
        FeatureId = SENSOR_FEATURE_SET_TEST_PATTERN;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_TEST_PATTERN_OUTPUT] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_ESHUTTER_GAIN:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER_GAIN;
        FeaturePara[0] = *parg1; // exposure time (us)
        LineTimeInus = m_LineTimeInus[sensorIdx];
        FeaturePara[0] = ((1000 * FeaturePara[0] ) / LineTimeInus);

        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_ESHUTTER_GAIN] m_LineTime = %llu %d", (long long)FeaturePara[0] , LineTimeInus);
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
        LOG_MSG("[CMD_SENSOR_SET_ESHUTTER_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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

        LineTimeInus = m_LineTimeInus[sensorIdx];
        MinFrameLength = m_MinFrameLength[sensorIdx];

        FeaturePara[0] = ((double)(1000 * (*parg1)) / LineTimeInus) + 0.5;
        FeaturePara[1] = ((double)(1000 * (*parg2)) / LineTimeInus) + 0.5;
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] LineTimeInus = %llu %d", (long long)FeaturePara[0], LineTimeInus);
            FeaturePara[0] = 1;
        }
        if(FeaturePara[1] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] LineTimeInus = %llu %d", (long long)FeaturePara[0], LineTimeInus);
            FeaturePara[1] = 1;
        }
        LOG_MSG("[SET_SENSOR_EXP_FRAME_TIME] ExpTime/FrmTime=(%u/%u) ExpLine/FrmLength/LineTimeInus/MinFrmLength=(%llu/%llu/%u/%u)", \
        *parg1, *parg2, (long long)FeaturePara[0], (long long)FeaturePara[1], LineTimeInus, MinFrameLength);

        if(MinFrameLength > FeaturePara[1]) {
            LOG_ERR("[SET_SENSOR_EXP_FRAME_TIME] Error: Set framelength (%llu) is smaller than MinFrmLength (%u)!!!", (long long)FeaturePara[1], MinFrameLength);
            err = SENSOR_INVALID_PARA;
            return err;
        }
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE:
        u8temp = *parg1; /*64bit extend */
        FeatureId = SENSOR_FEATURE_SET_SHUTTER_BUF_MODE;
        LineTimeInus = m_LineTimeInus[sensorIdx];

        FeaturePara[0] = ((1000 * (u8temp)) / LineTimeInus);
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE] LineTimeInus = %llu %d", (long long)FeaturePara[0], LineTimeInus);
            FeaturePara[0] = 1;
        }
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
    LOG_MSG("[CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
    break;

    case CMD_SENSOR_SET_SENSOR_EXP_LINE_BUF_MODE:
        FeatureId = SENSOR_FEATURE_SET_SHUTTER_BUF_MODE;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_ERR("[CMD_SENSOR_SET_SENSOR_EXP_LINE_BUF_MODE] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_GAIN_BUF_MODE:
        FeatureId = SENSOR_FEATURE_SET_GAIN_BUF_MODE;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[0] >>= 4;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_GAIN_BUF_MODE] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_I2C_BUF_MODE_EN:
        FeatureId = SENSOR_FEATURE_SET_I2C_BUF_MODE_EN;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_I2C_BUF_MODE_EN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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
        LineTimeInus = m_LineTimeInus[sensorIdx];

        if ((parg1 == NULL) || (*parg1 == 0)) {
            FeaturePara[0] = 0;
        } else {
            FeaturePara[0] = ((double)(1000 * (*parg1)) / LineTimeInus) + 0.5;
        }
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        if(FeaturePara[0] != 0) {
            LOG_MSG("SET_STREAMING_RESUME, senDev(%d),expTime_us(%6d),LineTimeInus(%d),expLine(%llu)", \
                sensorIdx, (MUINT32)*parg1, LineTimeInus, (long long)FeaturePara[0]);
        } else {
            LOG_MSG("SET_STREAMING_RESUME, senDev(%d),expTime_us is default, keep original value", \
                sensorIdx);
        }
        break;

    case CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT:
        {
#ifdef SENDCMD_LOG
            LOG_MSG("[CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT] *parg1 = %d", (MUINT32)*parg1);
#endif
            return err;
        }
        break;

    case CMD_SENSOR_GET_INPUT_BIT_ORDER:
        if (sensorIdx >= IMGSENSOR_SENSOR_IDX_MAX_NUM || sensorIdx < IMGSENSOR_SENSOR_IDX_MIN_NUM) {
            LOG_ERR("[sendCommand]<CMD_SENSOR_GET_INPUT_BIT_ORDER> - bad sensor id(%x)", (int)sensorIdx);
            *parg1 = 0;
            err = -1;
        } else {
            *parg1 = NSCamCustomSensor::getSensorInputDataBitOrder(sensorIdx);
            err = 0;
        }

        return  err;
        break;
    case CMD_SENSOR_GET_PAD_PCLK_INV:
        if (sensorIdx >= IMGSENSOR_SENSOR_IDX_MAX_NUM || sensorIdx < IMGSENSOR_SENSOR_IDX_MIN_NUM) {
            LOG_ERR("[sendCommand]<CMD_SENSOR_GET_PAD_PCLK_INV> - bad sensor id(%x)", (int)sensorIdx);
            *parg1 = 0;
            err = -1;
        } else {
            *parg1 = NSCamCustomSensor::getSensorPadPclkInv(sensorIdx);
            err = 0;
        }
        return  err;
        break;

    case CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE:
        if (sensorIdx >= IMGSENSOR_SENSOR_IDX_MAX_NUM || sensorIdx < IMGSENSOR_SENSOR_IDX_MIN_NUM) {
            LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE> - bad sensor id(%x)", (int)sensorIdx);
            *parg1 = 0;
            err = -1;
        } else {
            *parg1 = NSCamCustomSensor::getSensorOrientation(sensorIdx);
            err = 0;
        }
        return err;
        break;

    case CMD_SENSOR_GET_SENSOR_FACING_DIRECTION:
        if (sensorIdx >= IMGSENSOR_SENSOR_IDX_MAX_NUM || sensorIdx < IMGSENSOR_SENSOR_IDX_MIN_NUM) {
            LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_FACING_DIRECTION> - bad sensor id(%x)", (int)sensorIdx);
            *parg1 = 0;
            err = -1;
        } else {
            *parg1 = NSCamCustomSensor::getSensorFacingDirection(sensorIdx);
            err = 0;
        }
        return  err;
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

    case CMD_SENSOR_GET_SENSOR_FEATURE_INFO:
        *parg1Addr = (MUINTPTR)getSensorInfo(sensorIdx);
        err = 0;
        return  err;

        break;
    case CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        FeatureId = SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
        LOG_MSG("FeaturePara %d",(FeaturePara[1]));
        LOG_MSG("framerate = %d",(MUINT32)(*parg2));
#endif
        break;

    case CMD_SENSOR_GET_FAKE_ORIENTATION:
         return 0;
     break;

    case CMD_SENSOR_GET_SENSOR_VIEWANGLE:
        if (sensorIdx >= IMGSENSOR_SENSOR_IDX_MAX_NUM || sensorIdx < IMGSENSOR_SENSOR_IDX_MIN_NUM) {
            LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_VIEWANGLE> - bad sensor id(%x)", (int)sensorIdx);
            *parg1 = 0;
            err = -1;
        } else {

            *parg1 = NSCamCustomSensor::getSensorViewAngle_H(sensorIdx);
            *parg2 = NSCamCustomSensor::getSensorViewAngle_V(sensorIdx);
            err = 0;
        }
        return err;
        break;

    case CMD_SENSOR_GET_MCLK_CONNECTION:
        if (sensorIdx >= IMGSENSOR_SENSOR_IDX_MAX_NUM || sensorIdx < IMGSENSOR_SENSOR_IDX_MIN_NUM) {
            LOG_ERR("[sendCommand]<CMD_SENSOR_GET_MCLK_CONNECTION> - bad sensor id(%x)", (int)sensorIdx);
            *parg1 = 0;
            err = -1;
        } else {
            *parg1 = NSCamCustomSensor::getSensorMclkConnection(sensorIdx);
            err = 0;
        }

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_MCLK_CONNECTION] *parg1 = %d", (MUINT32)*parg1);
#endif
        return  err;
        break;

    case CMD_SENSOR_GET_MIPI_SENSOR_PORT:
        if (sensorIdx >= IMGSENSOR_SENSOR_IDX_MAX_NUM || sensorIdx < IMGSENSOR_SENSOR_IDX_MIN_NUM) {
            LOG_ERR("[sendCommand]<CMD_SENSOR_GET_MIPI_SENSOR_PORT> - bad sensor id(%x)", (int)sensorIdx);
            *parg1 = 0;
            err = -1;
        } else {
            *parg1 = NSCamCustomSensor::getMipiSensorPort(sensorIdx);
            err = 0;
        }

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_MIPI_SENSOR_PORT] *parg1 = %d", (MUINT32)*parg1);
#endif
        return  err;
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
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_VC_INFO:
        FeatureId = SENSOR_FEATURE_GET_VC_INFO;
        FeaturePara[0] = *parg2;
        FeaturePara[1] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_VC_INFO] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_PDAF_INFO:
        FeatureId = SENSOR_FEATURE_GET_PDAF_INFO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = (MUINTPTR)parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_GET_PDAF_INFO]%llu %llu",FeaturePara[0],FeaturePara[1]);
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
        FeaturePara[0] = *parg1;    //offset
        FeaturePara[1] = (MUINTPTR)(*parg2Addr);    //the address of pointer pointed
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
        LOG_MSG("[CMD_SET_PDFOCUS_AREA] 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1]);
        break;
    case CMD_SENSOR_GET_PDAF_DATA:
        FeatureId = SENSOR_FEATURE_GET_PDAF_DATA;
        FeaturePara[0] = *parg1;//offset
        FeaturePara[1] = (MUINTPTR)(*parg2Addr);//the address of pointer pointed
        FeaturePara[2] = *parg3;//size of buff
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_PDAF_DATA]0x%llu 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1], FeaturePara[2]);
#endif
        break;
    case CMD_SENSOR_GET_OFFSET_TO_START_OF_EXPOSURE:
        FeatureId = SENSOR_FEATURE_GET_OFFSET_TO_START_OF_EXPOSURE;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0;
        FeaturePara[2] = parg3 != NULL ?(MUINT32)*parg3 :0;
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
        break;

    case CMD_SENSOR_GET_MIPI_PIXEL_RATE:
        FeatureId = SENSOR_FEATURE_GET_MIPI_PIXEL_RATE;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0;
        FeaturePara[2] = *parg3;
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_ERR("[CMD_SENSOR_GET_MIPI_PIXEL_RATE]0x%llu", (long long)FeaturePara[0]);
#endif
        break;
   case CMD_SENSOR_GET_PIXEL_RATE:
            FeatureId = SENSOR_FEATURE_GET_PIXEL_RATE;
            FeaturePara[0] = *parg1;
            FeaturePara[1] = 0;
            FeaturePara[2] = *parg3;
            FeatureParaLen = sizeof(MUINT64) * 3;
            pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
            LOG_ERR("[SENSOR_FEATURE_GET_PIXEL_RATE]0x%llu", (long long)FeaturePara[0]);
#endif
            break;

    case CMD_SENSOR_GET_SENSOR_HDR_CAPACITY:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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
        LOG_MSG("[CMD_SENSOR_GET_4CELL_DATA]0x%llu 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1], FeaturePara[2]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_AWB_GAIN:
        FeatureId = SENSOR_FEATURE_SET_AWB_GAIN;
        pFeaturePara = (UINT8*)parg1;
        FeatureParaLen = sizeof(SET_SENSOR_AWB_GAIN);
        break;

    case CMD_SENSOR_SET_YUV_FEATURE_CMD:
        FeatureId = SENSOR_FEATURE_SET_YUV_CMD;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_YUV_FEATURE_CMD] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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
        //LOG_MSG("zone_addr=0x%llu", FeaturePara[0]);
        break;

    case CMD_SENSOR_SET_YUV_AE_WINDOW:
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("AEzone_addr=0x%llu", FeaturePara[0]);
        break;

    case CMD_SENSOR_SET_YUV_GAIN_AND_EXP_LINE:
        FeatureId = SENSOR_FEATURE_SET_GAIN_AND_ESHUTTER;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_YUV_FEATURE_CMD] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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
        LineTimeInus = m_LineTimeInus[sensorIdx];
        FeaturePara[0] = (1000 * (MUINT64)(*parg1)) / LineTimeInus;
        FeaturePara[1] = (1000 * (MUINT64)(*parg2)) / LineTimeInus;
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] LineTimeInus = %llu %d", (long long)FeaturePara[0], LineTimeInus);
            FeaturePara[0] = 1;
        }
        if(FeaturePara[1] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] LineTimeInus = %llu %d", (long long)FeaturePara[1], LineTimeInus);
            FeaturePara[1] = 1;
        }

        FeaturePara[2]= *parg3;
        FeaturePara[2] >>= 4; //from 10b to 6b base

        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_HDR_SHUTTER:
        FeatureId = SENSOR_FEATURE_SET_HDR_SHUTTER;
        LineTimeInus = m_LineTimeInus[sensorIdx];
        FeaturePara[0] = (1000 * (MUINT64)(*parg1)) / LineTimeInus;
        FeaturePara[1] = (1000 * (MUINT64)(*parg2)) / LineTimeInus;
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_HDR_SHUTTER] LineTimeInus = %llu %d", (long long)FeaturePara[0],LineTimeInus);
            FeaturePara[0] = 1;
        }
        if(FeaturePara[1] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_HDR_SHUTTER] LineTimeInus = %llu %d", (long long)FeaturePara[1], LineTimeInus);
            FeaturePara[1] = 1;
        }
        if(parg3 != NULL)
            FeaturePara[2]= *parg3;
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_HDR_SHUTTER] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
       break;
    case CMD_SENSOR_SET_SENSOR_ISO:
        FeatureId = SENSOR_FEATURE_SET_ISO;
        pFeaturePara = (UINT8*)parg1;
        FeatureParaLen = sizeof(SET_SENSOR_ISO);

        break;
    case CMD_SENSOR_GET_SENSOR_SYNC_MODE_CAPACITY:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_SYNC_MODE_CAPACITY;
        FeaturePara[0] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_SYNC_MODE_CAPACITY] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_SYNC_MODE:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_SYNC_MODE;
        FeaturePara[0] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_SYNC_MODE] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_SENSOR_SYNC_MODE:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_SYNC_MODE;
        FeaturePara[0]= *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_SYNC_MODE] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    default:
        LOG_ERR("[sendCommand]Command ID = %d is undefined",cmd);
        return SENSOR_UNKNOWN_ERROR;
    }

    err= featureControl(sensorIdx, FeatureId,  (MUINT8*)pFeaturePara,(MUINT32*)&FeatureParaLen);
    if (err < 0) {
        LOG_ERR("[sendCommand] Err-ctrlCode (%s)", strerror(errno));
        return -errno;
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
    case CMD_SENSOR_GET_PIXEL_RATE:
        *((MUINT32*)parg2)=FeaturePara[1];
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
        //LOG_MSG("FeaturePara %llx",(MUINTPTR)(&FeaturePara[1]));
        LOG_MSG("FeaturePara %d",(FeaturePara[1]));
        LOG_MSG("value = %d",(MUINT32)(*parg2));
#endif
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
ImgSensorDrv::getSensorID(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    MSDK_SENSOR_INIT_FUNCTION_STRUCT *pSensorInitFunc;
    GetSensorInitFuncList(&pSensorInitFunc);
    return (m_sensorIdx[sensorIdx] < UNKNOWN_SENSOR_INDEX) ?
        pSensorInitFunc[m_sensorIdx[sensorIdx]].SensorId :
        0xFFFFFFFF;
}

/*******************************************************************************
*
********************************************************************************/
NSFeature::SensorInfoBase*
ImgSensorDrv::getSensorInfo(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    MSDK_SENSOR_INIT_FUNCTION_STRUCT *pSensorInitFunc;
    GetSensorInitFuncList(&pSensorInitFunc);
    return (m_sensorIdx[sensorIdx] < UNKNOWN_SENSOR_INDEX) ?
        pSensorInitFunc[m_sensorIdx[sensorIdx]].pSensorInfo :
        NULL;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::open(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    MUINT32                           featureParaLen = sizeof(MUINT32);
    MUINT32                           featurePara;

    return featureControl(sensorIdx, SENSOR_FEATURE_OPEN, (MUINT8 *)&featurePara, &featureParaLen);
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::close(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    MUINT32                           featureParaLen = sizeof(MUINT32);
    MUINT32                           featurePara;

    return featureControl(sensorIdx, SENSOR_FEATURE_CLOSE, (MUINT8 *)&featurePara, &featureParaLen);
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::getInfo(
    IMGSENSOR_SENSOR_IDX sensorIdx,
    MUINT32 ScenarioId,
    ACDK_SENSOR_INFO_STRUCT *pSensorInfo,
    ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData
)
{
    IMGSENSOR_GET_CONFIG_INFO_STRUCT getInfo;
    MINT32 err = SENSOR_NO_ERROR;
    LOG_MSG("[getInfo],m_fdSensor = 0x%x", m_fdSensor);

    if (pSensorInfo == NULL|| pSensorConfigData == NULL) {
        LOG_ERR("[getInfo] NULL pointer");
        return SENSOR_UNKNOWN_ERROR;
    }

    getInfo.SensorId   = sensorIdx;
    getInfo.ScenarioId = (MSDK_SCENARIO_ID_ENUM)ScenarioId;
    getInfo.pInfo = pSensorInfo;
    getInfo.pConfig = pSensorConfigData;

    if ((err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GET_CONFIG_INFO , &getInfo)) < 0) {
        LOG_ERR("[getInfo]Err-ctrlCode (%s)", strerror(errno));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::getInfo2(
    IMGSENSOR_SENSOR_IDX sensorIdx,
    SENSORDRV_INFO_STRUCT *pSensorInfo
)
{
    IMAGESENSOR_GETINFO_STRUCT getInfo;

    MINT32 err                 = SENSOR_NO_ERROR;
    getInfo.SensorId           = (MUINT32)sensorIdx;
    getInfo.pInfo              = &pSensorInfo->info;
    getInfo.pSensorResolution  = &pSensorInfo->resolutionInfo;

    if ((err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GETINFO2 , &getInfo)) < 0) {
        LOG_ERR("[getInfo2]Err-ctrlCode (%s)", strerror(errno));
        return -errno;
    }

    LOG_MSG("prv w=0x%x,h=0x%x",pSensorInfo->resolutionInfo.SensorPreviewWidth,pSensorInfo->resolutionInfo.SensorPreviewHeight);
    LOG_MSG("cap w=0x%x,h=0x%x",pSensorInfo->resolutionInfo.SensorFullWidth,pSensorInfo->resolutionInfo.SensorFullHeight);
    LOG_MSG("vd  w=0x%x,h=0x%x",pSensorInfo->resolutionInfo.SensorVideoWidth,pSensorInfo->resolutionInfo.SensorVideoHeight);
    LOG_MSG("pre GrapX=0x%x,GrapY=0x%x",pSensorInfo->info.SensorGrabStartX_PRV,pSensorInfo->info.SensorGrabStartY_PRV);
    //LOG_MSG("cap GrapX=%d,GrapY=%d",pSensorInfo->info->SensorGrabStartX_CAP,pSensorInfo->info->SensorGrabStartY_CAP);
    //LOG_MSG("vd  GrapX=%d,GrapY=%d",pSensorInfo->info->SensorGrabStartX_VD,pSensorInfo->info->SensorGrabStartY_VD);
    //LOG_MSG("vd1 GrapX=%d,GrapY=%d",pSensorInfo->info->SensorGrabStartX_VD1,pSensorInfo->info->SensorGrabStartY_VD1);
    //LOG_MSG("vd2 GrapX=%d,GrapY=%d",pSensorInfo->info->SensorGrabStartX_VD2,pSensorInfo->info->SensorGrabStartY_VD2);

   return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::featureControl(
    IMGSENSOR_SENSOR_IDX sensorIdx,
    ACDK_SENSOR_FEATURE_ENUM FeatureId,
    MUINT8 *pFeaturePara,
    MUINT32 *pFeatureParaLen
)
{
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;

    if (m_fdSensor == -1) {
        LOG_ERR("[sendCommand]m_fdSensor fail, sendCommand must be called after init()!");
        return SENSOR_UNKNOWN_ERROR;
    }

    if (pFeaturePara == NULL || pFeatureParaLen == NULL) {
        return SENSOR_INVALID_PARA;
    }

    featureCtrl.InvokeCamera = sensorIdx;
    featureCtrl.FeatureId = FeatureId;
    featureCtrl.pFeaturePara = pFeaturePara;
    featureCtrl.pFeatureParaLen = pFeatureParaLen;

    if (ioctl(m_fdSensor, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl) < 0) {
        LOG_ERR("[featureControl] Err-ctrlCode (%s)", strerror(errno));
        return -errno;
    }

    return SENSOR_NO_ERROR;
}//halSensorFeatureControl

/*******************************************************************************
*
********************************************************************************/
IMAGE_SENSOR_TYPE
ImgSensorDrv::getCurrentSensorType(
    IMGSENSOR_SENSOR_IDX sensorIdx
)
{
    ACDK_SENSOR_INFO_STRUCT m_sensorInfo;
    ACDK_SENSOR_CONFIG_STRUCT m_sensorConfigData;

    LOG_MSG("[getCurrentSensorType]");

    if (SENSOR_NO_ERROR != getInfo(sensorIdx, SENSOR_SCENARIO_ID_NORMAL_PREVIEW, &m_sensorInfo, &m_sensorConfigData)) {
       LOG_ERR("[searchSensor] Error:getInfo()");
       return IMAGE_SENSOR_TYPE_UNKNOWN;
    }

    if (m_sensorInfo.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_B &&
         m_sensorInfo.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_R) {
        return IMAGE_SENSOR_TYPE_RAW;
    }
    else if (m_sensorInfo.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW8_B &&
         m_sensorInfo.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW8_R) {
        return IMAGE_SENSOR_TYPE_RAW8;
    }
    else if (m_sensorInfo.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_UYVY &&
                m_sensorInfo.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YVYU) {
        return IMAGE_SENSOR_TYPE_YUV;
    }
    else if (m_sensorInfo.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_CbYCrY &&
                m_sensorInfo.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YCrYCb) {
        return IMAGE_SENSOR_TYPE_YCBCR;
    }
    else if (m_sensorInfo.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_RWB_B &&
         m_sensorInfo.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_RWB_R) {
        return IMAGE_SENSOR_TYPE_RAW;
    }
    else if (m_sensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_RAW_MONO) {
        return IMAGE_SENSOR_TYPE_RAW;
    }
    else if (m_sensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_RAW8_MONO) {
        return IMAGE_SENSOR_TYPE_RAW8;
    }
    else {
        return IMAGE_SENSOR_TYPE_UNKNOWN;
    }

    return IMAGE_SENSOR_TYPE_UNKNOWN;
}

