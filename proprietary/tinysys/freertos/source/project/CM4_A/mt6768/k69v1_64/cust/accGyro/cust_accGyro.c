#include "cust_accGyro.h"

struct accGyro_hw cust_accGyro_hw[] __attribute__((section(".cust_accGyro"))) = {
#ifdef CFG_BMI160_SUPPORT
    {
        .name = "bmi160",
        .i2c_num = 1,
        .direction = 4,
        .eint_num = 2,
    },
#endif
#ifdef CFG_LSM6DSM_SUPPORT
    {
        .name = "lsm6dsm",
        .i2c_num = 1,
        .direction = 7,
        .eint_num = 2,
    },
#endif
#ifdef CFG_LSM6DS3_SUPPORT
    {
        .name = "lsm6ds3",
        .i2c_num = 1,
        .direction = 7,
        .eint_num = 2,
    },
#endif
#ifdef CFG_LIS3DH_SUPPORT
    {
        .name = "lis3dh",
        .i2c_num = 1,
        .direction = 6,
        .eint_num = 2,
    },
#endif
#ifdef CFG_LIS2HH12_SUPPORT
    {
        .name = "lis2hh12",
        .i2c_num = 1,
        .direction = 7,
        .eint_num = 2,
    },
#endif
};
