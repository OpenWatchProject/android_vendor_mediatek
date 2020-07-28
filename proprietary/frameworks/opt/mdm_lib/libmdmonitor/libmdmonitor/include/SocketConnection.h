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
 * @file SocketConnection.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c SocketConnection class used to manage Linux domain socket with abstract namespace.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */
#ifndef __SOCKET_CONNECTION_H__
#define __SOCKET_CONNECTION_H__
#include "DataQueue.h"
#include "libmdmonitor.h"
namespace libmdmonitor {

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class provides APIs to manipulate Linux domain socket with abstract namespace.
 */
class SocketConnection
{
public:
    /**
     * @brief Create a empty socket connection.
     */
    SocketConnection();

    /**
     * @brief Create a socket connection with a opened socket file descriptor @a fd.<br>
     *        The constructor is used by @c SocketListener after accepting a client connection.
     *
     * @param fd [IN] Socket file descriptor.
     * @param szAddr [IN] Client address.
     */
	SocketConnection(int fd, const char *szAddr = NULL);

    /**
     * @brief Cleanup.
     */
	virtual ~SocketConnection();

    /**
     * @brief Connect to a host with abstract domain socket host name.
     *
     * @param host [IN] The abstract domain socket host name string.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool Connect(void *host);

    /**
     * @brief Read data from socket to @a buffer.
     *
     * @param buffer [OUT] The buffer for read data.
     * @param bufferSize [IN] The size of @a buffer.
     *
     * @return -2: Bad parameters.<br>
     *         -1: Connection is closed.<br>
     *        >=0: Read size.
     */
	ssize_t Read(uint8_t *buffer, size_t bufferSize);

    /**
     * @brief Write data through the connection.
     *
     * The function will be block if the data are not totally written.<br>
     * @a bStopFlag is used to notify @b Write function should exit immediately.
     *
     * @param data [IN] The data to write. Should not be NULL or false will return.
     * @param len [IN] The data length. Can be 0 and true will return.
     * @param bStopFlag [IN] The address of a variable indicating the write operation should be stopped immediately.
     *
     * @return true: All data are successfully written.<br>
     *        false: Otherwise.
     */
    bool Write(const uint8_t *data, size_t len, volatile const bool *bStopFlag = NULL);

    /**
     * @brief Clean all data and reset to default.
     */
    void Reset();

    /**
     * @brief Check whether the connection is valid.
     *
     * @return true: The connection is valid.<br>
     *        false: Otherwise.
     */
    bool IsValid() const { return m_isValid; }

    /**
     * @brief Poll to check whether the connection is valid.
     * @return true: The connection is valid.<br>
     *        false: Otherwise.
     */
    bool CheckValid();

    /**
     * @brief Get socket file descriptor. 
     *
     * @return The socket file descriptor.
     */
    int GetFd() const { return m_fd; }

    /**
     * @brief Get the client address. 
     *
     * @return The client address.
     */
    const char *GetAddr() const { return m_addr; }

    /**
     * @brief Set the client address. 
     *
     * @param szAddr [IN] The client address.
     */
	void SetAddr(const char *szAddr);

    /**
     * @brief Read data into buffer.
     *
     * @return true: No error.<br>
     *        false: Error.
     */
    virtual bool ReadToBuffer();

    /**
     * @brief Get the reference of the buffer.
     *
     * @return The reference of the buffer .
     */
    DataQueue &GetDataQueue() { return m_dataQ; }

protected:
    /**
     * @brief The flag that indicates whether the connection is valid.
     */
	bool m_isValid;

    /**
     * @brief The socket file descriptor.
     */
    int m_fd;

    /**
     * @brief The client address.
     */
    char m_addr[MAX_SOCK_ADDR_LEN];

    /**
     * @brief Buffer.
     */
    DataQueue m_dataQ;
};
} //namespace libmdmonitor {

#endif
