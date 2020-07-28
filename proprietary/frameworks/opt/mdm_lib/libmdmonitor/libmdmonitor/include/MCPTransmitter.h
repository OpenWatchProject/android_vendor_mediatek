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
 * @file MCPTransmitter.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c MCPTransmitter class used to send MCP data out.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __MCP_TRANSMITTER_H__
#define __MCP_TRANSMITTER_H__
#include <unistd.h>
#include <pthread.h>
#include "libmdmonitor.h"
#include "Transmitter.h"
#include "SocketConnection.h"
namespace libmdmonitor {

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to send out MCP data through @c SocketConnection.
 *
 * @see MCPHandler
 * @see Transmitter
 */
class MCPTransmitter : public Transmitter
{
public:
    /**
     * @brief Crate @c MCPTransmitter instance that connects to @a szServerName.
     *
     * @param szServerName [IN] The host to connect.
     */
    MCPTransmitter(const char *szServerName);

    /**
     * @brief Cleanup.
     */
    virtual ~MCPTransmitter();

    /**
     * @brief Send out MCP data through @a m_connection.
     * It is synchronized, so that one send operation at one time
     * to ensure @ppReply is correct.
     *
     * @param cmd [IN] The command in MCP format to send out.
     * @param ppReply [OUT] The address of a pointer to the reply in MCP format.
     * @param bStopFlag [IN] The address of a variable indicating the write operation should be stopped immediately.
     *
     * @return true: All MCP data was successfully sent.<br>
     *        false: Error or the write operation was cancelled.
     */
    virtual bool Send(const MCPInfo &cmd, MCPInfo **ppReply, volatile bool *bStopFlag = NULL);

    /**
     * @brief Check whether the connection is valid.
     * @return true: The connection is valid.<br>
     *        false: Otherwise.
     */
    virtual bool CheckValid();

protected:
    /**
     * @brief The domain socket connection.
     */
    SocketConnection m_connection;

    /**
     * @brief The server name of the host to connect.
     */
    char m_serverName[MAX_SOCK_ADDR_LEN];

    pthread_mutex_t m_sendLock;
};
} //namespace libmdmonitor {

#endif
