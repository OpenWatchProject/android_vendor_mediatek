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
#ifndef _IMGSENSOR_DRV_H
#define _IMGSENSOR_DRV_H

#include "sensor_drv.h"
#include "camera_custom_sensor.h"
#include "kd_camera_feature.h"
#include <utils/threads.h>
#include <utils/Errors.h>
#include <cutils/log.h>
#include <mtkcam/utils/std/ULog.h>

using namespace android;

#ifndef USING_MTK_LDVT
#define LOG_MSG(fmt, arg...)    CAM_ULOGD(NSCam::Utils::ULog::MOD_DRV_SENSOR, "[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    CAM_ULOGD(NSCam::Utils::ULog::MOD_DRV_SENSOR, "[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    CAM_ULOGE(NSCam::Utils::ULog::MOD_DRV_SENSOR, "[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
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

/*******************************************************************************
*
********************************************************************************/
class ImgSensorDrv : public SensorDrv {
public:
    static SensorDrv* getInstance();

private:
    ImgSensorDrv();
    virtual ~ImgSensorDrv();

public:
    MINT32 init();
    MINT32 uninit();

    MINT32 open(IMGSENSOR_SENSOR_IDX sensorIdx);
    MINT32 close(IMGSENSOR_SENSOR_IDX sensorIdx);

    MINT32 setScenario(IMGSENSOR_SENSOR_IDX sensorIdx, SENSOR_DRIVER_SCENARIO_T scenarioconf);

    MINT32 getInfo(IMGSENSOR_SENSOR_IDX sensorIdx, MUINT32 ScenarioId, ACDK_SENSOR_INFO_STRUCT *pSensorInfo, ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);

    MINT32 getInfo2(IMGSENSOR_SENSOR_IDX sensorIdx, SENSORDRV_INFO_STRUCT *pSensorInfo);

    MINT32 sendCommand(IMGSENSOR_SENSOR_IDX sensorIdx, MUINT32 cmd, MUINTPTR parg1 = 0, MUINTPTR parg2 = 0, MUINTPTR parg3 = 0);

    MUINT32 getSensorID(IMGSENSOR_SENSOR_IDX sensorIdx);
    IMAGE_SENSOR_TYPE getCurrentSensorType(IMGSENSOR_SENSOR_IDX sensorIdx);
    NSFeature::SensorInfoBase*  getSensorInfo(IMGSENSOR_SENSOR_IDX sensorIdx);

    MINT32 searchSensor(IMGSENSOR_SENSOR_IDX sensorIdx);
private:
    MINT32 featureControl(IMGSENSOR_SENSOR_IDX sensorIdx, ACDK_SENSOR_FEATURE_ENUM FeatureId,  UINT8 *pFeaturePara,MUINT32 *pFeatureParaLen);

public:
    static ImgSensorDrv* singleton();

private:
    int     m_fdSensor;
    mutable Mutex mLock;
    volatile int mUsers;
    UINT16  m_sensorIdx[IMGSENSOR_SENSOR_IDX_MAX_NUM];
    MUINT32 m_LineTimeInus[IMGSENSOR_SENSOR_IDX_MAX_NUM];
    MUINT32 m_MinFrameLength[IMGSENSOR_SENSOR_IDX_MAX_NUM];

    enum {
		UNKNOWN_SENSOR_INDEX = 0xFFFE,
		BAD_SENSOR_INDEX     = 0xFFFF
	};
};

#endif // _IMGSENSOR_DRV_H

