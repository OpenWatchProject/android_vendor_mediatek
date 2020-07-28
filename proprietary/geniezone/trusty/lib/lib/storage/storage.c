/*
 * Copyright (C) 2015-2016 The Android Open Source Project
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
#include <stdio.h>
#include <string.h>

#include <lib/storage/storage.h>
#include <trusty_std.h>

#define LOCAL_TRACE 0

#define LOG_TAG "storage_client"

#define TLOGE(fmt, ...) \
    fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

#if LOCAL_TRACE
#define TLOGI(fmt, ...) \
    fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)
#else
#define TLOGI(fmt, ...)
#endif

// due to the limitation of TIPC/storageproxyd implementation, which
// use fixed size buffer for communication, the requested blocks of
// RPMB operation is accordingly limited.
#define MAX_RPMB_REQ_BLOCK     2
#define MAX_CHUNK_SIZE      4040

#define RPMB_BLOCK_SIZE 256

static inline file_handle_t make_file_handle(storage_session_t s, uint32_t fid)
{
    return ((uint64_t)s << 32) | fid;
}

static inline storage_session_t _to_session(file_handle_t fh)
{
    return (storage_session_t)(fh >> 32);
}

static inline uint32_t _to_handle(file_handle_t fh)
{
    return (uint32_t) fh;
}

static inline uint32_t _to_msg_flags(uint32_t opflags)
{
    uint32_t msg_flags = 0;

    if (opflags & STORAGE_OP_COMPLETE)
        msg_flags |= STORAGE_MSG_FLAG_TRANSACT_COMPLETE;

    return msg_flags;
}

static ssize_t check_response(struct storage_msg *msg, ssize_t res)
{
    if (res < 0)
        return res;

    if ((size_t)res < sizeof(*msg)) {
        TLOGE("invalid msg length (%zd < %zd)\n", (size_t)res, sizeof(*msg));
        return ERR_IO;
    }

    TLOGI("cmd 0x%x: server returned %u\n", msg->cmd, msg->result);

    switch(msg->result) {
    case STORAGE_NO_ERROR:
        return res - sizeof(*msg);

    case STORAGE_ERR_NOT_FOUND:
        return ERR_NOT_FOUND;

    case STORAGE_ERR_EXIST:
        return ERR_ALREADY_EXISTS;

    case STORAGE_ERR_NOT_VALID:
        return ERR_NOT_VALID;

    case STORAGE_ERR_TRANSACT:
        return ERR_BUSY;

    case STORAGE_ERR_ACCESS:
        return ERR_ACCESS_DENIED;

    case STORAGE_ERR_UNIMPLEMENTED:
        TLOGE("cmd 0x%x: is unhandles command\n", msg->cmd);
        return ERR_NOT_IMPLEMENTED;

    case STORAGE_ERR_GENERIC:
        TLOGE("cmd 0x%x: internal server error\n", msg->cmd);
        return ERR_GENERIC;

    default:
        TLOGE("cmd 0x%x: unhandled server response %u\n",
              msg->cmd, msg->result);
    }

    return ERR_IO;
}

static ssize_t get_response(storage_session_t session,
                            struct iovec *rx_iovs, uint rx_iovcnt)

{
    uevent_t ev;
    struct ipc_msg_info mi;
    struct ipc_msg rx_msg = {
        .iov = rx_iovs,
        .num_iov = rx_iovcnt,
    };

    if (!rx_iovcnt)
        return 0;

    /* wait for reply */
    int rc = wait(session, &ev, -1);
    if (rc != NO_ERROR) {
        TLOGE("%s: interrupted waiting for response", __func__);
        return rc;
    }

    rc = get_msg(session, &mi);
    if (rc != NO_ERROR) {
        TLOGE("%s: failed to get_msg (%d)\n", __func__, rc);
        return rc;
    }

    rc = read_msg(session, mi.id, 0, &rx_msg);
    put_msg(session, mi.id);
    if (rc < 0) {
        TLOGE("%s: failed to read msg (%d)\n", __func__, rc);
        return rc;
    }

    if ((size_t)rc != mi.len) {
        TLOGE("%s: partial message read (%zd vs. %zd)\n",
              __func__, (size_t)rc, mi.len);
        return ERR_IO;
    }

    return rc;
}

static int wait_to_send(handle_t session, struct ipc_msg *msg)
{
    int rc;
    struct uevent ev;

    rc = wait(session, &ev, -1);
    if (rc < 0) {
        TLOGE("failed to wait for outgoing queue to free up\n");
        return rc;
    }

    if (ev.event & IPC_HANDLE_POLL_SEND_UNBLOCKED) {
        return send_msg(session, msg);
    }

    if (ev.event & IPC_HANDLE_POLL_MSG) {
        return ERR_BUSY;
    }

    if (ev.event & IPC_HANDLE_POLL_HUP) {
        return ERR_CHANNEL_CLOSED;
    }

    return rc;
}

ssize_t send_reqv(storage_session_t session,
                         struct iovec *tx_iovs, uint tx_iovcnt,
                         struct iovec *rx_iovs, uint rx_iovcnt)
{
    ssize_t rc;

