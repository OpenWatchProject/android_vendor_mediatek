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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _PD_BUF_COMMON_H_
#define _PD_BUF_COMMON_H_

#include <cutils/log.h>
#include "MediaTypes.h"



typedef int                 MBOOL;
#define MTRUE       1
#define MFALSE      0


#define MAX_NUM_OF_SUPPORT_SENSOR 32

typedef enum
{
    EPDBuf_NotDef   = 0,
    EPDBuf_VC       = 1,
    EPDBuf_VC_Open  = 2,
    EPDBuf_Raw      = 3,
	EPDBuf_Raw_Open = 4
} EPDBuf_Type_t;


typedef struct
{
	EPDBuf_Type_t BufType;
	MINT32  i4CurrSensorId;
	MUINT32 u4IsZSD;
	MUINT32 u4Pdmode; /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode*/
	MUINT32 uImgXsz; /* current sensor output image size. */
	MUINT32 uImgYsz;
	MUINT32 uFullXsz; /* Full size. */
	MUINT32 uFullYsz;
	MUINT32 uSensorType; /*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(HDR), 3:PDAF VC mode(Binning)*/
    MUINT32 u4RawBit;
} SPDProfile_t;

typedef struct
{
	unsigned long		MajorVersion;
	unsigned long		MinorVersion;
} SPDLibVersion_t;

typedef struct
{
    MINT32 i4XStart;
    MINT32 i4YStart;
    MINT32 i4XEnd;
    MINT32 i4YEnd;
    MINT32 i4Info;
    
} SPDROI_T;

typedef struct
{
	MUINT32   curLensPos;
	UINT16   XSizeOfImage;
	UINT16   YSizeOfImage;	
	SPDROI_T ROI;
} SPDROIInput_T;


typedef struct
{
	signed long			Defocus;
	signed char			DefocusConfidence;
	unsigned long		DefocusConfidenceLevel;
	signed long			PhaseDifference;
} SPDROIResult_T;

typedef struct
{
	SPDROIResult_T ROIRes[MAX_NUM_OF_SUPPORT_SENSOR];
} SPDResult_T;



class SPDInputData_t
{
private :
	//default constructor is not allowed.
	SPDInputData_t(){}

public:
	SPDInputData_t( MUINT8 iNumROI, MUINT32 iBufSz, MUINT8 *iBufAddr)
	{
		numROI = iNumROI;
		ROI = new SPDROI_T [numROI]; 	

		databuf_size = iBufSz;
		databuf_virtAddr = iBufAddr; 	
	}
	
	~SPDInputData_t()
	{
		delete ROI;
		databuf_virtAddr = NULL;
	}

	MUINT32   frmNum;
	MUINT32   curLensPos;
	//ROI coordinate
	UINT16    XSizeOfImage;
	UINT16    YSizeOfImage;
	//PD analyze ROI
	MUINT8    numROI;
	SPDROI_T *ROI;
	//PD data buffer
	MUINT32   databuf_size;
	MUINT8   *databuf_virtAddr;

};


class SPDOutputData_t
{
private :
	//default constructor is not allowed.
	SPDOutputData_t(){}

public:
	SPDOutputData_t( MUINT8 iNumROI)
	{
		numRes = iNumROI;
		Res = new SPDROIResult_T [numRes]; 	
	}
	
	~SPDOutputData_t()
	{
		delete Res;
	}
	
	MUINT8          numRes;
	SPDROIResult_T *Res;


};





EPDBuf_Type_t GetPDBuf_Type( unsigned int a_u4CurrSensorDev, unsigned int a_u4CurrSensorId);


#endif // _PD_MGR_H_
