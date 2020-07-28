/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_V1_DEVICE_DUALCAMERADEVICE1_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_V1_DEVICE_DUALCAMERADEVICE1_H_
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

#include <mtkcam/feature/DualCam/ISyncManager.h>
#include <mtkcam/utils/hw/IScenarioControl.h>

#include <device/CameraDevice1Base.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam
{

/******************************************************************************
 *
 ******************************************************************************/
class ImgBufProviderClientBridge : public IImgBufProviderClient
{
    public:
        ImgBufProviderClientBridge() {}
        virtual                     ~ImgBufProviderClientBridge() {}
        //
        /**
         * Notify when IImgBufProvider is created.
         */
        virtual bool                onImgBufProviderCreated(sp<IImgBufProvider>const& rpProvider);
        /**
         * Notify when IImgBufProvider is destroyed.
         */
        virtual void                onImgBufProviderDestroyed(int32_t const i4ProviderId);

        virtual void                addImgBufProviderClient(sp<IImgBufProviderClient> rpClient);
        virtual void                clearImgBufProviderClient();
    private:
        mutable Mutex                        mLock;
        Vector<sp<IImgBufProviderClient>>    mvClient;

};

/******************************************************************************
 *
 ******************************************************************************/
class DualCameraDevice1 : public CameraDevice1Base,
                       public NSCam::ISyncManagerCallback
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Data Members.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    //
#define USER_NAME               "DualCameraDevice1"
    //
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    NSCam::IHalSensor*              mpHalSensor = nullptr;
    NSCam::IHalSensor*              mpHalSensor_2 = nullptr;
#endif
    //
#if '1'==MTKCAM_HAVE_3A_HAL
    NS3Av3::IHal3A*                 mpHal3a = nullptr;
    NS3Av3::IHal3A*                 mpHal3a_2 = nullptr;
#endif
    bool                            mbThreadRunning = false;
    bool                            mRet = false;
    pthread_t                       mThreadHandle;
    bool                            mDisableWaitSensorThread = false;

    // + for dual cam
    sp<ICamAdapter>                 mpCamAdapter_W = nullptr;
    sp<ICamAdapter>                 mpCamAdapter_T = nullptr;
    sp<NSCam::IScenarioControl>     mpScenarioCtrl_main1;
    sp<NSCam::IScenarioControl>     mpScenarioCtrl_main2;
    sp<IParamsManagerV3>            mpParamsMgrV3_Sup = nullptr;
    Mutex                           mSyncLock;

    int32_t                         mSensorId_Main2 = -1;
    bool                            mb4K2KVideoRecord = false;
    sp<ImgBufProviderClientBridge>  mpBridge = nullptr;
    sp<NSCam::ISyncManager>         mSyncMgr = nullptr;
    bool                            mbIsUninit = false;
    MBOOL                           isStoppreview = MFALSE;
    mutable Mutex                   mPostProcCheckLock;
    mutable Condition               mPostProcCheckCond;
    //
    std::future<status_t>           mBgStartTelePreviewJob;
    mutable Mutex                   mPreviewMetaLock;
    mutable Condition               mPreviewMetaCond;
    // - for dual cam
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Operations.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    virtual                         ~DualCameraDevice1();
    DualCameraDevice1(
                                    ICameraDeviceManager* deviceManager,
                                    std::string instanceName,
                                    int32_t instanceId,
                                    android::String8 const& rDevName
    );

public:    ////CameraDevice1Base Interfaces.

    bool                    disableWaitSensorThread(bool disable) override final;
    int32_t                 queryDisplayBufCount() const override final { return 6; }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  [Template method] Operations.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.

    /**
     * [Template method] Called by open().
     * Initialize the device resources owned by this object and open the camera
     */
    status_t        onOpenLocked() override final;

    /**
     * [Template method] Called by close().
     * Uninitialize and release the device resources owned by this object
     */
    void            onCloseLocked() override final;

    /**
     * [Template method] Called by startPreview().
     */
    bool            onStartPreview() override final;

    /**
     * [Template method] Called by stopPreview().
     */
    void            onStopPreview() override final;

    /**
     * [Template method] Called by startRecording().
     */
    bool            onStartRecording() override final;

    /**
     * [Template method] Called by setParameters().
     */
    status_t        onSetParameters(const char* params) override final;

    /**
     * Power on sensor.
     */
    bool            powerOnSensor();

    /**
     * [Template method] Called in the sensor_power_on thread
     */
    static void*    doThreadSensorOn(void* arg);

    /**
     * [Template method] Wait for sonsor_power_on thread done.
     */
    bool            waitThreadSensorOnDone() override final;

    /**
     * overwrite CameraDevice1Base API
     */
    // -------------------------------------------------------------------------
    Return<Status>     startPreview() override final;
    Return<void>       stopPreview() override final;
    Return<Status>     setParameters(const hidl_string& params) override final;
    Return<Status>     startRecording() override final;
    Return<void>       stopRecording() override final;
    Return<Status>     takePicture() override final;
    //
    Return<Status>     autoFocus() override final;
    Return<Status>     cancelAutoFocus() override final;
    // -------------------------------------------------------------------------
    //
    status_t           initDisplayClient(preview_stream_ops* window) override final;
    status_t           enableDisplayClient() override final;
    void               startPreviewImp() override final;
    // -------------------------------------------------------------------------

    /**
     * dual camera device specific adapter init flow
     */
    virtual bool       initDualCameraAdapter();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  SyncMgr callback Interfaces.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MINT32 onEvent(
        MINT32 const i4OpenId,
        MINT32 arg1, MINT32 arg2, void* arg3) override final;

private:
        int32_t     getOpenId_Main2() const {return mSensorId_Main2;}
        status_t    updateSensorId();
        status_t    setFeatureMode();
        status_t    setSensorModuleType();
        MBOOL       needReconstructRecordingPipe(
                            sp<IParamsManagerV3> param,
                            MBOOL isRecordMode);
        void        updateDeviceState(
                            MINT32 state);
        status_t    powerOffSensor(
                            int32_t openId,
                            NSCam::IHalSensor*& halSensor);
        status_t    uninit3A(
                            int32_t openId,
                            NS3Av3::IHal3A*& hal3A);
        status_t    dualAdapterStartPreview();

        bool        getPolicyDecision(
                            String8& rShotModeStr,
                            uint32_t& rShotMode) const;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace android
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_V1_DEVICE_DUALCAMERADEVICE1_H_

