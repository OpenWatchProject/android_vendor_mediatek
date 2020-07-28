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
 * @file CommandInterface.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c CommandInterface class which contains the set of modem monitor commands.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __COMMAND_INTERFACE_H__
#define __COMMAND_INTERFACE_H__
#include "libmdmonitor.h"
#include <vendor/mediatek/hardware/mdmonitor/1.0/IMDMonitorClientCallback.h>
namespace libmdmonitor {

using ::vendor::mediatek::hardware::mdmonitor::V1_0::IMDMonitorClientCallback;


/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief An abstract class for modem monitor command interface.
 *
 * @see JsonCmdEncoder
 * @see RawCmdEncoder
 * @see MonitorCore
 */
class CommandInterface 
{
public:
    /**
     * @brief CommandInterface constructor. 
     */
	CommandInterface() {}

    /**
     * @brief CommandInterface destructor.
     */
	virtual ~CommandInterface() {}

    /* Command Interface */
    /**
     * @brief Create a session for further operations.
     *
     * @param secret [IN] !NULL: A predefined string for privileged session.<br>
     *                     NULL: Cratea guest session.
     *
     * @return A session ID, an integer (1 ~ 2^63) in uint64_t: A session is successfully created.<br>
     *         SID_ERR (0): Exceed maximum session count, no empty slot is reserved for new session.
     */
    virtual SID onCreateSession(const char *secret = NULL) = 0;

    /**
     * @brief Close a session.
     *
     * @param doerId [IN] The doer's session ID.
     * @param sessionId [IN] The session ID to be closed.
     *
     * @return MONITOR_CMD_RESP_SUCCESS: @a sessionId was successfully closed.<br>
     *         MONITOR_CMD_RESP_DOER_NOT_EXIST: doer's session doesn't exist.<br>
     *         MONITOR_CMD_RESP_SESSION_EXPIRED: doer's session is expired.<br>
     *         MONITOR_CMD_RESP_PERMISSION_DENIED: doer's session is not the privileged one. No permission to close other's session.<br>
     *         MONITOR_CMD_RESP_NO_SUCH_SESSION: @a sessionId doesn't exist.<br>
     *         MONITOR_CMD_RESP_INTERNAL_ERROR: Internal errors.<br>
     *         MONITOR_CMD_RESP_BAD_CMD_FMT: Failed to parse the command payload.<br>
     *         MONITOR_CMD_RESP_UNDEFINED_CMD: The command was parsed successfully, but the command is not recognized.<br>
     *         MONITOR_CMD_RESP_BAD_PARAMETERS: The command is recognized, but the parameters are not in expectation.<br>
     *         MONITOR_CMD_RESP_NO_CONNECTION: The command couldn't be sent due to the connection error.<br>
     *         MONITOR_CMD_RESP_BAD_REPLY: The reply is in ill-format.
     */
    virtual MONITOR_CMD_RESP onCloseSession(SID doerId, SID sessionId) = 0;

