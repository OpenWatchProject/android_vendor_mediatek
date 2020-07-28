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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTIL_IHALLOGICALDEV_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTIL_IHALLOGICALDEV_H_

#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/LogicalCam/Type.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

/******************************************************************************
 *  Hal Sensor List Interface.
 ******************************************************************************/
class IHalLogicalDeviceList
{
public:     ////                    Instantiation.
    static  IHalLogicalDeviceList*          get();

protected:  ////                    Destructor.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IHalLogicalDeviceList() {}

public:     ////                    Attributes.

    /**
     * Query the number of logical devices.
     * This call is legal only after searchDevices().
     */
    virtual MUINT                   queryNumberOfDevices() const                = 0;

    /**
     * Query the number of image sensors.
     * This call is legal only after searchDevices().
     */
    virtual MUINT                   queryNumberOfSensors() const                = 0;

    /**
     * Query static information for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual IMetadata const&        queryStaticInfo(MUINT const index) const    = 0;

    /**
     * Query the driver name for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual char const*             queryDriverName(MUINT const index) const    = 0;

    /**
     * Query the sensor type of NSSensorType::Type for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual MUINT                   queryType(MUINT const index) const          = 0;

    /**
     * Query the sensor facing direction for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual MUINT                   queryFacingDirection(
                                        MUINT const index
                                    ) const                                     = 0;

    /**
     * Query SensorDev Index by sensor list index.
     * This call is legal only after searchSensors().
     * Return SENSOR_DEV_MAIN, SENSOR_DEV_SUB,...
     */
    virtual MUINT                   querySensorDevIdx(MUINT const index) const  = 0;

    /**
     * Query static SensorStaticInfo for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual SensorStaticInfo const* querySensorStaticInfo(
                                        MUINT const indexDual
                                    ) const                                     = 0;

    /**
     * Query Sensor Information.
     * This call is legal only after searchSensors().
     */
    virtual MVOID                   querySensorStaticInfo(
                                        MUINT indexDual,
                                        SensorStaticInfo *pSensorStaticInfo
                                    ) const                                     = 0;

    /**
     * Search sensors and return the number of logical devices.
     */
    virtual MUINT                   searchDevices()                             = 0;

    /**
     * get all sensor id belong this logical camera device
     * return index id list. (ex: 0,1,2)
     */
    virtual std::vector<MINT32>     getSensorId(MUINT deviceId) = 0;

    /**
     * get remapping sensor id list.
     */
    virtual std::vector<MINT32>     getRemappingSensorId(MUINT deviceId) = 0;

    /**
     * get logical device id by Remapping sensor id.
     */
    virtual MINT32                  getDeviceIdByVID(MUINT vid) = 0;

    /**
     * get logical device id for input sensor id
     */
    virtual MINT32                  getDeviceId(MINT32 sensorId) = 0;

    /**
     * get sync type
     */
    virtual SensorSyncType          getSyncType(MUINT deviceId) const = 0;

    /**
     * get supported feature in specific instance id
     */
    virtual MINT32                  getSupportedFeature(MUINT deviceId) const = 0;

    /**
     * get master dev id
     */
    virtual MINT32                  getSensorSyncMasterDevId(MUINT deviceId) const = 0;

    /**
     * get virtual instance id.
     */
    virtual uint32_t                getVirtualInstanceId(uint32_t instanceId) const = 0;

    /**
     * \brief Check if logical multicam capability is disabled by user
     * \details We'll enable logical multicam capability by default
     *          for logical device with more than one physical sensors.
     *          However, user can disable it for some reason.
     *          User can set "Disable Logical Multicam" to 1 under "Logical Device" section.
     *
     * \param deviceId The logical device ID
     * \return false if logical multicam is disabled, otherwise return true
     */
    virtual bool                    isLogicalMulticam(MUINT deviceId) const = 0;

    /**
     * get remapping sensor id list.
     */
    virtual bool                    isCameraHidden(uint32_t instanceId) const = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam

/**
 * @brief The definition of the maker of IHalLogicalDeviceList instance.
 */
typedef NSCam::IHalLogicalDeviceList* (*HalLogicalDeviceList_FACTORY_T)();

#define MAKE_HalLogicalDeviceList(...) \
    MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_UTILS_LOGICALDEV, HalLogicalDeviceList_FACTORY_T, __VA_ARGS__)


#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTIL_IHALLOGICALDEV_H_
