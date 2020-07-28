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

#include <err.h>
#include <trusty_std.h>

#include <stdio.h>
#include <stdlib.h>

#include <lib/keymaster/keymaster.h>
#include <interface/keymaster/keymaster.h>

#define LOG_TAG "libkeymaster"
#define TLOGE(fmt, ...) \
	    fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

static long send_req(keymaster_session_t session, enum secure_keymaster_command cmd)
{
	struct keymaster_message msg = {
		.cmd = cmd,
	};

	iovec_t tx_iov = {
		.base = &msg,
		.len = sizeof(msg),
	};
	ipc_msg_t tx_msg = {
		.iov = &tx_iov,
		.num_iov = 1,
	};

	long rc = send_msg(session, &tx_msg);
	if (rc < 0) {
		TLOGE("%s: failed (%d) to send_msg\n", __func__, rc);
		return rc;
	}

	if(((size_t) rc) != sizeof(msg)) {
		TLOGE("%s: msg invalid size (%d != %d)", __func__, rc, sizeof(msg));
		return ERR_IO;
	}

	return NO_ERROR;
}

static long await_response(keymaster_session_t session, struct ipc_msg_info *inf)
{
	uevent_t uevt;
	long rc = wait(session, &uevt, -1);
	if (rc != NO_ERROR) {
		TLOGE("%s: interrupted waiting for response", __func__, rc);
		return rc;
	}

	rc = get_msg(session, inf);
	if (rc != NO_ERROR) {
		TLOGE("%s: failed to get_msg (%d)\n", __func__, rc);
	}

	return rc;
}

static long read_response(keymaster_session_t session, uint32_t msg_id,
                          uint32_t cmd, uint8_t *buf, uint32_t size)
{
	struct keymaster_message msg;

	iovec_t rx_iov[2] = {
		{
			.base = &msg,
			.len = sizeof(msg)
		},
		{
			.base = buf,
			.len = size
		}
	};
	struct ipc_msg rx_msg = {
		.iov = rx_iov,
		.num_iov = 2,
	};

	long rc = read_msg(session, msg_id, 0, &rx_msg);
	put_msg(session, msg_id);

	if (msg.cmd != (cmd | KM_RESP_BIT)) {
		TLOGE("%s: invalid response (%d) for cmd (%d)\n", __func__,
				msg.cmd, cmd);
		return ERR_NOT_VALID;
	}

	return rc;
}


int keymaster_open(void)
{
	return connect(KEYMASTER_SECURE_PORT, IPC_CONNECT_WAIT_FOR_PORT);
}

void keymaster_close(keymaster_session_t session)
{
	close(session);
}

int keymaster_get_auth_token_key(keymaster_session_t session,
                                 uint8_t** key_buf_p, size_t* size_p)
{
	if (size_p == NULL || key_buf_p == NULL) {
		return ERR_NOT_VALID;
	}

	long rc = send_req(session, KM_GET_AUTH_TOKEN_KEY);
	if (rc < 0) {
		TLOGE("%s: failed to send req\n", __func__, rc);
		return rc;
	}

	struct ipc_msg_info inf;
	rc = await_response(session, &inf);
	if (rc < 0) {
		TLOGE("%s: failed (%d) to await response\n", __func__, rc);
		return rc;
	}

	if (inf.len <= sizeof(struct keymaster_message)) {
		TLOGE("%s: auth token key not returned (%d)\n", __func__, inf.len);
		put_msg(session, inf.id);
		return ERR_NOT_FOUND;
	}

	size_t size = inf.len - sizeof(struct keymaster_message);
	uint8_t *key_buf = malloc(size);
	if (key_buf == NULL) {
		TLOGE("%s: out of memory\n", __func__, inf.len);
		put_msg(session, inf.id);
		return ERR_NO_MEMORY;
	}

	rc = read_response(session, inf.id, KM_GET_AUTH_TOKEN_KEY,
                       key_buf, size);
	if (rc < 0) {
		goto err_bad_read;
	}

	size_t read_len = (size_t) rc;
	if (read_len != inf.len){
		// data read in does not match message length
		TLOGE("%s: invalid response length: %d\n", __func__, rc);
		rc = ERR_IO;
		goto err_bad_read;
	}

	*size_p = size;
	*key_buf_p = key_buf;
	return NO_ERROR;

err_bad_read:
	free(key_buf);
	TLOGE("%s: failed read_msg (%d)", __func__, rc);
	return rc;
}

