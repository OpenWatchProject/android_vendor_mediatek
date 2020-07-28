/******************************************************************************
 *
 *  $Id: AKManualCal.h 300 2015-01-05 05:31:57Z suzuki.thj $
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
#ifndef AKSC_INC_AKMANUALCAL_H
#define AKSC_INC_AKMANUALCAL_H

#include "AKMDevice.h"

//========================= Type declaration  ===========================//

//========================= Constant definition =========================//
#define AKSC_GDATA_SIZE             64   // Buffer size for gyro offset calculation.
#define AKSC_GDATA_MAX              80   // Q4 format
#define AKSC_GDATA_MAXDIFF          48   // Q4 format

//========================= Prototype of Function =======================//
AKLIB_C_API_START
int16 AKSC_HOffsetCal(      //(o)   : Calibration success(1), failure(0)
    const int16vec hdata[], //(i)   : Geomagnetic vectors(the size must be 3)
    int16vec       *ho      //(o)   : Offset(5Q11)
);

int16 AKSC_GetGyroOffsetAuto(
                                // (o)   : Whether offset calculation is enabled(return:1) or not(return:0).
    const int16vec gdata[],     // (i)   : Buffer for angular rate vector. Index 0 is earlier data.
    const int16    gNave,       // (i)   : The number of gyro data to be averaged. gNave must be 1, 2, 4, 8, 16, 32, 64.
    const int16    gMax,        // (i)   : Threshold for maximum gyro value to detect stable in Q4[dps], must be over 0.
    // (i)   : Thresould for maximum difference value to detect stable in Q4[dps], must be over 0.
    const int16    gMaxDif,
    int16vec       *goffset     // (i/o) : Gyro offset
);


AKLIB_C_API_END

#endif

