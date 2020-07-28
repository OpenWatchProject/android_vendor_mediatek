/******************************************************************************
 *
 *  $Id: CustomerSpec.h 1048 2013-07-18 07:56:35Z yamada.rj $
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
//#define CSPEC_CI_LICENSER    "ASAHIKASEI"
//#if defined(__LP64__)
//#define CSPEC_CI_LICENSEE    "WT_11_AG_16_32"
//#else
//#define CSPEC_CI_LICENSEE    "WT_11_AG_16_32"
//#endif

#define CSPEC_PG_FILTER     (-1)

// Parameters for Direction Calculation
#define CSPEC_DVEC_X        0
#define CSPEC_DVEC_Y        0
#define CSPEC_DVEC_Z        0

#endif //AKMD_INC_CUSTOMERSPEC_H

