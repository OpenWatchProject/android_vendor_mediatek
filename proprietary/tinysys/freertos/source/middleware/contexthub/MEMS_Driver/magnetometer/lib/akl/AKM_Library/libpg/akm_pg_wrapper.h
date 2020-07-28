#ifndef AKM_PG_WRAPPER_H
#define AKM_PG_WRAPPER_H
void akm_pg_magdata(
    const int16 mag_x,
    const int16 mag_y,
    const int16 mag_z,
    const int16 hdt,
    const int16 hdst);

void akm_pg_accdata(const int16 acc_x, const int16 acc_y, const int16 acc_z);

void akm_pg_get_gyro(AKSC_FLOAT gvec[3]);
void akm_pg_get_rv(AKSC_FLOAT rv[3]);
void akm_pg_get_gravity(AKSC_FLOAT gravity[3]);
void akm_pg_get_la(AKSC_FLOAT la[3]);
#endif
