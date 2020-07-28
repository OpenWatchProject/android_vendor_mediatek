
#define LOG_TAG "libtest"

//#include <cutils/log.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <FreeRTOSConfig.h>
#include <platform_mtk.h>
#include "FreeRTOS.h"
#include "task.h"
//#include "AKM_Common.h"
//#include "akm_apis.h"
#include "akm_apis.h"



#define PI                      3.14159265358979323846264338
#define Q16_TO_STD(x)    ((float32_t)(x)  / 65536.0f)
#define STD_TO_Q16(x)    ((float32_t)(x)  * 65536.0f)
#define RPS_TO_DPS      (180.f / PI)
#define DPS_TO_RPS      (PI / 180.f)


#define DATA_SIZE 3

uint8_t akm_wia[2] = {0};

void AKM_SetDevice(uint8_t *wia) {
    akm_wia[0] = wia[0];    // akm eric modify for mult lib
    akm_wia[1] = wia[1];    // akm eric modify for mult lib
    // printf("AKM_SetDevice %x,%x\n",akm_wia[0],akm_wia[1]);
}

int AKM_Open(void)
{
    int16_t ret = -1;
    ret = AKM_LibraryInit();
    if (ret != AKM_SUCCESS)
    {
        ALOGE("library_init failed\n");
        goto EXIT_AKM_OPEN;
    }
    ret = AKM_LoadAndStart();
    if (ret != AKM_SUCCESS)
    {
        ALOGE("load_and_start failed\n");
        goto EXIT_AKM_OPEN;
    }
    return AKM_SUCCESS;
EXIT_AKM_OPEN:
    return AKM_ERROR;
}

int AKM_Close(void)
{
    int16_t ret;
    ret = AKM_StopAndSave();
    ALOGE("AKM_Close\n");
    return ret;
}



int AKM_SetMagData(int32_t data_x, int32_t data_y, int32_t data_z, int64_t time_stamp)
{
    struct AKM_SENSOR_DATA sensordata;
    int ret;
    sensordata.stype = AKM_ST_MAG;
    sensordata.time_ns = time_stamp;

    sensordata.u.v[0] = data_x;
    sensordata.u.v[1] = data_y;
    sensordata.u.v[2] = data_z;

    #if 1
    sensordata.status[0] = 0x01;
    sensordata.status[1] = 0x00;//inputData->status & 0xff;
    //sensordata.device_id = (uint8_t)((inputData->status&0xff00)>>8);
    #endif
    // printf("AKM_SetMagData \n");
    ret = AKM_SetData(&sensordata);
    return ret;
}
int AKM_SetGyroData(double data_x, double data_y, double data_z, int64_t time_stamp)
{
    struct AKM_SENSOR_DATA sensordata;
    int16_t ret;
    sensordata.stype = AKM_ST_GYR;
    sensordata.time_ns = time_stamp;

    sensordata.u.v[0] = (int32_t)STD_TO_Q16((float)data_x * RPS_TO_DPS);
    sensordata.u.v[1] = (int32_t)STD_TO_Q16((float)data_y * RPS_TO_DPS);
    sensordata.u.v[2] = (int32_t)STD_TO_Q16((float)data_z * RPS_TO_DPS);

    ret = AKM_SetData(&sensordata);
    // printf("AKM_SetGyroData \n");
    return ret;
}

