/*
*  Copyright (C) 2014 MediaTek Inc.
*
*  Modification based on code covered by the below mentioned copyright
*  and/or permission notice(s).
*/

/* //device/system/rild/rild.c
**
** Copyright 2014 The Android Open Source Project
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
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <telephony/mtk_ril.h>

#undef LOG_TAG
#define LOG_TAG "RILD"

#include <log/log.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <sys/capability.h>
#include <linux/prctl.h>
#include <librilmtk/ril_ex.h>
#include <libmtkrilutils.h>
#include <private/android_filesystem_config.h>

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif

#define SOC_PATH_PROPERTY  "rild.modem.socket"

#define LIB_PATH_PROPERTY   "rild.libpath"
#define LIB_ARGS_PROPERTY   "rild.libargs"

#define MAX_LIB_ARGS        16

static void usage(const char *argv0) {
    fprintf(stderr, "Usage: %s -l <ril impl library> [-- <args for impl library>]\n", argv0);
    exit(EXIT_FAILURE);
}

extern char rild[MAX_SERVICE_NAME_LENGTH];

extern void RIL_register (const RIL_RadioFunctions *callbacks);
extern void RIL_register_socket (RIL_RadioFunctions *(*rilUimInit)
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

#ifdef MTK_RIL
extern void RIL_requestProxyTimedCallback (RIL_TimedCallback callback,
                               void *param, const struct timeval *relativeTime, int proxyId);
extern RILChannelId RIL_queryMyChannelId(RIL_Token t);
extern RILChannelId RIL_queryCommandChannelId(RIL_Token t);
extern int RIL_queryMyProxyIdByThread();
#endif

static struct RIL_Env s_rilEnv = {
    RIL_onRequestComplete,
    RIL_onUnsolicitedResponse,
    RIL_requestTimedCallback,
    RIL_onRequestAck
#ifdef MTK_RIL
    ,RIL_requestProxyTimedCallback
    ,RIL_queryMyChannelId
    ,RIL_queryCommandChannelId
    ,RIL_queryMyProxyIdByThread
#endif
};

extern void RIL_startEventLoop();
int mtkInit();

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

int main(int argc, char **argv) {
    // vendor ril lib path either passed in as -l parameter, or read from rild.libpath property
    const char *rilLibPath = NULL;
    // ril arguments either passed in as -- parameter, or read from rild.libargs property
    char **rilArgv;
    // handle for vendor ril lib
    void *dlHandle;
    // Pointer to ril init function in vendor ril
    const RIL_RadioFunctions *(*rilInit)(const struct RIL_Env *, int, char **);
    // Pointer to sap init function in vendor ril
    RIL_RadioFunctions *(*rilUimInit)(const struct RIL_Env *, int, char **);
    const char *err_str = NULL;

    // functions returned by ril init function in vendor ril
    const RIL_RadioFunctions *funcs;
    // lib path from rild.libpath property (if it's read)
    char libPath[PROPERTY_VALUE_MAX];
    // flat to indicate if -- parameters are present
    unsigned char hasLibArgs = 0;

    int i;
    // ril/socket id received as -c parameter, otherwise set to 0
    const char *clientId = NULL;

    RLOGD("**RIL Daemon Started**");
    RLOGD("**RILd param count=%d**", argc);

    if (mtkInit() == -1) goto done;

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

    if (rilLibPath == NULL) {
        if ( 0 == property_get(LIB_PATH_PROPERTY, libPath, "mtk-ril.so")) {
            // No lib sepcified on the command line, and nothing set in props.
            // Assume "no-ril" case.
            goto done;
        } else {
            rilLibPath = libPath;
        }
    }

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
        (RIL_RadioFunctions *(*)(const struct RIL_Env *, int, char **))
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
    } else {
        static char * newArgv[MAX_LIB_ARGS];
        static char args[PROPERTY_VALUE_MAX];
        rilArgv = newArgv;
        property_get(LIB_ARGS_PROPERTY, args, "-d /dev/ttyC0");
        argc = make_argv(args, rilArgv);
    }

    rilArgv[argc++] = "-c";
    rilArgv[argc++] = (char*)clientId;
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

/*
* Purpose:  Function responsible by all signal handlers treatment any new signal must be added here
* Input:      param - signal ID
* Return:    -
*/
void signal_treatment(int param)
{
    RLOGD("signal_no=%d", param);
    switch (param)
    {
    case SIGUSR1:
        RLOGD("SIGUSR1");
        break;
    case SIGUSR2:
        RLOGD("SIGUSR2");
        break;
    case SIGSEGV:
    case SIGABRT:
    case SIGPIPE:
    case SIGILL:
    case SIGBUS:
    case SIGFPE:
        RLOGD("trigger TRM");
    #ifdef HAVE_AEE_FEATURE
        char prop_value[PROPERTY_VALUE_MAX] = { 0 };
        property_get("persist.log.tag.tel_dbg", prop_value, "0");
        if(prop_value[0] == '1') {
            aee_system_exception("mtkrild", NULL, DB_OPT_DEFAULT, "rild NE");
        }
    #endif
        property_set("vendor.ril.mux.report.case","2");
        property_set("vendor.ril.cdma.report.case", "1");
        property_set("vendor.ril.muxreport", "1");
        break;
    default:
        exit(0);
        break;
    }
}

int mtkInit()
{
    char buildType[PROPERTY_VALUE_MAX] = {0};
    char rild_pid[PROPERTY_VALUE_MAX] = {0}; //store the pid for CCCI, majorly for modem state change

    //signals treatment
    signal(SIGUSR1, signal_treatment);
    signal(SIGUSR2, signal_treatment); // for MD state change

    if ((isInternalLoad() == 0) && (isUserLoad() == 1)) {
        RLOGD("Setup signal handling");
        signal(SIGSEGV, signal_treatment);
        signal(SIGABRT, signal_treatment);
        signal(SIGILL, signal_treatment);
        signal(SIGPIPE, signal_treatment);
        signal(SIGBUS, signal_treatment);
        signal(SIGFPE, signal_treatment);
    }

    snprintf(rild_pid, sizeof(rild_pid), "%d", getpid()); //get pid
    property_set("vendor.ril.pid.1", rild_pid);
    return 0;
}

