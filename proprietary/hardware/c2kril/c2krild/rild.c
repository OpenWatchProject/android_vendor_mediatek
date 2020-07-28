/* //device/system/rild/rild.c
**
** Copyright 2006 The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <telephony/mtk_ril.h>
#undef LOG_TAG
#define LOG_TAG "C2K_RILD"
#include <utils/Log.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <librilmtk/ril_ex.h>

#include <private/android_filesystem_config.h>
#include "hardware/ccci_intf.h"
#include <libmtkrilutils.h>
#ifdef C2K_RIL_LEGACY
#include <c2kutils.h>
#endif

#define MAX_LIB_ARGS        16
#define MAX_CAP_NUM         (CAP_TO_INDEX(CAP_LAST_CAP) + 1)

static void usage(const char *argv0) {
    fprintf(stderr, "Usage: %s -l <ril impl library> [-- <args for impl library>]\n", argv0);
    exit(EXIT_FAILURE);
}

extern char rild[MAX_SERVICE_NAME_LENGTH];

extern void RIL_register (const RIL_RadioFunctions *callbacks);

extern void RIL_register_socket (const RIL_RadioFunctions *(*rilUimInit)
        (const struct RIL_Env *, int, char **), RIL_SOCKET_TYPE socketType, int argc, char **argv);

extern void RIL_onRequestComplete(RIL_Token t, RIL_Errno e,
        void *response, size_t responselen);

extern void RIL_onRequestAck(RIL_Token t);

extern void RIL_setRilSocketName(char *);

#if defined(ANDROID_MULTI_SIM)
extern void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
        size_t datalen, RIL_SOCKET_ID socket_id);
#else
extern void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
        size_t datalen);
#endif

extern void RIL_requestTimedCallback (RIL_TimedCallback callback,
        void *param, const struct timeval *relativeTime);

// For multi channel support
extern void RIL_requestProxyTimedCallback (RIL_TimedCallback callback,
                               void *param, const struct timeval *relativeTime, int proxyId);
extern RILChannelId RIL_queryMyChannelId(RIL_Token t);
extern int RIL_queryMyProxyIdByThread();

static struct RIL_Env s_rilEnv = {
    RIL_onRequestComplete,
    RIL_onUnsolicitedResponse,
    RIL_requestTimedCallback,
    RIL_onRequestAck
    // For multi channel support
    ,RIL_requestProxyTimedCallback
    ,RIL_queryMyChannelId
    ,RIL_queryMyProxyIdByThread
};

extern void RIL_startEventLoop();
void add_sig_handler();

static int make_argv(char * args, char ** argv) {
    // Note: reserve argv[0]
    int count = 1;
    char * tok;
    char * s = args;

    while ((tok = strtok(s, " \0"))) {
        argv[count] = tok;
        s = NULL;
        count++;
    }
    return count;
}

int isEccciSupport() {
    int isEccciSupport = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.mtk_eccci_c2k", property_value, "0");
    isEccciSupport = atoi(property_value);
    return (isEccciSupport == 1);
}

/*
 * switchUser - Switches UID to radio, preserving CAP_NET_ADMIN capabilities.
 * Our group, cache, was set by init.
 */
