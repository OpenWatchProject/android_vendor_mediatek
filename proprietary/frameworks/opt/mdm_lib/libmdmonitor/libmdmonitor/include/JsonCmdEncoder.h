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
 * @file JsonCmdEncoder.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c JsonCmdEncoder class used to encode modem monitor commands into Json objects.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __JSON_CMD_ENCODER_H__
#define __JSON_CMD_ENCODER_H__
#include "libmdmonitor.h"
#include "MCPInfo.h"
#include "Transmitter.h"
namespace libmdmonitor {
/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to encode modem monitor commands into Json objects that will be sent via @a m_pMCPTransmitter.
 *
 * @see CmdEncoder
 * @see CommandInterface
 * @see JsonCmd
 */
class JsonCmdEncoder
{
public:
    /**
     * @brief Create a empty Json command encoder and set the MCP transmitter to @a pMCPTransmitter. 
     *
     * @param pMCPTransmitter [IN] The pointer to the MCP transmitter. Should not be NULL.
     */
    JsonCmdEncoder(Transmitter *pMCPTransmitter);

    /**
     * @brief A wrapper function to send command/receive reply via MCP transmitter.
     *
     * @param cmd [IN] The MCP information of encoded command to be sent.
     * @param ppReply [OUT] !NULL: The address of the variable pointing to the MCP information of reply.<br>
     *                       NULL: Do not need the reply.
     *
     * @return true: Command is successfully sent.<br>
     *        false: Error.
     *
     */
    bool Send(const MCPInfo &cmd, MCPInfo **ppReply);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onCreateSession 
     */
    SID CreateSession(const char *secret = NULL);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onCloseSession
     */
    MONITOR_CMD_RESP CloseSession(SID doerId, SID sessionId);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onSubscribeTrap
     */
    MONITOR_CMD_RESP SubscribeTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID msgId, bool enableAsyncResponse = false);
    MONITOR_CMD_RESP SubscribeMultiTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID *msgId, unsigned int msgIdSize, bool enableAsyncResponse = false);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onUnsubscribeTrap
     */
    MONITOR_CMD_RESP UnsubscribeTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID msgId, bool enableAsyncResponse = false);
    MONITOR_CMD_RESP UnsubscribeMultiTrap(SID doerId, SID sessionId, TRAP_TYPE type, MSGID *msgId, unsigned int msgIdSize, bool enableAsyncResponse = false);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onEnableTrap
     */

    MONITOR_CMD_RESP EnableTrap(SID doerId, SID sessionId);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onDisableTrap
     */
    MONITOR_CMD_RESP DisableTrap(SID doerId, SID sessionId);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onSetTrapReceiver
     */
    MONITOR_CMD_RESP SetTrapReceiver(SID doerId, SID sessionId, const char *szServerName);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onSetIPRawSize
     */
    MONITOR_CMD_RESP SetIPRawSize(SID doerId, SID sessionId, IP_TYPE type, unsigned int rawSize);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onGetSession
     */
    MONITOR_CMD_RESP GetSession(SID doerId, SID sessionId, MONITOR_SESSION_STATUS &status);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onListSession
     */
    MONITOR_CMD_RESP ListSession(SID doerId, std::vector<SID> &list);

    /**
     * @brief The API is used as a proxy to pack command and set to remote inter process.
     * @see CommandInterface::onSetDebugLevel
     */
    MONITOR_CMD_RESP SetDebugLevel(SID doerId, MONITOR_DEBUG_LEVEL level);
    /**
     * @brief Destructor. 
     */
    virtual ~JsonCmdEncoder() {}

protected:
    /**
     * @brief The hook of MCP transmitter.
     *
     * The hook should be initialized in constructor.
     */
    Transmitter *m_pMCPTransmitter;
};
} //namespace libmdmonitor {

#endif
