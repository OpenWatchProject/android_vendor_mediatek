/******************************************************************************
 *
 *  $Id: $
 *
 * -- Copyright Notice --
 *
 * Copyright (c) 2004 Asahi Kasei Microdevices Corporation, Japan
 * All Rights Reserved.
 *
 * This software program is the proprietary program of Asahi Kasei Microdevices
 * Corporation("AKM") licensed to authorized Licensee under the respective
 * agreement between the Licensee and AKM only for use with AKM's electronic
 * compass IC.
 *
 * THIS SOFTWARE IS PROVIDED TO YOU "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABLITY, FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT OF
 * THIRD PARTY RIGHTS, AND WE SHALL NOT BE LIABLE FOR ANY LOSSES AND DAMAGES
 * WHICH MAY OCCUR THROUGH USE OF THIS SOFTWARE.
 *
 * -- End Asahi Kasei Microdevices Copyright Notice --
 *
 ******************************************************************************/
#include "AKCommon.h"
#include "AkmApi.h"
#include "AKCompass.h"
#include "Measure.h"
#include "AK09911Register.h"
#include "ParameterIO.h"
#include "AKVersion.h"

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

const unsigned char PdcTable[27] =
{
    0x20,0x52,0xBA,0x2A,0x31,0xFF,0xFF,0x11,0xD6,
    0x37,0x86,0x55,0x26,0x47,0xFF,0x31,0xFF,0xFF,
    0x7F,0x9A,0xBF,0xFC,0xFF,0xFF,0x09,0x77,0xFF
};

#define AK09911_CUSTOM_DATA_ADDRESS  PdcTable
// this para should output the same X/Y/Z data
/*unsigned char mpad_pdc[27]=
{
    32,82,186,10,49,255,255,64,214,
    55,159,85,38,171,255,249,191,255,
    127,154,255,60,255,255,201,230,255
};*/

//parameters for X9 test
unsigned char mpad_pdc[27]=
{
    241,78,94,13,155,70,255,130,212,
    52,239,190,13,28,0,232,1,182,
    80,253,190,249,161,224,15,77,20
};

#ifdef USE_HEAP_MEMORY
static int16* bData;
static AKDEBUGINFO* debugData;
static AKSCPRMS* prms;
#else
int16 _bData[AKMD_BDATA_SIZE];
int16 *bData = _bData;
static AKDEBUGINFO _debugData;
static AKDEBUGINFO *debugData = &_debugData;
AKSCPRMS _prms;
AKSCPRMS *prms = &_prms;
#endif

uint8 var_ag_data[1796]; //maybe should int8

//AKSCPRMS And_prms;

static uint16 mag_initial_update_flag = 0;
//static uint16 gyro_num = 0;
static uint64_t prevHtm_ag = 0;
#if 0
static void setDebugData( int16_t errFlag);
static int16 getDecimator( int16 freq);
#endif
AKSCPRMS* GetPRMS(void)
{
    return prms;
}

int16* GetBDATA(void)
{
    return bData;
}

void AKM_ReadFUSEROM(void) {
    //prms->m_asa.u.x = 50;
    ReadFUSEROM(prms);
}

/*****************************************************************************/
void *AKD_GetCustomDataAddress( void)
{
    return (void *)AK09911_CUSTOM_DATA_ADDRESS;
}

int AKM_ForceCalibration(void)
{
    AKSC_SetHDOEaGLevel(
        prms->m_doeag_var,
        &prms->m_hlayout,
        &prms->m_ho,
        AKSC_HDST_UNSOLVED,
        1
    );
    prms->m_hdst = AKSC_HDST_UNSOLVED;

    return AKMAPI_SUCCESS;
}

/*****************************************************************************/

#if 1
int16_t pf;
AKTESTFAILED result;
#endif
uint8_t akm_wia[2] = {0};

void AKM_SetDevice(uint8_t *wia) {
    akm_wia[0] = wia[0];    // akm eric modify for mult lib
    akm_wia[1] = wia[1];    // akm eric modify for mult lib
    // printf("AKM_SetDevice %x,%x\n",akm_wia[0],akm_wia[1]);
}
void AKM_Open( void){
#ifndef AKMD_DISABLE_DOEPLUS
#error should disable doeplus
        int16_t  doep_size;
#endif

#ifdef USE_HEAP_MEMORY
    prms = malloc(sizeof(AKSCPRMS));
    bData = malloc(AKMD_BDATA_SIZE*sizeof(int16));
    debugData = malloc(sizeof(AKDEBUGINFO));
#endif

    InitAKSCPRMS( prms);
    if( LoadParameters( prms) == AKD_ERROR){
        SetDefaultPRMS( prms);
    }
    ReadFUSEROM( prms);

#ifndef AKMD_DISABLE_DOEPLUS
    /* malloc DOEPlus prms*/
    doep_size = AKSC_GetSizeDOEPVar();
    if (doep_size <= 0) {
        retValue = ERROR_GET_SIZE_DOEP;
        goto THE_END_OF_MAIN_FUNCTION;
    }
    prms->m_doep_var = (AKSC_DOEPVAR *)malloc(doep_size * sizeof(int32));
    if (prms.m_doep_var == NULL) {
        retValue = ERROR_MALLOC_DOEP;
        goto THE_END_OF_MAIN_FUNCTION;
    }
#endif

    prms->m_doeag_var = (AKSC_DOEAGVAR *) var_ag_data;

    Init_Measure( prms);

    //prms->m_pdcptr =mpad_pdc;  /// use this test array , to confirm soft-iron calibration code OK
    prms->m_pdcptr = 0;

    mag_initial_update_flag = 0;
}


