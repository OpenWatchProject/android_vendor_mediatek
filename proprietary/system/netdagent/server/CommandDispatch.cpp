/*
 * Copyright (C) 2009-2016 The Android Open Source Project
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <unistd.h>
#define LOG_TAG "NetdagentDispatch"
#include "log/log.h"
#include "CommandController.h"
#include "CommandDispatch.h"


using android::netdagent::gCtls;

namespace android {
namespace netdagent {

FirewallCmd::FirewallCmd() : CommandDispatch("firewall") {
}

int FirewallCmd::sendGenericOkFail(CommandRespondor *cr, int cond) {
    if (!cond) {
        cr->sendMsg(ResponseCode::CommandOkay, "Firewall command succeeded", false);
    } else {
        cr->sendMsg(ResponseCode::OperationFailed, "Firewall command failed", false);
    }
    return 0;
}

FirewallRule FirewallCmd::parseRule(const char* arg) {
    if (!strcmp(arg, "allow")) {
        return ALLOW;
    } else if (!strcmp(arg, "deny")) {
        return DENY;
    } else {
        ALOGE("failed to parse uid rule (%s)", arg);
        return ALLOW;
    }
}

ChildChain FirewallCmd::parseChildChain(const char* arg) {
    if (!strcmp(arg, "dozable")) {
        return DOZABLE;
    } else if (!strcmp(arg, "standby")) {
        return STANDBY;
    } else if (!strcmp(arg, "powersave")) {
        return POWERSAVE;
    } else if (!strcmp(arg, "none")) {
        return NONE;
    } else {
        ALOGE("failed to parse child firewall chain (%s)", arg);
        return INVALID_CHAIN;
    }
}

FirewallChinaRule FirewallCmd::parseChain(const char* arg) {
    if (!strcmp(arg, "mobile")) {
            return MOBILE;
    } else {
            return WIFI;
    }
}

int FirewallCmd::runCommand(CommandRespondor *cr, int argc, char **argv) {
    if (argc < 2) {
        cr->sendMsg(ResponseCode::CommandSyntaxError, "Missing command", false);
        return 0;
    }

    if (!strcmp(argv[1], "set_udp_forwarding")) {
        if (argc != 5) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall set_udp_forwarding internalInterface externalInterface ipAddr",
                         false);
            return 0;
        }

        const char* inInterface = argv[2];
        const char* extInterface = argv[3];
        const char* ipAddr = argv[4];

        int res = 0;
        res |= gCtls->firewallCtrl.setUdpForwarding(inInterface, extInterface, ipAddr);
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "clear_udp_forwarding")) {
        if (argc != 4) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall clear_udp_forwarding internalInterface externalInterface",
                         false);
            return 0;
        }

        const char* inInterface = argv[2];
        const char* extInterface = argv[3];

        int res = 0;
        res |= gCtls->firewallCtrl.clearUdpForwarding(inInterface, extInterface);
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "get_usb_client")) {
        if (argc != 3) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall get_usb_client interface",
                         false);
            return 0;
        }
        int res = 0;
        res |= gCtls->firewallCtrl.getUsbClientIp(argv[2]);
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "set_nsiot_firewall")) {
        int res = gCtls->firewallCtrl.setNsiotFirewall();
        return sendGenericOkFail(cr, res);
    }
    if (!strcmp(argv[1], "clear_nsiot_firewall")) {
        int res = gCtls->firewallCtrl.clearNsiotFirewall();
        return sendGenericOkFail(cr, res);
    }

    //add pass volte packet for VOLTE NSIOT test
    if(!strcmp(argv[1], "set_volte_nsiot_firewall")){
        if (argc != 3) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall set_volte_nsiot_firewall <interface>",
                         false);
            return 0;
        }
       int res = gCtls->firewallCtrl.setVolteNsiotFirewall(argv[2]);
       return sendGenericOkFail(cr, res);
    }

    if(!strcmp(argv[1], "clear_volte_nsiot_firewall")){
        if (argc != 3) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall clearVolteNsiotFirewall <interface>",
                         false);
            return 0;
        }
        int res = gCtls->firewallCtrl.clearVolteNsiotFirewall(argv[2]);
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "set_plmn_iface_rule")) {
        if (argc != 5) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall set_plmn_iface_rule <channel> <mask> <allow|deny>", false);
            return 0;
        }

        const char* channel = argv[2];
        int mask = atoi(argv[3]);
        FirewallRule rule = parseRule(argv[4]);

        int res = gCtls->firewallCtrl.setPlmnIfaceRule(channel, mask, rule);
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "set_interface_for_chain_rule")) {
        if (argc != 5) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall set_interface_for_chain_rule <interface> <dozable|powersave> <allow|deny>", false);
            return 0;
        }

        const char* iface = argv[2];
        ChildChain childChain = parseChildChain(argv[3]);
        FirewallRule rule = parseRule(argv[4]);
        int res = gCtls->firewallCtrl.setInterfaceForChainRule(iface, childChain, rule);
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "priority_set_uid")) {
        if (argc != 3) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall priority_set_uid <uid>", false);
            return 0;
        }
        int uid = atoi(argv[2]);
        int res = gCtls->firewallCtrl.setpriorityuid(uid);
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "priority_clear_uid")) {
        if (argc != 3) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall priority_clear_uid <uid>", false);
            return 0;
        }
        int uid = atoi(argv[2]);
        int res = gCtls->firewallCtrl.clearpriorityuid(uid);
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "priority_clear_uid_all")) {
        if (argc != 2) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall priority_clear_all", false);
            return 0;
        }
        int res = gCtls->firewallCtrl.clearpriorityuidall();
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "priority_set_toup")) {
        if (argc != 7) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall priority_set_toup <src_ip> <src_port> <dst_ip, <dst_port>, <protocol>", false);
            return 0;
        }
        const char *src_ip = argv[2];
        const char *src_port = argv[3];
        const char *dst_ip = argv[4];
        const char *dst_port = argv[5];
        const char *proto = argv[6];

        int res = gCtls->firewallCtrl.setprioritytoup(src_ip, src_port, dst_ip, dst_port, proto);
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "priority_clear_toup")) {
        if (argc != 7) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall priority_clear_toup <src_ip> <src_port> <dst_ip, <dst_port>, <protocol>", false);
            return 0;
        }
        const char *src_ip = argv[2];
        const char *src_port = argv[3];
        const char *dst_ip = argv[4];
        const char *dst_port = argv[5];
        const char *proto = argv[6];

        int res = gCtls->firewallCtrl.clearprioritytoup(src_ip, src_port, dst_ip, dst_port, proto);
        return sendGenericOkFail(cr, res);
    }

    if (!strcmp(argv[1], "priority_clear_toup_all")) {
        if (argc != 2) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                         "Usage: firewall priority_clear_toup_all", false);
            return 0;
        }
        int res = gCtls->firewallCtrl.clearprioritytoupall();
        return sendGenericOkFail(cr, res);
    }

    cr->sendMsg(ResponseCode::CommandSyntaxError, "Unknown firewall command", false);
    return 0;
}

int FirewallCmd::runCommand(int argc, char **argv) {
    if (argc < 2) {
        ALOGE("Missing command using %s\n", argv[0]);
        return -1;
    }

    if (!strcmp(argv[1], "set_udp_forwarding")) {
        if (argc != 5) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }

        const char* inInterface = argv[2];
        const char* extInterface = argv[3];
        const char* ipAddr = argv[4];

        int res = 0;
        res |= gCtls->firewallCtrl.setUdpForwarding(inInterface, extInterface, ipAddr);
        return res;
    }

    if (!strcmp(argv[1], "clear_udp_forwarding")) {
        if (argc != 4) {
        ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }

        const char* inInterface = argv[2];
        const char* extInterface = argv[3];

        int res = 0;
        res |= gCtls->firewallCtrl.clearUdpForwarding(inInterface, extInterface);
        return res;
    }

    if (!strcmp(argv[1], "get_usb_client")) {
        if (argc != 3) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }
        int res = 0;
        res |= gCtls->firewallCtrl.getUsbClientIp(argv[2]);
        return res;
    }

    if (!strcmp(argv[1], "set_nsiot_firewall")) {
        int res = gCtls->firewallCtrl.setNsiotFirewall();
        return res;
    }
    if (!strcmp(argv[1], "clear_nsiot_firewall")) {
        int res = gCtls->firewallCtrl.clearNsiotFirewall();
        return res;
    }

    //add pass volte packet for VOLTE NSIOT test
    if(!strcmp(argv[1], "set_volte_nsiot_firewall")){
        if (argc != 3) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }
       int res = gCtls->firewallCtrl.setVolteNsiotFirewall(argv[2]);
       return res;
    }

    if(!strcmp(argv[1], "clear_volte_nsiot_firewall")){
        if (argc != 3) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }
        int res = gCtls->firewallCtrl.clearVolteNsiotFirewall(argv[2]);
        return res;
    }

    if (!strcmp(argv[1], "set_plmn_iface_rule")) {
        if (argc != 5) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }

        const char* channel = argv[2];
        int mask = atoi(argv[3]);
        FirewallRule rule = parseRule(argv[4]);

        int res = gCtls->firewallCtrl.setPlmnIfaceRule(channel, mask, rule);
        return res;
    }

    if (!strcmp(argv[1], "set_uid_fw_rule")) {
        if (argc != 5) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }

        int uid = atoi(argv[2]);
        FirewallChinaRule chain = parseChain(argv[3]);
        FirewallRule rule = parseRule(argv[4]);

        int res = gCtls->firewallCtrl.setUidFwRule(uid, chain, rule);
        return res;
    }

    if (!strcmp(argv[1], "clear_fw_chain")) {
        if (argc != 3) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }

        const char* chain = argv[2];

        int res = gCtls->firewallCtrl.clearFwChain(chain);
        return res;
    }

    if (!strcmp(argv[1], "set_interface_for_chain_rule")) {
        if (argc != 5) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }

        const char* iface = argv[2];
        ChildChain childChain = parseChildChain(argv[3]);
        FirewallRule rule = parseRule(argv[4]);
        int res = gCtls->firewallCtrl.setInterfaceForChainRule(iface, childChain, rule);
        return res;
    }

   if (!strcmp(argv[1], "priority_set_uid")) {
        if (argc != 3) {
            ALOGE("Usage: firewall priority_set_uid <uid>");
            return 0;
        }
        int uid = atoi(argv[2]);
        int res = gCtls->firewallCtrl.setpriorityuid(uid);
        return res;
    }

    if (!strcmp(argv[1], "priority_clear_uid")) {
        if (argc != 3) {
            ALOGE("Usage: firewall priority_clear_uid <uid>");
            return 0;
        }
        int uid = atoi(argv[2]);
        int res = gCtls->firewallCtrl.clearpriorityuid(uid);
        return res;
    }

    if (!strcmp(argv[1], "priority_clear_uid_all")) {
        if (argc != 2) {
            ALOGE("Usage: firewall priority_clear_all");
            return 0;
        }
        int res = gCtls->firewallCtrl.clearpriorityuidall();
        return res;
    }

    if (!strcmp(argv[1], "priority_set_toup")) {
        if (argc != 7) {
            ALOGE("Usage: firewall priority_set_toup <src_ip> <src_port> <dst_ip, <dst_port>, <protocol>");
            return -1;
        }
        const char *src_ip = argv[2];
        const char *src_port = argv[3];
        const char *dst_ip = argv[4];
        const char *dst_port = argv[5];
        const char *proto = argv[6];

        int res = gCtls->firewallCtrl.setprioritytoup(src_ip, src_port, dst_ip, dst_port, proto);
        return res;
    }

    if (!strcmp(argv[1], "priority_clear_toup")) {
        if (argc != 7) {
            ALOGE("Usage: firewall priority_clear_toup <src_ip> <src_port> <dst_ip, <dst_port>, <protocol>");
            return -1;
        }
        const char *src_ip = argv[2];
        const char *src_port = argv[3];
        const char *dst_ip = argv[4];
        const char *dst_port = argv[5];
        const char *proto = argv[6];

        int res = gCtls->firewallCtrl.clearprioritytoup(src_ip, src_port, dst_ip, dst_port, proto);
        return res;
    }

    if (!strcmp(argv[1], "priority_clear_toup_all")) {
        if (argc != 2) {
            ALOGE("Usage: firewall priority_clear_toup_all");
            return -1;
        }
        int res = gCtls->firewallCtrl.clearprioritytoupall();
        return res;
    }

    ALOGE("Unknown firewall %s command\n", argv[1]);
    return -1;
}


ThrottleCmd::ThrottleCmd() : CommandDispatch("throttle") {
}

int ThrottleCmd::runCommand(CommandRespondor *cr, int argc, char **argv) {
    if (argc < 2) {
        cr->sendMsg(ResponseCode::CommandSyntaxError, "Missing argument", false);
        return 0;
    }

   if (!strcmp(argv[1], "cat")) {
        if (argc != 4 || (argc == 4 && (strcmp(argv[3], "rx") && (strcmp(argv[3], "tx"))))) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                    "Usage: throttle cat <interface> <rx|tx>", false);
            return 0;
        }
        int val = 0;
        int rc = 0;

        if(!strcmp(argv[2], "modem")){
            if (!strcmp(argv[3], "rx")) {
                rc = gCtls->throttleCtrl.getModemRxThrottle(&val);
            } else {
                rc = gCtls->throttleCtrl.getModemTxThrottle(&val);
            }
        } else {
            if (!strcmp(argv[3], "rx")) {
                rc = gCtls->throttleCtrl.getInterfaceRxThrottle(argv[2], &val);
            } else {
                rc = gCtls->throttleCtrl.getInterfaceTxThrottle(argv[2], &val);
            }
        }

        if (rc) {
            cr->sendMsg(ResponseCode::OperationFailed, "Failed to get throttle", true);
        } else {
            char *msg = NULL;
            asprintf(&msg, "%u", val);
            cr->sendMsg(ResponseCode::CommandOkay, msg, false);
            free(msg);
            return 0;
        }
        return 0;
    } 

    if (!strcmp(argv[1], "set")) {
        if (argc != 5) {
            cr->sendMsg(ResponseCode::CommandSyntaxError,
                    "Usage: throttle set <interface> <rx_kbps> <tx_kbps>", false);
            return 0;
        }
        if(!strcmp(argv[2], "modem")){
            if (gCtls->throttleCtrl.setModemThrottle(atoi(argv[3]),atoi(argv[4]))) {
                cr->sendMsg(ResponseCode::OperationFailed, "Failed to set modem throttle", true);
            } else {
                cr->sendMsg(ResponseCode::CommandOkay, "Interface throttling set", false);
            }
            return 0;
        }
        if (gCtls->throttleCtrl.setInterfaceThrottle(argv[2], atoi(argv[3]), atoi(argv[4]))) {
            cr->sendMsg(ResponseCode::OperationFailed, "Failed to set throttle", true);
        } else {
            cr->sendMsg(ResponseCode::CommandOkay, "Interface throttling set", false);
        }
        return 0;
    }

    cr->sendMsg(ResponseCode::CommandSyntaxError, "Unknown throttle cmd", false);
    return 0;
}

int ThrottleCmd::runCommand(int argc, char **argv) {
    if (argc < 2) {
        ALOGE("Missing command using %s\n", argv[0]);
        return -1;
    }

   if (!strcmp(argv[1], "cat")) {
        if (argc != 4 || (argc == 4 && (strcmp(argv[3], "rx") && (strcmp(argv[3], "tx"))))) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }
        int val = 0;
        int rc = 0;

        if(!strcmp(argv[2], "modem")){
            if (!strcmp(argv[3], "rx")) {
                rc = gCtls->throttleCtrl.getModemRxThrottle(&val);
            } else {
                rc = gCtls->throttleCtrl.getModemTxThrottle(&val);
            }
        } else {
            if (!strcmp(argv[3], "rx")) {
                rc = gCtls->throttleCtrl.getInterfaceRxThrottle(argv[2], &val);
            } else {
                rc = gCtls->throttleCtrl.getInterfaceTxThrottle(argv[2], &val);
            }
        }

        return rc;
    } 

    if (!strcmp(argv[1], "set")) {
        if (argc != 5) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }
        int res;
        if(!strcmp(argv[2], "modem")){
            res = gCtls->throttleCtrl.setModemThrottle(atoi(argv[3]),atoi(argv[4]));
            return res;
        }
        res = gCtls->throttleCtrl.setInterfaceThrottle(argv[2], atoi(argv[3]), atoi(argv[4]));
        return res;
    }

    ALOGE("Unknown throttle %s command\n", argv[1]);
    return -1;
}

NetworkCmd::NetworkCmd() : CommandDispatch("network") {
}

int NetworkCmd::runCommand(CommandRespondor* cr, int argc, char** argv) {
    if (argc < 2) {
        cr->sendMsg(ResponseCode::CommandSyntaxError, "Missing argument", false);
        return 0;
    }

    //    0       1         2              3           4         5        6        7
    // network forward <enable|disable> <inIface> <outIface> <nxthop> <tableId> <family>
    if(!strcmp(argv[1], "forward")) {
        if(argc < 7) {
            cr->sendMsg(ResponseCode::CommandSyntaxError, "Missing argument", false);
            return 0;
        }
        int family = atoi(argv[7]);
        if(!strcmp(argv[2], "enable"))
            gCtls->netCtrl.forwardIpsec(argv[3], argv[4], argv[5], argv[6], family, 1);
        else
            gCtls->netCtrl.forwardIpsec(argv[3], argv[4], argv[5], argv[6], family, 0);
        cr->sendMsg(ResponseCode::CommandOkay, "network command success", false);
        return 0;
    }

    cr->sendMsg(ResponseCode::CommandSyntaxError, "Unknown network command", false);
    return 0;
}

int NetworkCmd::runCommand(int argc, char** argv) {
    if (argc < 2) {
        ALOGE("Missing command using %s\n", argv[0]);
        return -1;
    }

    //    0       1         2              3           4         5        6        7
    // network forward <enable|disable> <inIface> <outIface> <nxthop> <tableId> <family>
    if(!strcmp(argv[1], "forward")) {
        if(argc < 7) {
            ALOGE("Missing command using %s %s\n", argv[0], argv[1]);
            return -1;
        }
        int family = atoi(argv[7]);
        if(!strcmp(argv[2], "enable"))
            gCtls->netCtrl.forwardIpsec(argv[3], argv[4], argv[5], argv[6], family, 1);
        else
            gCtls->netCtrl.forwardIpsec(argv[3], argv[4], argv[5], argv[6], family, 0);
        return 0;
    }

    ALOGE("Unknown network %s command\n", argv[1]);
    return -1;
}


#if 0
ThroughputCmd::ThroughputCmd() : CommandDispatch("throughput") {
}

int ThroughputCmd::runCommand(CommandRespondor *cr, int argc, char **argv) {

    int rc = 0;
    if (argc < 3) {
        ALOGE("ThroughputCmd argc error, argc = %d", argc);
        cr->sendMsg(ResponseCode::CommandSyntaxError, "Missing argument", false);
        return rc;
    }

    //ALOGI("ThroughputCmd: %s %s %s", argv[0],argv[1],argv[2]);
    if (!strcmp(argv[1], "monitor")) {
        if(!strcmp(argv[2], "start"))
            rc = gCtls->throughputMonitor.start();
        if(!strcmp(argv[2], "stop"))
            gCtls->throughputMonitor.stop();
    }

    if (!strcmp(argv[1], "dump")) {
        if(gCtls->perfCtrl.is_testsim() || (gCtls->perfCtrl.get_tether_perfhandle() != -1)) {
            if(!strcmp(argv[2], "start"))
                rc = gCtls->throughputMonitor.dumpOn();
            if(!strcmp(argv[2], "stop"))
                gCtls->throughputMonitor.dumpOff();
         } else
            rc = -1;
    }

    if (!rc) {
        cr->sendMsg(ResponseCode::CommandOkay, "throughput operation succeeded", false);
    } else {
        cr->sendMsg(ResponseCode::OperationFailed, "throughput operation failed", false);
    }

    return rc;
}


#endif

}  // namespace netdagent
}  // namespace android