    struct ipc_msg tx_msg = {
        .iov = tx_iovs,
        .num_iov = tx_iovcnt,
    };

    rc = send_msg(session, &tx_msg);
    if (rc == ERR_NOT_ENOUGH_BUFFER) {
        rc = wait_to_send(session, &tx_msg);
    }

    if (rc < 0) {
        TLOGE("%s: failed (%d) to send_msg\n", __func__, (int)rc);
        return rc;
    }

    rc = get_response(session, rx_iovs, rx_iovcnt);
    if (rc < 0) {
        TLOGI("%s: failed (%d) to get response\n", __func__, (int)rc);
        return rc;
    }

    return rc;
}

int storage_open_session(storage_session_t *session_p, const char *type)
{
    long rc = connect(type, IPC_CONNECT_WAIT_FOR_PORT);
    if (rc < 0) {
        return rc;
    }

    *session_p = (storage_session_t) rc;
    return NO_ERROR;
}

void storage_close_session(storage_session_t session)
{
    close(session);
}

int storage_open_file(storage_session_t session, file_handle_t *handle_p,
                      const char *name, uint32_t flags, uint32_t opflags)
{
    struct storage_msg msg = { .cmd = STORAGE_FILE_OPEN, .flags = _to_msg_flags(opflags) };
    struct storage_file_open_req req = { .flags = flags };
    struct iovec tx[3] = {{&msg, sizeof(msg)}, {&req, sizeof(req)}, {(void *)name, strlen(name)}};
    struct storage_file_open_resp rsp = { 0 };
    struct iovec rx[2] = {{&msg, sizeof(msg)}, {&rsp, sizeof(rsp)}};

    ssize_t rc = send_reqv(session, tx, 3, rx, 2);
    rc = check_response(&msg, rc);
    if (rc < 0)
        return rc;

    if ((size_t)rc != sizeof(rsp)) {
        TLOGE("%s: invalid response length (%zd != %zd)\n", __func__, (size_t)rc, sizeof(rsp));
        return ERR_IO;
    }
    *handle_p = make_file_handle(session, rsp.handle);
    return NO_ERROR;
}

void storage_close_file(file_handle_t fh)
{
    struct storage_msg msg = { .cmd = STORAGE_FILE_CLOSE };
    struct storage_file_close_req req = { .handle = _to_handle(fh)};
    struct iovec tx[2] = {{&msg, sizeof(msg)}, {&req, sizeof(req)}};
    struct iovec rx[1] = {{&msg, sizeof(msg)}};

    ssize_t rc = send_reqv(_to_session(fh), tx, 2, rx, 1);
    rc = check_response(&msg, rc);
    if (rc < 0) {
        TLOGE("close file failed (%d)\n", (int)rc);
    }
}

int storage_delete_file(storage_session_t session, const char *name,
                        uint32_t opflags)
{
    struct storage_msg msg = { .cmd = STORAGE_FILE_DELETE, .flags = _to_msg_flags(opflags) };
    struct storage_file_delete_req req = { .flags = 0, };
    struct iovec tx[3] = {{&msg, sizeof(msg)}, {&req, sizeof(req)}, {(void *)name, strlen(name)}};
    struct iovec rx[1] = {{&msg, sizeof(msg)}};

    ssize_t rc = send_reqv(session, tx, 3, rx, 1);
    return (int)check_response(&msg, rc);
}

static ssize_t _read_chunk(file_handle_t fh, storage_off_t off, void *buf, size_t size)
{
    struct storage_msg msg = { .cmd = STORAGE_FILE_READ };
    struct storage_file_read_req req = { .handle = _to_handle(fh), .size = size, .offset = off };
    struct iovec tx[2] = {{&msg, sizeof(msg)}, {&req, sizeof(req)}};
    struct iovec rx[2] = {{&msg, sizeof(msg)}, {buf, size}};

    ssize_t rc = send_reqv(_to_session(fh), tx, 2, rx, 2);
    return check_response(&msg, rc);
}

ssize_t storage_read(file_handle_t fh, storage_off_t off, void *buf, size_t size)
{
    ssize_t rc;
    size_t bytes_read = 0;
    size_t chunk = MAX_CHUNK_SIZE;
    uint8_t *ptr = buf;

    while (size) {
        if (chunk > size)
            chunk = size;
        rc = _read_chunk(fh, off, ptr, chunk);
        if (rc < 0)
            return rc;
        if (rc == 0)
            break;
        off += rc;
        ptr += rc;
        bytes_read += rc;
        size -= rc;
    }
    return bytes_read;
}

static ssize_t _write_req(file_handle_t fh, storage_off_t off,
                          const void *buf, size_t size, uint32_t msg_flags)
{
    struct storage_msg msg = { .cmd = STORAGE_FILE_WRITE, .flags = msg_flags, };
    struct storage_file_write_req req = { .handle = _to_handle(fh), .offset = off, };
    struct iovec tx[3] = {{&msg, sizeof(msg)}, {&req, sizeof(req)}, {(void *)buf, size}};
    struct iovec rx[1] = {{&msg, sizeof(msg)}};

    ssize_t rc = send_reqv(_to_session(fh), tx, 3, rx, 1);
    rc = check_response(&msg, rc);
    return rc < 0 ? rc : (ssize_t)size;
}

