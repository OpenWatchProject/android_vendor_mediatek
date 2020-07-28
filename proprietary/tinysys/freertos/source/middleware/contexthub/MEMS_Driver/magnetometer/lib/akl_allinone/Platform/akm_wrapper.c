#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "akm_apis.h"
#include "akm_wrapper.h"

//#include "quat_test.h"

#define AKM_ALGO_9D  0x84 //AKL_MODE_FUSION_9D | AKL_MODE_CALIB_DOEAG
#define AKM_ALGO_6D  0x22 //AKL_MODE_FUSION_6D_PG_ON | AKL_MODE_CALIB_DOEEX

#define PI           ((float)3.14159265358979323846264338)

#define RAD2DEG      (180.f / PI)
#define DEG2RAD      (PI / 180.f)
#define NS2US(x)       (int64_t)(x / 1000)

#define Q16_TO_STD(x)  (x / 65536.f)
#define STD_TO_Q16(x)  (x * 65536.f)

static int init_flag = 0;
uint8_t    akm_wia[2] = {0};

#define SIZEOFPDC  27
uint8_t pdc_parameter[SIZEOFPDC] = {0};

int16_t has_gyro_flag = 0;
static int16_t fusion_use_sort = 0;
void AKM_SetDevice(uint8_t *wia)
{
    akm_wia[0] = wia[0]; // akm eric modify for mult lib
    akm_wia[1] = wia[1]; // akm eric modify for mult lib
    //printf("akm_log AKM_SetDevice %x, %x\n", akm_wia[0], akm_wia[1]);
}

void AKM_SetPDC(uint8_t *pdc)
{
    memcpy(pdc_parameter, pdc, SIZEOFPDC);
}


int AKM_Open(void)
{
    int ret = 0;

    if (init_flag == 0) {
        ret = AKM_LibraryInit(0x05, 0, pdc_parameter);
        if (ret == AKM_SUCCESS) {
            init_flag = 1;
        }
    }
    return ret;
}
int AKM_SetGyroFlagForCalibrate(int32_t hasGyro)
{
    int32_t cali_mode = 0x02;

    if (hasGyro == 1) {
        //DOEaG
        cali_mode = 0x04;
    } else if (hasGyro == 0) {
        //DOEEX
        cali_mode = 0x02;
    }
    has_gyro_flag = hasGyro;
    AKM_InitCalibrate(cali_mode);
    return 0;
}

int AKM_SetGyroFlagForFusion(int32_t hasGyro)
{
    int32_t fusion_mode = 0x20;

    if (hasGyro == 1)
        fusion_mode = 0x80; //9D
    else if (hasGyro == 0)
        fusion_mode = 0x20; //6D+PG
    fusion_mode = 0x20;
    //printf("fusion_mode:%x\n",fusion_mode);
    AKM_InitFusion(fusion_mode);
    return 0;
}
void AKM_EnableCalibrate(void)
{
    AKM_LoadAndStartCalibrate();
    CalibrateBufferInit();
}
void AKM_EnableFusion(void)
{
    AKM_LoadAndStartFusion();
    FusionBufferInit(has_gyro_flag);
}
void AKM_DisableFusion(void)
{
    FusionBufferDeinit();
}
void AKM_FusionSetLatency(uint64_t latency)
{
    printf("AKM_FusionSetLatency latency:%lld\n",latency);
    if (latency != 0) {
        fusion_use_sort = 1;
    } else {
        FusionBufferDeinit();
        fusion_use_sort = 0;
    }
}
int AKM_Close(void)
{
    int ret = 0;

    if (init_flag == 1) {
        ret = AKM_StopAndSave();
    }

    return ret;
}

int AKM_ReloadCaliInfoFromFile(int32_t calipara[6])
{
    int ret = 0;
    ret = AKL_LoadCaliInfo(calipara);
    return ret;
}

int AKM_SaveCaliInfoToFile(int32_t calipara[6])
{
    int ret = 0;
    ret = AKL_SaveCaliInfo(calipara);
    return ret;
}

