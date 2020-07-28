#ifndef _AKM_WRAPPER_H_
#define _AKM_WRAPPER_H_
int AKM_Open(void);
int AKM_Close(void);

int AKM_SetMagData(int32_t data_x, int32_t data_y, int32_t data_z, int64_t time_stamp);
int AKM_SetGyroData(double data_x, double data_y, double data_z, int64_t time_stamp);
int AKM_Calibrate(double *cali_data, double *offset, int16_t *accuracy);
void AKM_SetDevice(uint8_t *wia);
void AKM_ReloadContext(float *offset);

#ifdef CFG_AKM_FUSION_SUPPORT
int AKM_FusionSetAccData(double data_x, double data_y, double data_z, int64_t time_stamp);
int AKM_FusionSetMagData(double data_x, double data_y, double data_z, int64_t time_stamp);
int AKM_FusionSetGyroData(double data_x, double data_y, double data_z, int64_t time_stamp);
int AKM_GetOri(double *cali_data, int16_t *accuracy);
#ifdef CFG_AKM_PG_SUPPORT
int AKM_GetGyroscope(double *cali_data, int16_t *accuracy);
int AKM_GetRotationVector(double *cali_data, int16_t *accuracy);
int AKM_GetLinearAccelerometer(double *cali_data, int16_t *accuracy);
int AKM_GetGravity(double *cali_data, int16_t *accuracy);
#endif
#endif

#endif
