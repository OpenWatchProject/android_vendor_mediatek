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

#define MAPI_VERSION "v2.1"

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include "SocketConnection.h"

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

#define LOG_TAG "MAPI-MdiRedirectorCtrl"
#define DBG(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define REDIRECTOR_COMMAND_RECEIVER_NAME "inno_mdi_redirector_cmd_receiver" // send command to mdi_redirector
#define COMMAND_ARGS_LEN_MAX 4096
//#define IS_MDM_STARTED_BY_MAPI "debug.isMDMStartedByMAPI"

#define UNUSED(x) do{(void)(x); } while (0)

enum {
    REDIREDCTOR_CMD_CODE_UNDEFINED,
    REDIREDCTOR_CMD_CODE_VERSION,
    REDIREDCTOR_CMD_CODE_ENTER_TESTING_MODE,
    REDIREDCTOR_CMD_CODE_LEAVE_TESTING_MODE,
    REDIREDCTOR_CMD_CODE_START_MONITOR,
    REDIREDCTOR_CMD_CODE_STOP_MONITOR,
    REDIREDCTOR_CMD_CODE_TRAP_SUBSCRIBE,
    REDIREDCTOR_CMD_CODE_TRAP_UNSUBSCRIBE,
    REDIREDCTOR_CMD_CODE_OBJECT_SUBSCRIBE,
    REDIREDCTOR_CMD_CODE_OBJECT_UNSUBSCRIBE,
    REDIREDCTOR_CMD_CODE_OBJECT_QUERY,
    REDIREDCTOR_CMD_CODE_STOP_PROCESS,
    REDIREDCTOR_CMD_CODE_GET_LATEST_LOG_PATH,
    REDIREDCTOR_CMD_CODE_SET_PAYLOAD_SIZE,
    REDIREDCTOR_CMD_CODE_SET_PACKET_TYPE,
    REDIREDCTOR_CMD_CODE_HELP
};

int gFUNC = REDIREDCTOR_CMD_CODE_UNDEFINED;
bool gDebug = false;

static struct option longopts[] = {
    {"version"              , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_VERSION},
    {"enter-testing-mode"   , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_ENTER_TESTING_MODE},
    {"leave-testing-mode"   , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_LEAVE_TESTING_MODE},
    {"start-monitor"        , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_START_MONITOR},
    {"stop-monitor"         , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_STOP_MONITOR},
    {"stop-process"         , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_STOP_PROCESS},
    {"get-latest-log-path"  , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_GET_LATEST_LOG_PATH},
    {"subscribe-frame"      , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_TRAP_SUBSCRIBE},
    {"unsubscribe-frame"    , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_TRAP_UNSUBSCRIBE},
    {"subscribe-object"     , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_OBJECT_SUBSCRIBE},
    {"unsubscribe-object"   , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_OBJECT_UNSUBSCRIBE},
    {"query-object"         , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_OBJECT_QUERY},
    {"set-payload-size"     , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_SET_PAYLOAD_SIZE},
    {"set-packet-type"      , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_SET_PACKET_TYPE},
    {"help"                 , no_argument, (int *) &gFUNC, REDIREDCTOR_CMD_CODE_HELP},
    {NULL                   , 0          , NULL          , 0}
};

int GetMapiVersion()
{
    printf(MAPI_VERSION);
    return 0;
}

int long_help(int ret)
{
    printf("Usage: mdi_redirector_ctrl\n");
    printf("\t--version\n");
    printf("\t--enter-testing-mode\n");
    printf("\t--leave-testing-mode\n");
    printf("\t--start-monitor\n");
    printf("\t--stop-monitor\n");
    printf("\t--subsrcibe-object\n");
    printf("\t--unsubsrcibe-object\n");
    printf("\t--query-object (need to call --subsrcibe-object first)\n");
    printf("\t--set-payload-size\n");
    printf("\t--set-packet-type\n");
    printf("\t--help\n");

    return ret;
}

// Utility functions prototype
bool SendCommand(uint16_t cmdCode, int argc, char **argv);

// Operation functions prototype
static int EnterTestingModeMain(int argc, char **argv);
static int LeaveTestingModeMain(int argc, char **argv);
static int StartMonitorMain(int argc, char **argv);
static int StopMonitorMain(int argc, char **argv);
static int StopProcessMain(int argc, char **argv);
static int GetLatestLogPathMain(int argc, char **argv);
static int SubscribeTrap(int argc, char **argv);
static int UnsubscribeTrap(int argc, char **argv);
static int QueryTrap(int argc, char **argv);
static int SetPayloadSize(int argc, char **argv);
static int SetPacketType(int argc, char **argv);

static SocketConnection connection;
#define CHECK_ARGV_EQ_ATTR(attr) (0 == strncasecmp(argv[i], attr"=", sizeof(attr"=")-1))
#define GET_ARGV_EQ_VALUE() (strchr(argv[i], '=') + 1)

// check there is "debug=1" in argv, if yes, enable gDebug
static void CheckDebugEnable(int argc, char **argv) {
    for (int i = 0 ; i < argc ; i++) {
        if (!CHECK_ARGV_EQ_ATTR("debug")) {
            continue;
        }
        if (1 == atoi(GET_ARGV_EQ_VALUE())) {
            gDebug = true;
        }
    }
}

