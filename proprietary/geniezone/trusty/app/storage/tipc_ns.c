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
#include <compiler.h>
#include <stdint.h>
#include <string.h>
#include <trusty_ipc.h>

#include <interface/storage/storage.h>

#include "ipc.h"
#include "rpmb.h"
#include "tipc_ns.h"

#define SS_ERR(args...)  fprintf(stderr, "ss: " args)
#define SS_WARN(args...)  fprintf(stderr, "ss: " args)
#define SS_DBG_IO(args...)

static inline int translate_error(enum storage_err err) {
	switch (err) {
	case STORAGE_NO_ERROR:
		return NO_ERROR;
	case STORAGE_ERR_NOT_VALID:
		return ERR_NOT_VALID;
	case STORAGE_ERR_GENERIC:
	default:
		return ERR_GENERIC;
	}
}

static inline int check_response(enum storage_cmd cmd, struct storage_msg *msg,
                                 size_t read_len)
{
	if (read_len < sizeof(*msg)) {
		SS_ERR("%s: invalid response size %d\n", __func__, read_len);
		return ERR_IO;
	}

	if ((enum storage_cmd)msg->cmd != (cmd | STORAGE_RESP_BIT)) {
		SS_ERR("%s: invalid response (%d) for cmd (%d)\n", __func__,
		       msg->cmd, cmd);
		return ERR_IO;
	}

	return translate_error(msg->result);
}

int rpmb_send(void *handle_,
              void *reliable_write_buf, size_t reliable_write_size,
              void *write_buf, size_t write_size,
              void *read_buf, size_t read_size, bool sync)
{
	SS_DBG_IO("%s: handle %p, rel_write size %zu, write size %zu, read size %zu\n",
		  __func__, handle_, reliable_write_size, write_size, read_size);

	int rc;
	handle_t *handlep = handle_;
	handle_t handle = *handlep;

	struct storage_rpmb_send_req req = {
		.read_size = read_size,
		.reliable_write_size = reliable_write_size,
		.write_size = write_size,
	};

	struct storage_msg msg = {
		.cmd = STORAGE_RPMB_SEND,
		.flags = sync ? (STORAGE_MSG_FLAG_PRE_COMMIT | STORAGE_MSG_FLAG_POST_COMMIT) : 0,
		.size = sizeof(msg) + sizeof(req) + reliable_write_size + write_size,
	};


	iovec_t rx_iov[] = {
		{
			.base = &msg,
			.len = sizeof(msg),
		},
		{
			.base = read_buf,
			.len = read_size,
		},
	};


	iovec_t tx_iov[] = {
		{
			.base = &msg,
			.len = sizeof(msg),
		},
		{
			.base = &req,
			.len = sizeof(req),
		},
		{
			.base = reliable_write_buf,
			.len = reliable_write_size,
		},
		{
			.base = write_buf,
			.len = write_size
		}
	};

    //SS_ERR("tx_iov[0].base = %p, .len = %zu\n", tx_iov[0].base, tx_iov[0].len);
    //SS_ERR("tx_iov[1].base = %p, .len = %zu\n", tx_iov[1].base, tx_iov[1].len);
    //SS_ERR("tx_iov[2].base = %p, .len = %zu\n", tx_iov[2].base, tx_iov[2].len);
    //SS_ERR("tx_iov[3].base = %p, .len = %zu\n", tx_iov[3].base, tx_iov[3].len);
    //SS_ERR("countof(tx_iov) = %d\n", countof(tx_iov));

	rc = sync_ipc_send_msg(handle, tx_iov, countof(tx_iov),
	                       rx_iov, countof(rx_iov));
	if (rc < 0) {
		SS_ERR("%s: rpmb send failed, ipc failure: %d\n", __func__, rc);
		return rc;
	}

	return check_response(STORAGE_RPMB_SEND, &msg, rc);
}