ssize_t storage_write(file_handle_t fh, storage_off_t off,
                      const void *buf, size_t size, uint32_t opflags)
{
    ssize_t rc;
    size_t bytes_written = 0;
    size_t chunk = MAX_CHUNK_SIZE;
    const uint8_t *ptr = buf;
    uint32_t msg_flags = _to_msg_flags(opflags & ~STORAGE_OP_COMPLETE);

    while (size) {
        if (chunk >= size) {
            /* last chunk in sequence */
            chunk = size;
            msg_flags = _to_msg_flags(opflags);
        }
        rc = _write_req(fh, off, ptr, chunk, msg_flags);
        if (rc < 0)
            return rc;
        if ((size_t)rc != chunk) {
            TLOGE("got partial write (%d)\n", (int)rc);
            return ERR_IO;
        }
        off += chunk;
        ptr += chunk;
        bytes_written += chunk;
        size -= chunk;
    }
    return bytes_written;
}

int storage_set_file_size(file_handle_t fh, storage_off_t file_size, uint32_t opflags)
{
    struct storage_msg msg = { .cmd = STORAGE_FILE_SET_SIZE, .flags = _to_msg_flags(opflags)};
    struct storage_file_set_size_req req = { .handle = _to_handle(fh), .size = file_size, };
    struct iovec tx[2] = {{&msg, sizeof(msg)}, {&req, sizeof(req)}};
    struct iovec rx[1] = {{&msg, sizeof(msg)}};

    ssize_t rc = send_reqv(_to_session(fh), tx, 2, rx, 1);
    return (int)check_response(&msg, rc);
}

int storage_get_file_size(file_handle_t fh, storage_off_t *size_p)
{
    struct storage_msg msg = { .cmd = STORAGE_FILE_GET_SIZE };
    struct storage_file_get_size_req  req = { .handle = _to_handle(fh), };
    struct iovec tx[2] = {{&msg, sizeof(msg)}, {&req, sizeof(req)}};
    struct storage_file_get_size_resp rsp;
    struct iovec rx[2] = {{&msg, sizeof(msg)}, {&rsp, sizeof(rsp)}};

    ssize_t rc = send_reqv(_to_session(fh), tx, 2, rx, 2);
    rc = check_response(&msg, rc);
    if (rc < 0)
        return rc;

    if ((size_t)rc != sizeof(rsp)) {
        TLOGE("%s: invalid response length (%zd != %zd)\n",
              __func__, (size_t)rc, sizeof(rsp));
        return ERR_IO;
    }

    *size_p = rsp.size;
    return NO_ERROR;
}

int storage_end_transaction(storage_session_t session, bool complete)
{
    struct storage_msg msg = {
        .cmd = STORAGE_END_TRANSACTION,
        .flags = complete ? STORAGE_MSG_FLAG_TRANSACT_COMPLETE : 0,
    };
    struct iovec iov = {&msg, sizeof(msg)};

    ssize_t rc = send_reqv(session, &iov, 1, &iov, 1);
    return (int)check_response(&msg, rc);
}

static int _read_rpmb_chunk(storage_session_t sess, uint32_t blk_addr, uint32_t blk_num, void *buf)
{
    struct storage_msg msg = { .cmd = STORAGE_RPMB_RAW_READ };
    struct storage_rpmb_raw_read_req req = { .blk_addr = blk_addr, .blk_num = blk_num };
    struct iovec tx[2] = {{&msg, sizeof(msg)}, {&req, sizeof(req)}};
    struct iovec rx[2] = {{&msg, sizeof(msg)}, {buf, blk_num * RPMB_BLOCK_SIZE}};

    int rc = send_reqv(sess, tx, 2, rx, 2);
    return check_response(&msg, rc);
}

int storage_rpmb_read(storage_session_t sess, uint32_t blk_addr, uint32_t blk_num, void *buf, size_t buflen)
{
    uint8_t *ptr = buf;

    uint32_t chunk_blk_num;

    int rc;
    int bytes_read = 0;

    if ((buflen >> 8) < blk_num)
        return ERR_NOT_ENOUGH_BUFFER;

    while (blk_num) {

        chunk_blk_num = (MAX_RPMB_REQ_BLOCK > blk_num)? blk_num : MAX_RPMB_REQ_BLOCK;
        TLOGI("%s: blk_addr = %d, blk_num = %d, chunk_blk_num = %d\n", __func__, blk_addr, blk_num, chunk_blk_num);

        rc = _read_rpmb_chunk(sess, blk_addr, chunk_blk_num, ptr);
        if (rc < 0)
            return rc;
        if (rc == 0)
            break;

        blk_addr += chunk_blk_num;
        blk_num  -= chunk_blk_num;

        ptr += rc;
        bytes_read += rc;
    }

    return bytes_read;
}
