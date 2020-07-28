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
	RSA_KEY = 0x00,
	RSA_SIGN = 0x01,
	RSA_VERI = 0x02,
	ECC_KEY = 0x10,
	ECC_SIGN = 0x11,
	ECC_VERI = 0x12,
	AES_128_ENC = 0x20,
	AES_256_ENC = 0x21,
	SHA_HASH = 0x30,
	RSA_TEST = 0x100,
	ECC_TEST = 0x101,
	AES_TEST = 0x102,
	SHA_TEST = 0x103,
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
