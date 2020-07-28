/* 

* Copyright Statement:

* This software/firmware and related documentation ("MediaTek Software") are

* protected under relevant copyright laws. The information contained herein

* is confidential and proprietary to MediaTek Inc. and/or its licensors.

* Without the prior written permission of MediaTek inc. and/or its licensors,

* any reproduction, modification, use or disclosure of MediaTek Software,

* and information contained herein, in whole or in part, shall be strictly prohibited.

* 

* Copyright  (C) 2019  MediaTek Inc. All rights reserved.

* 

* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES

* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")

* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON

* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,

* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF

* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.

* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE

* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR

* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH

* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES

* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSESss

* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK

* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR

* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND

* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,

* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,

* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO

* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.

* The following software/firmware and/or related documentation ("MediaTek Software")

* have been modified by MediaTek Inc. All revisions are subject to any receiver's

* applicable license agreements with MediaTek Inc.

*/


#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdarg.h>
#include "utils_xfrm.h"
#define LOG_TAG "ipsec_policy_mont"
#include <log/log.h>
#include <string>
#include <list>
#include "netutils_wrapper_interface.h"
#include "netlink_route.h"
#include "../../../../system/netdagent/include/forkexecwrap/fork_exec_wrap.h"
#include "ifcutils/ifc.h"

const char * const IPTABLES_PATH = "/system/bin/iptables-wrapper-1.0";
const char * const IP6TABLES_PATH = "/system/bin/ip6tables-wrapper-1.0";
const char * const IP_PATH = "/system/bin/ip-wrapper-1.0";
const char * const NDC_PATH = "/system/bin/ndc-wrapper-1.0";

extern "C" char* strchr(const char* p, int ch);

static void logExecError_xfrm(const char* argv[], int res_xfrm, int status_xfrm) {
    const char** argp = argv;
    std::string args = "";
    while (*argp) {
        args += *argp;
        args += ' ';
        argp++;
    }
    ALOGE("exec() res=%d, status=%d for %s", res_xfrm, status_xfrm, args.c_str());
}

static int execCommand_xfrm(int argc, const char *argv[], bool silent_xfrm) {
    int res_xfrm;
    int status_xfrm;

    res_xfrm = android_fork_execvp_ext(argc, (char **)argv, &status_xfrm, false, LOG_ALOG,!silent_xfrm,NULL,NULL,0);
    if (res_xfrm || !WIFEXITED(status_xfrm) || WEXITSTATUS(status_xfrm)) {
        if (!silent_xfrm) {
            logExecError_xfrm(argv, res_xfrm, status_xfrm);
        }
        if (res_xfrm)
            return res_xfrm;
        if (!WIFEXITED(status_xfrm))
            return ECHILD;
    }
    return res_xfrm;
}

static int execIptables(IptablesTarget target, bool silent_xfrm, va_list args) {
    /* Read arguments from incoming va_list; we expect the list to be NULL terminated. */
    std::list<const char*> argsList;
    argsList.push_back(NULL);
    const char* arg;

    // Wait to avoid failure due to another process holding the lock
    argsList.push_back("-w");

    do {
        arg = va_arg(args, const char *);
        argsList.push_back(arg);
    } while (arg);

    int i_xfrm = 0;
    const char* argv[argsList.size()];
    std::list<const char*>::iterator it_xfrm;
    for (it_xfrm = argsList.begin(); it_xfrm != argsList.end(); it_xfrm++, i_xfrm++) {
        argv[i_xfrm] = *it_xfrm;
    }

    const char** temp_xfrm = argv + 1; //skip argv[0]
    std::string debug = "";
    while (*temp_xfrm) {
        debug += *temp_xfrm;
        debug += " ";
        temp_xfrm++;
    }
    ALOGI("execIptables %s\n", debug.c_str());

    int res_xfrm = 0;
    if (target == V4 || target == V4V6) {
        argv[0] = IPTABLES_PATH;
        res_xfrm |= execCommand_xfrm(argsList.size(), argv, silent_xfrm);
    }
    if (target == V6 || target == V4V6) {
        argv[0] = IP6TABLES_PATH;
        res_xfrm |= execCommand_xfrm(argsList.size(), argv, silent_xfrm);
    }
    return res_xfrm;
}

