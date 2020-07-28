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
#ifndef INCLUDE_AKM_COMMON_H
#define INCLUDE_AKM_COMMON_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include <atomic.h>
#include <atomicBitset.h>
#include <contexthub_core.h>
#include <contexthub_fw.h>
#include <FreeRTOSConfig.h>
#include <platform_mtk.h>
#include <sensors.h>
#include <sensType.h>

#define AKM_LIBRARY_FOOTPRINT   "20180119"


typedef float  float32_t;
typedef double float64_t;


/*#define AKM_MALLOC   malloc
#define AKM_FREE     free
#define AKM_MEMSET   memset
#define AKM_MEMCOPY  memcpy*/

/*! \defgroup AKM_RETURN_VALUE AKM library's error code definition.
 * The return value (or error code) of AKM library API.
 @{*/
/*! This value represents success of operation. */
#define AKM_SUCCESS                 (0)
/*! This value represents some error happend while operation execution. */
#define AKM_ERROR                   (-1)
/*! This value represents argument of fuction is out of range or invalid. */
#define AKM_ERR_INVALID_ARG         (-2)
/*! This value represents the operation is not supported on this platform. */
#define AKM_ERR_NOT_SUPPORT         (-3)
/*! This value represents some I/O error happend while operation execution. */
#define AKM_ERR_IO                  (-4)
/*! This value represents device is busy or other process blocks the operation
 * execution. */
#define AKM_ERR_BUSY                (-5)
/*! This value represents the operation could not be done within the specified
 * duration time. */
#define AKM_ERR_TIMEOUT             (-6)
/*! This value represents that the calculation is not yet done. */
#define AKM_ERR_NOT_YET_CALCULATED  (-7)


/*! \defgroup AKM_VECTOR_TYPE_SIZE Library output vector size definition.
 * The size of vector which is got from #AKL_GetVector API.
 @{*/
/*! data[0,1,2] = (X,Y,Z) data[3,4,5] = (bias_X,bias_Y,bias_Z). */
#define AKM_VT_MAG_SIZE      6U
/*! data[0,1,2] = (X,Y,Z). */
#define AKM_VT_ACC_SIZE      3U
/*! data[0,1,2] = (X,Y,Z) data[3,4,5] = (bias_X,bias_Y,bias_Z). */
#define AKM_VT_GYR_SIZE      6U
/*! data[0,1,2] = (yaw,pitch,roll). */
#define AKM_VT_ORI_SIZE      3U
/*! data[0,1,2] = (X,Y,Z). */
#define AKM_VT_GRAVITY_SIZE  3U
/*! data[0,1,2] = (X,Y,Z). */
#define AKM_VT_LACC_SIZE     3U
/*! data[0,1,2,3] = (x,y,z,w). */
#define AKM_VT_QUAT_SIZE     4U
/*@}*/

/*! Type of vector. When get this type of vector,
 * the buffer should be larger than the corresponding definition size. */
typedef enum {
    /*! Magnetometer.\n
     * A data is stored in the vector in order of x, y, z,
     * bias_x, bias_y, and bias_z.\n
     * The data format is micro tesla in Q16 format. */
    AKM_VT_MAG = 0x01,
    /*! Accelerometer.\n
     * A data is stored in the vector in order of x, y, and z.\n
     * The data format is SI unit (m/s^2) in Q16 format. */
    AKM_VT_ACC = 0x02,
    /*! Gyroscope.\n
     * A data is stored in the vector in order of x, y, z,
     * bias_x, bias_y, and bias_z.\n
     * The data format is degree/second in Q16 format. */
    AKM_VT_GYR = 0x04,
    /*! Orientation.\n
     * A data is stored in the vector in order of yaw, pitch, and roll.
     *   Yaw angle   (0 to 360)    degree in Q16.\n
     *   Pitch angle (-180 to 180) degree in Q16.\n
     *   Roll angle  (-90 to 90)   degree in Q16.*/
    AKM_VT_ORI = 0x08,
    /*! Gravity.
     * The data format is the same as #AKM_VT_ACC. */
    AKM_VT_GRAVITY = 0x10,
    /*! Linear accelerometer.
     * The data format is the same as #AKM_VT_ACC. */
    AKM_VT_LACC = 0x20,
    /*! Quaternion.\n
     * A data is stored in the vector in order of x, y, z, and w. */
    AKM_VT_QUAT = 0x40,

    AKM_VT_FUSION_ACC = 0x100,
    AKM_VT_FUSION_MAG = 0x200,
    AKM_VT_FUSION_GYR = 0x400,

    //GYR|ORI|GRAVITY|LACC|QUAT
    AKM_VT_OUTPUT9D = 0x7c,
} AKM_VECTOR_TYPE;

/*!
 * A structure which holds set of sensor data.
 * The sensor's axis direction definition is same as Android.\n
 * i.e.\n
 *  \li from center to right: positive X.
 *  \li from center to forward: positive Y.
 *  \li from center to top (toward to sky): positive Z.
 *
 * The data unit is a little bit different because this structure
 *  takes only integer value.
 *  \li \b MAGNETOMETER: \n
 *  micro Tesla in Q16 format. e.g. 1 uT = 65536 (Q16)
 *  \li \b ACCELEROMETER: \n
 *  SI unit (m/s^2) in Q16 format. e.g. 1G = 9.81 m/s^2 = 642908 (Q16)
 *  \li \b GYROSCOPE: \n
 *  degree/second in Q16 format. e.g. 1 dps = 65536 (Q16)
 *
 *
 */
struct AKM_SENSOR_DATA {
    union {
        struct {
            float32_t x;
            float32_t y;
            float32_t z;
        } s;
        float32_t v[3];
    } u;
    /*!
     * This field holds time stamp (in nano seconds unit) for the data.
     * Ideally, this value is calculated from system tick, but fixed interval
     * value is acceptable.\n
     * For example, when data is get every 100msec, if current \c time_ns was
     * 10000, then next \c time_ns will be 100010000. */
    int64_t time_stamp;
    /*!
     * This field is used to store device id value.
     * for multi library it is used for CRC check */
    uint8_t device_id;
    /*!
     * This field is used to store pat value.
     * for multi library it is used for CRC check */
    uint8_t pat;
    /*!
     * This field is used to store device status register value.
     * In case of AKM compass, \c ST1 and \c ST2 value is put to
     * \c status[0] and \c status[1] respectively.*/
    int16_t status[2];
    /*!
     * This field is used to store sensor type of this data.
     * Some APIs use this field, in order to determine from which sensor type
     * this data was acquired.*/
    AKM_VECTOR_TYPE stype;
};
#endif /* INCLUDE_AKM_COMMON_H */
