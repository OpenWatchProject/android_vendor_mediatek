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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <err.h>
#include <trusty_std.h>
#include <stdio.h>

#include <lib/hwkey/hwkey.h>

#define LOG_TAG "libhwkey"
#define TLOGE(fmt, ...) \
	    fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

/**
 * long hwkey_err_to_tipc_err() - translates hwkey err value to tipc/lk err value
 * @hwkey_err: hwkey err value
 *
 * Returns: enum hwkey_err value
 */
static long hwkey_err_to_tipc_err(enum hwkey_err hwkey_err)
{
	switch(hwkey_err) {
		case HWKEY_NO_ERROR:
			return NO_ERROR;
		case HWKEY_ERR_NOT_VALID:
			return ERR_NOT_VALID;
		case HWKEY_ERR_BAD_LEN:
			return ERR_BAD_LEN;
		case HWKEY_ERR_NOT_IMPLEMENTED:
			return ERR_NOT_IMPLEMENTED;
		case HWKEY_ERR_NOT_FOUND:
			return ERR_NOT_FOUND;
		default:
			return ERR_GENERIC;
	}
}

/**
 * long send_req() - sends request to hwkey server
 * @session: the hwkey session handle
 * @msg: the request header to send to the hwkey server
 * @req_buf: the request payload to send to the hwkey server
 * @req_buf_len: the length of the request payload @req_buf
 * @rsp_buf: buffer in which to store the response payload
 * @rsp_buf_len: the size of the response buffer. Inout param, set
 *               to the actual response payload length.
 *
 * Returns: NO_ERROR on success, negative error code on failure
 */
static long send_req(hwkey_session_t session, struct hwkey_msg *msg, uint8_t *req_buf,
                     uint32_t req_buf_len, uint8_t *rsp_buf, uint32_t *rsp_buf_len)
{
	long rc;

	iovec_t tx_iov[2] = {
		{
			.base = msg,
			.len = sizeof(*msg),
		},
		{
			.base = req_buf,
			.len = req_buf_len,
		},
	};
	ipc_msg_t tx_msg = {
		.iov = tx_iov,
		.num_iov = 2,
	};

	rc = send_msg(session, &tx_msg);
	if (rc < 0) {
		goto err_send_fail;
	}

	if(((size_t) rc) != sizeof(*msg) + req_buf_len) {
		rc = ERR_IO;
		goto err_send_fail;
	}

	uevent_t uevt;
	rc = wait(session, &uevt, -1);
	if (rc != NO_ERROR) {
		goto err_send_fail;
	}

	ipc_msg_info_t inf;
	rc = get_msg(session, &inf);
	if (rc != NO_ERROR) {
		TLOGE("%s: failed to get_msg (%d)\n", __func__, rc);
		goto err_send_fail;
	}

	if (inf.len > sizeof(*msg) + *rsp_buf_len) {
		TLOGE("%s: insufficient output buffer size (%d > %d)\n", __func__,
				inf.len - sizeof(*msg), *rsp_buf_len);
		rc = ERR_TOO_BIG;
		goto err_get_fail;
	}

	if (inf.len < sizeof(*msg)) {
		TLOGE("%s: short buffer (%d)\n", __func__,
				inf.len);
		rc = ERR_NOT_VALID;
		goto err_get_fail;
	}

	uint32_t cmd_sent = msg->cmd;

	iovec_t rx_iov[2] = {
		{
			.base = msg,
			.len = sizeof(*msg)
		},
		{
			.base = rsp_buf,
			.len = *rsp_buf_len
		}
	};
	ipc_msg_t rx_msg = {
		.iov = rx_iov,
		.num_iov = 2,
	};

	rc = read_msg(session, inf.id, 0, &rx_msg);
	put_msg(session, inf.id);
	if (rc < 0) {
		goto err_read_fail;
	}

	size_t read_len = (size_t) rc;
	if (read_len != inf.len) {
		// data read in does not match message length
		TLOGE("invalid response length: %d\n", rc);
		rc = ERR_IO;
		goto err_read_fail;
	}

	if (msg->cmd != (cmd_sent | HWKEY_RESP_BIT)) {
		TLOGE("invalid response id (%d) for cmd (%d)\n", msg->cmd, cmd_sent);
		return ERR_NOT_VALID;
	}

	*rsp_buf_len = read_len - sizeof(*msg);
	return hwkey_err_to_tipc_err(msg->status);

err_get_fail:
	put_msg(session, inf.id);
err_send_fail:
err_read_fail:
	TLOGE("%s: failed read_msg (%d)", __func__, rc);
	return rc;
}

long hwkey_open(void)
{
	return connect(HWKEY_PORT, IPC_CONNECT_WAIT_FOR_PORT);
}

long hwkey_get_keyslot_data(hwkey_session_t session, const char *slot_id,
                            uint8_t *data, uint32_t *data_size)
{
	if (slot_id == NULL || data == NULL || data_size == NULL || *data_size == 0) {
		return ERR_NOT_VALID;
	}

	struct hwkey_msg msg = {
		.cmd = HWKEY_GET_KEYSLOT,
	};

	// TODO: remove const cast when const APIs are available
	return send_req(session, &msg, (uint8_t *) slot_id, strlen(slot_id), data, data_size);
}

long hwkey_derive(hwkey_session_t session, uint32_t *kdf_version, const uint8_t *src,
                  uint8_t *dest, uint32_t buf_size)
{
	if (src == NULL || buf_size == 0 || dest == NULL || kdf_version == NULL) {
		// invalid input
		return ERR_NOT_VALID;
	}

	struct hwkey_msg msg = {
		.cmd = HWKEY_DERIVE,
		.arg1 = *kdf_version,
	};

	// TODO: remove const cast when const APIs are available
	uint32_t stored_buf_size = buf_size;
	long rc = send_req(session, &msg, (uint8_t *) src, buf_size, dest, &buf_size);

	if (rc == NO_ERROR && stored_buf_size != buf_size) {
		return ERR_BAD_LEN;
	}

	*kdf_version = msg.arg1;

	return rc;
}

void hwkey_close(hwkey_session_t session)
{
	close(session);
}

