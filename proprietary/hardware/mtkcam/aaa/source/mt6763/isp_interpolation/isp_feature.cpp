/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "isp_feature"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <stdlib.h>
#include <cutils/log.h>
#include <math.h>
#include <aaa_types.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning_cam_info.h>
#include "isp_feature.h"

#define INTER_LOGD(fmt, arg...) ALOGD(fmt, ##arg)
#define INTER_LOGW(fmt, arg...) ALOGW(fmt, ##arg)
#define INTER_LOGE(fmt, arg...) ALOGE(fmt, ##arg)

#define INTER_LOG_IF(cond, ...)      do { if ( (cond) ) { INTER_LOGD(__VA_ARGS__); } }while(0)
#define INTER_LOGW_IF(cond, ...)      do { if ( (cond) ) { INTER_LOGW(__VA_ARGS__); } }while(0)
#define INTER_LOGE_IF(cond, ...)      do { if ( (cond) ) { INTER_LOGE(__VA_ARGS__); } }while(0)

#define PI 3.14159265358979323846


typedef struct
{
    MINT32 Race;
	MINT32 Y_Center;
	MINT32 S_Center;
	MINT32 H_Center;
} FaceInput_T;

typedef struct
{
    MINT32 Luma_Start;
	MINT32 Luma_End;
	MINT32 Luma_Offset;
	MINT32 Sat_Start;
	MINT32 Sat_End;
	MINT32 Sat_Offset;
	MINT32 Hue_Start;
	MINT32 Hue_End;
	MINT32 Hue_Offset;
} MemoryColorMappingInput_T;

typedef struct
{
    MINT32 L;
    MINT32 U;
	MINT32 POINT0;
	MINT32 POINT1;
	MINT32 POINT2;
	MINT32 POINT3;
	MINT32 POINT4;
	MINT32 SLOPE0;
	MINT32 SLOPE1;
	MINT32 SLOPE2;
	MINT32 SLOPE3;
	MINT32 SLOPE4;
	MINT32 SLOPE5;
	MINT32 WGT_LSLOPE;
	MINT32 WGT_USLOPE;
} MemoryColorCurve_T;



typedef struct
{
    MemoryColorCurve_T Y;
	MemoryColorCurve_T S;
	MemoryColorCurve_T H;
} MemoryColorMappingOutput_T;

typedef struct
{
    MINT32 Y[8][8][8];
	MINT32 U[8][8][8];
	MINT32 V[8][8][8];
} ANRTbl8x8x8_T;

typedef struct
{
    float Y_Mean;
	float Y_Gain;
	float Y_Range;
	float U_Mean;
	float U_Gain;
	float U_Range;
	float V_Mean;
	float V_Gain;
	float V_Range;
} ANRTblTuning_T;



//  "return value": Interpolated register value
//  "ISO":          ISO value of current image
//  "ISOpre":       A pre-defined ISO (100, 200, 400, ...) who is the cloeset but smaller than or equal to "ISO"
//  "ISOpos":       A pre-defined ISO (100, 200, 400, ...) who is the cloeset but larger than or equal to "ISO"
//  "PARAMpre":     Corresponding register value from "ISOpre"
//  "PARAMpos":     Corresponding register value from "ISOpos"
//  "Method:        0 for linear interpolation, 1 for closest one

MINT32 Clip(MINT32 input, MINT32 upper, MINT32 lower)
{
    MINT32 output = 0;
    MINT32 max = 0, min = 0;

    if (upper < lower)
    {
        max = lower;
        min = upper;
    }
    else
    {
        max = upper;
        min = lower;
    }

    if (input > max)
        output = max;
    else if (input < min)
        output = min;
    else
        output = input;

    return output;
}

MINT32 MIN(MINT32 a, MINT32 b)
{
    MINT32 output = 0;

    if (a < b)
    {
        output = a;
    }
    else
    {
        output = b;
    }

    return output;
}

MINT32 MAX(MINT32 a, MINT32 b)
{
    MINT32 output = 0;

    if (a > b)
    {
        output = a;
    }
    else
    {
        output = b;
    }

    return output;
}

MVOID FD2FaceIn(CAMERA_TUNING_FD_INFO_T const& rFdInfo,
               FaceInput_T& rFaceInput)
{
    //Race classification

    rFaceInput.Race= 1;

	MINT32 YMean=0;
	MINT32 UMean=0;
	MINT32 VMean=0;
	MINT32 Y5=0;
	MINT32 Y95=0;

	MINT32 SCenter=0;
	MINT32 HCenter=0;

    if((rFdInfo.FaceNum)>0)
		{
			for(int i=0; i < (rFdInfo.FaceNum); i++)
			{
				YMean = rFdInfo.YUVsts[i][0];
				UMean = rFdInfo.YUVsts[i][1];
				VMean = rFdInfo.YUVsts[i][2];
				Y5 = rFdInfo.YUVsts[i][3];
				Y95 = rFdInfo.YUVsts[i][4];

				
				SCenter = (MINT32) (sqrt(((UMean - 128) * (UMean - 128)) + ((VMean - 128) * (VMean - 128))));
				HCenter =  ((MINT32)((atan2((double)(VMean-128),(double)(UMean-128)) * 180 / PI)+360)%360);

				if((90 <= HCenter) && (HCenter <= 160))
				{break;}
			}
		}
	
    rFaceInput.Y_Center = (Y5 +Y95) / 2;
	rFaceInput.S_Center = SCenter;
	rFaceInput.H_Center = HCenter;
	
}


MVOID Face2MCPara(FaceInput_T const& rFaceInput,
	           ISP_NVRAM_FD_COLOR_PARAM_T const& FD_Para,
               MemoryColorMappingInput_T& rMCMappingIn)
{

    MINT32 Race_Idx = 1;
	MINT32 Y_Center = 25;
	MINT32 Y_Range = 50;
	MINT32 Y_Offset = 5;
	MINT32 S_Center = 25;
	MINT32 S_Range = 50;
	MINT32 S_Offset = 5;
	MINT32 H_Center = 25;
	MINT32 H_Range = 50;
	MINT32 H_Offset = 5;

	Race_Idx = rFaceInput.Race;
	Y_Center = rFaceInput.Y_Center;
	S_Center = rFaceInput.S_Center;
	H_Center = rFaceInput.H_Center;

	switch(Race_Idx)
	{
	    case 1 :
			Y_Range = FD_Para.u4Range_W1_Y;
			S_Range = FD_Para.u4Range_W1_S;
			H_Range = FD_Para.u4Range_W1_H;
			Y_Offset = FD_Para.u4Offset_W1_Y - 128;
			S_Offset = FD_Para.u4Offset_W1_S - 128;
			H_Offset = FD_Para.u4Offset_W1_H - 128;
		    break;
		case 2 :
			Y_Range = FD_Para.u4Range_W2_Y;
			S_Range = FD_Para.u4Range_W2_S;
			H_Range = FD_Para.u4Range_W2_H;
			Y_Offset = FD_Para.u4Offset_W2_Y - 128;
			S_Offset = FD_Para.u4Offset_W2_S - 128;
			H_Offset = FD_Para.u4Offset_W2_H - 128;
			break;
		case 3 :
			Y_Range = FD_Para.u4Range_W3_Y;
			S_Range = FD_Para.u4Range_W3_S;
			H_Range = FD_Para.u4Range_W3_H;
			Y_Offset = FD_Para.u4Offset_W3_Y - 128;
			S_Offset = FD_Para.u4Offset_W3_S - 128;
			H_Offset = FD_Para.u4Offset_W3_H - 128;
		    break;
		default :
		    break;
	}


	MINT32 Luma_Start = 0;
	MINT32 Luma_End = 50;
	MINT32 Luma_Offset = 5;
	MINT32 Sat_Start = 0;
	MINT32 Sat_End = 50;
	MINT32 Sat_Offset = 5;
	MINT32 Hue_Start = 0;
	MINT32 Hue_End = 50;
	MINT32 Hue_Offset = 5;

	Luma_Start = Clip((Y_Center - Y_Range/2), 255, 0) ;
	Luma_End = Clip((Y_Center + Y_Range/2), 255, 0) ;
	Luma_Offset = Y_Offset;

	Sat_Start = Clip((S_Center - S_Range/2), 127, 0) ;
	Sat_End = Clip((S_Center + S_Range/2), 127, 0) ;
	Sat_Offset = S_Offset;

	Hue_Start = Clip((H_Center - H_Range/2), 360, -360) ;
	Hue_Start = Hue_Start > 0 ? Hue_Start : Hue_Start + 360;
	Hue_End = Clip((H_Center + H_Range/2), 720, 0) ;
	Hue_End = Hue_End < 360 ? Hue_End : Hue_End - 360;
	Hue_Offset = H_Offset;

	

	rMCMappingIn.Luma_Start = Luma_Start;
	rMCMappingIn.Luma_End = Luma_End;
	rMCMappingIn.Luma_Offset = Luma_Offset;
	rMCMappingIn.Sat_Start = Sat_Start;
	rMCMappingIn.Sat_End = Sat_End;
	rMCMappingIn.Sat_Offset = Sat_Offset;
	rMCMappingIn.Hue_Start = Hue_Start;
	rMCMappingIn.Hue_End = Hue_End;
	rMCMappingIn.Hue_Offset = Hue_Offset;
	
}


