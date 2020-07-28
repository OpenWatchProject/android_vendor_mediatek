/**
 * Copyright (c) 2016, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "NetdagentService"
#include "log/log.h"

#include "CommandService.h"


using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::registerPassthroughServiceImplementation;
using android::netdagent::CommandService;

CommandService::CommandService(int maxThreads, std::string serviceName) {
    mMaxThreads = maxThreads;
    mName = serviceName;
    mThread = 0;
    mCommandDispatchList = new tCommandDispatchList();
    registerCmd(new FirewallCmd());
    registerCmd(new ThrottleCmd());
    registerCmd(new NetworkCmd());
}

CommandService::~CommandService() {

    for (auto it = mCommandDispatchList->begin(); it != mCommandDispatchList->end();) {
        delete (*it);
        it = mCommandDispatchList->erase(it);
    }
    delete mCommandDispatchList;
}

int CommandService::startService() {

    if (pthread_create(&mThread, NULL, CommandService::threadStart, this)) {
        ALOGE("pthread_create (%s)", strerror(errno));
        return -1;
    }
    return 0;
}

void *CommandService::threadStart(void *obj) {
    CommandService *me = reinterpret_cast<CommandService *>(obj);
    me->runListener();
    pthread_exit(NULL);
    return NULL;
}

void CommandService::runListener() {
    configureRpcThreadpool(mMaxThreads, true);
    if (this->registerAsService()) {
        ALOGE("register %s/%s failed\n", INetdagent::descriptor, mName.c_str());
        return;
    }
    joinRpcThreadpool();
}

Return<bool> CommandService::dispatchNetdagentCmd(const hidl_string& cmd) {
    /* store command */
    char *cmdBuffer = strdup(cmd.c_str());
    /* parse command */
    const char *delim = " ";
    char *token = nullptr;
    char *argv[CMD_ARG_COUNT];
    int argc = 0;
    int res = 0;

    //parse command
    token = strtok(cmdBuffer, delim);
    while (token != NULL) {
        if (argc >= CMD_ARG_COUNT) {
            ALOGE("Command contains too many parameters\n");
            res = -1;
            goto _release;
        }
        argv[argc++] = strdup(token);
        token = strtok(NULL, delim);
    }

    if (strcmp(argv[0], "netdagent")) {
        ALOGE("unknown command %s\n", cmdBuffer);
        res = -1;
        goto _release;
    }
    if (argc < 2) {
        ALOGE("Missing command %s\n", cmdBuffer);
        res = -1;
        goto _release;
    }
#ifdef MTK_DEBUG
    ALOGI("Netdagent command %s\n", cmd.c_str());
#endif
   {//enter critical zone
    android::netdagent::AutoMutexLock serviceLock(mServiceLock);
    //dispatch command
    for (auto it = mCommandDispatchList->begin(); it != mCommandDispatchList->end(); ++it) {
        if (!strcmp(argv[1], (*it)->getCommand())) {
            if ((*it)->runCommand(argc-1, &argv[1])) {
                ALOGE("run command %s failed\n", (*it)->getCommand());
                res = -1;
            } else
                res = 1;
        }
    }
    }//exit critical zone

    if (!res) {
        ALOGE("unknown command %s\n", cmdBuffer);
        res = -1;
    }

_release:
    int i;
    for (i = 0; i < argc; i++)
        free(argv[i]);
    free(cmdBuffer);
    return (res==1) ? true : false;
}
