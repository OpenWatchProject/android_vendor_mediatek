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

#include "AKL_APIs.h"          /* API decleration */
#include "akl_smart_compass.h" /* Definition of struct */
#include "measure.h"

#define AKM_DISABLE_D9D
/*! Identify the nv data. */
#define AKL_NV_MAGIC_NUMBER   (0xdeadbeefU)
#define AKL_INI_MAGIC_NUMBER  (0xbeefcafeU)
/*! The number of default form factor. */
#define AKL_DEFAULT_FORM_NUM  1
/*! 1G (= 9.8 m/s^2) in Q16 format. */
#define ACC_1G_IN_Q16         (9.80665f * 65536.0f)
/*! 1dps in Q16 format. */
#define GYR_1DPS_IN_Q16       (65536.0f)

/*! Convert from AKSC to micro tesla in Q16 format. */
#define MAG_AKSC_TO_Q16(x)    (((float32_t)(x) * 0.06f) * 65536.0f)
/*! Convert from AKSC to SI unit (m/s^2) in Q16 format. */
#define ACC_AKSC_TO_Q16(x)    (((float32_t)(x) * ACC_1G_IN_Q16) / 720.0f)
/*! Convert from SI unit (m/s^2) to AKSC format. */
#define ACC_Q16_TO_AKSC(x)    (((float32_t)(x) * 720.0f) / ACC_1G_IN_Q16)
/*! Convert from degree/second in Q4 (i.e. AKSC) format to Q16 format. */
#define GYR_AKSC_TO_Q16(x)    ((x) * 4096)
/*! Convert from degree/second in Q16 format to AKSC (i.e. Q4) format. */
#define GYR_Q16_TO_AKSC(x)    ((float32_t)(x) / 4096.0f)
/*! Convert from micro second to milli second in Q4 format. */
#define TIME_USEC_TO_AKSC(x)  (((x) * 16) / 1000)

/*! Maximum time [usec] in Q4 format. */
#define MAXTIME_USEC_IN_Q4  (2047000000)

/*! The length of measurement and status data. */
#define AKM_BDATA_SIZE_MAX  10

extern uint16_t set_cert(
    struct AKL_SCL_PRMS                 *mem,
    const struct AKL_CERTIFICATION_INFO *info);
extern uint8_t akm_wia[2];

/******************************************************************************/
/***** AKM static function prototype declarations *****************************/
static uint32_t byte_allign(
    const uint32_t sz
);

static int16_t akl_setv_mag_doeag(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data
);

#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
static int16_t akl_setv_mag_doe(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data
);
#endif

static int16_t akl_setv_acc(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data
);

static int16_t akl_setv_gyr(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data
);

#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
static int16_t akl_getv_mag_doe(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[6],
    int32_t                   *status,
    int64_t                   *timestamp
);
#endif

static int16_t akl_getv_mag_doeag(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[6],
    int32_t                   *status,
    int64_t                   *timestamp
);

static int16_t akl_getv_acc(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[3],
    int32_t                   *status,
    int64_t                   *timestamp
);

static int16_t akl_getv_gyr(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[6],
    int32_t                   *status,
    int64_t                   *timestamp
);

static int16_t akl_getv_gravity(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[3],
    int32_t                   *status,
    int64_t                   *timestamp
);

static int16_t akl_getv_lacc(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[3],
    int32_t                   *status,
    int64_t                   *timestamp
);

static int16_t akl_getv_ori(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[3],
    int32_t                   *status,
    int64_t                   *timestamp
);

static int16_t akl_getv_quat(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[4],
    int32_t                   *status,
    int64_t                   *timestamp
);

static int32_t akl_get_doelv(
    const struct AKL_SCL_PRMS *mem
);

#ifndef AKM_DISABLE_D9D
static int64_t akl_get_d9d_ts(
    const struct AKL_SCL_PRMS *mem
);
#else
static int64_t akl_get_d6d_ts(
    const struct AKL_SCL_PRMS *mem
);
#endif

/******************************************************************************/
/***** AKM static functions ***************************************************/
static uint32_t byte_allign(const uint32_t sz)
{
    if (0 >= sz) {
        return (uint32_t)0;
    }

    /* Another method.
     int32_t rem = sz % 4;
     return (rem ? (sz + (4 - rem)) : (sz));
     */
    return (((sz & 0x3) != 0) ? ((sz & ~(0x3)) + 0x4) : sz);
}