int execIptables(IptablesTarget target, ...) {
    va_list args;
    va_start(args, target);
    int res = execIptables(target, false, args);
    va_end(args);
    return res;
}

static int execNdcCmd(const char *command_xfrm, bool silent_xfrm, va_list args) {
    /* Read arguments from incoming va_list; we expect the list to be NULL terminated. */
    std::list<const char*> argsList;
    argsList.push_back(NULL);
    const char* arg;
    argsList.push_back(command_xfrm);
    do {
        arg = va_arg(args, const char *);
        argsList.push_back(arg);
    } while (arg);

    int i_xfrm = 0;
    const char* argv[argsList.size()];
    std::list<const char*>::iterator it_xfrm;
    for (it_xfrm = argsList.begin(); it_xfrm != argsList.end(); it_xfrm++, i_xfrm++) {
        argv[i_xfrm] = *it_xfrm;
    }

    const char** temp_xfrm = argv + 1; //skip argv[0]
    std::string debug = "";
    while (*temp_xfrm) {
        debug += *temp_xfrm;
        debug += " ";
        temp_xfrm++;
    }
    ALOGI("execNdcCmd %s\n", debug.c_str());

    int res_xfrm = 0;
    argv[0] = NDC_PATH;
    res_xfrm = execCommand_xfrm(argsList.size(), argv, silent_xfrm);
    return res_xfrm;
  
}

int execNdcCmd(const char *command, ...) {
    va_list args;
    va_start(args, command);
    int res = execNdcCmd(command, false, args);
    va_end(args);
    return res;
}

static int execIpCmd(int family_xfrm, bool silent_xfrm, va_list args) {
    /* Read arguments from incoming va_list; we expect the list to be NULL terminated. */
    std::list<const char*> argsList;
    argsList.push_back(NULL);
    argsList.push_back(NULL);
    const char* arg_xfrm;

    do {
        arg_xfrm = va_arg(args, const char *);
        argsList.push_back(arg_xfrm);
    } while (arg_xfrm);

    int i_xfrm = 0;
    const char* argv[argsList.size()];
    std::list<const char*>::iterator it_xfrm;
    for (it_xfrm = argsList.begin(); it_xfrm != argsList.end(); it_xfrm++, i_xfrm++) {
        argv[i_xfrm] = *it_xfrm;
    }


    const char** temp_xfrm = argv + 2; //skip argv[0] and argv[1]
    std::string debug = "";
    while (*temp_xfrm) {
        debug += *temp_xfrm;
        debug += " ";
        temp_xfrm++;
    }
    ALOGI("execIpCmd %s\n", debug.c_str());

    int res_xfrm = 0;
    argv[0] = IP_PATH;
    if (family_xfrm == AF_INET) {
        argv[1] = "-4";
        res_xfrm |= execCommand_xfrm(argsList.size(), argv, silent_xfrm);
    }
    if (family_xfrm == AF_INET6) {
        argv[1] = "-6";
        res_xfrm |= execCommand_xfrm(argsList.size(), argv, silent_xfrm);
    }
    return res_xfrm;
  
}

int execIpCmd(int family_xfrm, ...) {
    va_list args;
    va_start(args, family_xfrm);
    int res_xfrm = execIpCmd(family_xfrm, false, args);
    va_end(args);
    return res_xfrm;
}

const char* IptablesInterface::LOCAL_FILTER_INPUT = "fw_INPUT";  //AOSP chain
const char* IptablesInterface::LOCAL_FILTER_OUT = "fw_OUTPUT";  //AOSP chain
const char* IptablesInterface::LOCAL_MANGLE_PREROUTING = "PREROUTING";  //AOSP chain
const char* IptablesInterface::LOCAL_FILTER_FORWARD = "oem_fwd";  //AOSP chain
const char* IptablesInterface::LOCAL_HAPPY_BOX = "bw_happy_box";  //AOSP chain

