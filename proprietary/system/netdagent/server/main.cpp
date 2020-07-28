/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LOG_TAG "Netdagent"
#include "log/log.h"

#include "CommandController.h"
#include "CommandListener.h"
#include "CommandService.h"

using android::netdagent::CommandListener;
using android::netdagent::CommandController;
using android::netdagent::CommandService;
using android::netdagent::gCtls;

CommandService* gCs = nullptr;

static void blockSigpipe() {
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGPIPE);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) != 0)
        ALOGW("WARNING: SIGPIPE not blocked\n");
}

int main() {
    const int HWBINDER_MAXTHREAD = 10;
#if 0
    char netdagentPid[PROPERTY_VALUE_MAX] = {0};
    snprintf(netdagentPid, sizeof(netdagentPid), "%d", getpid());
    if (!property_set("vendor.mtk.netdagent.pid", netdagentPid))  //fixme
        ALOGI("Netdagent 1.0 starting as pid %s", netdagentPid);
#endif

    gCtls = new CommandController();
    blockSigpipe();

#if 0
    CommandListener cl("netdagent");
    if (cl.startListener()) {   //fork thread to run socket listener
        ALOGE("Unable to start CommandListener (%s)", strerror(errno));
        exit(1);
    }
#endif


    gCs = new CommandService(HWBINDER_MAXTHREAD);
    ALOGI(" Create CommandService  successfully");
    if (gCs->startService()) {  //fork thread to run hwbinder service
        ALOGE("Unable to start CommandService");
        exit(1);
    }

    while(1)
        sleep(30);   //main thread blocking
    ALOGI("Netdagent exiting");
    exit(0);
}
