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
#include "measure.h"

/*!
 * The minimum interval of the offset estimation.
 * The unit is msec in 12Q4 format.
 */
#define OFFSET_ESTIMATION_INTERVAL  (1440)
/*!
 * DOEaG mode
 * The mode set to 0, DOEaG is called.
 * The mode set to 1, DOE is called.
 */
#define DOEAG_MODE                  (0)

#ifdef AKSC_DOEAG_DEBUG
int16 AKSC_DOEaG_InputChecker(
    const int16 hdt_aG,
    const int16 gdt_aG,
    const int16 oedt,
    int16       *flg,
    int16       *hdt_aG_prev,
    int16       *gdt_aG_prev,
    int16       *gdt)
{
    /* check invalid input */
    if ((gdt_aG < 0) && (hdt_aG < 0)) {
        *gdt = 0;
        return 0;
    }

    /* update gyro data */
    if (gdt_aG >= 0) {
        if ((*hdt_aG_prev >= oedt) && (*flg == 1)) {
            *gdt = (int16)(gdt_aG + (*hdt_aG_prev - *gdt_aG_prev));
        } else {
            *gdt = (int16)(gdt_aG - *gdt_aG_prev);
        }

        *gdt_aG_prev = gdt_aG;
        *flg = 0;
    } else {
        *gdt = 0;
    }

    /* update mag data */
    if (hdt_aG >= 0) {
        *hdt_aG_prev = hdt_aG;
        *flg = 1;
    }

    /* check invalid input */
    if (*gdt <= 0) {
        return 0;
    } else {
        return 1;
    }
}
#endif

/*****************************************************************************/
void InitAKSCPRMS(struct AKL_SCL_PRMS *prms)
{
    /* Sensitivity */
    prms->v_hs.u.x = AKSC_HSENSE_TARGET;
    prms->v_hs.u.y = AKSC_HSENSE_TARGET;
    prms->v_hs.u.z = AKSC_HSENSE_TARGET;
    /* ASA */
    prms->v_asa.u.x = 0x80;
    prms->v_asa.u.y = 0x80;
    prms->v_asa.u.z = 0x80;

    /* HDOEaG */
    prms->m_hdst = AKSC_HDST_UNSOLVED;

    /* HDOE */
#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
    prms->m_hdst_doe = AKSC_HDST_UNSOLVED;
#endif

    /* (m_hdata is initialized with AKSC_InitDecomp) */
    prms->m_hnave = AKM_DEFAULT_HNAVE;

    /* Formation */
    prms->m_form = prms->m_curForm;
#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
    prms->m_form_doe = prms->m_curForm;
#endif
}

/*****************************************************************************/
void SetDefaultNV(
    struct AKL_NV_PRMS nv[],
    const uint8        num_formation)
{
    int16 i;
#ifdef AKMD_ENABLE_DOEPLUS
    int16 j;
#endif
#ifdef AKM_ENABLE_PDC
    int16 k;
#endif

#ifdef AKM_ENABLE_PDC
    /* UNIT parameter for PDC */
    const uint8 U_PDC[AKL_PDC_SIZE] = {
        32, 82, 186, 174, 49, 255, 255, 96, 214,
        55, 231, 85, 38, 206, 255, 242, 255, 255,
        127, 154, 191, 252, 255, 255, 9, 38, 255
    };
#endif

    /* Set parameter to HDST, HO, HREF, HBASE */
    for (i = 0; i < (int16_t)num_formation; i++) {
        nv[i].a_hsuc_hdst = AKSC_HDST_UNSOLVED;
        nv[i].va_hsuc_ho.u.x = 0;
        nv[i].va_hsuc_ho.u.y = 0;
        nv[i].va_hsuc_ho.u.z = 0;
        nv[i].va_hflucv_href.u.x = 0;
        nv[i].va_hflucv_href.u.y = 0;
        nv[i].va_hflucv_href.u.z = 0;
        nv[i].va_hsuc_hbase.u.x = 0;
        nv[i].va_hsuc_hbase.u.y = 0;
        nv[i].va_hsuc_hbase.u.z = 0;
#ifdef AKMD_ENABLE_DOEPLUS
        for (j = 0; j < AKSC_DOEP_SIZE; j++) {
            nv[i].a_doep_prms[j] = (AKSC_FLOAT)(0.);
        }
#endif
#ifdef AKM_ENABLE_PDC
        /* Set unit parameter */
        for (k = 0; k < AKL_PDC_SIZE; k++) {
            nv[i].a_pdc[k] = U_PDC[k];
        }
#endif
#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
        nv[i].a_hsuc_hdst_doe = AKSC_HDST_UNSOLVED;
        nv[i].va_hsuc_ho_doe.u.x = 0;
        nv[i].va_hsuc_ho_doe.u.y = 0;
        nv[i].va_hsuc_ho_doe.u.z = 0;
        nv[i].va_hflucv_href_doe.u.x = 0;
        nv[i].va_hflucv_href_doe.u.y = 0;
        nv[i].va_hflucv_href_doe.u.z = 0;
        nv[i].va_hsuc_hbase_doe.u.x = 0;
        nv[i].va_hsuc_hbase_doe.u.y = 0;
        nv[i].va_hsuc_hbase_doe.u.z = 0;
#ifdef AKMD_ENABLE_DOEPLUS
        for (j = 0; j < AKSC_DOEP_SIZE; j++) {
            nv[i].a_doep_prms_doe[j] = (AKSC_FLOAT)(0.);
        }
#endif
#endif
    }
}

