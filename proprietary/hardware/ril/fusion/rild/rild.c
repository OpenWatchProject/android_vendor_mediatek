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
#define LOG_TAG "RILD"
#include <cutils/sockets.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <librilmtk/ril_ex.h>
#include "sysenv_utils.h"
#include <cutils/android_filesystem_config.h>
#include <mtk_log.h>
#include <mtk_properties.h>
#include <mtkconfigutils.h>

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif

#define LIB_PATH_PROPERTY   "vendor.rild.libpath"
#define LIB_ARGS_PROPERTY   "vendor.rild.libargs"
#define MAX_LIB_ARGS        16

static void usage(const char *argv0) {
    fprintf(stderr, "Usage: %s -l <ril impl library> [-- <args for impl library>]\n", argv0);
    exit(EXIT_FAILURE);
}

extern char ril_service_name_base[MAX_SERVICE_NAME_LENGTH];
extern char ril_service_name[MAX_SERVICE_NAME_LENGTH];

extern void RIL_register (const RIL_RadioFunctions *callbacks);
extern void rilc_thread_pool ();

extern void RIL_register_socket (const RIL_RadioFunctions *(*rilUimInit)
        (const struct RIL_Env *, int, char **), RIL_SOCKET_TYPE socketType, int argc, char **argv);

extern void RIL_onRequestComplete(RIL_Token t, RIL_Errno e,
        void *response, size_t responselen);

extern void RIL_onRequestAck(RIL_Token t);

#if defined(ANDROID_MULTI_SIM)
extern void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
        size_t datalen, RIL_SOCKET_ID socket_id);
#else
extern void RIL_onUnsolicitedResponse(int unsolResponse, const void *data,
        size_t datalen);
#endif

extern void RIL_requestTimedCallback (RIL_TimedCallback callback,
        void *param, const struct timeval *relativeTime);


static struct RIL_Env s_rilEnv = {
    RIL_onRequestComplete,
    RIL_onUnsolicitedResponse,
    RIL_requestTimedCallback,
    RIL_onRequestAck,
    NULL,
    NULL,
    NULL
};

extern void RIL_startEventLoop();

static int isReseted = 0;

int isUserLoad() {
    int isUserLoad = 0;
    char property_value_emulation[MTK_PROPERTY_VALUE_MAX] = { 0 };
    char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    mtk_property_get("vendor.ril.emulation.userload", property_value_emulation, "0");
    if(strcmp("1", property_value_emulation) == 0) {
        return 1;
    }
    mtk_property_get("ro.build.type", property_value, "");
    isUserLoad = (strcmp("user", property_value) == 0);
    //mtkLogD(LOG_TAG, "isUserLoad: %d", isUserLoad);
    return isUserLoad;
}

int isInternalLoad() {
    #ifdef __PRODUCTION_RELEASE__
        return 0;
    #else
        char property_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
        mtk_property_get("vendor.ril.emulation.production", property_value, "0");
        return (strcmp("1", property_value) != 0);
    #endif /* __PRODUCTION_RELEASE__ */
}

/*
* Purpose:  Function responsible by all signal handlers treatment any new signal must be added here
* Input:      param - signal ID
* Return:    -
*/
void signal_treatment(int param)
{
    mtkLogD(LOG_TAG, "signal_no=%d", param);
    switch (param) {
        case SIGUSR1:
            mtkLogD(LOG_TAG, "SIGUSR1");
            break;
        case SIGUSR2:
            mtkLogD(LOG_TAG, "SIGUSR2");
            break;
        case SIGSEGV:
        case SIGABRT:
        case SIGPIPE:
        case SIGILL:
        case SIGBUS:
        case SIGFPE:
            if (!isReseted) {
                mtkLogD(LOG_TAG, "trigger TRM");
            #ifdef HAVE_AEE_FEATURE
                char prop_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
                mtk_property_get("persist.log.tag.tel_dbg", prop_value, "0");
                if(prop_value[0] == '1') {
                    aee_system_exception("mtkrild", NULL, DB_OPT_DEFAULT, "fusion rild NE");
                }
            #endif
                mtk_property_set("vendor.ril.mux.report.case","2");
                mtk_property_set("vendor.ril.muxreport", "1");
                isReseted = 1;
            } else {
                mtkLogD(LOG_TAG, "already reset");
            }
            break;
        default:
            exit(2);
            break;
    }
}

