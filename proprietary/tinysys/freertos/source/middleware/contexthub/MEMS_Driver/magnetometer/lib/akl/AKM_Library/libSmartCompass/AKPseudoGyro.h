/******************************************************************************
 *
 *  $Id: AKPseudoGyro.h 300 2015-01-05 05:31:57Z suzuki.thj $
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
#ifndef AKSC_INC_AKPSEUDOGYRO_H
#define AKSC_INC_AKPSEUDOGYRO_H

#include "AKMDevice.h"

//========================= Constant definition ==============================//
#define AKPG_FBUF_SIZE  32  //    buffer size definition for PseudoGyro algorithm
//    Please do not change this value.
#define AKPG_MBUF_SIZE  2   //    buffer size definition for PseudoGyro algorithm
//    Please do not change this value.
#define AKPG_NBUF_SIZE  8   //    buffer size definition for PseudoGyro algorithm
//    Please do not change this value.
#define AKPG_DBUF_SIZE  3   //    buffer size definition for PseudoGyro algorithm
//    Please do not change this value.

//========================= Type declaration  ================================//
/// variables for AKSC_PseudoGyro()
typedef struct _AKPG_VAR {
    int16vec  hfbuf[AKPG_FBUF_SIZE];
    int16vec  afbuf[AKPG_FBUF_SIZE];
    I16MATRIX mbuf[AKPG_MBUF_SIZE];
    int16vec  hnbuf[AKPG_NBUF_SIZE];
    int16vec  anbuf[AKPG_NBUF_SIZE];
    int16vec  hdbuf[AKPG_DBUF_SIZE];
    int16vec  adbuf[AKPG_DBUF_SIZE];
    int16     dtbuf[AKPG_FBUF_SIZE];
} AKPG_VAR;

typedef struct _AKPG_COND {
    int16  fmode;               //  0:filter mode 0, 1:filter mode 1
    int16  dtave;               //  average number of dt
    int16  ihave;               //  average number of input magnetic vector
    int16  iaave;               //  average number of input acceleration vector
    uint16 th_rmax;             //  |H| upper threshold [16.67code/uT]
    uint16 th_rmin;             //  |H| lower threshold [16.67code/uT]
    uint16 th_rdif;             //  |H| change threshold [16.67code/uT]
    int16  ocoef;               //  damping fuctor [0(static), 1024(through)]
} AKPG_COND;

//========================= Prototype of Function ============================//
AKLIB_C_API_START
void AKSC_InitPseudoGyro(
    AKPG_COND *cond,            //(o)   PG condition parameters
    AKPG_VAR  *var              //(o)   variable
);

int16 AKSC_PseudoGyro(
    const AKPG_COND *cond,      //(i)   PG condition parameters
    const int16     dt,         //(i)   measurement frequency [1/16ms]
    const int16vec  *hvec,      //(i)   input hdata [16.67code/uT]
    const int16vec  *avec,      //(i)   input adata [720code/G]
    const I16MATRIX *hlayout,   //(i)   hlayout
    const I16MATRIX *alayout,   //(i)   alayout
    AKPG_VAR        *pgvar,     //(i/o) PG variable
    int32vec        *pgangrate, //(o)   PG angular rate vector [1/64 deg/sec]
    I16QUAT         *pgquat,    //(o)   PG orientation quaternion
    int16vec        *pgGravity, //(o)   PG Gravity [720code/G]
    int16vec        *pgLinAcc   //(o)   PG linear acc. vector [720code/G]
);

AKLIB_C_API_END

#endif
