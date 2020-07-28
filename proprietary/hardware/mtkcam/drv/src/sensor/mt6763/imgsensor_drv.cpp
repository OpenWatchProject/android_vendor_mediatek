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
    LOG_MSG("[ImgSensorDrv] getInstance \n");
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

    LOG_MSG ("[~ImgSensorDrv]\n");
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::searchSensor(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    UCHAR cBuf[64];
    MSDK_SENSOR_INIT_FUNCTION_STRUCT *pSensorInitFunc;

    //! If imp sensor search process already done before,
    //! only need to return the sensorDevs, not need to
    //! search again.
    if (m_sensorIdx[sensorIdx] != BAD_SENSOR_INDEX) {
        //been processed.
        LOG_MSG("[searchSensor] Already processed \n");

        return SENSOR_ALREADY_SEARCH;
    }

    GetSensorInitFuncList(&pSensorInitFunc);

    LOG_MSG("SENSOR search start \n");

    //set sensor driver
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;
    MUINT32                           featureParaLen = sizeof(MUINT32);
    MINT32                            idx = 0;

    featureControl(sensorIdx, SENSOR_FEATURE_SET_DRIVER, (MUINT8 *)&idx, &featureParaLen);
    LOG_MSG("set sensor driver id =%x\n", idx);

    if((m_sensorIdx[sensorIdx] = (idx < 0) ? UNKNOWN_SENSOR_INDEX : idx) >= UNKNOWN_SENSOR_INDEX)
        return SENSOR_INVALID_DRIVER;

    NSFeature::SensorInfoBase* pSensorInfo = pSensorInitFunc[idx].pSensorInfo;

    if (pSensorInfo)
        LOG_MSG("found <%#x/%s/%s>", pSensorInfo->GetID(), pSensorInfo->getDrvName(), pSensorInfo->getDrvMacroName());
    else
        LOG_WRN("m_pSensorInfo[%d]==NULL\n", sensorIdx);

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
            LOG_ERR("[init]: error opening %s: %s \n", cBuf, strerror(errno));
            return SENSOR_INVALID_DRIVER;
        }
    }
    android_atomic_inc(&mUsers);
    LOG_MSG("[init] imgsensor_drv %d",mUsers);

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
    LOG_MSG("[uninit] imgsensor_drv %d\n", mUsers);

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

    MUINT32 i = 0;
    ACDK_SENSOR_EXPOSURE_WINDOW_STRUCT ImageWindow;
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;

    ACDK_SENSOR_CONTROL_STRUCT sensorCtrl;

    MUINT16 pFeaturePara16[2] = {0};
    MUINT32 FeaturePara32;
    MUINT32 FeatureParaLen;