void mtkInit() {
    //signals treatment
    signal(SIGUSR1, signal_treatment);
    signal(SIGUSR2, signal_treatment);

    if ((isInternalLoad() == 0) && (isUserLoad() == 1)) {
        mtkLogD(LOG_TAG, "Setup SIGSEGV signal handling");
        signal(SIGSEGV, signal_treatment);
        signal(SIGABRT, signal_treatment);
        signal(SIGILL, signal_treatment);
        signal(SIGPIPE, signal_treatment);
        signal(SIGBUS, signal_treatment);
        signal(SIGFPE, signal_treatment);
    }
}

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

void setDynamicMsimConfig() {
    char prop_value[MTK_PROPERTY_VALUE_MAX] = { 0 };
    // update dynamic multisim config
/*
#ifdef MTK_SYSENV_SUPPORT
    const char *pSysenv = sysenv_get_static("msim_config");
#else
    const char *pSysenv = NULL;
#endif
*/
    // sync persist.radio.multisim.config and persist.vendor.radio.msimmode
    FeatureValue featureValueOld = {0};
    FeatureValue featureValueNew = {0};
    mtkGetFeature(CONFIG_SIM_MODE, &featureValueOld);
    mtk_property_set("persist.vendor.radio.msimmode", featureValueOld.value);
/*
    mtkLogD(LOG_TAG, "sysenv_get_static: %s\n", pSysenv);
    if (pSysenv != NULL) {
        // for META tool
        if (strcmp(pSysenv, "dsds") == 0) {
            strncpy(featureValueNew.value, "dsds", 10);
            mtkSetFeature(CONFIG_SIM_MODE, &featureValueNew);
            mtk_property_set("persist.vendor.radio.msimmode", "dsds");
            mtk_property_set("persist.vendor.radio.multisimslot", "2");
        } else if (strcmp(pSysenv, "ss") == 0) {
            strncpy(featureValueNew.value, "ss", 10);
            mtkSetFeature(CONFIG_SIM_MODE, &featureValueNew);
            mtk_property_set("persist.vendor.radio.msimmode", "ss");
            mtk_property_set("persist.vendor.radio.multisimslot", "1");
        }
    } else
*/
    {
        // for EM
        if (strcmp(featureValueOld.value, "dsda") != 0) {
            // get ccci setting first, if not set then get EM setting.
            mtk_property_get("ro.boot.opt_sim_count", prop_value, "0");
            if (prop_value[0] == '0') {
                mtk_property_get("persist.vendor.radio.multisimslot", prop_value, "0");
            }
            if (prop_value[0] == '1') {
                strncpy(featureValueNew.value, "ss", 10);
                mtkSetFeature(CONFIG_SIM_MODE, &featureValueNew);
                mtk_property_set("persist.vendor.radio.msimmode", "ss");
            } else if (prop_value[0] == '2') {
                strncpy(featureValueNew.value, "dsds", 10);
                mtkSetFeature(CONFIG_SIM_MODE, &featureValueNew);
                mtk_property_set("persist.vendor.radio.msimmode", "dsds");
            } else if (prop_value[0] == '3') {
                strncpy(featureValueNew.value, "tsts", 10);
                mtkSetFeature(CONFIG_SIM_MODE, &featureValueNew);
                mtk_property_set("persist.vendor.radio.msimmode", "tsts");
            } else if (prop_value[0] == '4') {
                strncpy(featureValueNew.value, "qsqs", 10);
                mtkSetFeature(CONFIG_SIM_MODE, &featureValueNew);
                mtk_property_set("persist.vendor.radio.msimmode", "qsqs");
            }
        }
    }
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
    const RIL_RadioFunctions *(*rilUimInit)(const struct RIL_Env *, int, char **);
    const char *err_str = NULL;

    // functions returned by ril init function in vendor ril
    const RIL_RadioFunctions *funcs;
    // lib path from rild.libpath property (if it's read)
    char libPath[MTK_PROPERTY_VALUE_MAX];
    // flat to indicate if -- parameters are present
    unsigned char hasLibArgs = 0;

    int i;
    // ril/socket id received as -c parameter, otherwise set to 0
    const char *clientId = NULL;

    mtkLogD(LOG_TAG, "**RIL Proxy Started**");
    mtkLogD(LOG_TAG, "**RILd param count=%d**", argc);
    // check if rild is started by muxd
    mtk_property_get("vendor.ril.mux.start", libPath, "0");
    if(libPath[0] == '0') {
        mtkLogE(LOG_TAG, "RILD is not started by muxd, TRM");
        mtk_property_set("vendor.ril.disable.eboot","1");
        mtk_property_set("vendor.ril.mux.report.case","2");
        mtk_property_set("vendor.ril.muxreport", "1");
        exit(0);
    }
    mtk_property_set("vendor.ril.mux.start", "0");
    setDynamicMsimConfig();
    mtkInit();

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
        mtkLogE(LOG_TAG, "Max Number of rild's supported is: %d", MAX_RILDS);
        exit(0);
    }
    if (strncmp(clientId, "0", MAX_CLIENT_ID_LENGTH)) {
        snprintf(ril_service_name, sizeof(ril_service_name), "%s%s", ril_service_name_base,
                 clientId);
    }

    if (rilLibPath == NULL) {
        if ( 0 == mtk_property_get(LIB_PATH_PROPERTY, libPath, NULL)) {
            // No lib sepcified on the command line, and nothing set in props.
            // Assume "no-ril" case.
            goto done;
        } else {
            rilLibPath = libPath;
        }
    }
    mtkLogD(LOG_TAG, "rilLibPath:%s", rilLibPath);
    /* special override when in the emulator */

    dlHandle = dlopen(rilLibPath, RTLD_NOW);

    if (dlHandle == NULL) {
        mtkLogE(LOG_TAG, "dlopen failed: %s", dlerror());
        exit(EXIT_FAILURE);
    }

    RIL_startEventLoop();

    rilInit =
        (const RIL_RadioFunctions *(*)(const struct RIL_Env *, int, char **))
        dlsym(dlHandle, "RIL_Init");

    if (rilInit == NULL) {
        mtkLogE(LOG_TAG, "RIL_Init not defined or exported in %s\n", rilLibPath);
        exit(EXIT_FAILURE);
    }

    dlerror(); // Clear any previous dlerror
    rilUimInit =
        (const RIL_RadioFunctions *(*)(const struct RIL_Env *, int, char **))
        dlsym(dlHandle, "RIL_SAP_Init");
    err_str = dlerror();
    if (err_str) {
        mtkLogW(LOG_TAG, "RIL_SAP_Init not defined or exported in %s: %s\n", rilLibPath, err_str);
    } else if (!rilUimInit) {
        mtkLogW(LOG_TAG, "RIL_SAP_Init defined as null in %s. SAP Not usable\n", rilLibPath);
    }

    if (hasLibArgs) {
        rilArgv = argv + i - 1;
        argc = argc -i + 1;
    } else {
        static char * newArgv[MAX_LIB_ARGS];
        static char args[MTK_PROPERTY_VALUE_MAX];
        rilArgv = newArgv;
        mtk_property_get(LIB_ARGS_PROPERTY, args, "");
        argc = make_argv(args, rilArgv);
    }

    rilArgv[argc++] = "-c";
    rilArgv[argc++] = (char*)clientId;
    mtkLogD(LOG_TAG, "RIL_Init argc = %d clientId = %s", argc, rilArgv[argc-1]);

    // Make sure there's a reasonable argv[0]
    rilArgv[0] = argv[0];

    funcs = rilInit(&s_rilEnv, argc, rilArgv);
    mtkLogD(LOG_TAG, "RIL_Init rilInit completed");

    RIL_register(funcs);

    mtkLogD(LOG_TAG, "RIL_Init RIL_register completed");

    if (rilUimInit) {
        mtkLogD(LOG_TAG, "RIL_register_socket started");
        RIL_register_socket(rilUimInit, RIL_SAP_SOCKET, argc, rilArgv);
    }

    mtkLogD(LOG_TAG, "RIL_register_socket completed");

done:

    rilc_thread_pool();

    mtkLogD(LOG_TAG, "RIL_Init starting sleep loop");
    while (true) {
        sleep(UINT32_MAX);
    }
}
