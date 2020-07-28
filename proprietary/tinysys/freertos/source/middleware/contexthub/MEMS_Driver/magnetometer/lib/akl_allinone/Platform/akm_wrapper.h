#ifndef AKM_WRAPPER_INCLUDE_H
#define AKM_WRAPPER_INCLUDE_H

void AKM_ReloadContext(
    float *offset
);

void AKM_SetDevice(
    uint8_t *wia
);
void AKM_SetPDC(
    uint8_t *pdc
);

int AKM_SetGyroFlagForCalibrate(
    int32_t hasGyro
);

int AKM_SetGyroFlagForFusion(
    int32_t hasGyro
);

int AKM_Open(
    void
);
void AKM_FusionSetLatency(uint64_t lantency);
void AKM_DisableFusion(void);

int AKM_Close(
    void
);
void AKM_EnableCalibrate(void);

void AKM_EnableFusion(void);

int AKM_ReloadCaliInfoFromFile(int32_t CaliPara[6]);

int AKM_SaveCaliInfoToFile(int32_t CaliPara[6]);



void AKM_ReloadContext(
    float *offset
);

int akm_get_sensor_support(
    void
);

int AKM_SetMagData(
    int32_t data_x,
    int32_t data_y,
    int32_t data_z,
    int64_t time_stamp
);

int AKM_SetGyroData(
    double  data_x,
    double  data_y,
    double  data_z,
    int64_t time_stamp
);

int AKM_GetMagData(
    double  *cali_data,
    double  *offset,
    int16_t *accuracy
);

int AKM_FusionSetAccData(
    double  data_x,
    double  data_y,
    double  data_z,
    int64_t time_stamp
);

int AKM_FusionSetMagData(
    double  data_x,
    double  data_y,
    double  data_z,
    int64_t time_stamp
);

int AKM_FusionSetGyroData(
    double  data_x,
    double  data_y,
    double  data_z,
    int64_t time_stamp
);

int AKM_GetOri(
    double  *cali_data,
    int16_t *accuracy
);

int AKM_GetGyroscope(
    double  *cali_data,
    int16_t *accuracy
);

int AKM_GetRotationVector(
    double  *cali_data,
    int16_t *accuracy
);

int AKM_GetLinearAccelerometer(
    double  *cali_data,
    int16_t *accuracy
);

int AKM_GetGravity(
    double  *cali_data,
    int16_t *accuracy
);

#endif