#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
/*****************************************************************************/
static int16_t akl_setv_mag_doe(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data)
{
    int32_t   tmp_i32;
    float32_t tmp_f;
    int16_t   reg[3];
    int16_t   ret;
    int16     bData[10];
    int       i;
    float32_t akm_sensitivity;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

#if 0
    if ((mem->m_device == AK8963) ||
        (mem->m_device == AK09912) ||
        (mem->m_device == AK09913) ||
        (mem->m_device == AK09915) ||
        (mem->m_device == AK09916C) ||
        (mem->m_device == AK09916D) ||
        (mem->m_device == AK09918)) {
        akm_sensitivity = (1.f / 9830.f);
    } else if (mem->m_device == AK09911) {
        akm_sensitivity = (1.f / 39322.f);
    } else {
        AKMERROR;
        return AKM_ERR_NOT_SUPPORT;
    }
#endif
    for (i = 0; i < 3; i++) {
    #if 0
        tmp_f = (float32_t)(data->u.v[i]) * akm_sensitivity;
        tmp_i32 = (int32_t)tmp_f;
        /* Limit to 16-bit value */
        if ((tmp_i32 < INT16_MIN) || (INT16_MAX < tmp_i32)) {
            return AKM_ERR_INVALID_ARG;
        }

        reg[i] = (int16)tmp_i32;
    #else
    /* Limit to 16-bit value */
    if ((tmp_i32 < INT16_MIN) || (INT16_MAX < tmp_i32)) {
        return AKM_ERR_INVALID_ARG;
    }

    reg[i] = (int16)data->u.v[i];
    #endif
    }

    /* Inverse decomp, i.e. compose */
    bData[0] = (int16)(data->status[0]);
    bData[1] = (int16)(reg[0] & 0xFF);
    bData[2] = (int16)((reg[0] >> 8) & 0xFF);
    bData[3] = (int16)(reg[1] & 0xFF);
    bData[4] = (int16)((reg[1] >> 8) & 0xFF);
    bData[5] = (int16)(reg[2] & 0xFF);
    bData[6] = (int16)((reg[2] >> 8) & 0xFF);

    if (mem->m_device == AK8963) {
        bData[7] = (int16)(data->status[1]);
    } else if ((mem->m_device == AK09911) ||
               (mem->m_device == AK09912) ||
               (mem->m_device == AK09913) ||
               (mem->m_device == AK09915)) {
        bData[7] = (int16)(0x80);
        bData[8] = (int16)(data->status[1]);
    } else if ((mem->m_device == AK09916C) ||
               (mem->m_device == AK09916D) ||
               (mem->m_device == AK09918)) {
        bData[7] = (int16)(0x80);
        bData[8] = (int16)(data->status[1]);
        bData[9] = (int16)mem->m_pat;
    }
    ret = GetMagneticVector(
            bData,
            mem,
            data->time_ns);

    /* Check the return value */
    if (ret != AKRET_PROC_SUCCEED) {
        return AKM_ERROR;
    }

    return AKM_SUCCESS;
}
#endif
/*****************************************************************************/
static int16_t akl_setv_mag_doeag(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data)
{
    int32_t   tmp_i32;
    float32_t tmp_f;
    int64_t   tmp_dt;
    int16_t   hdt_ag;
    int16_t   reg[3];
    int16_t   ret;
    int16     bData[AKM_BDATA_SIZE_MAX];
    int16_t   i;
    //float32_t akm_sensitivity;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif
#if 0
    if ((mem->m_device == AK8963) ||
        (mem->m_device == AK09912) ||
        (mem->m_device == AK09913) ||
        (mem->m_device == AK09915) ||
        (mem->m_device == AK09916C) ||
        (mem->m_device == AK09916D) ||
        (mem->m_device == AK09918)) {
        akm_sensitivity = (1.f / 9830.f);
    } else if (mem->m_device == AK09911) {
        akm_sensitivity = (1.f / 39322.f);
    } else {
        AKMERROR;
        return AKM_ERR_NOT_SUPPORT;
    }
#endif
    for (i = 0; i < 3; i++) {
        tmp_f = (float32_t)(data->u.v[i]) * 1;
        tmp_i32 = (int32_t)tmp_f;

        /* Limit to 16-bit value */
        if ((tmp_i32 < INT16_MIN) || (INT16_MAX < tmp_i32)) {
            return AKM_ERR_INVALID_ARG;
        }

        reg[i] = (int16)tmp_i32;
    }

    /* Inverse decomp, i.e. compose */
    bData[0] = (int16)(data->status[0]);
    bData[1] = (int16)(reg[0] & 0xFF);
    bData[2] = (int16)((reg[0] >> 8) & 0xFF);
    bData[3] = (int16)(reg[1] & 0xFF);
    bData[4] = (int16)((reg[1] >> 8) & 0xFF);
    bData[5] = (int16)(reg[2] & 0xFF);
    bData[6] = (int16)((reg[2] >> 8) & 0xFF);

    if (mem->m_device == AK8963) {
        bData[7] = (int16)(data->status[1]);
    } else if ((mem->m_device == AK09911) ||
               (mem->m_device == AK09912) ||
               (mem->m_device == AK09913) ||
               (mem->m_device == AK09915)) {
        bData[7] = (int16)(0x80);
        bData[8] = (int16)(data->status[1]);
    } else if ((mem->m_device == AK09916C) ||
               (mem->m_device == AK09916D) ||
               (mem->m_device == AK09918)) {
        bData[7] = (int16)(0x80);
        bData[8] = (int16)(data->status[1]);
        bData[9] = (int16)mem->m_pat;
    }

    ret = GetMagneticVectorDecomp(
            bData,
            mem,
            data->time_ns
        );

    AKL_DEBUG_MEAS_FRET_CHK(mem, ret);

    /* Check the return value */
    if (ret != AKRET_PROC_SUCCEED) {
        return AKM_ERROR;
    }

    /* Calculate hdt_ag */
    /* If m_ts_hvec is 0, it means 'DOEaG is not initialized' */
    if (mem->m_ts_hvec != 0) {
        tmp_dt = data->time_ns - mem->m_ts_hvec;
        /* nano second -> micro second */
        tmp_dt = tmp_dt / (int64_t)1000;

        /* Limit to 16-bit value */
        if (tmp_dt > (int64_t)MAXTIME_USEC_IN_Q4) {
            tmp_dt = (int64_t)MAXTIME_USEC_IN_Q4;
        }

        tmp_dt = (int64_t)TIME_USEC_TO_AKSC(tmp_dt);
        hdt_ag = (int16_t)tmp_dt;
    } else {
        /* negative value means 'not updated' */
        hdt_ag = -1;
    }

    if (hdt_ag > 0) {
        ret = GetMagneticVectorOffset(
                mem,
                data->time_ns,
                hdt_ag,
                -1);

        AKL_DEBUG_MEAS_FRET_CHK(mem, ret);

        /* Check the return value */
        if (ret != AKRET_PROC_SUCCEED) {
            return AKM_ERROR;
        }
    }

    /* hvec is successfully updated */
    mem->m_ts_hvec = data->time_ns;

    return AKM_SUCCESS;
}