#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
/*****************************************************************************/
int16 InitMeasure_doe(struct AKL_SCL_PRMS *prms)
{
    prms->m_form_doe = 0;

    /* Restore the value when succeeding in estimating of HOffset. */
    prms->v_ho_doe = prms->ps_nv[prms->m_form_doe].va_hsuc_ho_doe;
    prms->v_ho32_doe.u.x =
        (int32)prms->ps_nv[prms->m_form_doe].va_hsuc_ho_doe.u.x;
    prms->v_ho32_doe.u.y =
        (int32)prms->ps_nv[prms->m_form_doe].va_hsuc_ho_doe.u.y;
    prms->v_ho32_doe.u.z =
        (int32)prms->ps_nv[prms->m_form_doe].va_hsuc_ho_doe.u.z;
    prms->m_hdst_doe = prms->ps_nv[prms->m_form_doe].a_hsuc_hdst_doe;
    prms->v_hbase_doe = prms->ps_nv[prms->m_form_doe].va_hsuc_hbase_doe;

    /* Initialize the decompose parameters */
    AKSC_InitDecomp(prms->va_hdata_doe);

    /* Initialize HDOE parameters */
    AKSC_InitHDOEProcPrmsS3(
        &prms->s_hdoev_doe,
        1,
        &prms->v_ho_doe,
        prms->m_hdst_doe
    );

    AKSC_InitHFlucCheck(
        &(prms->s_hflucv_doe),
        &(prms->ps_nv[prms->m_form_doe].va_hflucv_href_doe),
        HFLUCV_TH
    );

#ifdef AKMD_ENABLE_DOEPLUS
    /* Initialize DOEPlus parameters */
    AKSC_InitDOEPlus(prms->ps_doep_var_doe);
    prms->m_doep_lv_doe = AKSC_LoadDOEPlus(
            prms->ps_nv[prms->m_form_doe].a_doep_prms_doe,
            prms->ps_doep_var_doe);
    AKSC_InitDecomp(prms->va_hdata_plus_doe);
#endif

    /* Reset time stamp. */
    prms->m_ts_hvec_doe = 0;
    prms->m_ts_ev_form_doe = 0;
    prms->m_ts_ev_doe = 0;

    return AKRET_PROC_SUCCEED;
}
#endif
/*****************************************************************************/
int16 InitMeasure(struct AKL_SCL_PRMS *prms)
{
    int16 ret;
    /* Conversion matrix from Android to SmartCompass coordination */
    const I16MATRIX hlayout = {{
                                   0, 1, 0,
                                   -1, 0, 0,
                                   0, 0, 1
                               }};

    prms->m_form = 0U;

    /* Restore the value when succeeding in estimating of HOffset. */
    prms->v_ho = prms->ps_nv[prms->m_form].va_hsuc_ho;
    prms->v_ho32.u.x = (int32)prms->ps_nv[prms->m_form].va_hsuc_ho.u.x;
    prms->v_ho32.u.y = (int32)prms->ps_nv[prms->m_form].va_hsuc_ho.u.y;
    prms->v_ho32.u.z = (int32)prms->ps_nv[prms->m_form].va_hsuc_ho.u.z;
    prms->m_hdst = prms->ps_nv[prms->m_form].a_hsuc_hdst;
    prms->v_hbase = prms->ps_nv[prms->m_form].va_hsuc_hbase;

    /* Initialize the decompose parameters */
    AKSC_InitDecomp(prms->va_hdata);

    /* Initialize HDOEaG parameters */
    AKSC_InitHDOEaG(
        prms->ps_doeag_var,
        1,
        &hlayout,
        &prms->v_ho,
        prms->m_hdst
    );

    ret = AKSC_InitHFlucCheck(
            &(prms->s_hflucv),
            &(prms->ps_nv[prms->m_form].va_hflucv_href),
            HFLUCV_TH
        );

    if (ret == 0) {
        return AKRET_PROC_FAIL;
    }

    /* Initialize for Direction9D */
    ret = AKSC_InitDirection9D(prms->p_hd9d);

    if (ret == 0) {
        return AKRET_PROC_FAIL;
    }

#ifdef AKMD_ENABLE_DOEPLUS
    /* Initialize DOEPlus parameters */
    AKSC_InitDOEPlus(prms->ps_doep_var);
    prms->m_doep_lv = AKSC_LoadDOEPlus(
            prms->ps_nv[prms->m_form].a_doep_prms,
            prms->ps_doep_var);
    AKSC_InitDecomp(prms->va_hdata_plus);
#endif

#ifdef AKM_ENABLE_PDC
    /* Set PDC parameter pointer */
    prms->pa_pdcptr = prms->ps_nv[prms->m_form].a_pdc;
#else
    prms->pa_pdcptr = 0;
#endif

    /* Set offset estimation interval */
    prms->m_oedt = OFFSET_ESTIMATION_INTERVAL;

    /* Set DOEaG mode */
    prms->m_mode = DOEAG_MODE;

    /* Reset time stamp. */
    prms->m_ts_hvec = 0;
    prms->m_ts_avec = 0;
    prms->m_ts_gvec = 0;
    prms->m_ts_ev_form = 0;
    prms->m_ts_ev_d9d = 0;
#ifdef AKMD_ENABLE_DOEPLUS
    prms->m_ts_ev_doep = 0;
#endif

    return AKRET_PROC_SUCCEED;
}