MVOID MC_Mapping(MemoryColorMappingInput_T const& rParam,   
               MemoryColorMappingOutput_T& rMCMappingOut)
{
    MINT32 Y_Scale = 1023;
	MINT32 S_Scale = 127;
	MINT32 H_Scale = 1023;

	MINT32 Y_In_Range = 255;
	MINT32 S_In_Range = 127;
	MINT32 H_In_Range = 360;
	
    MINT32 Y_WGT_Scale = 8192;
	MINT32 S_WGT_Scale = 2048;
	MINT32 H_WGT_Scale = 1024;

	MINT32 Slope_Scale = 128;
	
    MINT32 Luma_Start = rParam.Luma_Start;
	MINT32 Luma_End = rParam.Luma_End;
	MINT32 Luma_Offset = rParam.Luma_Offset;
	MINT32 Sat_Start = rParam.Sat_Start;
	MINT32 Sat_End = rParam.Sat_End;
	MINT32 Sat_Offset = rParam.Sat_Offset;
	MINT32 Hue_Start = rParam.Hue_Start;
	MINT32 Hue_End = rParam.Hue_End;
	MINT32 Hue_Offset = rParam.Hue_Offset;

	if(Hue_End < Hue_Start)
	{
	    Hue_End = Hue_End + 360;
	}

	MINT32 Y_Gap = (Luma_End - Luma_Start);
	MINT32 S_Gap = (Sat_End - Sat_Start);
	MINT32 H_Gap = (Hue_End - Hue_Start);

	Y_Gap = MAX(Y_Gap, 1);
	S_Gap = MAX(S_Gap, 1);
	H_Gap = MAX(H_Gap, 1);
	
	rMCMappingOut.Y.SLOPE0= (Slope_Scale + Slope_Scale * Luma_Offset * 2 / Y_Gap);
	rMCMappingOut.Y.SLOPE1= (Slope_Scale + Slope_Scale * Luma_Offset * 2 / Y_Gap);
	rMCMappingOut.Y.SLOPE2= (Slope_Scale + Slope_Scale * Luma_Offset * 2 / Y_Gap);
    rMCMappingOut.Y.SLOPE3= (Slope_Scale - Slope_Scale * Luma_Offset * 2 / Y_Gap);
	rMCMappingOut.Y.SLOPE4= (Slope_Scale - Slope_Scale * Luma_Offset * 2 / Y_Gap);
	rMCMappingOut.Y.SLOPE5= (Slope_Scale - Slope_Scale * Luma_Offset * 2 / Y_Gap);

	rMCMappingOut.Y.L = Luma_Start * Y_Scale / Y_In_Range;
	rMCMappingOut.Y.POINT0= (Luma_Start + Y_Gap / 6)* Y_Scale / Y_In_Range;
	rMCMappingOut.Y.POINT1= (Luma_Start + Y_Gap * 2 / 6)* Y_Scale / Y_In_Range;
	rMCMappingOut.Y.POINT2= (Luma_Start + Y_Gap * 3 / 6)* Y_Scale / Y_In_Range;
	rMCMappingOut.Y.POINT3= (Luma_Start + Y_Gap * 4 / 6)* Y_Scale / Y_In_Range;
	rMCMappingOut.Y.POINT4= (Luma_Start + Y_Gap * 5 / 6)* Y_Scale / Y_In_Range;
	rMCMappingOut.Y.U = Luma_End * Y_Scale / Y_In_Range;

	rMCMappingOut.Y.WGT_LSLOPE =  Y_WGT_Scale / (rMCMappingOut.Y.POINT2 - rMCMappingOut.Y.L);
	rMCMappingOut.Y.WGT_USLOPE =  Y_WGT_Scale / (rMCMappingOut.Y.U - rMCMappingOut.Y.POINT2);

	rMCMappingOut.S.SLOPE0= (Slope_Scale + Slope_Scale * Sat_Offset * 2 / S_Gap);
	rMCMappingOut.S.SLOPE1= (Slope_Scale + Slope_Scale * Sat_Offset * 2 / S_Gap);
	rMCMappingOut.S.SLOPE2= (Slope_Scale + Slope_Scale * Sat_Offset * 2 / S_Gap);
    rMCMappingOut.S.SLOPE3= (Slope_Scale - Slope_Scale * Sat_Offset * 2 / S_Gap);
	rMCMappingOut.S.SLOPE4= (Slope_Scale - Slope_Scale * Sat_Offset * 2 / S_Gap);
	rMCMappingOut.S.SLOPE5= (Slope_Scale - Slope_Scale * Sat_Offset * 2 / S_Gap);

	rMCMappingOut.S.L = Sat_Start * S_Scale / S_In_Range;
	rMCMappingOut.S.POINT0= (Sat_Start + S_Gap / 6)* S_Scale / S_In_Range;
	rMCMappingOut.S.POINT1= (Sat_Start + S_Gap * 2 / 6)* S_Scale / S_In_Range;
	rMCMappingOut.S.POINT2= (Sat_Start + S_Gap * 3 / 6)* S_Scale / S_In_Range;
	rMCMappingOut.S.POINT3= (Sat_Start + S_Gap * 4 / 6)* S_Scale / S_In_Range;
	rMCMappingOut.S.POINT4= (Sat_Start + S_Gap * 5 / 6)* S_Scale / S_In_Range;
	rMCMappingOut.S.U = Sat_End * S_Scale / S_In_Range;

	rMCMappingOut.S.WGT_LSLOPE =  S_WGT_Scale / (rMCMappingOut.S.POINT2 - rMCMappingOut.S.L);
	rMCMappingOut.S.WGT_USLOPE =  S_WGT_Scale / (rMCMappingOut.S.U - rMCMappingOut.S.POINT2);

	rMCMappingOut.H.SLOPE0= (Slope_Scale + Slope_Scale * Hue_Offset * 2 / H_Gap);
	rMCMappingOut.H.SLOPE1= (Slope_Scale + Slope_Scale * Hue_Offset * 2 / H_Gap);
	rMCMappingOut.H.SLOPE2= (Slope_Scale + Slope_Scale * Hue_Offset * 2 / H_Gap);
    rMCMappingOut.H.SLOPE3= (Slope_Scale - Slope_Scale * Hue_Offset * 2 / H_Gap);
	rMCMappingOut.H.SLOPE4= (Slope_Scale - Slope_Scale * Hue_Offset * 2 / H_Gap);
	rMCMappingOut.H.SLOPE5= (Slope_Scale - Slope_Scale * Hue_Offset * 2 / H_Gap);

	rMCMappingOut.H.L = Hue_Start * H_Scale / H_In_Range;
	rMCMappingOut.H.POINT0= (Hue_Start + H_Gap / 6) < 360 ? (Hue_Start + H_Gap / 6)* H_Scale / H_In_Range : (Hue_Start + H_Gap / 6 - 360)* H_Scale / H_In_Range;
	rMCMappingOut.H.POINT1= (Hue_Start + H_Gap * 2 / 6) < 360 ? (Hue_Start + H_Gap * 2 / 6)* H_Scale / H_In_Range : (Hue_Start + H_Gap * 2 / 6 - 360)* H_Scale / H_In_Range;
	rMCMappingOut.H.POINT2= (Hue_Start + H_Gap * 3 / 6) < 360 ? (Hue_Start + H_Gap * 3 / 6)* H_Scale / H_In_Range : (Hue_Start + H_Gap * 3 / 6 - 360)* H_Scale / H_In_Range;
	rMCMappingOut.H.POINT3= (Hue_Start + H_Gap * 4 / 6) < 360 ? (Hue_Start + H_Gap * 4 / 6)* H_Scale / H_In_Range : (Hue_Start + H_Gap * 4 / 6 - 360)* H_Scale / H_In_Range;
	rMCMappingOut.H.POINT4= (Hue_Start + H_Gap * 5 / 6) < 360 ? (Hue_Start + H_Gap * 5 / 6)* H_Scale / H_In_Range : (Hue_Start + H_Gap * 5 / 6 - 360)* H_Scale / H_In_Range;
	rMCMappingOut.H.U = Hue_End * H_Scale / H_In_Range;

    if(rMCMappingOut.H.POINT2 > rMCMappingOut.H.L)
    {
	    rMCMappingOut.H.WGT_LSLOPE =  H_WGT_Scale / (rMCMappingOut.H.POINT2 - rMCMappingOut.H.L);
    }
	else
	{
	    rMCMappingOut.H.WGT_LSLOPE =  H_WGT_Scale / (rMCMappingOut.H.POINT2 + 360 * H_Scale / H_In_Range - rMCMappingOut.H.L);
    }

	if(rMCMappingOut.H.U > rMCMappingOut.H.POINT2)
    {
	   rMCMappingOut.H.WGT_USLOPE =  H_WGT_Scale / (rMCMappingOut.H.U - rMCMappingOut.H.POINT2);
    }
	else
	{
	   rMCMappingOut.H.WGT_USLOPE =  H_WGT_Scale / (rMCMappingOut.H.U + 360 * H_Scale / H_In_Range - rMCMappingOut.H.POINT2);
    }
	
	
}

