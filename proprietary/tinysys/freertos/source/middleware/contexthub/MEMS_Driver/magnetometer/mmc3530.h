#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <platform.h>


/*mmc3530 register address*/
#define MMC3530_DEVICE_NAME "mmc3530"
#define MMC3530_REG_PID 0x20
#define MMC3530_DEVICE_ID 0x09
#define MMC3524X_RETRY_COUNT 3

#define MMC3530_REG_STATUS 0x06
#define MMC3530_REG_CTRL 0x07
#define MMC3530_REG_BITS 0x08
#define MMC3530_REG_DATA 0x00
#define MMC3530_REG_PRODUCTID_1 0x20

#define MMC3530_REG_OTP  0x1b

#if 0
#define MMC3530_CTRL_TM 0x11
#define MMC3530_CTRL_SET 0x30
#define MMC3530_CTRL_RESET 0x50
#define MMC3530_CTRL_REFILL 0x10
#else
#define MMC3530_CTRL_TM 0x01
#define MMC3530_CTRL_SET 0x20
#define MMC3530_CTRL_RESET 0x40
#define MMC3530_CTRL_REFILL 0x80
#endif

#define MMC3530_BITS_SLOW_16 0x00

#define MMC3530_DELAY_SET 50 /* ms */
#define MMC3530_DELAY_RST 50 /* ms */
#define MMC3530_DELAY_TM 10 /* ms */