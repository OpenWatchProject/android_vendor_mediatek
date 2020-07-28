#ifndef __MODEM_SECURE_BASE_H__
#define __MODEM_SECURE_BASE_H__

#include <mmio.h>

#define mdsecure_write32(addr, value) \
	mmio_write_32(addr, value)

#endif
