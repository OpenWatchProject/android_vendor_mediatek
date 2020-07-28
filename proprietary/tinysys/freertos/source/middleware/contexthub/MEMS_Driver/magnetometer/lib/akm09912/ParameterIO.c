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
#include "ParameterIO.h"
//#include "AkmHal.h"
#include "AkmApi.h"
#include "./include/AKMDevice.h"
#include "CustomerSpec.h"
#include "AKCompass.h"

#define SIZE_PARAMETER_STORE ((2+6+6+12)*CSPEC_NUM_FORMATION)

//Test Variable
uint8 buf[SIZE_PARAMETER_STORE];


AK9911SAVEPRMS  ak_prms;

//call this API to get the max 64 bytes of data from algorithm lib, then the caller can save it to Flash
int16_t AKM_SaveContext(float offset[3])
{
    offset[0] = ak_prms.HSUC_HO[0].u.x * CONVERT_AKSC_TO_MICROTESLA;
    offset[1] = ak_prms.HSUC_HO[0].u.y * CONVERT_AKSC_TO_MICROTESLA;
    offset[2] = ak_prms.HSUC_HO[0].u.z * CONVERT_AKSC_TO_MICROTESLA;
    return 0;
}

//call this API to restore back the Max 64 bytes of data back to algorithm library
int16_t AKM_ReloadContext(float offset[3])
{
    ak_prms.HSUC_HO[0].u.x = (int16)(offset[0] / CONVERT_AKSC_TO_MICROTESLA);
    ak_prms.HSUC_HO[0].u.y = (int16)(offset[1] / CONVERT_AKSC_TO_MICROTESLA);
    ak_prms.HSUC_HO[0].u.z = (int16)(offset[2] / CONVERT_AKSC_TO_MICROTESLA);
    ak_prms.HSUC_HDST[0] = AKSC_HDST_L0;
    return 0;
}

int16 LoadParameters(
            AK09911PRMS*    prms    ///< A pointer to AK8963PRMS structure. Loaded
                                                    ///< parameter is stored to the member of this
                                                    ///< structure.
)
{
    int16   i;

//  printf("sizeofak_prms=%d\n",sizeof(ak_prms));

//  printf("AKSC_HDST=%d\n",sizeof(AKSC_HDST));

    for(i=0; i<CSPEC_NUM_FORMATION; i++){
        prms->HSUC_HDST[i] = ak_prms.HSUC_HDST[i];
        prms->HSUC_HO[i].u.x = ak_prms.HSUC_HO[i].u.x;
        prms->HSUC_HO[i].u.y = ak_prms.HSUC_HO[i].u.y;
        prms->HSUC_HO[i].u.z = ak_prms.HSUC_HO[i].u.z;
        prms->HFLUCV_HREF[i].u.x = ak_prms.HFLUCV_HREF[i].u.x;
        prms->HFLUCV_HREF[i].u.y =ak_prms.HFLUCV_HREF[i].u.y;
        prms->HFLUCV_HREF[i].u.z = ak_prms.HFLUCV_HREF[i].u.z;
        prms->HSUC_HBASE[i].u.x = ak_prms.HSUC_HBASE[i].u.x;
        prms->HSUC_HBASE[i].u.y = ak_prms.HSUC_HBASE[i].u.y;
        prms->HSUC_HBASE[i].u.z = ak_prms.HSUC_HBASE[i].u.z;
    }

    return AKD_SUCCESS;
}
int16 SaveParameters(
        const AK09911PRMS*  prms ///[in] Pointer to parameters of AK8963
)
{
    int16   i;

        //printf("sizeofak_prms=%d\n",sizeof(ak_prms));

        for(i=0; i<CSPEC_NUM_FORMATION; i++){
         ak_prms.HSUC_HDST[i]=prms->HSUC_HDST[i] ;
         ak_prms.HSUC_HO[i].u.x=prms->HSUC_HO[i].u.x ;
        ak_prms.HSUC_HO[i].u.y=prms->HSUC_HO[i].u.y;
        ak_prms.HSUC_HO[i].u.z=prms->HSUC_HO[i].u.z;
        ak_prms.HFLUCV_HREF[i].u.x=prms->HFLUCV_HREF[i].u.x;
        ak_prms.HFLUCV_HREF[i].u.y=prms->HFLUCV_HREF[i].u.y ;
         ak_prms.HFLUCV_HREF[i].u.z=prms->HFLUCV_HREF[i].u.z ;
         ak_prms.HSUC_HBASE[i].u.x=prms->HSUC_HBASE[i].u.x ;
        ak_prms.HSUC_HBASE[i].u.y=prms->HSUC_HBASE[i].u.y ;
        ak_prms.HSUC_HBASE[i].u.z=prms->HSUC_HBASE[i].u.z ;
    }

    return AKD_SUCCESS;
}



/**
 * This function reads byte array from AKD_LoadParameter(),
 * then convert it to AK09911PRMS structure.
 */
int16 LoadParameters2( AK09911PRMS *prms){
    uint16 i, j;
    //uint8 buf[SIZE_PARAMETER_STORE];
    int16 *pi16;
    int32 *pi32;

//  if( AKD_LoadParameter( buf, SIZE_PARAMETER_STORE) != AKD_SUCCESS){
//      return AKD_ERROR;
//  }

    pi16 = (int16*)buf;
    for( i=0; i < CSPEC_NUM_FORMATION; i++){
        //prms->HSUC_HDST[i] = /*(AKSC_HDST)*/*pi16++;
        for( j=0; j < 3; j++){
            prms->HSUC_HO[i].v[j] = *pi16++;
            prms->HFLUCV_HREF[i].v[j] = *pi16++;
            pi32 = (int32*)pi16;
            prms->HSUC_HBASE[i].v[j] = *pi32++;
            pi16 = (int16*)pi32;
        }
    }

    return AKD_SUCCESS;
}

/**
 * This function convert AK09911PRMS to byte array.
 * The byte array is saved by AKD_SaveParameter().
 */
int16 SaveParameters2( const AK09911PRMS *prms)
{
    uint16 i, j;
    //uint8 buf[SIZE_PARAMETER_STORE];
    int16* pi16;
    int32* pi32;

    pi16 = (int16*)buf;
    for( i=0; i < CSPEC_NUM_FORMATION; i++){
        *pi16++ = (int16)prms->HSUC_HDST[i];
        for( j=0; j < 3; j++){
            *pi16++ = prms->HSUC_HO[i].v[j];
            *pi16++ = prms->HFLUCV_HREF[i].v[j];
            pi32 = (int32*)pi16;
            *pi32++ = prms->HSUC_HBASE[i].v[j];
            pi16 = (int16*)pi32;
        }
    }

//  if( AKD_SaveParameter( buf, SIZE_PARAMETER_STORE) != AKD_SUCCESS){
//      return AKD_ERROR;
//  }

    return AKD_SUCCESS;
}





