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
#ifndef INCLUDE_AKL_API_H
#define INCLUDE_AKL_API_H

#include "AKM_Common.h"

/* For debugging */
#if 1//defined(DEBUG) | defined(_DEBUG)
#ifndef AKL_ARGUMENT_CHECK
#define AKL_ARGUMENT_CHECK
#endif
#endif

/*
 * Copied definition from SmartCompass Library
 */
/*! The maximum length of licenser/licensee. */
#define AKL_CI_MAX_CHARSIZE  16
/*! The maximum length of key array. */
#define AKL_CI_MAX_KEYSIZE   16
/*! The size of PDC parameter. */
#define AKL_PDC_SIZE         27
/*! The type of AKM device. */
typedef enum _AKL_DEVICE_TYPE {
    DEV_UNKNOW = 0,
    DEV_AK8963 = 1,
    DEV_AK09911 = 2,
    DEV_AK09912 = 3,
    DEV_AK09913 = 4,
    DEV_AK09915 = 5,
    DEV_AK09916C = 6,
    DEV_AK09916D = 7,
    DEV_AK09918 = 8,
} AKL_DEVICE_TYPE;


/*!
 * Type of compass parameter. The calculation data (e.g. data buffer) is packed
 * in this struct.
 */
struct AKL_SCL_PRMS;

/*!
 * Definition of library version number struct.
 */
struct AKL_LIBRARY_INFO {
    /*! A algorithm code of library. */
    uint32_t algocode;
    /*! Major version number of library. */
    int16_t major;
    /*! Minor version number of library. */
    int16_t minor;
    /*! Revision number of library. */
    int16_t revision;
    /*! Date code of library. */
    int16_t datecode;
    /*! Custom number of library. */
    int16_t variation;
};

/*!
 * Definition of certification parameter.
 */
struct AKL_CERTIFICATION_INFO {
    /*! Licenser. The string should be end with '\0' */
    uint8_t a_licenser[AKL_CI_MAX_CHARSIZE + 1];
    /*! Licensee. The string should be end with '\0' */
    uint8_t a_licensee[AKL_CI_MAX_CHARSIZE + 1];
    /*! An array of key values. */
    int16_t a_key[AKL_CI_MAX_KEYSIZE];
};

//#define AKM_ENABLE_DOEEX
/******************************************************************************
 * API decleration.
 ******************************************************************************/
/*!
 * \return size of memory for #AKL_SCL_PRMS.
 * \param max_form A number of formation. This value should be the same value
 * for #AKL_GetParameterSize, #AKL_GetNVdataSize and #AKL_Init.
 * If 0 is specified, #AKM_CUSTOM_NUM_FORM is used.
 */
uint32_t AKL_GetParameterSize(
    const uint8_t max_form
);


/*!
 * \return size of non-volatile data area.
 * \param max_form A number of formation. This value should be the same value
 * for #AKL_GetParameterSize, #AKL_GetNVdataSize and #AKL_Init.
 * If 0 is specified, #AKM_CUSTOM_NUM_FORM is used.
 */
uint32_t AKL_GetNVdataSize(
    const uint8_t max_form
);


/*!
 * Prepare magnetic measurement.
 * A data buffer which is pointed by \c mem should be filled with 0 before this
 * function is called.
 * \return #AKM_SUCCESS Succeeded.
 * \param mem A pointer to data buffer which size should be equal to the return
 * value of #AKL_GetParameterSize.
 * \param cert A pointer to #AKL_CERTIFICATION_INFO. The structure should be
 * filled with valid information.
 * \param max_form A number of formation. This value should be the same value
 * for #AKL_GetParameterSize, #AKL_GetNVdataSize and #AKL_Init.
 * If 0 is specified, #AKM_CUSTOM_NUM_FORM is used.
 * \param device AKM device type.
 */
int16_t AKL_Init(
    struct AKL_SCL_PRMS                 *mem,
    const struct AKL_CERTIFICATION_INFO *cert,
    const uint8_t                       max_form,
    AKL_DEVICE_TYPE                     device
);


