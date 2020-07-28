#include "cust_mag.h"

struct mag_hw cust_mag_hw[] __attribute__((section(".cust_mag"))) = {
#ifdef CFG_AKM09915_SUPPORT
    {
        .name = "akm09915",
        .i2c_num = 1,
        .direction = 4,
        .i2c_addr = {0x0c, 0},
    },
#endif
#ifdef CFG_AKM09918_SUPPORT
    {
        .name = "akm09918",
        .i2c_num = 1,
        .direction = 4,
        .i2c_addr = {0x0c, 0},
    },
#endif
#ifdef CFG_MMC3530_SUPPORT
    {
        .name = "mmc3530",
        .i2c_num = 1,
        .direction = 4,
        .i2c_addr = {0x30, 0},
    },
#endif
#ifdef CFG_MMC3630_SUPPORT
    {
        .name = "mmc3630",
        .i2c_num = 1,
        .direction = 4,
        .i2c_addr = {0x30, 0},
    },
#endif
#ifdef CFG_MMC5603_SUPPORT
    {
        .name = "mmc5603",
        .i2c_num = 1,
        .direction = 4,
        .i2c_addr = {0x30, 0},
    },
#endif
};
