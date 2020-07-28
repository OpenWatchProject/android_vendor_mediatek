#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define LOG_TAG_NWK_OPT_WRAP "nwk_opt_wrap_s"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include <vendor/mediatek/hardware/nwk_opt/1.0/INwkOpt.h>

#define DEBUG_ON 0
#if DEBUG_ON
#define nwk_opt_wrap_debug(fmt, arg...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG_NWK_OPT_WRAP, fmt"", ##arg)
#else
#define nwk_opt_wrap_debug(fmt, arg...) {}
#endif

using android::hardware::Return;
using android::hardware::hidl_death_recipient;
using android::hidl::base::V1_0::IBase;
using namespace vendor::mediatek::hardware::nwk_opt::V1_0;

static android::sp<INwkOpt> gMtkNwkOptHal;
static std::mutex gMtkNwkOptHalMutex;
static bool getMtkNwkOptHal();
struct NwkOptDeathRecipient : virtual public hidl_death_recipient
{
   // hidl_death_recipient interface
   virtual void serviceDied(uint64_t cookie, const android::wp<IBase>& /*who */) override {
       ALOGE("Abort due to INwkOpt hidl service failure %llu", cookie);
       gMtkNwkOptHal = nullptr;
       gMtkNwkOptHalMutex.lock();
       getMtkNwkOptHal();
       gMtkNwkOptHalMutex.unlock();
   }
};
static android::sp<NwkOptDeathRecipient> mtkNwkOptHalDeathRecipient = nullptr;
static bool getMtkNwkOptHal() {
    if (gMtkNwkOptHal == nullptr) {
        gMtkNwkOptHal = INwkOpt::getService();
        if (gMtkNwkOptHal != nullptr) {
            nwk_opt_wrap_debug("Loaded nwk opt HAL service");
            mtkNwkOptHalDeathRecipient = new NwkOptDeathRecipient();
            android::hardware::Return<bool> linked = gMtkNwkOptHal->linkToDeath(
            mtkNwkOptHalDeathRecipient, /*cookie*/ 0);
            if (!linked.isOk()) {
                ALOGE("Transaction error in linking to NwkOptHAL death: %s",
                linked.description().c_str());
                gMtkNwkOptHal = nullptr;
            } else if (!linked) {
               ALOGI("Unable to link to NwkOptHAL death notifications");
               gMtkNwkOptHal = nullptr;
            } else {
               ALOGI("Link to death notification successful");
            }
        } else {
            ALOGI("Couldn't load Nwk Opt HAL service");
        }
    }
    return gMtkNwkOptHal != nullptr;
}


extern "C"
int NwkOptHal_Wrap_notifyAppState_S(const char *packname, const char *actname, uint32_t pid)
{
    int data = 0;
    std::lock_guard<std::mutex> lock(gMtkNwkOptHalMutex);
    if (getMtkNwkOptHal()) {
        nwk_opt_wrap_debug("%s",__FUNCTION__);
        Return<void> ret = gMtkNwkOptHal->notifyAppState(packname, actname, pid);
        if (!ret.isOk()) {
            data = -1;
            ALOGE("(%s) failed: INwkOpt is not available", __FUNCTION__);
        }
    } else
        data = -1;


    return data;
}
extern "C"
int NwkOptHal_wrap_setDisplayViewport_S(int32_t orientation, int32_t width, int32_t height) {
    int data = 0;
    std::lock_guard<std::mutex> lock(gMtkNwkOptHalMutex);
    if (getMtkNwkOptHal()) {
        nwk_opt_wrap_debug("%s",__FUNCTION__);
        Return<void> ret = gMtkNwkOptHal->setDisplayData(orientation, width, height);
        if (!ret.isOk()) {
            data = -1;
            ALOGE("(%s) failed: INwkOpt is not available", __FUNCTION__);
        }
    } else
        data = -1;
    return data;
}
extern "C"
int NwkOptHal_wrap_switch_game_mode_S(uint32_t game_mode, int32_t uid) {
    int data = 0;
    std::lock_guard<std::mutex> lock(gMtkNwkOptHalMutex);
    if (getMtkNwkOptHal()) {
        nwk_opt_wrap_debug("%s:%d",__FUNCTION__, game_mode);
        Return<void> ret = gMtkNwkOptHal->switch_game_mode(game_mode, uid);
        if (!ret.isOk()) {
            data = -1;
            ALOGE("(%s) failed: INwkOpt is not available", __FUNCTION__);
        }
    } else
        data = -1;
    return data;

}
//} // namespace

