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
#ifndef AKM_INC_AKM_PSEUDO_GYRO_H
#define AKM_INC_AKM_PSEUDO_GYRO_H

#include "AKPGCommon.h"
#include "PGCustomerSpec.h"

//**************************************
// Include files for SmartCompass library.
//**************************************
//#include "AKCertification.h"
#include "AKDirection6D.h"
#include "AKPseudoGyroF.h"

#define AKSC_PI     ((AKSC_FLOAT)3.14159265358979323846264338)

#ifdef AKSC_MATH_DOUBLE
typedef double AKSC_FLOAT;
#else
typedef float AKSC_FLOAT;
#endif

//========================= Constant definition =========================//
#define AKSC_HSENSE_TARGET  833  // Target sensitivity for magnetic sensor

/*** Constant definition ******************************************************/
#define AKMD_ERROR      -1
#define AKMD_SUCCESS     0

/*! A parameter structure which is needed for HDOE and Direction calculation. */
typedef struct _AKSCPGPRMS {
    // Variables for magnetic sensor.
    int16vec    m_ho;
    int16vec    m_hs;

    // Variables for DecompS3 .
    int16vec    m_hdata;
    int16       m_hdst;

    // Variables for acceleration sensor.
    int16vec    m_avec;

    // PseudoGyro
    int16           m_pgRet;
    int16           m_pgdt;
    int16           m_pgFilter;
#if 0
    AKPG_COND       m_pgcond;
    AKPG_VAR        m_pgvar;
    int32vec        m_pgout;
    I16QUAT         m_pgquat;
    int16vec        m_pgGravity;
    int16vec        m_pgLinAcc;
    int16           m_pgFilter;
#else
    AKPG_COND_F     m_pgcond;
    AKPG_VAR_F      m_pgvar;
    AKSC_FVEC       m_pgout;
    AKSC_FQUAT      m_pgquat;
    AKSC_FVEC       m_pgGravity;
    AKSC_FVEC       m_pgLinAcc;
#endif

    // Certification
    //uint8        m_licenser[AKSC_CI_MAX_CHARSIZE+1];    //end with '\0'
    //uint8        m_licensee[AKSC_CI_MAX_CHARSIZE+1];    //end with '\0'
    //int16        m_key[AKSC_CI_MAX_KEYSIZE];
} AKSCPGPRMS;

#endif //AKMD_INC_AKCOMPASS_H

