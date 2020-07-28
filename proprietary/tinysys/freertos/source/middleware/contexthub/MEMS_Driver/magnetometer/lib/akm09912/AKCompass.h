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
#ifndef AKMD_INC_AKCOMPASS_H
#define AKMD_INC_AKCOMPASS_H

#include "AKCommon.h"
#include "CustomerSpec.h"

//**************************************
// Include files for SmartCompass library.
//**************************************
#include "./include/AKCertification.h"
#include "./include/AKConfigure.h"
#include "./include/AKDecomp.h"
#include "./include/AKDirection6D.h"
#include "./include/AKDirection9D.h"
#include "./include/AKHDOE.h"
#include "./include/AKHDOEaG.h"
#include "./include/AKHFlucCheck.h"
#include "./include/AKMDevice.h"
#include "./include/AKMDeviceF.h"
#include "./include/AKManualCal.h"
//#include "libSmartCompass/AKPseudoGyro.h"
#include "./include/AKVersion.h"

#define AKMD_DISABLE_DOEPLUS
#ifndef AKMD_DISABLE_DOEPLUS
#include "AKDOEPlus.h"
#endif

/*** Constant definition ******************************************************/
#define THETAFILTER_SCALE   4128
#define HFLUCV_TH           2500
#define PDC_SIZE            27


/*! A parameter structure which is needed for HDOE and Direction calculation. */
typedef struct _AKSCPRMS {
    // Variables for magnetic sensor.
    int16vec      m_ho;
    int16vec      HSUC_HO[CSPEC_NUM_FORMATION];
    int32vec      m_ho32;
    int16vec      m_hs;
    int16vec      HFLUCV_HREF[CSPEC_NUM_FORMATION];
    AKSC_HFLUCVAR m_hflucv;

    // Variable for uncalibrated magnetic field.
    int32vec      m_uncalib;
    int32vec      m_bias;

    // Variable for magnetic field.
    int16vec      m_calib;

    // Variables for DecompS3 .
    int16         m_hn;     // Number of acquired data
    int16vec      m_hdata[AKSC_HDATA_SIZE];
    int16vec      m_hvec;   // Averaged value(offset compensated)
    int16vec      m_have;   // Averaged value
    int16vec      m_asa;
    uint8         m_pdc[PDC_SIZE];
    uint8         *m_pdcptr;

#ifndef AKMD_DISABLE_DOEPLUS
    // Variables for DOEPlus.
    AKSC_DOEPVAR  *m_doep_var;
    int16         m_doep_lv;
    AKSC_FLOAT    DOEP_PRMS[CSPEC_NUM_FORMATION][AKSC_DOEP_SIZE];
    int16vec      m_hdata_plus[AKSC_HDATA_SIZE];
#endif

    // Variables for HDOEaG.
    AKSC_DOEAGVAR *m_doeag_var;
    AKSC_HDST     m_hdst;
    AKSC_HDST     HSUC_HDST[CSPEC_NUM_FORMATION];
    int16         m_rsv;
    int16         m_hdt_ag;
    int16         m_gdt_ag;
    int16         m_mode;
    int16           m_oedt;

    // Variables for formation change
    int16         m_form;
    int16         m_cntSuspend;

    // Variables for Direction9D.
    //D9D_Handle    m_hD9D;
    int16         m_d9dRet;
    int16         m_hnave;
    int16vec      m_dvec;
    int16         m_theta;
    int16         m_delta;
    int16         m_hr;
    int16         m_hrhoriz;
    int16         m_ar;
    int16         m_phi180;
    int16         m_phi90;
    int16         m_eta180;
    int16         m_eta90;
    I16MATRIX     m_mat;
    I16QUAT       m_quat;
    int16vec      m_gravity;
    int16vec      m_linacc;

    // Variables for acceleration sensor.
    int16vec      m_AO;
    int16vec      m_avec;

    // Variables for gyroscope sensor.
    int16vec      m_GO;
    int16vec      m_gvec;
    int16vec      m_gdata[AKSC_GDATA_SIZE];

    I16MATRIX     m_hlayout;
    I16MATRIX     m_alayout;
    I16MATRIX     m_glayout;

    // Variables for measurement interval
    int16         m_hdt;
    int16         m_adt;
    int16         m_gdt;

    // Ceritication
    uint8         m_licenser[AKSC_CI_MAX_CHARSIZE + 1]; //end with '\0'
    uint8         m_licensee[AKSC_CI_MAX_CHARSIZE + 1]; //end with '\0'
    int16         m_key[AKSC_CI_MAX_KEYSIZE];

    // base
    int32vec      m_hbase;
    int32vec      HSUC_HBASE[CSPEC_NUM_FORMATION];
} AKSCPRMS;

typedef struct _AKSCPRMS AK09911PRMS;

// A parameter structure.
typedef struct _AK9911SAVEPRMS{
int16vec        HSUC_HO[CSPEC_NUM_FORMATION];

int16vec        HFLUCV_HREF[CSPEC_NUM_FORMATION];

AKSC_HDST       HSUC_HDST[CSPEC_NUM_FORMATION];

int32vec        HSUC_HBASE[CSPEC_NUM_FORMATION];
} AK9911SAVEPRMS;
#endif //AKMD_INC_AKCOMPASS_H

