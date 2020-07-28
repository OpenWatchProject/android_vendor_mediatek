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
#include "libSmartCompass/AKHDOEaG.h"
#include "libSmartCompass/AKHFlucCheck.h"
#include "libSmartCompass/AKMDevice.h"
#include "libSmartCompass/AKMDeviceF.h"
#include "libSmartCompass/AKManualCal.h"
#include "libSmartCompass/AKVersion.h"

#ifdef AKMD_ENABLE_DOEPLUS
#include "libSmartCompass/AKDOEPlus.h"
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

    int16vec   va_hsuc_ho;                      /*!< fine offset of magnetic vector */
    int16vec   va_hflucv_href;                  /*!< rough value of magnetic vector */
    AKSC_HDST  a_hsuc_hdst;                     /*!< status of magnetic offset */
    int32vec   va_hsuc_hbase;                   /*!< rough offset of magnetic vector */
#ifdef AKMD_ENABLE_DOEPLUS
    AKSC_FLOAT a_doep_prms[AKSC_DOEP_SIZE];     /*!< a parameter for DOEPlus */
#endif
#ifdef AKM_ENABLE_PDC
    uint8      a_pdc[AKL_PDC_SIZE];             /*!< a parameter for PDC */
#endif
#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
    int16vec   va_hsuc_ho_doe;                  /*!< fine offset of magnetic vector */
    int16vec   va_hflucv_href_doe;              /*!< rough value of magnetic vector */
    AKSC_HDST  a_hsuc_hdst_doe;                 /*!< status of magnetic offset */
    int32vec   va_hsuc_hbase_doe;               /*!< rough offset of magnetic vector */
#ifdef AKMD_ENABLE_DOEPLUS
    AKSC_FLOAT a_doep_prms_doe[AKSC_DOEP_SIZE]; /*!< a parameter for DOEPlus */
#endif
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
    int16vec                      v_have;
    int16vec                      v_asa;
    uint8                         *pa_pdcptr;

    /* Variables for HDOEaG. */
    AKSC_DOEAGVAR                 *ps_doeag_var;
    AKSC_HDST                     m_hdst;
    int16                         m_mode;
    int16                         m_oedt;
    AKSC_HDST           m_pre_hdst;     //add by eric for save parameters when hdst change to 3

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
    int16vec                      v_gravity;
    /*! Linear acceleration vector */
    int16vec                      v_lacc;

    /* time stamp collection */
    /*! Time stamp for 'formation chage' event. */
    int64_t                       m_ts_ev_form;
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

#ifdef AKSC_DOEAG_DEBUG
    /* variables for AKSC_DOEaG_InputChecker */
    int16                         m_doeag_ic_ret;
    int16                         m_flg;
    int16                         m_hdt_aG_prev;
    int16                         m_gdt_aG_prev;
    int16                         m_gdt;
#endif

#ifdef AKMD_ENABLE_DOEPLUS
    /*! Time stamp for 'HDOEPlus' event. */
    int64_t        m_ts_ev_doep;
    /* Variables for DOEPlus. */
    AKSC_DOEPVAR   *ps_doep_var;
    int16          m_doep_lv;
    int16vec       va_hdata_plus[AKSC_HDATA_SIZE];
#endif

    /*! Layout pattern  number */
    int16_t        m_pat;

    /*! AKM device type */
    AKM_DEVICE_TYPE m_device;

    /*! parameters for debug */
    int16_t                       m_dbg_meas_fret;
    int16_t                       m_dbg_scl_fret;

#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
    /* Variables for magnetic sensor. */
    int32vec      v_ho32_doe;
    AKSC_HFLUCVAR s_hflucv_doe;
    /* base */
    int32vec      v_hbase_doe;

    /* Variables for HDOE. */
    AKSC_HDOEVAR  s_hdoev_doe;
    AKSC_HDST     m_hdst_doe;
    AKSC_HDST           m_pre_hdst_doe;     //add by eric for save parameters when hdst change to 3

    /* Variables for formation change */
    uint8         m_form_doe;

    /* vector collection */
    /*! Magnetic vector (offset subtracted) */
    int16vec      v_hvec_doe;
    /*! An offset of magnetic vector */
    int16vec      v_ho_doe;
    /*! A buffer of raw magnetic vector (with offset) */
    int16vec      va_hdata_doe[AKSC_HDATA_SIZE];

    /* time stamp collection */
    /*! Time stamp for 'formation chage' event. */
    int64_t      m_ts_ev_form_doe;
    /*! Time stamp for 'HDOE' event. */
    int64_t      m_ts_ev_doe;
    /*! Time stamp for v_hvec */
    int64_t      m_ts_hvec_doe;

#ifdef AKMD_ENABLE_DOEPLUS
    /* Variables for DOEPlus. */
    AKSC_DOEPVAR *ps_doep_var_doe;
    int16        m_doep_lv_doe;
    int16vec     va_hdata_plus_doe[AKSC_HDATA_SIZE];
#endif

#endif
#if 1//def CFG_AKM_FUSION_SUPPORT
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
#endif
};
#endif /* INCLUDE_AKL_SMART_COMPASS_H */
