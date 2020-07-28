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
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/**
 * @file MonitorTypes.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file contains the types that are used in modem monitor library.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __MONITOR_TYPES_H__
#define __MONITOR_TYPES_H__
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <vector>
#include <string>
namespace libmdmonitor {

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The enum lists the response codes of command execution.
 */
typedef enum {
    MONITOR_CMD_RESP_SUCCESS = 0,
    MONITOR_CMD_RESP_INTERNAL_ERROR,
    MONITOR_CMD_RESP_UNDEFINED_CMD,
    MONITOR_CMD_RESP_BAD_PARAMETERS,
    MONITOR_CMD_RESP_BAD_CMD_FMT,
    MONITOR_CMD_RESP_NO_SUCH_SESSION,
    MONITOR_CMD_RESP_SESSION_EXPIRED,
    MONITOR_CMD_RESP_DOER_NOT_EXIST,
    MONITOR_CMD_RESP_PERMISSION_DENIED,
    MONITOR_CMD_RESP_NO_CONNECTION,
    MONITOR_CMD_RESP_BAD_REPLY,
    MONITOR_CMD_RESP_OP_FAILED,
    MONITOR_CMD_RESP_NO_SUCH_TRAP_MESSAGE,
    MONITOR_CMD_RESP_NOT_IMPLEMENT,
    MONITOR_CMD_RESP_UNAUTHORIZED,
    MONITOR_CMD_RESP_OP_TIMEOUT,
    MONITOR_CMD_RESP_SIZE
} MONITOR_CMD_RESP;

/**
 * @brief Command response validation.
 *
 * @param cmd_resp_code [IN] The command response code.
 *
 * @return true: The command response code in valid.<br>
 *        false: Otherwise.
 */
inline bool IsCmdRespValid(int cmd_resp_code)
{
	if (cmd_resp_code < 0 || MONITOR_CMD_RESP_SIZE <= cmd_resp_code) {
		return false;
	}
	return true;
}

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The enum lists the command codes.
 */
typedef enum {
    MONITOR_CMD_CODE_UNDEFINED = 0,
    MONITOR_CMD_CODE_SESSION_CREATE,
    MONITOR_CMD_CODE_SESSION_CLOSE,
    MONITOR_CMD_CODE_TRAP_SUBSCRIBE,
    MONITOR_CMD_CODE_TRAP_UNSUBSCRIBE,
    MONITOR_CMD_CODE_TRAP_ENABLE,
    MONITOR_CMD_CODE_TRAP_DISABLE,
    MONITOR_CMD_CODE_TRAP_RECEIVER_SET,
    MONITOR_CMD_CODE_SESSION_GET,
    MONITOR_CMD_CODE_SESSION_LIST,
    MONITOR_CMD_CODE_IP_RAW_SIZE_SET,
    MONITOR_CMD_CODE_DEBUG_LEVEL_SET,
    MONITOR_CMD_CODE_SIZE
} MONITOR_CMD_CODE;

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The enum lists the trap types.
 */
typedef enum {
    TRAP_TYPE_UNDEFINED,
    TRAP_TYPE_DISCARDINFO = 1,
    TRAP_TYPE_OTA = 2,
    TRAP_TYPE_EM = 3,
    TRAP_TYPE_VOLTE = 4,
    TRAP_TYPE_PSTIME = 5,
    TRAP_TYPE_IP = 6,
    TRAP_TYPE_ICD_RECORD = 7,
    TRAP_TYPE_ICD_EVENT = 8,
    /* Add new type below */
    /* ------------------ */
    TRAP_TYPE_SIZE
} TRAP_TYPE;

/**
 * @brief Convert trap type name string to @c TRAP_TYPE.
 *
 * @param szType [IN] The trap type name string.
 *
 * @return The trap type enum.
 */
TRAP_TYPE StrToTrapType(const char *szType);

/**
 * @brief Convert trap type enum to name string.
 *
 * @param type [IN] The trap type enum.
 *
 * @return NULL: No name string mapping to @a type.<br>
 *        !NULL: The name string of @a type.
 */
const char *ToTrapStr(TRAP_TYPE type);