MVOID MC_UpdateHWReg(MINT32 WinIdx, MemoryColorMappingOutput_T const& rMCMappingOut,   
               ISP_NVRAM_COLOR_T& rHW_Color)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.FD_ColorHW_Dump.enable", value, "0");
    MBOOL bFDColorDebug = atoi(value);

    switch(WinIdx)
    {
		case 1 :
			rHW_Color.color_cm_control.bits.COLOR_CM_W1_EN       = 1;
            rHW_Color.color_cm_control.bits.COLOR_CM_W1_WGT_EN   = 1;
			
			// ISP_NVRAM_COLOR_CM_W1_HUE_0_T
			rHW_Color.color_cm_w1_hue_0.bits.COLOR_CM_H_W1_L 	 = rMCMappingOut.H.L;
			rHW_Color.color_cm_w1_hue_0.bits.COLOR_CM_H_W1_U 	 = rMCMappingOut.H.U;
			rHW_Color.color_cm_w1_hue_0.bits.COLOR_CM_H_W1_POINT0 = rMCMappingOut.H.POINT0;
	
			// ISP_NVRAM_COLOR_CM_W1_HUE_1_T
			rHW_Color.color_cm_w1_hue_1.bits.COLOR_CM_H_W1_POINT1 = rMCMappingOut.H.POINT1;
			rHW_Color.color_cm_w1_hue_1.bits.COLOR_CM_H_W1_POINT2 = rMCMappingOut.H.POINT2;
			rHW_Color.color_cm_w1_hue_1.bits.COLOR_CM_H_W1_POINT3 = rMCMappingOut.H.POINT3;
	
			// ISP_NVRAM_COLOR_CM_W1_HUE_2_T
			rHW_Color.color_cm_w1_hue_2.bits.COLOR_CM_H_W1_POINT4 = rMCMappingOut.H.POINT4;
			rHW_Color.color_cm_w1_hue_2.bits.COLOR_CM_H_W1_SLOPE0 = rMCMappingOut.H.SLOPE0;
			rHW_Color.color_cm_w1_hue_2.bits.COLOR_CM_H_W1_SLOPE1 = rMCMappingOut.H.SLOPE1;
	
			// ISP_NVRAM_COLOR_CM_W1_HUE_3_T
			rHW_Color.color_cm_w1_hue_3.bits.COLOR_CM_H_W1_SLOPE2 = rMCMappingOut.H.SLOPE2;
			rHW_Color.color_cm_w1_hue_3.bits.COLOR_CM_H_W1_SLOPE3 = rMCMappingOut.H.SLOPE3;
			rHW_Color.color_cm_w1_hue_3.bits.COLOR_CM_H_W1_SLOPE4 = rMCMappingOut.H.SLOPE4;
			rHW_Color.color_cm_w1_hue_3.bits.COLOR_CM_H_W1_SLOPE5 = rMCMappingOut.H.SLOPE5;
	
			// ISP_NVRAM_COLOR_CM_W1_HUE_4_T
			rHW_Color.color_cm_w1_hue_4.bits.COLOR_CM_H_W1_WGT_LSLOPE = rMCMappingOut.H.WGT_LSLOPE;
			rHW_Color.color_cm_w1_hue_4.bits.COLOR_CM_H_W1_WGT_USLOPE = rMCMappingOut.H.WGT_USLOPE;
	
			// ISP_NVRAM_COLOR_CM_W1_LUMA_0_T  ;
			rHW_Color.color_cm_w1_luma_0.bits.COLOR_CM_Y_W1_L	  = rMCMappingOut.Y.L;
			rHW_Color.color_cm_w1_luma_0.bits.COLOR_CM_Y_W1_U	  = rMCMappingOut.Y.U;
			rHW_Color.color_cm_w1_luma_0.bits.COLOR_CM_Y_W1_POINT0 = rMCMappingOut.Y.POINT0;
	
			// ISP_NVRAM_COLOR_CM_W1_LUMA_1_T  ;
			rHW_Color.color_cm_w1_luma_1.bits.COLOR_CM_Y_W1_POINT1 = rMCMappingOut.Y.POINT1;
			rHW_Color.color_cm_w1_luma_1.bits.COLOR_CM_Y_W1_POINT2 = rMCMappingOut.Y.POINT2;
			rHW_Color.color_cm_w1_luma_1.bits.COLOR_CM_Y_W1_POINT3 = rMCMappingOut.Y.POINT3;
	
			// ISP_NVRAM_COLOR_CM_W1_LUMA_2_T  ;
			rHW_Color.color_cm_w1_luma_2.bits.COLOR_CM_Y_W1_POINT4 = rMCMappingOut.Y.POINT4;
			rHW_Color.color_cm_w1_luma_2.bits.COLOR_CM_Y_W1_SLOPE0 = rMCMappingOut.Y.SLOPE0;
			rHW_Color.color_cm_w1_luma_2.bits.COLOR_CM_Y_W1_SLOPE1 = rMCMappingOut.Y.SLOPE1;
	
			// ISP_NVRAM_COLOR_CM_W1_LUMA_3_T  ;
			rHW_Color.color_cm_w1_luma_3.bits.COLOR_CM_Y_W1_SLOPE2 = rMCMappingOut.Y.SLOPE2;
			rHW_Color.color_cm_w1_luma_3.bits.COLOR_CM_Y_W1_SLOPE3 = rMCMappingOut.Y.SLOPE3;
			rHW_Color.color_cm_w1_luma_3.bits.COLOR_CM_Y_W1_SLOPE4 = rMCMappingOut.Y.SLOPE4;
			rHW_Color.color_cm_w1_luma_3.bits.COLOR_CM_Y_W1_SLOPE5 = rMCMappingOut.Y.SLOPE5;
	
			// ISP_NVRAM_COLOR_CM_W1_LUMA_4_T  ;
			rHW_Color.color_cm_w1_luma_4.bits.COLOR_CM_Y_W1_WGT_LSLOPE = rMCMappingOut.Y.WGT_LSLOPE;
			rHW_Color.color_cm_w1_luma_4.bits.COLOR_CM_Y_W1_WGT_USLOPE = rMCMappingOut.Y.WGT_USLOPE;
	
			// ISP_NVRAM_COLOR_CM_W1_SAT_0_T
			rHW_Color.color_cm_w1_sat_0.bits.COLOR_CM_S_W1_L 	 = rMCMappingOut.S.L;
			rHW_Color.color_cm_w1_sat_0.bits.COLOR_CM_S_W1_U 	 = rMCMappingOut.S.U;
			rHW_Color.color_cm_w1_sat_0.bits.COLOR_CM_S_W1_POINT0 = rMCMappingOut.S.POINT0;
	
			// ISP_NVRAM_COLOR_CM_W1_SAT_1_T
			rHW_Color.color_cm_w1_sat_1.bits.COLOR_CM_S_W1_POINT1 = rMCMappingOut.S.POINT1;
			rHW_Color.color_cm_w1_sat_1.bits.COLOR_CM_S_W1_POINT2 = rMCMappingOut.S.POINT2;
			rHW_Color.color_cm_w1_sat_1.bits.COLOR_CM_S_W1_POINT3 = rMCMappingOut.S.POINT3;
	
			// ISP_NVRAM_COLOR_CM_W1_SAT_2_T
			rHW_Color.color_cm_w1_sat_2.bits.COLOR_CM_S_W1_POINT4 = rMCMappingOut.S.POINT4;
			rHW_Color.color_cm_w1_sat_2.bits.COLOR_CM_S_W1_SLOPE0 = rMCMappingOut.S.SLOPE0;
			rHW_Color.color_cm_w1_sat_2.bits.COLOR_CM_S_W1_SLOPE1 = rMCMappingOut.S.SLOPE1;
	
			// ISP_NVRAM_COLOR_CM_W1_SAT_3_T
			rHW_Color.color_cm_w1_sat_3.bits.COLOR_CM_S_W1_SLOPE2 = rMCMappingOut.S.SLOPE2;
			rHW_Color.color_cm_w1_sat_3.bits.COLOR_CM_S_W1_SLOPE3 = rMCMappingOut.S.SLOPE3;
			rHW_Color.color_cm_w1_sat_3.bits.COLOR_CM_S_W1_SLOPE4 = rMCMappingOut.S.SLOPE4;
			rHW_Color.color_cm_w1_sat_3.bits.COLOR_CM_S_W1_SLOPE5 = rMCMappingOut.S.SLOPE5;
	
			// ISP_NVRAM_COLOR_CM_W1_SAT_4_T
			rHW_Color.color_cm_w1_sat_4.bits.COLOR_CM_S_W1_WGT_LSLOPE = rMCMappingOut.S.WGT_LSLOPE;
			rHW_Color.color_cm_w1_sat_4.bits.COLOR_CM_S_W1_WGT_USLOPE = rMCMappingOut.S.WGT_USLOPE;
	        break;
		case 2 :
			rHW_Color.color_cm_control.bits.COLOR_CM_W2_EN       = 1;
            rHW_Color.color_cm_control.bits.COLOR_CM_W2_WGT_EN   = 1;
			
			// ISP_NVRAM_COLOR_CM_W2_HUE_0_T
			rHW_Color.color_cm_w2_hue_0.bits.COLOR_CM_H_W2_L 	 = rMCMappingOut.H.L;
			rHW_Color.color_cm_w2_hue_0.bits.COLOR_CM_H_W2_U 	 = rMCMappingOut.H.U;
			rHW_Color.color_cm_w2_hue_0.bits.COLOR_CM_H_W2_POINT0 = rMCMappingOut.H.POINT0;
	
			// ISP_NVRAM_COLOR_CM_W2_HUE_1_T
			rHW_Color.color_cm_w2_hue_1.bits.COLOR_CM_H_W2_POINT1 = rMCMappingOut.H.POINT1;
			rHW_Color.color_cm_w2_hue_1.bits.COLOR_CM_H_W2_POINT2 = rMCMappingOut.H.POINT2;
			rHW_Color.color_cm_w2_hue_1.bits.COLOR_CM_H_W2_POINT3 = rMCMappingOut.H.POINT3;
	
			// ISP_NVRAM_COLOR_CM_W2_HUE_2_T
			rHW_Color.color_cm_w2_hue_2.bits.COLOR_CM_H_W2_POINT4 = rMCMappingOut.H.POINT4;
			rHW_Color.color_cm_w2_hue_2.bits.COLOR_CM_H_W2_SLOPE0 = rMCMappingOut.H.SLOPE0;
			rHW_Color.color_cm_w2_hue_2.bits.COLOR_CM_H_W2_SLOPE1 = rMCMappingOut.H.SLOPE1;
	
			// ISP_NVRAM_COLOR_CM_W2_HUE_3_T
			rHW_Color.color_cm_w2_hue_3.bits.COLOR_CM_H_W2_SLOPE2 = rMCMappingOut.H.SLOPE2;
			rHW_Color.color_cm_w2_hue_3.bits.COLOR_CM_H_W2_SLOPE3 = rMCMappingOut.H.SLOPE3;
			rHW_Color.color_cm_w2_hue_3.bits.COLOR_CM_H_W2_SLOPE4 = rMCMappingOut.H.SLOPE4;
			rHW_Color.color_cm_w2_hue_3.bits.COLOR_CM_H_W2_SLOPE5 = rMCMappingOut.H.SLOPE5;
	
			// ISP_NVRAM_COLOR_CM_W2_HUE_4_T
			rHW_Color.color_cm_w2_hue_4.bits.COLOR_CM_H_W2_WGT_LSLOPE = rMCMappingOut.H.WGT_LSLOPE;
			rHW_Color.color_cm_w2_hue_4.bits.COLOR_CM_H_W2_WGT_USLOPE = rMCMappingOut.H.WGT_USLOPE;
	
			// ISP_NVRAM_COLOR_CM_W2_LUMA_0_T  ;
			rHW_Color.color_cm_w2_luma_0.bits.COLOR_CM_Y_W2_L	  = rMCMappingOut.Y.L;
			rHW_Color.color_cm_w2_luma_0.bits.COLOR_CM_Y_W2_U	  = rMCMappingOut.Y.U;
			rHW_Color.color_cm_w2_luma_0.bits.COLOR_CM_Y_W2_POINT0 = rMCMappingOut.Y.POINT0;
	
			// ISP_NVRAM_COLOR_CM_W2_LUMA_1_T  ;
			rHW_Color.color_cm_w2_luma_1.bits.COLOR_CM_Y_W2_POINT1 = rMCMappingOut.Y.POINT1;
			rHW_Color.color_cm_w2_luma_1.bits.COLOR_CM_Y_W2_POINT2 = rMCMappingOut.Y.POINT2;
			rHW_Color.color_cm_w2_luma_1.bits.COLOR_CM_Y_W2_POINT3 = rMCMappingOut.Y.POINT3;
	
			// ISP_NVRAM_COLOR_CM_W2_LUMA_2_T  ;
			rHW_Color.color_cm_w2_luma_2.bits.COLOR_CM_Y_W2_POINT4 = rMCMappingOut.Y.POINT4;
			rHW_Color.color_cm_w2_luma_2.bits.COLOR_CM_Y_W2_SLOPE0 = rMCMappingOut.Y.SLOPE0;
			rHW_Color.color_cm_w2_luma_2.bits.COLOR_CM_Y_W2_SLOPE1 = rMCMappingOut.Y.SLOPE1;
	
			// ISP_NVRAM_COLOR_CM_W2_LUMA_3_T  ;
			rHW_Color.color_cm_w2_luma_3.bits.COLOR_CM_Y_W2_SLOPE2 = rMCMappingOut.Y.SLOPE2;
			rHW_Color.color_cm_w2_luma_3.bits.COLOR_CM_Y_W2_SLOPE3 = rMCMappingOut.Y.SLOPE3;
			rHW_Color.color_cm_w2_luma_3.bits.COLOR_CM_Y_W2_SLOPE4 = rMCMappingOut.Y.SLOPE4;
			rHW_Color.color_cm_w2_luma_3.bits.COLOR_CM_Y_W2_SLOPE5 = rMCMappingOut.Y.SLOPE5;
	
			// ISP_NVRAM_COLOR_CM_W2_LUMA_4_T  ;
			rHW_Color.color_cm_w2_luma_4.bits.COLOR_CM_Y_W2_WGT_LSLOPE = rMCMappingOut.Y.WGT_LSLOPE;
			rHW_Color.color_cm_w2_luma_4.bits.COLOR_CM_Y_W2_WGT_USLOPE = rMCMappingOut.Y.WGT_USLOPE;
	
			// ISP_NVRAM_COLOR_CM_W2_SAT_0_T
			rHW_Color.color_cm_w2_sat_0.bits.COLOR_CM_S_W2_L 	 = rMCMappingOut.S.L;
			rHW_Color.color_cm_w2_sat_0.bits.COLOR_CM_S_W2_U 	 = rMCMappingOut.S.U;
			rHW_Color.color_cm_w2_sat_0.bits.COLOR_CM_S_W2_POINT0 = rMCMappingOut.S.POINT0;
	
			// ISP_NVRAM_COLOR_CM_W2_SAT_1_T
			rHW_Color.color_cm_w2_sat_1.bits.COLOR_CM_S_W2_POINT1 = rMCMappingOut.S.POINT1;
			rHW_Color.color_cm_w2_sat_1.bits.COLOR_CM_S_W2_POINT2 = rMCMappingOut.S.POINT2;
			rHW_Color.color_cm_w2_sat_1.bits.COLOR_CM_S_W2_POINT3 = rMCMappingOut.S.POINT3;
	
			// ISP_NVRAM_COLOR_CM_W2_SAT_2_T
			rHW_Color.color_cm_w2_sat_2.bits.COLOR_CM_S_W2_POINT4 = rMCMappingOut.S.POINT4;
			rHW_Color.color_cm_w2_sat_2.bits.COLOR_CM_S_W2_SLOPE0 = rMCMappingOut.S.SLOPE0;
			rHW_Color.color_cm_w2_sat_2.bits.COLOR_CM_S_W2_SLOPE1 = rMCMappingOut.S.SLOPE1;
	
			// ISP_NVRAM_COLOR_CM_W2_SAT_3_T
			rHW_Color.color_cm_w2_sat_3.bits.COLOR_CM_S_W2_SLOPE2 = rMCMappingOut.S.SLOPE2;
			rHW_Color.color_cm_w2_sat_3.bits.COLOR_CM_S_W2_SLOPE3 = rMCMappingOut.S.SLOPE3;
			rHW_Color.color_cm_w2_sat_3.bits.COLOR_CM_S_W2_SLOPE4 = rMCMappingOut.S.SLOPE4;
			rHW_Color.color_cm_w2_sat_3.bits.COLOR_CM_S_W2_SLOPE5 = rMCMappingOut.S.SLOPE5;
	
			// ISP_NVRAM_COLOR_CM_W2_SAT_4_T
			rHW_Color.color_cm_w2_sat_4.bits.COLOR_CM_S_W2_WGT_LSLOPE = rMCMappingOut.S.WGT_LSLOPE;
			rHW_Color.color_cm_w2_sat_4.bits.COLOR_CM_S_W2_WGT_USLOPE = rMCMappingOut.S.WGT_USLOPE;
	        break;
		case 3 :
			rHW_Color.color_cm_control.bits.COLOR_CM_W3_EN       = 1;
            rHW_Color.color_cm_control.bits.COLOR_CM_W3_WGT_EN   = 1;
			
			// ISP_NVRAM_COLOR_CM_W3_HUE_0_T
			rHW_Color.color_cm_w3_hue_0.bits.COLOR_CM_H_W3_L 	 = rMCMappingOut.H.L;
			rHW_Color.color_cm_w3_hue_0.bits.COLOR_CM_H_W3_U 	 = rMCMappingOut.H.U;
			rHW_Color.color_cm_w3_hue_0.bits.COLOR_CM_H_W3_POINT0 = rMCMappingOut.H.POINT0;
	
			// ISP_NVRAM_COLOR_CM_W3_HUE_1_T
			rHW_Color.color_cm_w3_hue_1.bits.COLOR_CM_H_W3_POINT1 = rMCMappingOut.H.POINT1;
			rHW_Color.color_cm_w3_hue_1.bits.COLOR_CM_H_W3_POINT2 = rMCMappingOut.H.POINT2;
			rHW_Color.color_cm_w3_hue_1.bits.COLOR_CM_H_W3_POINT3 = rMCMappingOut.H.POINT3;
	
			// ISP_NVRAM_COLOR_CM_W3_HUE_2_T
			rHW_Color.color_cm_w3_hue_2.bits.COLOR_CM_H_W3_POINT4 = rMCMappingOut.H.POINT4;
			rHW_Color.color_cm_w3_hue_2.bits.COLOR_CM_H_W3_SLOPE0 = rMCMappingOut.H.SLOPE0;
			rHW_Color.color_cm_w3_hue_2.bits.COLOR_CM_H_W3_SLOPE1 = rMCMappingOut.H.SLOPE1;
	
			// ISP_NVRAM_COLOR_CM_W3_HUE_3_T
			rHW_Color.color_cm_w3_hue_3.bits.COLOR_CM_H_W3_SLOPE2 = rMCMappingOut.H.SLOPE2;
			rHW_Color.color_cm_w3_hue_3.bits.COLOR_CM_H_W3_SLOPE3 = rMCMappingOut.H.SLOPE3;
			rHW_Color.color_cm_w3_hue_3.bits.COLOR_CM_H_W3_SLOPE4 = rMCMappingOut.H.SLOPE4;
			rHW_Color.color_cm_w3_hue_3.bits.COLOR_CM_H_W3_SLOPE5 = rMCMappingOut.H.SLOPE5;
	
			// ISP_NVRAM_COLOR_CM_W3_HUE_4_T
			rHW_Color.color_cm_w3_hue_4.bits.COLOR_CM_H_W3_WGT_LSLOPE = rMCMappingOut.H.WGT_LSLOPE;
			rHW_Color.color_cm_w3_hue_4.bits.COLOR_CM_H_W3_WGT_USLOPE = rMCMappingOut.H.WGT_USLOPE;
	
			// ISP_NVRAM_COLOR_CM_W3_LUMA_0_T  ;
			rHW_Color.color_cm_w3_luma_0.bits.COLOR_CM_Y_W3_L	  = rMCMappingOut.Y.L;
			rHW_Color.color_cm_w3_luma_0.bits.COLOR_CM_Y_W3_U	  = rMCMappingOut.Y.U;
			rHW_Color.color_cm_w3_luma_0.bits.COLOR_CM_Y_W3_POINT0 = rMCMappingOut.Y.POINT0;
	
			// ISP_NVRAM_COLOR_CM_W3_LUMA_1_T  ;
			rHW_Color.color_cm_w3_luma_1.bits.COLOR_CM_Y_W3_POINT1 = rMCMappingOut.Y.POINT1;
			rHW_Color.color_cm_w3_luma_1.bits.COLOR_CM_Y_W3_POINT2 = rMCMappingOut.Y.POINT2;
			rHW_Color.color_cm_w3_luma_1.bits.COLOR_CM_Y_W3_POINT3 = rMCMappingOut.Y.POINT3;
	
			// ISP_NVRAM_COLOR_CM_W3_LUMA_2_T  ;
			rHW_Color.color_cm_w3_luma_2.bits.COLOR_CM_Y_W3_POINT4 = rMCMappingOut.Y.POINT4;
			rHW_Color.color_cm_w3_luma_2.bits.COLOR_CM_Y_W3_SLOPE0 = rMCMappingOut.Y.SLOPE0;
			rHW_Color.color_cm_w3_luma_2.bits.COLOR_CM_Y_W3_SLOPE1 = rMCMappingOut.Y.SLOPE1;
	
			// ISP_NVRAM_COLOR_CM_W3_LUMA_3_T  ;
			rHW_Color.color_cm_w3_luma_3.bits.COLOR_CM_Y_W3_SLOPE2 = rMCMappingOut.Y.SLOPE2;
			rHW_Color.color_cm_w3_luma_3.bits.COLOR_CM_Y_W3_SLOPE3 = rMCMappingOut.Y.SLOPE3;
			rHW_Color.color_cm_w3_luma_3.bits.COLOR_CM_Y_W3_SLOPE4 = rMCMappingOut.Y.SLOPE4;
			rHW_Color.color_cm_w3_luma_3.bits.COLOR_CM_Y_W3_SLOPE5 = rMCMappingOut.Y.SLOPE5;
	
			// ISP_NVRAM_COLOR_CM_W3_LUMA_4_T  ;
			rHW_Color.color_cm_w3_luma_4.bits.COLOR_CM_Y_W3_WGT_LSLOPE = rMCMappingOut.Y.WGT_LSLOPE;
			rHW_Color.color_cm_w3_luma_4.bits.COLOR_CM_Y_W3_WGT_USLOPE = rMCMappingOut.Y.WGT_USLOPE;
	
			// ISP_NVRAM_COLOR_CM_W3_SAT_0_T
			rHW_Color.color_cm_w3_sat_0.bits.COLOR_CM_S_W3_L 	 = rMCMappingOut.S.L;
			rHW_Color.color_cm_w3_sat_0.bits.COLOR_CM_S_W3_U 	 = rMCMappingOut.S.U;
			rHW_Color.color_cm_w3_sat_0.bits.COLOR_CM_S_W3_POINT0 = rMCMappingOut.S.POINT0;
	
			// ISP_NVRAM_COLOR_CM_W3_SAT_1_T
			rHW_Color.color_cm_w3_sat_1.bits.COLOR_CM_S_W3_POINT1 = rMCMappingOut.S.POINT1;
			rHW_Color.color_cm_w3_sat_1.bits.COLOR_CM_S_W3_POINT2 = rMCMappingOut.S.POINT2;
			rHW_Color.color_cm_w3_sat_1.bits.COLOR_CM_S_W3_POINT3 = rMCMappingOut.S.POINT3;
	
			// ISP_NVRAM_COLOR_CM_W3_SAT_2_T
			rHW_Color.color_cm_w3_sat_2.bits.COLOR_CM_S_W3_POINT4 = rMCMappingOut.S.POINT4;
			rHW_Color.color_cm_w3_sat_2.bits.COLOR_CM_S_W3_SLOPE0 = rMCMappingOut.S.SLOPE0;
			rHW_Color.color_cm_w3_sat_2.bits.COLOR_CM_S_W3_SLOPE1 = rMCMappingOut.S.SLOPE1;
	
			// ISP_NVRAM_COLOR_CM_W3_SAT_3_T
			rHW_Color.color_cm_w3_sat_3.bits.COLOR_CM_S_W3_SLOPE2 = rMCMappingOut.S.SLOPE2;
			rHW_Color.color_cm_w3_sat_3.bits.COLOR_CM_S_W3_SLOPE3 = rMCMappingOut.S.SLOPE3;
			rHW_Color.color_cm_w3_sat_3.bits.COLOR_CM_S_W3_SLOPE4 = rMCMappingOut.S.SLOPE4;
			rHW_Color.color_cm_w3_sat_3.bits.COLOR_CM_S_W3_SLOPE5 = rMCMappingOut.S.SLOPE5;
	
			// ISP_NVRAM_COLOR_CM_W3_SAT_4_T
			rHW_Color.color_cm_w3_sat_4.bits.COLOR_CM_S_W3_WGT_LSLOPE = rMCMappingOut.S.WGT_LSLOPE;
			rHW_Color.color_cm_w3_sat_4.bits.COLOR_CM_S_W3_WGT_USLOPE = rMCMappingOut.S.WGT_USLOPE;
	        break;
		default :
			break;
    }

	// dump CM W1/W2/W3 setting
    if (bFDColorDebug == 1)
    {
        INTER_LOG_IF(bFDColorDebug, "[%s()] CTRL: BYPASS=%d, INK_DELTA=%d, INK_CH=%d\n", __FUNCTION__,
                            rHW_Color.color_cm_control.bits.COLOR_CM_BYPASS,
                            rHW_Color.color_cm_control.bits.COLOR_CM_INK_DELTA_MODE,
                            rHW_Color.color_cm_control.bits.COLOR_CM_INK_CH);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W1  : EN=%d, WGR_EN=%d, INK_EN=%d\n",
                            __FUNCTION__, rHW_Color.color_cm_control.bits.COLOR_CM_W1_EN,
                                          rHW_Color.color_cm_control.bits.COLOR_CM_W1_WGT_EN,
                                          rHW_Color.color_cm_control.bits.COLOR_CM_W1_INK_EN);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W1_Y: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, rHW_Color.color_cm_w1_luma_0.bits.COLOR_CM_Y_W1_L, rHW_Color.color_cm_w1_luma_0.bits.COLOR_CM_Y_W1_POINT0,
                             rHW_Color.color_cm_w1_luma_1.bits.COLOR_CM_Y_W1_POINT1, rHW_Color.color_cm_w1_luma_1.bits.COLOR_CM_Y_W1_POINT2,
                            rHW_Color.color_cm_w1_luma_1.bits.COLOR_CM_Y_W1_POINT3, rHW_Color.color_cm_w1_luma_2.bits.COLOR_CM_Y_W1_POINT4,
                            rHW_Color.color_cm_w1_luma_0.bits.COLOR_CM_Y_W1_U,
                            rHW_Color.color_cm_w1_luma_2.bits.COLOR_CM_Y_W1_SLOPE0, rHW_Color.color_cm_w1_luma_2.bits.COLOR_CM_Y_W1_SLOPE1,
                            rHW_Color.color_cm_w1_luma_3.bits.COLOR_CM_Y_W1_SLOPE2, rHW_Color.color_cm_w1_luma_3.bits.COLOR_CM_Y_W1_SLOPE3,
                            rHW_Color.color_cm_w1_luma_3.bits.COLOR_CM_Y_W1_SLOPE4, rHW_Color.color_cm_w1_luma_3.bits.COLOR_CM_Y_W1_SLOPE5,
                            rHW_Color.color_cm_w1_luma_4.bits.COLOR_CM_Y_W1_WGT_LSLOPE, rHW_Color.color_cm_w1_luma_4.bits.COLOR_CM_Y_W1_WGT_LSLOPE);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W1_H: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, rHW_Color.color_cm_w1_hue_0.bits.COLOR_CM_H_W1_L, rHW_Color.color_cm_w1_hue_0.bits.COLOR_CM_H_W1_POINT0,
                            rHW_Color.color_cm_w1_hue_1.bits.COLOR_CM_H_W1_POINT1, rHW_Color.color_cm_w1_hue_1.bits.COLOR_CM_H_W1_POINT2,
                            rHW_Color.color_cm_w1_hue_1.bits.COLOR_CM_H_W1_POINT3, rHW_Color.color_cm_w1_hue_2.bits.COLOR_CM_H_W1_POINT4,
                            rHW_Color.color_cm_w1_hue_0.bits.COLOR_CM_H_W1_U,
                            rHW_Color.color_cm_w1_hue_2.bits.COLOR_CM_H_W1_SLOPE0, rHW_Color.color_cm_w1_hue_2.bits.COLOR_CM_H_W1_SLOPE1,
                            rHW_Color.color_cm_w1_hue_3.bits.COLOR_CM_H_W1_SLOPE2, rHW_Color.color_cm_w1_hue_3.bits.COLOR_CM_H_W1_SLOPE3,
                            rHW_Color.color_cm_w1_hue_3.bits.COLOR_CM_H_W1_SLOPE4, rHW_Color.color_cm_w1_hue_3.bits.COLOR_CM_H_W1_SLOPE5,
                            rHW_Color.color_cm_w1_hue_4.bits.COLOR_CM_H_W1_WGT_LSLOPE, rHW_Color.color_cm_w1_hue_4.bits.COLOR_CM_H_W1_WGT_LSLOPE);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W1_S: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, rHW_Color.color_cm_w1_sat_0.bits.COLOR_CM_S_W1_L, rHW_Color.color_cm_w1_sat_0.bits.COLOR_CM_S_W1_POINT0,
                            rHW_Color.color_cm_w1_sat_1.bits.COLOR_CM_S_W1_POINT1, rHW_Color.color_cm_w1_sat_1.bits.COLOR_CM_S_W1_POINT2,
                            rHW_Color.color_cm_w1_sat_1.bits.COLOR_CM_S_W1_POINT3, rHW_Color.color_cm_w1_sat_2.bits.COLOR_CM_S_W1_POINT4,
                            rHW_Color.color_cm_w1_sat_0.bits.COLOR_CM_S_W1_U,
                            rHW_Color.color_cm_w1_sat_2.bits.COLOR_CM_S_W1_SLOPE0, rHW_Color.color_cm_w1_sat_2.bits.COLOR_CM_S_W1_SLOPE1,
                            rHW_Color.color_cm_w1_sat_3.bits.COLOR_CM_S_W1_SLOPE2, rHW_Color.color_cm_w1_sat_3.bits.COLOR_CM_S_W1_SLOPE3,
                            rHW_Color.color_cm_w1_sat_3.bits.COLOR_CM_S_W1_SLOPE4, rHW_Color.color_cm_w1_sat_3.bits.COLOR_CM_S_W1_SLOPE5,
                            rHW_Color.color_cm_w1_sat_4.bits.COLOR_CM_S_W1_WGT_LSLOPE, rHW_Color.color_cm_w1_sat_4.bits.COLOR_CM_S_W1_WGT_LSLOPE);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W2  : EN=%d, WGR_EN=%d, INK_EN=%d\n",
                            __FUNCTION__, rHW_Color.color_cm_control.bits.COLOR_CM_W2_EN,
                            rHW_Color.color_cm_control.bits.COLOR_CM_W2_WGT_EN,
                            rHW_Color.color_cm_control.bits.COLOR_CM_W2_INK_EN);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W2_Y: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, rHW_Color.color_cm_w2_luma_0.bits.COLOR_CM_Y_W2_L, rHW_Color.color_cm_w2_luma_0.bits.COLOR_CM_Y_W2_POINT0,
                            rHW_Color.color_cm_w2_luma_1.bits.COLOR_CM_Y_W2_POINT1, rHW_Color.color_cm_w2_luma_1.bits.COLOR_CM_Y_W2_POINT2,
                            rHW_Color.color_cm_w2_luma_1.bits.COLOR_CM_Y_W2_POINT3, rHW_Color.color_cm_w2_luma_2.bits.COLOR_CM_Y_W2_POINT4,
                            rHW_Color.color_cm_w2_luma_0.bits.COLOR_CM_Y_W2_U,
                            rHW_Color.color_cm_w2_luma_2.bits.COLOR_CM_Y_W2_SLOPE0, rHW_Color.color_cm_w2_luma_2.bits.COLOR_CM_Y_W2_SLOPE1,
                            rHW_Color.color_cm_w2_luma_3.bits.COLOR_CM_Y_W2_SLOPE2, rHW_Color.color_cm_w2_luma_3.bits.COLOR_CM_Y_W2_SLOPE3,
                            rHW_Color.color_cm_w2_luma_3.bits.COLOR_CM_Y_W2_SLOPE4, rHW_Color.color_cm_w2_luma_3.bits.COLOR_CM_Y_W2_SLOPE5,
                            rHW_Color.color_cm_w2_luma_4.bits.COLOR_CM_Y_W2_WGT_LSLOPE, rHW_Color.color_cm_w2_luma_4.bits.COLOR_CM_Y_W2_WGT_LSLOPE);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W2_H: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, rHW_Color.color_cm_w2_hue_0.bits.COLOR_CM_H_W2_L, rHW_Color.color_cm_w2_hue_0.bits.COLOR_CM_H_W2_POINT0,
                            rHW_Color.color_cm_w2_hue_1.bits.COLOR_CM_H_W2_POINT1, rHW_Color.color_cm_w2_hue_1.bits.COLOR_CM_H_W2_POINT2,
                            rHW_Color.color_cm_w2_hue_1.bits.COLOR_CM_H_W2_POINT3, rHW_Color.color_cm_w2_hue_2.bits.COLOR_CM_H_W2_POINT4,
                            rHW_Color.color_cm_w2_hue_0.bits.COLOR_CM_H_W2_U,
                            rHW_Color.color_cm_w2_hue_2.bits.COLOR_CM_H_W2_SLOPE0, rHW_Color.color_cm_w2_hue_2.bits.COLOR_CM_H_W2_SLOPE1,
                            rHW_Color.color_cm_w2_hue_3.bits.COLOR_CM_H_W2_SLOPE2, rHW_Color.color_cm_w2_hue_3.bits.COLOR_CM_H_W2_SLOPE3,
                            rHW_Color.color_cm_w2_hue_3.bits.COLOR_CM_H_W2_SLOPE4, rHW_Color.color_cm_w2_hue_3.bits.COLOR_CM_H_W2_SLOPE5,
                            rHW_Color.color_cm_w2_hue_4.bits.COLOR_CM_H_W2_WGT_LSLOPE, rHW_Color.color_cm_w2_hue_4.bits.COLOR_CM_H_W2_WGT_LSLOPE);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W2_S: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, rHW_Color.color_cm_w2_sat_0.bits.COLOR_CM_S_W2_L, rHW_Color.color_cm_w2_sat_0.bits.COLOR_CM_S_W2_POINT0,
                            rHW_Color.color_cm_w2_sat_1.bits.COLOR_CM_S_W2_POINT1, rHW_Color.color_cm_w2_sat_1.bits.COLOR_CM_S_W2_POINT2,
                            rHW_Color.color_cm_w2_sat_1.bits.COLOR_CM_S_W2_POINT3, rHW_Color.color_cm_w2_sat_2.bits.COLOR_CM_S_W2_POINT4,
                            rHW_Color.color_cm_w2_sat_0.bits.COLOR_CM_S_W2_U,
                            rHW_Color.color_cm_w2_sat_2.bits.COLOR_CM_S_W2_SLOPE0, rHW_Color.color_cm_w2_sat_2.bits.COLOR_CM_S_W2_SLOPE1,
                            rHW_Color.color_cm_w2_sat_3.bits.COLOR_CM_S_W2_SLOPE2, rHW_Color.color_cm_w2_sat_3.bits.COLOR_CM_S_W2_SLOPE3,
                            rHW_Color.color_cm_w2_sat_3.bits.COLOR_CM_S_W2_SLOPE4, rHW_Color.color_cm_w2_sat_3.bits.COLOR_CM_S_W2_SLOPE5,
                            rHW_Color.color_cm_w2_sat_4.bits.COLOR_CM_S_W2_WGT_LSLOPE, rHW_Color.color_cm_w2_sat_4.bits.COLOR_CM_S_W2_WGT_LSLOPE);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W3  : EN=%d, WGR_EN=%d, INK_EN=%d\n",
                            __FUNCTION__, rHW_Color.color_cm_control.bits.COLOR_CM_W3_EN,
                            rHW_Color.color_cm_control.bits.COLOR_CM_W3_WGT_EN,
                            rHW_Color.color_cm_control.bits.COLOR_CM_W3_INK_EN);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W3_Y: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, rHW_Color.color_cm_w3_luma_0.bits.COLOR_CM_Y_W3_L, rHW_Color.color_cm_w3_luma_0.bits.COLOR_CM_Y_W3_POINT0,
                            rHW_Color.color_cm_w3_luma_1.bits.COLOR_CM_Y_W3_POINT1, rHW_Color.color_cm_w3_luma_1.bits.COLOR_CM_Y_W3_POINT2,
                            rHW_Color.color_cm_w3_luma_1.bits.COLOR_CM_Y_W3_POINT3, rHW_Color.color_cm_w3_luma_2.bits.COLOR_CM_Y_W3_POINT4,
                            rHW_Color.color_cm_w3_luma_0.bits.COLOR_CM_Y_W3_U,
                            rHW_Color.color_cm_w3_luma_2.bits.COLOR_CM_Y_W3_SLOPE0, rHW_Color.color_cm_w3_luma_2.bits.COLOR_CM_Y_W3_SLOPE1,
                            rHW_Color.color_cm_w3_luma_3.bits.COLOR_CM_Y_W3_SLOPE2, rHW_Color.color_cm_w3_luma_3.bits.COLOR_CM_Y_W3_SLOPE3,
                            rHW_Color.color_cm_w3_luma_3.bits.COLOR_CM_Y_W3_SLOPE4, rHW_Color.color_cm_w3_luma_3.bits.COLOR_CM_Y_W3_SLOPE5,
                            rHW_Color.color_cm_w3_luma_4.bits.COLOR_CM_Y_W3_WGT_LSLOPE, rHW_Color.color_cm_w3_luma_4.bits.COLOR_CM_Y_W3_WGT_LSLOPE);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W3_H: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, rHW_Color.color_cm_w3_hue_0.bits.COLOR_CM_H_W3_L, rHW_Color.color_cm_w3_hue_0.bits.COLOR_CM_H_W3_POINT0,
                            rHW_Color.color_cm_w3_hue_1.bits.COLOR_CM_H_W3_POINT1, rHW_Color.color_cm_w3_hue_1.bits.COLOR_CM_H_W3_POINT2,
                            rHW_Color.color_cm_w3_hue_1.bits.COLOR_CM_H_W3_POINT3, rHW_Color.color_cm_w3_hue_2.bits.COLOR_CM_H_W3_POINT4,
                            rHW_Color.color_cm_w3_hue_0.bits.COLOR_CM_H_W3_U,
                            rHW_Color.color_cm_w3_hue_2.bits.COLOR_CM_H_W3_SLOPE0, rHW_Color.color_cm_w3_hue_2.bits.COLOR_CM_H_W3_SLOPE1,
                            rHW_Color.color_cm_w3_hue_3.bits.COLOR_CM_H_W3_SLOPE2, rHW_Color.color_cm_w3_hue_3.bits.COLOR_CM_H_W3_SLOPE3,
                            rHW_Color.color_cm_w3_hue_3.bits.COLOR_CM_H_W3_SLOPE4, rHW_Color.color_cm_w3_hue_3.bits.COLOR_CM_H_W3_SLOPE5,
                            rHW_Color.color_cm_w3_hue_4.bits.COLOR_CM_H_W3_WGT_LSLOPE, rHW_Color.color_cm_w3_hue_4.bits.COLOR_CM_H_W3_WGT_LSLOPE);
        INTER_LOG_IF(bFDColorDebug, "[%s()] W3_S: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, rHW_Color.color_cm_w3_sat_0.bits.COLOR_CM_S_W3_L, rHW_Color.color_cm_w3_sat_0.bits.COLOR_CM_S_W3_POINT0,
                            rHW_Color.color_cm_w3_sat_1.bits.COLOR_CM_S_W3_POINT1, rHW_Color.color_cm_w3_sat_1.bits.COLOR_CM_S_W3_POINT2,
                            rHW_Color.color_cm_w3_sat_1.bits.COLOR_CM_S_W3_POINT3, rHW_Color.color_cm_w3_sat_2.bits.COLOR_CM_S_W3_POINT4,
                            rHW_Color.color_cm_w3_sat_0.bits.COLOR_CM_S_W3_U,
                            rHW_Color.color_cm_w3_sat_2.bits.COLOR_CM_S_W3_SLOPE0, rHW_Color.color_cm_w3_sat_2.bits.COLOR_CM_S_W3_SLOPE1,
                            rHW_Color.color_cm_w3_sat_3.bits.COLOR_CM_S_W3_SLOPE2, rHW_Color.color_cm_w3_sat_3.bits.COLOR_CM_S_W3_SLOPE3,
                            rHW_Color.color_cm_w3_sat_3.bits.COLOR_CM_S_W3_SLOPE4, rHW_Color.color_cm_w3_sat_3.bits.COLOR_CM_S_W3_SLOPE5,
                            rHW_Color.color_cm_w3_sat_4.bits.COLOR_CM_S_W3_WGT_LSLOPE, rHW_Color.color_cm_w3_sat_4.bits.COLOR_CM_S_W3_WGT_LSLOPE);
    }
			
			
}