int ns_open_file(handle_t ipc_handle, const char *fname,
                 ns_handle_t *handlep, bool create)
{
	SS_DBG_IO("%s: open %s create: %d\n", __func__, fname, create);

	size_t fname_size = strlen(fname);
	struct storage_file_open_resp resp;

	struct storage_file_open_req req = {
		.flags = create ? STORAGE_FILE_OPEN_CREATE :  0,
	};

	struct storage_msg msg = {
		.cmd = STORAGE_FILE_OPEN,
		.size = sizeof(msg) + sizeof(req) + fname_size,
	};

	iovec_t tx_iov[] = {
		{
			.base = &msg,
			.len = sizeof(msg),
		},
		{
			.base = &req,
			.len = sizeof(req),
		},
		{
			// TODO: change to const API when available
			.base = (char *) fname,
			.len = fname_size,
		}
	};

	iovec_t rx_iov[] = {
		{
			.base = &msg,
			.len = sizeof(msg),
		},
		{
			.base = &resp,
			.len = sizeof(resp),
		}
	};


	int rc = sync_ipc_send_msg(ipc_handle, tx_iov, countof(tx_iov),
	                           rx_iov, countof(rx_iov));
	if (rc < 0) {
		SS_ERR("%s: open failed, %d\n", __func__, rc);
		return rc;
	}

	size_t bytes_read = (size_t) rc;

	rc = check_response(STORAGE_FILE_OPEN, &msg, bytes_read);
	if (rc != NO_ERROR) {
		return rc;
	}

	if (bytes_read != sizeof(msg) + sizeof(resp)) {
		SS_ERR("%s: open failed, invalid response size (%d !=  %d)\n",
		       __func__, bytes_read, sizeof(resp));
		return ERR_NOT_VALID;
	}

	*handlep = resp.handle;

	return NO_ERROR;
}

void ns_close_file(handle_t ipc_handle, ns_handle_t handle)
{
	SS_DBG_IO("close handle: %llu\n", handle);
	struct storage_file_close_req req = {
		.handle = handle,
	};

	struct storage_msg msg = {
		.cmd = STORAGE_FILE_CLOSE,
		.size = sizeof(msg) + sizeof(req),
	};

	iovec_t iov[] = {
		{
			.base = &msg,
			.len = sizeof(msg),
		},
		{
			.base = &req,
			.len = sizeof(req),
		},
	};

	int rc = sync_ipc_send_msg(ipc_handle, iov, countof(iov), iov, 1);
	if (rc < 0) {
		SS_ERR("%s: close failed, %d\n", __func__, rc);
		return;
	}

	rc = check_response(STORAGE_FILE_CLOSE, &msg, rc);
	if (rc < 0) {
		SS_ERR("%s: close failed, %d\n", __func__, rc);
	}
}

int ns_read_pos(handle_t ipc_handle, ns_handle_t handle, ns_off_t pos, void *data, int data_size)
{
	SS_DBG_IO("%s: handle %llu, pos %llu, size %d\n",
		  __func__, handle, pos, data_size);

	struct storage_file_read_req req = {
		.handle = handle,
		.offset = pos,
		.size = data_size,
	};

	struct storage_msg msg = {
		.cmd = STORAGE_FILE_READ,
		.size = sizeof(msg) + sizeof(req),
	};

	iovec_t tx_iov[] = {
		{
			.base = &msg,
			.len = sizeof(msg),
		},
		{
			.base = &req,
			.len = sizeof(req),
		},
	};

	iovec_t rx_iov[] = {
		{
			.base = &msg,
			.len = sizeof(msg),
		},
		{
			.base = data,
			.len = data_size,
		}
	};

	int rc = sync_ipc_send_msg(ipc_handle, tx_iov, countof(tx_iov),
	                           rx_iov, countof(rx_iov));
	if (rc < 0) {
		SS_ERR("%s: read failed, %d\n", __func__, rc);
		return rc;
	}

	if ((size_t) rc != sizeof(msg) + data_size) {
		return ERR_NOT_VALID;
	}

	size_t data_len = rc;

	rc = check_response(STORAGE_FILE_READ, &msg, data_len);
	if (rc != NO_ERROR) {
		return rc;
	}

	return data_len - sizeof(msg);
}

int ns_write_pos(handle_t ipc_handle, ns_handle_t handle, ns_off_t pos,
                 const void *data, int data_size)
{
	SS_DBG_IO("%s: handle %llu, pos %llu, size %d\n",
		  __func__, handle, pos, data_size);

	struct storage_file_write_req req = {
		.handle = handle,
		.offset = pos,
	};

	struct storage_msg msg = {
		.cmd = STORAGE_FILE_WRITE,
		.size = sizeof(msg) + sizeof(req) + data_size,
	};

	iovec_t iov[] = {
		{
			.base = &msg,
			.len = sizeof(msg),
		},
		{
			.base = &req,
			.len = sizeof(req),
		},
		{
			// TODO: use const API
			.base = (void *) data,
			.len = data_size,
		}
	};

	int rc = sync_ipc_send_msg(ipc_handle, iov, countof(iov), iov, 1);
	if (rc < 0) {
		SS_ERR("%s: write failed, %d\n", __func__, rc);
		return rc;
	}

	rc = check_response(STORAGE_FILE_WRITE, &msg, rc);
	if (rc != NO_ERROR) {
		return rc;
	}

	return data_size;
}
