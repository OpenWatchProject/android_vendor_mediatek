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
#ifndef _SENINF_DRV_BASE_H_
#define _SENINF_DRV_BASE_H_
#include "seninf_type.h"
#include <mtkcam/drv/IHalSensor.h>

using namespace NSCamCustomSensor;
using namespace NSCam;

/*******************************************************************************
*
********************************************************************************/

typedef enum {
    CMD_SENINF_GET_SENINF_ADDR,
    CMD_SENINF_DEBUG_TASK,
    CMD_SENINF_DEBUG_TASK_CAMSV,
    CMD_SENINF_SET_DUAL_CAM_MODE,
    CMD_SENINF_DEBUG_PIXEL_METER,
    CMD_SENINF_REFRESH_DFS,
    CMD_SENINF_GET_PIXEL_MODE,
    CMD_SENINF_SYNC_REG_TO_PA,
    CMD_SENINF_CHECK_PIPE,
    CMD_SENINF_RESUME_FREE,
    CMD_SENINF_MAX
} CMD_SENINF;

class SeninfDrv {
public:
    //
    static SeninfDrv* createInstance();

#ifdef CONFIG_MTK_CAM_SECURE
    static SeninfDrv* createInstance(MBOOL isSecure);
#endif

    virtual void   destroyInstance() = 0;
protected:
    virtual ~SeninfDrv() {};

public:
    virtual int init() = 0;
    virtual int uninit() = 0;
    virtual int configMclk(SENINF_MCLK_PARA *mclk_para, unsigned long pcEn ) = 0;
    virtual int configSensorInput(SENINF_CONFIG_STRUCT *pseninfPara, SensorDynamicInfo *psensorDynamicInfo,
            ACDK_SENSOR_INFO2_STRUCT *sensorInfo, bool enable)= 0;
    virtual int calibrateSensor(SENINF_CONFIG_STRUCT *pseninfPara) = 0;
    virtual int getN3DDiffCnt(MUINT32 *pCnt) = 0;
	virtual int setAllMclkOnOff(unsigned long ioDrivingCurrent,bool enable) = 0;
    virtual int sendCommand(int cmd, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0) = 0;
    virtual int setMclkIODrivingCurrent(EMclkId mclkIdx, unsigned long ioDrivingCurrent) = 0;
    virtual int resetConfiguration(IMGSENSOR_SENSOR_IDX sensorIdx) = 0;

};

#endif // _ISP_DRV_H_

