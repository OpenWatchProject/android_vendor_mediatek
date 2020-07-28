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
#ifndef AKMD_INC_CUSTOMERSPEC_H
#define AKMD_INC_CUSTOMERSPEC_H

/*******************************************************************************
 User defines parameters.
 ******************************************************************************/

// Certification information
#define CSPEC_CI_AK_DEVICE  8963
#define CSPEC_CI_LICENSER   "ASAHIKASEI"
// #define CSPEC_CI_LICENSEE   "MTK_12_AG_GCC"
#define CSPEC_CI_LICENSEE   "MTK_X_SHUB"


// Parameters for Average
//  The number of magnetic data to be averaged.
//  CSPEC_HNAVE must be 1, 2, 4, 8, 16 or 32.
#define CSPEC_HNAVE     8
//#define CSPEC_HNAVE     1


// Parameters for Average
//  Variable that indicates how many gyroscope data are averaged.
//  gNave must be 1, 2, 4, 8, 16, 32, 64.
#define CSPEC_GNAVE             64

// Parameters for Direction Calculation
#define CSPEC_DVEC_X        0
#define CSPEC_DVEC_Y        0
#define CSPEC_DVEC_Z        0

// The number of formation
#define CSPEC_NUM_FORMATION     1

// the counter of Suspend
#define CSPEC_CNTSUSPEND_SNG    8

// Parameters for FST
//  1 : USE SPI
//  0 : NOT USE SPI(I2C)
#define CSPEC_SPI_USE           0

//Parameters for Temperature sensor (specific function of AK09912)
//  0x00 : disable
//  0x80 : enable
#define CSPEC_TEMPERATURE       0x80

//Parameters for Noise Suppression Filter (specific function of AK09912)
//  0x00 : disable
//  0x20 : Low
//  0x40 : Middle
//  0x60 : High
#define CSPEC_NSF               0x40

// Parameters for DOEaG mode (DOEaG=0, DOE=1)
// *** AKM recommendation ***
// This source code does NOT supprt DOE function
#define CSPEC_DOEAG_MODE        0
#define CSPEC_DOEONLY_MODE        1
// Since recommendation is 1600 ideally, but we need to consider timer gitter 10%.
#define CSPEC_DOEAG_OEDT        1440


#endif //AKMD_INC_CUSTOMERSPEC_H