int IptablesInterface::start()
{
	pthread_t thread;
	if(pthread_create(&thread, NULL, IptablesInterface::threadStart, this)) {
		ALOGE("pthread create failed(%s)", strerror(errno));
		ifc_set_txq_state(mInIface.c_str(),0x1);
		return -1;
	}
	pthread_detach(thread);
	return 0;
}

IptablesInterface::IptablesInterface(const char* inIface, const char* outIface, const char* nxthop,
                                     const char* tableId, const char* IMSinterfaceIP,
                                     const char* epdgTunnel, int family, int refCnt,
                                     IptablesAction action)
                 : mInIface(inIface), mOutIface(outIface), mNxthop(nxthop), mTableId(tableId),
                   mIMSinterfaceIP(IMSinterfaceIP), mEpdgTunnel(epdgTunnel), mFamily(family),
                   mRefCnt(refCnt), mAction(action) {
}

IptablesInterface::~IptablesInterface() {
}

void* IptablesInterface::threadStart(void* obj)
{
    IptablesInterface* handler = reinterpret_cast<IptablesInterface*>(obj);
    handler->run();
    delete handler;
    pthread_exit(NULL);
    return NULL;
}

void IptablesInterface::run()
{
    switch (mAction) {
        case IptablesAction::IPTABLES_ON:
            if(enableIptables())
                ALOGE("enable iptables failed from %s to %s\n", mInIface.c_str(),
                      mOutIface.c_str());
            else
                ALOGI("enable iptables successfully from %s to %s\n", mInIface.c_str(),
                      mOutIface.c_str());
            break;
        case IptablesAction::IPTABLES_OFF:
            if(disableIptables())
                ALOGE("disable iptables failed from %s to %s\n", mInIface.c_str(),
                      mOutIface.c_str());
            else
                ALOGE("disable iptables successfully from %s to %s\n", mInIface.c_str(),
                      mOutIface.c_str());
            break;
        case IptablesAction::IPTABLES_RESET:
	    {
		int ret = disableIptables();
		ret |= enableIptables();
                if(ret)
                    ALOGE("reset iptables failed from %s to %s\n", mInIface.c_str(), mOutIface.c_str());
                else
                    ALOGE("reset iptables successfully from %s to %s\n", mInIface.c_str(), mOutIface.c_str());
	    }
        default:
            break;
    }
}

int IptablesInterface::enableIptables()
{
    int res = 0;
    const char *FORWARD_MARK = "0x10000";
    uint32_t table = XfrmGetRouteTableForInterface(mOutIface.c_str());
    if (table == RT_TABLE_UNSPEC) {
        ALOGE("enableIptables: table == RT_TABLE_UNSPEC");
        return -ESRCH;
    }
    //enable forwarding
    res |= execNdcCmd("ipfwd", "enable", mInIface.c_str(), NULL);
    //add forward rule
    res |= execIpCmd(mFamily, "rule", "add", "to", mNxthop.c_str(), "iif",  mInIface.c_str(),
                         "fwmark", "0x0/0xffff", "lookup", mTableId.c_str(), "prio", "25000", NULL);

    //add forward route
    res |= ModifyIpSecMonRoute(RTM_NEWROUTE, table, mOutIface.c_str(), mNxthop.c_str());

    //add rorward mark
    res |= execIptables(V4V6, "-t", "mangle", "-I", LOCAL_MANGLE_PREROUTING, "-i", mInIface.c_str(),
                            "-j", "MARK", "--set-mark", FORWARD_MARK, NULL);
    //add forward exception iptables
    res |= execIptables(V4V6, "-t", "filter", "-I", LOCAL_FILTER_FORWARD, "-i", mInIface.c_str(),
                        "-o", mOutIface.c_str(), "-j", "ACCEPT", NULL);
    // start tx queue
    ifc_set_txq_state(mInIface.c_str(),0x1);
    //add datasaver output exception iptables
    res |= execIptables(strchr(mIMSinterfaceIP.c_str(),':')?V6:V4, "-t", "filter", "-I",
                        LOCAL_HAPPY_BOX, "-o", mOutIface.c_str(), "-s", mIMSinterfaceIP.c_str(),
                        "-j", "RETURN", NULL);
    res |= execIptables(strchr(mEpdgTunnel.c_str(),':')?V6:V4, "-t", "filter", "-I",
                        LOCAL_HAPPY_BOX, "-o", mOutIface.c_str(), "-d", mEpdgTunnel.c_str(), "-j",
                        "RETURN", NULL);
    //add datasaver input exception iptables
    res |= execIptables(strchr(mIMSinterfaceIP.c_str(),':')?V6:V4, "-t", "filter", "-I",
                        LOCAL_HAPPY_BOX, "-i", mOutIface.c_str(), "-d", mIMSinterfaceIP.c_str(),
                        "-j", "RETURN", NULL);
    res |= execIptables(strchr(mEpdgTunnel.c_str(),':')?V6:V4, "-t", "filter", "-I",
                        LOCAL_HAPPY_BOX, "-i", mOutIface.c_str(), "-s", mEpdgTunnel.c_str(), "-j",
                        "RETURN", NULL);

    return res;
}