/*****************************************************************************
  **** the unit of mx,my,mz is uT(Q16). And mx,my,mz should be Sensitivity Adjusted. (ASA)
  **** currTime is nanosecond.
  ****
  */
int16_t AKM_SetMagData(int regx, int regy, int regz, uint64_t currTime){
    //int16_t i;
    int16 ret;

    int64_t tmpDuration;

    //mag_freq = freq;

    //prms->m_hdt_ag = -1;
    prms->m_gdt_ag = -1;
    prms->m_hdt_ag = -1;

    // Inverse decomp, i.e. compose
    bData[0] = (int16)(1);
    bData[1] = (int16)(regx & 0xFF);
    bData[2] = (int16)((regx >> 8) & 0xFF);
    bData[3] = (int16)(regy & 0xFF);
    bData[4] = (int16)((regy >> 8) & 0xFF);
    bData[5] = (int16)(regz & 0xFF);
    bData[6] = (int16)((regz >> 8) & 0xFF);
    bData[7] = (int16)(AKMD_ST2);

    ret = GetMagneticVectorDecomp(
            bData,
            prms,
            0
     );
    //printf("zhx, %d, %d, %d\n\r",prms->m_hvec.u.x, prms->m_hvec.u.y, prms->m_hvec.u.z);
    // Check the return value
    if ((ret != AKRET_PROC_SUCCEED) && (ret != AKRET_FORMATION_CHANGED)) {
        printf("akm GetMagneticVector has failed (0x%04X).\n", ret);
    } else {
        if(mag_initial_update_flag ==1){
            // calculate time difference
            tmpDuration = currTime - prevHtm_ag;
            /*  Limit to 16-bit value */
            if (tmpDuration > 2047000000) {
                tmpDuration = 2047000000;
            }
            prms->m_hdt_ag = (int16)((tmpDuration * 16) / 1000000);
        }else{
            mag_initial_update_flag=1;
        }
        prevHtm_ag = currTime;
    }

    if ((mag_initial_update_flag == 1) && (prms->m_hdt_ag >= 0)) {
        ret = GetMagneticVectorOffset(prms);
        // Check the return value
        if ((ret != AKRET_PROC_SUCCEED) && (ret != AKRET_FORMATION_CHANGED)) {
           printf("akm %s(%d): GetMagneticVectorOffset has failed (0x%04X).\n", __FUNCTION__, __LINE__, ret);
        }
    }


    return AKMAPI_SUCCESS;
}


/*the unit of grx gry grz should be rps, currTime should be nanosecond*/
int16_t AKM_SetGyroData(const double grx, const double gry, const double grz, uint64_t currTime){
    int16 i;
    int64_t tmpDuration;
    int16 ret;

    //prms->m_gdt_ag = -1;
    prms->m_gdt_ag = -1;
    prms->m_hdt_ag = -1;

#if 1
    prms->m_gvec.v[0] = (int16)((float)grx * 16.f * 180.f / 3.1415926535f);
    prms->m_gvec.v[1] = (int16)((float)gry * 16.f * 180.f / 3.1415926535f);
    prms->m_gvec.v[2] = (int16)((float)grz * 16.f * 180.f / 3.1415926535f);
#else
    /* from radian to AKM format dps*16 */
    prms->m_gvec.v[0] = (int16)grx * 16 * 180 / 3.1415926535;
    prms->m_gvec.v[1] = (int16)gry * 16 * 180 / 3.1415926535;
    prms->m_gvec.v[2] = (int16)grz * 16 * 180 / 3.1415926535;
#endif

    /* buffering */
    for (i = CSPEC_GNAVE - 1; i >= 1; i--) {
        prms->m_gdata[i] = prms->m_gdata[i - 1];
    }
    prms->m_gdata[0] = prms->m_gvec;
    AKSC_GetGyroOffsetAuto(prms->m_gdata, CSPEC_GNAVE, AKSC_GDATA_MAX, AKSC_GDATA_MAXDIFF, &prms->m_GO);

    prms->m_gvec.u.x -= prms->m_GO.u.x;
    prms->m_gvec.u.y -= prms->m_GO.u.y;
    prms->m_gvec.u.z -= prms->m_GO.u.z;

    if(mag_initial_update_flag==1){
        tmpDuration = currTime - prevHtm_ag;
        /*  Limit to 16-bit value */
        if (tmpDuration > 2047000000) {
            tmpDuration = 2047000000;
        }
        prms->m_gdt_ag = (int16)((tmpDuration * 16) / 1000000);

        AKMDEBUG(AKMDBG_VECTOR, "gyr(dec)=%6d,%6d,%6d\n",
                 prms->m_gvec.u.x, prms->m_gvec.u.y, prms->m_gvec.u.z);

        if (prms->m_gdt_ag >= 0) {
            ret = GetMagneticVectorOffset(prms);
            // Check the return value
            if ((ret != AKRET_PROC_SUCCEED) && (ret != AKRET_FORMATION_CHANGED)) {
                printf("akm %s(%d): GetMagneticVectorOffset has failed (0x%04X).\n", __FUNCTION__, __LINE__, ret);
            }
        }
    }

    return AKMAPI_SUCCESS;
}

