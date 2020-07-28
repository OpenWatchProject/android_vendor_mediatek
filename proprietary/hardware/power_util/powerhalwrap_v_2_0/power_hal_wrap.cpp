#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define LOG_TAG "PowerWrap"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <log/log.h>
#include <inttypes.h>

#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>
#include <vendor/mediatek/hardware/power/2.1/IPower.h>
#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPerf.h>
#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPower.h>

//#include "mtkpower_hint.h" // cannot include vendor header file

using android::hardware::Return;
using android::hardware::hidl_death_recipient;
using android::hidl::base::V1_0::IBase;
using namespace vendor::mediatek::hardware::power::V2_1;
using namespace vendor::mediatek::hardware::mtkpower::V1_0;

using MtkPowerHint_2_0 = ::vendor::mediatek::hardware::power::V2_0::MtkPowerHint;
using MtkPowerCmd_2_1 = ::vendor::mediatek::hardware::power::V2_1::MtkPowerCmd;

#define BOOT_INFO_FILE "/sys/class/BOOT/BOOT/boot/boot_mode"
static bool gMtkPowerHalExists = true;
static android::sp<IMtkPower> gMtkPowerHal = nullptr;
static android::sp<IPower> gPowerHal = nullptr;
static std::mutex gMtkPowerHalMutex;
static bool getMtkPowerHal();

/* CAUSION: should be sync with mtkpower_hint.h */
enum {
    MTKPOWER_HINT_BASE                  = 30,
    MTKPOWER_HINT_PROCESS_CREATE        = 31,
    MTKPOWER_HINT_PACK_SWITCH           = 32,
    MTKPOWER_HINT_ACT_SWITCH            = 33,
    MTKPOWER_HINT_GAME_LAUNCH           = 34,
    MTKPOWER_HINT_APP_ROTATE            = 35,
    MTKPOWER_HINT_APP_TOUCH             = 36,
    //MTKPOWER_HINT_FRAME_UPDATE          = 37, // no user
    MTKPOWER_HINT_GAMING                = 38,
    MTKPOWER_HINT_GALLERY_BOOST         = 39,
    MTKPOWER_HINT_GALLERY_STEREO_BOOST  = 40,
    MTKPOWER_HINT_SPORTS                = 41,
    MTKPOWER_HINT_TEST_MODE             = 42,
    MTKPOWER_HINT_WFD                   = 43,
    MTKPOWER_HINT_PMS_INSTALL           = 44,
    MTKPOWER_HINT_EXT_LAUNCH            = 45,
    MTKPOWER_HINT_WHITELIST_LAUNCH      = 46,
    MTKPOWER_HINT_WIPHY_SPEED_DL        = 47,
    MTKPOWER_HINT_SDN                   = 48,
    MTKPOWER_HINT_NUM                   = 49,
};


struct PowerDeathRecipient : virtual public hidl_death_recipient
{
   // hidl_death_recipient interface
   virtual void serviceDied(uint64_t cookie, const android::wp<IBase>& /*who */) override {
       ALOGI("Abort due to IPower hidl service failure (power hal) %" PRIuMAX, cookie);
       gMtkPowerHal = nullptr;
       gPowerHal = nullptr;
       gMtkPowerHalMutex.lock();
       getMtkPowerHal();
       gMtkPowerHalMutex.unlock();
       /*LOG_ALWAYS_FATAL("Abort due to IPower hidl service failure,"
             " restarting powerhal");*/
   }
};

static android::sp<PowerDeathRecipient> mtkPowerHalDeathRecipient = nullptr;

static bool getMtkPowerHal() {
    if (gMtkPowerHalExists && (gMtkPowerHal == nullptr && gPowerHal == nullptr)) {

        gMtkPowerHal = IMtkPower::getService();
        gPowerHal = IPower::tryGetService();
        if (gMtkPowerHal != nullptr) {
            ALOGI("Loaded mtkpower HAL service");
            mtkPowerHalDeathRecipient = new PowerDeathRecipient();
            android::hardware::Return<bool> linked = gMtkPowerHal->linkToDeath(
            mtkPowerHalDeathRecipient, /*cookie*/ 0);
            if (!linked.isOk()) {
                ALOGE("Transaction error in linking to PowerHAL death: %s",
                linked.description().c_str());
            } else if (!linked) {
               ALOGI("Unable to link to PowerHAL death notifications");
            } else {
               ALOGI("Link to death notification successful");
            }
        } else if (gPowerHal != nullptr) {
            ALOGI("Loaded power HAL service");
            mtkPowerHalDeathRecipient = new PowerDeathRecipient();
            android::hardware::Return<bool> linked = gPowerHal->linkToDeath(
            mtkPowerHalDeathRecipient, /*cookie*/ 0);
            if (!linked.isOk()) {
                ALOGE("Transaction error in linking to PowerHAL death: %s",
                linked.description().c_str());
            } else if (!linked) {
               ALOGI("Unable to link to PowerHAL death notifications");
            } else {
               ALOGI("Link to death notification successful");
            }

        } else {
            ALOGI("Couldn't load power HAL service");
            gMtkPowerHalExists = false;
        }
    }
    return (gMtkPowerHal != nullptr || gPowerHal != nullptr);
}

static void processReturn(const Return<void> &ret, const char* functionName) {
    if(!ret.isOk()) {
        ALOGE("%s() failed: Power HAL service not available", functionName);
        gMtkPowerHal = nullptr;
        gPowerHal = nullptr;
    }
}

static int processReturnWithInt32(const Return<int32_t> &ret, const char* functionName) {
    if(!ret.isOk()) {
        ALOGE("%s() failed: Power HAL service not available", functionName);
        gMtkPowerHal = nullptr;
        gPowerHal = nullptr;
        return 0;
    } else {
        return 1;
    }
}

