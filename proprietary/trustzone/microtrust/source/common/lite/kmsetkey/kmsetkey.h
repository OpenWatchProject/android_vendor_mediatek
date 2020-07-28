#ifndef __KMSETKEY_H__
#define __KMSETKEY_H__

#include <hardware/hardware.h>

struct kmsetkey_module {

	hw_module_t common;
};

struct kmsetkey_device {

	hw_device_t common;
	int (*attest_key_install)(const uint8_t *peakb, const uint32_t peakb_len);
};

typedef struct kmsetkey_device kmsetkey_device_t;

#endif
