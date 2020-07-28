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

#ifndef _FIREWALL_CONTROLLER_H
#define _FIREWALL_CONTROLLER_H

#include "IptablesInterface.h"

namespace android {
namespace netdagent {

enum FirewallRule { DENY, ALLOW };

// WHITELIST means the firewall denies all by default, uids must be explicitly ALLOWed
// BLACKLIST means the firewall allows all by default, uids must be explicitly DENYed

enum FirewallType { WHITELIST, BLACKLIST };

enum ChildChain { NONE, DOZABLE, STANDBY, POWERSAVE, INVALID_CHAIN };

enum FirewallChinaRule { MOBILE, WIFI };

/*
 * Simple firewall that drops all packets except those matching explicitly
 * defined ALLOW rules.
 *
 * Methods in this class must be called when holding a write lock on |lock|, and may not call
 * any other controller without explicitly managing that controller's lock. There are currently
 * no such methods.
 */
class FirewallController {
public:

    static const char* LOCAL_FILTER_INPUT;
    static const char* LOCAL_FILTER_OUTPUT;
    static const char* LOCAL_FILTER_OEMOUTPUT;
    static const char* LOCAL_FILTER_FORWARD;
    static const char* LOCAL_MANGLE_POSTROUTING;
    static const char* LOCAL_NAT_PREROUTING;
    static const char* FIREWALL_CTA_MOBILE;
    static const char* FIREWALL_CTA_ALL;
    static const char* FIREWALL_BGDATA;
    static const char* FIREWALL_PRIOU;
    static const char* FIREWALL_PRIOT;

    int setupIptablesHooks(void);
    int setUdpForwarding(const char* inInterface, const char* extInterface, const char* ipAddr);
    int clearUdpForwarding(const char* inInterface, const char* extInterface);
    int getUsbClientIp(const char* iface);
    int setNsiotFirewall(void);
    int clearNsiotFirewall(void);
    int setVolteNsiotFirewall(const char* iface);
    int clearVolteNsiotFirewall(const char* iface);
    int setPlmnIfaceRule(const char* channel, int iface_mask, FirewallRule rule);
    int setInterfaceForChainRule(const char* iface, ChildChain chain, FirewallRule rule);
    int setUidFwRule(int, FirewallChinaRule, FirewallRule);
    int clearFwChain(const char* chain);
    int setpriorityuid(int uid);
    int clearpriorityuid(int uid);
    int clearpriorityuidall();
    int setprioritytoup(const char *src_ip,  const char *src_port, const char *dst_ip,
                                       const char *dst_port, const char *protocol);
    int clearprioritytoup(const char *src_ip,  const char *src_port, const char *dst_ip,
                                       const char *dst_port, const char *protocol);
    int clearprioritytoupall();

private:

    static bool openNsiotFlag;
    static bool openNsiotVolteFlag;
};

}  // namespace netdagent
}  // namespace android

#endif

