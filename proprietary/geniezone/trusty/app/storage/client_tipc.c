/*
 * Copyright (C) 2013-2016 The Android Open Source Project
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

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <list.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trusty_std.h>

#include <openssl/mem.h>

#include <interface/storage/storage.h>

#include "client_tipc.h"
#include "client_session_tipc.h"
#include "file.h"
#include "ipc.h"
#include "session.h"
#include "tipc_limits.h"

#include "rpmb.h"
#include "block_device_tipc.h"

/* macros to help manage debug output */
#define SS_ERR(args...)		fprintf(stderr, "ss: " args)
#define SS_DBG_IO(args...)	do {} while(0)

#if 0
/* this can generate alot of spew on debug builds */
#define SS_INFO(args...)	fprintf(stderr, "ss: " args)
#else
#define SS_INFO(args...) do {} while(0)
#endif

static int client_handle_msg(struct ipc_channel_context *ctx, void *msg, size_t msg_size);
static void client_disconnect(struct ipc_channel_context *context);
static int send_response(struct storage_client_session *session,
                         enum storage_err result, struct storage_msg *msg,
                         void *out, size_t out_size);

/*
 * Legal secure storage directory and file names contain only
 * characters from the following set: [a-z][A-Z][0-9][.-_]
 *
 * It is not null terminated.
 */
static int is_valid_name(const char *name, size_t name_len)
{
	size_t i;

	if (!name_len)
		return 0;

	for (i = 0; i < name_len; i++) {

		if ((name[i] >= 'a') && (name[i] <= 'z'))
			continue;
		if ((name[i] >= 'A') && (name[i] <= 'Z'))
			continue;
		if ((name[i] >= '0') && (name[i] <= '9'))
			continue;
		if ((name[i] == '.') || (name[i] == '-') || (name[i] == '_'))
			continue;

		/* not a legal character so reject this name */
		return 0;
	}

	return 1;
}

static int get_path(char *path_out, size_t path_out_size,
                    const uuid_t *uuid,
                    const char *file_name, size_t file_name_len)
{
	unsigned int rc;

	rc = snprintf(path_out, path_out_size,
		      "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x/",
		      uuid->time_low,
		      uuid->time_mid,
		      uuid->time_hi_and_version,
		      uuid->clock_seq_and_node[0],
		      uuid->clock_seq_and_node[1],
		      uuid->clock_seq_and_node[2],
		      uuid->clock_seq_and_node[3],
		      uuid->clock_seq_and_node[4],
		      uuid->clock_seq_and_node[5],
		      uuid->clock_seq_and_node[6],
		      uuid->clock_seq_and_node[7]);

	if (rc + file_name_len >= path_out_size) {
		return STORAGE_ERR_NOT_VALID;
	}

	memcpy(path_out + rc, file_name, file_name_len);
	path_out[rc + file_name_len] = '\0';

	return STORAGE_NO_ERROR;
}

static enum storage_err session_set_files_count(struct storage_client_session *session,
                                                size_t files_count)
{
	struct file_handle **files;

	if (files_count > STORAGE_MAX_OPEN_FILES) {
		SS_ERR("%s: too many open files\n", __func__);
		return STORAGE_ERR_NOT_VALID;
	}

	files = realloc(session->files, sizeof(files[0]) * files_count);
	if (!files) {
		SS_ERR("%s: out of memory\n", __func__);
		return STORAGE_ERR_GENERIC;
	}
	if (files_count > session->files_count)
		memset(files + session->files_count, 0,
		       sizeof(files[0]) * (files_count - session->files_count));

	session->files = files;
	session->files_count = files_count;

	SS_INFO("%s: new file table size, 0x%x\n", __func__, files_count);

	return STORAGE_NO_ERROR;
}

static void session_shrink_files(struct storage_client_session *session)
{
	uint32_t handle;

	handle = session->files_count;
	while (handle > 0 && !session->files[handle - 1])
		handle--;

	if (handle < session->files_count)
		session_set_files_count(session, handle);
}

