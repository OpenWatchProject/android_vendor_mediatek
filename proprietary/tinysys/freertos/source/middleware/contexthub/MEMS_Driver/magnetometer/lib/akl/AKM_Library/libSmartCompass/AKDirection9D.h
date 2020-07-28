/******************************************************************************
 *
 *  $Id: AKDirection9D.h 399 2017-04-27 04:46:18Z suzuki.thj $
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
#ifndef AKSC_INC_AKDIRECTION9D_H
#define AKSC_INC_AKDIRECTION9D_H

#include "AKMDevice.h"
#include "AKMDeviceF.h"

//========================= Type declaration  ===========================//
typedef int32 *D9D_Handle;

typedef enum tagAKSC_D9D_PARAMS_ID {
    AKSC_D9D_ID_TIME_CONST_MAG_DIST = 0,  // AKSC_D9D_Params.timeConstMagDisturb
    AKSC_D9D_ID_TIME_CONST_ACC_DIST,     // AKSC_D9D_Params.timeConstAccDisturb
    AKSC_D9D_ID_TIME_CONST_INC_LARGE,    // AKSC_D9D_Params.timeConstIncLarge
    AKSC_D9D_ID_TIME_CONST_TRANS,        // AKSC_D9D_Params.timeConstTransition
    AKSC_D9D_ID_MAG_OFFSET_SMOOTH,       // AKSC_D9D_Params.magOffsetSmooth
    AKSC_D9D_ID_MAG_INST_SMOOTH,         // AKSC_D9D_Params.magInstSmooth
    AKSC_D9D_ID_MAG_FLUCTUATION,         // AKSC_D9D_Params.magFluctuation
    AKSC_D9D_ID_MAG_MAX,                 // AKSC_D9D_Params.magMax
    AKSC_D9D_ID_MAG_MIN,                 // AKSC_D9D_Params.magMin
    AKSC_D9D_ID_MAG_HORIZ_MIN,           // AKSC_D9D_Params.magHorizontalMin
    AKSC_D9D_ID_ACC_FLUCTUATION,         // AKSC_D9D_Params.accFluctuation
    AKSC_D9D_ID_ACC_FLUCTUATION_DIFF,    // AKSC_D9D_Params.accFluctuationDiff
    AKSC_D9D_ID_AXIS_DIFF,               // AKSC_D9D_Params.axisDiff
    AKSC_D9D_ID_FIXED_MODE               // AKSC_D9D_Params.fixedMode
} AKSC_D9D_PARAMS_ID;

typedef enum _AKSC_STATE_NUM {           // return value definition for AKSC_D9D_watchState
    AKSC_STATE_NUM0 = 0,                 // STATE_NUM0
    AKSC_STATE_NUM1 = 1,                 // STATE_NUM1
    AKSC_STATE_NUM2 = 2,                 // STATE_NUM2
    AKSC_STATE_NUM3 = 3,                 // STATE_NUM3
    AKSC_STATE_NUM4 = 4,                 // STATE_NUM4
    AKSC_STATE_NUM5 = 5,                 // STATE_NUM5
    AKSC_STATE_NUM6 = 6,                 // STATE_NUM6
    AKSC_STATE_NUM7 = 7,                 // STATE_NUM7
    AKSC_STATE_NUM8 = 8,                 // STATE_NUM8
    AKSC_STATE_NUM9 = 9                  // STATE_NUM9
} AKSC_STATE_NUM;

//========================= Constant definition =========================//
// setting value definition for AKSC_D9D_setParams when paramsID is AKSC_D9D_ID_FIXED_MODE
#define AKSC_FIXED_6D_BASED_MODE    2
// setting value definition for AKSC_D9D_setParams when paramsID is AKSC_D9D_ID_FIXED_MODE
#define AKSC_FIXED_GYRO_BASED_MODE  1
// setting value definition for AKSC_D9D_setParams when paramsID is AKSC_D9D_ID_FIXED_MODE
#define AKSC_FIXED_MODE_OFF         0

//========================= Prototype of Function =======================//
AKLIB_C_API_START

int16 AKSC_GetSizeOfD9DObjectIn32BitWord(
    void
);

int16 AKSC_InitDirection9D(
                                // (o)   :
    D9D_Handle hD9D             // (i)   : AKSC_Direction9D handle
);

int16 AKSC_Direction9D(
                                // (o)   :
    D9D_Handle      hD9D,       // (i)   : AKSC_Direction9D handle
    const uint8     licenser[],  // (i)   : Licenser
    const uint8     licensee[],  // (i)   : Licensee
    const int16     key[],      // (i)   : Key
    const int16vec  *h,         // (i)   : Geomagnetic vector (offset and sensitivity are compensated)
    const int16vec  *a,         // (i)   : Acceleration vector (offset and sensitivity are compensated)
    const int16vec  *g,         // (i)   : Angular rate vector (offset and sensitivity are compensated)
    const int16     hdt,        // (i)   : Delta time of geomagnetic data in msec (Q4)
    const int16     adt,        // (i)   : Delta time of acceleration data in msec (Q4)
    const int16     gdt,        // (i)   : Delta time of gyroscope data in msec (Q4)
    // (i)  : A vector to define reference axis of the azimuth on the terminal coordinate system
    const int16vec  *dvec,
    const I16MATRIX *hlayout,   // (i)   : Layout matrix for geomagnetic vector
    const I16MATRIX *alayout,   // (i)   : Layout matrix for acceleration vector
    const I16MATRIX *glayout,   // (i)   : Layout matrix for angular rate vector
    int16           *theta,     // (o)   : Azimuth direction (degree)
    int16           *delta,     // (o)   : The inclination (degree)
    int16           *hr,        // (o)   : Geomagnetic vector size
    int16           *hrhoriz,   // (o)   : Horizontal element of geomagnetic vector
    int16           *ar,        // (o)   : Acceleration vector size
    int16           *phi180,    // (o)   : Pitch angle (-180 to +180 degree)
    int16           *phi90,     // (o)   : Pitch angle (-90 to +90 degree)
    int16           *eta180,    // (o)   : Roll angle  (-180 to +180 degree)
    int16           *eta90,     // (o)   : Roll angle  (-90 to +90 degree)
    I16MATRIX       *mat,       // (o)   : Rotation matrix
    I16QUAT         *quat,      // (o)   : Rotation Quaternion
    AKSC_FVEC       *gravity,   // (o)   : Gravity
    AKSC_FVEC       *linacc     // (o)   : Linear acceleration
);

// Set parameters of D9D
/////////////////////////////////////////////////////////////////////////////////////////////////////
// =============== paramsID =========== : === val ===================================================
// AKSC_D9D_ID_TIME_CONST_MAG_DIST      :   Setting value [sec]
// AKSC_D9D_ID_TIME_CONST_ACC_DIST      :   Setting value [sec]
// AKSC_D9D_ID_TIME_CONST_INC_LARGE     :   Setting value [sec]
// AKSC_D9D_ID_TIME_CONST_TRANS         :   Setting value [sec]
// AKSC_D9D_ID_MAG_OFFSET_SMOOTH        :   Setting value [-]
// AKSC_D9D_ID_MAG_INST_SMOOTH          :   Setting value [-]
// AKSC_D9D_ID_MAG_FLUCTUATION          :   Setting value [uT]
// AKSC_D9D_ID_MAG_MAX                  :   Setting value [uT]
// AKSC_D9D_ID_MAG_MIN                  :   Setting value [uT]
// AKSC_D9D_ID_MAG_HORIZ_MIN            :   Setting value [uT]
// AKSC_D9D_ID_ACC_FLUCTUATION          :   Setting value [G]
// AKSC_D9D_ID_ACC_FLUCTUATION_DIFF     :   Setting value [G]
// AKSC_D9D_ID_AXIS_DIFF                :   Setting value [deg]
/* AKSC_D9D_ID_FIXED_MODE :  AKSC_FIXED_MODE_OFF [-] or AKSC_FIXED_6D_BASED_MODE [-] 
* or AKSC_FIXED_GYRO_BASED_MODE [-]
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////
int16 AKSC_D9D_setParams(
                                        // (o)    : success: 1, failure: 0
    D9D_Handle               hD9D,      // (i)    : AKSC_Direction9D handle
    const AKSC_D9D_PARAMS_ID paramsID,  // (i)    : Parameter ID
    const AKSC_FLOAT         val        // (i)    : Setting value
);

int16 AKSC_D9D_watchState(
                                       // (o)   : return AKSC_STATE_NUM
    D9D_Handle hD9D                    // (i)   : AKSC_Direction9D handle
);

int16 AKSC_D9D_checkDiffOri(
                                       // (o)   : success: 1, failure: 0
    D9D_Handle     hD9D,               // (i)   : AKSC_Direction9D handle
    // (i) : A vector to define reference axis of the azimuth on the terminal coordinate system
    const int16vec *dvec,
    int16          *diffTheta,         // (o)   : differential between 6D-based and Current azimuth direction (degree)
    int16          *diffPhi180,        // (o)   : differential between 6D-based and Current pitch angle (degree)
    int16          *diffEta90          // (o)   : differential between 6D-based and Current roll angle (degree)
);

uint16 AKSC_D9D_checkAllStateCond(     // (o)   : return observed condition
    D9D_Handle hD9D                    // (i)   : AKSC_Direction9D handle
);
AKLIB_C_API_END
#endif