MVOID InterParam_FD_Color(double Ratio_L_L, double Ratio_L_U, double Ratio_U_L, double Ratio_U_U,
	                      ISP_NVRAM_FD_COLOR_PARAM_T FD_Para_L_L, ISP_NVRAM_FD_COLOR_PARAM_T FD_Para_L_U, ISP_NVRAM_FD_COLOR_PARAM_T FD_Para_U_L, ISP_NVRAM_FD_COLOR_PARAM_T FD_Para_U_U,
                          ISP_NVRAM_FD_COLOR_PARAM_T& FD_Para)
{
	FD_Para.u4FD_CM_Enable = FD_Para_L_L.u4FD_CM_Enable * Ratio_L_L + FD_Para_L_U.u4FD_CM_Enable * Ratio_L_U + FD_Para_U_L.u4FD_CM_Enable * Ratio_U_L + FD_Para_U_U.u4FD_CM_Enable * Ratio_U_U + 0.5;
	FD_Para.u4Priority = FD_Para_L_L.u4Priority * Ratio_L_L + FD_Para_L_U.u4Priority * Ratio_L_U + FD_Para_U_L.u4Priority * Ratio_U_L + FD_Para_U_U.u4Priority * Ratio_U_U + 0.5;
	
	FD_Para.u4Range_W1_Y = FD_Para_L_L.u4Range_W1_Y * Ratio_L_L + FD_Para_L_U.u4Range_W1_Y * Ratio_L_U + FD_Para_U_L.u4Range_W1_Y * Ratio_U_L + FD_Para_U_U.u4Range_W1_Y * Ratio_U_U + 0.5;
	FD_Para.u4Range_W1_S = FD_Para_L_L.u4Range_W1_S * Ratio_L_L + FD_Para_L_U.u4Range_W1_S * Ratio_L_U + FD_Para_U_L.u4Range_W1_S * Ratio_U_L + FD_Para_U_U.u4Range_W1_S * Ratio_U_U + 0.5;
	FD_Para.u4Range_W1_H = FD_Para_L_L.u4Range_W1_H * Ratio_L_L + FD_Para_L_U.u4Range_W1_H * Ratio_L_U + FD_Para_U_L.u4Range_W1_H * Ratio_U_L + FD_Para_U_U.u4Range_W1_H * Ratio_U_U + 0.5;
	FD_Para.u4Offset_W1_Y = FD_Para_L_L.u4Offset_W1_Y * Ratio_L_L + FD_Para_L_U.u4Offset_W1_Y * Ratio_L_U + FD_Para_U_L.u4Offset_W1_Y * Ratio_U_L + FD_Para_U_U.u4Offset_W1_Y * Ratio_U_U + 0.5;
	FD_Para.u4Offset_W1_S = FD_Para_L_L.u4Offset_W1_S * Ratio_L_L + FD_Para_L_U.u4Offset_W1_S * Ratio_L_U + FD_Para_U_L.u4Offset_W1_S * Ratio_U_L + FD_Para_U_U.u4Offset_W1_S * Ratio_U_U + 0.5;
	FD_Para.u4Offset_W1_H = FD_Para_L_L.u4Offset_W1_H * Ratio_L_L + FD_Para_L_U.u4Offset_W1_H * Ratio_L_U + FD_Para_U_L.u4Offset_W1_H * Ratio_U_L + FD_Para_U_U.u4Offset_W1_H * Ratio_U_U + 0.5;

	FD_Para.u4Range_W2_Y = FD_Para_L_L.u4Range_W2_Y * Ratio_L_L + FD_Para_L_U.u4Range_W2_Y * Ratio_L_U + FD_Para_U_L.u4Range_W2_Y * Ratio_U_L + FD_Para_U_U.u4Range_W2_Y * Ratio_U_U + 0.5;
	FD_Para.u4Range_W2_S = FD_Para_L_L.u4Range_W2_S * Ratio_L_L + FD_Para_L_U.u4Range_W2_S * Ratio_L_U + FD_Para_U_L.u4Range_W2_S * Ratio_U_L + FD_Para_U_U.u4Range_W2_S * Ratio_U_U + 0.5;
	FD_Para.u4Range_W2_H = FD_Para_L_L.u4Range_W2_H * Ratio_L_L + FD_Para_L_U.u4Range_W2_H * Ratio_L_U + FD_Para_U_L.u4Range_W2_H * Ratio_U_L + FD_Para_U_U.u4Range_W2_H * Ratio_U_U + 0.5;
	FD_Para.u4Offset_W2_Y = FD_Para_L_L.u4Offset_W2_Y * Ratio_L_L + FD_Para_L_U.u4Offset_W2_Y * Ratio_L_U + FD_Para_U_L.u4Offset_W2_Y * Ratio_U_L + FD_Para_U_U.u4Offset_W2_Y * Ratio_U_U + 0.5;
	FD_Para.u4Offset_W2_S = FD_Para_L_L.u4Offset_W2_S * Ratio_L_L + FD_Para_L_U.u4Offset_W2_S * Ratio_L_U + FD_Para_U_L.u4Offset_W2_S * Ratio_U_L + FD_Para_U_U.u4Offset_W2_S * Ratio_U_U + 0.5;
	FD_Para.u4Offset_W2_H = FD_Para_L_L.u4Offset_W2_H * Ratio_L_L + FD_Para_L_U.u4Offset_W2_H * Ratio_L_U + FD_Para_U_L.u4Offset_W2_H * Ratio_U_L + FD_Para_U_U.u4Offset_W2_H * Ratio_U_U + 0.5;

	FD_Para.u4Range_W3_Y = FD_Para_L_L.u4Range_W3_Y * Ratio_L_L + FD_Para_L_U.u4Range_W3_Y * Ratio_L_U + FD_Para_U_L.u4Range_W3_Y * Ratio_U_L + FD_Para_U_U.u4Range_W3_Y * Ratio_U_U + 0.5;
	FD_Para.u4Range_W3_S = FD_Para_L_L.u4Range_W3_S * Ratio_L_L + FD_Para_L_U.u4Range_W3_S * Ratio_L_U + FD_Para_U_L.u4Range_W3_S * Ratio_U_L + FD_Para_U_U.u4Range_W3_S * Ratio_U_U + 0.5;
	FD_Para.u4Range_W3_H = FD_Para_L_L.u4Range_W3_H * Ratio_L_L + FD_Para_L_U.u4Range_W3_H * Ratio_L_U + FD_Para_U_L.u4Range_W3_H * Ratio_U_L + FD_Para_U_U.u4Range_W3_H * Ratio_U_U + 0.5;
	FD_Para.u4Offset_W3_Y = FD_Para_L_L.u4Offset_W3_Y * Ratio_L_L + FD_Para_L_U.u4Offset_W3_Y * Ratio_L_U + FD_Para_U_L.u4Offset_W3_Y * Ratio_U_L + FD_Para_U_U.u4Offset_W3_Y * Ratio_U_U + 0.5;
	FD_Para.u4Offset_W3_S = FD_Para_L_L.u4Offset_W3_S * Ratio_L_L + FD_Para_L_U.u4Offset_W3_S * Ratio_L_U + FD_Para_U_L.u4Offset_W3_S * Ratio_U_L + FD_Para_U_U.u4Offset_W3_S * Ratio_U_U + 0.5;
	FD_Para.u4Offset_W3_H = FD_Para_L_L.u4Offset_W3_H * Ratio_L_L + FD_Para_L_U.u4Offset_W3_H * Ratio_L_U + FD_Para_U_L.u4Offset_W3_H * Ratio_U_L + FD_Para_U_U.u4Offset_W3_H * Ratio_U_U + 0.5;
	
}

