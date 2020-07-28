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

#define LOG_TAG "Postalgo-IonDevice"

#include "IIonDevice.h"
#include "MyUtils.h"
#include <utils/std/LogTool.h>
//#include <utils/debug/debug.h>
//
#include <ion/ion.h>
//#include <libion_mtk/include/ion.h>
//
#include <sys/resource.h>
//
#include <atomic>
#include <list>
#include <string>
//
#include <cutils/properties.h>
#include <utils/Mutex.h>
//
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/
static int mt_ion_open(const char *name)
{
    (void *)name;
    int fd;
    fd = ::ion_open();
    if(fd < 0)
    {
        ALOGE("ion_open failed!\n");
        return fd;
    }

/*
    ion_sys_data_t sys_data;
    sys_data.sys_cmd = ION_SYS_SET_CLIENT_NAME;
    strncpy(sys_data.client_name_param.name, name, sizeof(sys_data.client_name_param.name)-1);

    struct ion_custom_data custom_data;
    custom_data.cmd = ION_CMD_SYSTEM;
    custom_data.arg = (unsigned long) sys_data;

    int ret = ioctl(fd, ION_IOC_CUSTOM, &custom_data);
    if (ret < 0) {
        ALOGE("ion_custom_ioctl %x failed with code %d: %s\n", (unsigned int)ION_IOC_CUSTOM,
              ret, strerror(errno));
    }
    */

    return fd;
}


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class IonDeviceProviderImpl : public IIonDeviceProvider
                            /*, public IDebuggee*/
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    struct  IonDevice;
    using   UserListT = std::list<std::weak_ptr<IonDevice>>;

    struct  IonDevice : public IIonDevice
    {
    public:
        std::string const   mUserName;
        struct timespec     mTimestamp;
        int                 mDeviceFd = -1;
        bool                mUseSharedDeviceFd = false;

        using IteratorT = UserListT::iterator;
        IteratorT           mIterator;

    public:
                            IonDevice(char const* userName);
        virtual             ~IonDevice();
        virtual auto        getDeviceFd() const -> int { return mDeviceFd; }
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    static const std::string            mDebuggeeName;
    NSCam::Utils::LogTool*              mLogTool = nullptr;

    struct rlimit           mFdNumLimit{0, 0};
    bool                    mUseSharedIonDevFd = false;

    mutable android::Mutex  mLock;
    int                     mSharedIonDevFd = -1; //  Shared ION Device FD.
    std::atomic_int32_t     mSharedUserCount{0};
    UserListT               mUserList;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////            IDebuggee
    virtual auto            debuggeeName() const -> std::string { return mDebuggeeName; }
    virtual auto            debug(
                                android::Printer& printer,
                                const std::vector<std::string>& options __unused
                            ) -> void
                            {
                                print(printer);
                            }

public:
    static  auto            get() -> IonDeviceProviderImpl*;
                            IonDeviceProviderImpl();
                            ~IonDeviceProviderImpl();
    auto                    detach(IonDevice* pDevice) -> void;
    auto                    attach(std::shared_ptr<IonDevice>& pDevice) -> bool;

public:

    virtual auto            print(::android::Printer& printer) const -> void;

    virtual auto            makeIonDevice(
                                char const* userName,
                                int useSharedDeviceFd
                            ) -> std::shared_ptr<IIonDevice>;

};
};


/******************************************************************************
 *
 ******************************************************************************/
const std::string IonDeviceProviderImpl::mDebuggeeName{"NSCam::IIonDeviceProvider"};

/******************************************************************************
 *
 ******************************************************************************/
