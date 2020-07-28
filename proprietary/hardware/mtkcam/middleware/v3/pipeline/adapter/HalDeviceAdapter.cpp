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
#define LOG_TAG "MtkCam/HwPipeline/DeviceAdapter"
//
#include "IHalDeviceAdapter.h"
#include "MyUtils.h"
//
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
#include <mtkcam/drv/IHalSensor.h>
#endif
#include "IHal3AAdapter.h"
#include <future>
#include <vector>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
class HalDeviceAdapter : public IHalDeviceAdapter
{
protected:  ////            Data Members.
    int32_t                 mId;
    std::string const       mName;
    int32_t                 mInitSuccess;

#if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
    std::vector<NSCam::IHalSensor*>      mvHalSensor;
#endif
    std::vector<::android::sp<IHal3AAdapter>>      mvHal3A;
    std::vector<MINT32>                     mvIndex;

public:     ////    Operations.

                    HalDeviceAdapter(int32_t id, std::string const& name)
                        : mId(id)
                        , mName(name)
                        , mInitSuccess(false)
                    {
                    #if '0'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
                        MY_LOGW("'0'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER");
                    #endif
                        MY_LOGD("%p", this);
                    }

    virtual auto    init() -> bool
                    {
                        CAM_TRACE_NAME("Sensor creation");
                    #if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
                        auto pHalDeviceList = MAKE_HalLogicalDeviceList();
                        auto pHalSensorList = MAKE_HalSensorList();
                        if  ( pHalDeviceList == nullptr ) {
                            MY_LOGE("Bad pHalDeviceList");
                            return false;
                        }
                        if  ( pHalSensorList == nullptr ) {
                            MY_LOGE("Bad HalSensorList");
                            return false;
                        }
                        //
                        mvIndex = pHalDeviceList->getSensorId(mId);
                        mInitSuccess = true;
                        for (MINT32 i = 0; i < mvIndex.size(); i++)
                        {
                            NSCam::IHalSensor* pSensor = pHalSensorList->createSensor(mName.c_str(), mvIndex[i]);
                            mvHalSensor.push_back(pSensor);
                            if (pSensor == nullptr)
                            {
                               mInitSuccess = false;
                            }
                        }
                        MY_LOGE_IF(!mInitSuccess, "Bad HalSensor");
                        return mInitSuccess;
                    #else
                        return true;
                    #endif
                    }

    virtual auto    onLastStrongRef(const void* /*id*/) -> void override
                    {
                        CAM_TRACE_NAME("Sensor destruction");
                    #if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
                        for (MINT32 i = 0; i < mvHalSensor.size(); i++)
                        {
                            mvHalSensor[i]->destroyInstance(mName.c_str());
                        }
                        mvHalSensor.clear();
                        mvHal3A.clear();
                    #endif
                    }

    virtual auto    powerOn() -> bool
                    {
                        CAM_TRACE_NAME("LogicalDev powerOn");
                        NSCam::Utils::CamProfile profile(__FUNCTION__, mName.c_str());
                    #if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
                        if  ( !mInitSuccess ) {
                            MY_LOGE("Bad HalSensor");
                            return false;
                        }
                        // create thread to power on sensors
                        std::future<bool> future_initSensor =
                            std::async(std::launch::async,
                                [ this ]() {
                                    CAM_TRACE_NAME("Sensors powerOn");
                                    ::prctl(PR_SET_NAME, (unsigned long)"LogicalDevPoweron", 0, 0, 0);
                                    //
                                    for (MINT32 i = 0; i < mvIndex.size(); i++)
                                    {
                                        MUINT const sensorIndex = mvIndex[i];
                                        if ( (mvHalSensor[i] == nullptr) || (!mvHalSensor[i]->powerOn(mName.c_str(), 1, &sensorIndex)) )
                                        {
                                            return false;
                                        }
                                    }
                                    //
                                    return true;
                                }
                            );
                        // init 3A and poweron 3A
                        bool success_sensorPowerOn = false;
                        bool success_init3A = true;
                        for (MINT32 i = 0; i < mvIndex.size(); i++)
                        {
                            mvHal3A.push_back(IHal3AAdapter::create(mvIndex[i], mName.c_str()));
                            profile.print("3A Hal -");
                        }

                        //(3) Wait for Sensor
                        {
                            success_sensorPowerOn = future_initSensor.get();
                            if  ( ! success_sensorPowerOn ) {
                                return false;
                            }
                            profile.print("Sensor powerOn -");
                        }

                        //(4) Notify 3A of Power On
                        for (MINT32 i = 0; i < mvHal3A.size(); i++)
                        {
                            if (mvHal3A[i] != nullptr)
                            {
                                mvHal3A[i]->notifyPowerOn();
                            }
                            else
                            {
                                success_init3A = false;
                                break;
                            }
                        }
                        profile.print("3A notifyPowerOn -");

                        return  ( success_init3A && success_sensorPowerOn );
                        //
                    #endif
                        return true;
                    }

    virtual auto    powerOff() -> void
                    {
                        CAM_TRACE_NAME("LogicalDev powerOff");
                    #if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
                        for (MINT32 i = 0; i < mvIndex.size(); i++)
                        {
                            MUINT const sensorIndex = mvIndex[i];
                            mvHal3A[i]->notifyPowerOff();
                            mvHalSensor[i]->powerOff(mName.c_str(), 1, &sensorIndex);
                        }
                    #endif
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
auto
IHalDeviceAdapter::
create(
    int32_t id,
    std::string const& name
    ) -> android::sp<IHalDeviceAdapter>
{
    android::sp<HalDeviceAdapter> p = new HalDeviceAdapter(id, name);
    if  ( p == nullptr ) {
        CAM_LOGE("Fail to new HalSensorAdapter");
        return nullptr;
    }

    if  ( ! p->init() ) {
        p = nullptr;
        return nullptr;
    }
    return p;
}

