/*****************************************************************************
* Copyright (C), 2016, MEMSIC Inc.
* File Name		: MemsicConfig.c
* Author		: MEMSIC Inc.		
* Version		: 1.0	Data: 2015/06/18
* Description	: This file is the configuration file for MEMSIC algorithm. 
* History		: 1.Data		: 
* 			  	  2.Author		: 
*			      3.Modification: By Guopu on 2015/08/06	
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "MemsicConfig.h"

static float fMagSP[9] = {1,0,0,
                         0,1,0,
                         0,0,1};

/*******************************************************************************************
* Function Name	: SetMagSfMtrix//SetMagSensorPara
* Description	: Set the magnetic sensor softmatrix.
********************************************************************************************/
//int SetMagSensorPara(float *pa,float *pb)
int SetMagSfMtrix(float *pa)
{
	int i;	
	
	for(i=0;i<9;i++){
		pa[i] = fMagSP[i];
	}
	return 0;
}

/*******************************************************************************************
* Function Name	: SaveMagSensorPara
* Description	: Save the magnetic sensor parameter.
********************************************************************************************/
int SaveMagSensorPara(float *pa,float *pb)
{	
	return 1;
}

/*******************************************************************************************
* Function Name	: SetOriSensorPara
* Description	: Set the orientation sensor parameter
********************************************************************************************/
int SetOriSensorPara(float *pa, float *pb, int *pc)
{
	pa[0] = 18;		//FirstLevel	= can not be more than 15
	pa[1] = 25;		//SecondLevel	= can not be more than 35
	pa[2] = 35;		//ThirdLevel	= can not be more than 50
	pa[3] = 0.025;	//dCalPDistLOne	
	pa[4] = 0.08;	//dCalPDistLTwo	
	pa[5] = 0.85;   //dPointToCenterLow
	pa[6] = 1.15;	//dPointToCenterHigh
	pa[7] = 1.4;	//dMaxPiontsDistance

	pb[0] = 0.1;	//dSatAccVar 	= threshold for saturation judgment
	pb[1] = 0.00002;//dSatMagVar 	= threshold for saturation judgment
	pb[2] = 0.0002;	//dMovAccVar 	= threshold for move judgment
	pb[3] = 0.0001;	//dMovMagVar 	= threshold for move judgment

	pc[0] = 20;		//delay_ms 		= sampling interval	
	pc[1] = 15;		//yawFirstLen	= can not be more than 31, must be odd number.
	pc[2] = 10;		//yawFirstLen	= can not be more than 30
	pc[3] = 2;		//corMagLen		= can not be more than 20
	
	
	return 1;
}

