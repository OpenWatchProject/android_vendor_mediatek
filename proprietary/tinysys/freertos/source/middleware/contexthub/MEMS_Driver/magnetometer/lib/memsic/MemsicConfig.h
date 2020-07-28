/*****************************************************************************
* Copyright (C), 2016, MEMSIC Inc.
* File Name		: Config.h
* Author		: Yan Guopu		Version: 1.0	Data: 2014/12/11
* Description	: This file is the head file of config.c. It provides the 
* 			      interface function declarations of the lib. 
* History		: 1. Data			: 
* 			  	  2. Author			: 
*			  	  3. Modification	:	
*****************************************************************************/
#ifndef _MEMSICCONFIG_H_
#define _MEMSICCONFIG_H_

#include <stdlib.h>

//add Miao
//#ifndef MEMSIC_ALGORITHM_DATA_TYPES
//#define MEMSIC_ALGORITHM_DATA_TYPES
//typedef   signed char  int8; 	// signed 8-bit number    (-128 to +127)
//typedef unsigned char  uint8; 	// unsigned 8-bit number  (+0 to +255)
//typedef   signed short int16; 	// signed 16-bt number    (-32768 to +32767)
//typedef unsigned short uint16; 	// unsigned 16-bit number (+0 to +65535)
//typedef   signed int   int32; 	// signed 32-bt number    (-2,147,483,648 to +2,147,483,647)
//typedef unsigned int   uint32; 	// unsigned 32-bit number (+0 to +4,294,967,295)
//#endif

/*******************************************************************************************
* Function Name	: SetAlgoPara
* Description	: Set the magnetic sensor parameter
* Input			: Algorithm parameter.
* Return		: 1 --- succeed.
*				 -1 --- fail.
********************************************************************************************/
//int SetMagSensorPara(float *pa,float *pb);
int SetMagSfMtrix(float *pa);

/*******************************************************************************************
* Function Name	: SaveMagSensorPara
* Description	: Save the magnetic sensor parameter
* Input			: Algorithm parameter.
* Return		: 1 --- succeed.
*				 -1 --- fail.
********************************************************************************************/
int SaveMagSensorPara(float *pa,float *pb);

/*******************************************************************************************
* Function Name	: SetOriSensorPara
* Description	: Set the orientation sensor parameter
* Input			: Algorithm parameter.
* Return		: 1 --- succeed.
*				 -1 --- fail.
********************************************************************************************/
int SetOriSensorPara(float *pa, float *pb, int *pc);

/* Enable this flag to enable uImage operation */

#endif // _MEMSICCONFIG_H_
