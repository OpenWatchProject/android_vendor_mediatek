#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define LOG_TAG "mtkperf_client"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>
#include <log/log.h>
#include <inttypes.h>
#include <vector>

#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPerf.h>

using android::hardware::Return;
using android::hardware::hidl_death_recipient;
using android::hidl::base::V1_0::IBase;
using std::vector;
using ::android::hardware::hidl_vec;
using namespace vendor::mediatek::hardware::mtkpower::V1_0;

#define BOOT_INFO_FILE "/sys/class/BOOT/BOOT/boot/boot_mode"

static bool gMtkPerfHalExists = true;
static android::sp<IMtkPerf> gMtkPerfHal = nullptr;
static std::mutex gMtkPerfHalMutex;
static bool getMtkPerfHal();

static int check_meta_mode(void)
{
    char bootMode[4];
    int fd;
    //check if in Meta mode
    fd = open(BOOT_INFO_FILE, O_RDONLY);
    if(fd < 0) {
        return 0; // not meta mode
    }

    if(read(fd, bootMode, 4) < 1) {
        close(fd);
        return 0;
    }

    if (bootMode[0] == 0x31 || bootMode[0] == 0x34) {
        close(fd);
        return 1; // meta mode, factory mode
    }

    close(fd);
    return 0;
}

struct MtkPerfDeathRecipient : virtual public hidl_death_recipient
{
   // hidl_death_recipient interface
   virtual void serviceDied(uint64_t cookie, const android::wp<IBase>& /*who*/) override {
       ALOGI("Abort due to IPower hidl service failure (power hal) %" PRIuMAX, cookie);
       gMtkPerfHal = nullptr;
       gMtkPerfHalMutex.lock();
       getMtkPerfHal();
       gMtkPerfHalMutex.unlock();
       /*LOG_ALWAYS_FATAL("Abort due to IPower hidl service failure,"
             " restarting powerhal");*/
   }
};

static android::sp<MtkPerfDeathRecipient> mtkPerfHalDeathRecipient = nullptr;

static bool getMtkPerfHal() {
    if (check_meta_mode() != 0)
        return false;

    if (gMtkPerfHalExists && gMtkPerfHal == nullptr) {
        gMtkPerfHal = IMtkPerf::getService();
        if (gMtkPerfHal != nullptr) {
            ALOGI("Loaded mtkperf HAL service");
            mtkPerfHalDeathRecipient = new MtkPerfDeathRecipient();
            android::hardware::Return<bool> linked = gMtkPerfHal->linkToDeath(
            mtkPerfHalDeathRecipient, /*cookie*/ 0);
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
            gMtkPerfHalExists = false;
        }
    }
    return gMtkPerfHal != nullptr;
}

static void processReturn(const Return<void> &ret, const char* functionName) {
    if(!ret.isOk()) {
        ALOGE("%s() failed: Power HAL service not available", functionName);
        gMtkPerfHal = nullptr;
    }
}

static int processReturnWithInt32(const Return<int32_t> &ret, const char* functionName) {
    if(!ret.isOk()) {
        ALOGE("%s() failed: Power HAL service not available", functionName);
        gMtkPerfHal = nullptr;
        return 0;
    } else {
        return 1;
    }
}

extern "C"
int perf_lock_acq(int hdl, int duration, int list[], int numArgs)
{
    int my_tid = (int)gettid();
    std::vector<int32_t> rscList;
    int i, ret_hdl = -1;

    ALOGI("[perf_lock_acq] hdl:%d, dur:%d, num:%d", hdl, duration, numArgs);

    /* check input parameter */
    if(numArgs % 2 != 0) {
        ALOGE("perf_lock_acq numArgs is wrong");
        return -1;
    }

    /* log */
    for (i=0; i<numArgs; i+=2) {
        ALOGD("[perf_lock_acq] list:0x%08x, %d", list[i], list[i+1]);
    }

    std::lock_guard<std::mutex> lock(gMtkPerfHalMutex);
    if (getMtkPerfHal()) {
        rscList.assign(list, (list+numArgs));
        ALOGI("%s, hdl:%d, dur:%d, tid:%d",__FUNCTION__, hdl, duration, my_tid);
        Return<int32_t> ret = gMtkPerfHal->perfLockAcquire(hdl, duration, rscList, my_tid);
        if (processReturnWithInt32(ret, __FUNCTION__)) {
            ret_hdl = ret;
            ALOGI("ret_hdl:%d",ret_hdl);
        }
    }
    return ret_hdl;
}

extern "C"
int perf_lock_rel(int hdl)
{
    int my_tid = (int)gettid();

    std::lock_guard<std::mutex> lock(gMtkPerfHalMutex);
    if (getMtkPerfHal()) {
        ALOGI("%s, hdl:%d, tid:%d",__FUNCTION__, hdl, my_tid);
        Return<void> ret = gMtkPerfHal->perfLockRelease(hdl, my_tid);
        processReturn(ret, __FUNCTION__);
    }

    return 0;
}

//} // namespace

