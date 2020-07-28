/******************************************************************************
 *
 *  $Id: AKDOEPlus.h 300 2015-01-05 05:31:57Z suzuki.thj $
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
#ifndef AKSC_AKDOEPLUS_H
#define AKSC_AKDOEPLUS_H

#include "AKMDevice.h"
#include "AKMDeviceF.h"

//========================= Constant definition ==============================//
#define AKSC_DOEP_SIZE      16             // array size of save data

//========================= type definition ==================================//
typedef struct _AKSC_DOEPVAR AKSC_DOEPVAR; // DOEPlus parameters struct

//========================= Prototype of Function ============================//
AKLIB_C_API_START
int16 AKSC_GetSizeDOEPVar(void);          //(o)   : size of DOEPlus parameters

void AKSC_InitDOEPlus(
    AKSC_DOEPVAR *var                      //(o)   : pointer of DOEPlus parameters struct
);

void AKSC_SaveDOEPlus(
    const AKSC_DOEPVAR *var,               //(i)   : pointer of DOEPlus parameters struct
    AKSC_FLOAT         data[]              //(o)   : saved data
);

int16 AKSC_LoadDOEPlus(                    //(o)    : lv of save data
    const AKSC_FLOAT data[],               //(i)   : saved data
    AKSC_DOEPVAR     *var                  //(i/o) : pointer of DOEPlus parameters struct
);

int16 AKSC_DOEPlus(                        //(o)    : compensation parameter is updated(1) or not(0)
    const int16vec *i16v,                  //(i)   : new magnetic vector
    AKSC_DOEPVAR   *var,                   //(i/o) : pointer of DOEPlus parameters struct
    int16          *lv                     //(o)   : DOEPlus lv
);

void AKSC_DOEPlus_DistCompen(
    const int16vec     *ivec,              //(i)   : distorted vector
    const AKSC_DOEPVAR *var,               //(i)   : pointer of DOEPlus parameters struct
    int16vec           *ovec               //(o)   : compensated vector
);

AKLIB_C_API_END

#endif
