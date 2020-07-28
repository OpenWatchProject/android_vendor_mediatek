#include "AudioParamParserPriv.h"

#ifdef WIN32
#include <windows.h>
#include <io.h>
/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

/*
 * Description:
 *   Implement C++ & windows related APIs
 */

#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <stdio.h>

#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif
#else /* !WIN32 */
#include <binder/ProcessState.h>
#include <inttypes.h>

#if !defined(SYS_IMPL)
#include "AudioCustParam.h"
#else
#include <media/AudioSystem.h>
#endif

using namespace android;
#endif


#ifndef WIN32
#if defined(SYS_IMPL)
#include <android/hardware/audio/5.0/IDevicesFactory.h>
#include <android/hardware/audio/5.0/IDevice.h>
#include <vendor/mediatek/hardware/audio/5.1/IMTKPrimaryDevice.h>
#include <vendor/mediatek/hardware/audio/5.1/IAudioParameterChangedCallback.h>


using ::android::hardware::audio::V5_0::IDevicesFactory;
using ::android::hardware::audio::V5_0::IDevice;
using ::android::hardware::audio::V5_0::Result;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_string;
using ::android::hardware::Return;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::audio::V5_1::IAudioParameterChangedCallback;
using ::vendor::mediatek::hardware::audio::V5_1::IMTKPrimaryDevice;


/*
 * Class declaration
 */
class AudioHalDeathRecipient : public hidl_death_recipient {
public:
    AudioHalDeathRecipient(void) {}
    void serviceDied(uint64_t cookie, const wp<::android::hidl::base::V1_0::IBase> & /*who*/) override {
#if 1
        ERR_LOG("%s() AudioServer die... exit!(cookie = %" PRIu64 ")", __FUNCTION__, cookie);
        exit(1);
#else
        /* Re-connect to server */
        registerAudioParameterChangedCallback(appOpsGetInstance()->appHandleGetInstance());
#endif
    }
};

class AudioParameterChangedCallback : public IAudioParameterChangedCallback {
    Return<void> audioParameterChangedCallback(const hidl_string &audioTypeName) override {
        INFO_LOG("%s() got callback! (audioType: %s)", __FUNCTION__, audioTypeName.c_str());
        AudioType* audioType = appHandleGetAudioTypeByName(appHandleGetInstance(), audioTypeName.c_str());
        if (audioType) {
            audioType->allowReload = 1;
        }

        /* Notify all callback function */
        appHandleNotifyAllCallbacks(appHandleGetInstance(), audioTypeName.c_str());

        return ::android::hardware::Void();
    }
};


/*
 * Global variable
 */
const sp<IDevice> *gDevice;
sp<AudioHalDeathRecipient> gDeathRecipient;
Result gRetval = Result::NOT_INITIALIZED;


EXPORT APP_STATUS registerAudioParameterChangedCallback(AppHandle *appHandle) {
    /* Get IDevicesFactory */
    sp<IDevicesFactory> devicesFactory = IDevicesFactory::getService();
    if (devicesFactory == 0) {
        ALOGE("Failed to obtain IDevicesFactory service, terminating process.");
        return APP_ERROR;
    }

    /* Open Device */
    Return<void> ret = devicesFactory->openDevice(
                           "primary",
    [&](Result r, const sp<IDevice> &result) {
        gRetval = r;
        if (gRetval == Result::OK) {
            gDevice = new sp<IDevice>(result);
        }
    });

    if (!ret.isOk() || gRetval != Result::OK) {
        ERR_LOG("%s(), Load audio interface fail, (ret: %d, Result: %d)", __FUNCTION__, ret.isOk(), gRetval == Result::OK);
        return APP_ERROR;
    }
    INFO_LOG("%s() audio interface loaded, dev %p", __FUNCTION__, gDevice->get());

    /* Register AudioParameterChangedCallback */
    sp<IAudioParameterChangedCallback> callback = new AudioParameterChangedCallback();
    sp<IMTKPrimaryDevice> mtkPrimaryDev = IMTKPrimaryDevice::castFrom(*gDevice);
    Return<Result> result = mtkPrimaryDev->setAudioParameterChangedCallback(callback);

    if (!result.isOk()) {
        ERR_LOG("setAudioParameterChangedCallback return fail!");
        return APP_ERROR;
    }

    /* Link to death */
    gDeathRecipient = new AudioHalDeathRecipient();
    devicesFactory->linkToDeath(gDeathRecipient, 123456);
    INFO_LOG("%s() linkToDeath success", __FUNCTION__);

    return APP_NO_ERROR;
}

