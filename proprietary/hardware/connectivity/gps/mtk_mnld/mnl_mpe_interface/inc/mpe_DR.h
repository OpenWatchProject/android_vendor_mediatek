#ifndef __MPE_DR_H_INCLUDED__
#define __MPE_DR_H_INCLUDED__

#include <pthread.h>
#include "stdint.h"
#include "mpe_Attitude.h"

/******************************************************************************
* Version Info
******************************************************************************/

#define MPE_VERSION_HEAD 'M','P','E','_','V','E','R','_'
#define MPE_MAJOR_VERSION '1','9','0','9','2','3','0','1' // y,y,m,m,d,d,rev,rev
#define MPE_BRANCH_INFO '_','4','.','0','0','_'
#define MPE_MINER_VERSION '0','0'

#define MPE_VER_INFO MPE_VERSION_HEAD,MPE_MAJOR_VERSION,MPE_BRANCH_INFO,MPE_MINER_VERSION

#define MPE_GRAVITY_EARTH   9.8066f

#define MPE_LOG_BUF_POOL_LEN         15 * 1024  // buffer pool length
#define MPE_LOG_BUF_POOL_PROC_LEN    13 * 1024  // buffer pool process length
#define MPE_SENSOR_NAME_LEN          50         // sensor name length

#define MPE_ACC_DEFAULT_RATE         50         // default acc sampling rate (Hz)
#define MPE_GYRO_DEFAULT_RATE        50         // default gyro sampling rate (Hz)
#define MPE_MAG_DEFAULT_RATE         5          // default mag sampling rate (Hz)
#define MPE_BARO_DEFAULT_RATE        50         // default baro sampling rate (Hz)

#define MPE_200HZ_TIME_THRESHOLD     15         // (ms)
#define MPE_200HZ_COUNT_THRESHOLD    3

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
    float   velocityInfo[3]; /* Least Square velocity one sigma error in meter per second under (N,E,D) frame */
    float   HDOP; /* horizontal dilution of precision value in unitless */
    float   confidenceIndex[3]; /* GPS confidence index */
    float   GNSS_extra_info[5]; /* GNSS_extra_info */
    unsigned int   leap_sec; /* correct GPS time with phone kernel time */
    double   gps_sec; /* Timestamp of GPS location */
    signed long long  gps_kernel_time;
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

typedef struct MPE_SENSOR_NAME {
    int acc_freq;
    int gyro_freq;
    int mag_freq;
    char acc_name[MPE_SENSOR_NAME_LEN];
    char gyro_name[MPE_SENSOR_NAME_LEN];
    char mag_name[MPE_SENSOR_NAME_LEN];
} MPE_SENSOR_NAME;

typedef enum{
    MTK_MPE_SYS_FLAG = 0x00,
    MTK_MPE_KER_FLAG,
    MTK_MPE_INFO_FLAG,
    MTK_MPE_VER_FLAG,
    MTK_MPE_MAX_FLAG
}MTK_VALID_FLAG_TYPE;

typedef enum {
    MTK_MPE_SENSOR_INIT = 0x00,
    MTK_MPE_SENSOR_AT_50HZ,
    MTK_MPE_SENSOR_CHANGE_TO_200HZ,
    MTK_MPE_SENSOR_AT_200HZ
} MTK_SENSOR_FREQ_TYPE;

int mpe_update_dead_reckoning(LPIMU pImu, int deltaTime_us, MNL2MPE_AIDING_DATA *pGPS);
int get_map_matching_result(LPIMU pImu, MNL2MPE_AIDING_DATA *pGPS, MPE2MNL_AIDING_DATA dr_position, MNL2MPE_AIDING_DATA *pMAP);
int mpe_get_dr_position(MPE2MNL_AIDING_DATA *pos);
int mpe_get_dr_gyro_bias(float *bias);
int mpe_get_dr_acc_bias(float *bias);
int mpe_dr_re_initialize(MPE_SENSOR_NAME pSensor);
int mpe_get_dr_entry(LPIMU pImu, MNL2MPE_AIDING_DATA *pGPS, MPE2MNL_AIDING_DATA *mpe3_out, MPE2MNL_AIDING_DATA *mpe4_out, uint64_t sensor_utc);
int mpe_dr_deinitialize(void);

extern char mpe_log_buf_pool[MPE_LOG_BUF_POOL_LEN];
extern int buf_pool_idx;

extern char accelerometer_name[MPE_SENSOR_NAME_LEN];
extern char gyroscope_name[MPE_SENSOR_NAME_LEN];
extern char magnetic_name[MPE_SENSOR_NAME_LEN];

extern pthread_mutex_t mpe_sensor_freq_mutex;
extern pthread_cond_t mpe_sensor_freq_cond;
extern int mpe_sensor_freq_flag;

#ifdef __cplusplus
}
#endif

#endif /* __MPE_DR_H_INCLUDED__ */