static void session_close_all_files(struct storage_client_session *session)
{
	uint32_t f_handle;
	struct file_handle *file;

	for (f_handle = 0; f_handle < session->files_count; f_handle++) {
		file = session->files[f_handle];
		if (file) {
			file_close(file);
			free(file);
		}
	}
	if (session->files) {
		free(session->files);
	}
	session->files_count = 0;
}

static enum storage_err create_file_handle(struct storage_client_session *session,
                                           uint32_t *handlep,
                                           struct file_handle **file_p)
{
	enum storage_err result;
	uint32_t handle;
	struct file_handle *file;

	for (handle = 0; handle < session->files_count; handle++)
		if (!session->files[handle])
			break;

	if (handle >= session->files_count) {
		result = session_set_files_count(session, handle + 1);
		if (result != STORAGE_NO_ERROR)
			return result;
	}

	file = calloc(1, sizeof(*file));
	if (!file) {
		SS_ERR("%s: out of memory\n", __func__);
		return STORAGE_ERR_GENERIC;
	}

	session->files[handle] = file;

	SS_INFO("%s: created file handle 0x%x\n", __func__, handle);
	*handlep = handle;
	*file_p = file;
	return STORAGE_NO_ERROR;
}

static void free_file_handle(struct storage_client_session *session, uint32_t handle)
{
	if (handle >= session->files_count) {
		SS_ERR("%s: invalid handle, 0x%x\n", __func__, handle);
		return;
	}
	if (session->files[handle] == NULL) {
		SS_ERR("%s: closed handle, 0x%x\n", __func__, handle);
		return;
	}
	free(session->files[handle]);
	session->files[handle] = NULL;

	SS_INFO("%s: deleted file handle 0x%x\n", __func__, handle);

	session_shrink_files(session);
}

static struct file_handle *get_file_handle(struct storage_client_session *session,
                                       uint32_t handle)
{
	struct file_handle *file;
	if (handle >= session->files_count) {
		SS_ERR("%s: invalid handle, 0x%x\n", __func__, handle);
		return NULL;
	}
	file = session->files[handle];
	if (!file) {
		SS_ERR("%s: closed handle, 0x%x\n", __func__, handle);
		return NULL;
	}
	return file;
}

static enum storage_err storage_file_delete(struct storage_msg *msg,
                                            struct storage_file_delete_req *req, size_t req_size,
                                            struct storage_client_session *session)
{
	bool deleted;
	enum storage_err result;
	const char *fname;
	size_t fname_len;
	uint32_t flags;
	char path_buf[FS_PATH_MAX];

	if (req_size < sizeof(*req)) {
		SS_ERR("%s: invalid request size (%zd)\n", __func__, req_size);
		return STORAGE_ERR_NOT_VALID;
	}

	flags = req->flags;
	if ((flags & ~STORAGE_FILE_DELETE_MASK) != 0) {
		SS_ERR("invalid delete flags 0x%x\n", flags);
		return STORAGE_ERR_NOT_VALID;
	}

	/* make sure filename is legal */
	fname = req->name;
	fname_len = req_size - sizeof(*req);
	if (!is_valid_name(fname, fname_len)) {
		SS_ERR("%s: invalid filename\n", __func__);
		return STORAGE_ERR_NOT_VALID;
	}

	result = get_path(path_buf, sizeof(path_buf), &session->uuid, fname, fname_len);
	if (result != STORAGE_NO_ERROR) {
		return result;
	}

	SS_INFO("%s: path %s\n", __func__, path_buf);

	deleted = file_delete(&session->tr, path_buf);

	if (session->tr.failed) {
		SS_ERR("%s: transaction failed\n", __func__);
		return STORAGE_ERR_GENERIC;
	} else if (!deleted) {
		return STORAGE_ERR_NOT_FOUND;
	}

