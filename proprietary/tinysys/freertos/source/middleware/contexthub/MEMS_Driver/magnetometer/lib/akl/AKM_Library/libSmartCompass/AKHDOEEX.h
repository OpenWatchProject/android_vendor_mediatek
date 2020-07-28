/******************************************************************************
 *
 *  $Id: AKHDOEEX.h$
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
#ifndef AKSC_INC_AKHDOEEX_H
#define AKSC_INC_AKHDOEEX_H

#include "AKHDOE.h"
#include "AKMDevice.h"

//========================= Type declaration  ===========================//
typedef struct AKSC_DOEEXVAR_ AKSC_DOEEXVAR; // DOEEX parameters struct

//========================= Prototype of Function =======================//
AKLIB_C_API_START

int16 AKSC_GetSizeDOEEXVar(void); //(o)   : size of DOEEX parameters

// Initialize
void AKSC_InitHDOEEX(
    AKSC_DOEEXVAR   *var,        //(o)   : a struct of internal variables of DOEEX
    const int16     idxhcond,    //(i)   : initial index of hcond[]
    const int16vec  *ho,         //(i)   : initial offset [AKSC_FORMAT]
    const AKSC_HDST hdst         //(i)   : initial DOE level
);

int16 AKSC_HDOEEX(
    const uint8    licenser[],   //(i)   : licenser
    const uint8    licensee[],   //(i)   : licensee
    const int16    key[],        //(i)   : key
    const int16vec hdata[],      //(i)   : magnetic data [AKSC_FORMAT]
    AKSC_DOEEXVAR  *var,         //(i/o) : a struct of internal variables of DOEEX
    int16vec       *ho,          //(o)   : current offset [AKSC_FORMAT]
    AKSC_HDST      *hdst         //(o)   : HDOE status
);

// set parameter level
void AKSC_SetHDOEEXLevel(
    AKSC_DOEEXVAR   *var,        //(i/o) : a struct of internal variables of DOEEX
    const int16vec  *ho,         //(i)   : initial offset [AKSC_FORMAT]
    const AKSC_HDST hdst,        //(i)   : DOE level
    const int16     initBuffer   //(i)   : 0 : internal buffer will not be cleared / 1 : will be cleared
);
AKLIB_C_API_END

#endif