void switchUser() {
    char debuggable[PROP_VALUE_MAX];

    prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
    if (setresuid(AID_RADIO, AID_RADIO, AID_RADIO) == -1) {
        RLOGE("setresuid failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct __user_cap_header_struct header;
    memset(&header, 0, sizeof(header));
    header.version = _LINUX_CAPABILITY_VERSION_3;
    header.pid = 0;

    struct __user_cap_data_struct data[MAX_CAP_NUM];
    memset(&data, 0, sizeof(data));

    data[CAP_TO_INDEX(CAP_NET_ADMIN)].effective |= CAP_TO_MASK(CAP_NET_ADMIN);
    data[CAP_TO_INDEX(CAP_NET_ADMIN)].permitted |= CAP_TO_MASK(CAP_NET_ADMIN);

    data[CAP_TO_INDEX(CAP_NET_RAW)].effective |= CAP_TO_MASK(CAP_NET_RAW);
    data[CAP_TO_INDEX(CAP_NET_RAW)].permitted |= CAP_TO_MASK(CAP_NET_RAW);

    data[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].effective |= CAP_TO_MASK(CAP_BLOCK_SUSPEND);
    data[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].permitted |= CAP_TO_MASK(CAP_BLOCK_SUSPEND);

    /// M: In Denali C2K project, viarild is lunched by Statusd and didnot have root permission,
    /// so capset may fail. In this case we will ignore the fail.
    if (isEccciSupport() && capset(&header, &data[0]) == -1) {
        RLOGE("capset failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*
     * Debuggable build only:
     * Set DUMPABLE that was cleared by setuid() to have tombstone on RIL crash
     */
    property_get("ro.debuggable", debuggable, "0");
    if (strcmp(debuggable, "1") == 0) {
        prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
    }
}

int main(int argc, char **argv) {
    const char * rilLibPath = NULL;
    char **rilArgv;
    void *dlHandle;
    const RIL_RadioFunctions *(*rilInit)(const struct RIL_Env *, int, char **);
    const RIL_RadioFunctions *(*rilUimInit)(const struct RIL_Env *, int, char **);
    const char *err_str = NULL;

    const RIL_RadioFunctions *funcs;
    char libPath[PROPERTY_VALUE_MAX];
    unsigned char hasLibArgs = 0;

    int i;
    const char *clientId = NULL;
    RLOGD("**RIL Daemon Started**");
    RLOGD("**RILd param count=%d**", argc);

    umask(S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
    for (i = 1; i < argc ;) {
        if (0 == strcmp(argv[i], "-l") && (argc - i > 1)) {
            rilLibPath = argv[i + 1];
            i += 2;
        } else if (0 == strcmp(argv[i], "--")) {
            i++;
            hasLibArgs = 1;
            break;
        } else if (0 == strcmp(argv[i], "-c") &&  (argc - i > 1)) {
            clientId = argv[i+1];
            i += 2;
        } else {
            usage(argv[0]);
        }
    }

    if (clientId == NULL) {
        clientId = "0";
    } else if (atoi(clientId) >= MAX_RILDS) {
        RLOGE("Max Number of rild's supported is: %d", MAX_RILDS);
        exit(0);
    }
    if (strncmp(clientId, "0", MAX_CLIENT_ID_LENGTH)) {
        strlcat(rild, clientId, MAX_SERVICE_NAME_LENGTH);
        RIL_setRilSocketName(rild);
    }

    RLOGD("Android rild start");
    add_sig_handler();

    if (rilLibPath == NULL) {
        // No lib sepcified on the command line, and nothing set in props.
        // Assume "no-ril" case.
        RLOGD("No vendor so");
        goto done;
    }

    switchUser();

    dlHandle = dlopen(rilLibPath, RTLD_NOW);

    if (dlHandle == NULL) {
        RLOGE("dlopen failed: %s", dlerror());
        exit(EXIT_FAILURE);
    }

    RIL_startEventLoop();

    rilInit =
        (const RIL_RadioFunctions *(*)(const struct RIL_Env *, int, char **))
        dlsym(dlHandle, "RIL_Init");

    if (rilInit == NULL) {
        RLOGE("RIL_Init not defined or exported in %s\n", rilLibPath);
        exit(EXIT_FAILURE);
    }

    dlerror(); // Clear any previous dlerror
    rilUimInit =
        (const RIL_RadioFunctions *(*)(const struct RIL_Env *, int, char **))
        dlsym(dlHandle, "RIL_SAP_Init");
    err_str = dlerror();
    if (err_str) {
        RLOGW("RIL_SAP_Init not defined or exported in %s: %s\n", rilLibPath, err_str);
    } else if (!rilUimInit) {
        RLOGW("RIL_SAP_Init defined as null in %s. SAP Not usable\n", rilLibPath);
    }

    if (hasLibArgs) {
        rilArgv = argv + i - 1;
        argc = argc -i + 1;
    }

    rilArgv[argc++] = "-c";
    rilArgv[argc++] = clientId;
    RLOGD("RIL_Init argc = %d clientId = %s", argc, rilArgv[argc-1]);

    // Make sure there's a reasonable argv[0]
    rilArgv[0] = argv[0];

    funcs = rilInit(&s_rilEnv, argc, rilArgv);
    RLOGD("RIL_Init rilInit completed");

    RIL_register(funcs);

    RLOGD("RIL_Init RIL_register completed");

    if (rilUimInit) {
        RLOGD("RIL_register_socket started");
        RIL_register_socket(rilUimInit, RIL_SAP_SOCKET, argc, rilArgv);
    }

    RLOGD("RIL_register_socket completed");

done:

    RLOGD("RIL_Init starting sleep loop");
    while (true) {
        sleep(UINT32_MAX);
    }
}

int isEngLoad() {
    int isEngLoad = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.build.type", property_value, "");
    isEngLoad = (strcmp("eng", property_value) == 0);
    RLOGI("isEngLoad: %d", isEngLoad);
    return isEngLoad;
}

void signal_handler(int param)
{
    RLOGD("signal_handler, signal_no=%d", param);
    switch (param)
    {
    RLOGD("signal_no=%d", param);
    case SIGSEGV:
    case SIGABRT:
        LOGD("C2KReset");
        property_set("vendor.ril.cdma.report.case", "1");
        property_set("vendor.ril.mux.report.case", "2");

#ifdef MTK_ECCCI_C2K
        char ret[32];
        int fd = -1;
        snprintf(ret, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS3));
        fd = open(ret, O_RDWR);
        if(fd >= 0) {
            ioctl(fd, CCCI_IOC_MD_RESET);
            close(fd);
        } else {
            LOGE("open ccci_get_node_name USR_RILD_IOCTL failed, errno: %d", errno);
        }
#elif C2K_RIL_LEGACY
        C2KReset();
#endif
        break;
    default:
        exit(0);
        break;
    }
}

void add_sig_handler() {
    RLOGD("add signal handler");
    signal(SIGPIPE, SIG_IGN);
    signal(SIGUSR2, signal_handler);
    if (!isInternalLoad() && isUserLoad()) {
        signal(SIGSEGV, signal_handler);
        signal(SIGABRT, signal_handler);
    }
}
