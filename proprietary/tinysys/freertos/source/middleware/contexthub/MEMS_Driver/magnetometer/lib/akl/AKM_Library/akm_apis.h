
#ifndef INCLUDE_AKM_APIS_H
#define INCLUDE_AKM_APIS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AKM_Common.h"

#include "AKL_APIs.h"

#include "AKM_CustomerSpec.h"
#include "akl_smart_compass.h"

typedef struct AKM_DATA_FILTER_BUFFER{
    struct AKM_SENSOR_DATA data_package;
    struct AKM_DATA_FILTER_BUFFER *next;
}akm_data_filter_buffer;


int16_t AKM_LibraryInit(void);
int16_t AKM_LoadAndStart(void);
int16_t AKM_StopAndSave(void);
int16_t AKM_SetData(struct AKM_SENSOR_DATA *data);
int16_t AKM_GetData(int32_t result[6], int senor_type, int *accuracy);

#endif
