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

#include <vendor_fusion.h>
#include <magnetometer.h>

struct magDataOutPut {
    float x, y, z;
    float x_bias, y_bias, z_bias;
    int8_t status;
};
struct magDataInPut {
    int64_t timeStamp;
    float x, y, z;
};

int MEMSIC_DoCaliAPI(struct magDataInPut *inputMag, struct magDataOutPut *outputMag, int iRestart);
int MEMSIC_CaliApiSetGyroData(struct magDataInPut *inGyr);
int MEMSIC_InitLib(int hwGyroSupport);
int MEMSIC_FusionSetGyroData(struct InterfaceDataIn *inGyr);

int MEMSIC_FusionSetAccData(struct InterfaceDataIn *inAcc);
int MEMSIC_FusionSetMagData(struct InterfaceDataIn *inMag);
int MEMSIC_GetGravity(struct InterfaceDataOut *outGra);
int MEMSIC_GetRotaionVector(struct InterfaceDataOut *outRotVec);
int MEMSIC_GetOrientaion(struct InterfaceDataOut *outOri);
int MEMSIC_GetLinearaccel(struct InterfaceDataOut *outLinAcc);
int MEMSIC_GetGameRotaionVector(struct InterfaceDataOut *outGamRV);
int MEMSIC_GetGeoMagnetic(struct InterfaceDataOut *outGeoMag);
int MEMSIC_GetVirtualGyro(struct InterfaceDataOut *outVirGyr);

int MEMEMSIC_SetMagPara(int32_t *calmagpara);
int MEMEMSIC_GetMagPara(int32_t *caliParameter);


#endif  /* End include guard __MEMSIC_WRAPPER_H__ */


