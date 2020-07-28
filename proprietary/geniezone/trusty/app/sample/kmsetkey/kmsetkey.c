/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/

#include <assert.h>
#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trusty_std.h>

#include <kmsetkey.h>
#include <cryptolib.h>
#include <test.h>

#define LOG_TAG "KM_SETKEY"
#define TLOGE(fmt, ...) \
	fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

//extern int32_t tries, rand_tries;

static long rsa_key(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;
	//int s = sizeof(struct kmsetkey_msg) + 2 * sizeof(int32_t);
	//struct kmsetkey_msg *rsp = malloc(s);
	//rsp->cmd = req->cmd;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = generate_rsa_key();
	if (rc != NO_ERROR) {
		TLOGE("%s: rsa key returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);
	//memcpy(rsp->payload, &tries, sizeof(int32_t));
	//memcpy(rsp->payload + sizeof(int32_t), &rand_tries, sizeof(int32_t));

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long rsa_sign(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;
	//int s = sizeof(struct kmsetkey_msg) + 2 * sizeof(int32_t);
	//struct kmsetkey_msg *rsp = malloc(s);
	//rsp->cmd = req->cmd;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = generate_rsa_signature();
	if (rc != NO_ERROR) {
		TLOGE("%s: rsa key returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);
	//memcpy(rsp->payload, &tries, sizeof(int32_t));
	//memcpy(rsp->payload + sizeof(int32_t), &rand_tries, sizeof(int32_t));

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long rsa_veri(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;
	//int s = sizeof(struct kmsetkey_msg) + 2 * sizeof(int32_t);
	//struct kmsetkey_msg *rsp = malloc(s);
	//rsp->cmd = req->cmd;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = verify_rsa_signature();
	if (rc != NO_ERROR) {
		TLOGE("%s: rsa key returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);
	//memcpy(rsp->payload, &tries, sizeof(int32_t));
	//memcpy(rsp->payload + sizeof(int32_t), &rand_tries, sizeof(int32_t));

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long ecc_key(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = generate_ecc_key();
	if (rc != NO_ERROR) {
		TLOGE("%s: ecc key returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long ecc_sign(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = generate_ecc_signature();
	if (rc != NO_ERROR) {
		TLOGE("%s: rsa key returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long ecc_veri(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = verify_ecc_signature();
	if (rc != NO_ERROR) {
		TLOGE("%s: rsa key returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}


static long aes_128_enc(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = perform_aes_128_enc();
	if (rc != NO_ERROR) {
		TLOGE("%s: aes encryption returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long aes_256_enc(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = perform_aes_256_enc();
	if (rc != NO_ERROR) {
		TLOGE("%s: aes encryption returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long sha_hash(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = perform_sha_hash();
	if (rc != NO_ERROR) {
		TLOGE("%s: sha hash returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long rsa_test(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = generate_rsa_key_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: generate_rsa_key_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rc = generate_rsa_signature_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: generate_rsa_signature_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rc = verify_rsa_signature_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: verify_rsa_signature_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long ecc_test(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = generate_ecc_key_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: generate_ecc_key_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rc = generate_ecc_signature_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: generate_ecc_signature_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rc = verify_ecc_signature_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: verify_ecc_signature_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long aes_test(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = perform_aes_128_enc_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: perform_aes_128_enc_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rc = perform_aes_128_dec_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: perform_aes_128_dec_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rc = perform_aes_256_enc_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: perform_aes_256_enc_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rc = perform_aes_256_dec_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: perform_aes_256_dec_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}


static long sha_test(handle_t chan, struct kmsetkey_msg *req, size_t req_len)
{
	long rc = NO_ERROR;;
	struct kmsetkey_msg rsp = {
		.cmd = req->cmd,
	};

	iovec_t iov = {
		.base = &rsp,
		.len = sizeof(rsp),
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = perform_sha_hash_test();
	if (rc != NO_ERROR) {
		TLOGE("%s: perform_sha_hash_test returned (%ld)\n", __FUNCTION__, rc);
		goto exit;
	}

	rsp.cmd |= (RESP_FLAG | DONE_FLAG);

exit:
	send_msg(chan, &msg);
	return rc < 0 ? rc : NO_ERROR;
}

static long handle_msg(handle_t chan)
{
	/* get message info */
	ipc_msg_info_t msg_inf;

	long rc = get_msg(chan, &msg_inf);
	if (rc == ERR_NO_MSG)
		return NO_ERROR; /* no new messages */

	// fatal error
	if (rc != NO_ERROR) {
		TLOGE("%s: failed (%ld) to get_msg for chan (%d), closing connection\n", __FUNCTION__, rc, chan);
		return rc;
	}

	if (msg_inf.len < sizeof(struct kmsetkey_msg)) {
		TLOGE("%s: unexpected msg size (%u) for chan (%d)\n", __FUNCTION__, msg_inf.len, chan);
		put_msg(chan, msg_inf.id);
		return ERR_NOT_VALID;
	}

	struct kmsetkey_msg *req = malloc(msg_inf.len);

	/* read msg content */
	iovec_t iov = {
		.base = req,
		.len = msg_inf.len,
	};
	ipc_msg_t msg = {
		.iov = &iov,
		.num_iov = 1,
		.handles = NULL,
		.num_handles = 0,
	};

	rc = read_msg(chan, msg_inf.id, 0, &msg);

	if (rc < 0) {
		TLOGE("%s: failed to read msg (%ld) for chan (%d)\n", __FUNCTION__, rc, chan);
		goto exit;
	}

	if ((size_t)rc != msg_inf.len) {
		TLOGE("%s: invalid message of size (%zu) for chan (%d)\n", __FUNCTION__, (size_t)rc, chan);
		rc = ERR_NOT_VALID;
		goto exit;
	}

	switch (req->cmd) {
	case RSA_KEY:
		rc = rsa_key(chan, req, msg_inf.len);
		break;
	case RSA_SIGN:
		rc = rsa_sign(chan, req, msg_inf.len);
		break;
	case RSA_VERI:
		rc = rsa_veri(chan, req, msg_inf.len);
		break;
	case ECC_KEY:
		rc = ecc_key(chan, req, msg_inf.len);
		break;
	case ECC_SIGN:
		rc = ecc_sign(chan, req, msg_inf.len);
		break;
	case ECC_VERI:
		rc = ecc_veri(chan, req, msg_inf.len);
		break;
	case AES_128_ENC:
		rc = aes_128_enc(chan, req, msg_inf.len);
		break;
	case AES_256_ENC:
		rc = aes_256_enc(chan, req, msg_inf.len);
		break;
	case SHA_HASH:
		rc = sha_hash(chan, req, msg_inf.len);
		break;
	case RSA_TEST:
		rc = rsa_test(chan, req, msg_inf.len);
		break;
	case ECC_TEST:
		rc = ecc_test(chan, req, msg_inf.len);
		break;
	case AES_TEST:
		rc = aes_test(chan, req, msg_inf.len);
		break;
	case SHA_TEST:
		rc = sha_test(chan, req, msg_inf.len);
		break;
	default:
		rc = ERR_NOT_SUPPORTED;
		break;
	}

	if (rc != NO_ERROR)
		TLOGE("%s: unable (%ld) to handle request\n", __FUNCTION__, rc);

exit:
	free(req);
	put_msg(chan, msg_inf.id);
	return rc;
}

static void kmsetkey_handle_port(uevent_t *ev)
{
	if ((ev->event & IPC_HANDLE_POLL_ERROR) ||
		(ev->event & IPC_HANDLE_POLL_HUP) ||
		(ev->event & IPC_HANDLE_POLL_MSG) ||
		(ev->event & IPC_HANDLE_POLL_SEND_UNBLOCKED)) {
		/* should never happen with port handles */
		TLOGE("%s: error event (0x%x) for port (%d)\n", __FUNCTION__, ev->event, ev->handle);
		abort();
	}

	uuid_t peer_uuid;
	if (ev->event & IPC_HANDLE_POLL_READY) {
		/* incoming connection: accept it */
		int rc = accept(ev->handle, &peer_uuid);
		if (rc < 0) {
			TLOGE("%s: failed (%d) to accept on port %d\n", __FUNCTION__, rc, ev->handle);
			return;
		}
	}
}

static void kmsetkey_handle_channel(uevent_t *ev)
{
	if ((ev->event & IPC_HANDLE_POLL_ERROR) ||
		(ev->event & IPC_HANDLE_POLL_READY)) {
		/* close it as it is in an error state */
		TLOGE("%s: error event (0x%x) for chan (%d)\n", __FUNCTION__, ev->event, ev->handle);
		close(ev->handle);
		return;
	}

	handle_t chan = ev->handle;

	if (ev->event & IPC_HANDLE_POLL_MSG) {
		long rc = handle_msg(chan);
		if (rc != NO_ERROR) {
			/* report an error and close channel */
			TLOGE("%s: failed (%ld) to handle event on channel %d\n", __FUNCTION__, rc, ev->handle);
			close(chan);
			return;
		}
	}

	if (ev->event & IPC_HANDLE_POLL_HUP) {
		/* closed by peer. */
		close(chan);
		return;
	}

}

int main(void)
{
	int rc;
	uevent_t event;

	rc = port_create(KMSETKEY_PORT, 1, MAX_MSG_SIZE, IPC_PORT_ALLOW_NS_CONNECT);
	if (rc < 0) {
		TLOGE("%s: Failed (%d) to create port %s\n", __FUNCTION__, rc, KMSETKEY_PORT);
	}

	handle_t port = (handle_t)rc;

	init_data(0xF3, 0x88, 0x32, 0x46);

	/* enter main event loop */
	while (true) {
		event.handle = INVALID_IPC_HANDLE;
		event.event  = 0;
		event.cookie = NULL;

		rc = wait_any(&event, -1);
		if (rc < 0) {
			TLOGE("%s: wait_any failed (%ld)\n", __FUNCTION__, rc);
			break;
		}

		if (rc == NO_ERROR) { /* got an event */
			if (event.handle == port) {
				kmsetkey_handle_port(&event);
			} else {
				kmsetkey_handle_channel(&event);
			}
		} else {
			TLOGE("%s: wait_any returned (%d)\n", __FUNCTION__, rc);
			break;
		}
	}

	return 0;
}