//    if(SENSOR_SCENARIO_ID_UNNAMED_START != scenarioconf.sId) {//ToDo: remove
//    }

    //if(ACDK_SCENARIO_ID_MAX != sId[i]) {
    //FPS
    FeaturePara32 = scenarioconf.InitFPS;
    FeatureParaLen = sizeof(MUINTPTR);
    ret = featureControl(sensorIdx, SENSOR_FEATURE_SET_FRAMERATE,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
    if (ret < 0) {
         LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_FRAMERATE error\n");
         return SENSOR_UNKNOWN_ERROR;
    }
    //HDR Enable
    //if(scenarioconf.HDRMode <= 1)
    {
        FeaturePara32 = scenarioconf.HDRMode;
        FeatureParaLen = sizeof(MUINTPTR);
        ret = featureControl(sensorIdx, SENSOR_FEATURE_SET_HDR,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
             LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_FRAMERATE error\n");
             return SENSOR_UNKNOWN_ERROR;
        }
    }
    /* PDAF Type */
    {
        FeaturePara32 = scenarioconf.PDAFMode;
        FeatureParaLen = sizeof(MUINTPTR);
        ret = featureControl(sensorIdx, SENSOR_FEATURE_SET_PDAF,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
             LOG_ERR("[setScenario]: SENSOR_FEATURE_SET_FRAMERATE error\n");
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
        LOG_ERR("[setScenario]Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }
    //get exposure line time for each scenario
    FeatureParaLen = sizeof(MUINTPTR);
    ret = featureControl(sensorIdx,SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);//ToDo: remove

    if (ret < 0) {
       LOG_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
       return SENSOR_UNKNOWN_ERROR;
    }

    FeatureParaLen = sizeof(pFeaturePara16);
    ret = featureControl(sensorIdx, SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);
    if (ret < 0) {
        LOG_ERR("[setScenario]: SENSOR_FEATURE_GET_PERIOD error\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    if (FeaturePara32) {
        if(FeaturePara32 >= 1000) {
            m_LineTimeInus[sensorIdx] = (MUINT32)(((MUINT64)pFeaturePara16[0]*1000000 + ((FeaturePara32/1000)-1))/(FeaturePara32/1000));   // 1000 base , 33657 mean 33.657 us
            m_MinFrameLength[sensorIdx] = (MUINT32)pFeaturePara16[1];
        }
        else {
            LOG_ERR("[setScenario]: Sensor clock too slow = %d %d\n", FeaturePara32, pFeaturePara16[0]);
        }

        LOG_MSG("[setScenario]DevID = %d, m_LineTimeInus = %d Scenario id = %d, PixelClk = %d, PixelInLine = %d, Framelength = %d\n",
            sensorIdx, m_LineTimeInus[sensorIdx], scenarioconf.sId, FeaturePara32, pFeaturePara16[0], pFeaturePara16[1]);
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
    MUINTPTR* parg3Addr = (MUINTPTR*)parg3;

    MINT32 err = SENSOR_NO_ERROR;
    MUINT32  LineTimeInus = 0;
    MUINT32  MinFrameLength = 0;

    ACDK_SENSOR_FEATURE_ENUM FeatureId = SENSOR_FEATURE_BEGIN;
    MUINT8 *pFeaturePara = NULL; // Pointer to feature data that communicate with kernel
    MUINT32 FeatureParaLen = 0; // The length of feature data

    MUINT64 FeaturePara[4] = {0}; // Convert input paras to match the format used in kernel
    MUINT64  u8temp = 0;
    bool cmdBeforePowerOn = false;

    if (m_fdSensor == -1) {
        LOG_MSG("sendCommand before init, cmd = 0x%x\n", cmd);
        char cBuf[64];
        sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
        if ((m_fdSensor = ::open(cBuf, O_RDWR)) < 0) {
            LOG_ERR("[init]: error opening %s: %s \n", cBuf, strerror(errno));
            return SENSOR_INVALID_DRIVER;
        }
        cmdBeforePowerOn = true;
    }

//#define SENDCMD_LOG // Open sendcommand log, for test only
    switch (cmd) {
    case CMD_SENSOR_SET_SENSOR_EXP_TIME:
        u8temp = *parg1;/*64bit extend */
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        LineTimeInus = m_LineTimeInus[sensorIdx];
        FeaturePara[0] = ((1000 * (u8temp)) / LineTimeInus);
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %llu %d\n", (long long)FeaturePara[0], LineTimeInus);
            FeaturePara[0] = 1;
        }
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_EXP_LINE:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_EXP_LINE] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_GAIN:
        FeatureId = SENSOR_FEATURE_SET_GAIN;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[0] >>= 4;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_DUAL_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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
            LOG_MSG("[CMD_SENSOR_SET_SENSOR_SYNC] LineTimeInus = %llu %d\n", (long long)FeaturePara[2] , LineTimeInus);
            FeaturePara[2]  = 1;
        }
        FeaturePara[2] = (FeaturePara[2] ) | (((MUINT64)(*(parg1+3))>>4) << 16); // Sensor gain from 10b to 6b base
        FeaturePara[3] = *(parg1+4);  // Delay frame cnt
        FeatureParaLen = sizeof(MUINT64) * 4;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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
        LOG_MSG("[CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_TEST_PATTERN_OUTPUT:
        FeatureId = SENSOR_FEATURE_SET_TEST_PATTERN;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_TEST_PATTERN_OUTPUT] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_ESHUTTER_GAIN:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER_GAIN;
        FeaturePara[0] = *parg1; // exposure time (us)
        LineTimeInus = m_LineTimeInus[sensorIdx];
        FeaturePara[0] = ((1000 * FeaturePara[0] ) / LineTimeInus);

        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_ESHUTTER_GAIN] m_LineTime = %llu %d\n", (long long)FeaturePara[0] , LineTimeInus);
            FeaturePara[0]  = 1;
        }
        FeaturePara[2] = (FeaturePara[0] ) | (((MUINT64)(*(parg1+1))>>4) << 16); // Sensor gain from 10b to 6b base
        FeaturePara[0] = 0; // RAW Gain R, Gr
        FeaturePara[1] = 0;  // RAW Gain Gb, B
        FeaturePara[3] = 0;  // Delay frame cnt
        LOG_MSG("CMD_SENSOR_SET_ESHUTTER_GAIN: Exp=%d, SensorGain=%d\n", (MUINT32)FeaturePara[2]&0x0000FFFF, (MUINT32)FeaturePara[2]>>16);
        FeatureParaLen = sizeof(MUINT64) * 4;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_ESHUTTER_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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
            LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] LineTimeInus = %llu %d\n", (long long)FeaturePara[0], LineTimeInus);
            FeaturePara[0] = 1;
        }
        if(FeaturePara[1] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] LineTimeInus = %llu %d\n", (long long)FeaturePara[1], LineTimeInus);
            FeaturePara[1] = 1;
        }
        LOG_MSG("[SET_SENSOR_EXP_FRAME_TIME] ExpTime/FrmTime=(%u/%u) ExpLine/FrmLength/LineTimeInus/MinFrmLength=(%llu/%llu/%u/%u)\n", \
        *parg1, *parg2, (long long)FeaturePara[0], (long long)FeaturePara[1], LineTimeInus, MinFrameLength);

        if(MinFrameLength > FeaturePara[1]) {
            LOG_ERR("[SET_SENSOR_EXP_FRAME_TIME] Error: Set framelength (%llu) is smaller than MinFrmLength (%u)!!!", (long long)FeaturePara[1], MinFrameLength);
            err = SENSOR_INVALID_PARA;
            return err;
        }
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE:
        u8temp = *parg1; /*64bit extend */
        FeatureId = SENSOR_FEATURE_SET_SHUTTER_BUF_MODE;
        LineTimeInus = m_LineTimeInus[sensorIdx];

        FeaturePara[0] = ((1000 * (u8temp)) / LineTimeInus);
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE] LineTimeInus = %llu %d\n", (long long)FeaturePara[0], LineTimeInus);
            FeaturePara[0] = 1;
        }
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
    LOG_MSG("[CMD_SENSOR_SET_SENSOR_EXP_TIME_BUF_MODE] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
    break;

    case CMD_SENSOR_SET_SENSOR_EXP_LINE_BUF_MODE:
        FeatureId = SENSOR_FEATURE_SET_SHUTTER_BUF_MODE;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_ERR("[CMD_SENSOR_SET_SENSOR_EXP_LINE_BUF_MODE] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_SENSOR_GAIN_BUF_MODE:
        FeatureId = SENSOR_FEATURE_SET_GAIN_BUF_MODE;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[0] >>= 4;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_GAIN_BUF_MODE] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_I2C_BUF_MODE_EN:
        FeatureId = 	SENSOR_FEATURE_SET_I2C_BUF_MODE_EN;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_I2C_BUF_MODE_EN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_STREAMING_SUSPEND:
        FeatureId = SENSOR_FEATURE_SET_STREAMING_SUSPEND;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara =  (MUINT8*)FeaturePara;
        //LOG_MSG("SET_STREAMING_SUSPEND, senDev(%d)\n", sensorIdx);
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
        if (FeaturePara[0] != 0) {
            LOG_MSG("SET_STREAMING_RESUME, senDev(%d),expTime_us(%6d),LineTimeInus(%d),expLine(%llu)\n", \
                sensorIdx, (MUINT32)*parg1, LineTimeInus, (long long)FeaturePara[0]);
        } else {
            LOG_MSG("SET_STREAMING_RESUME, senDev(%d),expTime_us is default, keep original value\n", \
                sensorIdx);
        }
        break;

    case CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT:
        {
#ifdef SENDCMD_LOG
            LOG_MSG("[CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT] *parg1 = %d \n", (MUINT32)*parg1);
#endif
            return err;
        }
        break;

    case CMD_SENSOR_GET_INPUT_BIT_ORDER:
        // Bit 0~7 as data input
        switch(sensorIdx)
        {
            using namespace NSCamCustomSensor;
            case IMGSENSOR_SENSOR_IDX_MAIN:
                *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor0);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB:
                *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor1);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_MAIN2:
                *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor2);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB2:
                *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor3);
                err = 0;
                break;
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_INDATA_FORMAT> - bad sensor id(%x)", (int)sensorIdx);
                *parg1 = 0;
                err = -1;
                break;
        }
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_INPUT_BIT_ORDER] *parg1 = %d \n", (MUINT32)*parg1);
#endif
        return  err;
        break;

    case CMD_SENSOR_GET_PAD_PCLK_INV:
        switch(sensorIdx)
        {
            using namespace NSCamCustomSensor;
            case IMGSENSOR_SENSOR_IDX_MAIN:
                *parg1 = getSensorPadPclkInv(eDevId_ImgSensor0);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB:
                *parg1 = getSensorPadPclkInv(eDevId_ImgSensor1);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_MAIN2:
                *parg1 = getSensorPadPclkInv(eDevId_ImgSensor2);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB2:
                *parg1 = getSensorPadPclkInv(eDevId_ImgSensor2);
                err = 0;
                break;
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_PAD_PCLK_INV> - bad sensor id(%x)", (int)sensorIdx);
                *parg1 = 0;
                err = -1;
                break;
        }

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_PAD_PCLK_INV] *parg1 = %d \n", (MUINT32)*parg1);
#endif
        return  err;
        break;

    case CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE:
        using namespace NSCamCustomSensor;
        NSCamCustomSensor::SensorOrientation_T orientation;
        orientation = NSCamCustomSensor::getSensorOrientation();
        switch(sensorIdx)
        {
            case IMGSENSOR_SENSOR_IDX_MAIN:
                *parg1 = orientation.u4Degree_0;
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB:
                *parg1 = orientation.u4Degree_1;
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_MAIN2:
                *parg1 = orientation.u4Degree_2;
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB2:
                *parg1 = orientation.u4Degree_3;
                err = 0;
                break;
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_VIEWANGLE> - bad sensor id(%x)", (int)sensorIdx);
                *parg1 = 0;
                err = -1;
                break;
        }
        return err;
        break;

    case CMD_SENSOR_GET_SENSOR_FACING_DIRECTION:
        switch(sensorIdx)
        {
            using namespace NSCamCustomSensor;
            case IMGSENSOR_SENSOR_IDX_MAIN:
                *parg1 = getSensorFacingDirection(eDevId_ImgSensor0);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB:
                *parg1 = getSensorFacingDirection(eDevId_ImgSensor1);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_MAIN2:
                *parg1 = getSensorFacingDirection(eDevId_ImgSensor2);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB2:
                *parg1 = getSensorFacingDirection(eDevId_ImgSensor3);
                err = 0;
                break;
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_FACING_DIRECTION> - bad sensor id(%x)", (int)sensorIdx);
                *parg1 = 0;
                err = -1;
                break;
        }
        return  err;

        break;

    case CMD_SENSOR_GET_PIXEL_CLOCK_FREQ:
        FeatureId = SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_PIXEL_CLOCK_FREQ] parg1 = %p \n", parg1);
