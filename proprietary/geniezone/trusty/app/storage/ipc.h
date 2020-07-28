/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdio.h>

#include <trusty_ipc.h>
#include <trusty_uuid.h>

struct ipc_context;
struct ipc_port_context;
struct ipc_channel_context;

/**
 * ipc_connect_handler_t - handler for connect events
 * @parent:      the parent port context of the channel to be created
 * @peer_uuid:   the UUID of the connecting peer
 * @chan_handle: the handle for the connecting channel
 *
 * Returns a reference to an ipc_channel_context with its channel
 * operations initialized.
 */
typedef struct ipc_channel_context *(*ipc_connect_handler_t)(struct ipc_port_context *parent,
                                                             const uuid_t *peer_uuid,
                                                             handle_t chan_handle);

/**
 * ipc_msg_handler_t - handler for msg events
 * @context:  the channel context returned from ipc_connect_handler_t
 * @msg:      the received msg buffer
 * @msg_size: the received msg buffer size
 *
 * Returns NO_ERROR on success, error code < 0 on failure.
 * In case of error, the channel is disconnected.
 */
typedef int (*ipc_msg_handler_t)(struct ipc_channel_context *context,
                                 void *msg, size_t msg_size);

/**
 * ipc_disconnect_handler_t - handler for disconnect events
 * @context: the context to be freed
 *
 * This function must not close the channel handle.
 */
typedef void (*ipc_disconnect_handler_t)(struct ipc_channel_context *context);

typedef void (*ipc_evt_handler_t) (struct ipc_context *context, const struct uevent *ev);

/**
 * ipc_port_ops
 * @on_connect: required connect handler
 */
struct ipc_port_ops {
	ipc_connect_handler_t        on_connect;
};

/**
 * ipc_channel_ops
 * @on_handle_msg: optional msg handler
 * @on_disconnect: required disconnect handler
 */
struct ipc_channel_ops {
	ipc_msg_handler_t            on_handle_msg;
	ipc_disconnect_handler_t     on_disconnect;
};

struct ipc_context {
	ipc_evt_handler_t evt_handler;
	handle_t handle;
};

struct ipc_channel_context {
	struct ipc_context common;
	struct ipc_channel_ops ops;
};

struct ipc_port_context {
	struct ipc_context common;
	struct ipc_port_ops ops;
};

/**
 * sync_ipc_send_msg - send IPC message
 * @session:        the session handle
 * @tx_iovecs:      the buffers to send
 * @tx_iovec_count: the count of buffers to send
 * @rx_iovecs:      the buffers to receive
 * @rx_iovec_count: the count of buffers to receive
 */
int sync_ipc_send_msg(handle_t session, iovec_t *tx_iovecs, uint tx_iovec_count,
                      iovec_t *rx_iovecs, uint rx_iovec_count);

int ipc_port_create(struct ipc_port_context *contextp, const char *port_name,
                    size_t queue_size, size_t max_buffer_size, uint32_t flags);
int ipc_port_destroy(struct ipc_port_context *context);
void ipc_loop(void);

