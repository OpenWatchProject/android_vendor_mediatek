#ifndef __MPE_DR_H_INCLUDED__
#define __MPE_DR_H_INCLUDED__

#include "stdint.h"
#include "mpe_Attitude.h"

/******************************************************************************
* Version Info
******************************************************************************/

#define MPE_VERSION_HEAD 'M','P','E','_','V','E','R','_'
#define MPE_MAJOR_VERSION '1','7','0','8','1','0','0','2' // y,y,m,m,d,d,rev,rev
#define MPE_BRANCH_INFO '_','1','.','0','0','_'
#define MPE_MINER_VERSION '0','0'

#define MPE_VER_INFO MPE_VERSION_HEAD,MPE_MAJOR_VERSION,MPE_BRANCH_INFO,MPE_MINER_VERSION

#define MPE_GRAVITY_EARTH   9.8066f

#ifdef __cplusplus
extern "C" {
#endif


typedef struct MNL2MPE_AIDING_DATA{
    double  latitude[2]; /* latitude in radian */
    double  longitude[2]; /* longitude in radian */
    double  altitude[2]; /* altitude in meters above the WGS 84 reference ellipsoid */
    float   KF_velocity[3]; /* Kalman Filter velocity in meters per second under (N,E,D) frame */
    float   LS_velocity[3]; /* Least Square velocity in meters per second under (N,E,D) frame */
    float   HACC; /*  position horizontal accuracy in meters */
    float   VACC; /*  vertical accuracy in meters */
    float   KF_velocitySigma[3]; /* Kalman Filter velocity one sigma error in meter per second under (N,E,D) frame */
    float   LS_velocitySigma[3]; /* Least Square velocity one sigma error in meter per second under (N,E,D) frame */
    float   HDOP; /* horizontal dilution of precision value in unitless */
    float   confidenceIndex[3]; /* GPS confidence index */
    unsigned int   gps_sec; /* Timestamp of GPS location */
    unsigned int   leap_sec; /* correct GPS time with phone kernel time */
} MNL2MPE_AIDING_DATA;


typedef struct MPE2MNL_AIDING_DATA{
    double  latitude; /* latitude in radian */
    double  longitude; /* longitude in radian */
    double  altitude; /* altitude in meters above the WGS 84 reference ellipsoid */
    float   velocity[3]; /* SENSOR velocity in meters per second under (N,E,D) frame */
    float   acceleration[3]; /* SENSOR acceleration in meters per second^2 under (N,E,D) frame */
    float   HACC; /*  position horizontal accuracy in meters */
    float   VACC; /*  vertical accuracy in meters */
    float   velocitySigma[3]; /* SENSOR velocity one sigma error in meter per second under (N,E,D) frame */
    float   accelerationSigma[3]; /* SENSOR acceleration one sigma error in meter per second^2 under (N,E,D) frame */
    float   bearing; /* SENSOR heading in degrees UNDER (N,E,D) frame*/
    float   confidenceIndex[3]; /*  SENSOR confidence index */
    float   barometerHeight;         /*barometer height in meter*/
    int     valid_flag[4]; /*  SENSOR AGMB hardware valid flag */
    int     staticIndex; /* AR status [static, move, uncertain],[0,1,99]*/
    unsigned long long timestamp; /* Timestamp of SENSOR location */
} MPE2MNL_AIDING_DATA;


typedef enum{
    MTK_MPE_SYS_FLAG = 0x00,
    MTK_MPE_KER_FLAG,
    MTK_MPE_RAW_FLAG,
    MTK_MPE_RES1_FLAG,
    MTK_MPE_MAX_FLAG
}MTK_VALID_FLAG_TYPE;


int mpe_update_dead_reckoning(LPIMU pImu, int deltaTime_us, MNL2MPE_AIDING_DATA *pGPS);
int get_map_matching_result(LPIMU pImu, MNL2MPE_AIDING_DATA *pGPS, MPE2MNL_AIDING_DATA dr_position, MNL2MPE_AIDING_DATA *pMAP);
int mpe_get_dr_position(MPE2MNL_AIDING_DATA *pos);
int mpe_get_dr_gyro_bias(float *bias);
int mpe_dr_re_initialize(void);



#ifdef __cplusplus
}
#endif

#endif /* __MPE_DR_H_INCLUDED__ */
