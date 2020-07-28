/*****************************************************************************
* Copyright (C), 2016, MEMSIC Inc.
* File Name     : MemsicOri.h
* Author        : MEMSIC AE Algoritm Team
* Description   : This file is the head file of MemsicAlgo.lib. It provides the
*                 interface function declarations of the lib.
* History       : 1. Data           : 2016/06/07
*                 2. Author     : Yan Guopu
*                 3. Modification   :
*****************************************************************************/
#ifndef _MEMSICALGORITHM_H_
#define _MEMSICALGORITHM_H_

#include "./MemsicConfig.h"

//Add Miao
//#ifndef MEMSIC_ALGORITHM_DATA_TYPES
//#define MEMSIC_ALGORITHM_DATA_TYPES
//typedef   signed char int8;   // signed 8-bit number    (-128 to +127)
//typedef unsigned char uint8;  // unsigned 8-bit number  (+0 to +255)
//typedef   signed short  int16;    // signed 16-bt number    (-32,768 to +32,767)
//typedef unsigned short  uint16; // unsigned 16-bit number (+0 to +65,535)
//typedef   signed int  int32;  // signed 32-bit number   (-2,147,483,648 to +2,147,483,647)
//typedef unsigned int  uint32; // unsigned 32-bit number (+0 to +4,294,967,295)
//#endif

/*******************************************************************************************
* Function Name : AlgoInitial
* Description   : Initialize the algorithm.
* Input         : None
* Output        : None.
* Return        : 1 --- succeed.
*                -1 --- fail.
********************************************************************************************/
int AlgoInitial(float *MagPara);

/*******************************************************************************************
* Function Name : MainAlgorithmProcess
* Description   : Pass the Acc and Mag raw data to the algorithm library.
* Input         : acc[0-2] --- Acceleration raw data of three axis, unit is g;
*                 mag[0-2] --- Magnetic raw data of three axis, unit is gauss;
*                 reg_mag[0-2] --- Mag reg raw data of three axis, unit is gauss;
* Output        : None.
* Return        : 1 --- succeed.
*                -1 --- fail.
********************************************************************************************/
int MainAlgorithmProcess(float *acc, float *mag);

/*******************************************************************************************
* Function Name : GetMagAccuracy
* Description   : Get the accuracy of the calibration algorithm.
* Input         : None
* Output        : None
* Return        : 0,1,2,3 --- Accuracy of the calibration.
********************************************************************************************/
int GetMagAccuracy(void);

/*******************************************************************************************
* Function Name : GetMagSaturation
* Description   : Get the magnetic sensor saturation status.
* Input         : None
* Output        : None
* Return        : 0 --- Indicate that magnetic sensor works normally.
*               : 1 --- Indicate that magnetic sensor is saturated, need to do SET action.
********************************************************************************************/
int GetMagSaturation(void);

/*******************************************************************************************
* Function Name : GetCalPara
* Description   : Get the calibrated parameters of magnetic sensor.
* Input         : None
* Output        : cp[0-2] --- magnetic field offset, unit is gauss.
*                 cp[3] --- strength of the magnetic field, unit is gauss.
*                 cp[4-5] --- horizontal and vertical component, unit is gauss.
*                 cp[6] --- geomagnetic angle of inclination, unit is degree.
* Return        : 0 --- no need to save the calibrated magnetometer parameter.
*                 1 --- need to save the new reliable calibrated magnetometer parameter.
********************************************************************************************/
int GetCalPara_6(float *cp);

/*******************************************************************************************
* Function Name : GetCalMag
* Description   : Get the MEMSIC calibrated magnetic output.
* Input         : None
* Output        : cm[0-2]  --- calibrated magnetic sensor data, unit is gauss.
* Return        : debug use
********************************************************************************************/
int GetCalMag(float *cm);

/*******************************************************************************************
* Function Name : GetCalOri
* Description   : Get the MEMSIC calibrated orientation vector.
* Input         : None
* Output        : co[0-2]  --- orientation data, azimuth, pitch, roll, unit is degree.
* Return        : 1 --- succeed.
*                -1 --- fail.
********************************************************************************************/
int GetCalOri(float *co);

/*******************************************************************************************
* Function Name : GetOffset
* Description   : Get the MEMSIC calibrated offset vector.
* Input         : None
* Output        : co[0-2]  --- calibrated offset, unit is gauss.
* Return        : 1 --- succeed.
*                -1 --- fail.
********************************************************************************************/
int GetOffset(float *cof);


#endif

