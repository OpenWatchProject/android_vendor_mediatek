/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define MDMI_TOOL_VERSION "v1.0"

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include "CommandCode.h"
#include "SocketConnection.h"
#include "Mdmi_utils.h"
#include "Mdmi_defs.h"

#include <android/log.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <cutils/properties.h>
#include <inttypes.h>

//#define DBG(...) do { if (gDebug) { printf(__VA_ARGS__); printf("\n"); }} while (0)

#define LOG_TAG "MDMI-MdmiRedirectorCtrl"
#define DBG(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define REDIRECTOR_COMMAND_RECEIVER_NAME "inno_mdi_redirector_cmd_receiver" // send command to mdi_redirector

int gFUNC = MDMI_CMD_CODE_UNDEFINED;
bool gDebug = false;

static struct option longopts[] = {
    {"version"              , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_VERSION},
    {"start"                , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_START},
    {"create-session"       , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_CREATE_SESSION},
    {"close-session"        , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_CLOSE_SESSION},
    {"get"                  , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_GET},
    {"set"                  , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_SET},
    {"invoke"               , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_INVOKE},
    {"subscribe"            , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_SUBSCRIBE},
    {"unsubscribe"          , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_UNSUBSCRIBE},
    {"get-session-stats"    , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_GET_SESSION_STATS},
    {"stop"                 , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_STOP},
    {"help"                 , no_argument, (int *) &gFUNC, MDMI_CMD_CODE_HELP},
    {NULL                   , 0          , NULL          , 0}
};

int GetToolVersion()
{
    printf(MDMI_TOOL_VERSION);
    return 0;
}

int Help(int ret)
{
    printf("Usage: mdmi_redirector_ctrl\n");
    printf("\t--version\n");
    printf("\t--start\n");
    printf("\t--stop\n");
    printf("\t--create-session libtype=*\n");
    printf("\t--close-session libtype=*\n");
    printf("\t--get\n");
    printf("\t--set\n");
    printf("\t--invoke\n");
    printf("\t--subscribe libtype=X OID=*.*.*\n");
    printf("\t--unsubscribe libtype=X OID=*.*.*\n");
    printf("\t--get-session-stats libtype=*\n");
    printf("\t--help\n\n");

    printf("List of available libtype value\n");
    printf("\t%d: MDMI_LIB_DEBUG\n", MDMI_LIB_DEBUG);
    printf("\t%d: MDMI_LIB_LTE\n", MDMI_LIB_LTE);
    printf("\t%d: MDMI_LIB_GSM\n", MDMI_LIB_GSM);
    printf("\t%d: MDMI_LIB_UMTS\n", MDMI_LIB_UMTS);
    printf("\t%d: MDMI_LIB_WIFI\n", MDMI_LIB_WIFI);
    printf("\t%d: MDMI_LIB_COMMANDS\n", MDMI_LIB_COMMANDS);
    printf("\t%d: MDMI_LIB_WCDMA\n", MDMI_LIB_WCDMA);
    printf("\t%d: MDMI_LIB_HSUPA\n", MDMI_LIB_HSUPA);
    printf("\t%d: MDMI_LIB_HSDPA\n", MDMI_LIB_HSDPA);
    printf("\t%d: MDMI_LIB_EMBMS\n", MDMI_LIB_EMBMS);

    return ret;
}

// Utility functions prototype
bool SendCommand(uint16_t cmdCode, int argc, char **argv);

// Operation functions prototype
static int Start(int argc, char **argv);
static int Stop(int argc, char **argv);
static int CreateSession(int argc, char **argv);
static int CloseSession(int argc, char **argv);
static int Get(int argc, char **argv);
static int Set(int argc, char **argv);
static int Invoke(int argc, char **argv);
static int Subscribe(int argc, char **argv);
static int Unsubscribe(int argc, char **argv);
static int GetSessionStats(int argc, char **argv);

static SocketConnection connection;
#define CHECK_ARGV_EQ_ATTR(attr) (0 == strncasecmp(argv[i], attr"=", sizeof(attr"=")-1))
#define GET_ARGV_EQ_VALUE() (strchr(argv[i], '=') + 1)