auto
IonDeviceProviderImpl::get() -> IonDeviceProviderImpl*
{
    static auto inst = std::make_shared<IonDeviceProviderImpl>();
    MY_LOGF_IF((inst == nullptr), "nullptr instance on IonDeviceProviderImpl::get()");
    return inst.get();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IIonDeviceProvider::get() -> IIonDeviceProvider*
{
    return IonDeviceProviderImpl::get();
}


/******************************************************************************
 *
 ******************************************************************************/
IonDeviceProviderImpl::
IonDeviceProviderImpl()
    : IIonDeviceProvider()
    , mLogTool(NSCam::Utils::LogTool::get())
{
    int err = getrlimit(RLIMIT_NOFILE, &mFdNumLimit);
    if (err != 0) {
        mUseSharedIonDevFd = true;
        mFdNumLimit.rlim_cur = mFdNumLimit.rlim_max = 0;
        MY_LOGI("Use shared ion device fd since we cannot get max fd limit");
    }
    else {
        char const* key = "persist.vendor.camera.sharediondevicefd.threshold";
        auto threshold = ::property_get_int32(key, 0);
        MY_LOGI("%s=%d", key, threshold);
        if ( threshold == 0 ) {
            threshold = 2048;//default
        }

        if ( mFdNumLimit.rlim_cur <= threshold ) {
            mUseSharedIonDevFd = true;
        }
        MY_LOGI("Use shared ion device fd?(%c) threshold:%u max fd limit:(soft/hard)=%" PRIuMAX "/%" PRIuMAX "",
            (mUseSharedIonDevFd ? 'Y' : 'N'), threshold,
            (uintmax_t)mFdNumLimit.rlim_cur, (uintmax_t)mFdNumLimit.rlim_max);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
IonDeviceProviderImpl::
~IonDeviceProviderImpl()
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IonDeviceProviderImpl::
print(::android::Printer& printer) const -> void
{
    UserListT list;
    {
        int     sharedIonDevFd{-1};
        int32_t sharedUserCount{0};

        if (mLock.timedLock(50000000 /* 50ms */) == 0) {
            sharedIonDevFd = mSharedIonDevFd;
            sharedUserCount= mSharedUserCount.load();
            list = mUserList;
            mLock.unlock();
        }
        else {
            printer.printLine("Timed out on lock");
            sharedIonDevFd = mSharedIonDevFd;
            sharedUserCount= mSharedUserCount.load();
        }

        printer.printFormatLine(
            "## Use shared ion device fd by default?(%c)"
            ", max fd limit:(soft/hard)=(%" PRIuMAX "/%" PRIuMAX ")",
            (mUseSharedIonDevFd ? 'Y' : 'N'),
            (uintmax_t)mFdNumLimit.rlim_cur,
            (uintmax_t)mFdNumLimit.rlim_max);
        printer.printFormatLine("## Shared ion device fd=%d (usercount=%d)", sharedIonDevFd, sharedUserCount);
    }

    for (auto const& v : list) {
        auto s = v.lock();
        if ( s == nullptr ) {
            printer.printLine("dead user (outside locking)");
        }
        else {
            printer.printFormatLine("  %s fd=%d(shared?%c) %s",
                mLogTool->convertToFormattedLogTime(&s->mTimestamp).c_str(),
                s->mDeviceFd,
                (s->mUseSharedDeviceFd ? 'Y' : 'N'),
                s->mUserName.c_str()
            );
        }
        s = nullptr;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IonDeviceProviderImpl::
makeIonDevice(
    char const* userName,
    int useSharedDeviceFd
) -> std::shared_ptr<IIonDevice>
{
    std::shared_ptr<IonDevice> pDevice = std::make_shared<IonDevice>(userName);
    if ( pDevice == nullptr ) {
        MY_LOGE("User %s: fail to make a new device", userName);
        return nullptr;
    }

    if ( useSharedDeviceFd == -1 ) {
        pDevice->mUseSharedDeviceFd = mUseSharedIonDevFd;
    }
    else {
        pDevice->mUseSharedDeviceFd = (useSharedDeviceFd > 0);
    }
    if ( ! attach(pDevice) ) {
        MY_LOGE("User %s: fail to attach the device", userName);
        pDevice = nullptr;
        return nullptr;
    }

    return pDevice;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IonDeviceProviderImpl::
attach(std::shared_ptr<IonDevice>& pDevice) -> bool
{
    auto openIonDevice = [](int& deviceFd, char const* ionClientName, char const* userName) {
#if 1//defined(MTK_ION_SUPPORT)
        deviceFd = mt_ion_open(ionClientName);
        if  ( 0 > deviceFd ) {
            MY_LOGE("User %s: mt_ion_open(%s) return %d", userName, ionClientName, deviceFd);
            return false;
        }
        return true;
#else
        MY_LOGE("not defined: MTK_ION_SUPPORT");
        return false;
#endif
    };

    int deviceFd = -1;
    if ( pDevice->mUseSharedDeviceFd ) {

        android::Mutex::Autolock _l(mLock);

        if ( 0 > mSharedIonDevFd ) {
            MY_LOGI("open shared ion device +");
            if ( ! openIonDevice(mSharedIonDevFd, "mtkcam", pDevice->mUserName.c_str()) ) {
                return false;
            }
            MY_LOGI("open shared ion device - fd=%d", mSharedIonDevFd);
        }
        mSharedUserCount++;
        deviceFd = mSharedIonDevFd;
    }
    else {
        if ( ! openIonDevice(deviceFd, pDevice->mUserName.c_str(), pDevice->mUserName.c_str()) ) {
            return false;
        }
    }

    {
        android::Mutex::Autolock _l(mLock);

        auto iter = mUserList.insert(mUserList.end(), pDevice);
        pDevice->mIterator = iter;
        pDevice->mDeviceFd = deviceFd;
        mLogTool->getCurrentLogTime(&pDevice->mTimestamp);
    }
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IonDeviceProviderImpl::
detach(IonDevice* pDevice) -> void
{
    auto closeIonDevice = [](int& deviceFd __unused) {
#if 1//defined(MTK_ION_SUPPORT)
        if ( 0 <= deviceFd ) {
            ::ion_close(deviceFd);
            deviceFd = -1;
        }
#endif
    };

    int deviceFd = -1;
    if ( pDevice->mUseSharedDeviceFd ) {
        {
            android::Mutex::Autolock _l(mLock);
            mUserList.erase(pDevice->mIterator);
            if ( 0 == --mSharedUserCount ) {
                deviceFd = mSharedIonDevFd;
                mSharedIonDevFd = -1;
            }
        }
    }
    else {
        {
            android::Mutex::Autolock _l(mLock);
            mUserList.erase(pDevice->mIterator);
            deviceFd = pDevice->mDeviceFd;
            pDevice->mDeviceFd = -1;
        }
    }
    closeIonDevice(deviceFd);
}


/******************************************************************************
 *
 ******************************************************************************/
IonDeviceProviderImpl::
IonDevice::
IonDevice(char const* userName)
    : IIonDevice()
    , mUserName(userName)
{
    ::memset(&mTimestamp, 0, sizeof(mTimestamp));
}


/******************************************************************************
 *
 ******************************************************************************/
IonDeviceProviderImpl::
IonDevice::
~IonDevice()
{
    if  ( -1 == mDeviceFd ) {
        //Data members (e.g. mIterator) are not initialized if mDeviceFd==-1.
        //It may happen such as: attach() fails -> ~IonDevice() -> detach()
        MY_LOGW("Don't detach %p since mDeviceFd==-1", this);
    }
    else {
        IonDeviceProviderImpl::get()->detach(this);
    }
}