/**************************************/
static int16_t akl_setv_acc(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data)
{
    int32_t   tmp_i32;
    float32_t tmp_f;
    int16_t   i;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    /* from m/s2 Q16 fromat to AKSC format */
    for (i = 0; i < 3; i++) {
        tmp_f = ACC_Q16_TO_AKSC(data->u.v[i]);
        tmp_i32 = (int32_t)tmp_f;

        if ((tmp_i32 < INT16_MIN) || (INT16_MAX < tmp_i32)) {
            return AKM_ERR_INVALID_ARG;
        }

        mem->v_avec.v[i] = (int16_t)tmp_i32;
    }

    /* avec is successfully updated */
    mem->m_ts_avec = data->time_ns;

    return AKM_SUCCESS;
}

/**************************************/
static int16_t akl_setv_fusion_acc(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data)
{
    int32_t   tmp_i32;
    float32_t tmp_f;
    int16_t   i;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    /* from m/s2 Q16 fromat to AKSC format */
    for (i = 0; i < 3; i++) {
        tmp_f = ACC_Q16_TO_AKSC(data->u.v[i]);
        tmp_i32 = (int32_t)tmp_f;

        if ((tmp_i32 < INT16_MIN) || (INT16_MAX < tmp_i32)) {
            return AKM_ERR_INVALID_ARG;
        }

        mem->v_avec_fusion.v[i] = (int16_t)tmp_i32;
    }

    /* avec is successfully updated */
    mem->m_ts_avec_fusion= data->time_ns;

    return AKM_SUCCESS;
}
static int16_t akl_setv_fusion_mag(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data)
{
    int32_t   tmp_i32;
    float32_t tmp_f;
    int16_t   i;
    float32_t akm_sensitivity;
#ifdef AKL_ARGUMENT_CHECK

        if (mem == NULL) {
            return AKM_ERR_INVALID_ARG;
        }

        if (data == NULL) {
            return AKM_ERR_INVALID_ARG;
        }
#endif

if ((mem->m_device == AK8963) ||
    (mem->m_device == AK09912) ||
    (mem->m_device == AK09913) ||
    (mem->m_device == AK09915) ||
    (mem->m_device == AK09916C) ||
    (mem->m_device == AK09916D) ||
    (mem->m_device == AK09918)) {
    akm_sensitivity = (1.f / 9830.f);
} else if (mem->m_device == AK09911) {
    akm_sensitivity = (1.f / 39322.f);
} else {
    AKMERROR;
    return AKM_ERR_NOT_SUPPORT;
}
for (i = 0; i < 3; i++) {
    tmp_f = (float32_t)(data->u.v[i]) * akm_sensitivity;
    tmp_i32 = (int32_t)tmp_f;

    /* Limit to 16-bit value */
    if ((tmp_i32 < INT16_MIN) || (INT16_MAX < tmp_i32)) {
        return AKM_ERR_INVALID_ARG;
    }

    mem->v_hvec_fusion.v[i] = (int16_t)tmp_i32;
}

    /* avec is successfully updated */
    mem->m_ts_hvec_fusion= data->time_ns;

    return AKM_SUCCESS;
}
static int16_t akl_setv_fusion_gry(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data)
{
    return AKM_SUCCESS;
}

/**************************************/
static int16_t akl_setv_gyr(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA *data)
{
    int32_t   tmp_i32;
    float32_t tmp_f;
    int16vec  tmp_v, tmp_o;
    int64_t   tmp_dt;
    int16_t   gdt_ag;
    int16_t   ret;
    int16_t   i;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    /* buffering */
    for (i = AKM_DEFAULT_GOAVE - 1; i >= 1; i--) {
        mem->va_gdata[i] = mem->va_gdata[i - 1];
    }

    /* from dps Q16 fromat to AKSC format */
    for (i = 0; i < 3; i++) {
        tmp_f = GYR_Q16_TO_AKSC(data->u.v[i]);
        tmp_i32 = (int32_t)tmp_f;

        /* Limit to 16-bit value */
        if ((tmp_i32 < INT16_MIN) || (INT16_MAX < tmp_i32)) {
            return AKM_ERR_INVALID_ARG;
        }

        mem->va_gdata[0].v[i] = (int16_t)tmp_i32;
        tmp_v.v[i] = mem->va_gdata[0].v[i];
    }

    if (AKSC_GetGyroOffsetAuto(mem->va_gdata, AKM_DEFAULT_GOAVE, AKM_DEFAULT_GYRO_MAX, AKM_DEFAULT_GYRO_MAXDIFF, &tmp_o) == 1) {
        /* update offset */
        mem->v_go = tmp_o;
    } else {
        /* use old offset */
        tmp_o = mem->v_go;
    }

    mem->v_gvec.u.x = tmp_v.u.x - tmp_o.u.x;
    mem->v_gvec.u.y = tmp_v.u.y - tmp_o.u.y;
    mem->v_gvec.u.z = tmp_v.u.z - tmp_o.u.z;

    /* calculate gdt_ag */
    if (mem->m_ts_hvec != 0) {
        /* if result is 0, it means 'data is not updated' */
        /* But in the case of DOEaG, negative value means 'not updated' */
        tmp_dt = data->time_ns - mem->m_ts_hvec;
        /* nano second -> micro second */
        tmp_dt = tmp_dt / (int64_t)1000;

        /* Limit to 16-bit value */
        if (tmp_dt > (int64_t)MAXTIME_USEC_IN_Q4) {
            tmp_dt = MAXTIME_USEC_IN_Q4;
        }

        tmp_dt = (int64_t)TIME_USEC_TO_AKSC(tmp_dt);
        gdt_ag = (int16_t)tmp_dt;
    } else {
        gdt_ag = -1;
    }

    if (gdt_ag > 0) {
        ret = GetMagneticVectorOffset(
                mem,
                0,
                -1,
                gdt_ag);

        AKL_DEBUG_MEAS_FRET_CHK(mem, ret);

        /* Check the return value */
        if (ret != AKRET_PROC_SUCCEED) {
            return AKM_ERROR;
        }
    }

    /* gvec is successfully updated */
    mem->m_ts_gvec = data->time_ns;

    return AKM_SUCCESS;
}