	if (msg->flags & STORAGE_MSG_FLAG_TRANSACT_COMPLETE) {
		transaction_complete(&session->tr);
		if (session->tr.failed) {
			SS_ERR("%s: transaction commit failed\n", __func__);
			return STORAGE_ERR_GENERIC;
		}
		return STORAGE_NO_ERROR;
	}

	return STORAGE_NO_ERROR;
}

static int storage_file_open(struct storage_msg *msg,
                             struct storage_file_open_req *req, size_t req_size,
                             struct storage_client_session *session)

{
	bool found;
	enum storage_err result;
	struct file_handle *file = NULL;
	const char *fname;
	size_t fname_len;
	uint32_t flags, f_handle;
	char path_buf[FS_PATH_MAX];
	void *out = NULL;
	size_t out_size = 0;
	enum file_create_mode file_create_mode;

	if (req_size < sizeof(*req)) {
		SS_ERR("%s: invalid request size (%zd)\n", __func__, req_size);
		result = STORAGE_ERR_NOT_VALID;
		goto err_invalid_size;
	}

	flags = req->flags;
	if ((flags & ~STORAGE_FILE_OPEN_MASK) != 0) {
		SS_ERR("%s: invalid flags 0x%x\n", __func__, flags);
		result = STORAGE_ERR_NOT_VALID;
		goto err_invalid_mask;
	}

	/* make sure filename is legal */
	fname = req->name;
	fname_len = req_size - sizeof(*req);
	if (!is_valid_name(fname, fname_len)) {
		SS_ERR("%s: invalid filename\n", __func__);
		result = STORAGE_ERR_NOT_VALID;
		goto err_invalid_name;
	}

	result = get_path(path_buf, sizeof(path_buf), &session->uuid, fname, fname_len);
	if (result != STORAGE_NO_ERROR) {
		goto err_get_path;
	}

	SS_INFO("%s: path %s flags 0x%x\n", __func__, path_buf, flags);

	SS_INFO("%s: call create_file_handle\n", __func__);
	/* alloc file info struct */
	result = create_file_handle(session, &f_handle, &file);
	if (result != STORAGE_NO_ERROR)
		goto err_create_file_handle;

	if (flags & STORAGE_FILE_OPEN_CREATE) {
		if (flags & STORAGE_FILE_OPEN_CREATE_EXCLUSIVE) {
			file_create_mode = FILE_OPEN_CREATE_EXCLUSIVE;
		} else {
			file_create_mode = FILE_OPEN_CREATE;
		}
	} else {
		file_create_mode = FILE_OPEN_NO_CREATE;
	}

	found = file_open(&session->tr, path_buf, file, file_create_mode);
	if (!found) {
		/* TODO: get more accurate error code from file_open */
		if (session->tr.failed) {
			result = STORAGE_ERR_GENERIC;
		} else if (flags & STORAGE_FILE_OPEN_CREATE) {
			result = STORAGE_ERR_EXIST;
		} else {
			result = STORAGE_ERR_NOT_FOUND;
		}
		goto err_open_file;
	}

	if ((flags & STORAGE_FILE_OPEN_TRUNCATE) && file->size) {
		file_set_size(&session->tr, file, 0);
	}

	if (session->tr.failed) {
		SS_ERR("%s: transaction failed\n", __func__);
		result = STORAGE_ERR_GENERIC;
		goto err_transaction_failed;
	}

	if (msg->flags & STORAGE_MSG_FLAG_TRANSACT_COMPLETE) {
		transaction_complete(&session->tr);
		if (session->tr.failed) {
			SS_ERR("%s: transaction commit failed\n", __func__);
			result = STORAGE_ERR_GENERIC;
			goto err_transaction_failed;
		}
	}

	struct storage_file_open_resp resp = { .handle = f_handle };

	out = &resp;
	out_size = sizeof(resp);

