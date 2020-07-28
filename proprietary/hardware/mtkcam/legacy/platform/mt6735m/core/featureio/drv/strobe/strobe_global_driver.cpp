/* TODO: remove useless header */

#ifdef WIN32

#include "stdio.h"
#include "time.h"
#include "strobe_drv_2led.h"
#include "flashlight_drv_2led.h"
#include "flashlight.h"
#include "windows.h"
#include "win_test_cpp.h"
#include "strobe_global_driver.h"

#else
#define MTK_LOG_ENABLE 1
#define LOG_TAG "strobe_global_driver.cpp"
#include <utils/Errors.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <fcntl.h>

#include "ae_feature.h"
#include <strobe_drv.h>
#include "flashlight_drv.h"
#include "flashlight.h"

#include <mtkcam/Log.h>

#include "camera_custom_nvram.h"
#include "camera_custom_types.h"

#include "flash_feature.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <kd_camera_feature.h>
#include "strobe_global_driver.h"
#endif

/* TODO: refactor log macro */
#ifdef WIN32
    #define logI(fmt, ...)    {printf(fmt, __VA_ARGS__); printf("\n");}
    #define logW(fmt, ...)    {printf(fmt, __VA_ARGS__); printf("\n");}
    #define logE(fmt, ...)    {printf("merror: line=%d, ", __LINE__); printf(fmt, __VA_ARGS__); printf("\n");}
#else
    #define logI(fmt, arg...)  CAM_LOGD( fmt, ##arg)
    #define logW(fmt, arg...)  CAM_LOGD("warning: line=%d, " fmt, __LINE__, ##arg)
    #define logE(fmt, arg...)  CAM_LOGD("error: line=%d, " fmt, __LINE__, ##arg)
#endif


/* TODO:
 * - use kernel header file
 * - move to header file
 */
#define STROBE_DEV_NAME "/dev/flashlight"

StrobeGlobalDriver* StrobeGlobalDriver::getInstance()
{
    static StrobeGlobalDriver globalDriver;
    return &globalDriver;
}

StrobeGlobalDriver::StrobeGlobalDriver() : mStrobeHandle(-1), mUsers(0)
{
}

int StrobeGlobalDriver::sendCommandRet(int cmd, int typeId, int ctId, int *arg)
{
    Mutex::Autolock lock(mLock);
    return sendCommandRet_nolock(cmd, typeId, ctId, arg);
}

int StrobeGlobalDriver::sendCommandRet_nolock(int cmd, int typeId, int ctId, int *arg)
    {

    if (mStrobeHandle < 0) {
        logE("sendCommand() mStrobeHandle <= 0 ~");
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }

    /* setup arguments */
    /* TODO: redefine ioctl argument */
    struct flashlight_user_arg stbArg;
    stbArg.type_id = typeId;
    stbArg.ct_id = ctId;
    stbArg.arg=0;

    /* send ioctl */
    int ret;
    ret = ioctl(mStrobeHandle, cmd, &stbArg);
    *arg = stbArg.arg;

    return ret;
}

int StrobeGlobalDriver::sendCommand(int cmd, int typeId, int ctId, int arg)
{
    Mutex::Autolock lock(mLock);
    return sendCommand_nolock(cmd, typeId, ctId, arg);
}

int StrobeGlobalDriver::sendCommand_nolock(int cmd, int typeId, int ctId, int arg)
    {

    if (mStrobeHandle < 0) {
        logE("sendCommand() mStrobeHandle <= 0 ~");
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }

    /* setup arguments */
    struct flashlight_user_arg stbArg;
    stbArg.type_id = typeId;
    stbArg.ct_id = ctId;
    stbArg.arg=arg;

    /* send ioctl */
    return ioctl(mStrobeHandle, cmd, &stbArg);
}

int StrobeGlobalDriver::openkd_nolock()
{
    /* Note, there's no lock to protect usage count here */
    if (!mUsers) {
        mStrobeHandle = open(STROBE_DEV_NAME, O_RDWR);

        if (mStrobeHandle < 0) {
            logE("Failed to open device node %s: %s", STROBE_DEV_NAME, strerror(errno));
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }
    }

    android_atomic_inc(&mUsers);

    logI("open flash driver kd=%d, usage count=%d", mStrobeHandle, mUsers);

    return 0;
}

int StrobeGlobalDriver::openkd()
{
    Mutex::Autolock lock(mLock);
    return openkd_nolock();
}

int StrobeGlobalDriver::closekd_nolock()
{
    if (mUsers <= 0) {
        logW("closekd_nolock user<=0");
        return -1;
    }

    if (mUsers == 1) {
        if (mStrobeHandle > 0) {
            close(mStrobeHandle);
            logI("close flash driver kd=%d", mStrobeHandle);
        }
        mStrobeHandle = -1;
    }
    android_atomic_dec(&mUsers);

    return 0;
}

int StrobeGlobalDriver::closekd()
{
    Mutex::Autolock lock(mLock);
    return closekd_nolock();
}

int StrobeGlobalDriver::init(int typeId, int ctId)
{
    logI("init dev=%d id=%d", typeId, ctId);

    Mutex::Autolock lock(mLock);

    int err;

    err = openkd_nolock();
    if (err) {
        logE("Failed to open device node %s", STROBE_DEV_NAME);
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }

    err = sendCommand_nolock(FLASHLIGHTIOC_X_SET_DRIVER, typeId, ctId, 1);
    if (err) {
        /* TODO: should close fd or not */
        logE("FLASHLIGHTIOC_X_SET_DRIVER kd_err=%d", err);
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }
    /* TODO: should close fd or not */

    return 0;
}

int StrobeGlobalDriver::uninit(int typeId, int ctId)
{
    logI("uninit dev=%d id=%d", typeId, ctId);

    Mutex::Autolock lock(mLock);
    sendCommand_nolock(FLASHLIGHTIOC_X_SET_DRIVER, typeId, ctId, 0);
    closekd_nolock();

    return 0;
}