MVOID FD_Color(RAWIspCamInfo const& rCamInfo,
                  ISP_NVRAM_FD_COLOR_PARAM_T const (& FD_COLOR)[ISP_NVRAM_COLOR_SCENE_NUM][ISP_NVRAM_COLOR_CT_NUM],
                  CAMERA_TUNING_FD_INFO_T const& rFdInfo,
                  ISP_NVRAM_COLOR_T& rFdColor)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.FD_Color.enable", value, "0");
    MBOOL bFDColorDebug = atoi(value);
	

	MINT32 u4FaceNum = rFdInfo.FaceNum;

	INTER_LOG_IF(bFDColorDebug,"[%s()] u4FaceNum: %d\n", __FUNCTION__, u4FaceNum);

	if(u4FaceNum == 0)
	{return;}

	for(int i = 0; i < u4FaceNum; i++)
	{
	INTER_LOG_IF(bFDColorDebug,"[%s()] faceIdx : %d, Ymean: %d, Umean: %d, Vmean: %d, Y5: %d, Y95: %d\n", __FUNCTION__, i, rFdInfo.YUVsts[i][0], rFdInfo.YUVsts[i][1], rFdInfo.YUVsts[i][2], rFdInfo.YUVsts[i][3], rFdInfo.YUVsts[i][4] );
	}
    static MemoryColorMappingInput_T MCMappingIn;
	static MemoryColorMappingOutput_T MCMappingOut;
    static ISP_NVRAM_FD_COLOR_PARAM_T FD_Para_L_L;
	static ISP_NVRAM_FD_COLOR_PARAM_T FD_Para_L_U;
	static ISP_NVRAM_FD_COLOR_PARAM_T FD_Para_U_L;
	static ISP_NVRAM_FD_COLOR_PARAM_T FD_Para_U_U;
	static ISP_NVRAM_FD_COLOR_PARAM_T FD_Para;
	static FaceInput_T FaceInput;

	MINT32 Win_Idx = 1;

	MUINT32 CT_00 = 5000;
	MUINT32 CT_01 = 4000;
	MUINT32 CT_02 = 3000;


	MUINT32 LV_00 = 120;
	MUINT32 LV_01 = 100;
	MUINT32 LV_02 = 50;


	MUINT32 u4RealCT = rCamInfo.rAWBInfo.i4CCT;
    MUINT32 u4RealLV = rCamInfo.rAEInfo.i4LightValue_x10;
	
	MUINT32 u4UpperCT;
	MUINT32 u4LowerCT;
	MUINT32 u4UpperLV;
	MUINT32 u4LowerLV;

	MUINT32 u4UpperCT_Idx;
	MUINT32 u4LowerCT_Idx;
	MUINT32 u4UpperLV_Idx;
	MUINT32 u4LowerLV_Idx;

	if(u4RealCT >= CT_01)
	{
	    u4UpperCT = CT_00;
		u4LowerCT = CT_01;
		u4UpperCT_Idx = 0;
		u4LowerCT_Idx = 1;
	}
	else
	{
	    u4UpperCT = CT_01;
		u4LowerCT = CT_02;
		u4UpperCT_Idx = 1;
		u4LowerCT_Idx = 2;
	}
	

	if(u4RealLV >= LV_01)
	{
	    u4UpperLV = LV_00;
		u4LowerLV = LV_01;
		u4UpperLV_Idx = 0;
		u4LowerLV_Idx = 1;
	}
	else
	{
	    u4UpperLV = LV_01;
		u4LowerLV = LV_02;
		u4UpperLV_Idx = 1;
		u4LowerLV_Idx = 2;
	}

	double RATIO_CT_lo = 0;
	double RATIO_CT_up = 0;
	double RATIO_LV_lo = 0;
	double RATIO_LV_up = 0;

	//protection for out-of-bound


    if (u4RealCT <= u4LowerCT)
    {
        RATIO_CT_lo = 1;
    }
	else if(u4RealCT >= u4UpperCT)
	{
		RATIO_CT_lo = 0;
	}
	else
	{
		RATIO_CT_lo = (double)(u4UpperCT - u4RealCT) / (double)(u4UpperCT - u4LowerCT);
	}

	RATIO_CT_up = 1.0 - RATIO_CT_lo;

	if (u4RealLV <= u4LowerLV)
    {
        RATIO_LV_lo = 1;
    }
	else if(u4RealLV >= u4UpperLV)
	{
		RATIO_LV_lo = 0;
	}
	else
	{
		RATIO_LV_lo = (double)(u4UpperLV - u4RealLV) / (double)(u4UpperLV - u4LowerLV);
	}

	RATIO_LV_up = 1.0 - RATIO_LV_lo;

	double RATIO_L_L = (double)(RATIO_LV_lo * RATIO_CT_lo);
	double RATIO_L_U = (double)(RATIO_LV_lo * RATIO_CT_up);
	double RATIO_U_L = (double)(RATIO_LV_up * RATIO_CT_lo);
	double RATIO_U_U = (double)(RATIO_LV_up * RATIO_CT_up);

	INTER_LOG_IF(bFDColorDebug,"[%s()] u4RealCT: %d, u4UpperCT = %d, u4LowerCT = %d\n", __FUNCTION__, u4RealCT, u4UpperCT, u4LowerCT);
	INTER_LOG_IF(bFDColorDebug,"[%s()] u4RealLV: %d, u4UpperLV = %d, u4LowerLV = %d\n", __FUNCTION__, u4RealLV, u4UpperLV, u4LowerLV);
	INTER_LOG_IF(bFDColorDebug,"[%s()] u4UpperCT_Idx: %d, u4LowerCT_Idx = %d, u4UpperLV_Idx = %d, u4LowerLV_Idx = %d\n", __FUNCTION__, u4UpperCT_Idx, u4LowerCT_Idx, u4UpperLV_Idx, u4LowerLV_Idx);
	INTER_LOG_IF(bFDColorDebug,"[%s()] RATIO_L_L: %f, RATIO_L_U = %f, RATIO_U_L = %f, RATIO_U_U = %f\n", __FUNCTION__, RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U);
		

	memcpy(&FD_Para_L_L, &FD_COLOR[u4LowerLV_Idx][u4LowerCT_Idx], sizeof(ISP_NVRAM_FD_COLOR_PARAM_T));
	memcpy(&FD_Para_L_U, &FD_COLOR[u4LowerLV_Idx][u4UpperCT_Idx], sizeof(ISP_NVRAM_FD_COLOR_PARAM_T));
	memcpy(&FD_Para_U_L, &FD_COLOR[u4UpperLV_Idx][u4LowerCT_Idx], sizeof(ISP_NVRAM_FD_COLOR_PARAM_T));
	memcpy(&FD_Para_U_U, &FD_COLOR[u4UpperLV_Idx][u4UpperCT_Idx], sizeof(ISP_NVRAM_FD_COLOR_PARAM_T));

	InterParam_FD_Color(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, FD_Para_L_L, FD_Para_L_U, FD_Para_U_L, FD_Para_U_U, FD_Para);

	if(FD_Para.u4FD_CM_Enable == 0)
	{return;}

    INTER_LOG_IF(bFDColorDebug,"[%s()] FD_Para: u4FD_CM_Enable: %d, u4Range_W1_Y: %d, u4Range_W1_S: %d, u4Range_W1_H: %d, u4Offset_W1_Y: %d, u4Offset_W1_S: %d, u4Offset_W1_H: %d\n", 
		                        __FUNCTION__, FD_Para.u4FD_CM_Enable, 
		                                      FD_Para.u4Range_W1_Y, FD_Para.u4Range_W1_S, FD_Para.u4Range_W1_H,
		                                      FD_Para.u4Offset_W1_Y, FD_Para.u4Offset_W1_S, FD_Para.u4Offset_W1_H);

	
    FD2FaceIn(rFdInfo, FaceInput);
    INTER_LOG_IF(bFDColorDebug,"[%s()]  Race: %d, YCenter: %d, SCenter: %d, HCenter: %d, \n", __FUNCTION__, FaceInput.Race, FaceInput.Y_Center, FaceInput.S_Center, FaceInput.H_Center);
    if((FaceInput.H_Center < 90) || (FaceInput.H_Center > 160))
	{return;}
	
	Face2MCPara(FaceInput, FD_Para, MCMappingIn);
	MC_Mapping(MCMappingIn, MCMappingOut);
	MC_UpdateHWReg(Win_Idx, MCMappingOut, rFdColor);
	
    INTER_LOG_IF(bFDColorDebug,"[%s()] Luma_Start: %d, Luma_End: %d, Luma_Offset: %d, Sat_Start: %d, Sat_End: %d, Sat_Offset: %d, Hue_Start: %d, Hue_End: %d, Hue_Offset: %d\n", 
		                        __FUNCTION__, MCMappingIn.Luma_Start, MCMappingIn.Luma_End, MCMappingIn.Luma_Offset, 
		                                      MCMappingIn.Sat_Start, MCMappingIn.Sat_End, MCMappingIn.Sat_Offset,
		                                      MCMappingIn.Hue_Start, MCMappingIn.Hue_End, MCMappingIn.Hue_Offset);

	INTER_LOG_IF(bFDColorDebug, "[%s()] MC_out_Y: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, MCMappingOut.Y.L, MCMappingOut.Y.POINT0, MCMappingOut.Y.POINT1, MCMappingOut.Y.POINT2, MCMappingOut.Y.POINT3, MCMappingOut.Y.POINT4, MCMappingOut.Y.U,
                             MCMappingOut.Y.SLOPE0, MCMappingOut.Y.SLOPE1, MCMappingOut.Y.SLOPE2, MCMappingOut.Y.SLOPE3, MCMappingOut.Y.SLOPE4, MCMappingOut.Y.SLOPE5, MCMappingOut.Y.WGT_LSLOPE, MCMappingOut.Y.WGT_USLOPE);
	INTER_LOG_IF(bFDColorDebug, "[%s()] MC_out_S: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, MCMappingOut.S.L, MCMappingOut.S.POINT0, MCMappingOut.S.POINT1, MCMappingOut.S.POINT2, MCMappingOut.S.POINT3, MCMappingOut.S.POINT4, MCMappingOut.S.U,
                             MCMappingOut.S.SLOPE0, MCMappingOut.S.SLOPE1, MCMappingOut.S.SLOPE2, MCMappingOut.S.SLOPE3, MCMappingOut.S.SLOPE4, MCMappingOut.S.SLOPE5, MCMappingOut.S.WGT_LSLOPE, MCMappingOut.S.WGT_USLOPE);
	INTER_LOG_IF(bFDColorDebug, "[%s()] MC_out_H: PL=%04d, P0=%04d, P1=%04d, P2=%04d, P3=%04d, P4=%04d, PU=%04d, S0=%04d, S1=%04d, S2=%04d, S3=%04d, S4=%04d, S5=%04d, WGTL=%04d, WGTU=%04d\n",
                            __FUNCTION__, MCMappingOut.H.L, MCMappingOut.H.POINT0, MCMappingOut.H.POINT1, MCMappingOut.H.POINT2, MCMappingOut.H.POINT3, MCMappingOut.H.POINT4, MCMappingOut.H.U,
                             MCMappingOut.H.SLOPE0, MCMappingOut.H.SLOPE1, MCMappingOut.H.SLOPE2, MCMappingOut.H.SLOPE3, MCMappingOut.H.SLOPE4, MCMappingOut.H.SLOPE5, MCMappingOut.H.WGT_LSLOPE, MCMappingOut.H.WGT_USLOPE);
	

	

}