extern "C"
int PowerHal_Wrap_mtkPowerHint(uint32_t hint, int32_t data)
{
    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);

    if (getMtkPowerHal() && gMtkPowerHal != nullptr) {
        ALOGD("%s, hint:%d, data:%d",__FUNCTION__, hint, data);
        Return<void> ret = gMtkPowerHal->mtkPowerHint(hint, data);
        processReturn(ret, __FUNCTION__);
    }

    return 0;
}

extern "C"
int PowerHal_Wrap_mtkCusPowerHint(uint32_t hint, int32_t data)
{
    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gMtkPowerHal != nullptr) {
        ALOGD("%s, hint:%d, data:%d",__FUNCTION__, hint, data);
        Return<void> ret = gMtkPowerHal->mtkCusPowerHint(hint, data);
        processReturn(ret, __FUNCTION__);
    }

    return 0;
}

extern "C"
int PowerHal_Wrap_querySysInfo(uint32_t cmd, int32_t param)
{
    int data = 0;

    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gMtkPowerHal != nullptr) {
        ALOGD("%s",__FUNCTION__);
        Return<int32_t> ret = gMtkPowerHal->querySysInfo(cmd, param);
        if (processReturnWithInt32(ret, __FUNCTION__)) {
            data = ret;
            ALOGI("%s, data:%d", __FUNCTION__, data);
        }
    }

    return data;
}

extern "C"
int PowerHal_Wrap_notifyAppState(const char *packname, const char *actname, uint32_t pid, int32_t status, uint32_t uid)
{
    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gMtkPowerHal != nullptr) {
        ALOGD("%s",__FUNCTION__);
        Return<void> ret = gMtkPowerHal->notifyAppState(packname, actname, pid, status, uid);
        processReturn(ret, __FUNCTION__);
    }

    return 0;
}

extern "C"
int PowerHal_Wrap_scnReg(void)
{
    int handle = -1;

    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gPowerHal != nullptr) {
        ALOGI("%s",__FUNCTION__);
        Return<int32_t> ret = gPowerHal->scnReg();
        if (processReturnWithInt32(ret, __FUNCTION__)) {
            handle = ret;
            ALOGI("handle:%d",handle);
        }
    }

    return handle;
}

extern "C"
int PowerHal_Wrap_scnConfig(int32_t hdl, int32_t cmd, int32_t param1, int32_t param2, int32_t param3, int32_t param4)
{

    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gPowerHal != nullptr) {
        ALOGI("%s, hdl:%d, cmd:%d, param1:%d, param2:%d",__FUNCTION__, hdl, cmd, param1, param2);
        Return<void> ret = gPowerHal->scnConfig_2_1(hdl, (enum vendor::mediatek::hardware::power::V2_1::MtkPowerCmd)cmd, param1, param2, param3, param4);
        processReturn(ret, __FUNCTION__);
    }

    return 0;
}

extern "C"
int PowerHal_Wrap_scnUnreg(int32_t hdl)
{
    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gPowerHal != nullptr) {
        ALOGI("%s, hdl:%d",__FUNCTION__, hdl);
        Return<void> ret = gPowerHal->scnUnreg(hdl);
        processReturn(ret, __FUNCTION__);
    }

    return 0;
}

extern "C"
int PowerHal_Wrap_scnEnable(int32_t hdl, int32_t timeout)
{
    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gPowerHal != nullptr) {
        ALOGI("%s, hdl:%d, timeout:%d",__FUNCTION__, hdl, timeout);
        Return<void> ret = gPowerHal->scnEnable(hdl, timeout);
        processReturn(ret, __FUNCTION__);
    }

    return 0;
}

extern "C"
int PowerHal_Wrap_scnDisable(int32_t hdl)
{
    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gPowerHal != nullptr) {
        ALOGI("%s, hdl:%d",__FUNCTION__, hdl);
        Return<void> ret = gPowerHal->scnDisable(hdl);
        processReturn(ret, __FUNCTION__);
    }

    return 0;
}

extern "C"
int PowerHal_Wrap_scnUltraCfg(int32_t hdl, int32_t ultracmd, int32_t param1,
                                                     int32_t param2, int32_t param3, int32_t param4)
{
    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gPowerHal != nullptr) {
        ALOGI("%s",__FUNCTION__);
        Return<void> ret = gPowerHal->scnUltraCfg(hdl, ultracmd, param1, param2, param3, param4);
        processReturn(ret, __FUNCTION__);
    }

    return 0;
}

extern "C"
int PowerHal_TouchBoost(int32_t timeout)
{
    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gMtkPowerHal != nullptr) {
        ALOGD("%s",__FUNCTION__);
        Return<void> ret = gMtkPowerHal->mtkPowerHint(MTKPOWER_HINT_APP_TOUCH, timeout);
        processReturn(ret, __FUNCTION__);
    }

    return 0;
}

extern "C"
int PowerHal_Wrap_setSysInfo(int32_t type, const char *data)
{
    int result = 0;

    std::lock_guard<std::mutex> lock(gMtkPowerHalMutex);
    if (getMtkPowerHal() && gMtkPowerHal != nullptr) {
        ALOGD("%s, type:%d",__FUNCTION__, type);
        Return<int32_t> ret = gMtkPowerHal->setSysInfo(type, data);
        if (processReturnWithInt32(ret, __FUNCTION__)) {
            result = ret;
            ALOGI("%s, result:%d", __FUNCTION__, result);
        }
    }

    return result;
}

//} // namespace