int AKM_SetMagData(
    int32_t data_x,
    int32_t data_y,
    int32_t data_z,
    int64_t time_stamp)
{
    struct AKM_SENSOR_DATA sensordata;
    //int                    ret;
    sensordata.stype = AKM_VT_MAG;
    sensordata.time_stamp = NS2US(time_stamp);


    if (akm_wia[1] == 0x05) {
        sensordata.u.v[0] = (float)(data_x * 0.6f);
        sensordata.u.v[1] = (float)(data_y * 0.6f);
        sensordata.u.v[2] = (float)(data_z * 0.6f);
    } else if ((akm_wia[1] == 0x04) ||
               (akm_wia[1] == 0x08) ||
               (akm_wia[1] == 0x10) ||
               (akm_wia[1] == 0x09) ||
               (akm_wia[1] == 0x0b) ||
               (akm_wia[1] == 0x0c)) {
        sensordata.u.v[0] = (float)(data_x * 0.15f);
        sensordata.u.v[1] = (float)(data_y * 0.15f);
        sensordata.u.v[2] = (float)(data_z * 0.15f);
    }

    sensordata.status[0] = 0x01;
    sensordata.status[1] = 0x00;

    //printf("AKM_SetMagData data:%f,%f,%f,time:%lld\n",(double)sensordata.u.v[0],(double)sensordata.u.v[1],(double)sensordata.u.v[2],sensordata.time_stamp);

    if (has_gyro_flag == 0) {
        AKM_SetData(&sensordata);
    } else if (has_gyro_flag == 1) {
        CalibrateDataSort(&sensordata);
    }
    return 0;
}

int AKM_SetGyroData(
    double  data_x,
    double  data_y,
    double  data_z,
    int64_t time_stamp)
{
    struct AKM_SENSOR_DATA sensordata;
    //int16_t                ret;
    sensordata.stype = AKM_VT_GYR;
    sensordata.time_stamp = NS2US(time_stamp);

    sensordata.u.v[0] = (float)data_x;
    sensordata.u.v[1] = (float)data_y;
    sensordata.u.v[2] = (float)data_z;

    //printf("AKM_SetGyroData data:%f,%f,%f,time:%lld\n",(double)sensordata.u.v[0],(double)sensordata.u.v[1],(double)sensordata.u.v[2],sensordata.time_stamp);
    //ret = AKM_SetData(&sensordata);
    CalibrateDataSort(&sensordata);
    return 0;
}

int AKM_GetMagData(
    double  *cali_data,
    double  *offset,
    int16_t *accuracy)
{
    float32_t sensor_result[6] = {0};
    int       ret;
    int32_t   tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result, AKM_VT_MAG, &(tmp_accuracy));

    cali_data[0] = (float)(sensor_result[0]);
    cali_data[1] = (float)(sensor_result[1]);
    cali_data[2] = (float)(sensor_result[2]);

    offset[0] = (float)(sensor_result[3]);
    offset[1] = (float)(sensor_result[4]);
    offset[2] = (float)(sensor_result[5]);

    //printf("GetMag:%f,%f,%f,%d\n",(double)cali_data[0],(double)cali_data[1],(double)cali_data[2],tmp_accuracy);
    *accuracy = tmp_accuracy;

    return ret;
}

int AKM_FusionSetAccData(
    double  data_x,
    double  data_y,
    double  data_z,
    int64_t time_stamp)
{
    struct AKM_SENSOR_DATA sensordata;
    //int                    ret;
    sensordata.stype = AKM_VT_FUSION_ACC;
    sensordata.time_stamp = NS2US(time_stamp);

    sensordata.u.v[0] = (float)data_x;
    sensordata.u.v[1] = (float)data_y;
    sensordata.u.v[2] = (float)data_z;

    //printf("Acc:%f,%f,%f,time:%lld\n",(double)sensordata.u.v[0],(double)sensordata.u.v[1],(double)sensordata.u.v[2],sensordata.time_stamp);
    //printf("Acc:%lld,%d\n",sensordata.time_stamp,fusion_use_sort);

    AKM_SetFusionData(&sensordata);
    //if (fusion_use_sort == 0) {
    //   AKM_SetFusionData(&sensordata);
    //}else if (fusion_use_sort == 1) {
    //    FusionDataSort(&sensordata);
    //}
    return 0;
}

int AKM_FusionSetMagData(
    double  data_x,
    double  data_y,
    double  data_z,
    int64_t time_stamp)
{
    struct AKM_SENSOR_DATA sensordata;
    //int                    ret;
    sensordata.stype = AKM_VT_FUSION_MAG;
    sensordata.time_stamp = NS2US(time_stamp);

    sensordata.u.v[0] = (float)data_x;
    sensordata.u.v[1] = (float)data_y;
    sensordata.u.v[2] = (float)data_z;

    sensordata.status[0] = 0x01;
    sensordata.status[1] = 0x00;

    //printf("Mag:%f,%f,%f,time:%lld\n",(double)sensordata.u.v[0],(double)sensordata.u.v[1],(double)sensordata.u.v[2],sensordata.time_stamp);
    //printf("Mag:%lld,%d\n",sensordata.time_stamp,fusion_use_sort);
    AKM_SetFusionData(&sensordata);
    //if (fusion_use_sort == 0) {
    //    AKM_SetFusionData(&sensordata);
    //}else if (fusion_use_sort == 1) {
    //    FusionDataSort(&sensordata);
    //}
    return 0;
}

