#include "cust_accGyro.h"

struct accGyro_hw cust_accGyro_hw[] __attribute__((section(".cust_accGyro"))) = {
#ifdef CFG_BMI160_SUPPORT
    {
        .name = "bmi160",
        .i2c_num = 2,
        .direction = 7,
        .eint_num = 10,
    },
#endif
#ifdef CFG_LSM6DSM_SUPPORT
    {
        .name = "lsm6dsm",
        .i2c_num = 2,
        .direction = 7,
        .eint_num = 10,
    },
#endif
#ifdef CFG_LIS2HH12_SUPPORT
    {
        .name = "lis2hh12",
        .i2c_num = 2,
        .direction = 7,
        .eint_num = 10,
    },
#endif
};
