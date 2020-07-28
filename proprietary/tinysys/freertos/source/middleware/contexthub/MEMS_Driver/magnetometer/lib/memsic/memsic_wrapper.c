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
#include <seos.h>
#include <util.h>
#include <sensors.h>
#include <plat/inc/rtc.h>

#include "MemsicOri.h"
#include "MemsicGyr.h"
#include "MemsicRov.h"
#include "MemsicConfig.h"
#include "MemsicCustomSpec.h"
#include "platform.h"
#include "MemsicNineAxisFusion.h"

#include "memsic_wrapper.h"

static int has_Gyro = 0;
float raw_acc[3] = {0.0, 0.0, 9.8};
float raw_gyro[3] = {0.0, 0.0, 0.0};

int MEMEMSIC_SetMagPara(int32_t *calmagpara)
{
    float iniCalPara[4] = {0, 0, 0, 50};
    iniCalPara[0] = calmagpara[0] / 1000.0f;
    iniCalPara[1] = calmagpara[1] / 1000.0f;
    iniCalPara[2] = calmagpara[2] / 1000.0f;
    iniCalPara[3] = calmagpara[3] / 1000.0f;
    if (has_Gyro) {
        IniPara iniPara;
        if ((myabs(iniCalPara[0]) < 1e-6) && (myabs(iniCalPara[1]) < 1e-6) && (myabs(iniCalPara[2]) < 1e-6)) {
            iniPara.iniAccuracy = 0;
        } else {
            iniPara.iniAccuracy = 3;
        }
        iniPara.iniCalPara = iniCalPara;
        iniPara.t = ts0;
        iniPara.si = (float*)si;
        iniPara.magVar = magVar;
        iniPara.enableGyroCal = enableGyroCal;
        iniPara.outlierGyroRestart = outlierGyroRestart;
        iniPara.iniGyroBias = iniGyroBias;
        iniPara.gyroGrade = gyroGrade;
        iniPara.extraSampleCount = extraSampleCount;
        InitializeAlgo(iniPara);
    } else {
        AlgoInitial(iniCalPara);
    }
    return 0;
}

int  MEMEMSIC_GetMagPara(int32_t *caliParameter)
{
    if (has_Gyro) {
        int Accuracy = 0;
        static int LastAccuracy = 0;
        float cal_para[6];
        Accuracy = GetAccuracy();
        if (LastAccuracy != 3 && Accuracy == 3) {
            GetCalibrationPara(cal_para);
            caliParameter[0] = (int32_t)(cal_para[0] * 1000);
            caliParameter[1] = (int32_t)(cal_para[1] * 1000);
            caliParameter[2] = (int32_t)(cal_para[2] * 1000);
            caliParameter[3] = (int32_t)(cal_para[3] * 1000);
            caliParameter[4] = (int32_t)(cal_para[4] * 1000);
            caliParameter[5] = (int32_t)(cal_para[5] * 1000);
            //osLog(LOG_ERROR, "%s %ld, %ld,%ld,%ld,%ld\n\r",
            // __func__,caliParameter[0],caliParameter[1],caliParameter[2],caliParameter[3],caliParameter[4]);
        }
        LastAccuracy = Accuracy;
    } else {
        int Accuracy = 0;
        static int LastAccuracy = 0;
        float cal_para[7];
        Accuracy = GetMagAccuracy();
        if (LastAccuracy != 3 && Accuracy == 3) {
            GetCalPara_6(cal_para);
            caliParameter[0] = (int32_t)(cal_para[0] * 1000);
            caliParameter[1] = (int32_t)(cal_para[1] * 1000);
            caliParameter[2] = (int32_t)(cal_para[2] * 1000);
            caliParameter[3] = (int32_t)(cal_para[3] * 1000);
            caliParameter[4] = (int32_t)(cal_para[4] * 1000);
            caliParameter[5] = (int32_t)(cal_para[5] * 1000);
            //osLog(LOG_ERROR, "%s %ld, %ld,%ld,%ld,%ld\n\r",
            //     __func__,caliParameter[0],caliParameter[1],caliParameter[2],caliParameter[3],caliParameter[4]);
        }
        LastAccuracy = Accuracy;
    }
    return 0;
}