int main(int argc, char **argv) {
    // get command action to gFUNC
    int opt = getopt_long_only(argc, argv, "", longopts, NULL);
    if (opt == -1) {
        DBG("main: Invalid command!");
        return long_help(255);
    }

    argc -= optind;
    argv += optind;

    DBG("main: command code = %d", gFUNC);

    // check there is "debug=1" in argv, if yes, enable gDebug
    CheckDebugEnable(argc, argv);

    switch (gFUNC) {
        case REDIREDCTOR_CMD_CODE_VERSION:
            return GetMapiVersion();
        case REDIREDCTOR_CMD_CODE_ENTER_TESTING_MODE:
            return EnterTestingModeMain(argc, argv);
        case REDIREDCTOR_CMD_CODE_LEAVE_TESTING_MODE:
            return LeaveTestingModeMain(argc, argv);
        case REDIREDCTOR_CMD_CODE_START_MONITOR:
            connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
            return StartMonitorMain(argc, argv);
        case REDIREDCTOR_CMD_CODE_STOP_MONITOR:
            connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
            return StopMonitorMain(argc, argv);
        case REDIREDCTOR_CMD_CODE_STOP_PROCESS:
            return StopProcessMain(argc, argv);
        case REDIREDCTOR_CMD_CODE_GET_LATEST_LOG_PATH:
            return GetLatestLogPathMain(argc, argv);
        case REDIREDCTOR_CMD_CODE_TRAP_SUBSCRIBE:
            connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
            return SubscribeTrap(argc, argv);
        case REDIREDCTOR_CMD_CODE_TRAP_UNSUBSCRIBE:
            connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
            return UnsubscribeTrap(argc, argv);
        case REDIREDCTOR_CMD_CODE_OBJECT_SUBSCRIBE:
            connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
            return SubscribeTrap(argc, argv);
        case REDIREDCTOR_CMD_CODE_OBJECT_UNSUBSCRIBE:
            connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
            return UnsubscribeTrap(argc, argv);
        case REDIREDCTOR_CMD_CODE_OBJECT_QUERY:
            connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
            return QueryTrap(argc, argv);
        case REDIREDCTOR_CMD_CODE_SET_PAYLOAD_SIZE:
            connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
            return SetPayloadSize(argc, argv);
        case REDIREDCTOR_CMD_CODE_SET_PACKET_TYPE:
            connection.Connect((void *)REDIRECTOR_COMMAND_RECEIVER_NAME);
            return SetPacketType(argc, argv);
        case REDIREDCTOR_CMD_CODE_HELP:
            return long_help(0);
        default:
            DBG("Unknown parameters.");
            return long_help(255);
    }
}

//
// Operation functions
//
static int EnterTestingModeMain(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    int retry = -1;
    char buffer[PROP_VALUE_MAX] = {'\0'};

    for (retry = -1; retry < 3; retry++) {
        property_get("init.svc.mdi_redirector", buffer, "0");
        DBG("init.svc.mdi_redirector: %s", buffer);
        if (NULL == strstr(buffer, "running")) {
            DBG("Start mdi_redirector");
            property_set("ctl.start", "mdi_redirector");
            sleep(5);
        } else {
            DBG("Find mdi_redirector!");
            return 0;
        }
    }
    sleep(5);

    return 255;
}

static int LeaveTestingModeMain(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    DBG("Stop mdi_redirector");
    property_set("ctl.stop", "mdi_redirector");
    sleep(1);

    return 0;
}

static int StartMonitorMain(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    return SendCommand(0, 0, NULL)? 0: -1;
}

static int StopMonitorMain(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    return SendCommand(1, 0, NULL)? 0: -1;
}

static int StopProcessMain(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    DBG("command 'stop-process' was deprecated!");
    return 0;
}

static int GetLatestLogPathMain(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    DBG("command 'get-latest-log-path' was deprecated!");
    return 0;
}

static int SubscribeTrap(int argc, char **argv)
{
   /**
    * CLI: --subscribe-trap <type=x> [ <msgId=x> | <msgName=x> ]
    */

    // send argv to redirector (redirector parse and operation)
    return SendCommand(2, argc, argv)? 0: -1;
}

static int UnsubscribeTrap(int argc, char **argv)
{
    /**
    * CLI: --unsubscribe-trap <type=x> [ <msgId=x> | <msgName=x> ]
    */

    // send argv to redirector (redirector parse and operation)
    return SendCommand(3, argc, argv)? 0: -1;
}

static int QueryTrap(int argc, char **argv)
{
    /**
    * CLI: --query-trap <OID=x> <SIMID=y>
    */

    // send argv to redirector (redirector parse and operation)
    return SendCommand(6, argc, argv)? 0: -1;
}

static int SetPayloadSize(int argc, char **argv)
{
   /**
    * CLI: --set-payload-size <type=x> <max_raw_length=x> [ <msgId=x> ]
    */

    // send argv to redirector (redirector parse and operation)
    return SendCommand(4, argc, argv)? 0: -1;
}

static int SetPacketType(int argc, char **argv)
{
   /**
    * CLI: --set-ip-raw-size <type=x> <packet_type=x>
    */

    // send argv to redirector (redirector parse and operation)
    return SendCommand(5, argc, argv)? 0: -1;
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

