#ifndef INCLUDE_AKM_APIS_H
#define INCLUDE_AKM_APIS_H

#include "../AKM_Common.h"

int16_t AKM_LibraryInit(
    uint32_t device,
    uint32_t layout_pat,
    uint8_t  *pdc
);

int16_t AKM_InitCalibrate(int8_t mode);

int16_t AKM_InitFusion(int8_t mode);
int16_t AKM_LoadAndStartCalibrate(void);

int16_t AKM_LoadAndStartFusion(void);

int16_t AKM_StopAndSave(void);

/*------WARNING------
 *   SET DATA UNIT
 *	MAG: uT
 *	ACC: m/s2
 *	GYR: rad/s
 *	timestamp: us
 */
int16_t AKM_SetData(struct AKM_SENSOR_DATA *data);

int16_t AKM_SetFusionData(struct AKM_SENSOR_DATA *data);

/*------WARNING------
*	GET DATA UNIT
*  MAG: uT
*  ACC: m/s2
*  GYR: rad/s
*  ORI: deg
*  GRAVITY: m/s2
*  Linear ACC: m/s2
*  QUAT: 1
*  timestamp: us
*/
int16_t AKM_GetData(
    float32_t result[6],
    int       senor_type,
    int32_t   *accuracy
);

int16_t AKL_LoadCaliInfo(int32_t cali_data[6]);
int16_t AKL_SaveCaliInfo(int32_t cali_data[6]);

void CalibrateDataSort(struct AKM_SENSOR_DATA *event);
void CalibrateBufferInit(void);
void CalibrateBufferDeinit(void);

void FusionDataSort(struct AKM_SENSOR_DATA *event);
void FusionBufferInit(int16_t has_gyro);
void FusionBufferDeinit(void);


#endif
