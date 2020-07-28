/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#define LOG_TAG "NetdagentController"
#include "log/log.h"
#include "NetdagentUtils.h"
#include "CommandController.h"
#include "IptablesInterface.h"

namespace android {
namespace netdagent {

CommandController* gCtls = nullptr;

static const char* MANGLE_PREROUTING[] = {
                        NetworkController::LOCAL_MANGLE_PREROUTING,
                        NULL, };

CommandController::CommandController() {
    initControllers();
    initIptables();
}

void CommandController::initControllers() {
#if 0
    Stopwatch s;
    gCtls->perfCtrl.load_PerfService();   /* perfController api initialize */
    if(!gCtls->perfCtrl.is_eng())
        gCtls->throughputMonitor.start();     /* lauch throughput monitor*/
    ALOGI("Initializing controllers: %.1fms", s.getTimeAndReset());
#endif
}

void CommandController::initIptables() {
    Stopwatch s;
    createChildChains(V4V6, "mangle", "-I", "PREROUTING", MANGLE_PREROUTING);
    gCtls->firewallCtrl.setupIptablesHooks();
    ALOGI("Initializing iptables: %.1fms", s.getTimeAndReset());
}

}  // namespace netdagent
}  // namespace android
