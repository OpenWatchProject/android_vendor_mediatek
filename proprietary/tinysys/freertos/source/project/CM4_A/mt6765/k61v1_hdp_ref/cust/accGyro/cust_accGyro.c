#include "cust_accGyro.h"

struct accGyro_hw cust_accGyro_hw[] __attribute__((section(".cust_accGyro"))) = {
#ifdef CFG_STK832X_SUPPORT
    {
        .name = "stk832x",
        .i2c_num = 0,
        .direction = 1,
        .i2c_addr = {0x68, 0},
        .eint_num = 3,
    },
#endif

};
