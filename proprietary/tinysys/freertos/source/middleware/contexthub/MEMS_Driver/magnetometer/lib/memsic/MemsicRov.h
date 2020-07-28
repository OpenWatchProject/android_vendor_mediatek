/*****************************************************************************
* Copyright (C), 2016, MEMSIC Inc.
* File Name		: MemsicOri.h
* Author		: MEMSIC AE Algoritm Team
* Description	: This file is the head file of algo lib. It provides the 
* 			      interface function declarations of the lib. 
* History		: 1. Data			: 2016/06/07
* 			  	  2. Author			: Yan Guopu
*			  	  3. Modification	:	
*****************************************************************************/
#ifndef _MEMSICROV_H_
#define _MEMSICROV_H_

#include <stdio.h>
#include <math.h>
#include <string.h>

//add Miao
//#ifndef MEMSIC_ALGORITHM_DATA_TYPES
//#define MEMSIC_ALGORITHM_DATA_TYPES
//typedef   signed char	int8; 	// signed 8-bit number    (-128 to +127)
//typedef unsigned char	uint8; 	// unsigned 8-bit number  (+0 to +255)
//typedef   signed short  int16; 	// signed 16-bt number    (-32,768 to +32,767)
//typedef unsigned short  uint16; // unsigned 16-bit number (+0 to +65,535)
//typedef   signed int	int32; 	// signed 32-bit number   (-2,147,483,648 to +2,147,483,647)
//typedef unsigned int	uint32; // unsigned 32-bit number (+0 to +4,294,967,295)
//#endif

/*******************************************************************************************
* Function Name	: GetMemsicRotVec
* Description	: Pass the mag and acc data into lib, calculate and get rotation vetor output.
* Input			: mag[0-2] --- calibrated mag x, y and z data.
*				  acc[0-2] --- acc x, y and z data.
* Output		: dataOut[0-3] --- rotation vector;
* Return		: None.
********************************************************************************************/
void CalcMemsicRotVec(float* mag, float* acc);

/*******************************************************************************************
* Function Name	: GetMemsicGravityAcc
* Description	: Get the virtual gravity sensor output.
* Input			: None.
* Output		: DataOut[0-2] --- gravity vector, unit is g.
* Return		: None.
********************************************************************************************/
void GetMemsicGravityAcc(float *DataOut);

/*******************************************************************************************
* Function Name	: GetMemsicLinearAcc
* Description	: Get the virtual linear acceleration output.
* Input			: None.
* Output		: DataOut[0-2] --- linear acceleration vector, unit is g.
* Return		: None.
********************************************************************************************/
void GetMemsicLinearAcc(float *DataOut);

/*******************************************************************************************
* Function Name	: GetCalRov
* Description	: Get the MEMSIC calibrated rov vector.
* Input			: None
* Output		: co[0-3]  --- 1 wx wy wz
* Return		: None
*				 .
********************************************************************************************/
void GetCalRov(float *rov);

void GetGeomagneticRotVec(float *GeoMagrov);

void GameRotVec(float *Gamerov);
#endif // _MEMSICROV_H_