MVOID ANRTuningGen(CAMERA_TUNING_FD_INFO_T const& rFdInfo,
	                    ISP_NVRAM_FD_ANR_PARAM_T const& FD_ANR,
                          ANRTblTuning_T ANR_Tuning[5])
{
    for(int i = 0; i < rFdInfo.FaceNum; i++)
    {
        ANR_Tuning[i].Y_Mean = rFdInfo.YUVsts[i][0];
		ANR_Tuning[i].U_Mean = rFdInfo.YUVsts[i][1];
		ANR_Tuning[i].V_Mean = rFdInfo.YUVsts[i][2];


		switch(i)
        {
		    case 0 :
				ANR_Tuning[i].Y_Range = FD_ANR.u4Range_W1_Y;
				ANR_Tuning[i].U_Range = FD_ANR.u4Range_W1_U;
				ANR_Tuning[i].V_Range = FD_ANR.u4Range_W1_V;
				ANR_Tuning[i].Y_Gain = FD_ANR.u4Gain_W1_Y / 10;
				ANR_Tuning[i].U_Gain = FD_ANR.u4Gain_W1_U / 10;
				ANR_Tuning[i].V_Gain = FD_ANR.u4Gain_W1_V / 10;
			    break;
			case 1 :
				ANR_Tuning[i].Y_Range = FD_ANR.u4Range_W2_Y;
				ANR_Tuning[i].U_Range = FD_ANR.u4Range_W2_U;
				ANR_Tuning[i].V_Range = FD_ANR.u4Range_W2_V;
				ANR_Tuning[i].Y_Gain = FD_ANR.u4Gain_W2_Y / 10;
				ANR_Tuning[i].U_Gain = FD_ANR.u4Gain_W2_U / 10;
				ANR_Tuning[i].V_Gain = FD_ANR.u4Gain_W2_V / 10;
			    break;
			case 2 :
				ANR_Tuning[i].Y_Range = FD_ANR.u4Range_W3_Y;
				ANR_Tuning[i].U_Range = FD_ANR.u4Range_W3_U;
				ANR_Tuning[i].V_Range = FD_ANR.u4Range_W3_V;
				ANR_Tuning[i].Y_Gain = FD_ANR.u4Gain_W3_Y / 10;
				ANR_Tuning[i].U_Gain = FD_ANR.u4Gain_W3_U / 10;
				ANR_Tuning[i].V_Gain = FD_ANR.u4Gain_W3_V / 10;
			    break;
			case 3 :
				ANR_Tuning[i].Y_Range = FD_ANR.u4Range_W4_Y;
				ANR_Tuning[i].U_Range = FD_ANR.u4Range_W4_U;
				ANR_Tuning[i].V_Range = FD_ANR.u4Range_W4_V;
				ANR_Tuning[i].Y_Gain = FD_ANR.u4Gain_W4_Y / 10;
				ANR_Tuning[i].U_Gain = FD_ANR.u4Gain_W4_U / 10;
				ANR_Tuning[i].V_Gain = FD_ANR.u4Gain_W4_V / 10;
			    break;
			case 4 :
				ANR_Tuning[i].Y_Range = FD_ANR.u4Range_W5_Y;
				ANR_Tuning[i].U_Range = FD_ANR.u4Range_W5_U;
				ANR_Tuning[i].V_Range = FD_ANR.u4Range_W5_V;
				ANR_Tuning[i].Y_Gain = FD_ANR.u4Gain_W5_Y / 10;
				ANR_Tuning[i].U_Gain = FD_ANR.u4Gain_W5_U / 10;
				ANR_Tuning[i].V_Gain = FD_ANR.u4Gain_W5_V / 10;
			    break;
			default : 
				break;
        }
    }

	
}