int AKM_FusionSetGyroData(
    double  data_x,
    double  data_y,
    double  data_z,
    int64_t time_stamp)
{
    struct AKM_SENSOR_DATA sensordata;
    //int                    ret;
    sensordata.stype = AKM_VT_FUSION_GYR;
    sensordata.time_stamp = NS2US(time_stamp);

    sensordata.u.v[0] = (float)data_x;
    sensordata.u.v[1] = (float)data_y;
    sensordata.u.v[2] = (float)data_z;

    //printf("Gyro:%f,%f,%f,time:%lld\n",(double)sensordata.u.v[0],(double)sensordata.u.v[1],(double)sensordata.u.v[2],sensordata.time_stamp);
    //printf("Gyro:%lld,%d\n",sensordata.time_stamp,fusion_use_sort);
    AKM_SetFusionData(&sensordata);
    //if (fusion_use_sort == 0) {
    //    AKM_SetFusionData(&sensordata);
    //}else if (fusion_use_sort == 1) {
    //    FusionDataSort(&sensordata);
    //}
    return 0;
}

int AKM_GetOri(
    double  *cali_data,
    int16_t *accuracy)
{
    float32_t sensor_result[6] = {0};
    int       ret;
    int32_t   tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result, AKM_VT_ORI, &(tmp_accuracy));

    cali_data[0] = (float)(sensor_result[0]);
    cali_data[1] = (float)(sensor_result[1]);
    cali_data[2] = (float)(sensor_result[2]);

    //printf("AKM_GetOri, ori:%f,%f,%f\n",cali_data[0],cali_data[1],cali_data[2]);
    *accuracy = tmp_accuracy;
    return ret;
}

int AKM_GetGyroscope(
    double  *cali_data,
    int16_t *accuracy)
{
    float32_t sensor_result[6] = {0};
    int       ret;
    int32_t   tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result, AKM_VT_GYR, &(tmp_accuracy));

    cali_data[0] = (float)(sensor_result[0]);
    cali_data[1] = (float)(sensor_result[1]);
    cali_data[2] = (float)(sensor_result[2]);

    //printf("AKM_GetGyroscope data output:%f,%f,%f\n",(double)sensor_result[0],(double)sensor_result[1],(double)sensor_result[2]);
    *accuracy = tmp_accuracy;
    return ret;
}

int AKM_GetRotationVector(
    double  *cali_data,
    int16_t *accuracy)
{
    float32_t sensor_result[6] = {0};
    int       ret;
    int32_t   tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result, AKM_VT_QUAT, &(tmp_accuracy));

    cali_data[0] = (float)(sensor_result[0]);
    cali_data[1] = (float)(sensor_result[1]);
    cali_data[2] = (float)(sensor_result[2]);
    cali_data[3] = (float)(sensor_result[3]);

    //printf("AKM_GetRotationVector data:%f,%f,%f,%f\n",(double)cali_data[0],(double)cali_data[1],(double)cali_data[2],(double)cali_data[3]);
    *accuracy = tmp_accuracy;
    return ret;
}

int AKM_GetLinearAccelerometer(
    double  *cali_data,
    int16_t *accuracy)
{
    float32_t sensor_result[6] = {0};
    int       ret;
    int32_t   tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result, AKM_VT_LACC, &(tmp_accuracy));

    cali_data[0] = (float)(sensor_result[0]);
    cali_data[1] = (float)(sensor_result[1]);
    cali_data[2] = (float)(sensor_result[2]);

    *accuracy = tmp_accuracy;
    return ret;
}

int AKM_GetGravity(
    double  *cali_data,
    int16_t *accuracy)
{
    float32_t sensor_result[6] = {0};
    int       ret;
    int32_t   tmp_accuracy = 0;
    ret = AKM_GetData(sensor_result, AKM_VT_GRAVITY, &(tmp_accuracy));

    cali_data[0] = (float)(sensor_result[0]);
    cali_data[1] = (float)(sensor_result[1]);
    cali_data[2] = (float)(sensor_result[2]);

    *accuracy = tmp_accuracy;
    return ret;
}

void *akm_malloc(unsigned int size)
{
    return pvPortMalloc(size);
}

void akm_free(void *pv)
{
    vPortFree(pv);
}
