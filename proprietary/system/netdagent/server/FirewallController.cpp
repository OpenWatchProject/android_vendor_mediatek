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

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cutils/properties.h>
#include <arpa/inet.h>
#include <ifcutils/ifc.h>

#define LOG_TAG "NetdagentFirewall"
#include "log/log.h"
#include "NetdagentUtils.h"
#include "FirewallController.h"

namespace android {
namespace netdagent {

static const char* NSIOT_WHITE_LIST[] = {
        "1.1.1.1",
        "1.2.3.4",
        NULL,
};

#define PRIOMARK1 "0x80000000"
#define PRIOMARK2 "0x40000000"

static int prio_cnt = 0;

const char* FirewallController::LOCAL_FILTER_INPUT = "fw_INPUT";  //AOSP chain
const char* FirewallController::LOCAL_FILTER_OUTPUT = "fw_OUTPUT";  //AOSP chain
const char* FirewallController::LOCAL_FILTER_OEMOUTPUT = "oem_out";  //AOSP chain
const char* FirewallController::LOCAL_MANGLE_POSTROUTING = "oem_mangle_post"; //AOSP chain
const char* FirewallController::LOCAL_NAT_PREROUTING = "oem_nat_pre";  //AOSP chain
const char* FirewallController::LOCAL_FILTER_FORWARD = "oem_fwd";  //AOSP chain
const char* FirewallController::FIREWALL_CTA_MOBILE = "oem_cta_mobile";
const char* FirewallController::FIREWALL_CTA_ALL = "oem_cta_all";
const char* FirewallController::FIREWALL_BGDATA = "oem_data";
const char* FirewallController::FIREWALL_PRIOU = "oem_priou";
const char* FirewallController::FIREWALL_PRIOT = "oem_priot";

bool FirewallController::openNsiotFlag = false;
bool FirewallController::openNsiotVolteFlag = false;

int FirewallController::setupIptablesHooks(void) {

/*
    int res = 0;
    res |= execIptables(V4V6, "-t", "filter", "-F", FIREWALL_CTA_MOBILE, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-X", FIREWALL_CTA_MOBILE, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-N", FIREWALL_CTA_MOBILE, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-F", FIREWALL_CTA_ALL, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-X", FIREWALL_CTA_ALL, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-N", FIREWALL_CTA_ALL, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-F", FIREWALL_BGDATA, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-X", FIREWALL_BGDATA, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-N", FIREWALL_BGDATA, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-F", FIREWALL_PRIOU, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-X", FIREWALL_PRIOU, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-N", FIREWALL_PRIOU, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-F", FIREWALL_PRIOT, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-X", FIREWALL_PRIOT, NULL);
    res |= execIptables(V4V6, "-t", "filter", "-N", FIREWALL_PRIOT, NULL);
    res |= execIptables(V4V6, "-A", LOCAL_FILTER_OEMOUTPUT, "-o", "ppp+", "-j", FIREWALL_CTA_MOBILE, NULL);
    res |= execIptables(V4V6, "-A", LOCAL_FILTER_OEMOUTPUT, "-o", "ccmni+", "-j", FIREWALL_CTA_MOBILE, NULL);
    res |= execIptables(V4V6, "-A", LOCAL_FILTER_OEMOUTPUT, "-o", "ccemni+", "-j", FIREWALL_CTA_MOBILE, NULL);
    res |= execIptables(V4V6, "-A", LOCAL_FILTER_OEMOUTPUT, "-o", "usb+", "-j", FIREWALL_CTA_MOBILE, NULL);
    res |= execIptables(V4V6, "-A", LOCAL_FILTER_OEMOUTPUT, "-o", "cc2mni+", "-j", FIREWALL_CTA_MOBILE, NULL);
    res |= execIptables(V4V6, "-A", LOCAL_FILTER_OEMOUTPUT, "-o", "wlan+", "-j", FIREWALL_CTA_ALL, NULL);
    res |= execIptables(V4V6, "-A", LOCAL_FILTER_OEMOUTPUT, "-j", FIREWALL_BGDATA, NULL);
    res |= execIptables(V4V6, "-A", FIREWALL_BGDATA, "-j", FIREWALL_PRIOU, NULL);
    res |= execIptables(V4V6, "-A", FIREWALL_BGDATA, "-j", FIREWALL_PRIOT, NULL);
*/
    ALOGI("setupIptablesHooks done in oem_iptables_init\n");
    return 0;
}

/*MTK: support nsiot*/
int FirewallController::setUdpForwarding(const char* inInterface, const char* extInterface, const char* ipAddr) {
    struct in_addr s4;
    IptablesTarget target = V4;
    int res = 0;

    if(inInterface==NULL || extInterface==NULL || ipAddr==NULL){
        ALOGE("setUdpForwarding: invalid args");
        return -1;
    }
    /* Bug Fix : Support down stream mdbr0 for Nsiot Test*/
    if((0 != strncmp(extInterface, "cc", 2)) || (0 != strncmp(inInterface, "rndis", 5))) {
        ALOGE("setUdpForwarding: extInterface %s and inInterface %s is not permitted", extInterface, inInterface);
        return -1;
    }

    ALOGD("setUdpForwarding: %s-%s", inInterface, extInterface);

    if (inet_pton(AF_INET, ipAddr, &s4) != 1){
        ALOGE("setUdpForwarding: invalid IPv4 address");
        return -1;
    }

    //Delete the old IPTABLE rule
    res |= execIptables(target, "-F", LOCAL_FILTER_FORWARD, NULL);
    res |= execIptables(target, "-I", LOCAL_FILTER_FORWARD, "-i", inInterface, "-o", extInterface, "-j", "ACCEPT", NULL);
    res |= execIptables(target, "-I", LOCAL_FILTER_FORWARD, "-i", extInterface, "-o", inInterface, "-j", "ACCEPT", NULL);
    res |= execIptables(target, "-t", "nat", "-F", LOCAL_NAT_PREROUTING, NULL);
    res |= execIptables(target, "-t", "nat", "-I", LOCAL_NAT_PREROUTING, "-i", extInterface, "-j", "DNAT", "--to", ipAddr, NULL);

    return res;
}

/*MTK: support nsiot*/
int FirewallController::clearUdpForwarding(const char* inInterface, const char* extInterface) {
    IptablesTarget target = V4;
    int res = 0;

    if(inInterface==NULL || extInterface==NULL){
        ALOGW("clearUdpForwarding: invalid args");
    } else {
        ALOGD("clearUdpForwarding: %s-%s", inInterface, extInterface);
    }
    //Delete the old IPTABLE rule
    res |= execIptables(target, "-F", LOCAL_FILTER_FORWARD, NULL);
    res |= execIptables(target, "-t", "nat", "-F", LOCAL_NAT_PREROUTING, NULL);
    property_set("vendor.net.rndis.client", "");
    return res;

}

/*support nsiot*/
int FirewallController::setNsiotFirewall(void) {
    int res = 0;
    IptablesTarget target = V4;
    const char** allowed_ip = NSIOT_WHITE_LIST;

    //mkt07384: if nsiot is opened , do nothing
    if(openNsiotFlag){
        ALOGD("Nsiot already opened!");
         return 0;
      }
    //volte-nsiot open
     if(openNsiotVolteFlag){
        res |= execIptables(target, "-t", "filter", "-A", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-m", "string", "--string", "spirent",
                        "--algo", "bm","-j", "ACCEPT",NULL);
        res |= execIptables(target, "-t", "filter", "-A", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-m", "string", "--string", "slp.rs.de",
                         "--algo", "bm","-j", "ACCEPT",NULL);
        res |= execIptables(target, "-t", "filter", "-A", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-m", "string", "--string",
                       "3gppnetwork", "--algo", "bm","-j", "ACCEPT",NULL);
        res |= execIptables(V4V6, "-t", "filter", "-A", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-j", "DROP", NULL);
        while(*allowed_ip != NULL){
              res |= execIptables(target, "-t", "filter", "-A", FIREWALL_BGDATA, "-d", *allowed_ip, "-j", "ACCEPT", NULL);
              allowed_ip++;
         }
         res |= execIptables(target, "-t", "filter", "-A", FIREWALL_BGDATA, "-o", "cc+", "-j", "DROP", NULL);
         res |= execIptables(target, "-t", "filter", "-A", FIREWALL_BGDATA, "-o", "ppp+", "-j", "DROP", NULL);
      }else{
        // volte-nsiot
        res |= execIptables(V4V6, "-t", "filter", "-I", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-j", "DROP", NULL);
        res |= execIptables(target, "-t", "filter", "-I", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-m", "string", "--string", "spirent",
              "--algo", "bm","-j", "ACCEPT",NULL);
        res |= execIptables(target, "-t", "filter", "-I", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-m", "string", "--string", "slp.rs.de",
            "--algo", "bm","-j", "ACCEPT",NULL);
        res |= execIptables(target, "-t", "filter", "-I", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-m", "string", "--string",
             "3gppnetwork", "--algo", "bm","-j", "ACCEPT",NULL);
        res |= execIptables(target, "-t", "filter", "-A", FIREWALL_BGDATA, "-o", "ppp+", "-j", "DROP", NULL);
        }
        openNsiotFlag = true;
        return res;
}

/*MTK: support nsiot*/
int FirewallController::clearNsiotFirewall(void) {
    int res = 0;
    IptablesTarget target = V4;

    if(openNsiotFlag)
    {
        res = execIptables(target, "-t", "filter", "-F", FIREWALL_BGDATA, NULL);
        openNsiotFlag = false;
    }else
       {
          ALOGE("clearNsiotFirewall invalid,openNsiotFlag = flase");
       }

    return res;
}

/*MTK: support nsiot*/
int FirewallController::setVolteNsiotFirewall(const char* iface){
    int res = 0;
    IptablesTarget target = V4;

     if(iface == NULL){
          ALOGE("setVolteNsiotFirewall: Error iface");
          return -1;
       }
     if(openNsiotVolteFlag){
         ALOGD("VolteNsiot already opened!");
           return 0;
       }
    res |= execIptables(V4V6, "-t", "filter", "-I", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-m", "string", "--string",
           "xcap", "--algo", "bm","-j", "ACCEPT",NULL);
    res |= execIptables(V4V6, "-t", "filter", "-I", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-m", "string", "--string",
           "bsf", "--algo", "bm","-j", "ACCEPT",NULL);
    res |= execIptables(target, "-t", "filter", "-I", FIREWALL_BGDATA, "-o", iface,"-j", "ACCEPT",NULL);
    openNsiotVolteFlag = true;
    return res;
}

/*MTK: support nsiot*/
int FirewallController::clearVolteNsiotFirewall(const char* iface){

    int res = 0;
    IptablesTarget target = V4;
    if(iface == NULL){
       ALOGE("clearVolteNsiotFirewall: Error iface");
       return -1;
     }
    if(openNsiotVolteFlag){
        res |= execIptables(target, "-t", "filter", "-D", FIREWALL_BGDATA, "-o", iface,"-j", "ACCEPT",NULL);
        res |= execIptables(V4V6, "-t", "filter", "-D", FIREWALL_BGDATA,"-p", "udp", "--dport", "53", "-m", "string",
                 "--string", "xcap", "--algo", "bm","-j", "ACCEPT",NULL);
        res |= execIptables(V4V6, "-t", "filter", "-D", FIREWALL_BGDATA, "-p", "udp", "--dport", "53", "-m", "string",
                 "--string", "bsf", "--algo", "bm","-j", "ACCEPT",NULL);
        openNsiotVolteFlag = false;
     }
      //skip fail
      return 0;
}

/*MTK: support nsiot*/
int FirewallController::getUsbClientIp(const char * iface){
        const char *file_path = "/proc/net/arp";
        char rawaddrstr[20];
        char tempaddrstr[20];
        char flag[10];
        char arp_line[128];
        char netdev[20];
        unsigned int line, found;
        FILE *f = fopen(file_path, "r");
        if (!f) {
           ALOGE("open file:%s failed\n", file_path);
          return -errno;
         }
         memset(rawaddrstr, 0, sizeof(rawaddrstr));
         memset(arp_line, 0, sizeof(arp_line));
          // Format:
          // lease file: 1234567890 00:11:22:33:44:55 255.255.255.255 android-hostname *
          // arp: 255.255.255.255 0x1 0x2 00:11:22:33:44:55 * rndis0
          // Fix MDT bug : Support mdbr0
         line = 0; found = 0;
         while(fgets(arp_line, sizeof(arp_line), f) != NULL){
             if(line == 0)
                  ALOGD("arp head: %s", arp_line);
                    else{
                       if(sscanf(arp_line, "%16s %*8s %8s %*18s %*16s %16s",
                                 rawaddrstr, flag, netdev) == 3){
                            ALOGD("IP addr = %s, flag=%s, dev=%s\n", rawaddrstr, flag, netdev);
                            if(0 == strncmp(netdev, "rndis", 5) || 0 == strncmp(netdev, "mdbr", 4)){
                                       memset(tempaddrstr, 0, sizeof(tempaddrstr));
                                       strncpy(tempaddrstr, rawaddrstr, strlen(rawaddrstr)+1);
                                       found = 1;
                                       if(0 == strncmp(flag, "0x2", 3)){
                                        /*get the reachable client, break out now*/
                                         ALOGI("find rndis client: %s\n", tempaddrstr);
                                          break;
                                       }
                             }
                        }
                      memset(rawaddrstr, 0, sizeof(rawaddrstr));
                 }
                line++;
           }
          fclose(f);
        if(found == 1){
            property_set("vendor.net.rndis.client", tempaddrstr);
        } else {
            ALOGW("can't find %s client!", iface);
          }
         return 0;
}

int FirewallController::setInterfaceForChainRule(const char* iface, ChildChain chain, FirewallRule rule) {
    const char* op;
    const char* target;

    if (!isIfaceName(iface)) {
        errno = ENOENT;
        return -1;
    }

    int res = 0;
    switch(chain) {
        case DOZABLE:
        case POWERSAVE:
            target = "RETURN";
            // When adding, insert RETURN rules at the front, before the catch-all DROP at the end.
            op = (rule == ALLOW)? "-I" : "-D";
            res |= execIptables(V4V6, op, LOCAL_FILTER_INPUT, "-i", iface, "-j", target, NULL);
            res |= execIptables(V4V6, op, LOCAL_FILTER_OUTPUT, "-o", iface, "-j", target, NULL);
            break;
        default:
            ALOGW("UnSupport child chain: %d", chain);
            break;
    }
    return res;
}

/****************************************************************************
* MTK: support CTA firewall
* set FIREWALL_CTA_MOBILE and FIREWALL_CTA_ALL rule to reject packets based on uid
*****************************************************************************/
int FirewallController::setUidFwRule(int uid, FirewallChinaRule chain, FirewallRule rule) {
    char uidStr[16];
    int res = 0;
    const char* op;
    const char* fwChain;

    sprintf(uidStr, "%d", uid);

    if (rule == ALLOW) {
        op = "-I";
    } else {
        op = "-D";
    }

    if (chain == MOBILE) {
        fwChain = FIREWALL_CTA_MOBILE;
    } else {
        fwChain = FIREWALL_CTA_ALL;
    }

    res |= execIptables(V4, op, fwChain, "-m", "owner", "--uid-owner", uidStr,
                "-j", "DROP", NULL);

    res |= execIptables(V6, op, fwChain, "-m", "owner", "--uid-owner", uidStr,
                "-j", "DROP", NULL);

    return res;
}

/************************************************************
* MTK: support CTA firewall
* flush FIREWALL_CTA_MOBILE and FIREWALL_CTA_ALL
************************************************************/
int FirewallController::clearFwChain(const char* chain) {
    int res = 0;

    if(chain != NULL){
        if(strlen(chain) > 0){
            res |= execIptables(V4V6, "-F", chain, NULL);
        }else{
            ALOGD("Clear all chain");
            res |= execIptables(V4V6, "-F", NULL);
        }
    }else{
        ALOGE("Chain is NULL");
    }

    return res;
}


/*mtk for plmn interface blocking*/
int FirewallController::setPlmnIfaceRule(const char* channel, int iface_mask, FirewallRule rule) {
    int res = 0;
    char iface[IFNAMSIZ];
    int i;
    #define BITMAPSIZE 32
    long bitmap[BITMAPSIZE] = { 1, 1<<1, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7, 1<<8, 1<<9,
                                1<<10,1<<11, 1<<12, 1<<13, 1<<14, 1<<15, 1<<16, 1<<17,
                                1<<18, 1<<19, 1<<20, 1<<21, 1<<22, 1<<23, 1<<24, 1<<25,
                                1<<26, 1<<27, 1<<28, 1<<29, 1<<30, 1<<31 };


    if (!isIfaceName(channel)) {
        ALOGI("setPlmnIfaceRule: %s is not interface name", channel);
        return -1;
    }

    if(strncmp(channel, "cc", 2)) {
        ALOGI("setPlmnIfaceRule: %s channel is not supported", channel);
        return -1;
    }

    const char* op;
    if (rule == ALLOW) {
        op = "-I";
    } else {
        op = "-D";
    }

    for (i = 0; i < BITMAPSIZE; i++) {
        if (iface_mask & bitmap[i]) {
            sprintf(iface, "%s%u", channel, i);
            res |= execIptables(V4V6, "-t", "mangle", op, LOCAL_MANGLE_POSTROUTING, "-o", iface, "-j", "DROP", NULL);
        }
    }
    return res;
}

/*MTK support uid priority*/
int FirewallController::setpriorityuid(int uid) {
    char uidStr[16];
    int res = 0;
    if (!prio_cnt)
        ifc_enable_ack_prio(false);
    prio_cnt ++;
    sprintf(uidStr, "%d", uid);
    res |= execIptables(V4V6, "-t", "filter", "-A", FIREWALL_PRIOU, "-m", "owner", "--uid-owner", uidStr,
                        "-j", "MARK", "--set-mark", PRIOMARK1, NULL);
    return res;
}

int FirewallController::clearpriorityuid(int uid) {
    char uidStr[16];
    int res = 0;
    prio_cnt --;
    if (!prio_cnt)
        ifc_enable_ack_prio(true);
    sprintf(uidStr, "%d", uid);
    res |= execIptables(V4V6, "-D", FIREWALL_PRIOU, "-t", "filter", "-m", "owner", "--uid-owner", uidStr,
                        "-j", "MARK", "--set-mark", PRIOMARK1, NULL);
    return res;
}

int FirewallController::clearpriorityuidall() {
    int res = 0;
    if (!prio_cnt)
        ifc_enable_ack_prio(true);
    res |= execIptables(V4V6, "-F", FIREWALL_PRIOU, "-t", "filter", NULL);
    return res;
}

/*MTK support touple priority*/
int FirewallController::setprioritytoup(const char *src_ip,  const char *src_port, const char *dst_ip,
                                       const char *dst_port, const char *protocol) {
    int res = 0;
    if (!prio_cnt)
        ifc_enable_ack_prio(false);
    prio_cnt ++;
    if (!strcmp(src_port, "none") && !strcmp(dst_port, "none"))
            res |= execIptables(strchr(src_ip, ':') ? V6 : V4, "-t", "filter", "-A", FIREWALL_PRIOT, "-p",
                                protocol, "-s", src_ip, "-d", dst_ip, "-j", "MARK", "--set-mark", PRIOMARK2, NULL);
    else if (!strcmp(src_port, "none"))
            res |= execIptables(strchr(src_ip, ':') ? V6 : V4, "-t", "filter", "-A", FIREWALL_PRIOT, "-p",
                                protocol, "-s", src_ip, "-d", dst_ip, "--dport", dst_port, "-j", "MARK",
                                "--set-mark", PRIOMARK2, NULL);
    else if (!strcmp(dst_port, "none"))
            res |= execIptables(strchr(src_ip, ':') ? V6 : V4, "-t", "filter", "-A", FIREWALL_PRIOT, "-p",
                                protocol, "-s", src_ip, "--sport", src_port, "-d", dst_ip, "-j", "MARK",
                                "--set-mark", PRIOMARK2, NULL);
   else
        res |= execIptables(strchr(src_ip, ':') ? V6 : V4, "-t", "filter", "-A", FIREWALL_PRIOT, "-p",
                            protocol, "-s", src_ip, "--sport", src_port, "-d", dst_ip,
                            "--dport", dst_port, "-j", "MARK", "--set-mark", PRIOMARK2, NULL);
    return res;
}

int FirewallController::clearprioritytoup(const char *src_ip,  const char *src_port, const char *dst_ip,
                                       const char *dst_port, const char *protocol) {
    int res = 0;
    prio_cnt --;
    if (!prio_cnt)
        ifc_enable_ack_prio(true);
    if (!strcmp(src_port, "none") && !strcmp(dst_port, "none"))
            res |= execIptables(strchr(src_ip, ':') ? V6 : V4, "-t", "filter", "-D", FIREWALL_PRIOT, "-p",
                                protocol, "-s", src_ip, "-d", dst_ip, "-j", "MARK", "--set-mark", PRIOMARK2, NULL);
    else if (!strcmp(src_port, "none"))
            res |= execIptables(strchr(src_ip, ':') ? V6 : V4, "-t", "filter", "-D", FIREWALL_PRIOT, "-p",
                                protocol, "-s", src_ip, "-d", dst_ip, "--dport", dst_port, "-j", "MARK",
                                "--set-mark", PRIOMARK2, NULL);
    else if (!strcmp(dst_port, "none"))
            res |= execIptables(strchr(src_ip, ':') ? V6 : V4, "-t", "filter", "-D", FIREWALL_PRIOT, "-p",
                                protocol, "-s", src_ip, "--sport", src_port, "-d", dst_ip, "-j", "MARK",
                                "--set-mark", PRIOMARK2, NULL);
   else
        res |= execIptables(strchr(src_ip, ':') ? V6 : V4, "-t", "filter", "-D", FIREWALL_PRIOT, "-p",
                            protocol, "-s", src_ip, "--sport", src_port, "-d", dst_ip,
                            "--dport", dst_port, "-j", "MARK", "--set-mark", PRIOMARK2, NULL);
    return res;
}

int FirewallController::clearprioritytoupall() {
    int res = 0;
    if (!prio_cnt)
        ifc_enable_ack_prio(true);
    res |= execIptables(V4V6, "-F", FIREWALL_PRIOT, "-t", "filter", NULL);
    return res;
}

}  // namespace netdagent
}  // namespace android