MVOID ANRTBL_YUV8x8x8(ISP_NVRAM_ANR_LUT_T Anr_Tbl,
                          ANRTbl8x8x8_T& ANR8x8x8)
{
    for(int i = 0; i < 8; i++)
	{   
	    
		for(int j = 0; j < 8; j++)
			{
			    
				for(int k = 0; k < 4; k++)
					{
					    ANR8x8x8.Y[i][j][k*2] =  Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_Y0;
					    ANR8x8x8.U[i][j][k*2] =  Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_U0;
						ANR8x8x8.V[i][j][k*2] =  Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_V0;
						ANR8x8x8.Y[i][j][k*2+1] =  Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_Y1;
					    ANR8x8x8.U[i][j][k*2+1] =  Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_U1;
						ANR8x8x8.V[i][j][k*2+1] =  Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_V1;
		
					}
			}
	}
}

MVOID YUV8x8x8_ANRTBL(ANRTbl8x8x8_T ANR8x8x8,
                          ISP_NVRAM_ANR_LUT_T& Anr_Tbl)
{
    for(int i = 0; i < 8; i++)
	{   
	    
		for(int j = 0; j < 8; j++)
			{
			    
				for(int k = 0; k < 4; k++)
					{
					    Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_Y0 = ANR8x8x8.Y[i][j][k*2];
					    Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_U0 = ANR8x8x8.U[i][j][k*2];
						Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_V0 = ANR8x8x8.V[i][j][k*2];
						Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_Y1 = ANR8x8x8.Y[i][j][k*2+1];
					    Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_U1 = ANR8x8x8.U[i][j][k*2+1];
						Anr_Tbl.lut[i*32 + j*4 + k].bits.ANR_TBL_V1 = ANR8x8x8.V[i][j][k*2+1];
		
					}
			}
	}
}


MVOID ANRTablePBC(MINT32 FaceNum, MINT32 Precision,
	                      ANRTblTuning_T ANR_Tuning[5],
                          ANRTbl8x8x8_T& ANR8x8x8)
{
    float y_range;
	float u_range;
	float v_range;
	float y_range2;
	float u_range2;
	float v_range2;
	float y_step = 32;
	float uv_step = (Precision) ? 16 : 32;
	float uv_start = (Precision) ? 64 : 0;

	float Hue;

	for (int i = 0; i < FaceNum; i++)
    {
        //apply on skin color only
        Hue =  ((MINT32)((atan2((double)(ANR_Tuning[i].V_Mean-128),(double)(ANR_Tuning[i].U_Mean-128)) * 180 / PI)+360)%360);

		if((Hue < 90) || (Hue > 160))
		{continue;}

		y_range = ANR_Tuning[i].Y_Range;
		u_range = ANR_Tuning[i].U_Range;
		v_range = ANR_Tuning[i].V_Range;

		y_range2 = y_range * y_range * 2;
		u_range2 = u_range * u_range * 2;
		v_range2 = v_range * v_range * 2;

		for (int y_idx = 0; y_idx < 8; y_idx++)
        {
            for (int u_idx = 0; u_idx < 8; u_idx++)
            {
                for (int v_idx = 0; v_idx < 8; v_idx++)
                {
                    float y = y_step / 2 + y_idx * y_step;
                    float u = uv_start + uv_step / 2 + u_idx * uv_step;
                    float v = uv_start + uv_step / 2 + v_idx * uv_step;
                    float diff_y = ANR_Tuning[i].Y_Mean- y;
                    float diff_u = ANR_Tuning[i].U_Mean- u;
                    float diff_v = ANR_Tuning[i].V_Mean- v;
                    //float diff_sum = diff_r * diff_r + diff_g * diff_g + diff_b * diff_b;
                    float weight_y = exp(-(diff_y * diff_y) / y_range2);
                    float weight_u = exp(-(diff_u * diff_u) / u_range2);
                    float weight_v = exp(-(diff_v * diff_v) / v_range2);
                    float weight = weight_y * weight_u * weight_v;

                    float ori_y_std = ANR8x8x8.Y[y_idx][u_idx][v_idx];
                    float ori_u_std = ANR8x8x8.U[y_idx][u_idx][v_idx];
                    float ori_v_std = ANR8x8x8.V[y_idx][u_idx][v_idx];

                    float new_y_std = ori_y_std * (1 - weight) + ori_y_std * ANR_Tuning[i].Y_Gain* weight + 0.5;
                    float new_u_std = ori_u_std * (1 - weight) + ori_u_std * ANR_Tuning[i].U_Gain* weight + 0.5;
                    float new_v_std = ori_v_std * (1 - weight) + ori_v_std * ANR_Tuning[i].V_Gain* weight + 0.5;

					new_y_std = Clip(new_y_std, 31, 0);
					new_u_std = Clip(new_u_std, 31, 0);
					new_v_std = Clip(new_v_std, 31, 0);

                    ANR8x8x8.Y[y_idx][u_idx][v_idx] = new_y_std;
                    ANR8x8x8.U[y_idx][u_idx][v_idx] = new_u_std;
                    ANR8x8x8.V[y_idx][u_idx][v_idx] = new_v_std;
                }
            }
		}
    }
}