int main(int argc, char **argv) {
    // get command action to gFUNC
    int opt = getopt_long_only(argc, argv, "", longopts, nullptr);
    if (opt == -1) {
        DBG("main: Invalid command!");
        return Help(255);
    }

    argc -= optind;
    argv += optind;

    DBG("main: command code = %d", gFUNC);

    switch (gFUNC) {
        case MDMI_CMD_CODE_VERSION:
            return GetToolVersion();
        case MDMI_CMD_CODE_START:
            return Start(argc, argv);
        case MDMI_CMD_CODE_STOP:
            return Stop(argc, argv);
        case MDMI_CMD_CODE_CREATE_SESSION:
            return CreateSession(argc, argv);
        case MDMI_CMD_CODE_CLOSE_SESSION:
            return CloseSession(argc, argv);
        case MDMI_CMD_CODE_GET:
            return Get(argc, argv);
        case MDMI_CMD_CODE_SET:
            return Set(argc, argv);
        case MDMI_CMD_CODE_INVOKE:
            return Invoke(argc, argv);
        case MDMI_CMD_CODE_SUBSCRIBE:
            return Subscribe(argc, argv);
        case MDMI_CMD_CODE_UNSUBSCRIBE:
            return Unsubscribe(argc, argv);
        case MDMI_CMD_CODE_GET_SESSION_STATS:
            return GetSessionStats(argc, argv);
        case MDMI_CMD_CODE_HELP:
            return Help(0);
        default:
            DBG("Unknown parameters.");
            return Help(255);
    }
}

//
// Operation functions
//
static bool StartMdmiRedirectorSync() {
    int retry = -1;
    char buffer[PROP_VALUE_MAX] = {'\0'};

    for (retry = -1; retry < 3; retry++) {
        property_get("init.svc.mdmi_redirector", buffer, "0");
        DBG("init.svc.mdmi_redirector: %s", buffer);
        if (NULL == strstr(buffer, "running")) {
            DBG("Start mdmi_redirector");
            property_set("ctl.start", "mdmi_redirector");
            sleep(1);
        } else {
            DBG("Find mdmi_redirector!");
            return true;
        }
    }
    sleep(5);

    return false;
}

static void StopMdmiRedirector() {
    DBG("Stop mdmi_redirector");
    property_set("ctl.stop", "mdmi_redirector");
    sleep(1);
}

static int Start(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    if (!StartMdmiRedirectorSync()) {
        printf("Start MDMI redirector failed!");
    } else {
        printf("Start MDMI redirector successfully!");
    }
    return 0;
}

static int Stop(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    StopMdmiRedirector();
    return 0;
}

static int CreateSession(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
    return SendCommand(MDMI_CMD_CODE_CREATE_SESSION, argc, argv)? 0: -1;
}

static int CloseSession(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
    bool ret = SendCommand(MDMI_CMD_CODE_CLOSE_SESSION, argc, argv);
    return (ret)? 0: -1;
}

static int Get(int argc, char **argv) {
    connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
    return SendCommand(MDMI_CMD_CODE_GET, argc, argv)? 0: -1;
}

static int Set(int argc, char **argv) {
    connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
    return SendCommand(MDMI_CMD_CODE_SET, argc, argv)? 0: -1;
}

static int Invoke(int argc, char **argv) {
    connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
    return SendCommand(MDMI_CMD_CODE_INVOKE, argc, argv)? 0: -1;
}

static int Subscribe(int argc, char **argv) {
    connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
    return SendCommand(MDMI_CMD_CODE_SUBSCRIBE, argc, argv)? 0: -1;
}

static int Unsubscribe(int argc, char **argv) {
    connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
    return SendCommand(MDMI_CMD_CODE_UNSUBSCRIBE, argc, argv)? 0: -1;
}

static int GetSessionStats(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
    return SendCommand(MDMI_CMD_CODE_GET_SESSION_STATS, argc, argv)? 0: -1;
}

//
// Utility Functions
//
bool SendCommand(uint16_t cmdCode, int argc, char **argv)
{
    static const uint8_t SYNC[4] = {0xF7, 0xDC, 0xE2, 0xB9};
    static uint8_t buffer[COMMAND_ARGS_LEN_MAX * 2] = {0};
    uint8_t *pCurr = buffer;

    if (COMMAND_ARGS_LEN_MAX < argc) {
        DBG("Error! Argument length [%" PRIu32 "] > [%d] for command [%" PRIu16 "].", argc, COMMAND_ARGS_LEN_MAX, cmdCode);
        return false;
    }

    if (0 < argc && NULL == argv) {
        DBG("Error! Argument length [%" PRIu32 "] > 0 for command [%" PRIu16 "] but args is NULL.", argc, cmdCode);
        return false;
    }

    // concatenate argv tp argvString
    std::string argvString;
    for(int i = 0 ; i < argc ; ++i) {
        argvString.append(argv[i]);
        argvString.append(" ");
    }

    // fill buffer
    memcpy(pCurr, SYNC, sizeof(SYNC)); pCurr += sizeof(SYNC);
    *((uint16_t *)pCurr) = cmdCode; pCurr += sizeof(uint16_t);
    *((uint32_t *)pCurr) = argvString.length(); pCurr += sizeof(uint32_t);
    if(argvString.length()) {
        memcpy(pCurr, argvString.c_str(), argvString.length()); pCurr += argvString.length();
        DBG("[Debug] argvString = \"%s\" , len = %u.", argvString.c_str(), argvString.length());
    }

    return connection.Write(buffer, pCurr - buffer, NULL);
}