int MEMSIC_DoCaliAPI(struct magDataInPut *inputMag, struct magDataOutPut *outputMag, int iRestart)
{
    float raw_mag[3] = {0.0, 0.0, 0.0};
    float cali_mag[3] = {0.0, 0.0, 0.0};
    float mag_offset[3] = {0.0, 0.0, 0.0};
    float calmag_para[6];
    static uint64_t pre_timestamp = 0;
    int64_t delt_timestamp = 0;
    float ts = 0.0;
    raw_mag[0] = inputMag->x;
    raw_mag[1] = inputMag->y;
    raw_mag[2] = inputMag->z;
    delt_timestamp = inputMag->timeStamp - pre_timestamp;
    pre_timestamp = inputMag->timeStamp;
    ts = (float)delt_timestamp / (1e9); //convert to seconds
    if (ts < 0 || ts > 0.04)
        ts = 0.02f;
    if (has_Gyro) {
        raw_mag[0] = raw_mag[0] * 100;
        raw_mag[1] = raw_mag[1] * 100;
        raw_mag[2] = raw_mag[2] * 100;
        //osLog(LOG_INFO, "Input:Mag[%f, %f, %f]\n", (double)raw_mag[0], (double)raw_mag[1], (double)raw_mag[2]);
        //osLog(LOG_INFO, "Input:Acc[%f, %f, %f]\n", (double)raw_acc[0], (double)raw_acc[1], (double)raw_acc[2]);
        //osLog(LOG_INFO, "Input:Gyro[%f, %f, %f]\n", (double)raw_gyro[0], (double)raw_gyro[1], (double)raw_gyro[2]);

        MainAlgoProcess(raw_acc, raw_mag, raw_gyro , ts, iRestart, 1);
        GetCalibratedMag(cali_mag);
        NineAxisFusion(raw_acc, cali_mag, raw_gyro, ts, iRestart);
        outputMag->x = cali_mag[0];
        outputMag->y = cali_mag[1];
        outputMag->z = cali_mag[2];
        //osLog(LOG_INFO, "Output:Mag[%f, %f, %f], ts=%f\n", (double)cali_mag[0], (double)cali_mag[1], (double)cali_mag[2], (double)ts);
        GetCalibrationPara(calmag_para);
        outputMag->x_bias = calmag_para[0];
        outputMag->y_bias = calmag_para[1];
        outputMag->z_bias = calmag_para[2];
        outputMag->status  = GetMagCalAccuracy();
    } else {
        MainAlgorithmProcess(raw_acc, raw_mag);
        GetCalMag(cali_mag);
        outputMag->x = cali_mag[0] * 100;
        outputMag->y = cali_mag[1] * 100;
        outputMag->z = cali_mag[2] * 100;
        CalcMemsicGyro(cali_mag, raw_acc, ts, iRestart);
        CalcMemsicRotVec(cali_mag, raw_acc);
        outputMag->status = GetMagAccuracy();
        GetOffset(mag_offset);
        outputMag->x_bias = mag_offset[0] * 100;
        outputMag->y_bias = mag_offset[1] * 100;
        outputMag->z_bias = mag_offset[2] * 100;
    }
    return 0;
}

int MEMSIC_InitLib(int hwGyroSupport)
{
    has_Gyro = hwGyroSupport;
    return 0;
}

int MEMSIC_FusionSetGyroData(struct InterfaceDataIn *inGyr)
{
    //fusion_gyro[0]=inGyr->vec[0];
    //fusion_gyro[1]=inGyr->vec[1];
    //fusion_gyro[2]=inGyr->vec[2];
    //fusion_gyro_time = inGyr->timeStamp;
    return 0;
}
int MEMSIC_CaliApiSetGyroData(struct magDataInPut *inGyr)
{
    raw_gyro[0] = inGyr->x;
    raw_gyro[1] = inGyr->y;
    raw_gyro[2] = inGyr->z;
    return 0;
}
int MEMSIC_FusionSetAccData(struct InterfaceDataIn *inAcc)
{
    raw_acc[0] = inAcc->vec[0];
    raw_acc[1] = inAcc->vec[1];
    raw_acc[2] = inAcc->vec[2];
    return 0;
}

int MEMSIC_FusionSetMagData(struct InterfaceDataIn *inMag)
{
    //Log(LOG_ERROR, "%s Enter\n",__func__);
    //fusion_mag[0]=inMag->vec[0];
    //fusion_mag[1]=inMag->vec[1];
    //fusion_mag[2]=inMag->vec[2];
    return 0;
}

int MEMSIC_GetGravity(struct InterfaceDataOut *outGra)
{

    float gravity_vec[3] = {0.0, 0.0, 1.0};
    if (has_Gyro) {
        GetGravity(gravity_vec);
        outGra->vec[0] = gravity_vec[0];
        outGra->vec[1] = gravity_vec[1];
        outGra->vec[2] = gravity_vec[2];
        outGra->status = GetAccuracy();
    } else {
        GetMemsicGravityAcc(gravity_vec);
        outGra->vec[0] = gravity_vec[0] * 9.8;
        outGra->vec[1] = gravity_vec[1] * 9.8;
        outGra->vec[2] = gravity_vec[2] * 9.8;
        outGra->status = GetMagAccuracy();
    }
    return 0;
}

