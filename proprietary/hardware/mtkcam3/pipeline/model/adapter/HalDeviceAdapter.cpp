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

#define LOG_TAG "mtkcam-DeviceAdapter"
//
#include <impl/IHalDeviceAdapter.h>
//
#include <sys/prctl.h>
//
#include <future>
#include <vector>
//
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <mtkcam/utils/hw/IPlugProcessing.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/drv/IHalSensor.h>
#include "IHal3AAdapter.h"
#include "MyUtils.h"
#include <mtkcam/aaa/IHalISP.h>
//
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif

#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_MODEL);

//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::model;
using namespace NSCamHW;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[HalDeviceAdapter::%s] " fmt, mId, __FUNCTION__, ##arg)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {
class HalDeviceAdapter : public IHalDeviceAdapter
{
protected:  ////            Data Members.
    int32_t                 mId = -1;       //logical device ID
    std::string const       mName;

    //init
    std::vector<int32_t>    mvPhySensorId;  //physical sensor ID (index)

    //open/close
    bool                    mIsOpen = false;
    std::vector<sp<IPlugProcessing>>
                            mvpPlugProcess;

    //powerOn/powerOff
    std::vector<NSCam::IHalSensor*>             mvHalSensor;
    std::vector<::android::sp<IHal3AAdapter>>   mvHal3A;
    std::vector<NS3Av3::IHalISP*>               mvHalIsp;

public:     ////    Operations.

                    HalDeviceAdapter(int32_t id)
                        : mId(id)
                        , mName("HalDeviceAdapter:" + std::to_string(id))
                    {
                        MY_LOGD("%p %s", this, mName.c_str());
                    }

    virtual auto    init() -> bool
                    {
                        auto pHalDeviceList = MAKE_HalLogicalDeviceList();
                        if  ( CC_UNLIKELY(pHalDeviceList == nullptr) ) {
                            MY_LOGE("Bad pHalDeviceList");
                            return false;
                        }
                        //
                        mvPhySensorId = pHalDeviceList->getSensorId(mId);
                        return true;
                    }

    virtual auto    open() -> bool override
                    {
                        CAM_TRACE_NAME("Sensor creation");
                        auto pHalSensorList = MAKE_HalSensorList();
                        if  ( CC_UNLIKELY(pHalSensorList == nullptr) ) {
                            MY_LOGE("Bad HalSensorList");
                            return false;
                        }

                        if ( CC_UNLIKELY(mvPhySensorId.size() == 0) ) {
                            MY_LOGE("No physical sensor");
                            return false;
                        }
                        //
                        bool ret = true;
                        for (size_t i = 0; i < mvPhySensorId.size(); i++) {
                            auto pSensor = pHalSensorList->createSensor(mName.c_str(), mvPhySensorId[i]);
                            mvHalSensor.push_back(pSensor);
                            if ( CC_UNLIKELY(pSensor == nullptr) ) {
                                ret = false;
                                MY_LOGE("Bad HalSensor - mvPhySensorId[%zu]=%d", i, mvPhySensorId[i]);
                            }
                        }

                        if  (CC_UNLIKELY(!ret)) {
                            MY_LOGE("Fail on open(). [TODO] let's clean up resources");
                        }
                        mIsOpen = ret;
                        return ret;
                    }

    virtual auto    close() -> void override
                    {
                        CAM_TRACE_NAME("Sensor destruction");
                        for (size_t i = 0; i < mvHalSensor.size(); i++) {
                            mvHalSensor[i]->destroyInstance(mName.c_str());
                        }
                        mvHalSensor.clear();
                        mvHal3A.clear();
                        mIsOpen = false;
                        for(auto pPlugProcessing : mvpPlugProcess)
                        {
                            pPlugProcessing->uninit();
                            pPlugProcessing = NULL;
                        }
                        mvpPlugProcess.clear();
                    }

