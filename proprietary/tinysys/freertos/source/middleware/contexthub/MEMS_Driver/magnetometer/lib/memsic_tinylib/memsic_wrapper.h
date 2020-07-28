/*===========================================================================
*
* MAGNETIC SENSOR DRIVER
* Copyright (c) 2016, "Memsic Inc."
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. Neither the name of "Memsic Inc." nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

============================================================================*/
/*===========================================================================

REVISON HISTORY FOR FILE
This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when               who       what, where, why
----------         ----      ------------------------------------------------------------------------------
12  26, 2017       zhangyx      V1.0 Create
=============================================================================*/
#ifndef __MEMSIC_WRAPPER_H__
#define  __MEMSIC_WRAPPER_H__
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int MEMSIC_SetAData(int16_t Ax, int16_t Ay, int16_t Az,  int64_t  timestamp);
int MEMSIC_SetGData(int16_t Ax, int16_t Ay, int16_t Az, int16_t Gx, int16_t Gy, int16_t Gz, int64_t  timestamp);
int MEMSIC_SetMagData(float RawX, float RawY, float RawZ,  int64_t  timestamp);
int MEMSIC_Calibrate(float data_cali[3], float data_offset[3], int8_t *sf_status);


int MEMSIC_InitLib(int hwGyroSupport);
int MEMSIC_FusionSetGyroData(float data_x, float data_y, float data_z, int64_t time_stamp);
int MEMSIC_FusionSetAccData(float data_x, float data_y, float data_z, int64_t time_stamp);
int MEMSIC_FusionSetMagData(float data_x, float data_y, float data_z, int64_t time_stamp);
int MEMSIC_GetGravity(float *cali_data, int16_t *accuracy);
int MEMSIC_GetRotaionVector(float *cali_data, int16_t *accuracy);
int MEMSIC_GetOrientaion(float *cali_data, int16_t *accuracy);
int MEMSIC_GetLinearaccel(float *cali_data, int16_t *accuracy);
int MEMSIC_GetGameRotaionVector(float *cali_data, int16_t *accuracy);
int MEMSIC_GetGeoMagnetic(float *cali_data, int16_t *accuracy);
int MEMSIC_GetVirtualGyro(float *output, int16_t *accuracy);
void SetMagpara(float *calmagpara);

#endif  /* End include guard __MEMSIC_WRAPPER_H__ */