	result = STORAGE_NO_ERROR;
	goto done;

err_transaction_failed:
	file_close(file);
err_open_file:
	free_file_handle(session, f_handle);
err_create_file_handle:
err_get_path:
err_invalid_name:
err_invalid_mask:
err_invalid_size:
done:
	return send_response(session, result, msg, out, out_size);
}

static enum storage_err storage_file_close(struct storage_msg *msg,
                                           struct storage_file_close_req *req, size_t req_size,
                                           struct storage_client_session *session)
{
	struct file_handle *file;

	if (req_size < sizeof(*req)) {
		SS_ERR("%s: invalid request size (%zd)\n", __func__, req_size);
		return STORAGE_ERR_NOT_VALID;
	}

	file = get_file_handle(session, req->handle);
	if (!file)
		return STORAGE_ERR_NOT_VALID;

	file_close(file);

	free_file_handle(session, req->handle);

	if (msg->flags & STORAGE_MSG_FLAG_TRANSACT_COMPLETE) {
		transaction_complete(&session->tr);
		if (session->tr.failed) {
			SS_ERR("%s: transaction commit failed\n", __func__);
			return STORAGE_ERR_GENERIC;
		}
		return STORAGE_NO_ERROR;
	}

	return STORAGE_NO_ERROR;
}

static int storage_file_read(struct storage_msg *msg,
                             struct storage_file_read_req *req, size_t req_size,
                             struct storage_client_session *session)
{
	enum storage_err result = STORAGE_NO_ERROR;
	void *bufp = NULL;
	size_t buflen;
	size_t bytes_left, len;
	uint64_t offset;
	struct file_handle *file;
	void *out = NULL;
	size_t out_size = 0;
	size_t block_size = get_file_block_size(session->tr.fs);
	data_block_t block_num;
	const uint8_t *block_data;
	obj_ref_t block_data_ref = OBJ_REF_INITIAL_VALUE(block_data_ref);
	size_t block_offset;

	if (req_size < sizeof(*req)) {
		SS_ERR("%s: invalid request size (%zd)\n", __func__, req_size);
		result = STORAGE_ERR_NOT_VALID;
		goto err_invalid_input;
	}

	file = get_file_handle(session, req->handle);
	if (!file) {
		SS_ERR("%s: invalid file handle (%d)\n", __func__, req->handle);
		result = STORAGE_ERR_NOT_VALID;
		goto err_invalid_input;
	}

	buflen = req->size;
	if (buflen > STORAGE_MAX_BUFFER_SIZE - sizeof(*msg)) {
		SS_ERR("can't read more than %d bytes, requested %zd\n",
		       STORAGE_MAX_BUFFER_SIZE, buflen);
		result = STORAGE_ERR_NOT_VALID;
		goto err_invalid_input;
	}

	offset = req->offset;
	if (offset > file->size) {
		SS_ERR("can't read past end of file (%lld > %lld)\n",
		       offset, file->size);
		result = STORAGE_ERR_NOT_VALID;
		goto err_invalid_input;
	}

	// reuse the input buffer
	bufp = (uint8_t *)(msg + 1);

	/* calc number of bytes to read */
	if ((offset + buflen) > file->size) {
		bytes_left = (size_t)(file->size - offset);
	} else {
		bytes_left = buflen;
	}
	buflen = bytes_left; /* save to return it to caller */

	SS_INFO("%s: start 0x%x cnt %d\n", __func__, offset, bytes_left);

	result = STORAGE_NO_ERROR;
	while (bytes_left) {
		block_num = offset / block_size;
		block_data = file_get_block(&session->tr, file, block_num,
		                            &block_data_ref);
		if (!block_data) {
			SS_ERR("error reading block %lld\n", block_num);
			result = STORAGE_ERR_GENERIC;
			goto err_get_block;
		}

		block_offset = offset % block_size;
		len = (block_offset + bytes_left > block_size) ?
		      block_size - block_offset : bytes_left;

		memcpy(bufp, block_data + block_offset, len);
		file_block_put(block_data, &block_data_ref);

		bytes_left -= len;
		offset += len;
		bufp += len;
	}