#endif
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
        LOG_MSG("[CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
        LOG_MSG("FeaturePara %d\n",(FeaturePara[1]));
        LOG_MSG("framerate = %d",(MUINT32)(*parg2));
#endif
        break;

    case CMD_SENSOR_GET_FAKE_ORIENTATION:
         return 0;
     break;
    case CMD_SENSOR_GET_SENSOR_VIEWANGLE:
        using namespace NSCamCustomSensor;
        NSCamCustomSensor::SensorViewAngle_T viewangle;
        viewangle = NSCamCustomSensor::getSensorViewAngle();
        switch(sensorIdx)
        {
            case IMGSENSOR_SENSOR_IDX_MAIN:
                *parg1 = viewangle.MainSensorHorFOV;
                *parg2 = viewangle.MainSensorVerFOV;
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB:
                *parg1 = viewangle.SubSensorHorFOV;
                *parg2 = viewangle.SubSensorVerFOV;
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_MAIN2:
                *parg1 = viewangle.Main2SensorHorFOV;
                *parg2 = viewangle.Main2SensorVerFOV;
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB2:
                *parg1 = viewangle.Sub2SensorHorFOV;
                *parg2 = viewangle.Sub2SensorVerFOV;
                err = 0;
                break;
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_VIEWANGLE> - bad sensor id(%x)", (int)sensorIdx);
                *parg1 = 0;
                err = -1;
                break;
        }
        return err;
       break;

    case CMD_SENSOR_GET_MCLK_CONNECTION:
        switch(sensorIdx)
        {
            using namespace NSCamCustomSensor;
            case IMGSENSOR_SENSOR_IDX_MAIN:
                *parg1 = getSensorMclkConnection(eDevId_ImgSensor0);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB:
                *parg1 = getSensorMclkConnection(eDevId_ImgSensor1);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_MAIN2:
                *parg1 = getSensorMclkConnection(eDevId_ImgSensor2);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB2:
                *parg1 = getSensorMclkConnection(eDevId_ImgSensor3);
                err = 0;
                break;
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_MCLK_CONNECTION> - bad sensor id(%x)", (int)sensorIdx);
                *parg1 = 0;
                err = -1;
                break;
        }
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_MCLK_CONNECTION] *parg1 = %d \n", (MUINT32)*parg1);
#endif
        return  err;
        break;

    case CMD_SENSOR_GET_MIPI_SENSOR_PORT:
        switch(sensorIdx)
        {
            using namespace NSCamCustomSensor;
            case IMGSENSOR_SENSOR_IDX_MAIN:
                *parg1 = getMipiSensorPort(eDevId_ImgSensor0);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB:
                *parg1 = getMipiSensorPort(eDevId_ImgSensor1);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_MAIN2:
                *parg1 = getMipiSensorPort(eDevId_ImgSensor2);
                err = 0;
                break;
            case IMGSENSOR_SENSOR_IDX_SUB2:
                *parg1 = getMipiSensorPort(eDevId_ImgSensor3);
                err = 0;
                break;
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_MIPI_SENSOR_PORT> - bad sensor id(%x)", (int)sensorIdx);
                *parg1 = 0;
                err = -1;
                break;
        }
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_MIPI_SENSOR_PORT] *parg1 = %d \n", (MUINT32)*parg1);
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
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_VC_INFO:
        FeatureId = SENSOR_FEATURE_GET_VC_INFO;
        FeaturePara[0] = *parg2;
        FeaturePara[1] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_VC_INFO] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_PDAF_INFO:
        FeatureId = SENSOR_FEATURE_GET_PDAF_INFO;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = (MUINTPTR)parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_GET_PDAF_INFO]%llu %llu\n",FeaturePara[0],FeaturePara[1]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_PDAF_REG_SETTING:
		FeatureId = SENSOR_FEATURE_GET_PDAF_REG_SETTING;
		FeaturePara[0] = *parg1; //size of buff in Byte
		FeaturePara[1] = (MUINTPTR)(*parg2);//the address of pointer pointed
		FeatureParaLen = sizeof(MUINT64) * 2;
		pFeaturePara = (MUINT8*)FeaturePara;
		LOG_MSG("[CMD_SENSOR_GET_SENSOR_PDAF_REG_SETTING] 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1]);
		break;
    case CMD_SENSOR_SET_SENSOR_PDAF_REG_SETTING:
		FeatureId = SENSOR_FEATURE_SET_PDAF_REG_SETTING;
		FeaturePara[0] = *parg1; //size of buff in Byte
		FeaturePara[1] = (MUINTPTR)(*parg2);//the address of pointer pointed
		FeatureParaLen = sizeof(MUINT64) * 2;
		pFeaturePara = (MUINT8*)FeaturePara;
		LOG_MSG("[CMD_SENSOR_SET_SENSOR_PDAF_REG_SETTING] 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1]);
		break;
    case CMD_SET_PDFOCUS_AREA:
		FeatureId = SENSOR_FEATURE_SET_PDFOCUS_AREA;
		FeaturePara[0] = *parg1;	//offset
		FeaturePara[1] = *parg2;	//the address of pointer pointed
		FeatureParaLen = sizeof(MUINT64) * 2;
		pFeaturePara = (MUINT8*)FeaturePara;
		LOG_MSG("[CMD_SET_PDFOCUS_AREA] 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1]);
		break;
    case CMD_SENSOR_GET_PDAF_DATA:
        FeatureId = SENSOR_FEATURE_GET_PDAF_DATA;
        FeaturePara[0] = *parg1;//offset
        FeaturePara[1] = (MUINTPTR)(*parg2);//the address of pointer pointed
        FeaturePara[2] = *parg3;//size of buff
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_PDAF_DATA]0x%llu 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1], FeaturePara[2]);
#endif
        break;