#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
/*****************************************************************************/
int16 GetMagneticVector(
    const int16         bData[],
    struct AKL_SCL_PRMS *prms,
    const int64_t       ts_mag)
{
    const int16vec hrefZero = {{0, 0, 0}};

    int16vec have;
    int16    temperature, dor, derr, hofl, cb, dc;
    int32vec preHbase;
    int16    overflow;
    int16    hfluc;
    int16    hdSucc;
    int16    aksc_ret;
    int16    ret;
#ifdef AKMD_ENABLE_DOEPLUS
    int16 i;
    int16 doep_ret;
#endif

    have.u.x = 0;
    have.u.y = 0;
    have.u.z = 0;
    temperature = 0;
    dor = 0;
    derr = 0;
    hofl = 0;
    cb = 0;
    dc = 0;

    preHbase = prms->v_hbase_doe;
    overflow = 0;
    ret = AKRET_PROC_SUCCEED;

    /* Subtract the formation suspend counter */
    if (prms->m_ts_ev_form_doe > 0) {
        /* If suspended time exceeds the threshold, reset parameters. */
        if ((ts_mag - prms->m_ts_ev_form_doe) >= AKM_INTERVAL_SUSPEND) {
            /* Restore the value when succeeding in estimating of HOffset. */
            prms->v_ho_doe = prms->ps_nv[prms->m_form_doe].va_hsuc_ho_doe;
            prms->v_ho32_doe.u.x =
                (int32)prms->ps_nv[prms->m_form_doe].va_hsuc_ho_doe.u.x;
            prms->v_ho32_doe.u.y =
                (int32)prms->ps_nv[prms->m_form_doe].va_hsuc_ho_doe.u.y;
            prms->v_ho32_doe.u.z =
                (int32)prms->ps_nv[prms->m_form_doe].va_hsuc_ho_doe.u.z;

            prms->m_hdst_doe = prms->ps_nv[prms->m_form_doe].a_hsuc_hdst_doe;
            prms->v_hbase_doe = prms->ps_nv[prms->m_form_doe].va_hsuc_hbase_doe;

            /* Initialize the decompose parameters */
            AKSC_InitDecomp(prms->va_hdata_doe);

#ifdef AKMD_ENABLE_DOEPLUS
            /* Initialize DOEPlus parameters */
            AKSC_InitDOEPlus(prms->ps_doep_var_doe);
            prms->m_doep_lv_doe = AKSC_LoadDOEPlus(
                    prms->ps_nv[prms->m_form_doe].a_doep_prms_doe,
                    prms->ps_doep_var_doe);
            AKSC_InitDecomp(prms->va_hdata_plus_doe);
#endif
#ifdef AKM_ENABLE_PDC
            prms->pa_pdcptr = prms->ps_nv[prms->m_form_doe].a_pdc;
#endif

            /* Initialize HDOE parameters */
            AKSC_InitHDOEProcPrmsS3(
                &prms->s_hdoev_doe,
                1,
                &prms->v_ho_doe,
                prms->m_hdst_doe
            );

            /* Initialize HFlucCheck parameters */
            AKSC_InitHFlucCheck(
                &(prms->s_hflucv_doe),
                &(prms->ps_nv[prms->m_form_doe].va_hflucv_href_doe),
                HFLUCV_TH
            );
            /* Reset timestamp */
            prms->m_ts_ev_form_doe = 0;
        }
    }

    /* Decompose one block data into each Magnetic sensor's data */
    aksc_ret = AKSC_DecompS3(
            prms->m_device,
            bData,
            prms->m_hnave,
            &prms->v_asa,
            prms->pa_pdcptr,
            prms->va_hdata_doe,
            &prms->v_hbase_doe,
            &prms->m_hn,
            &have,
            &temperature,
            &dor,
            &derr,
            &hofl,
            &cb,
            &dc
        );

    if (aksc_ret == 0) {
        return AKRET_PROC_FAIL;
    }

    if (prms->m_form_doe != prms->m_curForm) {
        /* Formation changed! */
        prms->m_form_doe = prms->m_curForm;
        prms->m_ts_ev_form_doe = ts_mag;
        ret |= AKRET_FORMATION_CHANGED;
        return ret;
    }

    /* Check derr */
    if (derr == 1) {
        ret |= AKRET_DATA_READERROR;
        return ret;
    }

    /* Check hofl */
    if (hofl == 1) {
        if (prms->m_ts_ev_form_doe == 0) {
            /* Set a HDOE level as "HDST_UNSOLVED" */
            AKSC_SetHDOELevel(
                &prms->s_hdoev_doe,
                &prms->v_ho_doe,
                AKSC_HDST_UNSOLVED,
                1
            );
            prms->m_hdst_doe = AKSC_HDST_UNSOLVED;
        }

        ret |= AKRET_DATA_OVERFLOW;
        return ret;
    }

    /* Check hbase */
    if (cb == 1) {
        /* Translate HOffset */
        AKSC_TransByHbase(
            &(preHbase),
            &(prms->v_hbase_doe),
            &(prms->v_ho_doe),
            &(prms->v_ho32_doe),
            &overflow
        );

        if (overflow == 1) {
            ret |= AKRET_OFFSET_OVERFLOW;
        }

        /* Set hflucv.href to 0 */
        AKSC_InitHFlucCheck(
            &(prms->s_hflucv_doe),
            &(hrefZero),
            HFLUCV_TH
        );

        if (prms->m_ts_ev_form_doe == 0) {
            AKSC_SetHDOELevel(
                &prms->s_hdoev_doe,
                &prms->v_ho_doe,
                AKSC_HDST_UNSOLVED,
                1
            );
            prms->m_hdst_doe = AKSC_HDST_UNSOLVED;
        }

        ret |= AKRET_HBASE_CHANGED;
        return ret;
    }

    if (prms->m_ts_ev_form_doe == 0) {
        /* Detect a fluctuation of magnetic field. */
        hfluc =
            AKSC_HFlucCheck(&(prms->s_hflucv_doe), &(prms->va_hdata_doe[0]));

        if (hfluc == 1) {
            /* Set a HDOE level as "HDST_UNSOLVED" */
            AKSC_SetHDOELevel(
                &prms->s_hdoev_doe,
                &prms->v_ho_doe,
                AKSC_HDST_UNSOLVED,
                1
            );
            prms->m_hdst_doe = AKSC_HDST_UNSOLVED;
            ret |= AKRET_HFLUC_OCCURRED;
            return ret;
        } else {
            if ((ts_mag - prms->m_ts_ev_doe) >= AKM_INTERVAL_DOE) {
                prms->m_ts_ev_doe = ts_mag;
#ifdef AKMD_ENABLE_DOEPLUS
                /* Compensate Magnetic Distortion by DOEPlus */
                doep_ret = AKSC_DOEPlus(
                        prms->va_hdata_doe,
                        prms->ps_doep_var_doe,
                        &prms->m_doep_lv_doe);

                /* Save DOEPlus parameters */
                if ((doep_ret == 1) && (prms->m_doep_lv_doe == 3)) {
                    AKSC_SaveDOEPlus(
                        prms->ps_doep_var_doe,
                        prms->ps_nv[prms->m_form_doe].a_doep_prms_doe
                    );
                }

                /* Calculate compensated vector for DOE */
                for (i = 0; i < prms->m_hn; i++) {
                    AKSC_DOEPlus_DistCompen(
                        &prms->va_hdata_doe[i],
                        prms->ps_doep_var_doe,
                        &prms->va_hdata_plus_doe[i]);
                }

                /*Calculate Magnetic sensor's offset by DOE */
                hdSucc = AKSC_HDOEProcessS3(
                        prms->s_cert.a_licenser,
                        prms->s_cert.a_licensee,
                        prms->s_cert.a_key,
                        &prms->s_hdoev_doe,
                        prms->va_hdata_plus_doe,
                        prms->m_hn,
                        &prms->v_ho_doe,
                        &prms->m_hdst_doe);

#else
                /*Calculate Magnetic sensor's offset by DOE */
                hdSucc = AKSC_HDOEProcessS3(
                        prms->s_cert.a_licenser,
                        prms->s_cert.a_licensee,
                        prms->s_cert.a_key,
                        &prms->s_hdoev_doe,
                        prms->va_hdata_doe,
                        prms->m_hn,
                        &prms->v_ho_doe,
                        &prms->m_hdst_doe);
#endif

                if (hdSucc == AKSC_CERTIFICATION_DENIED) {
                    return AKRET_CERTIFICATION_ERROR;
                }

                if (hdSucc > 0) {
                    prms->ps_nv[prms->m_form_doe].va_hsuc_ho = prms->v_ho_doe;
                    prms->v_ho32_doe.u.x = (int32)prms->v_ho_doe.u.x;
                    prms->v_ho32_doe.u.y = (int32)prms->v_ho_doe.u.y;
                    prms->v_ho32_doe.u.z = (int32)prms->v_ho_doe.u.z;

                    prms->ps_nv[prms->m_form_doe].a_hsuc_hdst =
                        prms->m_hdst_doe;
                    prms->ps_nv[prms->m_form_doe].va_hflucv_href_doe =
                        prms->s_hflucv_doe.href;
                    prms->ps_nv[prms->m_form_doe].va_hsuc_hbase_doe =
                        prms->v_hbase_doe;
                }
            }
        }
    }

#ifdef AKMD_ENABLE_DOEPLUS
    /* Calculate compensated vector */
    AKSC_DOEPlus_DistCompen(&have, prms->ps_doep_var_doe, &have);
#endif

    /* Subtract offset and normalize magnetic field vector. */
    aksc_ret = AKSC_VNorm(
            &have,
            &prms->v_ho_doe,
            &prms->v_hs,
            AKSC_HSENSE_TARGET,
            &prms->v_hvec_doe
        );

    if (aksc_ret == 0) {
        ret |= AKRET_VNORM_ERROR;
        return ret;
    }

    /* update time stamp for hvec */
    prms->m_ts_hvec_doe = ts_mag;

    return AKRET_PROC_SUCCEED;
}
#endif
/*****************************************************************************/
int16 GetMagneticVectorDecomp(
    const int16         bData[],
    struct AKL_SCL_PRMS *prms,
    const int64_t       ts_mag)
{
    const int16vec hrefZero = {{0, 0, 0}};
    /* Conversion matrix from Android to SmartCompass coordination */
    const I16MATRIX hlayout = {{
                                   0, 1, 0,
                                   -1, 0, 0,
                                   0, 0, 1
                               }};

    int16    temperature, dor, derr, hofl, cb, dc;
    int32vec preHbase;
    int16    overflow;
    int16    aksc_ret;
    int16    ret;
    int16    tmpret;

    temperature = 0;
    dor = 0;
    derr = 0;
    hofl = 0;
    cb = 0;
    dc = 0;

    preHbase = prms->v_hbase;
    overflow = 0;
    ret = AKRET_PROC_SUCCEED;

    /* Subtract the formation suspend counter */
    if (prms->m_ts_ev_form > 0U) {
        /* If suspended time exceeds the threshold, reset parameters. */
        if ((ts_mag - prms->m_ts_ev_form) >= AKM_INTERVAL_SUSPEND) {
            /* Restore the value when succeeding in estimating of HOffset. */
            prms->v_ho = prms->ps_nv[prms->m_form].va_hsuc_ho;
            prms->v_ho32.u.x = (int32)prms->ps_nv[prms->m_form].va_hsuc_ho.u.x;
            prms->v_ho32.u.y = (int32)prms->ps_nv[prms->m_form].va_hsuc_ho.u.y;
            prms->v_ho32.u.z = (int32)prms->ps_nv[prms->m_form].va_hsuc_ho.u.z;

            prms->m_hdst = prms->ps_nv[prms->m_form].a_hsuc_hdst;
            prms->v_hbase = prms->ps_nv[prms->m_form].va_hsuc_hbase;

            /* Initialize the decompose parameters */
            AKSC_InitDecomp(prms->va_hdata);

#ifdef AKMD_ENABLE_DOEPLUS
            /* Initialize DOEPlus parameters */
            AKSC_InitDOEPlus(prms->ps_doep_var);
            prms->m_doep_lv = AKSC_LoadDOEPlus(
                    prms->ps_nv[prms->m_form].a_doep_prms,
                    prms->ps_doep_var);
            AKSC_InitDecomp(prms->va_hdata_plus);
#endif
#ifdef AKM_ENABLE_PDC
            prms->pa_pdcptr = prms->ps_nv[prms->m_form].a_pdc;
#endif
            /* Initialize HDOEaG parameters */
            AKSC_InitHDOEaG(
                prms->ps_doeag_var,
                1,
                &hlayout,
                &prms->v_ho,
                prms->m_hdst
            );

            /* Initialize HFlucCheck parameters */
            tmpret = AKSC_InitHFlucCheck(
                    &(prms->s_hflucv),
                    &(prms->ps_nv[prms->m_form].va_hflucv_href),
                    HFLUCV_TH
                );
            if (tmpret != 0)
                {
                }
            /* Reset timestamp */
            prms->m_ts_ev_form = 0U;
        }
    }

    /* Decompose one block data into each Magnetic sensor's data */
    aksc_ret = AKSC_DecompS3(
            prms->m_device,
            bData,
            prms->m_hnave,
            &prms->v_asa,
            prms->pa_pdcptr,
            prms->va_hdata,
            &prms->v_hbase,
            &prms->m_hn,
            &prms->v_have,
            &temperature,
            &dor,
            &derr,
            &hofl,
            &cb,
            &dc
        );

    if (aksc_ret == 0) {
        return AKRET_PROC_FAIL;
    }

    /* Example of LOG for PDC parameter
     * LOG("GetMagneticVector : ST1, HXH&HXL, HYH&HYL, HZH&HZL, ST2,"
     *     " hdata[0].u.x, hdata[0].u.y, hdata[0].u.z,"
     *     " asax, asay, asaz ="
     *     " %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
     *     bData[0],
     *     (int16_t)(((uint16_t)bData[2]) << 8 | bData[1]),
     *     (int16_t)(((uint16_t)bData[4]) << 8 | bData[3]),
     *     (int16_t)(((uint16_t)bData[6]) << 8 | bData[5]), bData[7 or 8],
     *     prms->va_hdata[0].u.x, prms->va_hdata[0].u.y, prms->va_hdata[0].u.z,
     *     prms->v_asa.u.x, prms->v_asa.u.y, prms->v_asa.u.z);
     */

    if (prms->m_form != prms->m_curForm) {
        /* Formation changed! */
        prms->m_form = prms->m_curForm;
        prms->m_ts_ev_form = ts_mag;
        ret |= AKRET_FORMATION_CHANGED;
        return ret;
    }

    /* Check derr */
    if (derr == 1) {
        ret |= AKRET_DATA_READERROR;
        return ret;
    }

    /* Check hofl */
    if (hofl == 1) {
        if (prms->m_ts_ev_form == 0U) {
            /* Set a HDOE level as "HDST_UNSOLVED" */
            AKSC_SetHDOEaGLevel(
                prms->ps_doeag_var,
                &hlayout,
                &prms->v_ho,
                AKSC_HDST_UNSOLVED,
                1
            );
            prms->m_hdst = AKSC_HDST_UNSOLVED;
        }

        ret |= AKRET_DATA_OVERFLOW;
        return ret;
    }

    /* Check hbase */
    if (cb == 1) {
        /* Translate HOffset */
        AKSC_TransByHbase(
            &(preHbase),
            &(prms->v_hbase),
            &(prms->v_ho),
            &(prms->v_ho32),
            &overflow
        );

        if (overflow == 1) {
            ret |= AKRET_OFFSET_OVERFLOW;
        }

        /* Set hflucv.href to 0 */
        tmpret = AKSC_InitHFlucCheck(
                &(prms->s_hflucv),
                &(hrefZero),
                HFLUCV_TH
            );
        if (tmpret != 0)
            {
            }

        if (prms->m_ts_ev_form == 0U) {
            AKSC_SetHDOEaGLevel(
                prms->ps_doeag_var,
                &hlayout,
                &prms->v_ho,
                AKSC_HDST_UNSOLVED,
                1
            );
            prms->m_hdst = AKSC_HDST_UNSOLVED;
        }

        ret |= AKRET_HBASE_CHANGED;
        return ret;
    }

    return AKRET_PROC_SUCCEED;
}

