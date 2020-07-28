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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_HALSENSORLIST_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_HALSENSORLIST_H_
//
#include <mtkcam/drv/IHalSensor.h>
#include "sensor_drv.h"
#include "seninf_drv.h"

extern NSCam::SensorStaticInfo sensorStaticInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSHalSensor {

/******************************************************************************
 *
 ******************************************************************************/
class HalSensor;

/******************************************************************************
 *
 ******************************************************************************/

typedef struct {
    MUINT u4BitDepth;
    MUINT u4IsPacked;
    MUINT u1Order;
    MUINT sensorType;
}SENSOR_HAL_RAW_INFO_STRUCT;



/******************************************************************************
 *  Hal Sensor List.
 ******************************************************************************/
class HalSensorList : public IHalSensorList
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IHalSensorList Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
#ifdef CONFIG_MTK_CAM_SECURE
    virtual MUINT                   enableSecure(char const *szCallerName);
    virtual MUINT                   disableSecure(char const *szCallerName);
    virtual MUINT                   querySecureState();
#endif
    /**
     * Query the number of image sensors.
     * This call is legal only after searchSensors().
     */
    virtual MUINT                   queryNumberOfSensors() const;

    /**
     * Query static information for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual IMetadata const&        queryStaticInfo(MUINT const index) const;

    /**
     * Query the driver name for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual char const*             queryDriverName(MUINT const index) const;

    /**
     * Query the sensor type of NSSensorType::Type for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual MUINT                   queryType(MUINT const index) const;

    /**
     * Query the sensor facing direction for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual MUINT                   queryFacingDirection(
                                        MUINT const index
                                    ) const;

    /**
     * Query SensorDev Index by sensor list index.
     * This call is legal only after searchSensors().
     * Return SENSOR_DEV_MAIN, SENSOR_DEV_SUB,...
     */
    virtual MUINT                   querySensorDevIdx(MUINT const index) const;

    /**
     * Query static SensorStaticInfo for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual SensorStaticInfo const* querySensorStaticInfo(
                                        MUINT const index
                                    ) const;

public:     ////                    Operations.

    /**
     * Search sensors and return the number of image sensors.
     */
    virtual MUINT                   searchSensors();

    /**
     * Create an instance of IHalSensor for a single specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual IHalSensor*             createSensor(
                                        char const* szCallerName,
                                        MUINT const index
                                    );

    /**
     * Create an instance of IHalSensor for multiple specific sensor indexes.
     * This call is legal only after searchSensors().
     */
    virtual IHalSensor*             createSensor(
                                        char const* szCallerName,
                                        MUINT const uCountOfIndex,
                                        MUINT const*pArrayOfIndex
                                    );

    /**
     * Query sneosr related information for a specific sensor indexes.
     * This call is legal only after searchSensors().
     */

    virtual MVOID                   querySensorStaticInfo(
                                        MUINT sensorDevIdx,
                                        SensorStaticInfo *pSensorStaticInfo
                                    ) const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Enum Sensor List.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.
    class EnumInfo : public Info
    {
    public:
        IMetadata mMetadata;
    };

protected:  ////                    Data Members.
    mutable Mutex                   mEnumSensorMutex;
    Vector<EnumInfo>                mEnumSensorList;

#ifdef CONFIG_MTK_CAM_SECURE
    MBOOL                           mEnableSecure;
    mutable Mutex                   mSecureSensorMutex;
#endif

protected:  ////                    Operations.
    MUINT                           enumerateSensor_Locked();
    EnumInfo const*                 addAndInitSensorEnumInfo_Locked(
                                        IMGSENSOR_SENSOR_IDX eSensorIdx,
                                        MUINT           eSensorType,
                                        char const*     szSensorDrvName
                                    );

public:     ////                    Operations.
    EnumInfo const*                 queryEnumInfoByIndex(MUINT index) const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Open Sensor List.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.
                                    struct OpenInfo
                                    {
                                        volatile MINT           miRefCount;
                                        HalSensor*              mpHalSensor;
                                        //
                                        OpenInfo();
                                        OpenInfo(MINT iRefCount, HalSensor* pHalSensor);
                                    };
    typedef List<OpenInfo>          OpenList_t;

protected:  ////                    Data Members.
    mutable Mutex                   mOpenSensorMutex;
    OpenList_t                      mOpenSensorList;

public:     ////                    Operations.
    virtual MVOID                   closeSensor(
                                        HalSensor*const pHalSensor,
                                        char const* szCallerName
                                    );
    virtual HalSensor*              openSensor(
                                        SortedVector<MUINT>const& vSensorIndex,
                                        char const* szCallerName
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    static  HalSensorList*          singleton();

protected:  ////                    Instantiation.
    virtual                         ~HalSensorList() {}
                                    HalSensorList();

protected:  ////                    Operations.
    /**
     * Build static information for a specific sensor.
     */
    virtual MBOOL                   buildStaticInfo(Info const& rInfo, IMetadata& rMetadata) const;

protected:
    virtual MVOID                 querySensorDrvInfo(IMGSENSOR_SENSOR_IDX sensorDev);

    virtual MUINT                 getRawInfo(IMGSENSOR_SENSOR_IDX sensorDevId, SENSOR_HAL_RAW_INFO_STRUCT *pInfo);

    virtual MVOID                 querySensorInfo(IMGSENSOR_SENSOR_IDX sensorDev);

    virtual MVOID                 buildSensorMetadata(IMGSENSOR_SENSOR_IDX sensorDev);
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSHalSensor
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_HALSENSORLIST_H_

