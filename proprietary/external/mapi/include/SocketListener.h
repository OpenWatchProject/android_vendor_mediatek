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

#ifndef __SOCKET_LISTENER_H__
#define __SOCKET_LISTENER_H__
#include <pthread.h>
#include <map>
#include <errno.h>
#include <sys/select.h> // for fd_set
#include "SocketConnection.h"

class ConnectionHandlerInterface 
{
public:
    /**
     * @brief Cleanup.
     */
    virtual ~ConnectionHandlerInterface() {}

    /**
     * @brief Derived class should implement this function to process the incoming data from @a connection. 
     *
     * @param connection [IN] The connection with data incoming.
     *
     * @return true: Success.<br>
     *        false: Error.
     */
	virtual bool DataIn(SocketConnection &connection) = 0;
};

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to listen the domain sockets and dispatch incoming to connection handlers.
 */
class SocketListener
{
public:
    /**
     * @brief Create a socket server with the abstract name @a szServerName and listens in the background thread. 
     *
     * @param szServerName [IN] The socket server abstract name string.
     */
    SocketListener(const char *szServerName);

    /**
     * @brief Cleanup.
     */
    virtual ~SocketListener();

    /**
     * @brief The entry function of the background thread to listen sockets.
     *
     * @param obj [IN] The @c SocketListener instance.
     *
     * @return NULL.
     */
    static void *ListenThread(void *obj);

    /**
     * @brief Register the connection handler.
     *
     * @param handlerInstance [IN] The connection handler instance.
     */
	void SetConnectionHandler(ConnectionHandlerInterface *handlerInstance);

protected:
    /**
     * @brief The enum lists the type of conections.
     */
	typedef enum {
		SOCKET_TYPE_CLIENT,
		SOCKET_TYPE_SERVER,
		SOCKET_TYPE_SIZE
	} SOCKET_TYPE;

    /**
     * @brief Add a opened socket.
     *
     * @param type [IN] The socket type.
     * @param fd [IN] The socket file descriptor.
     * @param szAddr [IN] The address of client/server.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
	bool AddSocket(SOCKET_TYPE type, int fd, const char *szAddr);

    /**
     * @brief Remove a socket.
     *
     * @param type [IN] The socket type.
     * @param fd [IN] The socket file descriptor.
     */
	void RemoveSocket(SOCKET_TYPE type, int fd);

    /**
     * @brief Clean all data. 
     */
	void Cleanup();

    /**
     * @brief The major set of file descriptors.
     */
    fd_set m_masterFDs;

    /**
     * @brief The maximum file descriptor.
     */
	int m_maxFD;

    /**
     * @brief A table to map file descriptors to socket connection instances.
     */
	std::map<int, SocketConnection *> m_socket[SOCKET_TYPE_SIZE];

    /**
     * @brief The socket listener thread ID.
     */
    pthread_t m_threadId;

    /**
     * @brief A flag to notify the listen thread @a m_threadId to stop listening.
     */
    volatile bool m_notifyListenThreadStop;

    /**
     * @brief The function will be run in the background thread to listen domain sockets. 
     */
    void Listen();

    /**
     * @brief Setup domain socket server. 
     *
     * @param szServerName [IN] The abstract domain socket name string.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool SetupDomainSocket(const char *szServerName);

    /**
     * @brief The registered connection handler instance.
     */
	ConnectionHandlerInterface *m_connHandlerInstance;
};

#endif
