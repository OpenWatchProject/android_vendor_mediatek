/*
 * Copyright (c) 2015 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
#include <stdlib.h>
#include <stdint.h>

#include "buildTag.h"
#include "mcLoadFormat.h"
#include "log.h"

#include "MobiCore_defines.h"
#include "DebugInfo.h"
#include "Utils.h"

const char * LAST_MCP_COMMANDS_PATH  = "/d/trustonic_tee/last_mcp_commands";
const char * SESSIONS_PATH           = "/d/trustonic_tee/sessions";
const char * STRUCTS_PATH            = "/d/trustonic_tee/structs";

// /!\ apparently the debugfs is bugged : fopen cannot be performed on a debugfs
// file, the linux cat function is executed instead to display its content

void DebugInfo::showLastMcpCommand() {
    printf("\n" BOLD("TEE (dgb FS) last mcp commands : ") "\n");
    if (showFileContent(LAST_MCP_COMMANDS_PATH)) {
        IP_LOG_E("Can't display %s content", LAST_MCP_COMMANDS_PATH);
    }
}

void DebugInfo::showSessions() {
    printf("\n" BOLD("TEE (dgb FS) sessions : ") "\n");
    if (showFileContent(SESSIONS_PATH)) {
        IP_LOG_E("Can't display %s content", SESSIONS_PATH);
    }
}

void DebugInfo::showStructs() {
    printf("\n" BOLD("TEE (dgb FS) structs : ") "\n");
    if (showFileContent(STRUCTS_PATH)) {
        IP_LOG_E("Can't display %s content", STRUCTS_PATH);
    }
}
