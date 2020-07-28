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
#ifndef AKM_INC_AKMPG_MEASURE_H
#define AKM_INC_AKMPG_MEASURE_H
#include "AKMPseudoGyro.h"
#include "AKPGCommon.h"

/*** Constant definition ******************************************************/
#define AKRET_PROC_SUCCEED      0x00    /*!< The process has been successfully done. */

void InitAKSC_PGPRMS(AKSCPGPRMS *prms);
int16 Init_PG_Measure(AKSCPGPRMS *prms);
void PG_InitConfig(AKSCPGPRMS* prms);
int16 CalcAngularRate(AKSCPGPRMS* prms);

#endif
