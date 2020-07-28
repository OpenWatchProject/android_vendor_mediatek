#ifndef __KMSETKEY_IPC_H__
#define __KMSETKEY_IPC_H__

#include <sys/types.h>
#include <stdbool.h>

#define KMSETKEY_PORT		"com.mediatek.kmsetkey"
#define MAX_MSG_SIZE		4096
#define IPC_MSG_SIZE		16
#define RESP_FLAG		0x80000000
#define DONE_FLAG		0x40000000

enum kmsetkey_cmd {
	KEY_LEN = 0x0,
	KEY_BUF = 0x1,
	SET_KEY = 0x2,
};

struct kmsetkey_msg {
	uint32_t cmd;
	uint8_t payload[0];
};

__BEGIN_DECLS
int kmsetkey_connect();
int kmsetkey_call(const uint32_t cmd, const bool finish, const uint8_t *in, const uint32_t in_size, uint8_t *out, uint32_t *out_size);
void kmsetkey_disconnect();
__END_DECLS

#endif
