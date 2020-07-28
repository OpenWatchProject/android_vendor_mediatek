/*
 * Copyright (c) 2013 Google Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __USR_TRUSTY_IPC_H
#define __USR_TRUSTY_IPC_H

#include <sys/types.h>

/*
 *  handle_t is an opaque 32 bit value that is used to reference an
 *  object (like ipc port or channel) in kernel space
 */
typedef uint32_t handle_t;

/*
 *  Invalid IPC handle
 */
#define INVALID_IPC_HANDLE  (0xFFFFFFFFu)

/*
 * Combination of these flags sets additional options
 * for port_create syscall.
 */
enum {
	/* allow Trusted Apps to connect to this port */
	IPC_PORT_ALLOW_TA_CONNECT = 0x1,
	/* allow non-secure clients to connect to this port */
	IPC_PORT_ALLOW_NS_CONNECT = 0x2,
};

/*
 * Options for connect syscall
 */
enum {
	IPC_CONNECT_WAIT_FOR_PORT = 0x1,
	IPC_CONNECT_ASYNC = 0x2,
};

/*
 *  IPC message
 */
typedef struct iovec {
	void		*base;
	size_t		len;
} iovec_t;

typedef struct ipc_msg {
	uint		num_iov;
	iovec_t		*iov;

	uint		num_handles;
	handle_t	*handles;
} ipc_msg_t;

typedef struct ipc_msg_info {
	size_t		len;
	uint32_t	id;
} ipc_msg_info_t;

/*
 *  Combination of these values is used for event field
 *  ot uevent_t structure.
 */
enum {
	IPC_HANDLE_POLL_NONE	= 0x0,
	IPC_HANDLE_POLL_READY	= 0x1,
	IPC_HANDLE_POLL_ERROR	= 0x2,
	IPC_HANDLE_POLL_HUP	= 0x4,
	IPC_HANDLE_POLL_MSG	= 0x8,
	IPC_HANDLE_POLL_SEND_UNBLOCKED = 0x10,
};

/*
 *  Is used by wait and wait_any calls to return information
 *  about event.
 */
typedef struct uevent {
	uint32_t	handle;  /* handle this event is related too */
	uint32_t	event;   /* combination of IPC_HANDLE_POLL_XXX flags */
	void		*cookie; /* cookie aasociated with handle */
} uevent_t;

#endif