case CMD_SENSOR_GET_4CELL_DATA:
        FeatureId = SENSOR_FEATURE_GET_4CELL_DATA;
        FeaturePara[0] = *parg1;//4CELL_CAL_TYPE_ENUM
        FeaturePara[1] = (MUINTPTR)(*parg2);//the address of pointer
        FeaturePara[2] = *parg3;//size of buff
        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_4CELL_DATA]0x%llu 0x%llu 0x%llu", (long long)FeaturePara[0], (long long)FeaturePara[1], FeaturePara[2]);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_HDR_CAPACITY:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
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
        LOG_MSG("[CMD_SENSOR_SET_YUV_FEATURE_CMD] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_YUV_SINGLE_FOCUS_MODE:
        FeatureId = SENSOR_FEATURE_SINGLE_FOCUS_MODE;
        //LOG_MSG("CMD_SENSOR_SINGLE_FOCUS_MODE\n");
        break;


    case CMD_SENSOR_SET_YUV_CANCEL_AF:
        FeatureId = SENSOR_FEATURE_CANCEL_AF;
        //LOG_MSG("CMD_SENSOR_CANCEL_AF\n");
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
        //LOG_MSG("zone_addr=0x%llu\n", FeaturePara[0]);
        break;

    case CMD_SENSOR_SET_YUV_AE_WINDOW:
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("AEzone_addr=0x%llu\n", FeaturePara[0]);
        break;

    case CMD_SENSOR_SET_YUV_GAIN_AND_EXP_LINE:
        FeatureId = SENSOR_FEATURE_SET_GAIN_AND_ESHUTTER;
        FeaturePara[0] = *parg1; //from 10b to 6b base
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_YUV_FEATURE_CMD] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;

    case CMD_SENSOR_SET_YUV_3A_CMD:
        FeatureId = SENSOR_FEATURE_SET_YUV_3A_CMD;
        FeaturePara[0] = *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("YUV_3A_CMD=0x%x\n", (MUINT32)*parg1);
        break;

    case CMD_SENSOR_GET_YUV_AF_STATUS:
        FeatureId = SENSOR_FEATURE_GET_AF_STATUS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AF_STATUS,parg1=0x%x,FeatureParaLen=0x%x,pFeaturePara=0x%x\n",
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
        //LOG_MSG("p_ref=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN:
        FeatureId = SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("p_cur=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS:
        FeatureId = SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AF_MAX_NUM_FOCUS_AREAS,p_cur=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS:
        FeatureId = SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AE_MAX_NUM_METERING_AREAS,p_cur=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_EXIF_INFO:
        FeatureId = SENSOR_FEATURE_GET_EXIF_INFO;
        FeaturePara[0] = (MUINTPTR)parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("EXIF_addr=0x%x\n", u4FeaturePara[0]);
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
        //LOG_MSG("FLASHLIGHT_INFO=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO:
        FeatureId = SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
        //LOG_MSG("TRIGGER_FLASHLIGHT=0x%x\n", (MUINT32)parg1);
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
    case CMD_SENSOR_SET_FRAMERATE:
        FeatureId = SENSOR_FEATURE_SET_FRAMERATE;
        FeatureParaLen = sizeof(MUINTPTR);
        pFeaturePara = (MUINT8*)parg1;
        break;
    case CMD_SENSOR_SET_IHDR_SHUTTER_GAIN:
        FeatureId = SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN;
        LineTimeInus = m_LineTimeInus[sensorIdx];
        FeaturePara[0] = (1000 * (MUINT64)(*parg1)) / LineTimeInus;
        FeaturePara[1] = (1000 * (MUINT64)(*parg2)) / LineTimeInus;
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] LineTimeInus = %llu %d\n", (long long)FeaturePara[0], LineTimeInus);
            FeaturePara[0] = 1;
        }
        if(FeaturePara[1] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] LineTimeInus = %llu %d\n", (long long)FeaturePara[1], LineTimeInus);
            FeaturePara[1] = 1;
        }

        FeaturePara[2]= *parg3;
        FeaturePara[2] >>= 4; //from 10b to 6b base

        FeatureParaLen = sizeof(MUINT64) * 3;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_HDR_SHUTTER:
        FeatureId = SENSOR_FEATURE_SET_HDR_SHUTTER;
        LineTimeInus = m_LineTimeInus[sensorIdx];
        FeaturePara[0] = (1000 * (MUINT64)(*parg1)) / LineTimeInus;
        FeaturePara[1] = (1000 * (MUINT64)(*parg2)) / LineTimeInus;
        if(FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_HDR_SHUTTER] LineTimeInus = %llu %d\n", (long long)FeaturePara[0],LineTimeInus);
            FeaturePara[0] = 1;
        }
        if(FeaturePara[1] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_HDR_SHUTTER] LineTimeInus = %llu %d\n", (long long)FeaturePara[1], LineTimeInus);
            FeaturePara[1] = 1;
        }

        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_HDR_SHUTTER] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_MIN_MAX_FPS:
        FeatureId = SENSOR_FEATURE_SET_MIN_MAX_FPS;
        FeaturePara[0] = *parg1;
        FeaturePara[1] = *parg2;
        FeatureParaLen = sizeof(MUINT64) * 2;
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_IHDR_SHUTTER_GAIN] FeaturePara[0] = %llu, FeaturePara[1] = %llu, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_ROLLING_SHUTTER:
       break;
    case CMD_SENSOR_SET_SENSOR_ISO:
        FeatureId = SENSOR_FEATURE_SET_ISO;
        pFeaturePara = (UINT8*)parg1;
        FeatureParaLen = sizeof(SET_SENSOR_ISO);

        break;
    case CMD_SENSOR_GET_SENSOR_SYNC_MODE_CAPACITY:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_SYNC_MODE_CAPACITY;
        FeaturePara[0] = 0;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_SYNC_MODE_CAPACITY] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_GET_SENSOR_SYNC_MODE:
        FeatureId = SENSOR_FEATURE_GET_SENSOR_SYNC_MODE;
        FeaturePara[0] = 0; // Pass the address of parg2
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_SENSOR_SYNC_MODE] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    case CMD_SENSOR_SET_SENSOR_SYNC_MODE:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_SYNC_MODE;
        FeaturePara[0]= *parg1;
        FeatureParaLen = sizeof(MUINT64);
        pFeaturePara = (MUINT8*)FeaturePara;

