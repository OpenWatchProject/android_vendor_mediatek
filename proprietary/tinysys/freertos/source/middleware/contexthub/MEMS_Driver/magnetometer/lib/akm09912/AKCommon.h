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
#ifndef AKMD_INC_AKCOMMON_H
#define AKMD_INC_AKCOMMON_H

#include <errno.h>     //errno
#include <stdarg.h>    //va_list
#include <stdio.h>     //fpintf
#include <stdlib.h>    //atoi
#include <string.h>    //memset
//#include <unistd.h>

#include "AKMLog.h"

/*** Constant definition ******************************************************/

#if 0
#define AK09911_BDATA_SIZE  9
#define AKMD_BDATA_SIZE     9
#define AKMD_WIA                        0x548
#define AKMD_ASA                        0x80
#define AKMD_ST2                        0x00
#define CONVERT_TO_AKM          (0.6f)  /* From uT(Q16) to AKM */
#else
#define AKMD_BDATA_SIZE     9
#define AKMD_WIA            0x48
#define AKMD_ASA            0x80

#define AKMD_ST2_16BIT      (0x10)
#define AKMD_ST2_14BIT      (0x00)
#define AKMD_ST2 AKMD_ST2_14BIT//AKMD_ST2_16BIT

#define CONVERT_TO_AKM      (0.15f) /* From android to AKM */

#endif

#endif //AKMD_INC_AKCOMMON_H