int IptablesInterface::disableIptables()
{
    int res = 0;
    const char *FORWARD_MARK = "0x10000";
    uint32_t table = XfrmGetRouteTableForInterface(mOutIface.c_str());
    if (table == RT_TABLE_UNSPEC) {
        ALOGE("enableIptables: table == RT_TABLE_UNSPEC");
        res |= -ESRCH;
    }
    XfrmEarseInterfaceFromTable(mOutIface.c_str());

    //disable forwarding
    res |= execNdcCmd("ipfwd", "disable", mInIface.c_str(), NULL);
    //del forward mark
    res |= execIptables(V4V6, "-t", "mangle", "-D", LOCAL_MANGLE_PREROUTING, "-i", mInIface.c_str(),
                        "-j", "MARK", "--set-mark", FORWARD_MARK, NULL);
    //del forward exception iptables
    res |= execIptables(V4V6, "-t", "filter", "-D", LOCAL_FILTER_FORWARD, "-i", mInIface.c_str(),
                        "-o", mOutIface.c_str(), "-j", "ACCEPT", NULL);
    //del datasaver output exception iptables
    res |= execIptables(strchr(mIMSinterfaceIP.c_str(),':')?V6:V4, "-t", "filter", "-D",
                        LOCAL_HAPPY_BOX, "-o", mOutIface.c_str(), "-s", mIMSinterfaceIP.c_str(),
                        "-j", "RETURN", NULL);
    res |= execIptables(strchr(mEpdgTunnel.c_str(),':')?V6:V4, "-t", "filter", "-D", LOCAL_HAPPY_BOX,
                        "-o", mOutIface.c_str(), "-d", mEpdgTunnel.c_str(), "-j", "RETURN", NULL);
    //del datasaver input exception iptables
    res |= execIptables(strchr(mIMSinterfaceIP.c_str(),':')?V6:V4, "-t", "filter", "-D",
                        LOCAL_HAPPY_BOX, "-i", mOutIface.c_str(), "-d", mIMSinterfaceIP.c_str(),
                        "-j", "RETURN", NULL);
    res |= execIptables(strchr(mEpdgTunnel.c_str(),':')?V6:V4, "-t", "filter", "-D", LOCAL_HAPPY_BOX,
                        "-i", mOutIface.c_str(), "-s", mEpdgTunnel.c_str(), "-j", "RETURN", NULL);
    if (!mRefCnt) {
        //del/forward rule
        res |= execIpCmd(mFamily, "rule", "del", "to", mNxthop.c_str(), "iif",  mInIface.c_str(), "fwmark",
                        "0x0/0xffff", "lookup", mTableId.c_str(), "prio", "25000", NULL);
        //del forward route
        res |= ModifyIpSecMonRoute(RTM_DELROUTE, table, mOutIface.c_str(), mNxthop.c_str());
    }
    return res;
}
