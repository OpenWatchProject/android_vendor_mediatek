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
#ifndef INCLUDE_MEASURE_H
#define INCLUDE_MEASURE_H

/*! Include files for SmartCompass Library. */
#include "akl_smart_compass.h"

/*! The process has been successfully done. */
#define AKRET_PROC_SUCCEED         0x00
/*! The formation is changed. */
#define AKRET_FORMATION_CHANGED    0x01
/*! Data read error occurred. */
#define AKRET_DATA_READERROR       0x02
/*! Data overflow occurred. */
#define AKRET_DATA_OVERFLOW        0x04
/*! Offset values overflow. */
#define AKRET_OFFSET_OVERFLOW      0x08
/*! hbase was changed. */
#define AKRET_HBASE_CHANGED        0x10
/*! A fluctuation of magnetic field occurred. */
#define AKRET_HFLUC_OCCURRED       0x20
/*! AKSC_VNorm error. */
#define AKRET_VNORM_ERROR          0x40
/*! The process failes. */
#define AKRET_PROC_FAIL            0x80
/*! Certification error. */
#define AKRET_CERTIFICATION_ERROR  0x100
/*! The number of average for magnetic vector. */
#define AKM_DEFAULT_HNAVE          4
/*! The number of average for gyroscope offset. */
#define AKM_DEFAULT_GOAVE          32
#define AKM_DEFAULT_GO_MAX         80   //Q4 format
#define AKM_DEFAULT_GO_DIFF        32   //Q4 format


/*! DOE suspend time in nano second. */
#define AKM_INTERVAL_SUSPEND       1000000000
/*! DOE execute interval in nano second.
 * 100 msec is recommended. */
//#define AKM_INTERVAL_DOE           100000000
#define AKM_INTERVAL_DOE           10000000


/*! AKL debug function. */
#ifdef AKL_DEBUG
#define AKL_DEBUG_SCL_FRET_CHK(mem, fret) \
    do{ \
        mem->m_dbg_scl_fret = (int16_t)fret; \
    } while (0)
#define AKL_DEBUG_MEAS_FRET_CHK(mem, fret) \
    do{ \
        mem->m_dbg_meas_fret = (int16_t)fret; \
    } while (0)
#else
#define AKL_DEBUG_SCL_FRET_CHK(mem, fret)
#define AKL_DEBUG_MEAS_FRET_CHK(mem, fret)
#endif

/*!
 * Initialize #AKL_SCL_PRMS structure.
 * Some of initial values can be customized by editing the file "CustomerSpec.h".
 * \param[out] prms A pointer to #AKL_SCL_PRMS structure.
 */
void InitAKSCPRMS(
    struct AKL_SCL_PRMS *prms
);

/*!
 * Fill #AKL_NV_PRMS structure with default value. This function will
 * be called when parameters couldn't be get from non-volatile area.
 * \param[out] nv A pointer to #AKL_NV_PRMS structure.
 * \param[in] num_formation The number of formation.
 */
void SetDefaultNV(
    struct AKL_NV_PRMS *nv,
    const uint8        num_formation
);

/*!
 * Initialize SmartCompass library for starting measurement.
 * \retval #AKM_SUCCESS When operation is succeeded.
 * \param[in,out] prms A pointer to a #AKL_SCL_PRMS structure.
 */
int16 InitMeasure(
    struct AKL_SCL_PRMS *prms
);

/*!
 * This function will be processed when new magnetic data is obtained from
 * e-compass device, then calculate offset.
 * \retval #AKM_SUCCESS When operation is succeeded.
 * \retval negative-value When operation failed.
 * \param[in] bData An array of register values which holds,
 * ST1, HXL, HXH, HYL, HYH, HZL, HZH (TMPS) and ST2 value respectively.
 * \param[in,out] prms A pointer to a #AKL_SCL_PRMS structure.
 * \param[in] ts_mag A time stamp of bData in nanosecond.
 */
int16 GetMagneticVector(
    const int16         bData[],
    struct AKL_SCL_PRMS *prms,
    const int64_t       ts_mag
);

/*!
 * Call AKSC_Direction9D to calculate Yaw, Pitch, Roll angle.
 */
int16 CalcDirection9D(
    struct AKL_SCL_PRMS *prms,
    int16_t             hdt,
    int16_t             adt,
    int16_t             gdt
);

/*!
 * Call AKSC_DirectionS3 to calculate Yaw, Pitch, Roll angle.
 */
int16 CalcDirectionS3(
    struct AKL_SCL_PRMS *prms
);

#endif /* INCLUDE_MEASURE_H */
