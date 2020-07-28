/******************************************************************************
 *
 *  $Id: AKPseudoGyroF.h 399 2017-04-27 04:46:18Z suzuki.thj $
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
#ifndef AKSC_INC_AKPSEUDOGYROF_H
#define AKSC_INC_AKPSEUDOGYROF_H

#include "AKMDevice.h"
#include "AKMDeviceF.h"

//========================= Constant definition ==============================//
#define AKPG_FBUF_SIZE_F  32  //    buffer size definition for PseudoGyro algorithm
//    Please do not change this value.
#define AKPG_MBUF_SIZE_F  2   //    buffer size definition for PseudoGyro algorithm
//    Please do not change this value.
#define AKPG_NBUF_SIZE_F  8   //    buffer size definition for PseudoGyro algorithm
//    Please do not change this value.
#define AKPG_DBUF_SIZE_F  3   //    buffer size definition for PseudoGyro algorithm
//    Please do not change this value.

//========================= Type declaration  ================================//
// variables for AKSC_PseudoGyroF()
typedef struct _AKPG_VAR_F {
    AKSC_FVEC hfbuf[AKPG_FBUF_SIZE_F];
    AKSC_FVEC afbuf[AKPG_FBUF_SIZE_F];
    AKSC_FMAT mbuf[AKPG_MBUF_SIZE_F];
    AKSC_FVEC hnbuf[AKPG_NBUF_SIZE_F];
    AKSC_FVEC anbuf[AKPG_NBUF_SIZE_F];
    AKSC_FVEC hdbuf[AKPG_DBUF_SIZE_F];
    AKSC_FVEC adbuf[AKPG_DBUF_SIZE_F];
    int16     dtbuf[AKPG_FBUF_SIZE_F];
} AKPG_VAR_F;

typedef struct _AKPG_COND_F {
    int16      fmode;             //  0:filter mode 0, 1:filter mode 1
    int16      dtave;             //  average number of dt
    int16      ihave;             //  average number of input magnetic vector
    int16      iaave;             //  average number of input acceleration vector
    int16      th_rmax;           //  |H| upper threshold [uT]
    int16      th_rmin;           //  |H| lower threshold [uT]
    int16      th_rdif;           //  |H| change threshold [uT]
    AKSC_FLOAT ocoef;             //  damping fuctor [0(static), 1(through)]
} AKPG_COND_F;

//========================= Prototype of Function ============================//
AKLIB_C_API_START
void AKSC_InitPseudoGyroF(
    AKPG_COND_F *cond,            //(o)   PG condition parameters
    AKPG_VAR_F  *var              //(o)   variable
);

int16 AKSC_PseudoGyroF(
    const AKPG_COND_F *cond,      ///< [in]       PG condition parameters
    const int16       dt,         ///< [in]       measurement frequency [1/16ms]
    const AKSC_FVEC   *hvec,      ///< [in]       input hdata [16.67code/uT]
    const AKSC_FVEC   *avec,      ///< [in]       input adata [720code/G]
    const I16MATRIX   *hlayout,   ///< [in]       hlayout
    const I16MATRIX   *alayout,   ///< [in]       alayout
    AKPG_VAR_F        *pgvar,     ///< [in, out]  PG variable
    AKSC_FVEC         *pgangrate, ///< [out]      PG angular rate vector [deg/sec]
    AKSC_FQUAT        *pgquat,    ///< [out]      PG orientation quaternion
    AKSC_FVEC         *pgGravity, ///< [out]      PG Gravity [720code/G]
    AKSC_FVEC         *pgLinAcc   ///< [out]      PG linear acc. vector [720code/G]
);

AKLIB_C_API_END

#endif
