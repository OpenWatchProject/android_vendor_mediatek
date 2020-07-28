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

when              who       what, where, why
----------      ----      ----------------------------------------------------
12.25            zhangyx     V1.0 Create
=============================================================================*/

#include <memsic_wrapper.h>
//#include <MemsicCustomSpec.h>
#include <MemsicNineAxisFusion.h>
#include <MemsicCommon.h>
#include <platform.h>

//static int has_Gyro = 0;

float raw_acc[3] = {0.0, 0.0, 9.8};
float raw_mag[3] = {0.0, 0.0, 0.0};
float raw_gyro[3] = {0.0, 0.0, 0.0};

static int iRestart = 1; //algo restart

int MEMSIC_InitLib(int hwGyroSupport)
{

    return 0;
}

int MEMSIC_FusionSetGyroData(float data_x, float data_y, float data_z, int64_t time_stamp)
{
    //fusion sensor output
    float gravity[3];
    float linearAcc[3];
    float gameQuat[4];

    float gyro_sample = 0.02; //gyro sample rate 50HZ=0.02s

    int64_t now_timestamp = 0, delt_timestamp = 0;
    static uint64_t pre_timestamp = 0;

    //read the gyro raw data and convert rad/s
    raw_gyro[0] = data_x;
    raw_gyro[1] = data_y;
    raw_gyro[2] = data_z;

    now_timestamp = time_stamp;
    delt_timestamp = now_timestamp - pre_timestamp;
    pre_timestamp = now_timestamp;

    gyro_sample = (float)delt_timestamp / ((1000 * 1000 * 1000) * 1.0f);
    if ((gyro_sample < 0) || (gyro_sample > 0.04))
        gyro_sample = 0.02f;

    //algo main function
    //osLog(LOG_INFO, "FUSION: acc[%f, %f, %f], gyro[%f, %f, %f],TS =%f, iRestart = %d\n",
    //(double)raw_acc[0], (double)raw_acc[1], (double)raw_acc[2],
    //(double)raw_gyro[0], (double)raw_gyro[1], (double)raw_gyro[2],
    //(double)gyro_sample, iRestart);
    UpdateIMU(raw_acc, raw_gyro, gyro_sample, iRestart, gravity, linearAcc, gameQuat);
    iRestart = 0;

    //osLog(LOG_INFO, "FUSION: gameQuat[%f, %f, %f, %f] \n",
    //(double)gameQuat[0], (double)gameQuat[1], (double)gameQuat[2],(double)gameQuat[3]);

    return 1;
}

int MEMSIC_FusionSetAccData(float data_x, float data_y, float data_z, int64_t time_stamp)
{
    //read the acc raw data and convert m/s2
    raw_acc[0] = data_x;
    raw_acc[1] = data_y;
    raw_acc[2] = data_z;
    return 1;
}

int MEMSIC_FusionSetMagData(float data_x, float data_y, float data_z, int64_t time_stamp)
{
    raw_mag[0] = data_x;
    raw_mag[1] = data_y;
    raw_mag[2] = data_z;

    //osLog(LOG_ERROR, "%s: x:%f, y:%f, z:%f\n", __func__, (double)raw_mag[0], (double)raw_mag[1], (double)raw_mag[2]);
    return 1;
}

int MEMSIC_GetGravity(float *outData, int16_t *accuracy)
{
    float gravity_vec[3] = {0.0, 0.0, 1.0};
    GetGravity(gravity_vec);
    outData[0] = gravity_vec[0];
    outData[1] = gravity_vec[1];
    outData[2] = gravity_vec[2];
    return 1;
}

int MEMSIC_GetRotaionVector(float *outData, int16_t *accuracy)
{

    return 1;
}
int MEMSIC_GetOrientaion(float *outData, int16_t *accuracy)
{
    return 1;
}

int MEMSIC_GetLinearaccel(float *outData, int16_t *accuracy)
{
    float linear_acc_vec[3] = {0.0, 0.0, 0.0};
    GetLinearAcc(linear_acc_vec);
    outData[0] = linear_acc_vec[0];
    outData[1] = linear_acc_vec[1];
    outData[2] = linear_acc_vec[2];
    return 1;
}

int MEMSIC_GetGameRotaionVector(float *outData, int16_t *accuracy)
{
    float Game_rov_vec[4] = {0};
    GetGameRotVec(Game_rov_vec);
    outData[0] = Game_rov_vec[1];
    outData[1] = Game_rov_vec[2];
    outData[2] = Game_rov_vec[3];
    outData[3] = Game_rov_vec[0];
    return 1;
}

int MEMSIC_GetGeoMagnetic(float *outData, int16_t *accuracy)
{

    return 1;
}

int MEMSIC_GetVirtualGyro(float *outData, int16_t *accuracy)
{
    return 1;
}