#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
/*****************************************************************************/
static int16_t akl_getv_mag_doe(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[6],
    int32_t                   *status,
    int64_t                   *timestamp)
{
    int       i;
    float32_t tmp_f;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (status == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    if (mem->m_ts_hvec == 0) {
        return AKM_ERR_NOT_YET_CALCULATED;
    }

    /* Convert from SmartCompass to Q16 */
    for (i = 0; i < 3; i++) {
        tmp_f = MAG_AKSC_TO_Q16(mem->v_hvec_doe.v[i]);
        data[i] = (int32_t)tmp_f;
        tmp_f = MAG_AKSC_TO_Q16(
                (float32_t)mem->v_ho_doe.v[i] +
                (float32_t)mem->v_hbase_doe.v[i]);
        data[i + 3] = (int32_t)tmp_f;
    }

#ifdef AKMD_ENABLE_DOEPLUS

    if ((mem->m_hdst_doe == 3) && (mem->m_doep_lv_doe <= 2)) {
        *status = 2;
    } else if ((mem->m_hdst_doe == 2) && (mem->m_doep_lv_doe <= 1)) {
        *status = 1;
    } else {
        *status = (int32_t)mem->m_hdst_doe;
    }

#else
    *status = (int32_t)mem->m_hdst_doe;
#endif
    *timestamp = mem->m_ts_hvec_doe;

    return AKM_SUCCESS;
}
#endif
/*****************************************************************************/
static int16_t akl_getv_mag_doeag(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[6],
    int32_t                   *status,
    int64_t                   *timestamp)
{
    int16_t   i;
    float32_t tmp_f;

#ifdef AKL_ARGUMENT_CHECK
    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (status == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    if (mem->m_ts_hvec == 0) {
        return AKM_ERR_NOT_YET_CALCULATED;
    }

    /* Convert from SmartCompass to Q16 */
    for (i = 0; i < 3; i++) {
        tmp_f = MAG_AKSC_TO_Q16(mem->v_hvec.v[i]);
        data[i] = (int32_t)tmp_f;
        tmp_f = MAG_AKSC_TO_Q16(
                (float32_t)mem->v_ho.v[i] + (float32_t)mem->v_hbase.v[i]);
        data[i + 3] = (int32_t)tmp_f;
    }

    *status = akl_get_doelv(mem);
    *timestamp = mem->m_ts_hvec;

    return AKM_SUCCESS;
}

/**************************************/
static int16_t akl_getv_acc(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[3],
    int32_t                   *status,
    int64_t                   *timestamp)
{
    int16_t   i;
    float32_t tmp_f;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (status == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    if (mem->m_ts_avec == 0) {
        return AKM_ERR_NOT_YET_CALCULATED;
    }

    for (i = 0; i < 3; i++) {
        tmp_f = ACC_AKSC_TO_Q16(mem->v_avec.v[i]);
        data[i] = (int32_t)tmp_f;
    }

    *status = (int32_t)(3);
    *timestamp = mem->m_ts_avec;

    return AKM_SUCCESS;
}

/**************************************/
static int16_t akl_getv_gyr(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[6],
    int32_t                   *status,
    int64_t                   *timestamp)
{
    int16_t i;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (status == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    if (mem->m_ts_gvec == 0) {
        return AKM_ERR_NOT_YET_CALCULATED;
    }

    for (i = 0; i < 3; i++) {
        data[i] = GYR_AKSC_TO_Q16((int32_t)mem->v_gvec.v[i]);
        data[i + 3] = GYR_AKSC_TO_Q16((int32_t)mem->v_go.v[i]);
    }

    *status = (int32_t)(3);
    *timestamp = mem->m_ts_gvec;

    return AKM_SUCCESS;
}

/**************************************/
static int16_t akl_getv_gravity(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[3],
    int32_t                   *status,
    int64_t                   *timestamp)
{
#ifndef AKM_DISABLE_D9D
    int16_t   i;
    float32_t tmp_f;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (status == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    if (mem->m_ts_ev_d9d == 0) {
        return AKM_ERR_NOT_YET_CALCULATED;
    }

    for (i = 0; i < 3; i++) {
        tmp_f = ACC_AKSC_TO_Q16(mem->v_gravity.v[i]);
        data[i] = (int32_t)tmp_f;
    }

    *status = (int32_t)(3);
    *timestamp = mem->m_ts_ev_d9d;

    return AKM_SUCCESS;
#else
    AKMERROR;
    return AKM_ERR_NOT_SUPPORT;
#endif
}

/**************************************/
static int16_t akl_getv_lacc(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[3],
    int32_t                   *status,
    int64_t                   *timestamp)
{
#ifndef AKM_DISABLE_D9D
    int16_t   i;
    float32_t tmp_f;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (status == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    if (mem->m_ts_ev_d9d == 0) {
        return AKM_ERR_NOT_YET_CALCULATED;
    }

    for (i = 0; i < 3; i++) {
        tmp_f = ACC_AKSC_TO_Q16(mem->v_lacc.v[i]);
        data[i] = (int32_t)tmp_f;
    }

    *status = (int32_t)(3);
    *timestamp = mem->m_ts_ev_d9d;

    return AKM_SUCCESS;
#else
    AKMERROR;
    return AKM_ERR_NOT_SUPPORT;
#endif
}

/**************************************/
static int16_t akl_getv_ori(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[3],
    int32_t                   *status,
    int64_t                   *timestamp)
{
    if (mem->m_ts_ev_d9d == 0) {
        return AKM_ERR_NOT_YET_CALCULATED;
    }

    /* Convert unit */
    /* from Q6 to Q16 */
    data[0] = ((int32_t)mem->m_theta) << 10;
    data[1] = ((int32_t)mem->m_phi180) << 10;
    data[2] = ((int32_t)mem->m_eta90) << 10;

    *status = (int32_t)(3);
    *timestamp = mem->m_ts_ev_d9d;

    return AKM_SUCCESS;
}

/**************************************/
static int16_t akl_getv_quat(
    const struct AKL_SCL_PRMS *mem,
    int32_t                   data[4],
    int32_t                   *status,
    int64_t                   *timestamp)
{
    if (mem->m_ts_ev_d9d == 0) {
        return AKM_ERR_NOT_YET_CALCULATED;
    }

    /* from Q14 to Q16 */
    data[0] = ((int32_t)mem->s_quat.u.x) << 2;
    data[1] = ((int32_t)mem->s_quat.u.y) << 2;
    data[2] = ((int32_t)mem->s_quat.u.z) << 2;
    data[3] = ((int32_t)mem->s_quat.u.w) << 2;

    *status = akl_get_doelv(mem);
    *timestamp = mem->m_ts_ev_d9d;

    return AKM_SUCCESS;
}

/**************************************/
static int32_t akl_get_doelv(const struct AKL_SCL_PRMS *mem)
{
    int32_t lv;

#ifdef AKMD_ENABLE_DOEPLUS

    if ((mem->m_hdst == 3) && (mem->m_doep_lv <= 2)) {
        lv = 2;
    } else if ((mem->m_hdst == 2) && (mem->m_doep_lv <= 1)) {
        lv = 1;
    } else {
        lv = (int32_t)mem->m_hdst;
    }

#else
    lv = (int32_t)mem->m_hdst;
#endif

    return lv;
}

/**************************************/
#ifndef AKM_DISABLE_D9D
static int64_t akl_get_d9d_ts(const struct AKL_SCL_PRMS *mem)
{
    int64_t maxt, mt, at, gt;

    mt = mem->m_ts_hvec;
    at = mem->m_ts_avec;
    gt = mem->m_ts_gvec;

    maxt = mt;

    if (at > maxt) {
        maxt = at;
    }

    if (gt > maxt) {
        maxt = gt;
    }

    return maxt;
}
#else
/**************************************/
static int64_t akl_get_d6d_ts(const struct AKL_SCL_PRMS *mem)
{
    int64_t maxt, mt, at;

    mt = mem->m_ts_hvec_fusion;
    at = mem->m_ts_avec_fusion;

    maxt = mt;

    if (at > maxt) {
        maxt = at;
    }

    return maxt;
}
#endif

/******************************************************************************/
/***** AKM public APIs ********************************************************

 Memory map
  |-----------------------------------|
  | parameters for AKL/SCL            |
  |-----------------------------------|
  | NV data (AKL_NV_PRMS) x Formation |
  |-----------------------------------|
  | Direction9D (D9D_Handle)          |
  |-----------------------------------|
  | DOEaG (AKSC_DOEAGVAR)             |
  |-----------------------------------|
  | DOEPlus for DOEaG (AKSC_DOEPVAR)  |
  |-----------------------------------|
  | DOEPlus for DOE (AKSC_DOEPVAR)    |
  |-----------------------------------|

 ******************************************************************************/
/***** Function manual is described in header file. ***************************/
uint32_t AKL_GetParameterSize(const uint8_t max_form)
{
    uint8_t  num;
    uint32_t size;
    uint32_t tmp_size;

    if (max_form == 0U) {
        num = (uint8_t)AKL_DEFAULT_FORM_NUM;
    } else {
        num = max_form;
    }

    size = byte_allign(sizeof(struct AKL_SCL_PRMS));
    size += AKL_GetNVdataSize(num);
    tmp_size = (uint32_t)AKSC_GetSizeOfD9DObjectIn32BitWord() * sizeof(int32);
    size += tmp_size;
    tmp_size = (uint32_t)AKSC_GetSizeDOEaGVar() * sizeof(int32);
    size += tmp_size;
#ifdef AKMD_ENABLE_DOEPLUS
    tmp_size = (uint32_t)AKSC_GetSizeDOEPVar() * sizeof(int32);
    size += tmp_size;
#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
    tmp_size = (uint32_t)AKSC_GetSizeDOEPVar() * sizeof(int32);
    size += tmp_size;
#endif
#endif

    return size;
}

/*****************************************************************************/
uint32_t AKL_GetNVdataSize(const uint8_t max_form)
{
    uint8_t num;

    if (max_form == 0U) {
        num = (uint8_t)AKL_DEFAULT_FORM_NUM;
    } else {
        num = max_form;
    }

    return byte_allign(sizeof(struct AKL_NV_PRMS)) * num;
}

/*****************************************************************************/
int16_t AKL_Init(
    struct AKL_SCL_PRMS                 *mem,
    const struct AKL_CERTIFICATION_INFO *cert,
    const uint8_t                       max_form,
    AKL_DEVICE_TYPE                     device)
{
#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (cert == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    /* set AKM device type */
    if (((AKM_DEVICE_TYPE)device == AK8963) ||
        ((AKM_DEVICE_TYPE)device == AK09911) ||
        ((AKM_DEVICE_TYPE)device == AK09912) ||
        ((AKM_DEVICE_TYPE)device == AK09913) ||
        ((AKM_DEVICE_TYPE)device == AK09915) ||
        ((AKM_DEVICE_TYPE)device == AK09916C) ||
        ((AKM_DEVICE_TYPE)device == AK09916D) ||
        ((AKM_DEVICE_TYPE)device == AK09918)) {
        mem->m_device = (AKM_DEVICE_TYPE)device;
    } else {
        return AKM_ERR_NOT_SUPPORT;
    }

    /* Initialize parameter */
    InitAKSCPRMS(mem);

    /* save number of maximum form. */
    if (max_form == 0U) {
        mem->m_maxForm = (uint8_t)AKL_DEFAULT_FORM_NUM;
    } else {
        mem->m_maxForm = max_form;
    }

    if (set_cert(mem, cert) != (uint16_t)AKM_SUCCESS) {
        AKMERROR;
        return AKM_ERR_NOT_SUPPORT;
    }

    /* Set NV data pointer */
    mem->ps_nv = (struct AKL_NV_PRMS *)(
            (long)mem
            + byte_allign(sizeof(struct AKL_SCL_PRMS)));

    mem->p_hd9d = (D9D_Handle)(
            (long)(mem->ps_nv)
            + AKL_GetNVdataSize(mem->m_maxForm));

    /* Set DOEaG data pointer */
    mem->ps_doeag_var = (AKSC_DOEAGVAR *)(
            (long)(mem->p_hd9d)
            + (long)(AKSC_GetSizeOfD9DObjectIn32BitWord() * sizeof(int32)));

#ifdef AKMD_ENABLE_DOEPLUS
    /* Set DOEPlus data pointer */
    mem->ps_doep_var = (AKSC_DOEPVAR *)(
            (long)(mem->ps_doeag_var)
            + (long)(AKSC_GetSizeDOEaGVar() * sizeof(int32)));
    #ifdef AKM_ENABLE_BOTH_DOE_DOEAG
    /* Set DOEPlus data pointer */
    mem->ps_doep_var_doe = (AKSC_DOEPVAR *)(
            (long)(mem->ps_doep_var)
            + (long)(AKSC_GetSizeDOEPVar() * sizeof(int32)));
    #endif
#endif
    mem->init = (uint32_t)AKL_INI_MAGIC_NUMBER;

    return AKM_SUCCESS;
}

/*****************************************************************************/
int16_t AKL_StartMeasurement(
    struct AKL_SCL_PRMS *mem,
    uint8_t             *nv_data)
{
    struct AKL_NV_PRMS *p_nv;
    struct AKL_NV_PRMS *p_pr;
    uint8_t            i;

#ifdef AKL_ARGUMENT_CHECK

    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    /* Check initialized */
    if (mem->init != (uint32_t)AKL_INI_MAGIC_NUMBER) {
        return AKM_ERR_INVALID_ARG;
    }

    p_nv = (struct AKL_NV_PRMS *)nv_data;
    p_pr = mem->ps_nv;

    if (p_nv == NULL) {
        /* If parameters couldn't be got, set default value. */
        SetDefaultNV(p_pr, mem->m_maxForm);
    } else {
        for (i = 0U; i < mem->m_maxForm; i++) {
            if (p_nv[i].magic == (uint32_t)AKL_NV_MAGIC_NUMBER) {
                /* Copy NV data to mem struct. */
                p_pr[i] = p_nv[i];
            } else {
                SetDefaultNV(&p_pr[i], 1U);
            }
        }
    }

    /* Init SmartCompass library functions. */
    if (InitMeasure(mem) != AKRET_PROC_SUCCEED) {
        return AKM_ERROR;
    }

#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
    if (InitMeasure_doe(mem) != AKRET_PROC_SUCCEED) {
        return AKM_ERROR;
    }
#endif

    return AKM_SUCCESS;
}

/*****************************************************************************/
int16_t AKL_StopMeasurement(
    const struct AKL_SCL_PRMS *mem,
    uint8_t                   *nv_data)
{
    struct AKL_NV_PRMS *p_nv;
    struct AKL_NV_PRMS *p_pr;
    uint8_t            i;

#ifdef AKL_ARGUMENT_CHECK
    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    /* Check initialized */
    if (mem->init != (uint32_t)AKL_INI_MAGIC_NUMBER) {
        return AKM_ERR_INVALID_ARG;
    }

    p_nv = (struct AKL_NV_PRMS *)nv_data;
    p_pr = mem->ps_nv;

    if (p_nv != NULL) {
        for (i = 0U; i < mem->m_maxForm; i++) {
            /* Copy mem data to NV buffer. */
            p_nv[i] = p_pr[i];
            p_nv[i].magic = (uint32_t)AKL_NV_MAGIC_NUMBER;
        }
    }

    return AKM_SUCCESS;
}

/*****************************************************************************/
AKL_DEVICE_TYPE AKL_GetDeviceType(int device_id)
{
   AKL_DEVICE_TYPE result = DEV_UNKNOW;
   switch(device_id)
   {
       case 0x05:
           result = DEV_AK09911;
           break;
       case 0x04:
           result = DEV_AK09912;
           break;
       case 0x08:
           result = DEV_AK09913;
           break;
       case 0x10:
           result = DEV_AK09915;
           break;
       case 0x09:
           result = DEV_AK09916C;
           break;
       case 0x0b:
           result = DEV_AK09916D;
           break;
       case 0x0c:
           result = DEV_AK09918;
           break;
       default:
           result = DEV_UNKNOW;
   }
   return result;
}

int16_t AKL_SetVector(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA data[],
    const uint8_t                num)
{
    uint8_t i;
    int16_t ret;
#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
    int16_t ret_doe;
    ret_doe = 0;
#endif

#ifdef AKL_ARGUMENT_CHECK
    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    /* Check initialized */
    if (mem->init != (uint32_t)AKL_INI_MAGIC_NUMBER) {
        return AKM_ERR_INVALID_ARG;
    }

    for (i = 0U; i < num; i++) {
        switch (data[i].stype) {
        case AKM_ST_MAG:
            if (mem->m_ts_hvec == data[i].time_ns) {
                return AKM_SUCCESS;
            }
            mem->m_device = AKL_GetDeviceType(akm_wia[1]);

            ret = akl_setv_mag_doeag(mem, &data[i]);
#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
            ret_doe = akl_setv_mag_doe(mem, &data[i]);
#endif
            break;

        case AKM_ST_ACC:
            if (mem->m_ts_avec == data[i].time_ns) {
                return AKM_SUCCESS;
            }

            ret = akl_setv_acc(mem, &data[i]);
            break;

        case AKM_ST_GYR:
            if (mem->m_ts_gvec == data[i].time_ns) {
                return AKM_SUCCESS;
            }

            ret = akl_setv_gyr(mem, &data[i]);
            break;

        case AKM_ST_FUSION_ACC:
            if (mem->m_ts_gvec == data[i].time_ns) {
                return AKM_SUCCESS;
            }

            ret = akl_setv_fusion_acc(mem, &data[i]);
            break;

        case AKM_ST_FUSION_MAG:
            if (mem->m_ts_gvec == data[i].time_ns) {
                return AKM_SUCCESS;
            }

            ret = akl_setv_fusion_mag(mem, &data[i]);
            break;

        case AKM_ST_FUSION_GYR:
            if (mem->m_ts_gvec == data[i].time_ns) {
                return AKM_SUCCESS;
            }

            ret = akl_setv_fusion_gry(mem, &data[i]);
            break;
        default:
            AKMERROR;
            ret = AKM_ERR_NOT_SUPPORT;
            break;
        }

        if (ret != AKM_SUCCESS) {
            return ret;
        }

#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
        if (ret_doe != AKM_SUCCESS) {
            return ret_doe;
        }
#endif
    }

    return AKM_SUCCESS;
}

/*****************************************************************************/
int16_t AKL_CalcFusion(struct AKL_SCL_PRMS *mem)
{
#ifndef AKM_DISABLE_D9D
    int16_t hdt, adt, gdt;
    int64_t tmp_dt;

#ifdef AKL_ARGUMENT_CHECK
    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    /* Check initialized */
    if (mem->init != (uint32_t)AKL_INI_MAGIC_NUMBER) {
        return AKM_ERR_INVALID_ARG;
    }

    /* calculate dt */
    /* if result is 0, it means 'data is not updated' */
    /* But in the case of D9D, negative value means 'not updated' */
    tmp_dt = mem->m_ts_hvec_fusion - mem->m_ts_ev_9d_hvec;
    /* nano second -> micro second */
    tmp_dt = tmp_dt / 1000;

    /* Limit to 16-bit value */
    if (tmp_dt > MAXTIME_USEC_IN_Q4) {
        tmp_dt = MAXTIME_USEC_IN_Q4;
    }

    tmp_dt = TIME_USEC_TO_AKSC(tmp_dt);
    hdt = (int16_t)tmp_dt;

    if (hdt == 0) {
        hdt = -1;
    }

    tmp_dt = mem->m_ts_avec_fusion- mem->m_ts_ev_9d_avec;
    tmp_dt = tmp_dt / 1000;

    if (tmp_dt > MAXTIME_USEC_IN_Q4) {
        tmp_dt = MAXTIME_USEC_IN_Q4;
    }

    tmp_dt = TIME_USEC_TO_AKSC(tmp_dt);
    adt = (int16_t)tmp_dt;

    if (adt == 0) {
        adt = -1;
    }

    tmp_dt = mem->m_ts_gvec_fusion- mem->m_ts_ev_9d_gvec;
    tmp_dt = tmp_dt / 1000;

    if (tmp_dt > MAXTIME_USEC_IN_Q4) {
        tmp_dt = MAXTIME_USEC_IN_Q4;
    }

    tmp_dt = TIME_USEC_TO_AKSC(tmp_dt);
    gdt = (int16_t)tmp_dt;

    if (gdt == 0) {
        gdt = -1;
    }

    /* event timestamp should be always updated whether
     * the D9D function success or not. */
    mem->m_ts_ev_9d_hvec = mem->m_ts_hvec_fusion;
    mem->m_ts_ev_9d_avec = mem->m_ts_avec_fusion;
    mem->m_ts_ev_9d_gvec = mem->m_ts_gvec_fusion;

#ifdef AKM_ENABLE_D9D
    if (CalcDirection9D(mem, hdt, adt, gdt) != AKRET_PROC_SUCCEED) {
        return AKM_ERROR;
    }
#endif

    mem->m_ts_ev_d9d = akl_get_d9d_ts(mem);

    return AKM_SUCCESS;
#else
#ifdef AKL_ARGUMENT_CHECK
    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    /* Check initialized */
    if (mem->init != (uint32_t)AKL_INI_MAGIC_NUMBER) {
        return AKM_ERR_INVALID_ARG;
    }

    if (CalcDirectionS3(mem) != AKRET_PROC_SUCCEED) {
        return AKM_ERROR;
    }

    mem->m_ts_ev_d9d = akl_get_d6d_ts(mem);

    return AKM_SUCCESS;
#endif
}

/*****************************************************************************/
int16_t AKL_GetVector(
    const AKM_VECTOR_TYPE     vtype,
    const struct AKL_SCL_PRMS *mem,
    int32_t                   *data,
    uint8_t                   size,
    int32_t                   *status,
    int64_t                   *timestamp)
{
    /* Check initialized */
    if (mem->init != (uint32_t)AKL_INI_MAGIC_NUMBER) {
        return AKM_ERR_INVALID_ARG;
    }

    switch (vtype) {
    case AKM_VT_MAG_DOEAG:

        if (AKM_VT_MAG_SIZE > size) {
            return AKM_ERR_INVALID_ARG;
        }

        return akl_getv_mag_doeag(mem, data, status, timestamp);

#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
    case AKM_VT_MAG:

        if (AKM_VT_MAG_SIZE > size) {
            return AKM_ERR_INVALID_ARG;
        }
        return akl_getv_mag_doe(mem, data, status, timestamp);
#endif

    case AKM_VT_ACC:

        if (AKM_VT_ACC_SIZE > size) {
            return AKM_ERR_INVALID_ARG;
        }

        return akl_getv_acc(mem, data, status, timestamp);

    case AKM_VT_GYR:

        if (AKM_VT_GYR_SIZE > size) {
            return AKM_ERR_INVALID_ARG;
        }

        return akl_getv_gyr(mem, data, status, timestamp);

    case AKM_VT_ORI:

        if (AKM_VT_ORI_SIZE > size) {
            return AKM_ERR_INVALID_ARG;
        }

        return akl_getv_ori(mem, data, status, timestamp);

    case AKM_VT_GRAVITY:

        if (AKM_VT_GRAVITY_SIZE > size) {
            return AKM_ERR_INVALID_ARG;
        }

        return akl_getv_gravity(mem, data, status, timestamp);

    case AKM_VT_LACC:

        if (AKM_VT_LACC_SIZE > size) {
            return AKM_ERR_INVALID_ARG;
        }

        return akl_getv_lacc(mem, data, status, timestamp);

    case AKM_VT_QUAT:

        if (AKM_VT_QUAT_SIZE > size) {
            return AKM_ERR_INVALID_ARG;
        }

        return akl_getv_quat(mem, data, status, timestamp);

    default:
        ALOGE("akm get vector vtype:%x\n",vtype);
        AKMERROR;
        return AKM_ERR_NOT_SUPPORT;
    }
}

/*****************************************************************************/
void AKL_GetLibraryInfo(struct AKL_LIBRARY_INFO *info)
{
    info->algocode = AKSC_GetVersion_AlgorithmCode();
    info->major = AKSC_GetVersion_Major();
    info->minor = AKSC_GetVersion_Minor();
    info->variation = AKSC_GetVersion_Variation();
    info->revision = AKSC_GetVersion_Revision();
    info->datecode = AKSC_GetVersion_DateCode();
}

/*****************************************************************************/
void AKL_ForceReCalibration(struct AKL_SCL_PRMS *mem)
{
    /* Conversion matrix from Android to SmartCompass coordination */
    const I16MATRIX hlayout = {{
                                   0, 1, 0,
                                   -1, 0, 0,
                                   0, 0, 1
                               }};

    /* Check initialized */
    if (mem->init == (uint32_t)AKL_INI_MAGIC_NUMBER) {
        AKSC_SetHDOEaGLevel(
            mem->ps_doeag_var,
            &hlayout,
            &mem->v_ho,
            AKSC_HDST_UNSOLVED,
            1
        );
        mem->m_hdst = AKSC_HDST_UNSOLVED;
#ifdef AKM_ENABLE_BOTH_DOE_DOEAG
        AKSC_SetHDOELevel(
            &mem->s_hdoev_doe,
            &mem->v_ho_doe,
            AKSC_HDST_UNSOLVED,
            1
        );
        mem->m_hdst_doe = AKSC_HDST_UNSOLVED;
#endif
    }
}

/*****************************************************************************/
int16_t AKL_ChangeFormation(
    struct AKL_SCL_PRMS *mem,
    const uint8_t       formNumber)
{
    /* Check initialized */
    if (mem->init != (uint32_t)AKL_INI_MAGIC_NUMBER) {
        return AKM_ERR_INVALID_ARG;
    }

    if (mem->m_maxForm <= formNumber) {
        return AKM_ERR_INVALID_ARG;
    }

    /* Set to struct */
    mem->m_curForm = formNumber;

    return AKM_SUCCESS;
}

/*****************************************************************************/
int16_t AKL_SetPDC(
    const struct AKL_SCL_PRMS *mem,
    const uint8_t             pdc[AKL_PDC_SIZE],
    const uint8_t             formNumber)
{
#ifdef AKM_ENABLE_PDC
    uint8   *p_pdc;
    uint8_t i;

#ifdef AKL_ARGUMENT_CHECK
    if (mem == NULL) {
        return AKM_ERR_INVALID_ARG;
    }

    if (pdc == NULL) {
        return AKM_ERR_INVALID_ARG;
    }
#endif

    /* Check initialized */
    if (mem->init != (uint32_t)AKL_INI_MAGIC_NUMBER) {
        return AKM_ERR_INVALID_ARG;
    }

    if (mem->m_maxForm <= formNumber) {
        return AKM_ERR_INVALID_ARG;
    }

    p_pdc = mem->ps_nv[formNumber].a_pdc;

    for (i = 0U; i < (uint8_t)AKL_PDC_SIZE; i++) {
        p_pdc[i] = pdc[i];
    }

    return AKM_SUCCESS;
#else
    AKMERROR;
    return AKM_ERR_NOT_SUPPORT;
#endif
}
