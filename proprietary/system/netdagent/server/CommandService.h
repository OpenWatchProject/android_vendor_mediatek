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

#ifndef _COMMANDSERVICE_H__
#define _COMMANDSERVICE_H__


#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>
#include <vendor/mediatek/hardware/netdagent/1.0/INetdagent.h>
#include <NetdagentUtils.h>
#include <CommandDispatch.h>
#include <list>
#include <string>

using android::hardware::hidl_string;
using android::hardware::Return;
using vendor::mediatek::hardware::netdagent::V1_0::INetdagent;

typedef std::list<android::netdagent::CommandDispatch *> tCommandDispatchList;

namespace android {
namespace netdagent {

class CommandService : public INetdagent {

public:
    CommandService(int maxThreads, std::string serviceName =  "default");
    virtual ~CommandService();

    int startService();
    virtual Return<bool> dispatchNetdagentCmd(const hidl_string& cmd);

private:
    static void* threadStart(void *obj);
    void runListener();
    void registerCmd(CommandDispatch *cmd) {
        mCommandDispatchList->push_back(cmd);
    }

    int mMaxThreads;
    std::string mName;
    pthread_t mThread;
    mutable android::netdagent::MutexLock mServiceLock;
    tCommandDispatchList *mCommandDispatchList;
};

}  //netdagent
}  //android

#endif
