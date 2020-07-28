/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

// System headers required for setgroups, etc.
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <sys/prctl.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>

#include <vendor/mediatek/hardware/videotelephony/1.0/IVideoTelephony.h>
#include <hidl/LegacySupport.h>

#include "main_vtservice_hidl.h"

using namespace android;
using android::hardware::defaultPassthroughServiceImplementation;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;

using ::vendor::mediatek::hardware::videotelephony::V1_0::IVideoTelephony;


int main(int argc, char** argv) {

    configureRpcThreadpool(1, true /* callerWillJoin */);

    ALOGI("[VT][SRV]before VTService_HiDL_instantiate");

    int ret = defaultPassthroughServiceImplementation<IVideoTelephony>();
    ALOGI("[VT][SRV] defaultPassthroughServiceImplementation : ret = %d", ret);

    // use Pass through mode instead of New instance directly
    //
    //sp<IVideoTelephony> vt = VideoTelephony::getInstance();
    //status_t err = vt->registerAsService();
    //ALOGE_IF(err != OK, "[VT][SRV] VTService_HiDL Cannot register %s: %d", IVideoTelephony::descriptor, err);

    ALOGI("[VT][SRV]after VTService_HiDL_instantiate");

    joinRpcThreadpool();

    return 0;
}
