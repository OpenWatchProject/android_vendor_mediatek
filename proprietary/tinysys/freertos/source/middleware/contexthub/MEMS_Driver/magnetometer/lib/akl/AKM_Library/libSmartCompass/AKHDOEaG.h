/******************************************************************************
 *
 *  $Id: AKHDOEaG.h$
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
#ifndef AKSC_INC_AKHDOEAG_H
#define AKSC_INC_AKHDOEAG_H

#include "AKHDOE.h"
#include "AKMDevice.h"

//========================= Type declaration  ===========================//
typedef struct _AKSC_DOEAGVAR AKSC_DOEAGVAR; // DOEaG parameters struct

//========================= Prototype of Function =======================//
AKLIB_C_API_START

int16 AKSC_GetSizeDOEaGVar(void);           //(o)   : size of DOEaG parameters

// Get AKSC_HDOEVAR struct in AKSC_DOEAGVAR struct
void AKSC_GetDOEVarInDOEaGVar(
    const AKSC_DOEAGVAR *varDOEaG,
    AKSC_HDOEVAR        *varDOE
);

// Initialize
void AKSC_InitHDOEaG(
    AKSC_DOEAGVAR   *var,                    //(o)   : a struct of internal variables
    const int16     idxhcond,                //(i)   : initial index of hcond[]
    const I16MATRIX *hlayout,                //(i)   : layout matrix for magnetic data
    const int16vec  *ho,                     //(i)   : initial offset [AKSC_FORMAT]
    const AKSC_HDST hdst                     //(i)   : initial DOE level
);

// set parameter level
void AKSC_SetHDOEaGLevel(
    AKSC_DOEAGVAR   *var,                    //(o)   : a struct of internal variables
    const I16MATRIX *hlayout,                //(i)   : layout matrix for magnetic data
    const int16vec  *ho,                     //(i)   : initial offset [AKSC_FORMAT]
    const AKSC_HDST hdst,                    //(i)   : DOE level
    const int16     initBuffer               //(i)   : 0 : internal buffer will not be cleared / 1 : will be cleared
);

// DOEaG (DOE advanced by Gyro) Process
int16 AKSC_HDOEaG(
    const uint8     licenser[],              //(i)   : licenser
    const uint8     licensee[],              //(i)   : licensee
    const int16     key[],                   //(i)   : key
    const int16     mode,                    //(i)   : (0) DOEaG. (1) DOE
    const int16     oedt,                    //(i)   : cycle of offset estimation [Q4 ms]
    const int16     hdt_aG,                  //(i)   : hdata elapsed time since the last hdata [Q4 ms] (negative value represents hdata is not updated)
    const int16     gdt_aG,                  //(i)   : gdata elapsed time since the last "hdata" [Q4 ms] (negative value represents gdata is not updated)
    const int16vec  *hdata,                  //(i)   : magnetic data [AKSC_FORMAT]
    const int16vec  *gdata,                  //(i)   : gyro data [Q4 dps] (offset and sensitivity are compensated)
    const I16MATRIX *hlayout,                //(i)   : layout matrix for magnetic data
    const I16MATRIX *glayout,                //(i)   : layout matrix for gyro data
    AKSC_DOEAGVAR   *var,                    //(i/o) : a struct of internal variables
    int16vec        *ho,                     //(i/o) : current offset [AKSC_FORMAT]
    AKSC_HDST       *hdst                    //(o)   : HDOEaG status
);

AKLIB_C_API_END

#endif
