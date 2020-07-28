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

#ifndef _COMMANDCONTROLLER_H__
#define _COMMANDCONTROLLER_H__

#include "FirewallController.h"
#include "ThrottleController.h"
#include "NetworkController.h"

namespace android {
namespace netdagent {

class CommandController {
public:
    CommandController();

    FirewallController firewallCtrl;
    ThrottleController throttleCtrl;
    NetworkController netCtrl;
#if 0
    ThroughputMonitor throughputMonitor;
    PerfController perfCtrl;
#endif
private:
    void initControllers();
    void initIptables();

};

extern CommandController* gCtls;

}  // namespace netdagent
}  // namespace android

#endif  // _COMMANDCONTROLLER_H__
