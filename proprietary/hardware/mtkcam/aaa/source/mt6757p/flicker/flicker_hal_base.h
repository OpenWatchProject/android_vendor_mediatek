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

#ifndef _FLICKER_HAL_BASE_H_
#define _FLICKER_HAL_BASE_H_

#include "af_param.h"


#define HAL_FLICKER_ERROR    -1
#define HAL_FLICKER_AUTO_50HZ    50
#define HAL_FLICKER_AUTO_60HZ    60
#define HAL_FLICKER_AUTO_OFF       0

/*******************************************************************************
*
********************************************************************************/

namespace NS3Av3
{
struct FlickerInput
{
    int aeExpTime;
    AF_FULL_STAT_T afFullStat; //for 6x6 focus value
    MVOID *pBuf;
};

struct FlickerOutput
{
    int flickerResult;
};



class FlickerHalBase {
public:
    static FlickerHalBase& getInstance();
    int setTGSize(int sensorDev, int width, int height);
    int setSensorMode(int sensorDev, int i4NewSensorMode, int width, int height);
    int createBuf(int sensorDev);
    int releaseBuf(int sensorDev);
    int open(int sensorDev, int sensorId, int tgInfo);
    int close(int sensorDev);
    int update(int sensorDev, FlickerInput* in, FlickerOutput* out);
    int setFlickerMode(int sensorDev, int mode);
    int getFlickerResult(int& flickerResult);
    int getFlickerResult(int sensorDev, int& flickerResult);

    MBOOL Start(int sensorDev);
    MBOOL Stop(int sensorDev);
    int setUniInfo(int sensorDev, MBOOL const i4UniInfo);
    MRESULT Init(int sensorDev, MINT32 const i4SensorIdx);
    MRESULT Uninit(int sensorDev);

protected:
    virtual ~FlickerHalBase(){};

};

};



#endif