/*****************************************************************************/
int16 GetMagneticVectorOffset(
    struct AKL_SCL_PRMS *prms,
    const int64_t       ts_mag,
    int16_t             hdt_ag,
    int16_t             gdt_ag)
{
    /* Conversion matrix from Android to SmartCompass coordination */
    const I16MATRIX hlayout = {{
                                   0, 1, 0,
                                   -1, 0, 0,
                                   0, 0, 1
                               }};
    const I16MATRIX glayout = {{
                                   0, 1, 0,
                                   -1, 0, 0,
                                   0, 0, 1
                               }};
    int16           hfluc;
    int16           hdSucc;
    int16           aksc_ret;
#ifdef AKMD_ENABLE_DOEPLUS
    int16 i;
    int16 doep_ret;
#endif

    if (prms->m_ts_ev_form == 0U) {
        /* Detect a fluctuation of magnetic field. */
        hfluc = AKSC_HFlucCheck(&(prms->s_hflucv), &(prms->va_hdata[0]));

        if (hfluc == 1) {
            /* Set a HDOE level as "HDST_UNSOLVED" */
            AKSC_SetHDOEaGLevel(
                prms->ps_doeag_var,
                &hlayout,
                &prms->v_ho,
                AKSC_HDST_UNSOLVED,
                1
            );

            prms->m_hdst = AKSC_HDST_UNSOLVED;
            return AKRET_HFLUC_OCCURRED;
        } else {
#ifdef AKMD_ENABLE_DOEPLUS
            if (hdt_ag > 0) {
                if ((ts_mag - prms->m_ts_ev_doep) >= AKM_INTERVAL_DOEP) {
                    prms->m_ts_ev_doep = ts_mag;
                    /* Compensate Magnetic Distortion by DOEPlus */
                    doep_ret = AKSC_DOEPlus(
                            prms->va_hdata,
                            prms->ps_doep_var,
                            &prms->m_doep_lv);

                    /* Save DOEPlus parameters */
                    if ((doep_ret == 1) && (prms->m_doep_lv == 3)) {
                        AKSC_SaveDOEPlus(
                            prms->ps_doep_var,
                            prms->ps_nv[prms->m_form].a_doep_prms
                        );
                    }
                }

                /* Calculate compensated vector for DOEaG */
                for (i = 0; i < prms->m_hn; i++) {
                    AKSC_DOEPlus_DistCompen(
                        &prms->va_hdata[i],
                        prms->ps_doep_var,
                        &prms->va_hdata_plus[i]);
                }
            }

            /* Calculate Magnetic sensor's offset by DOEaG */
            hdSucc = AKSC_HDOEaG(
                    prms->s_cert.a_licenser,
                    prms->s_cert.a_licensee,
                    prms->s_cert.a_key,
                    prms->m_mode,
                    prms->m_oedt,
                    hdt_ag,
                    gdt_ag,
                    prms->va_hdata_plus,
                    &prms->v_gvec,
                    &hlayout,
                    &glayout,
                    prms->ps_doeag_var,
                    &prms->v_ho,
                    &prms->m_hdst);
#else
            hdSucc = AKSC_HDOEaG(
                    prms->s_cert.a_licenser,
                    prms->s_cert.a_licensee,
                    prms->s_cert.a_key,
                    prms->m_mode,
                    prms->m_oedt,
                    hdt_ag,
                    gdt_ag,
                    prms->va_hdata,
                    &prms->v_gvec,
                    &hlayout,
                    &glayout,
                    prms->ps_doeag_var,
                    &prms->v_ho,
                    &prms->m_hdst);
#if 0
            printf("AKSC_HDOEaG return:%dmode:%d hdt,gdt,hdata,gvec:%d,%d,%d,%d,%d,%d,%d,%d,hdst:%d\n",
                hdSucc,prms->m_mode,hdt_ag,gdt_ag,
                prms->va_hdata[0],prms->va_hdata[1],prms->va_hdata[2],
                prms->v_gvec.u.x,prms->v_gvec.u.y,prms->v_gvec.u.z,
                prms->m_hdst);
#endif
#endif
            AKL_DEBUG_SCL_FRET_CHK(prms, hdSucc);

            if (hdSucc == AKSC_CERTIFICATION_DENIED) {
                return AKRET_CERTIFICATION_ERROR;
            }

            if (hdSucc > 0) {
                prms->ps_nv[prms->m_form].va_hsuc_ho = prms->v_ho;
                prms->v_ho32.u.x = (int32)prms->v_ho.u.x;
                prms->v_ho32.u.y = (int32)prms->v_ho.u.y;
                prms->v_ho32.u.z = (int32)prms->v_ho.u.z;

                prms->ps_nv[prms->m_form].a_hsuc_hdst = prms->m_hdst;
                prms->ps_nv[prms->m_form].va_hflucv_href =
                    prms->s_hflucv.href;
                prms->ps_nv[prms->m_form].va_hsuc_hbase = prms->v_hbase;
            }
        }
    }

#ifdef AKSC_DOEAG_DEBUG
    prms->m_doeag_ic_ret = AKSC_DOEaG_InputChecker(
            hdt_ag,
            gdt_ag,
            prms->m_oedt,
            &prms->m_flg,
            &prms->m_hdt_aG_prev,
            &prms->m_gdt_aG_prev,
            &prms->m_gdt
        );
    /* Example of LOG for AKSC_DOEaG_InputChecker
    LOG("AKSC_DOEaG_InputChecker: ret, hdt_ag, gdt_ag, gdt = %d, %d, %d, %d\n",
        prms->m_doeag_ic_ret, hdt_ag, gdt_ag, prms->m_gdt);
    */
#endif

#ifdef AKMD_ENABLE_DOEPLUS
    /* Only the case when valid mag data is updated */
    if (hdt_ag > 0) {
        /* Calculate compensated vector */
        AKSC_DOEPlus_DistCompen(&prms->v_have, prms->ps_doep_var,
                                &prms->v_have);
    }
#endif

    /* Subtract offset and normalize magnetic field vector. */
    aksc_ret = AKSC_VNorm(
            &prms->v_have,
            &prms->v_ho,
            &prms->v_hs,
            AKSC_HSENSE_TARGET,
            &prms->v_hvec
        );

    if (aksc_ret == 0) {
        return AKRET_VNORM_ERROR;
    }

    return AKRET_PROC_SUCCEED;
}

