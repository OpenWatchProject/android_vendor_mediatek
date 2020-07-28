/*****************************************************************************
* Copyright (C), 2015, MEMSIC Inc.
* File Name     : MemsicGyr.h
* Author        : Dong Xiaoguang
* Version       : 2.1b, Date: 2016/06/02
* Description   : Soft gyro algorithm
*****************************************************************************/
#ifndef _MEMSIC_GYR_H_
#define _MEMSIC_GYR_H_

/*******************************************************************************************
* Function Name:    GetMemsicGyro
* Description:      Pass the mag and acc data into lib, calculate and get soft gyro output.
* Input:            mag[0-2] --- calibrated mag x, y and z data.
*                   acc[0-2] --- acc x, y and z data.
*                   ts --- sampling interval, unit is second.
*                   algoRestart --- 1 means algo is restarted, 0 not
* Output:           None
* Return:           0 means success, 1 failure
********************************************************************************************/
 void CalcMemsicGyro(float* mag, float* acc, float ts, int algoRestart);
 
/*******************************************************************************************
* Function Name	: GetCalGyr
* Description	: Get the MEMSIC calibrated Gyro vector.
* Input			: None
* Output		: co[0-2]  --- wx wy wz
* Return		: none.
*				 
********************************************************************************************/
void GetCalGyr(float *gyro);

#endif //_MEMSIC_GYR_H_