MVOID InterParam_FD_ANR_TBL(double Ratio_Up, double Ratio_Lo,
	                      ISP_NVRAM_FD_ANR_PARAM_T FD_Para_Up, ISP_NVRAM_FD_ANR_PARAM_T FD_Para_Lo,
                          ISP_NVRAM_FD_ANR_PARAM_T& FD_Para)
{


	FD_Para.u4FD_ANR_LUT_Enable = FD_Para_Up.u4FD_ANR_LUT_Enable * Ratio_Up + FD_Para_Lo.u4FD_ANR_LUT_Enable * Ratio_Lo + 0.5;
	FD_Para.u4Range_W1_Y = FD_Para_Up.u4Range_W1_Y * Ratio_Up + FD_Para_Lo.u4Range_W1_Y * Ratio_Lo + 0.5;
	FD_Para.u4Range_W1_U = FD_Para_Up.u4Range_W1_U * Ratio_Up + FD_Para_Lo.u4Range_W1_U * Ratio_Lo + 0.5;
	FD_Para.u4Range_W1_V = FD_Para_Up.u4Range_W1_V * Ratio_Up + FD_Para_Lo.u4Range_W1_V * Ratio_Lo + 0.5;
	FD_Para.u4W1_enable = FD_Para_Up.u4W1_enable * Ratio_Up + FD_Para_Lo.u4W1_enable * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W1_Y = FD_Para_Up.u4Gain_W1_Y * Ratio_Up + FD_Para_Lo.u4Gain_W1_Y * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W1_U = FD_Para_Up.u4Gain_W1_U * Ratio_Up + FD_Para_Lo.u4Gain_W1_U * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W1_V = FD_Para_Up.u4Gain_W1_V * Ratio_Up + FD_Para_Lo.u4Gain_W1_V * Ratio_Lo + 0.5;
	
	FD_Para.u4Range_W2_Y = FD_Para_Up.u4Range_W2_Y * Ratio_Up + FD_Para_Lo.u4Range_W2_Y * Ratio_Lo + 0.5;
	FD_Para.u4Range_W2_U = FD_Para_Up.u4Range_W2_U * Ratio_Up + FD_Para_Lo.u4Range_W2_U * Ratio_Lo + 0.5;
	FD_Para.u4Range_W2_V = FD_Para_Up.u4Range_W2_V * Ratio_Up + FD_Para_Lo.u4Range_W2_V * Ratio_Lo + 0.5;
	FD_Para.u4W2_enable = FD_Para_Up.u4W2_enable * Ratio_Up + FD_Para_Lo.u4W2_enable * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W2_Y = FD_Para_Up.u4Gain_W2_Y * Ratio_Up + FD_Para_Lo.u4Gain_W2_Y * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W2_U = FD_Para_Up.u4Gain_W2_U * Ratio_Up + FD_Para_Lo.u4Gain_W2_U * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W2_V = FD_Para_Up.u4Gain_W2_V * Ratio_Up + FD_Para_Lo.u4Gain_W2_V * Ratio_Lo + 0.5;
	
	FD_Para.u4Range_W3_Y = FD_Para_Up.u4Range_W3_Y * Ratio_Up + FD_Para_Lo.u4Range_W3_Y * Ratio_Lo + 0.5;
	FD_Para.u4Range_W3_U = FD_Para_Up.u4Range_W3_U * Ratio_Up + FD_Para_Lo.u4Range_W3_U * Ratio_Lo + 0.5;
	FD_Para.u4Range_W3_V = FD_Para_Up.u4Range_W3_V * Ratio_Up + FD_Para_Lo.u4Range_W3_V * Ratio_Lo + 0.5;
	FD_Para.u4W3_enable = FD_Para_Up.u4W3_enable * Ratio_Up + FD_Para_Lo.u4W3_enable * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W3_Y = FD_Para_Up.u4Gain_W3_Y * Ratio_Up + FD_Para_Lo.u4Gain_W3_Y * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W3_U = FD_Para_Up.u4Gain_W3_U * Ratio_Up + FD_Para_Lo.u4Gain_W3_U * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W3_V = FD_Para_Up.u4Gain_W3_V * Ratio_Up + FD_Para_Lo.u4Gain_W3_V * Ratio_Lo + 0.5;
	
	FD_Para.u4Range_W4_Y = FD_Para_Up.u4Range_W4_Y * Ratio_Up + FD_Para_Lo.u4Range_W4_Y * Ratio_Lo + 0.5;
	FD_Para.u4Range_W4_U = FD_Para_Up.u4Range_W4_U * Ratio_Up + FD_Para_Lo.u4Range_W4_U * Ratio_Lo + 0.5;
	FD_Para.u4Range_W4_V = FD_Para_Up.u4Range_W4_V * Ratio_Up + FD_Para_Lo.u4Range_W4_V * Ratio_Lo + 0.5;
	FD_Para.u4W4_enable = FD_Para_Up.u4W4_enable * Ratio_Up + FD_Para_Lo.u4W4_enable * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W4_Y = FD_Para_Up.u4Gain_W4_Y * Ratio_Up + FD_Para_Lo.u4Gain_W4_Y * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W4_U = FD_Para_Up.u4Gain_W4_U * Ratio_Up + FD_Para_Lo.u4Gain_W4_U * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W4_V = FD_Para_Up.u4Gain_W4_V * Ratio_Up + FD_Para_Lo.u4Gain_W4_V * Ratio_Lo + 0.5;
	
	FD_Para.u4Range_W5_Y = FD_Para_Up.u4Range_W5_Y * Ratio_Up + FD_Para_Lo.u4Range_W5_Y * Ratio_Lo + 0.5;
	FD_Para.u4Range_W5_U = FD_Para_Up.u4Range_W5_U * Ratio_Up + FD_Para_Lo.u4Range_W5_U * Ratio_Lo + 0.5;
	FD_Para.u4Range_W5_V = FD_Para_Up.u4Range_W5_V * Ratio_Up + FD_Para_Lo.u4Range_W5_V * Ratio_Lo + 0.5;
	FD_Para.u4W5_enable = FD_Para_Up.u4W5_enable * Ratio_Up + FD_Para_Lo.u4W5_enable * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W5_Y = FD_Para_Up.u4Gain_W5_Y * Ratio_Up + FD_Para_Lo.u4Gain_W5_Y * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W5_U = FD_Para_Up.u4Gain_W5_U * Ratio_Up + FD_Para_Lo.u4Gain_W5_U * Ratio_Lo + 0.5;
	FD_Para.u4Gain_W5_V = FD_Para_Up.u4Gain_W5_V * Ratio_Up + FD_Para_Lo.u4Gain_W5_V * Ratio_Lo + 0.5;
	

	

	
}

MVOID FD_ANR_TBL(RAWIspCamInfo const& rCamInfo,
                          ISP_NVRAM_FD_ANR_PARAM_T const (& FD_ANR)[ISP_NVRAM_ANR_TBL_LV_NUM],
                          CAMERA_TUNING_FD_INFO_T const& rFdInfo,
                          ISP_NVRAM_ANR_LUT_T& rFd_Anr_Tbl)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.FD_ANR_TBL.enable", value, "0");
    MBOOL bFDANRDebug = atoi(value);

	
	MINT32 u4FaceNum = rFdInfo.FaceNum;

	MINT32 u4precision = 1;

	INTER_LOG_IF(bFDANRDebug,"[%s()] u4FaceNum: %d\n", __FUNCTION__, u4FaceNum);

	if(u4FaceNum == 0)
	{return;}
	
    for(int i = 0; i < u4FaceNum; i++)
	{
	INTER_LOG_IF(bFDANRDebug,"[%s()] faceIdx : %d, Ymean: %d, Umean: %d, Vmean: %d, Y5: %d, Y95: %d\n", __FUNCTION__, i, rFdInfo.YUVsts[i][0], rFdInfo.YUVsts[i][1], rFdInfo.YUVsts[i][2], rFdInfo.YUVsts[i][3], rFdInfo.YUVsts[i][4] );
	}
	
    MUINT32 u4RealLV = rCamInfo.rAEInfo.i4LightValue_x10;
	MUINT32 LV_00 = 120;
	MUINT32 LV_01 = 100;
	MUINT32 LV_02 = 50;

	MUINT32 u4UpperLV;
	MUINT32 u4LowerLV;

	MUINT32 u4UpperLV_Idx;
	MUINT32 u4LowerLV_Idx;

	
	static ISP_NVRAM_FD_ANR_PARAM_T FD_Para;
	static ISP_NVRAM_FD_ANR_PARAM_T FD_Para_Upper;
	static ISP_NVRAM_FD_ANR_PARAM_T FD_Para_Lower;
	ANRTbl8x8x8_T ANR8x8x8;
	ANRTblTuning_T ANR_Tuning[5];
	

	////FD Para interpolation////

	if(u4RealLV >= LV_01)
	{
	    u4UpperLV = LV_00;
		u4LowerLV = LV_01;
		u4UpperLV_Idx = 0;
		u4LowerLV_Idx = 1;
	}
	else
	{
	    u4UpperLV = LV_01;
		u4LowerLV = LV_02;
		u4UpperLV_Idx = 1;
		u4LowerLV_Idx = 2;
	}

	double RATIO_LV_lo = 0;
	double RATIO_LV_up = 0;

	if (u4RealLV <= u4LowerLV)
    {
        RATIO_LV_lo = 1;
    }
	else if(u4RealLV >= u4UpperLV)
	{
		RATIO_LV_lo = 0;
	}
	else
	{
		RATIO_LV_lo = (double)(u4UpperLV - u4RealLV) / (double)(u4UpperLV - u4LowerLV);
	}

	RATIO_LV_up = 1.0 - RATIO_LV_lo;


	memcpy(&FD_Para_Upper, &FD_ANR[u4UpperLV_Idx], sizeof(ISP_NVRAM_FD_ANR_PARAM_T));
	memcpy(&FD_Para_Lower, &FD_ANR[u4LowerLV_Idx], sizeof(ISP_NVRAM_FD_ANR_PARAM_T));
	InterParam_FD_ANR_TBL(RATIO_LV_up, RATIO_LV_lo, FD_Para_Upper, FD_Para_Lower, FD_Para);



	INTER_LOG_IF(bFDANRDebug,"[%s()] u4RealLV: %d, u4UpperLV = %d, u4LowerLV = %d\n", __FUNCTION__, u4RealLV, u4UpperLV, u4LowerLV);
	INTER_LOG_IF(bFDANRDebug,"[%s()] u4UpperLV_Idx = %d, u4LowerLV_Idx = %d\n", __FUNCTION__, u4UpperLV_Idx, u4LowerLV_Idx);
	INTER_LOG_IF(bFDANRDebug,"[%s()] RATIO_LV_up = %f, RATIO_LV_lo = %f\n", __FUNCTION__, RATIO_LV_up, RATIO_LV_lo);
	
	INTER_LOG_IF(bFDANRDebug,"[%s()] FD_Para: u4FD_ANR_LUT_Enable: %d, u4Range_W1_Y: %d, u4Range_W1_U: %d, u4Range_W1_V: %d, u4Gain_W1_Y: %d, u4Gain_W1_U: %d, u4Gain_W1_V: %d\n", 
		                        __FUNCTION__, FD_Para.u4FD_ANR_LUT_Enable, 
		                                      FD_Para.u4Range_W1_Y, FD_Para.u4Range_W1_U, FD_Para.u4Range_W1_V,
		                                      FD_Para.u4Gain_W1_Y, FD_Para.u4Gain_W1_U, FD_Para.u4Gain_W1_V);

	INTER_LOG_IF(bFDANRDebug,"[%s()] FD_Para:  u4Range_W2_Y: %d, u4Range_W2_U: %d, u4Range_W2_V: %d, u4Gain_W2_Y: %d, u4Gain_W2_U: %d, u4Gain_W2_V: %d\n", 
		                        __FUNCTION__, FD_Para.u4Range_W2_Y, FD_Para.u4Range_W2_U, FD_Para.u4Range_W2_V,
		                                      FD_Para.u4Gain_W2_Y, FD_Para.u4Gain_W2_U, FD_Para.u4Gain_W2_V);
	INTER_LOG_IF(bFDANRDebug,"[%s()] FD_Para:  u4Range_W3_Y: %d, u4Range_W3_U: %d, u4Range_W3_V: %d, u4Gain_W3_Y: %d, u4Gain_W3_U: %d, u4Gain_W3_V: %d\n", 
		                        __FUNCTION__, FD_Para.u4Range_W3_Y, FD_Para.u4Range_W3_U, FD_Para.u4Range_W3_V,
		                                      FD_Para.u4Gain_W3_Y, FD_Para.u4Gain_W3_U, FD_Para.u4Gain_W3_V);
	INTER_LOG_IF(bFDANRDebug,"[%s()] FD_Para:  u4Range_W4_Y: %d, u4Range_W4_U: %d, u4Range_W4_V: %d, u4Gain_W4_Y: %d, u4Gain_W4_U: %d, u4Gain_W4_V: %d\n", 
		                        __FUNCTION__, FD_Para.u4Range_W4_Y, FD_Para.u4Range_W4_U, FD_Para.u4Range_W4_V,
		                                      FD_Para.u4Gain_W4_Y, FD_Para.u4Gain_W4_U, FD_Para.u4Gain_W4_V);
	INTER_LOG_IF(bFDANRDebug,"[%s()] FD_Para:  u4Range_W5_Y: %d, u4Range_W5_U: %d, u4Range_W5_V: %d, u4Gain_W5_Y: %d, u4Gain_W5_U: %d, u4Gain_W5_V: %d\n", 
		                        __FUNCTION__, FD_Para.u4Range_W5_Y, FD_Para.u4Range_W5_U, FD_Para.u4Range_W5_V,
		                                      FD_Para.u4Gain_W5_Y, FD_Para.u4Gain_W5_U, FD_Para.u4Gain_W5_V);

	

	if(FD_Para.u4FD_ANR_LUT_Enable== 0)
	{return;}
	
    ANRTuningGen(rFdInfo, FD_Para, ANR_Tuning);
	ANRTBL_YUV8x8x8(rFd_Anr_Tbl, ANR8x8x8);
	ANRTablePBC(u4FaceNum, u4precision, ANR_Tuning, ANR8x8x8);
	YUV8x8x8_ANRTBL(ANR8x8x8, rFd_Anr_Tbl);
	
}

