/******************************************************************************
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
#ifndef AKM_INC_AKMPG_H
#define AKM_INC_AKMPG_H

#include "AKMPseudoGyro.h"
#include "AKPGCommon.h"

int16 AKMPG_Init(void);
void AKMPG_Release(void);
void AKMPG_Start(void);
void AKMPG_Stop(void);

void AKMPG_SetMagData(
    const int16 mx,
    const int16 my,
    const int16 mz,
    const int16 mdt,
    const int16 mag_lv
);

void AKMPG_SetAccData(
        const int16 ix,
        const int16 iy,
        const int16 iz
);

void AKMPG_GetGyro(AKSC_FLOAT gyro_buf[]);
void AKMPG_GetRotationVector(AKSC_FLOAT rv_buf[]);
void AKMPG_GetGravity(AKSC_FLOAT gravity_buf[]);
void AKMPG_GetLA(AKSC_FLOAT la_buf[]);

#endif