	out = (uint8_t *)(msg + 1);
	out_size = buflen;

err_get_block:
err_invalid_input:
	return send_response(session, result, msg, out, out_size);
}

static enum storage_err storage_file_write(struct storage_msg *msg,
                                           struct storage_file_write_req *req, size_t req_size,
                                           struct storage_client_session *session)
{
	enum storage_err result = STORAGE_NO_ERROR;
	const void *bufp = NULL;
	size_t buflen;
	uint64_t offset, end_offset, bytes_left;
	size_t len;
	struct file_handle *file;
	size_t block_size = get_file_block_size(session->tr.fs);
	data_block_t block_num;
	uint8_t *block_data;
	obj_ref_t block_data_ref = OBJ_REF_INITIAL_VALUE(block_data_ref);
	size_t block_offset;

	if (req_size <= sizeof(*req)) {
		SS_ERR("%s: invalid request size (%zd)\n", __func__, req_size);
		return STORAGE_ERR_NOT_VALID;
	}

	file = get_file_handle(session, req->handle);
	if (!file) {
		SS_ERR("%s: invalid file handle (%d)\n", __func__, req->handle);
		return STORAGE_ERR_NOT_VALID;
	}

	offset = req->offset;
	if (offset > file->size) {
		SS_ERR("%s: can't start writing past end of file (%lld > %lld) \n",
		       __func__, offset, file->size);
		return STORAGE_ERR_NOT_VALID;
	}

	bufp = req->data;
	buflen = req_size - sizeof(*req);

	end_offset = offset + buflen - 1;
	bytes_left = end_offset - offset + 1;

	/* transfer data one ss block at a time */
	while (bytes_left) {
		block_num = offset / block_size;
		block_offset = offset % block_size;
		len = (block_offset + bytes_left > block_size) ?
		      block_size - block_offset : bytes_left;

		block_data = file_get_block_write(&session->tr, file, block_num,
		                                  len != block_size, &block_data_ref);
		if (!block_data) {
			SS_ERR("error getting block %lld\n", block_num);
			result = STORAGE_ERR_GENERIC;
			goto err_write;
		}

		memcpy(block_data + block_offset, bufp, len);
		file_block_put_dirty(&session->tr, file, block_num,
		                     block_data, &block_data_ref);

		SS_INFO("%s: bufp %p offset 0x%llx len 0x%x\n",
			__func__, bufp, offset, len);

		bytes_left -= len;
		offset += len;
		bufp += len;
	}

	if (offset > file->size) {
		file_set_size(&session->tr, file, offset);
	}

	if (session->tr.failed) {
		SS_ERR("%s: transaction failed\n", __func__);
		return STORAGE_ERR_GENERIC;
	}

	if (msg->flags & STORAGE_MSG_FLAG_TRANSACT_COMPLETE) {
		transaction_complete(&session->tr);
		if (session->tr.failed) {
			SS_ERR("%s: transaction commit failed\n", __func__);
			return STORAGE_ERR_GENERIC;
		}
	}

	return STORAGE_NO_ERROR;

err_write:
	if (!session->tr.failed) {
		transaction_fail(&session->tr);
	}
err_transaction_complete:
	return result;
}

static int storage_file_get_size(struct storage_msg *msg,
                                 struct storage_file_get_size_req *req, size_t req_size,
                                 struct storage_client_session *session)
{
	bool valid;
	struct file_handle *file;
	enum storage_err result = STORAGE_NO_ERROR;
	void *out = NULL;
	size_t out_size = 0;

	if (req_size != sizeof(req)) {
		SS_ERR("%s: inavlid request size (%zd)\n", __func__, req_size);
		result = STORAGE_ERR_NOT_VALID;
		goto err_invalid_input;
	}