#ifdef CFG_AKM_FUSION_SUPPORT
int AKM_FusionSetAccData(double data_x, double data_y, double data_z, int64_t time_stamp)
{
    struct AKM_SENSOR_DATA sensordata;
    int ret;
    sensordata.stype = AKM_ST_FUSION_ACC;
    sensordata.time_ns = time_stamp;

    sensordata.u.v[0] = (int32_t)STD_TO_Q16((float)data_x );
    sensordata.u.v[1] = (int32_t)STD_TO_Q16((float)data_y );
    sensordata.u.v[2] = (int32_t)STD_TO_Q16((float)data_z );

    ret = AKM_SetData(&sensordata);
    return ret;
}
int AKM_FusionSetMagData(double data_x, double data_y, double data_z, int64_t time_stamp)
{
    struct AKM_SENSOR_DATA sensordata;
    int ret;
    sensordata.stype = AKM_ST_FUSION_MAG;
    sensordata.time_ns = time_stamp;

    sensordata.u.v[0] = (int32_t)STD_TO_Q16((float)data_x );
    sensordata.u.v[1] = (int32_t)STD_TO_Q16((float)data_y );
    sensordata.u.v[2] = (int32_t)STD_TO_Q16((float)data_z );

    ret = AKM_SetData(&sensordata);
    return ret;
}
int AKM_FusionSetGyroData(double data_x, double data_y, double data_z, int64_t time_stamp)
{
    struct AKM_SENSOR_DATA sensordata;
    int ret;
    sensordata.stype = AKM_ST_FUSION_GYR;
    sensordata.time_ns = time_stamp;

    sensordata.u.v[0] = (int32_t)STD_TO_Q16((float)data_x * RPS_TO_DPS);
    sensordata.u.v[1] = (int32_t)STD_TO_Q16((float)data_y * RPS_TO_DPS);
    sensordata.u.v[2] = (int32_t)STD_TO_Q16((float)data_z * RPS_TO_DPS);

    ret = AKM_SetData(&sensordata);
    return ret;
}
int AKM_GetOri(double *cali_data, int16_t *accuracy)
{
    int32_t sensor_result[6] = {0};
    int ret;
    int tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result,AKM_VT_ORI,&(tmp_accuracy));

    cali_data[0] = (float)Q16_TO_STD(sensor_result[0]);
    cali_data[1] = (float)Q16_TO_STD(sensor_result[1]);
    cali_data[2] = (float)Q16_TO_STD(sensor_result[2]);

    *accuracy = tmp_accuracy;
    return ret;
}
#ifdef CFG_AKM_PG_SUPPORT
int AKM_GetGyroscope(double *cali_data, int16_t *accuracy)
{
    int32_t sensor_result[6] = {0};
    int ret;
    int tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result,AKM_VT_GYR,&(tmp_accuracy));

    cali_data[0] = (float)Q16_TO_STD(sensor_result[0]);
    cali_data[1] = (float)Q16_TO_STD(sensor_result[1]);
    cali_data[2] = (float)Q16_TO_STD(sensor_result[2]);
    cali_data[0] = cali_data[0] * (double)DPS_TO_RPS;
    cali_data[1] = cali_data[1] * (double)DPS_TO_RPS;
    cali_data[2] = cali_data[2] * (double)DPS_TO_RPS;

    *accuracy = tmp_accuracy;
    return ret;
}
int AKM_GetRotationVector(double *cali_data, int16_t *accuracy)
{
    int32_t sensor_result[6] = {0};
    int ret;
    int tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result,AKM_VT_QUAT,&(tmp_accuracy));

    cali_data[0] = (float)Q16_TO_STD(sensor_result[0]);
    cali_data[1] = (float)Q16_TO_STD(sensor_result[1]);
    cali_data[2] = (float)Q16_TO_STD(sensor_result[2]);
    cali_data[3] = (float)Q16_TO_STD(sensor_result[3]);

    *accuracy = tmp_accuracy;
    return ret;
}
int AKM_GetLinearAccelerometer(double *cali_data, int16_t *accuracy)
{
    int32_t sensor_result[6] = {0};
    int ret;
    int tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result,AKM_VT_LACC,&(tmp_accuracy));

    cali_data[0] = (float)Q16_TO_STD(sensor_result[0]);
    cali_data[1] = (float)Q16_TO_STD(sensor_result[1]);
    cali_data[2] = (float)Q16_TO_STD(sensor_result[2]);

    *accuracy = tmp_accuracy;
    return ret;
}
int AKM_GetGravity(double *cali_data, int16_t *accuracy)
{
    int32_t sensor_result[6] = {0};
    int ret;
    int tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result,AKM_VT_GRAVITY,&(tmp_accuracy));

    cali_data[0] = (float)Q16_TO_STD(sensor_result[0]);
    cali_data[1] = (float)Q16_TO_STD(sensor_result[1]);
    cali_data[2] = (float)Q16_TO_STD(sensor_result[2]);

    *accuracy = tmp_accuracy;
    return ret;
}
#endif
#endif
int AKM_Calibrate(double *cali_data, double *offset, int16_t *accuracy)
{
    int32_t sensor_result[6] = {0};
    int ret;
    int tmp_accuracy = 0;
    #ifdef CFG_FAST_CALIBRATION_SUPPORT
    ret = AKM_GetData(sensor_result,AKM_VT_MAG_DOEAG,&(tmp_accuracy));
    #else
    ret = AKM_GetData(sensor_result,AKM_VT_MAG,&(tmp_accuracy));
    #endif

    cali_data[0] = (float)Q16_TO_STD(sensor_result[0]);
    cali_data[1] = (float)Q16_TO_STD(sensor_result[1]);
    cali_data[2] = (float)Q16_TO_STD(sensor_result[2]);

    offset[0] = (float)Q16_TO_STD(sensor_result[3]);
    offset[1] = (float)Q16_TO_STD(sensor_result[4]);
    offset[2] = (float)Q16_TO_STD(sensor_result[5]);

    *accuracy = tmp_accuracy;

    return ret;
}

