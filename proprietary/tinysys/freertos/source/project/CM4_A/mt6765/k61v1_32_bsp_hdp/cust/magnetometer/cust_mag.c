#include "cust_mag.h"

struct mag_hw cust_mag_hw[] __attribute__((section(".cust_mag"))) = {
#ifdef CFG_AKM09918_SUPPORT
    {
        .name = "akm09918",
        .i2c_num = 0,
        .direction = 4,
        .i2c_addr = {0x0c, 0},
    },
#endif
};