	file = get_file_handle(session, req->handle);
	if (!file) {
		SS_ERR("%s: invalid file handle (%d)\n", __func__, req->handle);
		result = STORAGE_ERR_NOT_VALID;
		goto err_invalid_input;
	}

	struct storage_file_get_size_resp resp;

	valid = file_get_size(&session->tr, file, &resp.size);
	if (!valid) {
		result = STORAGE_ERR_NOT_VALID;
		goto err_invalid_input;
	}

	out = &resp;
	out_size = sizeof(resp);

err_invalid_input:
	return send_response(session, result, msg, out, out_size);
}

static enum storage_err storage_file_set_size(struct storage_msg *msg,
                                              struct storage_file_set_size_req *req, size_t req_size,
                                              struct storage_client_session *session)
{
	struct file_handle *file;
	uint64_t new_size;

	if (req_size != sizeof(*req)) {
		SS_ERR("%s: inavlid request size (%zd)\n", __func__, req_size);
		return STORAGE_ERR_NOT_VALID;
	}

	new_size = req->size;

	file = get_file_handle(session, req->handle);
	if (!file) {
		SS_ERR("%s: invalid file handle (%d)\n", __func__, req->handle);
		return STORAGE_ERR_NOT_VALID;
	}

	SS_INFO("%s: new size 0x%llx, old size 0x%llx\n",
	        __func__, new_size, file->size);

	/* for now we only support shrinking the file */
	if (new_size > file->size) {
		SS_ERR("%s: bad trunc length 0x%llx\n", __func__, new_size);
		return STORAGE_ERR_NOT_VALID;
	}

	/* check for nop */
	if (new_size != file->size) {
		/* update size */
		file_set_size(&session->tr, file, new_size);
	}

	/* try to commit */
	if (msg->flags & STORAGE_MSG_FLAG_TRANSACT_COMPLETE) {
		transaction_complete(&session->tr);
	}

	if (session->tr.failed) {
		SS_ERR("%s: transaction failed\n", __func__);
		return STORAGE_ERR_GENERIC;
	}

	return STORAGE_NO_ERROR;
}

static enum storage_err storage_rpmb_raw_read(struct storage_msg *msg,
                                    struct storage_rpmb_raw_read_req *req, size_t req_size,
                                    struct storage_client_session *session)
{
    enum storage_err result = STORAGE_NO_ERROR;

    int ret;

    uint32_t blk_req_addr = req->blk_addr,
             blk_req_num  = req->blk_num;

    uint32_t blk_count;

    struct block_device_rpmb *dev_rpmb;

    void  *bufp   = NULL;
    size_t buflen = 0;

    void  *out    = NULL;
    size_t outlen = 0;

    // check request parameter
    if (req_size != sizeof(*req)) {
		SS_ERR("%s: inavlid request size (%zd)\n", __func__, req_size);
		result = STORAGE_ERR_NOT_VALID;
        goto err_invalid_input;
	}

    // check requested RPMB blocks
    dev_rpmb  = dev_rpmb_to_state(session->tr.fs->super_dev);
    blk_count = dev_rpmb->state->rpmb_total_blocks;
    if (blk_req_addr >= blk_count
    ||  blk_count - blk_req_addr < blk_req_num) {
		SS_ERR("%s: inavlid request RPMB blocks (total:%u) base=%u, num=%u\n",
                __func__, blk_count, blk_req_addr, blk_req_num);
        result = STORAGE_ERR_NOT_VALID;
        goto err_invalid_input;
    }

    // check exceed max TIPC buffer
    buflen = blk_req_num * RPMB_BUF_SIZE;
    if (buflen > STORAGE_MAX_BUFFER_SIZE - sizeof(*msg)) {
        SS_ERR("%s: can't read more than %d bytes, requested %zd(%d * %d)\n",
		       __func__, STORAGE_MAX_BUFFER_SIZE, buflen, blk_req_num, RPMB_BUF_SIZE);
        result = STORAGE_ERR_NOT_VALID;
        goto err_invalid_input;
    }

