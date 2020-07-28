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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MAIN_HAL_ICAMERADEVICEMANAGER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MAIN_HAL_ICAMERADEVICEMANAGER_H_
//
#include <memory>
#include <string>
#include <vector>
//
#include <utils/Errors.h>
#include <utils/Printer.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
//
#include <hidl/HidlSupport.h>
//
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *
 ******************************************************************************/
class IMetadataProvider;


/******************************************************************************
 *
 ******************************************************************************/
class ICameraDeviceManager
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    typedef ::android::hidl::base::V1_0::IBase  IBase_t;

    class IVirtualDevice : virtual public ::android::RefBase
    {
    public:
        /**
         * The device instance names must be of the form
         *      "device@<major>.<minor>/<type>/<id>" where
         * <major>/<minor> is the HIDL version of the interface.
         * <id> is a small incrementing integer for "internal" device types,
         * with 0 being the main back-facing camera and 1 being the main
         * front-facing camera, if they exist.
         */
        struct Info
        {
            std::string mInstanceName;      //  instance device: "device@<major>.<minor>/<type>/<id>"
            int32_t     mInstanceId;        //  instance id
            uint32_t    mMajorVersion;      //  major version
            uint32_t    mMinorVersion;      //  minor version
            bool        mHasFlashUnit;      //  has flash unit
        };

    public:

        virtual auto    getDeviceInterfaceBase(
                            ::android::sp<IBase_t>& rpDevice
                            ) const -> ::android::status_t                  = 0;

        virtual auto    getDeviceInfo() const -> Info const&                = 0;
        virtual auto    getInstanceName() const -> char const*  { return getDeviceInfo().mInstanceName.c_str(); }
        virtual auto    getInstanceId() const -> int32_t        { return getDeviceInfo().mInstanceId; }
        virtual auto    getMajorVersion() const -> uint32_t     { return getDeviceInfo().mMajorVersion; }
        virtual auto    getMinorVersion() const -> uint32_t     { return getDeviceInfo().mMinorVersion; }
        virtual auto    hasFlashUnit() const -> bool            { return getDeviceInfo().mHasFlashUnit; }
    };

    struct Callback : virtual public ::android::RefBase
    {
        virtual auto    onCameraDeviceStatusChange(
                            char const* deviceName, uint32_t new_status
                            ) -> void                                       = 0;

        virtual auto    onTorchModeStatusChange(
                            char const* deviceName, uint32_t new_status
                            ) -> void                                       = 0;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Destructor.

    //  Disallowed to directly delete a raw pointer.
    virtual         ~ICameraDeviceManager() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces - called by Camera Provider
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual auto    debug(
                        std::shared_ptr<android::Printer> printer,
                        const std::vector<std::string>& options
                        ) -> void                                           = 0;

    virtual auto    setCallbacks(
                        const ::android::sp<Callback>& callback
                        ) -> ::android::status_t                            = 0;

    /**
     * The device instance names enumerated must be of the form
     *      "device@<major>.<minor>/<type>/<id>"
     * where
     *   - <major>/<minor> is the version of the interface,
     *   - <type> is the device type, such as "internal", "legacy", "usb", or
     *      "remote". The "legacy" type is only used for passthrough legacy HAL
     *      mode, and must not be used by a standalone binderized HAL.
     *   - <id> is a small incrementing integer for "internal" device types,
     *      with 0 being the main back-facing camera and 1 being the main
     *      front-facing camera, if they exist.
     */
    virtual auto    getDeviceNameList(
                        std::vector<std::string>& rDeviceNames
                        ) -> ::android::status_t                            = 0;

    virtual auto    getDeviceInterface(
                        const std::string& deviceName,
                        ::android::sp<IBase_t>& rpDevice
                        ) -> ::android::status_t                            = 0;

    virtual auto    getType() const -> const std::string&                   = 0;

    virtual auto    isSetTorchModeSupported() const -> bool                 = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces - called by Camera Device
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual auto    setTorchMode(
                        const std::string& deviceName,
                        uint32_t mode
                        ) -> ::android::status_t                            = 0;

    /**
     *  startOpenDevice/finishOpenDevice must be called in pair
     */
    virtual auto    startOpenDevice(
                        const std::string& deviceName
                        ) -> ::android::status_t                            = 0;
    virtual auto    finishOpenDevice(
                        const std::string& deviceName,
                        bool cancel /*cancel open if true*/
                        ) -> ::android::status_t                            = 0;

    /**
     *  startCloseDevice/finishCloseDevice must be called in pair
     */
    virtual auto    startCloseDevice(
                        const std::string& deviceName
                        ) -> ::android::status_t                            = 0;
    virtual auto    finishCloseDevice(
                        const std::string& deviceName
                        ) -> ::android::status_t                            = 0;
    virtual auto    getOpenedCameraNum() -> uint32_t                        = 0;
    virtual auto    updatePowerOnDone() -> void                             = 0;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
{
    struct CreateVirtualCameraDeviceParams
    {
        int32_t                         instanceId;
        const char*                     deviceType;
        NSCam::IMetadataProvider*       pMetadataProvider;
        NSCam::ICameraDeviceManager*    pDeviceManager;
    };

    extern "C"
    NSCam::ICameraDeviceManager::IVirtualDevice*
    createVirtualCameraDevice(CreateVirtualCameraDeviceParams* params);
}


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MAIN_HAL_ICAMERADEVICEMANAGER_H_