/*!
 * Start measurement process.
 * \return #AKM_SUCCESS Succeeded.
 * \param mem A pointer to data buffer which size should be equal to the return
 * value of #AKL_GetParameterSize.
 * \param nv_data A pointer to an data array which is obtained by
 * AKH_LoadParameter function.
 */
int16_t AKL_StartMeasurement(
    struct AKL_SCL_PRMS *mem,
    uint8_t             *nv_data
);


/*!
 * Stop measurement process.
 * \return #AKM_SUCCESS Succeeded.
 * \param mem A pointer to data buffer which size should be equal to the return
 * value of #AKL_GetParameterSize.
 * \param nv_data A pointer to an data array which will be saved by
 * AKH_SaveParameter function.
 */
int16_t AKL_StopMeasurement(
    const struct AKL_SCL_PRMS *mem,
    uint8_t                   *nv_data
);


/*!
 * Set measurement data to the library.
 * \return #AKM_SUCCESS Succeeded.
 * \param mem A pointer to data buffer which size should be equal to the return
 * value of #AKL_GetParameterSize.
 * \param data A pointer to #AKM_SENSOR_DATA struct (or array).
 * \param num A number of data to be input.
 */
int16_t AKL_SetVector(
    struct AKL_SCL_PRMS          *mem,
    const struct AKM_SENSOR_DATA data[],
    const uint8_t                num
);


/*!
 * Calculate.
 * \return #AKM_SUCCESS Succeeded.
 * \param mem A pointer to data buffer which size should be equal to the return
 */
int16_t AKL_CalcFusion(
    struct AKL_SCL_PRMS *mem
);

/*!
 * Get a vector data from library.
 * \return #AKM_SUCCESS Succeeded.
 * \param vtype Specify a vector type.
 * \param mem A pointer to data buffer which size should be equal to the return
 * value of #AKL_GetParameterSize.
 * \param data A pointer to vector. Unit and axis definition are the same as
 * the definition of #AKM_SENSOR_DATA.
 * \param size The size of data array. The size should be larger than
 * corresponding AKM_VT_xx_SIZE.
 * \param status Accuracy level (from 0 to 3).
 * \param timestamp time in nano second.
 */
int16_t AKL_GetVector(
    const AKM_VECTOR_TYPE     vtype,
    const struct AKL_SCL_PRMS *mem,
    int32_t                   *data,
    uint8_t                   size,
    int32_t                   *status,
    int64_t                   *timestamp
);

/*!
 * Get library version information.
 * \return #AKM_SUCCESS Succeeded.
 * \param info A pointer to #AKL_LIBRARY_INFO struct.
 */
void AKL_GetLibraryInfo(
    struct AKL_LIBRARY_INFO *info
);


/*!
 * Set calibration accuracy to the lowest. When this function is called,
 * offset will be re-calibrated.
 * \return #AKM_SUCCESS Succeeded.
 * \param mem A pointer to data buffer which size should be equal to the return
 * value of #AKL_GetParameterSize.
 */
void AKL_ForceReCalibration(
    struct AKL_SCL_PRMS *mem
);


/*!
 * Set form fuctor
 * \return #AKM_SUCCESS Succeeded.
 * \param mem A pointer to data buffer which size should be equal to the return
 * value of #AKL_GetParameterSize.
 * \param formNumber This value must not exceed the value of max_form specified
 * when #AKL_Init was called.
 */
int16_t AKL_ChangeFormation(
    struct AKL_SCL_PRMS *mem,
    const uint8_t       formNumber
);

/*!
 * Set PDC parameter.
 * \return #AKM_SUCCESS Succeeded.
 * \param mem A pointer to data buffer which size should be equal to the return
 * value of #AKL_GetParameterSize.
 * \param pdc A pointer to PDC parameter array which size should be
 * #AKL_PDC_SIZE.
 * \param formNumber This value must not exceed the value of max_form specified
 * when AKL_Init was called.
 */
int16_t AKL_SetPDC(
    const struct AKL_SCL_PRMS *mem,
    const uint8_t             pdc[AKL_PDC_SIZE],
    const uint8_t             formNumber
);

#endif /* INCLUDE_AKL_API_H */