EXPORT APP_STATUS unregisterAudioParameterChangedCallback(AppHandle *appHandle) {
    INFO_LOG("%s()", __FUNCTION__);

    /* Clear callback first */
    sp<IMTKPrimaryDevice> mtkPrimaryDev = IMTKPrimaryDevice::castFrom(*gDevice);
    mtkPrimaryDev->clearAudioParameterChangedCallback();

    /* unlinkToDeath */
    sp<IDevicesFactory> devicesFactory = IDevicesFactory::getService();
    devicesFactory->unlinkToDeath(gDeathRecipient);

    gDevice = NULL;

    return APP_NO_ERROR;
}

#endif

void initProcessState() {
    static int processStateInited = 0;

    /* Init ProcessState, */
    if (!processStateInited) {
        ProcessState::self()->startThreadPool();
        sp<ProcessState> proc(ProcessState::self());
        processStateInited = 1;
    }
}

EXPORT int isCustXmlEnable(void) {
#if !defined(APP_FORCE_ENABLE_CUS_XML) && !defined(CONFIG_MT_ENG_BUILD)
    int res = 0;
#if !defined(SYS_IMPL)
    /* Only vnd AudioParamParser can query NVRam */
    AUDIO_CUSTOM_AUDIO_FUNC_SWITCH_PARAM_STRUCT eParaAudioFuncSwitch;
    res = GetAudioFuncSwitchParamFromNV(&eParaAudioFuncSwitch);
    if (res) {
        INFO_LOG("%s(), Not eng load, Get cust xml enabled from NVRam: GET_CUST_XML_ENABLE=%d\n", __FUNCTION__, (eParaAudioFuncSwitch.cust_xml_enable == 1));
        return (eParaAudioFuncSwitch.cust_xml_enable == 1);
    } else
#endif
    {
        /* If process cannot get parameter due to permission issue, using AudioSystem API instead */
        ALOGW("%s(), Query nvram fail! don't enable cust XML", __FUNCTION__);
        return 0;
    }
#else
    INFO_LOG("%s(), always return 1\n", __FUNCTION__);
    return 1;
#endif
}

EXPORT char *audioSystemGetParameters(const char *str) {
#if defined(SYS_IMPL)
    String8 res;
    const char* ret = NULL;
    initProcessState();
    res = AudioSystem::getParameters(0, String8(str));
    if (res) {
        ret = res.string() + strlen(str) + 1;
        return strdup(ret);
    } else {
        ERR_LOG("AudioSystem::getParameters return NULL!\n");
        return strdup("");
    }
#else
    return strdup("");
#endif
}

EXPORT void audioSystemSetParameters(const char *str) {
#if defined(SYS_IMPL)
    initProcessState();
    AudioSystem::setParameters(0, String8(str));
#endif
}
#else   /* WIN32 */

/* For Tuning Tool show the debug message */
EXPORT void redirectIOToConsole() {
    CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferHandle;
    AllocConsole();
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleScreenBufferHandle);
    consoleScreenBufferHandle.dwSize.Y = 600;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), consoleScreenBufferHandle.dwSize);

    long stdHandle = (long) GetStdHandle(STD_INPUT_HANDLE);
    int osfHandle = _open_osfhandle(stdHandle, _O_TEXT);
    FILE *fp = _fdopen(osfHandle, "r");
    *stdin = *fp;
    setvbuf(stdin, NULL, _IONBF, 0);

    stdHandle = (long) GetStdHandle(STD_OUTPUT_HANDLE);
    osfHandle = _open_osfhandle(stdHandle, _O_TEXT);
    fp = _fdopen(osfHandle, "w");
    *stdout = *fp;
    setvbuf(stdout, NULL, _IONBF, 0);

    stdHandle = (long) GetStdHandle(STD_ERROR_HANDLE);
    osfHandle = _open_osfhandle(stdHandle, _O_TEXT);
    fp = _fdopen(osfHandle, "w");
    *stderr = *fp;
    setvbuf(stderr, NULL, _IONBF, 0);

    ios::sync_with_stdio();

}
#endif
