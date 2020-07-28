/******************************************************************************
 *
 *  $Id: AKVersion.h 300 2015-01-05 05:31:57Z suzuki.thj $
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
#ifndef AKSC_INC_AKVERSION_H
#define AKSC_INC_AKVERSION_H

#include "AKMDevice.h"

//========================= Type declaration  ===========================//

//========================= Constant definition =========================//

//========================= Prototype of Function =======================//
AKLIB_C_API_START
int16 AKSC_GetVersion_Major(void);
int16 AKSC_GetVersion_Minor(void);
int16 AKSC_GetVersion_Revision(void);
int16 AKSC_GetVersion_DateCode(void);
int16 AKSC_GetVersion_Variation(void);
uint32 AKSC_GetVersion_AlgorithmCode(void);
AKLIB_C_API_END

#endif