    virtual auto    powerOn() -> bool override
                    {
                        MY_LOGI("powerOn +");
                        CAM_TRACE_NAME("LogicalDev powerOn");
                        NSCam::Utils::CamProfile profile(__FUNCTION__, mName.c_str());
                        if  ( CC_UNLIKELY(!mIsOpen) ) {
                            MY_LOGE("Bad HalSensor");
                            return false;
                        }

                        // enable secure sensor
                        auto pHalDeviceList = MAKE_HalLogicalDeviceList();
                        if  ( CC_UNLIKELY(pHalDeviceList == nullptr) ) {
                            MY_LOGE("Bad pHalDeviceList");
                            return false;
                        }
                        if (pHalDeviceList->getSupportedFeature(mId) & DEVICE_FEATURE_SECURE_CAMERA)
                        {
                            MY_LOGD("enable secure sensor");
                            IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
                            if  ( CC_UNLIKELY(pHalSensorList == nullptr) ) {
                                MY_LOGE("Bad pHalSensorList");
                                return false;
                            }
                            pHalSensorList->enableSecure(LOG_TAG);
                        }

                        // create thread to power on sensors
                        std::future<bool> future_initSensor =
                            std::async(std::launch::async,
                                [ this ]() {
                                    CAM_TRACE_NAME("Sensors powerOn");
                                    ::prctl(PR_SET_NAME, (unsigned long)"LogicalDevPoweron", 0, 0, 0);
                                    //
                                    for (size_t i = 0; i < mvPhySensorId.size(); i++)
                                    {
                                        MUINT const sensorIndex = mvPhySensorId[i];
                                        if (CC_UNLIKELY( mvHalSensor[i] == nullptr ))
                                        {
                                            return false;
                                        }
                                        if (CC_UNLIKELY( !mvHalSensor[i]->powerOn(mName.c_str(), 1, &sensorIndex) ))
                                        {
                                            #if (MTKCAM_HAVE_AEE_FEATURE == 1)
                                            aee_system_exception(
                                                LOG_TAG,
                                                NULL,
                                                DB_OPT_DEFAULT,
                                                android::String8::format(
                                                    "[%zu] sensorIndex:%d powerOn failed"
                                                    "\nCRDISPATCH_KEY:MtkCam/Cam1Device:Sensor power on failed"
                                                    , i, sensorIndex).c_str()
                                            );
                                            #endif
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
                        for (size_t i = 0; i < mvPhySensorId.size(); i++)
                        {
                            mvHal3A.push_back(IHal3AAdapter::create(mvPhySensorId[i], mName.c_str()));
                            profile.print("3A Hal -");
                            mvHalIsp.push_back(MAKE_HalISP(mvPhySensorId[i], mName.c_str()));
                            profile.print("ISP Hal -");
                        }

                        //(3) Wait for Sensor
                        {
                            success_sensorPowerOn = future_initSensor.get();
                            if  ( ! success_sensorPowerOn ) {
                                MY_LOGE("powerOn fail");
                                return false;
                            }
                            profile.print("Sensor powerOn -");
                        }

                        /// create plugprocessingFcell instance and init.
                        for(auto physicalID : mvPhySensorId)
                        {
                            HwInfoHelper helper(physicalID);
                            if( ! helper.updateInfos() )
                            {
                                MY_LOGE("cannot properly update infos");
                            }
                            //
                            MSize sensorSize;
                            if( !helper.getSensorSize(SENSOR_SCENARIO_ID_NORMAL_CAPTURE, sensorSize) )
                                MY_LOGE("get sensor Size fail");
                            HwInfoHelper::e4CellSensorPattern
                                SensorPattern = helper.get4CellSensorPattern();
                            if( SensorPattern == HwInfoHelper::e4CellSensorPattern_Unpacked )
                            {
                                sp<IPlugProcessing> pPlugProcessing = IPlugProcessing::createInstance((MUINT32)IPlugProcessing::PLUG_ID_FCELL, (NSCam::IPlugProcessing::DEV_ID)physicalID);
                                PlugInitParam initParam;
                                initParam.openId = physicalID;
                                initParam.img_w = sensorSize.w;
                                initParam.img_h = sensorSize.h;
                                pPlugProcessing->sendCommand(NSCam::NSCamPlug::SET_PARAM, NSCam::IPlugProcessing::PARAM_INIT, (MINTPTR)&initParam);
                                pPlugProcessing->init(IPlugProcessing::OP_MODE_ASYNC);
                                mvpPlugProcess.push_back(pPlugProcessing);
                            }
                        }

                        //(4) Notify 3A of Power On
                        for (size_t i = 0; i < mvHal3A.size(); i++)
                        {
                            MY_LOGI("3A notifyPowerOn");
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
                        MY_LOGI("powerOn -");

                        return  ( success_init3A && success_sensorPowerOn );
                    }

    virtual auto    powerOff() -> void override
                    {
                        MY_LOGI("powerOff +");
                        CAM_TRACE_NAME("LogicalDev powerOff");
                        for (size_t i = 0; i < mvPhySensorId.size(); i++)
                        {
                            MUINT const sensorIndex = mvPhySensorId[i];
                            mvHal3A[i]->notifyPowerOff();
                            mvHalSensor[i]->powerOff(mName.c_str(), 1, &sensorIndex);
                            mvHalIsp[i]->destroyInstance(mName.c_str());
                        }
                        // disable secure sensor
                        auto pHalDeviceList = MAKE_HalLogicalDeviceList();
                        if  ( CC_UNLIKELY(pHalDeviceList == nullptr) ) {
                            MY_LOGE("Bad pHalDeviceList");
                            return;
                        }
                        if (pHalDeviceList->getSupportedFeature(mId) & DEVICE_FEATURE_SECURE_CAMERA)
                        {
                            MY_LOGD("disable secure sensor");
                            IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
                            if  ( CC_UNLIKELY(pHalSensorList == nullptr) ) {
                                MY_LOGE("Bad pHalSensorList");
                                return;
                            }
                            pHalSensorList->disableSecure(LOG_TAG);
                        }
                        mvHalIsp.clear();
                        MY_LOGI("powerOff -");
                    }

    virtual auto    getPhysicalSensorId(
                        std::vector<int32_t>& rvSensorId
                    ) const -> bool override
                    {
                        if ( mvPhySensorId.empty() )
                            return false;
                        rvSensorId = mvPhySensorId;
                        return true;
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
auto
IHalDeviceAdapter::
create(int32_t id) -> android::sp<IHalDeviceAdapter>
{
    android::sp<HalDeviceAdapter> p = new HalDeviceAdapter(id);
    if  ( CC_UNLIKELY(p == nullptr) ) {
        CAM_ULOGME("Fail on new HalSensorAdapter");
        return nullptr;
    }

    if  ( CC_UNLIKELY(!p->init()) ) {
        p = nullptr;
        return nullptr;
    }
    return p;
}


};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