/**
 *
 * @brief The enum lists the IP types.
 */
typedef enum {
    IP_TYPE_UNDEFINED,
    IP_TYPE_ALL = 1,

    /* Add new type below */
    /* ------------------ */
    IP_TYPE_SIZE
} IP_TYPE;
/**
 * @brief Convert ip type name string to @c IP_TYPE.
 *
 * @param szType [IN] The ip type name string.
 *
 * @return The ip type enum.
 */
IP_TYPE StrToIPType(const char *szType);


/**
 * @brief Convert ip type enum to name string.
 *
 * @param type [IN] The ip type enum.
 *
 * @return NULL: No name string mapping to @a type.<br>
 *        !NULL: The name string of @a type.
 */
const char *ToIPTypeStr(IP_TYPE type);

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The enum lists Modem Monitor Communication Packet (MCP) types.
 */
typedef enum {
	MCP_TYPE_UNDEFINED,
	MCP_TYPE_JSON_CMD,
	MCP_TYPE_JSON_CMD_RESP,
	MCP_TYPE_TRAP,
	/* ----------- */
	MCP_TYPE_SIZE
} MCP_TYPE;

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The enum lists transmitter types.
 *
 * Currently only @a TRANSMITTER_TYPE_DOMAIN_SOCKET is used.
 */
typedef enum {
    TRANSMITTER_TYPE_DOMAIN_SOCKET,
    TRANSMITTER_TYPE_BINDER,
    TRANSMITTER_TYPE_FIFO,
    TRANSMITTER_TYPE_HIDL,
} TRANSMITTER_TYPE;

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The enum lists the privilege level of sessions.
 */
typedef enum {
    PRIVILEGE_LEVEL_GUEST,
    /* Add new level below */

    /* ------------------- */
    PRIVILEGE_LEVEL_SESSION_MANAGER, // Manage all sessions
    PRIVILEGE_LEVEL_ROOT // Do everything
} PRIVILEGE_LEVEL;

/**
 * @brief Define type of session ID to uint64_t but true value will less than (1<<63).
 */
typedef uint64_t SID;

/**
 * @brief 0 is reserved for error/initial value of SID
 */
#define SID_ERR (SID)0

/**
 * @brief Define type of message ID to uint64_t. MSGID is used as primitive message ID and OTA global ID.
 */
typedef uint64_t MSGID;
/**
 * @brief Define type of EM ID to uint64_t. EMID is used as EM ID.
 */
typedef uint64_t EMID;

/**
 * @brief Define type of trace ID to uint64_t.
 */
typedef uint64_t TRACEID;

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief For storing the information of a DHL frame during decoding.
 */
typedef struct {
    unsigned char dhlFrameType;
    TRAP_TYPE trapType;
    uint32_t simIdx;

    // use as EM message ID or ICD code
    uint64_t msgID;
    uint64_t traceID;
    uint64_t otaGlobalID;

    /* ICD trap only */
    uint8_t version;
    uint64_t timestamp;
} FRAME_INFO;

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The enum lists the debug levels for modem monitor library.
 */
typedef enum {
    MONITOR_DEBUG_LEVEL_RAW_DATA_DUMPER = 1, // enable RawDataDumper
    MONITOR_DEBUG_LEVEL_VERBOSE = 2,
    MONITOR_DEBUG_LEVEL_DEBUG = 3,
    MONITOR_DEBUG_LEVEL_INFO = 4,
    MONITOR_DEBUG_LEVEL_WARN = 5,
    MONITOR_DEBUG_LEVEL_ERROR = 6,
    MONITOR_DEBUG_LEVEL_ASSERT = 7,
    MONITOR_DEBUG_LEVEL_NA = 10
} MONITOR_DEBUG_LEVEL;

struct MONITOR_SESSION_STATUS
{
    MONITOR_SESSION_STATUS():bTrapEnabled(false) {}
    bool bTrapEnabled;
    std::vector<MSGID> subscribedTraps[TRAP_TYPE_SIZE];
    std::string trapReceiverHost;
};
} //namespace libmdmonitor {

#endif
