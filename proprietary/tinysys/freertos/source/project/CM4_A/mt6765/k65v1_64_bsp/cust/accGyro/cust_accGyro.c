#include "cust_accGyro.h"

struct accGyro_hw cust_accGyro_hw[] __attribute__((section(".cust_accGyro"))) = {
#ifdef CFG_LIS2HH12_SUPPORT
    {
        .name = "lis2hh12",
        .i2c_num = 0,
        .direction = 7,
        .i2c_addr = {0x68, 0},
        .eint_num = 12,
    },
#endif

};
