#include "cust_alsps.h"

struct alsps_hw cust_alsps_hw[] __attribute__((section(".cust_alsps"))) = {
#ifdef CFG_CM36558_SUPPORT
    {
        .name = "cm36558",
        .i2c_num    = 0,
        .i2c_addr = {0x51, 0},
        .polling_mode_ps = 0,
        .polling_mode_als = 1,
        .als_level  = {0, 328, 861, 1377, 3125, 7721, 7767, 12621, 23062, 28430, 33274, 47116, 57694, 57694, 65535},
        .als_value  = {0, 200, 320, 502, 1004, 2005, 3058, 5005, 8008, 10010, 12000, 16000, 20000, 20000, 20000, 20000},
        .ps_threshold_high = 26,
        .ps_threshold_low = 21,
        .eint_num = 10,
    },
#endif
#ifdef CFG_TMD2725_SUPPORT
    {
        .name = "tmd2725",
        .i2c_num    = 0,
        .i2c_addr = {0x39, 0},
        .polling_mode_ps = 0,
        .polling_mode_als = 1,
        .als_level  = {0, 328, 861, 1377, 3125, 7721, 7767, 12621, 23062, 28430, 33274, 47116, 57694, 57694, 65535},
        .als_value  = {0, 200, 320, 502, 1004, 2005, 3058, 5005, 8008, 10010, 12000, 16000, 20000, 20000, 20000, 20000},
        .ps_threshold_high = 80,
        .ps_threshold_low = 30,
        .eint_num = 10,
    },
#endif
#ifdef CFG_APDS9922_SUPPORT
    {
        .name = "apds9922",
        .i2c_num    = 0,
        .i2c_addr = {0x53, 0},
        .polling_mode_ps = 0,
        .polling_mode_als = 1,
        .als_level  = {0, 328, 861, 1377, 3125, 7721, 7767, 12621, 23062, 28430, 33274, 47116, 57694, 57694, 65535},
        .als_value  = {0, 200, 320, 502, 1004, 2005, 3058, 5005, 8008, 10010, 12000, 16000, 20000, 20000, 20000, 20000},
        .ps_threshold_high = 26,
        .ps_threshold_low = 21,
        .eint_num = 10,
    },
#endif
#ifdef CFG_STK3X3X_SUPPORT
    {
        .name = "stk3x3x",
        .i2c_num    = 0,
        .i2c_addr = {0x47, 0},
        .polling_mode_ps = 0,
        .polling_mode_als = 1,
        .als_level  = {0, 328, 861, 1377, 3125, 7721, 7767, 12621, 23062, 28430, 33274, 47116, 57694, 57694, 65535},
        .als_value  = {0, 200, 320, 502, 1004, 2005, 3058, 5005, 8008, 10010, 12000, 16000, 20000, 20000, 20000, 20000},
        .ps_threshold_high = 90,
        .ps_threshold_low = 40,
        .eint_num = 10,
    },
#endif
};
