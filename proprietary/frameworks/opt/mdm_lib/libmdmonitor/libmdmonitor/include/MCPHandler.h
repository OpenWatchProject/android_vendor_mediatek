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
 * @file MCPHandler.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c MCPHandler class providing APIs to handle Modem Monitor Communication Packet (MCP) processing.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */
#ifndef __MCP_HANDLER_H__
#define __MCP_HANDLER_H__
#include "libmdmonitor.h"
#include "MCPInfo.h"
namespace libmdmonitor {
class SocketConnection;
class DataQueue;

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to handle read/write data for @c BufferedConnection<br>
 *        and deal with Modem Monitor Communication Packet (MCP) processing. 
 *
 * @see MCPReceiver
 * @see MCPTransmitter
 */
class MCPHandler
{
private:
    /**
     * @brief Create an empty MCPHandler.
     */
    MCPHandler() {}

    /**
     * @brief Cleanup.
     */
    virtual ~MCPHandler() {}
public:
    /**
     * @brief Write MCP data to buffered connection.
     *
     * @param connection [IN] The connection to write data.
     * @param cmd [IN] The command data in MCP format to write.
     * @param bStopFlag [IN] The address of a variable indicating the write operation should be stopped immediately.
     *
     * @return true: All data are successfully written.<br> 
     *        false: Error or write operation was cancelled.
     */
    static bool WriteToConnection(SocketConnection &connection, const MCPInfo &cmd, volatile bool *bStopFlag = NULL);

    static bool WriteToBuffer(DataQueue &dataQ, const MCPInfo &cmd, volatile bool *bStopFlag);

    /**
     * @brief Read MCP data from buffered connection. 
     *
     * @param connection [IN] The connection to read data from.
     * @param reply [OUT] The read reply data in MCP format.
     * @param bStopFlag [IN] The address of a variable indicating the write operation should be stopped immediately.
     *
     * @return true: An MCP reply was read successfully.<br>
     *        false: Error or read operation was cancelled. 
     *
     * @par Examples
     * MCPReceiver.cpp
     * @snippet MCPReceiver.cpp MCPHandler::ProcessFromBuffer
     *
     * MCPTransmitter.cpp
     * @snippet MCPTransmitter.cpp MCPHandler::ProcessFromBuffer
     */
    static bool ProcessFromBuffer(SocketConnection &connection, MCPInfo &reply, volatile bool *bStopFlag = NULL);

    static bool ProcessFromBuffer(const uint8_t* data, size_t dataLength, MCPInfo &reply, volatile bool *bStopFlag);

protected:
    /**
     * @brief The function was used by @b ProcessFromBuffer to parse data into MCP.
     *
     * @param mcpInfo [OUT] The parsed MCP.
     * @param dataQ [IN/OUT] The data to be parsed.
     *
     * @return true: A MCP was found.<br>
     *        false: No MCP found.
     */
    static bool SearchHeader(MCPInfo &mcpInfo, DataQueue &dataQ);

    /**
     * @brief Check if @a data starts with MCP header magic.
     *
     * @param data [IN] The data to check. Should not be NULL or false will return.
     *
     * @return true: @a data starts with MCP header magic.<br>
     *        false: Otherwise.
     *
     */
    static bool IsHeaderMagic(const uint8_t *data);

    /**
     * @brief Check if @a data starts with MCP type magic.
     *
     * @param data [IN] The data to check. Should not be NULL or false will return.
     * @param type [OUT] Parsed MCP type.
     *
     * @return true: @a data starts with MCP type magic.<br>
     *        false: Otherwise.
     */
    static bool IsTypeMagic(const uint8_t *data, MCP_TYPE &type);

    /**
     * @brief Get the MCP type magic of @a type. 
     *
     * @param type [IN] The MCP type.
     *
     * @return NULL: @a type was not found in @a m_typeMagicTable.<br>
     *        !NULL: The MCP type magic of @a type.
     */
    static const uint8_t* GetTypeMagic(MCP_TYPE type);

    /**
     * @brief Check if the length of payload is valid.
     *
     * @param len [IN] The length of payload.
     *
     * @return true: The length is valid.<br>
     *        false: Otherwise.
     */
    static bool IsPayloadLenValid(size_t len); 

    /**
     * @brief 4 bytes MCP header magic.
     */
    static const uint8_t HEADER_MAGIC[4];

    /**
     * @brief The size of MCP payload length is 4 bytes. 
     */
    static const size_t PAYLOAD_LEN_BYTES = sizeof(uint32_t);

    /**
     * @brief The size of MCP type magic is 2 bytes.
     */
    static const size_t TYPE_MAGIC_BYTES = 2;

    /**
     * @brief The struct is used to map type magic.
     */
    struct TypeMagic {
        /**
         * @brief The MCP type.
         */
        MCP_TYPE m_type;

        /**
         * @brief The MCP type magic.
         */
        uint8_t m_magic[TYPE_MAGIC_BYTES];
    };

    /**
     * @brief The mapping table of MCP magic types.
     */
    static const TypeMagic m_typeMagicTable[];
};
} //namespace libmdmonitor {

#endif
