/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_V1_DEVICE_DEFAULTCAMERADEVICE1_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_V1_DEVICE_DEFAULTCAMERADEVICE1_H_
//
#include <pthread.h>
//
#if '1'==MTKCAM_HAVE_SENSOR_HAL
#include <mtkcam/drv/IHalSensor.h>
#endif
//
#if '1'== MTKCAM_HAVE_3A_HAL
#include <mtkcam/aaa/IHal3A.h>
#endif
//
#include <device/CameraDevice1Base.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

/******************************************************************************
 *
 ******************************************************************************/
class DefaultCameraDevice1 : public CameraDevice1Base
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    #define USER_NAME               "DefaultCameraDevice1"

    // setup during constructor

    // setup during opening camera
#if '1'==MTKCAM_HAVE_3A_HAL
    NS3Av3::IHal3A*                   mpHal3a;
#endif

#if '1'==MTKCAM_HAVE_SENSOR_HAL
    NSCam::IHalSensor*              mpHalSensor;
#endif

    // sensor_power_on thread related
    pthread_t                       mThreadHandle;
    bool                            mbThreadRunning;
    bool                            mRet;

    //preview related
    bool                            mDisableWaitSensorThread;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:    ////Instantiation.
    virtual                         ~DefaultCameraDevice1();
                                    DefaultCameraDevice1(
                                        ICameraDeviceManager* deviceManager,
                                        std::string instanceName,
                                        int32_t instanceId,
                                        android::String8 const& rDevName
                                    );

public:    ////CameraDevice1Base Interfaces.

    virtual bool                    disableWaitSensorThread(bool disable);
    virtual int32_t                 queryDisplayBufCount() const { return 6; }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  [Template method] Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.

    /**
     * [Template method] Called by open().
     * Initialize the device resources owned by this object and open the camera
     */
    virtual status_t        onOpenLocked();

    /**
     * [Template method] Called by close().
     * Uninitialize and release the device resources owned by this object
     */
    virtual void            onCloseLocked();

    /**
     * [Template method] Called by startPreview().
     */
    virtual bool            onStartPreview();

    /**
     * [Template method] Called by stopPreview().
     */
    virtual void            onStopPreview();

    /**
     * [Template method] Called by startRecording().
     */
    virtual bool            onStartRecording();

    /**
     * [Template method] Power on sensor.
     */
    virtual bool            powerOnSensor();

    /**
     * [Template method] Called in the sensor_power_on thread
     */
    static void*            doThreadSensorOn(void* arg);

    /**
     * [Template method] Wait for sonsor_power_on thread done.
     */
    virtual bool            waitThreadSensorOnDone();

    /**
     * [Template method] transform string awbmode to integer value.
     */
    virtual int awbModeStringToEnum(const char *awbMode);

    /**
     * [Template method] Called by setParameters().
     */
    virtual status_t        onSetParameters(const char* params);
};
};  //namespace NSCAM
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_V1_DEVICE_DEFAULTCAMERADEVICE1_H_
