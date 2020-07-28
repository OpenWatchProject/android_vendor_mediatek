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
#ifndef INCLUDE_AKM_CUSTOMERSPEC_H
#define INCLUDE_AKM_CUSTOMERSPEC_H


/*! \defgroup CUSTOMER_SPEC Customer specific parameters.
 * SmartCompass library parameters. <b> Please change these parameters
 * according to the directions from AKM.</b>
 @{*/
/*! A string of licenser. Don't change this string. */
#define AKM_CUSTOM_LICENSER  "ASAHIKASEI"
/*! A string of licensee. This string should be changed. */
#define AKM_CUSTOM_LICENSEE "MTK_X_SHUB"



/*! The number of formation. */
#define AKM_CUSTOM_NUM_FORM  1

/*! Measurement frequency */
#define AKM_CUSTOM_MAG_FREQ  10
#define AKM_CUSTOM_ACC_FREQ  10
#define AKM_CUSTOM_GYR_FREQ  100

/* define akm device type */
#define AKM_DEVICE_AK09911      0x05
#define AKM_DEVICE_AK09912      0x04
#define AKM_DEVICE_AK09913      0x08
#define AKM_DEVICE_AK09915      0x10
#define AKM_DEVICE_AK09916C    0x09
#define AKM_DEVICE_AK09916D    0x0b
#define AKM_DEVICE_AK09918      0x0c

#define AKM_DEVICE_ID AKM_DEVICE_AK09915


/*! \defgroup CSPEC_AXIS The axis conversion
 * Axis conversion parameters.
 @{*/
#define AKM_CUSTOM_MAG_AXIS_ORDER_X  0
#define AKM_CUSTOM_MAG_AXIS_ORDER_Y  1
#define AKM_CUSTOM_MAG_AXIS_ORDER_Z  2
#define AKM_CUSTOM_MAG_AXIS_SIGN_X   0
#define AKM_CUSTOM_MAG_AXIS_SIGN_Y   0
#define AKM_CUSTOM_MAG_AXIS_SIGN_Z   0

#define AKM_CUSTOM_ACC_AXIS_ORDER_X  0
#define AKM_CUSTOM_ACC_AXIS_ORDER_Y  1
#define AKM_CUSTOM_ACC_AXIS_ORDER_Z  2
#define AKM_CUSTOM_ACC_AXIS_SIGN_X   0
#define AKM_CUSTOM_ACC_AXIS_SIGN_Y   0
#define AKM_CUSTOM_ACC_AXIS_SIGN_Z   0

#define AKM_CUSTOM_GYR_AXIS_ORDER_X  0
#define AKM_CUSTOM_GYR_AXIS_ORDER_Y  1
#define AKM_CUSTOM_GYR_AXIS_ORDER_Z  2
#define AKM_CUSTOM_GYR_AXIS_SIGN_X   0
#define AKM_CUSTOM_GYR_AXIS_SIGN_Y   0
#define AKM_CUSTOM_GYR_AXIS_SIGN_Z   0
/*@}*/

/*! If you want to use continuous measurement mode, uncomment the below line */
/* Please define AKM_CUSTOM_CONTINUOUS_MEASURE */

/*@}*/
#endif /* INCLUDE_AKM_CUSTOMERSPEC_H */
