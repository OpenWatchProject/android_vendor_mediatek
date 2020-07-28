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

#define LOG_TAG "MtkCam/HwPipeline"

#include "IHalSensorAdapter.h"
#include "MyUtils.h"
//
#if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
#include <mtkcam/hal/IHalSensor.h>
#endif
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("%d[HalSensorAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("%d[HalSensorAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("%d[HalSensorAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("%d[HalSensorAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("%d[HalSensorAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[HalSensorAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[HalSensorAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
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
class HalSensorAdapter : public IHalSensorAdapter
{
protected:  ////            Data Members.
    int32_t                 mId;
    std::string const       mName;

#if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
    NSCam::IHalSensor*      mHalSensor = nullptr;
#endif

public:     ////    Operations.

                    HalSensorAdapter(int32_t id, std::string const& name)
                        : mId(id)
                        , mName(name)
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
                        auto pHalSensorList = NSCam::IHalSensorList::get();
                        if  ( pHalSensorList == nullptr ) {
                            MY_LOGE("Bad HalSensorList");
                            return false;
                        }
                        //
                        MUINT const sensorIndex = mId;
                        mHalSensor = pHalSensorList->createSensor(mName.c_str(), sensorIndex);
                        MY_LOGE_IF(!mHalSensor, "Bad mHalSensor");
                        return (nullptr != mHalSensor);
                    #else
                        return true;
                    #endif
                    }

    virtual auto    onLastStrongRef(const void* /*id*/) -> void override
                    {
                        CAM_TRACE_NAME("Sensor destruction");
                    #if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
                        if  ( mHalSensor ) {
                            mHalSensor->destroyInstance(mName.c_str());
                            mHalSensor = nullptr;
                        }
                    #endif
                    }

    virtual auto    powerOn() -> bool
                    {
                        CAM_TRACE_NAME("Sensor powerOn");
                    #if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
                        if  ( ! mHalSensor ) {
                            MY_LOGE("Bad mHalSensor");
                            return false;
                        }
                        //
                        MUINT const sensorIndex = mId;
                        if  ( ! mHalSensor->powerOn(mName.c_str(), 1, &sensorIndex) ) {
                            MY_LOGE("Fail to power on the sensor %u", sensorIndex);
                            return false;
                        }
                    #endif
                        return true;
                    }

    virtual auto    powerOff() -> void
                    {
                        CAM_TRACE_NAME("Sensor powerOff");
                    #if '1'==MTKCAM_HAVE_HAL_SENSOR_ADAPTER
                        if  ( mHalSensor ) {
                            MY_LOGD("HalSensor:%p", mHalSensor);
                            MUINT const sensorIndex = mId;
                            mHalSensor->powerOff(mName.c_str(), 1, &sensorIndex);
                        }
                    #endif
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
auto
IHalSensorAdapter::
create(
    int32_t id,
    std::string const& name
    ) -> android::sp<IHalSensorAdapter>
{
    android::sp<HalSensorAdapter> p = new HalSensorAdapter(id, name);
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

