#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <platform.h>
#include "memsic_wrapper.h"

/*mmc5603 register address*/
#define MMC5603_REG_PRODUCT   0x39
#define MMC5603_PRODUCT_ID_VALUE 0x10


#define MMC5603_REG_DATA      0x00


#define MMC5603NJ_CMD_BW00 0x00
#define MMC5603NJ_REG_CTRL0 0x1B
#define MMC5603NJ_REG_CTRL1 0x1C
#define MMC5603NJ_REG_CTRL2 0x1D
#define MMC5603NJ_REG_ODR   0x1A
#define MMC5603NJ_ODR_50HZ  0X32
#define MMC5603NJ_CMD_AUTO_SR_EN    0xA0
#define MMC5603NJ_MANU_SET          0x08
#define MMC5603NJ_MANU_RESET        0x10
#define MMC5603NJ_CMD_CMM_EN        0x10
#define MMC5603NJ_CMD_CMM_DISEN     0x00
#define MMC5603NJ_MANU_TM           0x01

