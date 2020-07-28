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
 * @file MCPReceiver.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines both @c PayloadHandlerInterface and @c MCPReceiver classes that handle the processing of MCP data.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __MCP_RECEIVER_H__
#define __MCP_RECEIVER_H__

#include <sys/types.h>
#include "CommandInterface.h"
#include "HIDLHandlerInterface.h"
#include "SocketListener.h"
#include "MCPInfo.h"
namespace libmdmonitor {
class SocketConnection;

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is an abstract class defined the interfaces for @c MCPReceiver to call back.
 *
 * @see CmdDecoder
 * @see MonitorTrapReceiver
 */
class PayloadHandlerInterface
{
public:
    /**
     * @brief Cleanup.
     */
    virtual ~PayloadHandlerInterface() {}

    /**
     * @brief Derived class should implement this function for payload processing. 
     *
     * @param in [IN] The input MCP information with data for processing.
     * @param out [OUT] The output MCP information with data.
     *
     * @return true: Success.<br>
     *        false: Error.
     */
	virtual bool ProcessPayload(const MCPInfo &in, MCPInfo &out) = 0;
};

/**
 * @brief The class is used to receive data from @c BufferedConnection,<br>
 *        parse data into MCP format and pass to @a ProcessPayload of @a m_payloadHandlerInstance by type.
 *
 * @see CommandInterface
 * @see MCPHandler
 */
class MCPReceiver : public ConnectionHandlerInterface, public HIDLHandlerInterface
{
public:
    /**
     * @brief Create an empty @c MCPReceiver instance.
     */
	MCPReceiver();

    /**
     * @brief Cleanup. 
     */
    virtual ~MCPReceiver() {}

    /**
     * @brief An implementation of @a ConnectionHandlerInterface::DataIn<br>
     *        which is called by @c SocketListener to receive data from @c BufferedConnection.
     *
     * @todo Add timeout mechanism.
     *
     * @param connection [IN] The buffered connection for data to receive.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
	bool DataIn(SocketConnection &connection);





    bool DataIn(const uint8_t* data, size_t dataLength, DataQueue & retBuf);

    /**
     * @brief Setup @c PayloadHandlerInterface 
     *
     * @param type [IN] The type to register.
     * @param handlerInstance [IN] The instance of @c PayloadHandlerInterface.<br>
     *                             Set to NULL is used to unregister the handler.
     *
     * @return true: Success.<br>
     *        false: @a type is invalid.
     */
	void SetPayloadHandler(MCP_TYPE type, PayloadHandlerInterface *handlerInstance);

protected:
    /**
     * @brief The array of pointers to payload handler interface. 
     */
    PayloadHandlerInterface *m_payloadHandlerInstance[MCP_TYPE_SIZE];
    MCPInfo m_receiveData;
};
} //namespace libmdmonitor {

#endif
