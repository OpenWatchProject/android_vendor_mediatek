#include "cust_sar.h"

struct sar_hw cust_sar_hw[] __attribute__((section(".cust_sar"))) = {
#ifdef CFG_SAR_SUPPORT
    {
        .name = "sx9310",
        .i2c_num = 0,
        .direction = 0,
        .i2c_addr = {0x28, 0},
    },
#endif
};
