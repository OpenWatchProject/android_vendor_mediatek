/******************************************************************************
 *
 * Copyright (c) 2016 Asahi Kasei Microdevices Corporation, Japan
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
#ifndef AKL_HELPER_DUMPOBJ
#define AKL_HELPER_DUMPOBJ

#if defined(AKM3D)
#include "lib3d/akl_smart_compass.h"
#elif defined(AKM6D)
#include "lib6d/akl_smart_compass.h"
#elif defined(AKM9D)
#include "lib9d/akl_smart_compass.h"
#elif defined(AKMAG)
#include "libag/akl_smart_compass.h"
#else
#error No library defined.
#endif

int32_t AKL_dumpObj(
    struct AKL_SCL_PRMS *obj,
    void (*dumpS)(const char *str),
    void (*dumpP)(const char *desc, const void *addr),
    void (*dumpI)(const char *desc, const int16_t val),
    void (*dumpF)(const char *desc, const float val))
{
    if ((obj == NULL) ||
        (dumpS == NULL) ||
        (dumpP == NULL) ||
        (dumpI == NULL) ||
        (dumpF == NULL)) {
        return AKM_ERR_INVALID_ARG;
    }

    /* YAML format */
    dumpS("# BEGIN: AKL_SCL_PRMS obj (YAML)");
    dumpI(" hdata[0].u.x: ", obj->va_hdata[0].u.x);
    dumpI(" hdata[0].u.y: ", obj->va_hdata[0].u.y);
    dumpI(" hdata[0].u.z: ", obj->va_hdata[0].u.z);

#ifdef AKMD_ENABLE_DOEPLUS
    dumpI(" hdata_plus[0].u.x: ", obj->va_hdata_plus[0].u.x);
    dumpI(" hdata_plus[0].u.y: ", obj->va_hdata_plus[0].u.y);
    dumpI(" hdata_plus[0].u.z: ", obj->va_hdata_plus[0].u.z);
#endif

#if defined(AKM6D) || defined(AKM9D) || defined(AKMAG)
    dumpI(" avec.u.x: ", obj->v_avec.u.x);
    dumpI(" avec.u.y: ", obj->v_avec.u.y);
    dumpI(" avec.u.z: ", obj->v_avec.u.z);
#endif

#if defined(AKM9D) || defined(AKMAG)
    dumpI(" gvec.u.x: ", obj->v_gvec.u.x);
    dumpI(" gvec.u.y: ", obj->v_gvec.u.y);
    dumpI(" gvec.u.z: ", obj->v_gvec.u.z);
#endif

    dumpF(" ts_hvec: ", (float)(obj->m_ts_hvec/1000000)); /* convert from [ns] to [ms] */

#if defined(AKM6D) || defined(AKM9D) || defined(AKMAG)
    dumpF(" ts_avec: ", (float)(obj->m_ts_avec/1000000)); /* convert from [ns] to [ms] */
#endif

#if defined(AKM9D) || defined(AKMAG)
    dumpF(" ts_gvec: ", (float)(obj->m_ts_gvec/1000000)); /* convert from [ns] to [ms] */
#endif

    dumpI(" ho.u.x: ", obj->v_ho.u.x);
    dumpI(" ho.u.y: ", obj->v_ho.u.y);
    dumpI(" ho.u.z: ", obj->v_ho.u.z);
    dumpI(" hdst: ", (int16_t)obj->m_hdst);

    dumpI(" dbg_meas_fret: ", obj->m_dbg_meas_fret);
    dumpI(" dbg_scl_fret:", obj->m_dbg_scl_fret);

    dumpS("# END: AKL_SCL_PRMS obj (YAML)");

    return AKM_SUCCESS;
}

#endif /* AKL_EXCLUDE_HELPER */