#ifdef AKM_ENABLE_D9D
/*****************************************************************************/
int16 CalcDirection9D(
    struct AKL_SCL_PRMS *prms,
    int16_t             hdt,
    int16_t             adt,
    int16_t             gdt)
{
    int16     preThe, fret, swp;
    int16     delta, hr, hrhoriz, ar;
    int16     phi90, eta180;
    I16MATRIX mat;
    int16vec  dvec;

    /* Conversion matrix from Android to SmartCompass coordination */
    const I16MATRIX hlayout = {{
                                   0, 1, 0,
                                   -1, 0, 0,
                                   0, 0, 1
                               }};
    const I16MATRIX alayout = {{
                                   0, -1, 0,
                                   1, 0, 0,
                                   0, 0, -1
                               }};
    const I16MATRIX glayout = {{
                                   0, 1, 0,
                                   -1, 0, 0,
                                   0, 0, 1
                               }};

    /* store previous value */
    preThe = prms->m_theta;

    /* dvec is always 0 */
    dvec.u.x = 0;
    dvec.u.y = 0;
    dvec.u.z = 0;

    fret = AKSC_Direction9D(
            prms->p_hd9d,
            prms->s_cert.a_licenser,
            prms->s_cert.a_licensee,
            prms->s_cert.a_key,
            &prms->v_hvec,
            &prms->v_avec,
            &prms->v_gvec,
            hdt,
            adt,
            gdt,
            &dvec,
            &hlayout,
            &alayout,
            &glayout,
            &prms->m_theta,
            &delta,
            &hr,
            &hrhoriz,
            &ar,
            &prms->m_phi180,
            &phi90,
            &eta180,
            &prms->m_eta90,
            &mat,
            &prms->s_quat,
            &prms->v_gravity,
            &prms->v_lacc
        );

    prms->m_theta = AKSC_ThetaFilter(
            prms->m_theta,
            preThe,
            THETAFILTER_SCALE
        );

    AKL_DEBUG_SCL_FRET_CHK(prms, fret);

    if (fret != 0x1F) {
        return AKRET_PROC_FAIL;
    }

    /* Convert Yaw, Pitch, Roll angle to Android coordinate system
     * from: AKM coordinate, AKSC units
     * to  : Android coordinate, AKSC units.
     */
    prms->m_eta90 = -prms->m_eta90;
    /* Convert Gravity vector
     * from: AKM coordinate, AKSC units
     * to  : Android coordinate, AKSC units.
     */
    swp = prms->v_gravity.u.x;
    prms->v_gravity.u.x = prms->v_gravity.u.y;
    prms->v_gravity.u.y = -(swp);
    prms->v_gravity.u.z = -(prms->v_gravity.u.z);
    /* Convert Linear acceleration vector
     * from: AKM coordinate, AKSC units
     * to  : Android coordinate, AKSC units.
     */
    swp = prms->v_lacc.u.x;
    prms->v_lacc.u.x = prms->v_lacc.u.y;
    prms->v_lacc.u.y = -(swp);
    prms->v_lacc.u.z = -(prms->v_lacc.u.z);
    /* Convert Quaternion
     * from: AKM coordinate, AKSC units
     * to  : Android coordinate, AKSC units.
     */
    swp = prms->s_quat.u.x;
    prms->s_quat.u.x = prms->s_quat.u.y;
    prms->s_quat.u.y = -(swp);
    prms->s_quat.u.z = -(prms->s_quat.u.z);

    return AKRET_PROC_SUCCEED;
}
#endif
/*****************************************************************************/
int16 CalcDirectionS3(struct AKL_SCL_PRMS *prms)
{
    int16     preThe, fret, swp;
    int16     delta, hr, hrhoriz, ar;
    int16     phi90, eta180;
    I16MATRIX mat;
    int16vec  dvec;

    /* Conversion matrix from Android to SmartCompass coordination */
    const I16MATRIX hlayout = {{
                                   0, 1, 0,
                                   -1, 0, 0,
                                   0, 0, 1
                               }};
    const I16MATRIX alayout = {{
                                   0, -1, 0,
                                   1, 0, 0,
                                   0, 0, -1
                               }};

    /* store previous value */
    preThe = prms->m_theta;

    /* dvec is always 0 */
    dvec.u.x = 0;
    dvec.u.y = 0;
    dvec.u.z = 0;

    fret = AKSC_DirectionS3(
            prms->s_cert.a_licenser,
            prms->s_cert.a_licensee,
            prms->s_cert.a_key,
            &prms->v_hvec_fusion,
            &prms->v_avec_fusion,
            &dvec,
            &hlayout,
            &alayout,
            &prms->m_theta,
            &delta,
            &hr,
            &hrhoriz,
            &ar,
            &prms->m_phi180,
            &phi90,
            &eta180,
            &prms->m_eta90,
            &mat,
            &prms->s_quat
        );
    prms->m_theta = AKSC_ThetaFilter(
            prms->m_theta,
            preThe,
            THETAFILTER_SCALE
        );
    AKL_DEBUG_SCL_FRET_CHK(prms, fret);

    if (fret == AKSC_CERTIFICATION_DENIED) {
        return AKRET_CERTIFICATION_ERROR;
    }
    printf("CalcDirectionS3 %d,%d,%d\n",prms->m_theta,prms->m_phi180,prms->m_eta90);
    if (fret & 0x02) {
        /* Convert Yaw, Pitch, Roll angle to Android coordinate system
         * from: AKM coordinate, AKSC units
         * to  : Android coordinate, AKSC units.
         */
        prms->m_eta90 = -prms->m_eta90;
        /* Convert Quaternion
         * from: AKM coordinate, AKSC units
         * to  : Android coordinate, AKSC units.
         */
        swp = prms->s_quat.u.x;
        prms->s_quat.u.x = prms->s_quat.u.y;
        prms->s_quat.u.y = -(swp);
        prms->s_quat.u.z = -(prms->s_quat.u.z);
    }

    return AKRET_PROC_SUCCEED;
}