int MEMSIC_GetRotaionVector(struct InterfaceDataOut *outRotVec)
{
    float cali_rov_vec[4] = {0};
    if (has_Gyro) {
        GetRotVec(cali_rov_vec);
        outRotVec->vec[0] = cali_rov_vec[0];
        outRotVec->vec[1] = cali_rov_vec[1];
        outRotVec->vec[2] = cali_rov_vec[2];
        outRotVec->vec[3] = cali_rov_vec[3];
        outRotVec->status = GetAccuracy();
    } else {
        GetCalRov(cali_rov_vec);
        outRotVec->vec[0] = cali_rov_vec[0];
        outRotVec->vec[1] = cali_rov_vec[1];
        outRotVec->vec[2] = cali_rov_vec[2];
        outRotVec->vec[3] = cali_rov_vec[3];
        outRotVec->status = GetMagAccuracy();
    }
    return 0;
}
int MEMSIC_GetOrientaion(struct InterfaceDataOut *outOri)
{
    float ypr[3] = {0};
    if (has_Gyro) {
        GetOri(ypr);
        outOri->vec[0] = ypr[0];
        outOri->vec[1] = ypr[1];
        outOri->vec[2] = ypr[2];
        outOri->status = GetAccuracy();
    } else {
        GetCalOri(ypr);
        outOri->vec[0] = ypr[0];
        outOri->vec[1] = ypr[1];
        outOri->vec[2] = ypr[2];
        outOri->status = GetMagAccuracy();
    }
    return 0;
}

int MEMSIC_GetLinearaccel(struct InterfaceDataOut *outLinAcc)
{
    float linear_acc_vec[3] = {0.0, 0.0, 0.0};
    if (has_Gyro) {
        GetLinearAcc(linear_acc_vec);
        outLinAcc->vec[0] = linear_acc_vec[0];
        outLinAcc->vec[1] = linear_acc_vec[1];
        outLinAcc->vec[2] = linear_acc_vec[2];
        outLinAcc->status = GetAccuracy();
    } else {
        GetMemsicLinearAcc(linear_acc_vec);
        outLinAcc->vec[0] = (float)linear_acc_vec[0] * 9.8f;
        outLinAcc->vec[1] = (float)linear_acc_vec[1] * 9.8f;
        outLinAcc->vec[2] = (float)linear_acc_vec[2] * 9.8f;
        outLinAcc->status = GetMagAccuracy();
    }
    return 0;
}

int MEMSIC_GetGameRotaionVector(struct InterfaceDataOut *outGamRV)
{
    float cali_rov_vec[4] = {0};
    if (has_Gyro) {
        GetRotVec(cali_rov_vec);
        outGamRV->vec[0] = cali_rov_vec[0];
        outGamRV->vec[1] = cali_rov_vec[1];
        outGamRV->vec[2] = cali_rov_vec[2];
        outGamRV->vec[3] = cali_rov_vec[3];
        outGamRV->status = GetAccuracy();
    } else {
        GetCalRov(cali_rov_vec);
        outGamRV->vec[0] = cali_rov_vec[0];
        outGamRV->vec[1] = cali_rov_vec[1];
        outGamRV->vec[2] = cali_rov_vec[2];
        outGamRV->vec[3] = cali_rov_vec[3];
        outGamRV->status = GetMagAccuracy();
    }

    return 0;
}

int MEMSIC_GetGeomagRotationVector(struct InterfaceDataOut *outGeoRV)
{
    float cali_rov_vec[4] = {0};
    if (has_Gyro) {
        GetRotVec(cali_rov_vec);
        outGeoRV->vec[0] = cali_rov_vec[0];
        outGeoRV->vec[1] = cali_rov_vec[1];
        outGeoRV->vec[2] = cali_rov_vec[2];
        outGeoRV->vec[3] = cali_rov_vec[3];
        outGeoRV->status = GetAccuracy();
    } else {
        GetCalRov(cali_rov_vec);
        outGeoRV->vec[0] = cali_rov_vec[0];
        outGeoRV->vec[1] = cali_rov_vec[1];
        outGeoRV->vec[2] = cali_rov_vec[2];
        outGeoRV->vec[3] = cali_rov_vec[3];
        outGeoRV->status = GetMagAccuracy();
    }
    return 0;
}

int MEMSIC_GetGeoMagnetic(struct InterfaceDataOut *outGeoMag)
{
    static float cali_mag_vec[3] = {0};
    if (has_Gyro) {
        GetCalibratedMag(cali_mag_vec);
        outGeoMag->vec[0] = cali_mag_vec[0];
        outGeoMag->vec[1] = cali_mag_vec[1];
        outGeoMag->vec[2] = cali_mag_vec[2];
        outGeoMag->status = GetAccuracy();
    } else {
        GetCalMag(cali_mag_vec);
        outGeoMag->vec[0] = cali_mag_vec[0] * 100;
        outGeoMag->vec[1] = cali_mag_vec[1] * 100;
        outGeoMag->vec[2] = cali_mag_vec[2] * 100;
        outGeoMag->status = GetMagAccuracy();
    }
    return 0;
}

int MEMSIC_GetVirtualGyro(struct InterfaceDataOut *outVirGyr)
{
    if (!has_Gyro) {
        float cali_gyr_vec[3] = {0};
        GetCalGyr(cali_gyr_vec);
        outVirGyr->vec[0] = cali_gyr_vec[0];
        outVirGyr->vec[1] = cali_gyr_vec[1];
        outVirGyr->vec[2] = cali_gyr_vec[2];
        outVirGyr->status = GetMagAccuracy();
    }
    return 0;
}

