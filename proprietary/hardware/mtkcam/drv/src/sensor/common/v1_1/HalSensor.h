/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_HALSENSOR_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_HALSENSOR_H_
//
#include <mtkcam/drv/IHalSensor.h>
#include "iseninf_drv.h"
#include "imgsensor_drv.h"
#include <mtkcam/utils/exif/IBaseCamExif.h>

namespace NSCam {
namespace NSHalSensor {

class HalSensorList;

/******************************************************************************
 *  Hal Sensor.
 ******************************************************************************/
class HalSensor : public IHalSensor
{
    typedef struct {
        IMGSENSOR_SENSOR_IDX     sensorIdx;
        SENINF_CSI_INFO         *pCsiInfo;
        ACDK_SENSOR_INFO_STRUCT *pInfo;
        const ConfigParam       *pConfigParam;
    } HALSENSOR_SENINF_CSI;

private:
    /*
     * For recording global power on count by sensor
     */
    static Mutex sSensorPowerOnMutex[IMGSENSOR_SENSOR_IDX_MAX_NUM];
    static volatile int sSensorPowerOnCount[IMGSENSOR_SENSOR_IDX_MAX_NUM];

    static int sensorPowerOnCountInc(char const *szCallerName, IMGSENSOR_SENSOR_IDX sensorIdx);

    static int sensorPowerOnCountDec(char const *szCallerName, IMGSENSOR_SENSOR_IDX sensorIdx);

    static int getSensorPowerOnCount(IMGSENSOR_SENSOR_IDX sensorIdx);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IHalSensor Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    /**
     * Destroy this instance created from IHalSensorList::createSensor.
     */
    virtual MVOID                   destroyInstance(
                                        char const* szCallerName
                                    );

public:     ////                    Operations.

    /**
     * Turn on/off the power of sensor(s).
     */
    virtual MBOOL                    powerOn(
                                           char  const *szCallerName,
                                           MUINT const  uCountOfIndex,
                                           MUINT const *pArrayOfIndex
                                    );
    virtual MBOOL                    powerOff(
                                          char  const *szCallerName,
                                          MUINT const  uCountOfIndex,
                                          MUINT const *pArrayOfIndex
                                    );


    /**
     * Configure the sensor(s).
     */
    virtual MBOOL                   configure(
                                        MUINT const         uCountOfParam,
                                        ConfigParam const*  pConfigParam
                                    );
    /**
     * Configure the sensor(s).
     */
    virtual MINT                    sendCommand(
                                        MUINT    indexDual,
                                        MUINTPTR cmd,
                                        MUINTPTR arg1,
                                        MUINTPTR arg2,
                                        MUINTPTR arg3
                                    );

    /**
     * Query sensorDynamic information after calling configure
      */
    virtual MBOOL                   querySensorDynamicInfo(
                                        MUINT32            indexDual,
                                        SensorDynamicInfo *pSensorDynamicInfo
                                    );
   /**
     * Sensor set exif debug information.
     */

    virtual MINT32                  setDebugInfo(IBaseCamExif *pIBaseCamExif);
    virtual MINT32                  reset();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  HalSensor Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~HalSensor();
                                    HalSensor();

public:     ////                    Operations.
    virtual MBOOL                   isMatch(
                                        SortedVector<MUINT>const& vSensorIndex
                                    ) const;

    virtual MVOID                   onDestroy();
    virtual MBOOL                   onCreate(
                                        SortedVector<MUINT>const& vSensorIndex
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    class PerData : public RefBase
    {
    private:    ////        Disallowed.
                            PerData(PerData const&);
        PerData&            operator=(PerData const&);

    public:     ////        Operations.
                            PerData();

    public:     ////        Fields.
    };

    typedef sp<PerData>                     PerDataPtr;
    typedef KeyedVector<MUINT, PerDataPtr>  DataMap_t;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

protected:
    mutable Mutex mMutex;
    DataMap_t     mSensorDataMap;
    SensorDynamicInfo mSensorDynamicInfo;
    IMGSENSOR_SENSOR_IDX mSensorIdx;
    int mPowerOnCount; /* Recording object power on successfuly count. */

    MUINT32 mScenarioId;
    MUINT32 mHdrMode;
    MUINT32 mPdafMode;
    MUINT32 mFramerate;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Control Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

protected:
    virtual MINT configureMipi(HALSENSOR_SENINF_CSI &csi);
    virtual MINT configureSerial(HALSENSOR_SENINF_CSI &csi);
    virtual MINT configureParallel(HALSENSOR_SENINF_CSI &csi);
    virtual MINT setSensorMclk(IMGSENSOR_SENSOR_IDX sensorIdx, MINT32 pcEn);
    virtual MINT setSensorMclkDrivingCurrent(IMGSENSOR_SENSOR_IDX sensorIdx);
    virtual SENINF_MUX_ENUM seninfMUXArbitration(SENINF_MUX_ENUM searchStart, SENINF_MUX_ENUM searchEnd);
    virtual MINT seninfMUXReleaseAll(CUSTOM_CFG_CSI_PORT port);
    virtual MINT seninfLowPowerConfigure(MSDK_SCENARIO_ID_ENUM scenarioId);
};
};  //namespace NSHalSensor
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_HALSENSOR_H_