    // read RPMB raw blocks, resue the input buffer
    bufp = (uint8_t*)(msg + 1);
    ret = rpmb_read(dev_rpmb->state->rpmb_state, bufp, blk_req_addr, blk_req_num);
    if (ret != 0) {
        SS_ERR("%s: failed to read RPMB (ret=%d)\n", __func__, ret);
        result = STORAGE_ERR_NOT_VALID;
        goto err_get_rpmb_block;
    }

    out    = (uint8_t*)(msg + 1);
    outlen = buflen;

err_get_rpmb_block:
err_invalid_input:
    return send_response(session, result, msg, out, outlen);
}

static struct storage_client_session *chan_context_to_client_session(struct ipc_channel_context *ctx)
{
	assert(ctx != NULL);
	struct storage_client_session *session;

	session = containerof(ctx, struct storage_client_session, context);
	assert(session->magic == STORAGE_CLIENT_SESSION_MAGIC);
	return session;
}

static struct client_port_context *port_context_to_client_port_context(struct ipc_port_context *context)
{
	assert(context != NULL);

	return containerof(context, struct client_port_context, client_ctx);
}

static void client_channel_ops_init(struct ipc_channel_ops *ops)
{
	ops->on_handle_msg = client_handle_msg;
	ops->on_disconnect = client_disconnect;
}

static struct ipc_channel_context *client_connect(struct ipc_port_context *parent_ctx,
                                                  const uuid_t *peer_uuid,
                                                  handle_t chan_handle)
{
	struct client_port_context *client_port_context;
	struct storage_client_session *client_session;

	client_port_context = port_context_to_client_port_context(parent_ctx);

	client_session = calloc(1, sizeof(*client_session));
	if (client_session == NULL) {
		SS_ERR("out of memory allocating client session\n");
		return NULL;
	}

	client_session->magic = STORAGE_CLIENT_SESSION_MAGIC;

	client_session->files = NULL;
	client_session->files_count = 0;

	transaction_init(&client_session->tr, client_port_context->tr_state,
	                 false);

	/* cache identity information */
	memcpy(&client_session->uuid, peer_uuid, sizeof(*peer_uuid));

	client_channel_ops_init(&client_session->context.ops);
	return &client_session->context;

err_derive_master_key:
err_init_metadata:
	free(client_session);
	return NULL;
}

static void client_disconnect(struct ipc_channel_context *context)
{
	struct storage_client_session *session;

	session = chan_context_to_client_session(context);

	if (list_in_list(&session->tr.allocated.node) && !session->tr.failed) {
		transaction_fail(&session->tr); /* discard partial transaction */
	}
	session_close_all_files(session);
	transaction_free(&session->tr);

	OPENSSL_cleanse(session, sizeof(struct storage_client_session));
	free(session);
}

static int send_response(struct storage_client_session *session,
                         enum storage_err result, struct storage_msg *msg,
                         void *out, size_t out_size)
{
	size_t resp_buf_count = 1;
	if (result == STORAGE_NO_ERROR && out != NULL && out_size != 0) {
		++resp_buf_count;
	}

	iovec_t resp_bufs[resp_buf_count];

	msg->cmd |= STORAGE_RESP_BIT;
	msg->flags = 0;
	msg->size = sizeof(struct storage_msg) + out_size;
	msg->result = result;

	resp_bufs[0].base = msg;
	resp_bufs[0].len = sizeof(struct storage_msg);

	if (resp_buf_count == 2) {
		resp_bufs[1].base = out;
		resp_bufs[1].len = out_size;
	}

	struct ipc_msg resp_ipc_msg = {
		.iov = resp_bufs,
		.num_iov = resp_buf_count,
	};

	return send_msg(session->context.common.handle, &resp_ipc_msg);
}

static int send_result(struct storage_client_session *session,
                       struct storage_msg *msg, enum storage_err result)
{
	return send_response(session, result, msg, NULL, 0);
}

