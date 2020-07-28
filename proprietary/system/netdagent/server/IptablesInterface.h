/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef _IPTABLESINTERFACE_H
#define _IPTABLESINTERFACE_H

#include <string>
#include <list>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdarg.h>
#include "NetdagentUtils.h"

namespace android {
namespace netdagent {

enum IptablesTarget { V4, V6, V4V6 };

int execIptables(IptablesTarget target, ...);
int execIptablesSilently(IptablesTarget target, ...);
int system_nosh(const char *command);
int execTcCmd(const char *cmd);
int execNdcCmd(const char *command, ...);
int execIpCmd(int family, ...);
void createChildChains(IptablesTarget target, const char* table,const char* op,
                       const char* parentChain, const char** childChains);

}  // namespace netdagent
}  // namespace android

#endif  // _IPTABLESINTERFACE_H
