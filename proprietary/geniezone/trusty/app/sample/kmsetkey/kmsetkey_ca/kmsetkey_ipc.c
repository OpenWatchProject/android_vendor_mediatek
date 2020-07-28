#define LOG_TAG "KMSetkey_IPC"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <log/log.h>
#include <trusty/tipc.h>

#include <kmsetkey_ipc.h>

#define TRUSTY_DEVICE_NAME "/dev/trusty-ipc-dev0"

static int handle_ = 0;

int kmsetkey_connect()
{
	int rc = tipc_connect(TRUSTY_DEVICE_NAME, KMSETKEY_PORT);
	if (rc < 0) {
		return rc;
	}

	handle_ = rc;
	return 0;
}

int kmsetkey_call(const uint32_t cmd, const bool finish, const uint8_t *in, const uint32_t in_size, uint8_t *out, uint32_t *out_size)
{
	if (handle_ == 0) {
		ALOGE("not connected\n");
		printf("not connected\n");
		return -EINVAL;
	}

	size_t msg_size = sizeof(struct kmsetkey_msg) + in_size;
	struct kmsetkey_msg *msg = (struct kmsetkey_msg *)malloc(msg_size);
	msg->cmd = cmd;
	if (in_size > 0)
		memcpy(msg->payload, in, in_size);

	ssize_t rc = write(handle_, msg, msg_size);
	free(msg);

	if (rc < 0) {
		ALOGE("failed to send cmd (%d) to %s: %s\n", cmd, KMSETKEY_PORT, strerror(errno));
		printf("failed to send cmd (%d) to %s: %s\n", cmd, KMSETKEY_PORT, strerror(errno));
		return -errno;
	}

	rc = read(handle_, out, *out_size);
	if (rc < 0) {
		ALOGE("failed to retrieve response for cmd (%d) to %s: %s\n", cmd, KMSETKEY_PORT, strerror(errno));
		printf("failed to retrieve response for cmd (%d) to %s: %s\n", cmd, KMSETKEY_PORT, strerror(errno));
		return -errno;
	}

	if ((size_t)rc < sizeof(struct kmsetkey_msg)) {
		ALOGE("invalid response size (%d)\n", (int)rc);
		printf("invalid response size (%d)\n", (int)rc);
		return -EINVAL;
	}

	msg = (struct kmsetkey_msg *)out;
	if ((finish && msg->cmd != (cmd | RESP_FLAG | DONE_FLAG)) || (!finish && msg->cmd != (cmd | RESP_FLAG))) {
		ALOGE("invalid response (%u) for cmd (%u)\n", msg->cmd, cmd);
		printf("invalid response (%u) for cmd (%u)\n", msg->cmd, cmd);
		return -EINVAL;
	}

	*out_size = (uint32_t)rc;
	return 0;
}

void kmsetkey_disconnect()
{
	if (handle_ != 0) {
		tipc_close(handle_);
	}
}