/*****************************************************************************
 * mag_cal[0] mag_cal[1] mag_cal[2]   Calibrated Magnetometer, Unit: uT
 * mag_offset[0] mag_offset[1] mag_offset[2]   Magnetometer offset, Unit: uT *
 */
int16_t AKM_Calibrate(
        double  mag_cal[],
        double  mag_offset[],
        int16_t  *accuracy
)
{
    mag_cal[0] = prms->m_hvec.u.x * CONVERT_AKSC_TO_MICROTESLA;
    mag_cal[1] = prms->m_hvec.u.y * CONVERT_AKSC_TO_MICROTESLA;
    mag_cal[2] = prms->m_hvec.u.z * CONVERT_AKSC_TO_MICROTESLA;

    mag_offset[0] = ((int32)(prms->m_ho.u.x) + prms->m_hbase.u.x) * CONVERT_AKSC_TO_MICROTESLA;
    mag_offset[1] = ((int32)(prms->m_ho.u.y) + prms->m_hbase.u.y) * CONVERT_AKSC_TO_MICROTESLA;
    mag_offset[2] = ((int32)(prms->m_ho.u.z) + prms->m_hbase.u.z) * CONVERT_AKSC_TO_MICROTESLA;

    *accuracy = (int16_t)prms->m_hdst;
    return AKMAPI_SUCCESS;
}


/*****************************************************************************/
void AKM_Close( void){
    SaveParameters( prms);
    mag_initial_update_flag = 0;
    prms->m_hdt_ag = -1;
    prms->m_gdt_ag = -1;

#ifndef AKMD_DISABLE_DOEPLUS
    /* free */
    if (prms->m_doep_var != NULL) {
        free(prms->m_doep_var);
        prms->m_doep_var = NULL;
    }
#endif

#ifdef USE_HEAP_MEMORY
    free( bData);
    free( prms);
    free( debugData);
#endif /* USE_HEAP_MEMORY */
}

/*****************************************************************************/
void AKM_GetLibraryInfo( AKLIBINFO* info){
    info->partno = AKSC_GetVersion_AlgorithmCode();
    info->major = AKSC_GetVersion_Major();
    info->minor = AKSC_GetVersion_Minor();
    info->variation = AKSC_GetVersion_Variation();
    info->revision = AKSC_GetVersion_Revision();
    info->datecode = AKSC_GetVersion_DateCode();
}

/*****************************************************************************/
AKDEBUGINFO* AKM_GetDebugInfo( void){
    return debugData;
}


/*****************************************************************************/
#if 0
/*****************************************************************************/
static void setDebugData( int16_t errFlag){
    debugData->errFlag = errFlag;

    debugData->ST1 = (uint8_t)bData[0];
    debugData->HXHL = (bData[2] << 8 | bData[1]);
    debugData->HYHL = (bData[4] << 8 | bData[3]);
    debugData->HZHL = (bData[6] << 8 | bData[5]);
    debugData->ST2 = (uint8_t)bData[7];

    debugData->ASAX = (uint8_t)prms->m_asa.u.x;
    debugData->ASAY = (uint8_t)prms->m_asa.u.y;
    debugData->ASAZ = (uint8_t)prms->m_asa.u.z;

    debugData->hdata_x = (int16_t)prms->m_hdata[0].u.x;
    debugData->hdata_y = (int16_t)prms->m_hdata[0].u.y;
    debugData->hdata_z = (int16_t)prms->m_hdata[0].u.z;

    debugData->hbase_x = (int32_t)prms->m_hbase.u.x;
    debugData->hbase_y = (int32_t)prms->m_hbase.u.y;
    debugData->hbase_z = (int32_t)prms->m_hbase.u.z;
}
#endif
/*****************************************************************************/
int16_t AKD_GetFormation( void)
{
    //return g_AkmFormation;
        return 0;
}

