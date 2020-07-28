#define LOG_TAG "KMSetkey"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <type_traits>

#include <log/log.h>
#include <cutils/log.h>

#include <kmsetkey.h>
#include <kmsetkey_ipc.h>

static uint32_t kb_offset = 0, kb_len = 0;
static uint8_t *kb_buf = NULL;

static void attest_keybox_clear(void)
{
	kb_offset = 0;
	kb_len = 0;
	free(kb_buf);
	kb_buf = NULL;
}

int32_t ree_import_attest_keybox(const uint8_t *peakb, const uint32_t peakb_len, const bool finish)
{
	int32_t rc;
	uint32_t payload_offset, payload_len, out_size;
	uint8_t out[MAX_MSG_SIZE], *tmp_buf;
	struct kmsetkey_msg *msg;

	if (peakb == NULL || peakb_len == 0) {
		ALOGE("attest keybox is NULL\n");
		printf("attest keybox is NULL\n");
		return -1;
	}

	payload_offset = kb_offset + peakb_len;
	if (payload_offset > kb_len) {
		payload_len = finish ? payload_offset : 2 * payload_offset;
		tmp_buf = (uint8_t *)malloc(payload_len);
		memcpy(tmp_buf, kb_buf, kb_offset);
		free(kb_buf);
		kb_buf = tmp_buf;
		kb_len = payload_len;
	}
	memcpy(kb_buf + kb_offset, peakb, peakb_len);
	kb_offset += peakb_len;

	if (!finish)
		return 0;

	rc = kmsetkey_connect();
	if (rc < 0) {
		ALOGE("kmsetkey_connect failed: %d\n", rc);
		printf("kmsetkey_connect failed: %d\n", rc);
		attest_keybox_clear();
		return rc;
	}

	out_size = MAX_MSG_SIZE;
	rc = kmsetkey_call(KEY_LEN, true, (uint8_t *)&kb_offset, sizeof(uint32_t), out, &out_size);
	if (rc < 0) {
		ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, KEY_LEN);
		printf("kmsetkey_call failed: %d for cmd %u\n", rc, KEY_LEN);
		goto exit;
	}

	for (payload_offset = 0; payload_offset < kb_offset; payload_offset += payload_len) {
		payload_len = kb_offset - payload_offset < MAX_MSG_SIZE - IPC_MSG_SIZE - sizeof(struct kmsetkey_msg) ? kb_offset - payload_offset : MAX_MSG_SIZE - IPC_MSG_SIZE - sizeof(struct kmsetkey_msg);
		printf("payload_len = %u\n", payload_len);
		out_size = MAX_MSG_SIZE;
		rc = kmsetkey_call(KEY_BUF, payload_offset + payload_len < kb_offset ? false : true, kb_buf + payload_offset, payload_len, out, &out_size);
		if (rc < 0) {
			ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, KEY_BUF);
			printf("kmsetkey_call failed: %d for cmd %u\n", rc, KEY_BUF);
			goto exit;
		}
	}

	out_size = MAX_MSG_SIZE;
	rc = kmsetkey_call(SET_KEY, true, NULL, 0, out, &out_size);
	if (rc < 0) {
		ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, SET_KEY);
		printf("kmsetkey_call failed: %d for cmd %u\n", rc, SET_KEY);
	}

exit:
	kmsetkey_disconnect();
	attest_keybox_clear();
	return rc < 0 ? rc : 0;
}
