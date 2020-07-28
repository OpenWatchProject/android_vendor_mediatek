#include "AKMPG_APIs.h"
#include "akm_pg_wrapper.h"

int g_init_flag = 0;

void akm_pg_magdata(
    const int16 mag_x,
    const int16 mag_y,
    const int16 mag_z,
    const int16 hdt,
    const int16 hdst)
{
    if (g_init_flag == 0) {
        if (AKMPG_Init() == AKMD_SUCCESS) {
            g_init_flag = 1;
        }
    }

    if (g_init_flag == 1) {
        AKMPG_SetMagData(mag_x, mag_y, mag_z, hdt, hdst);
    }
}

void akm_pg_accdata(const int16 acc_x, const int16 acc_y, const int16 acc_z)
{
    if (g_init_flag == 0) {
        if (AKMPG_Init() == AKMD_SUCCESS) {
            g_init_flag = 1;
        }
    }

    if (g_init_flag == 1) {
        AKMPG_SetAccData(acc_x, acc_y, acc_z);
    }
}

void akm_pg_get_gyro(AKSC_FLOAT gvec[3])
{
    AKMPG_GetGyro(gvec);

    return;
}

void akm_pg_get_rv(AKSC_FLOAT rv[5])
{
    AKMPG_GetRotationVector(rv);
}

void akm_pg_get_gravity(AKSC_FLOAT gravity[3])
{
    AKMPG_GetGravity(gravity);
}

void akm_pg_get_la(AKSC_FLOAT la[3])
{
    AKMPG_GetLA(la);
}