    /**
     * @brief Subscribe the trap KPI, Message and so on.
     *
     * @param doerId [IN] The doer's session ID.
     * @param sessionId [IN] The session to subscribe.
     * @param type [IN] The trap type of message.
     * @param msgId [IN] The message ID.
     *
     * @return MONITOR_CMD_RESP_SUCCESS: The subscription was successfully.<br>
     *         MONITOR_CMD_RESP_DOER_NOT_EXIST: doer's session doesn't exist.<br>
     *         MONITOR_CMD_RESP_SESSION_EXPIRED: doer's session is expired.<br> 
     *         MONITOR_CMD_RESP_PERMISSION_DENIED: doer's session is not the privileged one. No permission to close other's session.<br>
     *         MONITOR_CMD_RESP_NO_SUCH_SESSION: @a sessionId doesn't exist.<br>
     *         MONITOR_CMD_RESP_INTERNAL_ERROR: Internal errors.<br>
     *         MONITOR_CMD_RESP_BAD_CMD_FMT: Failed to parse the command payload.<br>
     *         MONITOR_CMD_RESP_UNDEFINED_CMD: The command was parsed successfully, but the command is not recognized.<br>
     *         MONITOR_CMD_RESP_BAD_PARAMETERS: The command is recognized, but the parameters are not in expectation.<br>
     *         MONITOR_CMD_RESP_NO_CONNECTION: The command couldn't be sent due to the connection error.<br>
     *         MONITOR_CMD_RESP_BAD_REPLY: The reply is in ill-format.
     */
    virtual MONITOR_CMD_RESP onSubscribeTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID msgId, bool enableAsyncResponse = false) = 0;
    virtual MONITOR_CMD_RESP onSubscribeMultiTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID *msgId, unsigned int msgIdSize, bool enableAsyncResponse = false) = 0;

    /**
     * @brief Unsubscribe the trap KPI, Message and so on.
     *
     * @param doerId [IN] The doer's session ID.
     * @param sessionId [IN] The session to unsubscribe.
     * @param type [IN] The trap type of message.
     * @param msgId [IN] The message ID.
     *
     * @return MONITOR_CMD_RESP_SUCCESS: The unsubscription was successfully.<br>
     *         MONITOR_CMD_RESP_DOER_NOT_EXIST: doer's session doesn't exist.<br>
     *         MONITOR_CMD_RESP_SESSION_EXPIRED: doer's session is expired.<br> 
     *         MONITOR_CMD_RESP_PERMISSION_DENIED: doer's session is not the privileged one. No permission to close other's session.<br>
     *         MONITOR_CMD_RESP_NO_SUCH_SESSION: @a sessionId doesn't exist.<br>
     *         MONITOR_CMD_RESP_INTERNAL_ERROR: Internal errors.<br>
     *         MONITOR_CMD_RESP_BAD_CMD_FMT: Failed to parse the command payload.<br>
     *         MONITOR_CMD_RESP_UNDEFINED_CMD: The command was parsed successfully, but the command is not recognized.<br>
     *         MONITOR_CMD_RESP_BAD_PARAMETERS: The command is recognized, but the parameters are not in expectation.<br>
     *         MONITOR_CMD_RESP_NO_CONNECTION: The command couldn't be sent due to the connection error.<br>
     *         MONITOR_CMD_RESP_BAD_REPLY: The reply is in ill-format.
     */
    virtual MONITOR_CMD_RESP onUnsubscribeTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID msgId, bool enableAsyncResponse = false) = 0;
    virtual MONITOR_CMD_RESP onUnsubscribeMultiTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID *msgId, unsigned int msgIdSize, bool enableAsyncResponse = false) = 0;

    /**
     * @brief Start to send traps.
     *
     * @param doerId [IN] The doer's session ID.
     * @param sessionId [IN] The session to start to send traps.
     *
     * @return MONITOR_CMD_RESP_SUCCESS: The traps are starting to send.<br>
     *         MONITOR_CMD_RESP_DOER_NOT_EXIST: doer's session doesn't exist.<br>
     *         MONITOR_CMD_RESP_SESSION_EXPIRED: doer's session is expired.<br> 
     *         MONITOR_CMD_RESP_PERMISSION_DENIED: doer's session is not the privileged one. No permission to close other's session.<br>
     *         MONITOR_CMD_RESP_NO_SUCH_SESSION: @a sessionId doesn't exist.<br>
     *         MONITOR_CMD_RESP_INTERNAL_ERROR: Internal errors.<br>
     *         MONITOR_CMD_RESP_BAD_CMD_FMT: Failed to parse the command payload.<br>
     *         MONITOR_CMD_RESP_UNDEFINED_CMD: The command was parsed successfully, but the command is not recognized.<br>
     *         MONITOR_CMD_RESP_BAD_PARAMETERS: The command is recognized, but the parameters are not in expectation.<br>
     *         MONITOR_CMD_RESP_NO_CONNECTION: The command couldn't be sent due to the connection error.<br>
     *         MONITOR_CMD_RESP_BAD_REPLY: The reply is in ill-format.
     */
    virtual MONITOR_CMD_RESP onEnableTrap(SID doerId, SID sessionId) = 0;

    /**
     * @brief Stop to send traps.
     *
     * @param doerId [IN] The doer's session ID.
     * @param sessionId [IN] The session to stop to send traps.
     *
     * @return MONITOR_CMD_RESP_SUCCESS: Sending of the traps is stopped.<br>
     *         MONITOR_CMD_RESP_DOER_NOT_EXIST: doer's session doesn't exist.<br>
     *         MONITOR_CMD_RESP_SESSION_EXPIRED: doer's session is expired.<br> 
     *         MONITOR_CMD_RESP_PERMISSION_DENIED: doer's session is not the privileged one. No permission to close other's session.<br>
     *         MONITOR_CMD_RESP_NO_SUCH_SESSION: @a sessionId doesn't exist.<br>
     *         MONITOR_CMD_RESP_INTERNAL_ERROR: Internal errors.<br>
     *         MONITOR_CMD_RESP_BAD_CMD_FMT: Failed to parse the command payload.<br>
     *         MONITOR_CMD_RESP_UNDEFINED_CMD: The command was parsed successfully, but the command is not recognized.<br>
     *         MONITOR_CMD_RESP_BAD_PARAMETERS: The command is recognized, but the parameters are not in expectation.<br>
     *         MONITOR_CMD_RESP_NO_CONNECTION: The command couldn't be sent due to the connection error.<br>
     *         MONITOR_CMD_RESP_BAD_REPLY: The reply is in ill-format.
     */
    virtual MONITOR_CMD_RESP onDisableTrap(SID doerId, SID sessionId) = 0;

    /**
     * @brief Set the hostname of the trap receiver.
     *
     * @param doerId [IN] The doer's session ID.
     * @param sessionId [IN] The session to set.
     * @param szServerName [IN] The server name of the trap receiver.
     *
     * @return MONITOR_CMD_RESP_SUCCESS: Trap receiver was successfully set.<br>
     *         MONITOR_CMD_RESP_DOER_NOT_EXIST: doer's session doesn't exist.<br>
     *         MONITOR_CMD_RESP_SESSION_EXPIRED: doer's session is expired.<br> 
     *         MONITOR_CMD_RESP_PERMISSION_DENIED: doer's session is not the privileged one. No permission to close other's session.<br>
     *         MONITOR_CMD_RESP_NO_SUCH_SESSION: @a sessionId doesn't exist.<br>
     *         MONITOR_CMD_RESP_INTERNAL_ERROR: Internal errors.<br>
     *         MONITOR_CMD_RESP_BAD_CMD_FMT: Failed to parse the command payload.<br>
     *         MONITOR_CMD_RESP_UNDEFINED_CMD: The command was parsed successfully, but the command is not recognized.<br>
     *         MONITOR_CMD_RESP_BAD_PARAMETERS: The command is recognized, but the parameters are not in expectation.<br>
     *         MONITOR_CMD_RESP_NO_CONNECTION: The command couldn't be sent due to the connection error.<br>
     *         MONITOR_CMD_RESP_BAD_REPLY: The reply is in ill-format.
     */
    virtual MONITOR_CMD_RESP onSetTrapReceiver(SID doerId, SID sessionId, const char *szServerName) = 0;

    virtual MONITOR_CMD_RESP onSetHIDLTrapCallback(const char *szServerName, const android::sp<IMDMonitorClientCallback>& callback) = 0;

    /**
     * @brief Set the raw data size of IP packet data.
     *
     * @param doerId [IN] The doer's session ID.
     * @param sessionId [IN] The session to set.
     * @param type [IN] The type of IP packet to set.
     * @param rawSize [IN] The raw data size to set.
     *
     * @return MONITOR_CMD_RESP_SUCCESS: Trap receiver was successfully set.<br>
     *         MONITOR_CMD_RESP_DOER_NOT_EXIST: doer's session doesn't exist.<br>
     *         MONITOR_CMD_RESP_SESSION_EXPIRED: doer's session is expired.<br>
     *         MONITOR_CMD_RESP_PERMISSION_DENIED: doer's session is not the privileged one. No permission to close other's session.<br>
     *         MONITOR_CMD_RESP_NO_SUCH_SESSION: @a sessionId doesn't exist.<br>
     *         MONITOR_CMD_RESP_INTERNAL_ERROR: Internal errors.<br>
     *         MONITOR_CMD_RESP_BAD_CMD_FMT: Failed to parse the command payload.<br>
     *         MONITOR_CMD_RESP_UNDEFINED_CMD: The command was parsed successfully, but the command is not recognized.<br>
     *         MONITOR_CMD_RESP_BAD_PARAMETERS: The command is recognized, but the parameters are not in expectation.<br>
     *         MONITOR_CMD_RESP_NO_CONNECTION: The command couldn't be sent due to the connection error.<br>
     *         MONITOR_CMD_RESP_BAD_REPLY: The reply is in ill-format.
     */
    virtual MONITOR_CMD_RESP onSetIPRawSize(SID doerId, SID sessionId, IP_TYPE type, unsigned int rawSize) = 0;

    /**
     * @brief Get the session status.
     *
     * @param doerId [IN] The doer's session ID.
     * @param sessionId [IN] The session status to get.
     * @param status [OUT] The returned status.
     *
     * @return MONITOR_CMD_RESP_SUCCESS: Trap receiver was successfully set.<br>
     *         MONITOR_CMD_RESP_DOER_NOT_EXIST: doer's session doesn't exist.<br>
     *         MONITOR_CMD_RESP_SESSION_EXPIRED: doer's session is expired.<br> 
     *         MONITOR_CMD_RESP_PERMISSION_DENIED: doer's session is not the privileged one. No permission to close other's session.<br>
     *         MONITOR_CMD_RESP_NO_SUCH_SESSION: @a sessionId doesn't exist.<br>
     *         MONITOR_CMD_RESP_INTERNAL_ERROR: Internal errors.<br>
     *         MONITOR_CMD_RESP_BAD_CMD_FMT: Failed to parse the command payload.<br>
     *         MONITOR_CMD_RESP_UNDEFINED_CMD: The command was parsed successfully, but the command is not recognized.<br>
     *         MONITOR_CMD_RESP_BAD_PARAMETERS: The command is recognized, but the parameters are not in expectation.<br>
     *         MONITOR_CMD_RESP_NO_CONNECTION: The command couldn't be sent due to the connection error.<br>
     *         MONITOR_CMD_RESP_BAD_REPLY: The reply is in ill-format.
     */
    virtual MONITOR_CMD_RESP onGetSession(SID doerId, SID sessionId, MONITOR_SESSION_STATUS &status) = 0;

    /**
     * @brief Get the session list.
     *
     * @param doerId [IN] The doer's session ID.
     * @param list [OUT] The returned session list.
     *
     * @return MONITOR_CMD_RESP_SUCCESS: Trap receiver was successfully set.<br>
     *         MONITOR_CMD_RESP_DOER_NOT_EXIST: doer's session doesn't exist.<br>
     *         MONITOR_CMD_RESP_SESSION_EXPIRED: doer's session is expired.<br> 
     *         MONITOR_CMD_RESP_PERMISSION_DENIED: doer's session is not the privileged one. No permission to close other's session.<br>
     *         MONITOR_CMD_RESP_NO_SUCH_SESSION: @a sessionId doesn't exist.<br>
     *         MONITOR_CMD_RESP_INTERNAL_ERROR: Internal errors.<br>
     *         MONITOR_CMD_RESP_BAD_CMD_FMT: Failed to parse the command payload.<br>
     *         MONITOR_CMD_RESP_UNDEFINED_CMD: The command was parsed successfully, but the command is not recognized.<br>
     *         MONITOR_CMD_RESP_BAD_PARAMETERS: The command is recognized, but the parameters are not in expectation.<br>
     *         MONITOR_CMD_RESP_NO_CONNECTION: The command couldn't be sent due to the connection error.<br>
     *         MONITOR_CMD_RESP_BAD_REPLY: The reply is in ill-format.
     */
    virtual MONITOR_CMD_RESP onListSession(SID doerId, std::vector<SID> &list) = 0;

    /**
     * @brief Set the debug level.
     *
     * @param doerId [IN] The doer's session ID.
     * @param level [IN] debug level, type of MONITOR_DEBUG_LEVEL.
     *
     * @return MONITOR_CMD_RESP_SUCCESS: Trap receiver was successfully set.<br>
     *         MONITOR_CMD_RESP_DOER_NOT_EXIST: doer's session doesn't exist.<br>
     *         MONITOR_CMD_RESP_SESSION_EXPIRED: doer's session is expired.<br> 
     *         MONITOR_CMD_RESP_PERMISSION_DENIED: doer's session is not the privileged one. No permission to close other's session.<br>
     *         MONITOR_CMD_RESP_NO_SUCH_SESSION: @a sessionId doesn't exist.<br>
     *         MONITOR_CMD_RESP_INTERNAL_ERROR: Internal errors.<br>
     *         MONITOR_CMD_RESP_BAD_CMD_FMT: Failed to parse the command payload.<br>
     *         MONITOR_CMD_RESP_UNDEFINED_CMD: The command was parsed successfully, but the command is not recognized.<br>
     *         MONITOR_CMD_RESP_BAD_PARAMETERS: The command is recognized, but the parameters are not in expectation.<br>
     *         MONITOR_CMD_RESP_NO_CONNECTION: The command couldn't be sent due to the connection error.<br>
     *         MONITOR_CMD_RESP_BAD_REPLY: The reply is in ill-format.
     */
    virtual MONITOR_CMD_RESP onSetDebugLevel(SID doerId, MONITOR_DEBUG_LEVEL level) = 0;

    /**
     * @judge is java client packagename in the white list.
     *
     * @param szClientPackage java caller package name.
     */
    virtual bool isAuthorized(const char *) = 0;
};
} //namespace libmdmonitor {

#endif
