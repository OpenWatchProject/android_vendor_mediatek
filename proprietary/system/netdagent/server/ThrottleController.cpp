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
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/pkt_sched.h>

#define LOG_TAG "NetdagentThrottle"
#include "log/log.h"
#include "NetdagentUtils.h"
#include "ThrottleController.h"
#include "IptablesInterface.h"

namespace android {
namespace netdagent {

ThrottleController::ThrottleController(){
    mModemRx = -1;
    mModemTx = -1;
}

ThrottleController::~ThrottleController(){
}

int ThrottleController::setInterfaceThrottle(const char *iface, int rxKbps, int txKbps) {
    char cmd[512];
    char ifn[65];

    memset(ifn, 0, sizeof(ifn));
    strncpy(ifn, iface, sizeof(ifn)-1);

    if (txKbps == -1) {
        reset(ifn);
        return 0;
    }

    /*
     * by mtk80842, reset configuration before setting
     */
    reset(ifn);

    /*
     *
     * Target interface configuration
     *
     */

    /*
     * Add root qdisc for the interface
     */
    sprintf(cmd, "qdisc add dev %s root handle 1: htb default 1 r2q 1000", ifn);
    if (execTcCmd(cmd)) {
        ALOGE("Failed to add root qdisc (%s)", strerror(errno));
        goto fail;
    }

    /*
     * Add our egress throttling class
     */
    sprintf(cmd, "class add dev %s parent 1: classid 1:1 htb rate %dkbit", ifn, txKbps);
    if (execTcCmd(cmd)) {
        ALOGE("Failed to add egress throttling class (%s)", strerror(errno));
        goto fail;
    }

    if(rxKbps == -1){
        ALOGI("setInterfaceThrottle success but NO RX, ifn = %s", ifn);
        return 0;
    }

    /*
     * Bring up the IFD device
     */
    ifc_init();
    if (ifc_up("ifb0")) {
        ALOGE("Failed to up ifb0 (%s)", strerror(errno));
        goto fail;
    }

    /*
     * Add root qdisc for IFD
     */
    sprintf(cmd, "qdisc add dev ifb0 root handle 1: htb default 1 r2q 1000");
    if (execTcCmd(cmd)) {
        ALOGE("Failed to add root ifb qdisc (%s)", strerror(errno));
        goto fail;
    }

    /*
     * Add our ingress throttling class
     */
    sprintf(cmd, "class add dev ifb0 parent 1: classid 1:1 htb rate %dkbit", rxKbps);
    if (execTcCmd(cmd)) {
        ALOGE("Failed to add ingress throttling class (%s)", strerror(errno));
        goto fail;
    }

    /*
     * Add ingress qdisc for pkt redirection
     */
    sprintf(cmd, "qdisc add dev %s ingress", ifn);
    if (execTcCmd(cmd)) {
        ALOGE("Failed to add ingress qdisc (%s)", strerror(errno));
        goto fail;
    }

    /*
     * Add filter to link <ifn> -> ifb0
     */
    sprintf(cmd, "filter add dev %s parent ffff: protocol ip prio 10 u32 match "
            "u32 0 0 flowid 1:1 action mirred egress redirect dev ifb0", ifn);
    if (execTcCmd(cmd)) {
        ALOGE("Failed to add ifb filter (%s)", strerror(errno));
        goto fail;
    }

    ALOGI("setInterfaceThrottle success, ifn = %s", ifn);

    return 0;
fail:
    reset(ifn);
    return -1;
}

void ThrottleController::reset(const char *iface) {
    char cmd[128];

    ALOGI("reset %s qdisc", iface);

    sprintf(cmd, "qdisc del dev %s root", iface);
    if (execTcCmd(cmd)) {
        ALOGE("Fail to execute %s", cmd);
        return;
    }
    sprintf(cmd, "qdisc del dev %s ingress", iface);
    if (execTcCmd(cmd)) {
        ALOGE("Fail to execute %s", cmd);
        return;
    }
    sprintf(cmd, "qdisc del dev ifb0 root");
    if (execTcCmd(cmd)) {
        ALOGE("Fail to execute %s", cmd);
        return;
    }
    return;
}

int ThrottleController::getInterfaceRxThrottle(const char *iface, int *rx) {
    ALOGI("getInterfaceRxThrottle %s", iface);
    *rx = 0;
    return 0;
}

int ThrottleController::getInterfaceTxThrottle(const char *iface, int *tx) {
    ALOGI("getInterfaceTxThrottle %s", iface);
    *tx = 0;
    return 0;
}

int ThrottleController::getModemRxThrottle(int *rx) {
    *rx = mModemRx;
    return 0;
}

int ThrottleController::getModemTxThrottle(int *tx) {
    *tx = mModemTx;
    return 0;
}

int ThrottleController::setModemThrottle(int rxKbps, int txKbps) {
    unsigned int n = 0; unsigned int mask = 0;
    int idx = 0; unsigned int up = 0;
    const char* iface_list[] = {
      "ccmni0",
      "ccmni1",
      "ccmni2",
      0
    };

    mModemRx = rxKbps;
    mModemTx = txKbps;  
    ifc_init();
    while(iface_list[idx] != 0){
        up = 0;
        ifc_is_up(iface_list[idx], &up);
        if(up == 1){
            n++;
            mask |= (1 << idx);
            ALOGD("%s is up, n = %d, mask = 0x%x", iface_list[idx], n, mask); 
        }
        idx++;
    }
    ifc_close();
    if(n == 0){
        ALOGE("setModemThrottle: no modem interface is up !");
        return -1;
    }
    if(rxKbps >= 0)
        rxKbps = rxKbps/n;
    if(txKbps >= 0)
        txKbps = txKbps/n;
  
    idx = 0;
    while(iface_list[idx] != 0){
        if(mask & (1 << idx)){
            ALOGD("setModemThrottle for %s: rx %d, tx %d !", 
            iface_list[idx], rxKbps, txKbps);
            setInterfaceThrottle(iface_list[idx], rxKbps, txKbps);
        }
        idx++;
    }

    return 0;
}

int  ThrottleController::updateModemThrottle() {
   if(mModemRx < 0 && mModemTx < 0){
       ALOGW("updateModemThrottle warning: rx %d, tx %d !", mModemRx, mModemTx);
       return -1;
   }
   return setModemThrottle(mModemRx, mModemTx);
}

}  // namespace netdagent
}  // namespace android
