/******************************************************************************
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
 ******************************************************************************/
#ifndef INCLUDE_AKL_SMART_COMPASS_H
#define INCLUDE_AKL_SMART_COMPASS_H

/* Root file of AKL module. */
#include "AKL_APIs.h"

/* Headers of SmartCompass Library */
#include "libSmartCompass/AKCertification.h"
#include "libSmartCompass/AKConfigure.h"
#include "libSmartCompass/AKDecomp.h"
#include "libSmartCompass/AKDirection6D.h"
#include "libSmartCompass/AKDirection9D.h"
#include "libSmartCompass/AKHDOE.h"
#include "libSmartCompass/AKHFlucCheck.h"
#include "libSmartCompass/AKMDevice.h"
#include "libSmartCompass/AKManualCal.h"
#include "libSmartCompass/AKVersion.h"

#ifdef AKM_ENABLE_DOEPLUS
#include "libSmartCompass/AKDOEPlus.h"
#include "libSmartCompass/AKMDeviceF.h"
#endif

#ifdef AKM_ENABLE_DOEEX
#include "libSmartCompass/AKHDOEEX.h"
#endif

/*! Parameter for #AKSC_HFlucCheck function. */
#define HFLUCV_TH          2500
#define THETAFILTER_SCALE  4128

/*
 * The following prefix joined this order.
 * e.g. A pointer to a struct is: 'ps_foo'
 *      An array of struct is   : 'sa_bar'
 * p_ : pointer
 * v_ : vector (a special type of struct)
 * s_ : struct
 * a_ : array
 * m_ : integer/float/enum etc..
 */

/*!
 * A parameter struct which is saved to non-volatile area for warm start up.
 */
struct AKL_NV_PRMS {
    /*! This value is used to identify the data area is AKL_NV_PRMS.
     * This value should be #AKL_NV_MAGIC_NUMBER. */
    uint32     magic;

    int16vec   va_hsuc_ho;                  /*!< fine offset of magnetic vector */
    int16vec   va_hflucv_href;              /*!< rough value of magnetic vector */
    AKSC_HDST  a_hsuc_hdst;                 /*!< status of magnetic offset */
    int32vec   va_hsuc_hbase;               /*!< rough offset of magnetic vector */
#ifdef AKM_ENABLE_DOEPLUS
    AKSC_FLOAT a_doep_prms[AKSC_DOEP_SIZE]; /*!< a parameter for DOEPlus */
#endif
#ifdef AKM_ENABLE_PDC
    uint8      a_pdc[AKL_PDC_SIZE];         /*!< a parameter for PDC */
#endif
};

/*! A parameter structure which is needed for SmartCompass Library. */
struct AKL_SCL_PRMS {
    struct AKL_NV_PRMS            *ps_nv;
    uint32_t                      init;
    struct AKL_CERTIFICATION_INFO s_cert;

    /* Variables for magnetic sensor. */
    int32vec                      v_ho32;
    int16vec                      v_hs;
    AKSC_HFLUCVAR                 s_hflucv;
    /* base */
    int32vec                      v_hbase;

    /* Variables for DecompS3. */
    int16                         m_hn;
    int16                         m_hnave;
    int16vec                      v_asa;
    uint8                         *pa_pdcptr;

#ifdef AKM_ENABLE_DOEEX
    /* Variables for HDOEEX. */
    AKSC_DOEEXVAR                 *ps_doeex_var;
    AKSC_HDST                     m_hdst;
#else
    /* Variables for HDOE. */
    AKSC_HDOEVAR                  s_hdoev;
    AKSC_HDST                     m_hdst;
#endif

    /* Variables for formation change */
    uint8                         m_form;
    uint8                         m_maxForm;
    uint8                         m_curForm;


    /* Variables for Direction9D. */
    D9D_Handle                    p_hd9d;
    int16                         m_theta;
    int16                         m_phi180;
    int16                         m_eta90;

    /* vector collection */
    /*! Magnetic vector (offset subtracted) */
    int16vec                      v_hvec;
    /*! Acceleration vector (offset subtracted) */
    int16vec                      v_avec;
    /*! Angular rate vector (offset subtracted) */
    int16vec                      v_gvec;
    /*! An offset of magnetic vector */
    int16vec                      v_ho;
    /*! An offset of angular rate vector */
    int16vec                      v_go;
    /*! A buffer of raw magnetic vector (with offset) */
    int16vec                      va_hdata[AKSC_HDATA_SIZE];
    /*! A buffer of raw angular rate vector (with offset) */
    int16vec                      va_gdata[AKSC_GDATA_SIZE];
    /*! Quaternion */
    I16QUAT                       s_quat;
    /*! Gravity vector */
    AKSC_FVEC                      v_gravity;
    /*! Linear acceleration vector */
    AKSC_FVEC                      v_lacc;

    /* time stamp collection */
    /*! Time stamp for 'formation chage' event. */
    int64_t                       m_ts_ev_form;
    /*! Time stamp for 'HDOE' event. */
    int64_t                       m_ts_ev_doe;
    /*! Time stamp for 'Direction9D' event. */
    int64_t                       m_ts_ev_d9d;
    /*! Time stamp for v_hvec */
    int64_t                       m_ts_hvec;
    /*! Time stamp for v_avec */
    int64_t                       m_ts_avec;
    /*! Time stamp for v_gvec */
    int64_t                       m_ts_gvec;
    /*! Time stamp for D9D hvec */
    int64_t                       m_ts_ev_9d_hvec;
    /*! Time stamp for D9D avec */
    int64_t                       m_ts_ev_9d_avec;
    /*! Time stamp for D9D gvec */
    int64_t                       m_ts_ev_9d_gvec;

#ifdef AKM_ENABLE_DOEPLUS
    /* Variables for DOEPlus. */
    AKSC_DOEPVAR                  *ps_doep_var;
    int16                         m_doep_lv;
    int16vec                      va_hdata_plus[AKSC_HDATA_SIZE];
#endif

    /*! Layout pattern  number */
    int16_t                       m_pat;

    /*! AKM device type */
    AKM_DEVICE_TYPE               m_device;

    /*! parameters for debug */
    int16_t                       m_dbg_meas_fret;
    int16_t                       m_dbg_scl_fret;
    /*! Magnetic vector (offset subtracted) */
    int16vec                      v_hvec_fusion;
    /*! Acceleration vector (offset subtracted) */
    int16vec                      v_avec_fusion;
    /*! Angular rate vector (offset subtracted) */
    int16vec                      v_gvec_fusion;
    /*! Time stamp for v_hvec */
    int64_t                       m_ts_hvec_fusion;
    /*! Time stamp for v_avec */
    int64_t                       m_ts_avec_fusion;
    /*! Time stamp for v_gvec */
    int64_t                       m_ts_gvec_fusion;
    /*! Time stamp for v_hvec_pre */
    int64_t                       m_ts_hvec_fusion_pre;
    /*! Time stamp for v_avec */
    int64_t                       m_ts_avec_fusion_pre;
    /*! Time stamp for v_gvec */
    int64_t                       m_ts_gvec_fusion_pre;
};
#endif /* INCLUDE_AKL_SMART_COMPASS_H */
