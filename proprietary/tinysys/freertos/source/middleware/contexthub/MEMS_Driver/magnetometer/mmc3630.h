#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <platform.h>


/*mmc3630 register address*/
#define MMC3630_DEVICE_NAME "mmc3630"
#define MMC3630_REG_PID 0x2F
#define MMC3630_DEVICE_ID 0x0A
#define MMC3524X_RETRY_COUNT 3

#define MMC3630_REG_STATUS 0x06
#define MMC3630_REG_CTRL 0x08
#define MMC3630_REG_BITS 0x09
#define MMC3630_REG_DATA 0x00
#define MMC3630_REG_PRODUCTID_1 0x20

#define MMC36XX_MAG_REG_ADDR_OTP_GATE0 0x0F
#define MMC36XX_MAG_REG_ADDR_OTP_GATE1 0x12
#define MMC36XX_MAG_REG_ADDR_OTP_GATE2 0x13
#define MMC36XX_MAG_REG_ADDR_CTRL2     0x0A

#define MMC36XX_MAG_OTP_OPEN0 0xE1
#define MMC36XX_MAG_OTP_OPEN1 0x11
#define MMC36XX_MAG_OTP_OPEN2 0x80
#define MMC3630_REG_OTP       0x2A

#define MMC3630_CTRL_TM       0x01
#define MMC3630_CTRL_SET      0x08
#define MMC3630_CTRL_RESET    0x10
#define MMC3630_CTRL_REFILL   0x20
#define MMC3630_BITS_SLOW_16  0x00

#define MMC3630_DELAY_SET     50 /* ms */
#define MMC3630_DELAY_RST     50 /* ms */
#define MMC3630_DELAY_TM      10 /* ms */