static int client_handle_msg(struct ipc_channel_context *ctx, void *msg_buf, size_t msg_size)
{
	struct storage_client_session *session;
	struct storage_msg *msg = msg_buf;
	size_t payload_len;
	enum storage_err result;
	void *payload;

	session = chan_context_to_client_session(ctx);

	if (msg_size < sizeof(struct storage_msg)) {
		SS_ERR("%s: invalid message of size (%zd)\n", __func__, msg_size);
		struct storage_msg err_msg = {.cmd = STORAGE_RESP_MSG_ERR};
		send_result(session, &err_msg, STORAGE_ERR_NOT_VALID);
		return ERR_NOT_VALID; /* would force to close connection */
	}

	payload_len = msg_size - sizeof(struct storage_msg);
	payload = msg->payload;

	/* abort transaction and clear sticky transaction error */
	if (msg->cmd == STORAGE_END_TRANSACTION) {
		if (msg->flags & STORAGE_MSG_FLAG_TRANSACT_COMPLETE) {
			/* try to complete current transaction */
			if (transaction_is_active(&session->tr)) {
				transaction_complete(&session->tr);
			}
			if (session->tr.failed) {
				SS_ERR("%s: failed to complete transaction\n", __func__);
				/* clear transaction failed state */
				session->tr.failed = false;
				return send_result(session, msg, STORAGE_ERR_TRANSACT);
			}
			return send_result(session, msg, STORAGE_NO_ERROR);
		} else {
			/* discard current transaction */
			if (transaction_is_active(&session->tr)) {
				transaction_fail(&session->tr);
			}
			/* clear transaction failed state */
			session->tr.failed = false;
			return send_result(session, msg, STORAGE_NO_ERROR);
		}
	}

	if (session->tr.failed) {
		if (msg->flags & STORAGE_MSG_FLAG_TRANSACT_COMPLETE) {
			/* last command in current trunsaction: reset failed state and return error */
			session->tr.failed = false;
		}
		return send_result(session, msg, STORAGE_ERR_TRANSACT);
	}

	if (!transaction_is_active(&session->tr)) {
		/* previous transaction complete */
		transaction_activate(&session->tr);
	}

	switch (msg->cmd) {
	case STORAGE_FILE_DELETE:
		result = storage_file_delete(msg, payload, payload_len, session);
		break;
	case STORAGE_FILE_OPEN:
		return storage_file_open(msg, payload, payload_len, session);
	case STORAGE_FILE_CLOSE:
		result = storage_file_close(msg, payload, payload_len, session);
		break;
	case STORAGE_FILE_WRITE:
		result = storage_file_write(msg, payload, payload_len, session);
		break;
	case STORAGE_FILE_READ:
		return storage_file_read(msg, payload, payload_len, session);
	case STORAGE_FILE_GET_SIZE:
		return storage_file_get_size(msg, payload, payload_len, session);
	case STORAGE_FILE_SET_SIZE:
		result = storage_file_set_size(msg, payload, payload_len, session);
		break;
    case STORAGE_RPMB_RAW_READ:
		return storage_rpmb_raw_read(msg, payload, payload_len, session);
	default:
		SS_ERR("%s: unsupported command 0x%x\n", __func__, msg->cmd);
		result = STORAGE_ERR_UNIMPLEMENTED;
		break;
	}

	return send_result(session, msg, result);
}

int client_create_port(struct ipc_port_context *client_ctx,
                       const char *port_name)
{
	int ret;

	/* start accepting client connections */
	client_ctx->ops.on_connect = client_connect;
	ret = ipc_port_create(client_ctx, port_name,
	                      1, STORAGE_MAX_BUFFER_SIZE,
	                      IPC_PORT_ALLOW_TA_CONNECT);
	if (ret < 0) {
		SS_ERR("%s: failure initializing client port (%d)\n", __func__,
		       ret);
		return ret;
	}
	return 0;
}