#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_SET_SENSOR_SYNC_MODE] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
#endif
        break;
    default:
        LOG_ERR("[sendCommand]Command ID = %d is undefined\n",cmd);
        return SENSOR_UNKNOWN_ERROR;
    }

    err= featureControl(sensorIdx, FeatureId,  (MUINT8*)pFeaturePara,(MUINT32*)&FeatureParaLen);
    if (err < 0) {
        LOG_ERR("[sendCommand] Err-ctrlCode (%s) \n", strerror(errno));
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
        *((MUINT32*)parg1)=FeaturePara[0];
        break;


    case CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO:
    case CMD_SENSOR_SET_YUV_AUTOTEST:
        *((MUINT32*)parg1)=FeaturePara[0];
        *((MUINT32*)parg2)=FeaturePara[1];
        break;
    case CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY:
    case CMD_SENSOR_GET_SENSOR_HDR_CAPACITY:
    case CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        *((MUINT32*)parg2)=FeaturePara[1];
#ifdef SENDCMD_LOG
        LOG_MSG("[CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO] FeaturePara[0] = %llu, FeaturePara[1] = %llx, FeaturePara[2] = %llu, FeaturePara[3] = %llu \n", FeaturePara[0], FeaturePara[1], FeaturePara[2], FeaturePara[3]);
        //LOG_MSG("FeaturePara %llx\n",(MUINTPTR)(&FeaturePara[1]));
        LOG_MSG("FeaturePara %d\n",(FeaturePara[1]));
        LOG_MSG("value = %d",(MUINT32)(*parg2));
#endif
    	break;
    default:
        break;
    }

    if (cmdBeforePowerOn && m_fdSensor > 0) {
        ::close(m_fdSensor);
        m_fdSensor = -1;
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
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;
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
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;
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
    MINT32 i=0;
    LOG_MSG("[getInfo],m_fdSensor = 0x%x \n", m_fdSensor);

    if (pSensorInfo == NULL || pSensorConfigData == NULL) {
        LOG_ERR("[getInfo] NULL pointer\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    getInfo.SensorId   = sensorIdx;
    getInfo.ScenarioId = (MSDK_SCENARIO_ID_ENUM)ScenarioId;
    getInfo.pInfo = pSensorInfo;
    getInfo.pConfig = pSensorConfigData;

    if ((err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GET_CONFIG_INFO , &getInfo)) < 0) {
        LOG_ERR("[getInfo]Err-ctrlCode (%s) \n", strerror(errno));
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
    IMAGESENSOR_GETINFO_STRUCT getInfo2;
    ACDK_SENSOR_INFO2_STRUCT    Info2;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT SensorResolution;

    MINT32 err                  = SENSOR_NO_ERROR;
    getInfo2.SensorId           = (MUINT32)sensorIdx;
    getInfo2.pInfo              = &Info2;
    getInfo2.pSensorResolution  = &SensorResolution;

    if ((err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GETINFO2 , &getInfo2)) < 0) {
        LOG_ERR("[getInfo2]Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }
    // Basic information
    pSensorInfo->SensorPreviewResolutionX                = Info2.SensorPreviewResolutionX;
    pSensorInfo->SensorPreviewResolutionY                = Info2.SensorPreviewResolutionY;
    pSensorInfo->SensorFullResolutionX                   = Info2.SensorFullResolutionX ;
    pSensorInfo->SensorFullResolutionY                   = Info2.SensorFullResolutionY;
    pSensorInfo->SensorClockFreq                         = Info2.SensorClockFreq;
    pSensorInfo->SensorCameraPreviewFrameRate            = Info2.SensorCameraPreviewFrameRate;
    pSensorInfo->SensorVideoFrameRate                    = Info2.SensorVideoFrameRate;
    pSensorInfo->SensorStillCaptureFrameRate             = Info2.SensorStillCaptureFrameRate;
    pSensorInfo->SensorWebCamCaptureFrameRate            = Info2.SensorWebCamCaptureFrameRate;
    pSensorInfo->SensorClockPolarity                     = Info2.SensorClockPolarity;
    pSensorInfo->SensorClockFallingPolarity              = Info2.SensorClockFallingPolarity;
    pSensorInfo->SensorClockRisingCount                  = Info2.SensorClockRisingCount;
    pSensorInfo->SensorClockFallingCount                 = Info2.SensorClockFallingCount;
    pSensorInfo->SensorClockDividCount                   = Info2.SensorClockDividCount;
    pSensorInfo->SensorPixelClockCount                   = Info2.SensorPixelClockCount;
    pSensorInfo->SensorDataLatchCount                    = Info2.SensorDataLatchCount;
    pSensorInfo->SensorHsyncPolarity                     = Info2.SensorHsyncPolarity;
    pSensorInfo->SensorVsyncPolarity                     = Info2.SensorVsyncPolarity;
    pSensorInfo->SensorInterruptDelayLines               = Info2.SensorInterruptDelayLines;
    pSensorInfo->SensorResetActiveHigh                   = Info2.SensorResetActiveHigh;
    pSensorInfo->SensorResetDelayCount                   = Info2.SensorResetDelayCount;
    pSensorInfo->SensroInterfaceType                     = Info2.SensroInterfaceType;
    pSensorInfo->SensorOutputDataFormat                  = Info2.SensorOutputDataFormat;
    pSensorInfo->SensorMIPILaneNumber                    = Info2.SensorMIPILaneNumber;
    pSensorInfo->CaptureDelayFrame                       = Info2.CaptureDelayFrame;
    pSensorInfo->PreviewDelayFrame                       = Info2.PreviewDelayFrame;
    pSensorInfo->VideoDelayFrame                         = Info2.VideoDelayFrame;
    pSensorInfo->HighSpeedVideoDelayFrame                = Info2.HighSpeedVideoDelayFrame;
    pSensorInfo->SlimVideoDelayFrame                     = Info2.SlimVideoDelayFrame;
    pSensorInfo->Custom1DelayFrame                       = Info2.Custom1DelayFrame;
    pSensorInfo->Custom2DelayFrame                       = Info2.Custom2DelayFrame;
    pSensorInfo->Custom3DelayFrame                       = Info2.Custom3DelayFrame;
    pSensorInfo->Custom4DelayFrame                       = Info2.Custom4DelayFrame;
    pSensorInfo->Custom5DelayFrame                       = Info2.Custom5DelayFrame;
    pSensorInfo->YUVAwbDelayFrame                        = Info2.YUVAwbDelayFrame;
    pSensorInfo->YUVEffectDelayFrame                     = Info2.YUVEffectDelayFrame;
    pSensorInfo->SensorGrabStartX_PRV                    = Info2.SensorGrabStartX_PRV;
    pSensorInfo->SensorGrabStartY_PRV                    = Info2.SensorGrabStartY_PRV;
    pSensorInfo->SensorGrabStartX_CAP                    = Info2.SensorGrabStartX_CAP;
    pSensorInfo->SensorGrabStartY_CAP                    = Info2.SensorGrabStartY_CAP;
    pSensorInfo->SensorGrabStartX_VD                     = Info2.SensorGrabStartX_VD;
    pSensorInfo->SensorGrabStartY_VD                     = Info2.SensorGrabStartY_VD;
    pSensorInfo->SensorGrabStartX_VD1                    = Info2.SensorGrabStartX_VD1;
    pSensorInfo->SensorGrabStartY_VD1                    = Info2.SensorGrabStartY_VD1;
    pSensorInfo->SensorGrabStartX_VD2                    = Info2.SensorGrabStartX_VD2;
    pSensorInfo->SensorGrabStartY_VD2                    = Info2.SensorGrabStartY_VD2;
    pSensorInfo->SensorGrabStartX_CST1                   = Info2.SensorGrabStartX_CST1;
    pSensorInfo->SensorGrabStartY_CST1                   = Info2.SensorGrabStartY_CST1;
    pSensorInfo->SensorGrabStartX_CST2                   = Info2.SensorGrabStartX_CST2;
    pSensorInfo->SensorGrabStartY_CST2                   = Info2.SensorGrabStartY_CST2;
    pSensorInfo->SensorGrabStartX_CST3                   = Info2.SensorGrabStartX_CST3;
    pSensorInfo->SensorGrabStartY_CST3                   = Info2.SensorGrabStartY_CST3;
    pSensorInfo->SensorGrabStartX_CST4                   = Info2.SensorGrabStartX_CST4;
    pSensorInfo->SensorGrabStartY_CST4                   = Info2.SensorGrabStartY_CST4;
    pSensorInfo->SensorGrabStartX_CST5                   = Info2.SensorGrabStartX_CST5;
    pSensorInfo->SensorGrabStartY_CST5                   = Info2.SensorGrabStartY_CST5;
    pSensorInfo->SensorDrivingCurrent                    = Info2.SensorDrivingCurrent;
    pSensorInfo->SensorMasterClockSwitch                 = Info2.SensorMasterClockSwitch;
    pSensorInfo->AEShutDelayFrame                        = Info2.AEShutDelayFrame;
    pSensorInfo->AESensorGainDelayFrame                  = Info2.AESensorGainDelayFrame;
    pSensorInfo->AEISPGainDelayFrame                     = Info2.AEISPGainDelayFrame;
    pSensorInfo->FrameTimeDelayFrame                     = Info2.FrameTimeDelayFrame;
    pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount  = Info2.MIPIDataLowPwr2HighSpeedTermDelayCount;
    pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount= Info2.MIPIDataLowPwr2HighSpeedSettleDelayCount;
    pSensorInfo->MIPIDataLowPwr2HSSettleDelayM0          = Info2.MIPIDataLowPwr2HSSettleDelayM0;
    pSensorInfo->MIPIDataLowPwr2HSSettleDelayM1          = Info2.MIPIDataLowPwr2HSSettleDelayM1;
    pSensorInfo->MIPIDataLowPwr2HSSettleDelayM2          = Info2.MIPIDataLowPwr2HSSettleDelayM2;
    pSensorInfo->MIPIDataLowPwr2HSSettleDelayM3          = Info2.MIPIDataLowPwr2HSSettleDelayM3;
    pSensorInfo->MIPIDataLowPwr2HSSettleDelayM4          = Info2.MIPIDataLowPwr2HSSettleDelayM4;
    pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount   = Info2.MIPICLKLowPwr2HighSpeedTermDelayCount;
    pSensorInfo->SensorWidthSampling                     = Info2.SensorWidthSampling;
    pSensorInfo->SensorHightSampling                     = Info2.SensorHightSampling;
    pSensorInfo->SensorPacketECCOrder                    = Info2.SensorPacketECCOrder;
    pSensorInfo->iHDR_First_IS_LE                        = Info2.IHDR_LE_FirstLine;
    pSensorInfo->SensorModeNum                           = Info2.SensorModeNum;
    /*Deskew*/
    pSensorInfo->SensorMIPIDeskew                        = Info2.SensorMIPIDeskew;
    if(Info2.IHDR_Support == 1)
    {
        pSensorInfo->iHDRSupport                         = MTRUE;
    }
    pSensorInfo->PDAF_Support                            = Info2.PDAF_Support;
    pSensorInfo->HDR_Support                             = Info2.HDR_Support;
    pSensorInfo->PerFrameCTL_Support                     = Info2.PerFrameCTL_Support;
    pSensorInfo->virtualChannelSupport = MFALSE;
    pSensorInfo->MIPIsensorType                          = Info2.MIPIsensorType;
    pSensorInfo->SettleDelayMode                         = Info2.SettleDelayMode;
    pSensorInfo->IMGSENSOR_DPCM_TYPE_PRE                 = Info2.IMGSENSOR_DPCM_TYPE_PRE;
    pSensorInfo->IMGSENSOR_DPCM_TYPE_CAP                 = Info2.IMGSENSOR_DPCM_TYPE_CAP;
    pSensorInfo->IMGSENSOR_DPCM_TYPE_VD                  = Info2.IMGSENSOR_DPCM_TYPE_VD;
    pSensorInfo->IMGSENSOR_DPCM_TYPE_VD1                 = Info2.IMGSENSOR_DPCM_TYPE_VD1;
    pSensorInfo->IMGSENSOR_DPCM_TYPE_VD2                 = Info2.IMGSENSOR_DPCM_TYPE_VD2;
    pSensorInfo->ZHDR_MODE                               = Info2.ZHDR_Mode;
    /*SCAM*/
    pSensorInfo->SCAM_DataNumber                         = Info2.SCAM_DataNumber;
    pSensorInfo->SCAM_DDR_En                              = Info2.SCAM_DDR_En;
    pSensorInfo->SCAM_CLK_INV                              = Info2.SCAM_CLK_INV;
    pSensorInfo->SCAM_DEFAULT_DELAY                      = Info2.SCAM_DEFAULT_DELAY;
    pSensorInfo->SCAM_CRC_En                             = Info2.SCAM_CRC_En;
    pSensorInfo->SCAM_SOF_src                            = Info2.SCAM_SOF_src;
    pSensorInfo->SCAM_Timout_Cali                        = Info2.SCAM_Timout_Cali;
    //Support Resolution
    pSensorInfo->SensorPreviewWidth                      = SensorResolution.SensorPreviewWidth;
    pSensorInfo->SensorPreviewHeight                     = SensorResolution.SensorPreviewHeight;
    pSensorInfo->SensorCapWidth                         = SensorResolution.SensorFullWidth;
    pSensorInfo->SensorCapHeight                        = SensorResolution.SensorFullHeight;
    pSensorInfo->SensorVideoWidth                        = SensorResolution.SensorVideoWidth;
    pSensorInfo->SensorVideoHeight                       = SensorResolution.SensorVideoHeight;
    pSensorInfo->SensorVideo1Width                        = SensorResolution.SensorHighSpeedVideoWidth;
    pSensorInfo->SensorVideo1Height                       = SensorResolution.SensorHighSpeedVideoHeight;
    pSensorInfo->SensorVideo2Width                        = SensorResolution.SensorSlimVideoWidth;
    pSensorInfo->SensorVideo2Height                       = SensorResolution.SensorSlimVideoHeight;
    pSensorInfo->SensorCustom1Width                        = SensorResolution.SensorCustom1Width;
    pSensorInfo->SensorCustom1Height                       = SensorResolution.SensorCustom1Height;
    pSensorInfo->SensorCustom2Width                        = SensorResolution.SensorCustom2Width;
    pSensorInfo->SensorCustom2Height                       = SensorResolution.SensorCustom2Height;
    pSensorInfo->SensorCustom3Width                        = SensorResolution.SensorCustom3Width;
    pSensorInfo->SensorCustom3Height                       = SensorResolution.SensorCustom3Height;
    pSensorInfo->SensorCustom4Width                        = SensorResolution.SensorCustom4Width;
    pSensorInfo->SensorCustom4Height                       = SensorResolution.SensorCustom4Height;
    pSensorInfo->SensorCustom5Width                        = SensorResolution.SensorCustom5Width;
    pSensorInfo->SensorCustom5Height                       = SensorResolution.SensorCustom5Height;
    pSensorInfo->SensorEffectivePreviewWidth             = SensorResolution.SensorEffectivePreviewWidth;
    pSensorInfo->SensorEffectivePreviewHeight            = SensorResolution.SensorEffectivePreviewHeight;
    pSensorInfo->SensorEffectiveCapWidth                = SensorResolution.SensorEffectiveFullWidth;
    pSensorInfo->SensorEffectiveCapHeight               = SensorResolution.SensorEffectiveFullHeight;
    pSensorInfo->SensorEffectiveVideoWidth               = SensorResolution.SensorEffectiveVideoWidth;
    pSensorInfo->SensorEffectiveVideoHeight              = SensorResolution.SensorEffectiveVideoHeight;
    pSensorInfo->SensorPreviewWidthOffset                = SensorResolution.SensorPreviewWidthOffset;//from effective width to output width
    pSensorInfo->SensorPreviewHeightOffset               = SensorResolution.SensorPreviewHeightOffset;//from effective height to output height
    pSensorInfo->SensorCapWidthOffset                   = SensorResolution.SensorFullWidthOffset;//from effective width to output width
    pSensorInfo->SensorCapHeightOffset                  = SensorResolution.SensorFullHeightOffset;//from effective height to output height
    pSensorInfo->SensorVideoWidthOffset                  = SensorResolution.SensorVideoWidthOffset;//from effective width to output width
    pSensorInfo->SensorVideoHeightOffset                 = SensorResolution.SensorVideoHeightOffset;//from effective height to output height
    LOG_MSG("prv w=0x%x,h=0x%x\n",pSensorInfo->SensorPreviewWidth,pSensorInfo->SensorPreviewHeight);
    LOG_MSG("cap w=0x%x,h=0x%x\n",pSensorInfo->SensorCapWidth,pSensorInfo->SensorCapHeight);
    LOG_MSG("vd  w=0x%x,h=0x%x\n",pSensorInfo->SensorVideoWidth,pSensorInfo->SensorVideoHeight);
    LOG_MSG("pre GrapX=0x%x,GrapY=0x%x\n",pSensorInfo->SensorGrabStartX_PRV,pSensorInfo->SensorGrabStartY_PRV);
    //LOG_MSG("cap GrapX=%d,GrapY=%d\n",pSensorInfo->SensorGrabStartX_CAP,pSensorInfo->SensorGrabStartY_CAP);
    //LOG_MSG("vd  GrapX=%d,GrapY=%d\n",pSensorInfo->SensorGrabStartX_VD,pSensorInfo->SensorGrabStartY_VD);
    //LOG_MSG("vd1 GrapX=%d,GrapY=%d\n",pSensorInfo->SensorGrabStartX_VD1,pSensorInfo->SensorGrabStartY_VD1);
    //LOG_MSG("vd2 GrapX=%d,GrapY=%d\n",pSensorInfo->SensorGrabStartX_VD2,pSensorInfo->SensorGrabStartY_VD2);

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
        LOG_ERR("[sendCommand]m_fdSensor fail, sendCommand must be called after init()!\n");
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
        LOG_ERR("[featureControl] Err-ctrlCode (%s) \n", strerror(errno));
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

    LOG_MSG("[getCurrentSensorType] \n");

    if (SENSOR_NO_ERROR != getInfo(sensorIdx, SENSOR_SCENARIO_ID_NORMAL_PREVIEW, &m_sensorInfo, &m_sensorConfigData)) {
       LOG_ERR("[searchSensor] Error:getInfo() \n");
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
    else {
        return IMAGE_SENSOR_TYPE_UNKNOWN;
    }

    return IMAGE_SENSOR_TYPE_UNKNOWN;
}

